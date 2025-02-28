project(minzs)

add_executable(minzs
        minzs.c
)
target_link_libraries(minzs PRIVATE brdemo)
set_property(TARGET minzs PROPERTY FOLDER "BRender/Examples/Samples")

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS minzs DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
