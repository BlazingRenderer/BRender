project(v1compat)

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

set(V1COMPAT_FILES
        zb.c
        zs.c
        )

add_library(v1compat OBJECT ${V1COMPAT_FILES})

target_sources(v1compat INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(v1compat INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ddi
        FILES ${DDI_FILES}
        )

target_include_directories(v1compat PRIVATE .)

target_link_libraries(v1compat PRIVATE brender-inc-ddi)
