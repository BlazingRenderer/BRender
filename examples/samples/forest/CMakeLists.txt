project(forest)

add_executable(forest
        forest.c
)
target_link_libraries(forest PRIVATE brdemo)
set_property(TARGET forest PROPERTY FOLDER "BRender/Examples/Samples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS forest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
