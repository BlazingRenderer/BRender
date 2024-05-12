cmake_minimum_required(VERSION 3.23)
project(BRender VERSION 1.4.0 LANGUAGES C CXX ASM
    DESCRIPTION "Modernized fork of BRender"
    HOMEPAGE_URL "https://github.com/BlazingRenderer/BRender"
)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

cmake_policy(SET CMP0079 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(Perl REQUIRED)

option(BRENDER_DISABLE_FINDSDL "Disable searching for SDL2" OFF)
if (NOT BRENDER_DISABLE_FINDSDL)
    find_package(SDL2)
endif ()

option(BRENDER_BUILD_DRIVERS "Build Drivers" ON)
option(BRENDER_BUILD_SOFT "Build Software Renderer" OFF)

if (BRENDER_BUILD_SOFT AND (NOT WIN32 OR NOT MSVC OR (NOT CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")))
    message(FATAL_ERROR "Can only build the software renderer on 32-bit Windows using MSVC.")
endif ()

if (NOT BRENDER_DISABLE_INSTALL)
    include(GNUInstallDirs)

    # Set this so CMake doesn't strip the RUNPATH on install.
    if (NOT APPLE)
        list(APPEND CMAKE_INSTALL_RPATH
                "${CMAKE_INSTALL_FULL_LIBDIR}"          # For BRender itself.
                "${CMAKE_INSTALL_FULL_LIBDIR}/brender"  # For the drivers.
        )
    endif ()
endif ()

if (WIN32)
    set(BRENDER_DRIVER_SUFFIX ".bdd")
else ()
    set(BRENDER_DRIVER_SUFFIX ".sobdd")
endif ()

add_subdirectory(core)

if (BRENDER_BUILD_DRIVERS)
    add_subdirectory(drivers)
endif ()

##
# Core BRender, no drivers, no DDI.
##
add_library(BRender::Core ALIAS brender)

##
# Full BRender, with drivers, no DDI.
##
add_library(BRender::Full ALIAS brender-full)

##
# Core BRender, no drivers, with DDI.
##
add_library(BRender::DDI ALIAS brender-ddi)


option(BRENDER_BUILD_TOOLS "Build Tools" ON)
option(BRENDER_BUILD_EXAMPLES "Build Examples" ON)
option(BRENDER_DISABLE_INSTALL "Disable installation" OFF)

if (BRENDER_BUILD_TOOLS)
    add_subdirectory(tools)
endif ()

if (BRENDER_BUILD_EXAMPLES AND BRENDER_BUILD_DRIVERS AND TARGET SDL2::SDL2)
    add_subdirectory(examples)
endif ()

if (NOT BRENDER_DISABLE_INSTALL)
    include(cmake/packaging.cmake)
endif ()
