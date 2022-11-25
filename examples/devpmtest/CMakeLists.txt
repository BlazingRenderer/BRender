project(devpmtest)

add_executable(devpmtest
        main.c
)
target_link_libraries(devpmtest BRender::Full)

