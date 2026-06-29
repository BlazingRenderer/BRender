project(glrend1x)

set(INC_FILES
        include/brglrend1x.h
        include/brglrend1x_p.h
)

add_library(glrend1x-headers INTERFACE)
target_sources(glrend1x-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
)

target_include_directories(glrend1x-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/glrend1x-headers>
)

set_target_properties(glrend1x-headers PROPERTIES
        EXPORT_NAME "Drivers::GLRend1x::Headers"
        FOLDER "BRender/Drivers"
)

add_library(BRender::Drivers::GLRend1x::Headers ALIAS glrend1x-headers)

set(GLAD_FILES
        glad/src/gl.c
        glad/include/glad/gl.h
        glad/include/KHR/khrplatform.h
)
source_group("GLAD" FILES ${GLAD_FILES})

set(GLREND_FILES
        drv.h
        drv_ip.h

        driver.c

        outfcty.c
        outfcty.h

        ext_procs.c

        device.c
        device.h

        devpixmp.c
        devpixmp.h
        devpmglf.c
        devpmsub.c

        devclut.c
        devclut.h

        template.h

        util.c
        formats.c

        sbuffer.h
        sbuffer.c
        )

add_library(glrend1x
        ${GLAD_FILES}
        ${GLREND_FILES}

        plib.c
        plib.h
        pstate.c
        pstate.h
        rendfunc.c
        )

get_target_property(target_type glrend1x TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(glrend1x PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_include_directories(glrend1x PRIVATE glad/include)

target_link_libraries(glrend1x PRIVATE BRender::DDI glrend-headers)
target_link_libraries(glrend1x PUBLIC glrend1x-headers)

# Hacky, but prevents the linker getting confused with glrend's
target_compile_definitions(glrend1x PRIVATE
        gladLoadGLContextUserPtr=gladLoadGL1xContextUserPtr
        gladLoadGLContext=gladLoadGL1xContext
)

set_target_properties(glrend1x PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE
        EXPORT_NAME "Drivers::GLRend1x"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::GLRend1x ALIAS glrend1x)
