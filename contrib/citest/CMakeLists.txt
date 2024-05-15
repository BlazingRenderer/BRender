cmake_minimum_required(VERSION 3.23)
project(brender-ci-test)

find_package(BRender 1.4.0 REQUIRED)

add_executable(citest main.c)
target_link_libraries(citest PRIVATE BRender::Core BRender::DDI)

