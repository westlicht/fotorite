#include "settings.h"

#include <doctest/doctest.h>

#include <iostream>

using namespace fr;

TEST_SUITE_BEGIN("settings");

TEST_CASE("Settings")
{
    std::filesystem::path path{"settings.json"};

    Properties save_section1;
    save_section1.set("bool", false);
    save_section1.set("int", 123);

    Properties save_section2;
    save_section2.set("bool", true);
    save_section2.set("int", 234);

    Settings save_settings;
    save_settings.set("section1", save_section1);
    save_settings.set("section2", save_section2);

    REQUIRE_EQ(save_settings.save(path), true);

    Settings load_settings;
    REQUIRE_EQ(load_settings.load(path), true);

    // Properties load_section1 = load_settings.get("section1");
    // CHECK_EQ(load_section1.get<bool>("bool", true), false);
    // CHECK_EQ(load_section1.get<int>("int", 234), 123);

    // Properties load_section2 = load_settings.get("section2");
    // CHECK_EQ(load_section2.get<bool>("bool", false), true);
    // CHECK_EQ(load_section2.get<int>("int", 123), 234);
}
