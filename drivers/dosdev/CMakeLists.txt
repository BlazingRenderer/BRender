project(dosdev)

set(INC_FILES
        include/brdosdev.h
        include/brdosdev_p.h
        )

add_library(dosdev-headers INTERFACE)
target_sources(dosdev-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )
target_include_directories(dosdev-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/dosdev>
        )

set_target_properties(dosdev-headers PROPERTIES
        EXPORT_NAME "Drivers::DOS::Headers"
        FOLDER "BRender/Drivers"
        )


add_library(dosdev
        drv.h
        drv_ip.h
        driver.c

        device.c
        device.h

        outfcty.c
        outfcty.h

        devpixmp.c
        devpixmp.h

        devclut.h

        ${INC_FILES}
        )

get_target_property(target_type dosdev TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(dosdev PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_link_libraries(dosdev PRIVATE BRender::DDI glrend-headers)
target_link_libraries(dosdev PUBLIC dosdev-headers)

set_target_properties(dosdev PROPERTIES
        EXPORT_NAME "Drivers::DOS"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::DOSDev::Headers ALIAS dosdev-headers)
add_library(BRender::Drivers::DOSDev ALIAS dosdev)
