project(parg)

add_library(parg STATIC
        parg.c
        parg.h
)

target_include_directories(parg PUBLIC .)
set_property(TARGET parg PROPERTY FOLDER "BRender/Tools")
