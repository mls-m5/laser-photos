
#include "json/json.h"
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <fmt/core.h>
#include <sstream>

// One example
EM_JS(void, say_hello, (const char *str), {
    console.log('hello ' + UTF8ToString(str));
})

EMSCRIPTEN_KEEPALIVE
void jsonCallback(char *str, unsigned len) {
    auto ss = std::stringstream{std::string{str, len}};

    auto json = Json{ss};

    say_hello("json test");
}

int main() {
    fmt::print("hello\n");

    say_hello("you");
}
