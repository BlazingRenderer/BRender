project(sdl2dev)

set(INC_FILES
        include/brsdl2dev.h
        include/brsdl2dev_p.h
        )

add_library(sdl2dev-headers INTERFACE)
target_sources(sdl2dev-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )
target_include_directories(sdl2dev-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/sdl2dev>
        )

set_target_properties(sdl2dev-headers PROPERTIES
        EXPORT_NAME "Drivers::SDL2::Headers"
        FOLDER "BRender/Drivers"
        )


add_library(sdl2dev
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

get_target_property(target_type sdl2dev TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(sdl2dev PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_compile_definitions(sdl2dev PRIVATE -DNO_MEMORY_FALLBACK=1)

target_link_libraries(sdl2dev PRIVATE SDL2::SDL2 BRender::DDI glrend-headers)
target_link_libraries(sdl2dev PUBLIC SDL2::SDL2 sdl2dev-headers)

set_target_properties(sdl2dev PROPERTIES
        EXPORT_NAME "Drivers::SDL2"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::SDL2Dev::Headers ALIAS sdl2dev-headers)
add_library(BRender::Drivers::SDL2Dev ALIAS sdl2dev)
