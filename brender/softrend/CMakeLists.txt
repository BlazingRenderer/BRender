project(softrend C ASM_MASM)

include(h2inc)

make_h2inc(softrend "${CMAKE_CURRENT_SOURCE_DIR}/drv.h" "${CMAKE_CURRENT_BINARY_DIR}/drv.inc")

set(ASM_FILES
        cull.asm
        gen_386.asm
        geometry.asm
        v1m_386.asm
        face_386.asm
        subdiv.asm
)
set_source_files_properties(${ASM_FILES} PROPERTIES COMPILE_OPTIONS "-safeseh")

add_library(softrend STATIC
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
        face_386.h
        gen_386.h
        gprim.h
        gv1buckt.h
        gv1model.h
        lighting.h
        lightmac.h
        object.h
        priminfo.h
        renderer.h
        rendfcty.h
        rend.h
        sstate.h
        state.h
        template.h
        timestmp.h
        v1m_386.h
        zsrmacro.h

        "${CMAKE_CURRENT_BINARY_DIR}/drv.inc"

        ${ASM_FILES}
)

get_target_property(target_type softrend TYPE)
if(NOT target_type STREQUAL STATIC_LIBRARY)
    target_compile_definitions(softrend PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_compile_definitions(softrend PRIVATE -D__386__)

target_link_libraries(softrend PRIVATE v1db)
set_property(TARGET softrend PROPERTY FOLDER ${BRENDER_FOLDER})
