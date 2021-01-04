#pragma once

#include "files/filesystem.h"
#include "files/popenstream.h"
#include "fmt/core.h"
#include "hash.h"
#include "image/metadata.h"
#include <fstream>
#include <iostream>

inline void index() {
    auto root = filesystem::current_path();

    auto inputDir = filesystem::path{"input"};

    for (const auto &it : filesystem::recursive_directory_iterator(inputDir)) {
        if (filesystem::is_directory(it.path())) {
            continue;
        }

        auto ext = it.path().extension();
        if (ext != ".jpg" && ext != ".jpeg") {
            fmt::print("skipping non-image {}\n", it.path().string());
            continue;
        }

        std::cout << it.path() << " -> ";

        auto file = std::ifstream{it.path()};

        auto hash = ::hash(file);

        std::cout << hash << "\n";

        std::cout.flush();

        POpenStream stream{"identify -format '%[EXIF:*]' '" +
                           it.path().string() + "'"};

        if (stream.returnCode()) {
            fmt::print("skipping {}\n", it.path().string());
            continue;
        }

        auto thumbPath = filesystem::path{"thumbs"} / (hash + ".jpg");

        // see
        // https://www.cyberciti.biz/tips/howto-linux-creating-a-image-thumbnails-from-shell-prompt.html
        bool failed =
            system((std::string{"convert -auto-orient -thumbnail x200 '"} +
                    it.path().string() + "' '" + thumbPath.string() + "'")
                       .c_str());

        if (failed) {
            fmt::print("skipping {} because of thumb failed\n",
                       it.path().string());
            continue;
        }

        auto metaData = MetaData{stream,
                                 it.path(),
                                 filesystem::path{"data"} / (hash + ".json"),
                                 thumbPath,
                                 hash};

        auto time = metaData.time;

        auto tm = timepointToTm(time);

        auto dir = filesystem::path{"data"} /
                   std::to_string(tm.tm_year + 1900) /
                   std::to_string(tm.tm_mon + 1) / std::to_string(tm.tm_mday);

        filesystem::create_directories(dir);

        auto newName = dir / it.path().filename();

        filesystem::copy(
            it.path(), newName, filesystem::copy_options::skip_existing);

        metaData.imgPath = newName;

        metaData.save();
    }

    filesystem::remove("index.html");
}
