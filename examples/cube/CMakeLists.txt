project(cube)

add_executable(cube
        main.c
)
target_link_libraries(cube PRIVATE BRender::Full)
set_property(TARGET cube PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET cube PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(cube PRIVATE SDL2::SDL2main)
endif()

set(CUBE_FILES cube.dat checkerboard.mat checkerboard.pix)

foreach(file ${CUBE_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS cube DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
