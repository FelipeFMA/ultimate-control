cmake_minimum_required(VERSION 3.10)
project(ultimate_control VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(
    ${GTKMM_INCLUDE_DIRS}
    src
)

link_directories(
    ${GTKMM_LIBRARY_DIRS}
)

add_definitions(${GTKMM_CFLAGS_OTHER})

file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/core/*.cpp
    src/settings/*.cpp
    src/utils/*.cpp
    src/utils/qrcodegen/*.cpp
    src/bluetooth/*.cpp
    src/wifi/*.cpp
    src/volume/*.cpp
    src/power/*.cpp
    src/display/*.cpp
)

add_executable(ultimate-control ${SOURCES})

target_link_libraries(ultimate-control ${GTKMM_LIBRARIES})
