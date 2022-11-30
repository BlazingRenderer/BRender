project(brsdl)

add_library(brsdl STATIC
        util.c
        include/brsdl.h
)

target_link_libraries(brsdl PUBLIC BRender::Core)
target_link_libraries(brsdl PUBLIC SDL2::SDL2)

target_include_directories(brsdl PUBLIC include)
