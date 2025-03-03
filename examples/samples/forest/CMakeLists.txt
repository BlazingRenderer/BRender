project(forest)

add_executable(forest
        forest.c
)
target_link_libraries(forest PRIVATE brdemo)
set_property(TARGET forest PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
    target_link_libraries(forest PRIVATE SDL2::SDL2main)
endif()

set(FOREST_FILES cloud.pix column.dat column.pix floor.dat floor.pix forest15.tab forest16.tab forest8.tab forest.mat forest.pal plant.pix scloud.pix scolumn.pix sfloor.pix shadew15.tab shadew16.tab shadew8.tab ship.dat spcship.pix splant.pix)

foreach(file ${FOREST_FILES})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS forest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
