project(pixelmap)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_include_directories(brender-inc-ddi INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ddi>
        $<INSTALL_INTERFACE:ddi>
        )

set(DDI_FILES
        ddi/pm.h
        ddi/pm_ip.h
        ddi/memloops.h
        )

set(INC_FILES
        )

set(PIXELMAP_FILES
        fontptrs.c
        genclip.c
        gencopy.c
        mipsupt.c
        pixelmap.c
        pmclone.c
        pmdsptch.c
        pmfail.c
        pmfile.c
        pmgen.c
        pmmem.c
        pmmem.h
        pmnull.c
        pmsetup.c
        pmutil.c
        quantize.c
        scale.c
        cmemloops.c

        fnt3x5.c
        fntp4x6.c
        fntp7x9.c
        )

add_library(pixelmap OBJECT ${PIXELMAP_FILES})

target_sources(pixelmap INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(pixelmap INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ddi
        FILES ${DDI_FILES}
        )

target_include_directories(pixelmap PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(pixelmap PRIVATE __BR_V1DB__=0)
target_link_libraries(pixelmap PRIVATE brender-inc-ddi)
