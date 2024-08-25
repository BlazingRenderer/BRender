project(softcube)

add_executable(softcube16 main.c)
target_compile_definitions(softcube16 PUBLIC -DSOFTCUBE_16BIT)
target_link_libraries(softcube16 PRIVATE BRender::Full BRender::DDI)
set_property(TARGET softcube16 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET softcube16 PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(softcube16 PRIVATE SDL2::SDL2main)
endif()

add_executable(softcube24 main.c)
target_link_libraries(softcube24 PRIVATE BRender::Full BRender::DDI)
set_property(TARGET softcube24 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET softcube24 PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(softcube24 PRIVATE SDL2::SDL2main)
endif()

set(SOFTCUBE_FILES cube.dat checkerboard8.mat checkerboard8.pix checkerboard24.mat checkerboard24.pix)

foreach(file ${SOFTCUBE_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
