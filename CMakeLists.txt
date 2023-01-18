cmake_minimum_required(VERSION 3.23)

project(fotorite)

# -----------------------------------------------------------------------------
# configuration
# -----------------------------------------------------------------------------

option(FOTORITE_ENABLE_TESTS "Enable tests" ON)
option(FOTORITE_GITHUB_CI "GitHub CI" OFF)

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
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)

find_path(BS_THREAD_POOL_INCLUDE_DIRS "BS_thread_pool.hpp")
add_library(BS_thread_pool INTERFACE)
target_include_directories(BS_thread_pool INTERFACE ${BS_THREAD_POOL_INCLUDE_DIRS})

# nanogui
set(NANOGUI_BUILD_EXAMPLES ON)
set(NANOGUI_BUILD_SHARED OFF)
set(NANOGUI_BUILD_PYTHON OFF)
set(NANOGUI_INSTALL OFF)
add_subdirectory(ext/nanogui)

# dxc
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(DXC_URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/dxc_2022_12_16.zip)
    set(DXC_REL_PATH bin/x64/dxc.exe)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(DXC_URL https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2212/linux_dxc_2022_12_16.tar.gz)
    set(DXC_REL_PATH bin/x64/dxc)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(DXC_URL http://westlicht.ch/macos_dxc_2021_07_08.zip)
    set(DXC_REL_PATH bin/dxc)
endif()

include(FetchContent)
FetchContent_Declare(dxc URL ${DXC_URL})
FetchContent_MakeAvailable(dxc)

set(DXC ${dxc_SOURCE_DIR}/${DXC_REL_PATH})

if (CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    file(CHMOD ${DXC} FILE_PERMISSIONS OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE)
endif()

# -----------------------------------------------------------------------------
# shaders
# -----------------------------------------------------------------------------

set(DXC_FLAGS -spirv -fspv-target-env=vulkan1.2 -fvk-use-dx-layout)
set(SHADER_SRC_DIR ${PROJECT_SOURCE_DIR}/src/shaders)
set(SHADER_BIN_DIR ${PROJECT_BINARY_DIR}/shaders)

set(shaders
    test.cs.hlsl
)

foreach(shader ${shaders})
    get_filename_component(name ${shader} NAME_WLE)
    string(MAKE_C_IDENTIFIER ${name} cid)
    set(source_file ${SHADER_SRC_DIR}/${shader})
    set(header_file ${SHADER_BIN_DIR}/${cid}.h)
    set(spirv_file ${SHADER_BIN_DIR}/${name}.spv)
    add_custom_command(
        OUTPUT ${header_file}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
        COMMAND ${DXC} ${DXC_FLAGS} -I ${SHADER_SRC_DIR} -T cs_6_0 -Fo ${spirv_file} -Vn ${cid} -Fh ${header_file} ${source_file}
        DEPENDS ${source_file}
    )
    list(APPEND header_files ${header_file})
endforeach()

add_custom_target(shaders DEPENDS ${header_files})

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
    src/model/catalog.cpp
    src/model/document.cpp
    src/process/device.cpp
    src/shaders/shaders.cpp
    src/ui/catalog_view.cpp
    src/ui/main_screen.cpp
    src/ui/panel.cpp
)
target_include_directories(core PUBLIC src PRIVATE ${JPEG_INCLUDE_DIR} ${SHADER_BIN_DIR})
target_compile_features(core PUBLIC cxx_std_20)
target_compile_definitions(core PRIVATE _CRT_SECURE_NO_WARNINGS UNICODE)
target_compile_options(core PUBLIC
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4 /WX>
    $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/wd4505 /wd4189 /wd4244 /wd4100>
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wall -Wextra -Wpedantic -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,AppleClang,Clang>:-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-field-initializers>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wall -Wextra -Werror>
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers>
)
target_link_libraries(core PUBLIC fmt::fmt spdlog::spdlog nlohmann_json::nlohmann_json nanogui PRIVATE ${JPEG_LIBRARIES} PNG::PNG TinyEXIFstatic BS_thread_pool Vulkan::Headers volk::volk)

add_dependencies(core shaders)

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
        src/core/pool_tests.cpp
        src/core/properties_tests.cpp
        src/core/settings_tests.cpp
        src/core/stringutils_tests.cpp
        src/process/device_tests.cpp
    )
    target_link_libraries(fotorite_tests PRIVATE core doctest::doctest)
    target_compile_definitions(fotorite_tests PRIVATE FOTORITE_GITHUB_CI=$<BOOL:${FOTORITE_GITHUB_CI}>)

    add_test(NAME fotorite_tests COMMAND fotorite_tests)
endif()
