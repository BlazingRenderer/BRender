project(minzb)

add_executable(minzb
        minzb.c
)
target_link_libraries(minzb PRIVATE brdemo)
set_property(TARGET minzb PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(minzb PRIVATE SDL2::SDL2main)
endif()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS minzb DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
