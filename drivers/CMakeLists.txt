add_subdirectory(glrend)

 if (TARGET SDL3::SDL3)
     add_subdirectory(sdl3dev)
     target_link_libraries(brender-full INTERFACE sdl3dev)
 else ()
     message("SDL3 not found, not building sdl3dev")
 endif ()

target_link_libraries(brender-full INTERFACE glrend)

if (BRENDER_BUILD_SOFT)
    add_subdirectory(softrend)
    add_subdirectory(pentprim)
    target_link_libraries(brender-full INTERFACE softrend pentprim)
endif ()
