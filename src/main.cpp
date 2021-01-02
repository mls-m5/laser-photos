
#include "files/filesystem.h"
#include "files/popenstream.h"
#include "hash.h"
#include "jpeglib.h"
#include <filesystem>
#include <fstream>
#include <iostream>

void index() {
    auto root = filesystem::current_path();

    for (const auto &it : filesystem::recursive_directory_iterator(root)) {
        if (filesystem::is_directory(it.path())) {
            continue;
        }

        std::cout << it.path() << " -> ";

        auto file = std::ifstream{it.path()};

        auto hash = ::hash(file);

        std::cout << hash << "\n";

        std::cout.flush();

        POpenStream stream{"identify -format '%[EXIF:*]' " +
                           it.path().string()};

        std::cout << stream.rdbuf();
    }
}

int main(int /*argc*/, char ** /*argv*/) {
    index();

    return 0;
}
