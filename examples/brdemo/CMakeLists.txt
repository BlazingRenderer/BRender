project(brdemo)

add_library(brdemo STATIC
        brdemo.h
        brdemo.c

        parg.c
        parg.h
)

target_link_libraries(brdemo PUBLIC BRender::Full)
# The vkrend and glrend headers exist whether or not the drivers were built -
# a missing shader compiler skips the driver, not its header - so they are
# linked unconditionally, as sdl3dev does. The vkrend *function* the begin hook
# calls is only there when the driver is.
target_link_libraries(brdemo PRIVATE glrend-headers vkrend-headers)
target_include_directories(brdemo PUBLIC .)
if(BRENDER_HAVE_VK)
    target_compile_definitions(brdemo PRIVATE BRDEMO_HAVE_VKREND=1)
endif()
set_property(TARGET brdemo PROPERTY FOLDER "BRender/Examples")
