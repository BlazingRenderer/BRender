project(rdemo)

add_executable(rdemo
        rdemo.c
)
target_compile_definitions(rdemo PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(rdemo BRender::Full dosbase)
set_property(TARGET sheet PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET rdemo PROPERTY FOLDER "BRender/Examples/Samples")

set(RDEMO_FILES ast.pix earth.pix moon.pix shade.tab sph32.dat std.pal teapot.dat testenv1.pix)

foreach(file ${RDEMO_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
