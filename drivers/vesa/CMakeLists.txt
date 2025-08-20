project(vesa C ASM_MASM)

set(ASM_FILES
        vesabank.asm
        vesaloop.asm
        )

set(INC_FILES
        include/brvesadev.h
        include/brvesadev_p.h
        )

add_library(vesa-headers INTERFACE)
target_sources(vesa-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )
target_include_directories(vesa-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/vesa>
        )

set_target_properties(vesa-headers PROPERTIES
        EXPORT_NAME "Drivers::VESA::Headers"
        FOLDER "BRender/Drivers"
        )


add_library(vesa
        bios.c

        devclut.c

        devclut.h
        device.c
        device.h

        devpixmp.c
        devpixmp.h
        devpml.c
        devpmw.c
        devpmwb.c

        driver.c

        drv.h
        drv_ip.h

        object.c
        object.h

        ocfree.c

        outfcty.c
        outfcty.h

        ports.asm
        ports.h

        vesaapi.c
        vesaapi.h

        vesaloop.h

        ${ASM_FILES}
        ${INC_FILES}
        )

get_target_property(target_type vesa TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(vesa PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_link_libraries(vesa PRIVATE BRender::DDI glrend-headers)
target_link_libraries(vesa PUBLIC vesa-headers)

set_target_properties(vesa PROPERTIES
        EXPORT_NAME "Drivers::VESA"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::VESA::Headers ALIAS vesa-headers)
add_library(BRender::Drivers::VESA ALIAS vesa)
