project(robot)

add_executable(robot
        main.c
)

target_link_libraries(robot PUBLIC brdemo)
set_property(TARGET robot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET robot PROPERTY FOLDER "BRender/Examples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(robot PRIVATE SDL2::SDL2main)
endif()

set(ROBOT_FILES mask.pix robo.act robo.dat robo.msc robo.vue tile01_de.pix tile01.pix os2shade.tab os2pal.pal)

foreach(file ${ROBOT_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS robot DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
