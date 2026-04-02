##
# We always want glrend-headers so unconditionally include glrend.
##
add_subdirectory(glrend)
if (TARGET glrend)
    target_link_libraries(brender-full INTERFACE glrend)
else ()
    target_link_libraries(brender-full INTERFACE glrend-headers)
endif ()

if (TARGET SDL2::SDL2)
    add_subdirectory(sdl2dev)
    target_link_libraries(brender-full INTERFACE sdl2dev)
else ()
    message(STATUS "SDL2 not found, not building sdl2dev")
endif ()

 if (TARGET SDL3::SDL3)
     add_subdirectory(sdl3dev)
     target_link_libraries(brender-full INTERFACE sdl3dev)
 else ()
     message(STATUS "SDL3 not found, not building sdl3dev")
 endif ()

add_subdirectory(softrend)
target_link_libraries(brender-full INTERFACE softrend)

if (BRENDER_BUILD_SOFT)
    add_subdirectory(pentprim)
    target_link_libraries(brender-full INTERFACE pentprim)
endif ()

if (CMAKE_SYSTEM_NAME STREQUAL "DOS")
    add_subdirectory(mcga)
    target_link_libraries(brender-full INTERFACE mcga)

    add_subdirectory(vesa)
    target_link_libraries(brender-full INTERFACE vesa)
endif()
