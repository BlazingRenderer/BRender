project(minzb)

add_executable(minzb
        minzb.c
)
target_link_libraries(minzb PRIVATE brdemo)
set_property(TARGET minzb PROPERTY FOLDER "BRender/Examples/Samples")

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS minzb DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
