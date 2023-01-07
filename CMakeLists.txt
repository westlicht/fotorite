cmake_minimum_required(VERSION 3.23)

project(fotorite)

find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenImageIO CONFIG REQUIRED)

add_executable(fotorite src/main.cpp)

target_link_libraries(fotorite PRIVATE fmt::fmt spdlog::spdlog OpenImageIO::OpenImageIO)


