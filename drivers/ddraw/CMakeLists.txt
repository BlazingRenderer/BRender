project(ddraw)

add_library(ddraw
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
if(target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(ddraw PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_link_libraries(ddraw PRIVATE BRender::DDI dxguid)

set_target_properties(ddraw PROPERTIES
        EXPORT_NAME "Drivers::DDRAW"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::DDRAW ALIAS ddraw)
