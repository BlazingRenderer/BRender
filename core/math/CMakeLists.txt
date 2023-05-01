project(math)

target_include_directories(brender-inc INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_include_directories(brender-inc-ddi INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/ddi>
        $<INSTALL_INTERFACE:ddi>
        )

set(DDI_FILES
        ddi/math_ip.h
        ddi/vecifns.h
        )

set(INC_FILES
        include/brmath.h
        )

set(MATH_FILES
        angles.c
        fixed.c
        matrix4.c
        matrix23.c
        matrix34.c
        plane.c
        quat.c
        scalar.c
        transfrm.c
        vector.c
        )

add_library(math OBJECT ${MATH_FILES})

target_sources(math INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_sources(math INTERFACE
        FILE_SET ddi
        TYPE HEADERS
        BASE_DIRS ddi
        FILES ${DDI_FILES}
        )

target_compile_definitions(math PRIVATE __BR_V1DB__=0)
target_link_libraries(math PRIVATE brender-inc-ddi)

find_library(math_library m)
if (math_library)
    target_link_libraries(math PUBLIC ${math_library})
endif ()
