project(gltfdemo)

add_executable(gltfdemo
        gltfdemo.c
)

target_link_libraries(gltfdemo PRIVATE brdemo)
set_property(TARGET gltfdemo PROPERTY FOLDER "BRender/Examples/Samples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS gltfdemo DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
