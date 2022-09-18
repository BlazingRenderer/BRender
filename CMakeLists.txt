cmake_minimum_required(VERSION 3.13)
project(BRenderSample)

# Prefer glvnd
cmake_policy(SET CMP0072 NEW)

find_package(Perl REQUIRED)

# glrend needs this
find_package(SDL2 REQUIRED)

add_subdirectory(brender)

add_subdirectory(mkshades)
add_subdirectory(mkranges)
add_subdirectory(robot)
