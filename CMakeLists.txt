cmake_minimum_required(VERSION 3.23)

project(fotorite)

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------
option(FOTORITE_ENABLE_TESTS "Enable tests" ON)

# -----------------------------------------------------------------------------
# dependencies
# -----------------------------------------------------------------------------

# find dependencies provided by vcpkg
find_package(doctest CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)

# -----------------------------------------------------------------------------
# core library
# -----------------------------------------------------------------------------
add_library(core STATIC)
target_sources(core PRIVATE
    src/libcore/core/core.cpp
)
target_compile_features(core PUBLIC cxx_std_17)
target_link_libraries(core PUBLIC fmt::fmt spdlog::spdlog)

if(MSVC)
    target_compile_options(core PUBLIC /W4 /WX)
else()
    target_compile_options(core PUBLIC -Wall -Wextra -Wpedantic -Werror)
endif()

# -----------------------------------------------------------------------------
# application
# -----------------------------------------------------------------------------
add_executable(fotorite src/main.cpp)
target_link_libraries(fotorite PRIVATE core)

# -----------------------------------------------------------------------------
# tests
# -----------------------------------------------------------------------------
if(FOTORITE_ENABLE_TESTS)
    enable_testing()

    add_executable(fotorite_tests
        tests/test_main.cpp
    )
    target_link_libraries(fotorite_tests PRIVATE core doctest::doctest)

    add_test(NAME fotorite_tests COMMAND fotorite_tests)
endif()
