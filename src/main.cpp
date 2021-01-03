
#include "files/filesystem.h"
#include "files/popenstream.h"
#include "fmt/core.h"
#include "hash.h"
#include "html/html.h"
#include "image/metadata.h"
#include "jpeglib.h"

namespace {

void index() {
    auto root = filesystem::current_path();

    //    auto inputDir = root / "input";
    auto inputDir = filesystem::path{"input"};

    for (const auto &it : filesystem::recursive_directory_iterator(inputDir)) {
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

        auto thumbPath = filesystem::path{"thumbs"} / (hash + ".jpg");

        // see
        // https://www.cyberciti.biz/tips/howto-linux-creating-a-image-thumbnails-from-shell-prompt.html
        system((std::string{"convert -auto-orient -thumbnail x200 '"} +
                it.path().string() + "' '" + thumbPath.string() + "'")
                   .c_str());

        auto metaData = MetaData{stream,
                                 it.path(),
                                 filesystem::path{"data"} / (hash + ".json"),
                                 thumbPath};

        metaData.save();
    }
}

filesystem::path generate() {
    //    auto root = filesystem::current_path();
    //    auto dataDir = root / "data";
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
            {{"img",
              {},
              {
                  {"src", meta.thumbPath},
                  {"title", meta.dateString()},
              }}},
            {{"href", meta.imgPath.string()}},
        });
    }

    auto html = Html{
        "html",
        {
            {
                "head",
                {
                    {"meta", {}, {{"charset", "utf-8"}}},
                },
            },
            images,
        },
    };

    html.print(std::cout);

    auto filename = filesystem::path{"index.html"};

    std::ofstream file{filename};

    if (!file.is_open()) {
        throw std::runtime_error{"could not open output file " +
                                 filesystem::absolute(filename).string()};
    }

    html.print(file);

    return filename;
}

const std::string helpString = R"_(
index              index the image folder
generate           generate

)_";

struct Settings {
    bool shouldIndex = false;
    bool shouldGenerate = false;

    Settings(int argc, char **argv) {
        std::vector<std::string> args{argv + 1, argv + argc};

        if (args.size() < 1) {
            std::cerr << "no arguments\n";
            std::cerr << helpString << "\n";
            throw std::runtime_error("to few arguments");
        }

        for (auto arg : args) {
            if (arg == "index") {
                shouldIndex = true;
            }
            else if (arg == "generate") {
                shouldGenerate = true;
            }
            else {
                fmt::print("unknown argument {}\n", arg);
            }
        }
    }
};

} // namespace

int main(int argc, char **argv) {

    const auto settings = Settings{argc, argv};

    if (settings.shouldIndex) {
        index();
    }

    if (settings.shouldGenerate) {
        generate();
    }

    return 0;
}
