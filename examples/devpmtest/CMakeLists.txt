project(devpmtest)

add_executable(devpmtest
        main.c
)
target_compile_definitions(devpmtest PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(devpmtest BRender::Full)
set_property(TARGET devpmtest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
