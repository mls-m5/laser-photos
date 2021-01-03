
#define DO_NOT_CATCH_ERRORS

#include "image/metadata.h"
#include "mls-unit-test/unittest.h"

TEST_SUIT_BEGIN

TEST_CASE("string to timepoint") {
    auto tp = parseTime("2021:02:03 00:10:40");

    auto tt = std::chrono::system_clock::to_time_t(tp);

    std::cout << ctime(&tt) << std::endl;
}

TEST_CASE("convert time_point to tm") {
    auto tp = parseTime("2021:02:03 00:10:40");

    auto tm = timepointToTm(tp);

    ASSERT_EQ(tm.tm_year, 121);
}

TEST_SUIT_END
