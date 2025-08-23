project(doscube)

add_executable(doscube main.c)
target_link_libraries(doscube PRIVATE BRender::Full BRender::DDI)
set_property(TARGET doscube PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET doscube PROPERTY FOLDER "BRender/Examples")

set(DOSCUBE_FILES cube.dat checkerboard8.mat checkerboard8.pix std.pal shade.tab)

foreach (file ${DOSCUBE_FILES})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach ()
