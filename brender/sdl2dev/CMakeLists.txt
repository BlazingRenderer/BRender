project(sdl2dev)

if(NOT BRENDER_DISABLE_FINDSDL)
    find_package(SDL2 REQUIRED)
endif()

add_library(sdl2dev STATIC
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
)

get_target_property(target_type sdl2dev TYPE)
if(target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(sdl2dev PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_compile_definitions(sdl2dev PRIVATE -DNO_MEMORY_FALLBACK=1)

target_link_libraries(sdl2dev fw pixelmap glrend)
target_link_libraries(sdl2dev SDL2::SDL2)

set_property(TARGET sdl2dev PROPERTY FOLDER ${BRENDER_FOLDER})
