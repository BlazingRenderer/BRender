project(vesa C)

add_library(vesa
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
        outfcty.c
        outfcty.h
        vesaapi.c
        vesaapi.h

        template.h
)

get_target_property(target_type vesa TYPE)
if(vesa STREQUAL SHARED_LIBRARY)
    target_compile_definitions(vesa PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_include_directories(vesa PRIVATE ddi ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(vesa PRIVATE -D__386__)

target_link_libraries(vesa PRIVATE BRender::DDI)

set_target_properties(vesa PROPERTIES
        EXPORT_NAME "Drivers::VESA"
        FOLDER "BRender/Drivers"
)

add_library(BRender::Drivers::VESA ALIAS vesa)

