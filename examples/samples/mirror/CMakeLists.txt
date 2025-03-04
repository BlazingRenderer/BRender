project(mirror)

add_executable(mirror
        mirror.c
)
target_link_libraries(mirror PRIVATE brdemo)
set_property(TARGET mirror PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
    target_link_libraries(mirror PRIVATE SDL2::SDL2main)
endif()

set(MIRROR_FILES blend.tab cube.dat floor.dat test.mat testtex.pix tile.pix winsh8.tab winstd.pal)

foreach(file ${MIRROR_FILES})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS mirror DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
