cmake_minimum_required(VERSION 3.23)

project(fotorite)

option(FOTORITE_ENABLE_TESTS "Enable tests" ON)

# dependencies provided by vcpkg
find_package(doctest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)

add_executable(fotorite src/main.cpp)

target_compile_features(fotorite PRIVATE cxx_std_17)
target_link_libraries(fotorite PRIVATE fmt::fmt spdlog::spdlog OpenImageIO::OpenImageIO)

if(FOTORITE_ENABLE_TESTS)
    enable_testing()

    add_executable(fotorite_tests
        tests/test_main.cpp
    )
    target_link_libraries(fotorite_tests PRIVATE doctest::doctest)

    if (MSVC)
        target_compile_options(fotorite_tests PRIVATE /W4 /WX)
    else()
        add_compile_options(fotorite_tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()

    add_test(NAME fotorite_tests COMMAND fotorite_tests)
endif()
