cmake_minimum_required(VERSION 3.13)
project(BRenderSample)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

#if(CMAKE_HOST_WIN32)
#    set(PERL "${CMAKE_CURRENT_SOURCE_DIR}/utils/perl.exe")
#else()
#    set(PERL "perl")
#endif()

set(PERL "perl")

# glrend needs this
find_package(SDL2 REQUIRED)

add_subdirectory(brender-crocde-1.4.0-dev)

add_subdirectory(mkshades)
add_subdirectory(mkranges)
