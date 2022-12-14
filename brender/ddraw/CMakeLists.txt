project(ddraw)

add_library(ddraw STATIC
        devclut.c
        devclut.h
        device.c
        device.h
        devpixmp.c
        devpixmp.h
        devpmddo.c
        driver.c
        drv.h
        drv_ip.h
        dx_error.c
        dx_error.h
        object.c
        object.h
        ocfree.c
        outfcty.c
        outfcty.h
        template.h
)

get_target_property(target_type ddraw TYPE)
if(target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(ddraw PRIVATE -DBrDrv1Begin=BrDrv1DirectDrawBegin)
endif()

target_link_libraries(ddraw v1db)
set_property(TARGET ddraw PROPERTY FOLDER ${BRENDER_FOLDER})
