project(robot)

add_executable(robot
        main.c
)

target_compile_definitions(robot PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(robot PUBLIC BRender::Full)
set_property(TARGET robot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET robot PROPERTY FOLDER "BRender/Examples")

set(ROBOT_FILES mask.pix robo.act robo.dat robo.msc robo.vue tile01_de.pix tile01.pix)

foreach(file ${ROBOT_FILES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
