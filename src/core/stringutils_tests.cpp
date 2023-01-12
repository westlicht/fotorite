#include "stringutils.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("stringutils");

TEST_CASE("to_lower") {
    CHECK_EQ(to_lower("ABC"), "abc");
    CHECK_EQ(to_lower(".TXT"), ".txt");
    CHECK_EQ(to_lower("Hello World"), "hello world");
}

TEST_CASE("to_upper") {
    CHECK_EQ(to_upper("abc"), "ABC");
    CHECK_EQ(to_upper(".txt"), ".TXT");
    CHECK_EQ(to_upper("Hello World"), "HELLO WORLD");
}

TEST_SUITE_END();
