project(sheet)

add_executable(sheet
        sheet.c
)
target_link_libraries(sheet PRIVATE brdemo)
set_property(TARGET sheet PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET sheet PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(sheet PRIVATE SDL2::SDL2main)
endif()

set(SHEET_FILES 8ball.pix earth.pix rivrock1.pix shade.tab sheet.mat sph32.dat std.pal wirscrn2.pix)

foreach(file ${SHEET_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS sheet DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
