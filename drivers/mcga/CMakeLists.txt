project(mcga C)

add_library(mcga
        bios.c
        devclut.c
        devclut.h
        device.c
        device.h
        devpixmp.c
        devpixmp.h
        driver.c
        drv.h
        drv_ip.h
        object.c
        object.h
        outfcty.c
        outfcty.h
        template.h
)

get_target_property(target_type mcga TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(mcga PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_include_directories(mcga PRIVATE ddi ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(mcga PRIVATE BRender::DDI)

set_target_properties(mcga PROPERTIES
        EXPORT_NAME "Drivers::MCGA"
        FOLDER "BRender/Drivers"
)

add_library(BRender::Drivers::MCGA ALIAS mcga)
