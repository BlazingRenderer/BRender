project(fmt)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_include_directories(brender-inc-ddi INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ddi>
        $<INSTALL_INTERFACE:ddi>
        )

set(DDI_FILES
        )
source_group("DDI" FILES ${DDI_FILES})

set(INC_FILES
        include/fmt.h
        include/fmt_p.h
        )

set(FMT_FILES
        brstb.c
        brstb.h

        load3ds.c
        load3ds.h
        loadasc.c
        loadbmp.c
        loadgif.c
        loadiff.c
        loadnff.c
        loadpcx.c
        loadscr.c
        loadstb.c
        loadtga.c
        loadvue.c
        savescr.c
        savestb.c
        savegltf.c

        stb_image.h
        stb_image_write.h
        dr_pcx.h

        cbase64.h
        cbase64.c

        cgltf.h
        cgltf_write.h
        cgltf_write_brender.h
        cgltf_brender.h
        cgltf_brender_impl.h
        cgltf.c
        )

add_library(fmt OBJECT
        ${FMT_FILES}
        ${INC_FILES}
        ${DDI_FILES}
        )

target_sources(fmt INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(fmt INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ddi
        FILES ${DDI_FILES}
        )

target_include_directories(fmt PRIVATE .)
target_link_libraries(fmt PRIVATE brender-inc-ddi)
