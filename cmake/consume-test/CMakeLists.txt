##
# A consumer of an installed BRender, for the packaging checks: find_package()
# against a prefix that is not the one BRender was configured with, and link the
# core library through the exported target.
##
cmake_minimum_required(VERSION 3.23)
project(brender-consume-test C)

find_package(BRENDER CONFIG REQUIRED)

add_executable(app main.c)
target_link_libraries(app PRIVATE BRender::Core)
