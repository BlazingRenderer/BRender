project(brintro)

add_executable(brintro
        main.c
        intro.c
        intro.h
)
target_compile_definitions(brintro PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(brintro BRender::Full brdemo)
set_property(TARGET brintro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET brintro PROPERTY FOLDER "BRender/Examples")
