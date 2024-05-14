project(softrend C ASM_MASM)

include(h2inc)

add_library(softrend)
get_target_property(target_type softrend TYPE)

if(target_type STREQUAL "STATIC_LIBRARY")
	set(BR_SOFTREND_BUILD_STATIC 1)
	set(BR_SOFTREND_BUILD_SHARED 0)
else()
	set(BR_SOFTREND_BUILD_STATIC 0)
	set(BR_SOFTREND_BUILD_SHARED 1)
endif()

configure_file(config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

target_compile_definitions(softrend PRIVATE "-DBR_SOFTREND_EXPORTS=1")

make_h2inc(softrend "${CMAKE_CURRENT_SOURCE_DIR}/drv.h" "${CMAKE_CURRENT_BINARY_DIR}/drv.inc")

set(ASM_FILES
        subdiv.asm
)
set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_OPTIONS "-safeseh")

target_sources(softrend PRIVATE
        alpha.c
        clip.c
        convert.c
        depth.c
        device.c
        driver.c
        faceops.c
        ffront.c
        genrend.c
        gprim.c
        gv1buckt.c
        gv1model.c
        heap.c
        light24.c
        light8.c
        lighting.c
        mapping.c
        object.c
        ocfree.c
        onscreen.c
        renderer.c
        rendfcty.c
        rndstate.c
        rprim.c
        rv1point.c
        setup.c
        sstate.c
        state.c
        v1model.c

        device.h
        drv.h
        drv_ip.h
        gprim.h
        gv1buckt.h
        gv1model.h
        lighting.h
        lightmac.h
        object.h
        ddi/priminfo.h
        renderer.h
        rendfcty.h
        rend.h
        sstate.h
        state.h
        template.h
        timestmp.h
        zsrmacro.h

        config.h.in

        "${CMAKE_CURRENT_BINARY_DIR}/config.h"
        "${CMAKE_CURRENT_BINARY_DIR}/drv.inc"

        ${ASM_FILES}
)

target_include_directories(softrend PRIVATE ddi)
target_compile_definitions(softrend PRIVATE -D__386__)

target_link_libraries(softrend PRIVATE BRender::DDI)
target_include_directories(softrend PRIVATE ${CMAKE_CURRENT_BINARY_DIR})


set_target_properties(softrend PROPERTIES
        EXPORT_NAME "Drivers::SoftRend"
        FOLDER "BRender/Drivers"
        OUTPUT_NAME "softrndf"
        )

if (target_type STREQUAL SHARED_LIBRARY)
    set_target_properties(softrend PROPERTIES
            PREFIX ""
            SUFFIX ${BRENDER_DRIVER_SUFFIX}
    )
endif ()

add_library(BRender::Drivers::SoftRend ALIAS softrend)
