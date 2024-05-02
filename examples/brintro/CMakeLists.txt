project(brintro)

add_executable(brintro
        main.c
        intro.c
        intro.h
)
target_link_libraries(brintro PRIVATE brdemo)
set_property(TARGET brintro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET brintro PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
    target_link_libraries(brintro PRIVATE SDL2::SDL2main)
endif()
