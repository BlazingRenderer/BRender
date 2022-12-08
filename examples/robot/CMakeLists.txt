project(robot)

add_executable(robot
        app.c
        app.h
        world.h
        robot.c
)

target_compile_definitions(robot PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(robot PUBLIC BRender::Full brsdl)
