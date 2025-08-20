project(cube)

add_executable(cube main.c)
target_link_libraries(cube PRIVATE BRender::Full BRender::DDI)
set_property(TARGET cube PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET cube PROPERTY FOLDER "BRender/Examples")

set(CUBE_FILES cube.dat checkerboard8.mat checkerboard8.pix)

foreach(file ${CUBE_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
