project(fogdem)

add_executable(fogdem
        fogdem.c
)
target_link_libraries(fogdem PRIVATE brdemo)
set_property(TARGET fogdem PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
    target_link_libraries(fogdem PRIVATE SDL2::SDL2main)
endif()

set(FOGDEM_FILES earth.pix fogdem.mat fogdem.pal fogshade.tab fogtable.tab)

foreach(file ${FOGDEM_FILES})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS fogdem DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
