#pragma once

#include "files/filesystem.h"
#include "fmt/core.h"
#include "json/json.h"
#include <chrono>
#include <map>
#include <string>

using ClockT = std::chrono::system_clock;

ClockT::time_point parseTime(std::string str) {
    std::tm tm = {};

    if (str.empty()) {
        return {};
    }

    tm.tm_year = std::stol(str.substr(0, 4)) - 1900;
    tm.tm_mon = std::stol(str.substr(5, 2)) - 1;
    tm.tm_mday = std::stol(str.substr(8, 2));

    tm.tm_hour = std::stol(str.substr(11, 2));
    tm.tm_min = std::stol(str.substr(14, 2));
    tm.tm_sec = std::stol(str.substr(17, 2));

    auto time = std::mktime(&tm);

    return ClockT::from_time_t(time);
}

std::tm timepointToTm(ClockT::time_point tp) {
    auto tt = ClockT::to_time_t(tp);

    return *std::localtime(&tt);
}

struct MetaData {

    std::map<std::string, std::string> values;

    filesystem::path metaPath;
    filesystem::path imgPath;
    filesystem::path thumbPath;

    ClockT::time_point time;

    std::pair<std::string, std::string> parse(std::string line) {
        auto f = line.find('=');

        return {line.substr(5, f - 5), line.substr(f + 1)};
    }

    //! Create metadata from output from "identify"
    MetaData(std::istream &stream,
             filesystem::path imgPath,
             filesystem::path metaPath,
             filesystem::path thumb) {
        for (std::string line; std::getline(stream, line);) {
            auto pair = parse(line);
            values[pair.first] = pair.second;
        }

        this->imgPath = imgPath;
        this->metaPath = metaPath;
        thumbPath = thumb;

        time = parseTime(get("DateTime"));
    }

    std::string get(std::string name) const {
        for (const auto &it : values) {
            if (it.first == name) {
                return it.second;
            }
        }
        return "";
    }

    //! Load metadata from json
    MetaData(filesystem::path path) {
        load(path);
    }

    friend std::ostream &operator<<(std::ostream &stream,
                                    const MetaData &data) {
        for (const auto &it : data.values) {
            fmt::print("{} = {}\n", it.first, it.second);
        }

        return stream;
    }

    void save() {
        auto json = Json{};
        json.type = Json::Object;

        json["img"] = imgPath.string();
        json["thumb"] = thumbPath.string();
        json["metaPath"] = metaPath.string();
        json["time"] = std::to_string(ClockT::to_time_t(time));

        auto &meta = json["meta"];

        meta.reserve(values.size());
        meta.type = Json::Array;

        for (const auto &it : values) {
            meta[it.first] = it.second;
        }

        std::ofstream{metaPath} << json;
    }

    void load(filesystem::path path) {
        auto json = Json{};

        metaPath = path;

        std::ifstream{path} >> json;

        const auto meta = json["meta"];

        imgPath = json["img"].value;
        thumbPath = json["thumb"].value;

        time = ClockT::from_time_t(std::stoul(json["time"].value));

        for (const auto &it : meta) {
            values[it.name] = it.value;
        }
    }

    std::string dateString() const {
        auto tt = ClockT::to_time_t(time);
        return std::ctime(&tt);
    }
};
