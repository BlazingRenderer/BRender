cmake_minimum_required(VERSION 3.12)
project(BRenderSample)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

find_package(Perl REQUIRED)

option(BRENDER_DISABLE_FINDSDL "Disable searching for SDL2" OFF)
if(NOT BRENDER_DISABLE_FINDSDL)
    find_package(SDL2)
endif()

option(BRENDER_BUILD_DRIVERS  "Build Drivers"  ON)

add_subdirectory(brender)

option(BRENDER_BUILD_TOOLS    "Build Tools"    ON)
option(BRENDER_BUILD_EXAMPLES "Build Examples" ON)

if(BRENDER_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BRENDER_BUILD_EXAMPLES AND BRENDER_BUILD_DRIVERS AND TARGET SDL2::SDL2)
    add_subdirectory(examples)
endif()
