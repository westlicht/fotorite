#include "properties.h"

#include <doctest/doctest.h>

using namespace fr;

TEST_SUITE_BEGIN("properties");

TEST_CASE("Properties") {

    Properties p;

    p.set("bool1", false);
    p.set("bool2", true);

}
