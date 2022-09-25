cmake_minimum_required(VERSION 3.13)
project(BRenderSample)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

find_package(Perl REQUIRED)

option(BRENDER_DISABLE_FINDSDL "Disable searching for SDL2" OFF)

if(NOT BRENDER_DISABLE_FINDSDL)
    # glrend needs this
    find_package(SDL2 REQUIRED)
endif()

add_subdirectory(brender)

option(BRENDER_BUILD_TOOLS    "Build Tools"    ON)
option(BRENDER_BUILD_EXAMPLES "Build Examples" ON)

if(BRENDER_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BRENDER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
