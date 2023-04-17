project(texview)

add_executable(texview
        main.c
        )

target_compile_definitions(texview PRIVATE -DSDL_MAIN_HANDLED)
target_link_libraries(texview PRIVATE BRender::Core BRender::Drivers::SDL2Dev)

install(TARGETS texview)
