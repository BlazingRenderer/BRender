project(dosio)

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
        include/dos_p.h
        include/dosio.h
        include/eventq.h
        include/keyboard.h
        )

set(DOSIO_FILES
        divtrap.c
        dossetup.c
        eventq.c
        hook.c
        keyboard.c
        readmse.c
        )

add_library(dosio OBJECT
        ${DOSIO_FILES}
        ${INC_FILES}
        ${DDI_FILES}
        )

target_sources(dosio INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(dosio INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ddi
        FILES ${DDI_FILES}
        )

target_compile_definitions(dosio PRIVATE __BR_V1DB__=0)
target_include_directories(dosio PRIVATE .)
target_link_libraries(dosio PRIVATE brender-inc-ddi)
