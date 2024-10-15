project(sdl3dev)

set(INC_FILES
        include/brsdl2dev.h
        include/brsdl2dev_p.h
        )

add_library(sdl3dev-headers INTERFACE)
target_sources(sdl3dev-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )
target_include_directories(sdl3dev-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/sdl3dev>
        )

set_target_properties(sdl3dev-headers PROPERTIES
        EXPORT_NAME "Drivers::SDL3::Headers"
        FOLDER "BRender/Drivers"
        )


add_library(sdl3dev
        drv.h
        drv_ip.h
        driver.c

        template.h

        device.c
        device.h

        outfcty.c
        outfcty.h

        devpixmp.c
        devpixmp.h

        devclut.c
        devclut.h

        surface.c

        glrend.c

        util.c

        ${INC_FILES}
        )

get_target_property(target_type sdl3dev TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(sdl3dev PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_compile_definitions(sdl3dev PRIVATE -DNO_MEMORY_FALLBACK=1)

target_link_libraries(sdl3dev PRIVATE SDL3::SDL3 BRender::DDI glrend-headers)
target_link_libraries(sdl3dev PUBLIC SDL3::SDL3 sdl3dev-headers)

set_target_properties(sdl3dev PROPERTIES
        EXPORT_NAME "Drivers::SDL3"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::SDL3Dev::Headers ALIAS sdl3dev-headers)
add_library(BRender::Drivers::SDL3Dev ALIAS sdl3dev)
