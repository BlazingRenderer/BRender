project(cube)

add_executable(cube
        main.c
)
target_compile_definitions(cube PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(cube BRender::Full)
set_property(TARGET cube PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
