project(nulldev)

add_library(nulldev OBJECT
        drv.h
        drv_ip.h

        template.h

        device.c
        device.h

        glighting.c
        glighting.h

        gprim.c
        gprim.h
        )

target_link_libraries(nulldev PRIVATE brender-inc-ddi)
target_compile_definitions(nulldev PRIVATE __BR_V1DB__=0)
