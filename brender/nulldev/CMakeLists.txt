project(nulldev)


add_library(nulldev STATIC
        drv.h
        drv_ip.h
        driver.c

        template.h

        device.c
        device.h

        glighting.c
        glighting.h

        gprim.c
        gprim.h
)

get_target_property(target_type nulldev TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(nulldev PRIVATE -DBrDrv1Begin=BrDrvNullBegin)
endif()

target_link_libraries(nulldev fw pixelmap)
set_property(TARGET nulldev PROPERTY FOLDER ${BRENDER_FOLDER})

