project(dosio)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

set(INC_FILES
        include/dosio.h
        include/dosio_p.h
        include/dosio_eventq.h
        include/dosio_keyboard.h
        )

set(DOSIO_FILES
        dosio.c
        )

add_library(dosio)

set_target_properties(dosio PROPERTIES
        EXPORT_NAME "DOSIO"
        FOLDER "BRender"
        )

target_sources(dosio
		PRIVATE
		${DOSIO_FILES}
		)

target_link_libraries(dosio PRIVATE SDL2::SDL2 BRender::DDI)
target_link_libraries(dosio PUBLIC SDL2::SDL2 dosio-headers)

add_library(dosio-headers INTERFACE)
target_sources(dosio-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_include_directories(dosio-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/dosio>
        )

set_target_properties(dosio PROPERTIES
        EXPORT_NAME "DOSIO"
        FOLDER "BRender"
        )

set_target_properties(dosio-headers PROPERTIES
        EXPORT_NAME "DOSIO::Headers"
        FOLDER "BRender"
        )

add_library(BRender::DOSIO ALIAS dosio)
add_library(BRender::DOSIO::Headers ALIAS dosio-headers)
