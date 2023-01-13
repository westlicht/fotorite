#include "pool.h"

#include <doctest/doctest.h>

#include <string>

using namespace fr;

TEST_SUITE_BEGIN("pool");

TEST_CASE("Handle")
{
    using HandleType = Handle<struct Tag>;
    HandleType null = HandleType::null();
    CHECK_EQ(null.is_null(), true);
    CHECK_EQ(null, false);
}

TEST_CASE("Pool")
{
    using HandleType = Handle<struct Tag>;
    Pool<std::string, HandleType> pool;

    auto a = pool.alloc();
    REQUIRE(a);
    REQUIRE(pool.is_valid(a));
    *pool.get(a) = "hello";

    auto b = pool.alloc();
    REQUIRE(b);
    REQUIRE(pool.is_valid(b));
    *pool.get(b) = "world";

    pool.free(a);
    REQUIRE(!pool.is_valid(a));

    auto a2 = pool.alloc();
    REQUIRE(a2);
    REQUIRE(pool.is_valid(a2));
    *pool.get(a2) = "hello2";

    pool.free(b);
    REQUIRE(!pool.is_valid(b));

    auto b2 = pool.alloc();
    REQUIRE(b2);
    REQUIRE(pool.is_valid(b2));
    *pool.get(b2) = "world2";

}

TEST_SUITE_END();
