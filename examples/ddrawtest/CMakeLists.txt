project(ddrawtest)

add_executable(ddrawtest main.c)

target_compile_definitions(ddrawtest PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(ddrawtest BRender::Full BRender::DDI)
set_property(TARGET ddrawtest PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET ddrawtest PROPERTY FOLDER "BRender/Examples")
