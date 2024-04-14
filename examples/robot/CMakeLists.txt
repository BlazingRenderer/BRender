project(robot)

add_executable(robot
        main.c
)

target_compile_definitions(robot PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(robot PUBLIC BRender::Full)
set_property(TARGET robot PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat")
set_property(TARGET robot PROPERTY FOLDER "BRender/Examples")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/mask.pix ${CMAKE_CURRENT_BINARY_DIR}/mask.pix COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/robo.act ${CMAKE_CURRENT_BINARY_DIR}/robo.act COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/robo.dat ${CMAKE_CURRENT_BINARY_DIR}/robo.dat COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/robo.msc ${CMAKE_CURRENT_BINARY_DIR}/robo.msc COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/robo.vue ${CMAKE_CURRENT_BINARY_DIR}/robo.vue COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/tile01_de.pix ${CMAKE_CURRENT_BINARY_DIR}/tile01_de.pix COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/tile01.pix ${CMAKE_CURRENT_BINARY_DIR}/tile01.pix COPYONLY)
