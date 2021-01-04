#pragma once

#include "files/filesystem.h"
#include "html/html.h"
#include "image/metadata.h"
#include <iostream>

void generate(std::ostream &stream) {
    auto dataDir = "data";

    std::vector<MetaData> metaData;

    metaData.reserve(10000);

    for (const auto &it : filesystem::recursive_directory_iterator{dataDir}) {
        if (it.path().extension() != ".json") {
            continue;
        }

        metaData.emplace_back(it.path());
    }

    auto images = Html{"div"};

    images.children().reserve(metaData.size());

    for (const auto &meta : metaData) {
        fmt::print("{}\n", meta.imgPath.string());

        images.children().push_back({
            "a",
            {{"href", meta.imgPath.string()}},
            {{"img",
              {
                  {"src", meta.thumbPath},
                  {"title", meta.dateString()},
              }}},
        });
    }

    auto html = Html{
        "html",
        {},
        {
            {
                "head",
                {},
                {
                    {"meta", {{"charset", "utf-8"}}, {}},
                },
            },
            images,
        },
    };

    html.print(std::cout);

    html.print(stream);
}
