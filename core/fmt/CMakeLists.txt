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
        loadgltf.c
        animgltf.c

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
#
# savegltf.c converts colours on the way out, and under FLT_EVAL_METHOD 2 the
# 80-bit intermediates of that conversion reach the file: what mkres writes
# would then depend on which floating-point evaluation class built the writer.
# -ffloat-store rounds every float and double store to its declared type, so
# the file no longer depends on the class.
#
# Only the generator. The rasteriser has to keep its excess precision - the x87
# half of the rendertest reference is pentprim's output and depends on it.
#
include(CheckCCompilerFlag)
check_c_compiler_flag("-ffloat-store" BRENDER_HAVE_FFLOAT_STORE)
if(BRENDER_HAVE_FFLOAT_STORE)
    set_source_files_properties(savegltf.c PROPERTIES COMPILE_OPTIONS "-ffloat-store")
endif()
target_link_libraries(fmt PRIVATE brender-inc-ddi)
