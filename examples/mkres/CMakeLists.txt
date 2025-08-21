project(mkres)

add_executable(mkres
        mkres.c
        quad.c
        cube.c
        checkerboard.c
        smpte.c
        vga.c
)
target_link_libraries(mkres BRender::Core)
set_property(TARGET mkres PROPERTY FOLDER "BRender/Examples")