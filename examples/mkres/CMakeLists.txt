project(mkres)

add_executable(mkres
        mkres.c
        quad.c
        cube.c
        checkerboard.c
)
target_link_libraries(mkres BRender::Core)
