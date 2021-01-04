// Copyright Mattias Larsson SKöld 2020

#include "files/filesystem.h"
#include "generate.h"
#include "http/httpserver.h"
#include <array>
#include <fmt/core.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Settings {
    Settings(int argc, char **argv) {
        auto args = std::vector<std::string>{argv + 1, argv + argc};
    }
};

size_t fileLen(filesystem::path path) {
    auto file = std::ifstream{path, std::ios::binary};
    file.seekg(0, std::ios::end);
    return file.tellg();
}

void sendFile(asio::ip::tcp::socket &socket, filesystem::path path) {

    auto fullPath = filesystem::absolute(filesystem::current_path()) / path;

    auto file = std::ifstream{fullPath, std::ios::binary};
    if (file.is_open()) {
        auto data = std::array<char, 100000>{};

        while (auto count = file.readsome(data.data(), data.size())) {
            socket.write_some(asio::buffer(data.data(), count));
        }
    }
    else {
        fmt::print("could not load {}\n" + path.string());
        socket.write_some(
            asio::buffer("could not load " + path.string() + "\n"));
    }
}

void sendFileNotFound(asio::ip::tcp::socket &socket, filesystem::path path) {
    auto httpHeader =
        asio::buffer("HTTP/1.1 404 OK\r\n"
                     "Content-Type: text/html; charset=\"UTF-8\"\r\n"
                     "Connection: Keep-Alive\r\n"
                     "\r\n");

    socket.write_some(httpHeader);
}

void sendIndexFile(asio::ip::tcp::socket &socket) {
    constexpr auto httpHeader =
        std::string_view{"HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=\"UTF-8\"\r\n"
                         "Connection: Keep-Alive\r\n"
                         "\r\n"};

    std::ostringstream ss;

    ss << httpHeader;

    generate(ss);

    socket.write_some(asio::buffer(ss.str()));
}

void sendHtml(asio::ip::tcp::socket &socket, filesystem::path path) {
    constexpr auto httpHeader =
        std::string_view{"HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html; charset=\"UTF-8\"\r\n"
                         "Connection: Keep-Alive\r\n"
                         "\r\n"};

    socket.write_some(asio::buffer(httpHeader));

    sendFile(socket, "../html" / path);
}

void sendCode(asio::ip::tcp::socket &socket, filesystem::path path) {
    constexpr auto httpHeader =
        std::string_view{"HTTP/1.1 200 OK\r\n"
                         "Content-Type: application/wasm\r\n"
                         //"Content-Type: application/octet-stream\r\n"
                         "\r\n"};

    socket.write_some(asio::buffer(httpHeader));

    sendFile(socket, "../bin/html" / path);
}

void sendImg(asio::ip::tcp::socket &socket, filesystem::path path) {

    auto httpHeader = fmt::format("HTTP/1.1 200 OK\r\n"
                                  "Content-Type: image/jpeg\r\n"
                                  "Content-length: {} \r\n"
                                  "\r\n",
                                  std::to_string(fileLen(path)));

    socket.write_some(asio::buffer(httpHeader));

    sendFile(socket, path);
}

bool isImage(filesystem::path path) {
    auto ext = path.extension();

    if (ext == ".jpg" || ext == ".jpeg") {
        return true;
    }
    else if (ext == ".png") {
        return true;
    }

    return false;
}

} // namespace

int main(int argc, char *argv[]) {
    HttpServer server;

    server.addFilter(
        [](auto &&header) { return header.location == "./"; },
        [](auto &&socket, auto &&header) { sendIndexFile(socket); });

    server.addFilter(
        [](auto &&header) { return isImage(header.location); },
        [](auto &&socket, auto &&header) { sendImg(socket, header.location); });

    server.addFilter(
        [](auto &&header) { return header.location.extension() == ".html"; },
        [](auto &&socket, auto &&header) {
            sendHtml(socket, header.location);
        });

    server.addFilter(
        [](auto &&header) {
            return header.location.extension() == ".wasm" ||
                   header.location.extension() == ".js";
        },
        [](auto &&socket, auto &&header) {
            sendCode(socket, header.location);
        });

    server.defaultAction([](auto &&socket, const HttpHeader &header) {
        sendFileNotFound(socket, header.location);
    });

    server.start();

    return 0;
}
