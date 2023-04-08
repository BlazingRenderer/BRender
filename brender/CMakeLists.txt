set(BRENDER_FOLDER "Brender")

add_library(brender INTERFACE)

add_subdirectory(inc)
add_subdirectory(host)
add_subdirectory(fw)
add_subdirectory(fmt)
add_subdirectory(math)
add_subdirectory(pixelmap)
add_subdirectory(std)
add_subdirectory(v1db)

target_link_libraries(brender INTERFACE v1db)

if(BRENDER_BUILD_DRIVERS)
    add_subdirectory(nulldev)
    add_subdirectory(glrend)

    if(TARGET SDL2::SDL2)
        add_subdirectory(sdl2dev)
        target_link_libraries(brender INTERFACE sdl2dev)
    else()
        message("SDL2 not found, not building sdl2dev")
    endif()

    target_link_libraries(brender INTERFACE glrend)

    if(WIN32)
        add_subdirectory(ddraw)
        target_link_libraries(brender INTERFACE ddraw)
    endif()

endif()

##
# Full BRender, with drivers.
##
add_library(BRender::Full ALIAS brender)

##
# Core BRender, no drivers.
##
add_library(BRender::Core ALIAS v1db)
