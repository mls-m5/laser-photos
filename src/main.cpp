
#include "generate.h"
#include "index.h"

namespace {

const std::string helpString = R"_(
index              index the image folder
generate           generate

)_";

//! Settings loaded from command line arguments
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
        auto filename = filesystem::path{"index.html"};
        std::ofstream file{filename};
        if (!file.is_open()) {
            throw std::runtime_error{"could not open output file " +
                                     filesystem::absolute(filename).string()};
        }

        generate(file);
    }

    return 0;
}
