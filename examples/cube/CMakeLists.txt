project(cube)

add_executable(cube
        main.c
)
target_compile_definitions(cube PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(cube BRender::Full brsdl)
