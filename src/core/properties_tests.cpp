#include "properties.h"

#include <doctest/doctest.h>

#include <iostream>

using namespace fr;

TEST_SUITE_BEGIN("properties");

template <typename T>
struct TestValues {
    static inline const T a{0};
    static inline const T b{1};
};

template <>
struct TestValues<nanogui::Vector2i> {
    static inline const nanogui::Vector2i a{1, 2};
    static inline const nanogui::Vector2i b{2, 3};
};

template <>
struct TestValues<nanogui::Vector3i> {
    static inline const nanogui::Vector3i a{1, 2, 3};
    static inline const nanogui::Vector3i b{2, 3, 4};
};

template <>
struct TestValues<std::string> {
    static inline const std::string a{"foo"};
    static inline const std::string b{"bar"};
};

TEST_CASE_TEMPLATE("Properties", T, bool, int, nanogui::Vector2i, nanogui::Vector3i, float, nanogui::Vector2f,
                   nanogui::Vector3f, std::string)
{
    Properties p;

    T a = TestValues<T>::a;
    T b = TestValues<T>::b;

    CHECK_EQ(p.has("test"), false);
    CHECK_EQ(p.get("test", b), b);
    p.set("test", a);
    CHECK_EQ(p.has("test"), true);
    CHECK_EQ(p.get("test", b), a);
}
