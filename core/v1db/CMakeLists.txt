project(v1db)

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

set(INC_FILES
        )

set(V1DB_FILES
        actsupt.c
        custsupt.c
        dbquery.c
        dbsetup.c
        def_mat.c
        def_mdl.c
        def_otab.c
        enables.c
        ffhooks.c
        matsupt.c
        modrend.c
        modsupt.c
        otable.c
        pick.c
        prelight.c
        prepmap.c
        prepmatl.c
        prepmesh.c
        preptab.c
        regsupt.c
        render.c
        v1dbfile.c

        v1db.h
        v1db_ip.h
        zsmacro.h
        )

add_library(v1db OBJECT ${V1DB_FILES})

target_sources(v1db INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(v1db INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ddi
        FILES ${DDI_FILES}
        )

target_include_directories(v1db PRIVATE .)

target_link_libraries(v1db PRIVATE brender-inc-ddi)
