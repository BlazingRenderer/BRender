project(rdemo)

add_executable(rdemo
        rdemo.c
)
target_link_libraries(rdemo PRIVATE brdemo)
set_property(TARGET sheet PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET rdemo PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(rdemo PRIVATE SDL2::SDL2main)
endif()

set(RDEMO_FILES ast.pix earth.pix moon.pix shade.tab sph32.dat std.pal teapot.dat testenv1.pix)

foreach(file ${RDEMO_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
