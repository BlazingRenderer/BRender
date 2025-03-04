project(softrend C ASM_MASM)

include(h2inc)

make_h2inc(softrend "${CMAKE_CURRENT_SOURCE_DIR}/drv.h" "${CMAKE_CURRENT_BINARY_DIR}/drv.inc")

set(ASM_FILES

)
set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_OPTIONS  "-q;-safeseh;-Zne;-6;-Zp4;-elf;-zcw")

add_library(softrend
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

        subdiv.c

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

        "${CMAKE_CURRENT_BINARY_DIR}/drv.inc"

        ${ASM_FILES}
)

get_target_property(target_type softrend TYPE)
if(target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(softrend PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_include_directories(softrend PRIVATE ddi ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(softrend PRIVATE -D__386__)

target_link_libraries(softrend PRIVATE BRender::DDI)

set_target_properties(softrend PROPERTIES
        EXPORT_NAME "Drivers::SoftRend"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::SoftRend ALIAS softrend)
