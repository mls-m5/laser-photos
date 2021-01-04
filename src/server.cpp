
#include "files/filesystem.h"
#include "generate.h"
#include <array>
#include <asio/io_service.hpp>
#include <asio/ip/tcp.hpp>
#include <fmt/core.h>
#include <fstream>
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

struct Header {
    std::string protocol;
    filesystem::path location;

    Header(std::string data) {
        std::istringstream ss(data);

        {
            std::string line;
            std::getline(ss, line);

            fmt::print("first line: {}\n", line);

            std::istringstream liness(line);

            std::string location;

            liness >> protocol;
            liness >> location;

            if (location.find("..") != std::string::npos) {
                location = ""; // Should not be possible to exit root folder
            }

            if (location.front() == '/') {
                location.insert(location.begin(), '.');
            }

            this->location = location;
        }

        for (std::string line; std::getline(ss, line);) {
            fmt::print("settings: {}\n", line);
        }
    }
};

size_t fileLen(filesystem::path path) {
    auto file = std::ifstream{path, std::ios::binary};
    file.seekg(0, std::ios::end);
    return file.tellg();
}

void sendFile(asio::ip::tcp::socket &socket, filesystem::path path) {

    auto fullPath = filesystem::absolute(filesystem::current_path()) / path;
    //    auto fullPath = "bajs" / path;

    auto file = std::ifstream{fullPath, std::ios::binary};
    if (file.is_open()) {

        auto data = std::array<char, 100000>{};

        while (auto count = file.readsome(data.data(), data.size())) {
            socket.write_some(asio::buffer(data.data(), count));
        }
    }
    else {
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

    sendFile(socket, path);
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

    if (ext == "jpg" || ext == "jpeg") {
        return true;
    }
    else if (ext == "png") {
        return true;
    }

    return false;
}

class HttpServer {
public:
    void start() {
        using asio::ip::tcp;

        try {
            asio::io_service service;

            tcp::acceptor acceptor{service, tcp::endpoint{tcp::v4(), 9042}};

            while (true) {
                if (true) {
                    auto socket = tcp::socket{service};
                    acceptor.accept(socket);

                    std::cout << "request started" << std::endl;

                    socket.wait(socket.wait_read);

                    std::vector<char> buffer(socket.available());

                    socket.read_some(asio::buffer(buffer));

                    auto header =
                        Header{std::string{buffer.data(), buffer.size()}};

                    std::cout << "trying to access " << header.location
                              << std::endl;

                    if (header.location == "./") {
                        sendIndexFile(socket);
                    }
                    else if (isImage(header.location)) {
                        sendImg(socket, header.location);
                    }
                    else {
                        sendFileNotFound(socket, header.location);
                    }
                }

                std::cout << "request finished" << std::endl;
            }
        }
        catch (std::exception &e) {
            std::cerr << e.what() << "\n";
        }
    }
};

} // namespace

int main(int argc, char *argv[]) {
    HttpServer server;

    server.start();

    return 0;
}
