project(brdemo)

add_library(brdemo STATIC
        brdemo.h
        brdemo.c
)

target_link_libraries(brdemo PUBLIC BRender::Full)
target_include_directories(brdemo PUBLIC .)
set_property(TARGET brdemo PROPERTY FOLDER "BRender/Examples")
