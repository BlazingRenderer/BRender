project(cube)

add_executable(cube
        main.c
)
target_compile_definitions(cube PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(cube BRender::Full)
set_property(TARGET cube PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET cube PROPERTY FOLDER "BRender/Examples")

set(CUBE_FILES cube.dat checkerboard.mat checkerboard.pix)

foreach(file ${CUBE_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
