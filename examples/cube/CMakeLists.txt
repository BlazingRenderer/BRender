project(cube)

add_executable(cube
        main.c
)
target_link_libraries(cube BRender::Full)
