project(brintro)

add_executable(brintro
        main.c
        intro.c
        intro.h
)
target_link_libraries(brintro PRIVATE brdemo)
set_property(TARGET brintro PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_property(TARGET brintro PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS brintro DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
