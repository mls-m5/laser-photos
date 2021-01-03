#include "image/metadata.h"
#include "mls-unit-test/unittest.h"

TEST_SUIT_BEGIN

TEST_CASE("string to timepoint") {
    auto tp = parseTime("2021:01:01 00:10:40");
}

TEST_SUIT_END
