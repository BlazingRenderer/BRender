project(host)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )
target_include_directories(brender-inc-ddi INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ddi>
        $<INSTALL_INTERFACE:ddi>
        )

set(DDI_FILES
        ddi/host.h
        ddi/host_p.h
        )
source_group("DDI" FILES ${DDI_FILES})

set(INC_FILES
        )

set(HOST_FILES
        himage.c
        hostcfg.c
        hstsetup.c
        host_ip.h
        )

add_library(host OBJECT
        ${HOST_FILES}
        ${INC_FILES}
        ${DDI_FILES}
        )

target_sources(host INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(host INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR} ddi
        FILES ${DDI_FILES}
        )

target_include_directories(host PRIVATE .)
target_link_libraries(host PRIVATE brender-inc-ddi)

if(NOT devkitA64)
    target_link_libraries(host PRIVATE ${CMAKE_DL_LIBS})
endif()
