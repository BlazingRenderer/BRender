cmake_minimum_required(VERSION 3.12)
project(BRender)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Perl REQUIRED)

option(BRENDER_DISABLE_FINDSDL "Disable searching for SDL2" OFF)
if(NOT BRENDER_DISABLE_FINDSDL)
    find_package(SDL2)
endif()

option(BRENDER_BUILD_DRIVERS  "Build Drivers"  ON)
option(BRENDER_BUILD_SOFT "Build Software Renderer" OFF)

if(BRENDER_BUILD_SOFT AND (NOT WIN32 OR NOT MSVC OR (NOT CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")))
	message(FATAL_ERROR "Can only build the software renderer on 32-bit Windows using MSVC.")
endif()

add_subdirectory(brender)

option(BRENDER_BUILD_TOOLS    "Build Tools"    ON)
option(BRENDER_BUILD_EXAMPLES "Build Examples" ON)

if(BRENDER_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BRENDER_BUILD_EXAMPLES AND BRENDER_BUILD_DRIVERS AND TARGET SDL2::SDL2)
    add_subdirectory(examples)
endif()
