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
find_package(tinyxml2 CONFIG REQUIRED)
find_package(TinyEXIF CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_path(BS_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE ${BS_THREAD_POOL_INCLUDE_DIRS})

# nanogui
set(NANOGUI_BUILD_EXAMPLES ON)
set(NANOGUI_BUILD_SHARED OFF)
set(NANOGUI_BUILD_PYTHON OFF)
set(NANOGUI_INSTALL OFF)
add_subdirectory(ext/nanogui)

# -----------------------------------------------------------------------------
# core library
# -----------------------------------------------------------------------------
add_library(core STATIC)
target_sources(core PRIVATE
    src/core/core.cpp
    src/core/fileio.cpp
    src/core/imageio.cpp
    src/core/properties.cpp
    src/core/settings.cpp
    src/core/stringutils.cpp
    src/model/document.cpp
    src/model/catalog.cpp
    src/ui/catalog_view.cpp
    src/ui/main_screen.cpp
    src/ui/panel.cpp
)
target_include_directories(core PUBLIC src PRIVATE ${JPEG_INCLUDE_DIR})
target_compile_features(core PUBLIC cxx_std_20)
target_compile_definitions(core PRIVATE _CRT_SECURE_NO_WARNINGS UNICODE)
target_compile_options(core PUBLIC
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4505 /wd4189 /wd4244 /wd4100>
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-gnu-anonymous-struct -Wno-nested-anon-types>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wall -Wextra -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable>
)
target_link_libraries(core PUBLIC fmt::fmt spdlog::spdlog nlohmann_json::nlohmann_json nanogui PRIVATE ${JPEG_LIBRARIES} PNG::PNG TinyEXIFstatic BS_thread_pool)

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
        src/tests.cpp
        src/core/fileio_tests.cpp
        src/core/imageio_tests.cpp
        src/core/properties_tests.cpp
        src/core/settings_tests.cpp
        src/core/stringutils_tests.cpp
    )
    target_link_libraries(fotorite_tests PRIVATE core doctest::doctest)

    add_test(NAME fotorite_tests COMMAND fotorite_tests)
endif()
