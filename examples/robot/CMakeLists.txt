project(robot)

add_executable(robot WIN32
        app.c
        app.h
        world.h
        robot.c
)

target_link_libraries(robot PUBLIC brender)
