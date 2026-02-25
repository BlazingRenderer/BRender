project(vkrend)

set(GLAD_FILES
        glad/src/vulkan.c
        glad/include/glad/vulkan.h
        glad/include/vk_platform.h
)
source_group("GLAD" FILES ${GLAD_FILES})

set(INC_FILES
        include/brvkrend.h
        include/brvkrend_p.h
)

add_library(vkrend-headers INTERFACE)
target_sources(vkrend-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
)

target_include_directories(vkrend-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/vkrend>
)

set_target_properties(vkrend-headers PROPERTIES
        EXPORT_NAME "Drivers::VKRend::Headers"
        FOLDER "BRender/Drivers"
)
add_library(BRender::Drivers::VKRend::Headers ALIAS vkrend-headers)

if (NOT BRENDER_BUILD_VKREND)
    return()
endif ()

add_library(vkrend
        ${GLAD_FILES}

        driver.c
        drv.h
        drv_ip.h

        template.h

        formats.c

        device.c
        device.h

        outfcty.c
        outfcty.h

        devpixmp.h
        devpixmp.c
        devpixmpf.c

        rendfcty.c
        rendfcty.h

        select.c

        uuid.c
        uuid.h

        util.c

        ext_procs.c

        vk_mem_alloc.cpp
        vk_mem_alloc.h
)

get_target_property(target_type vkrend TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(vkrend PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_include_directories(vkrend PRIVATE ${CMAKE_CURRENT_BINARY_DIR} glad/include)

target_link_libraries(vkrend PRIVATE BRender::DDI)
target_link_libraries(vkrend PUBLIC vkrend-headers)

set_target_properties(vkrend PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE
        EXPORT_NAME "Drivers::VKRend"
        FOLDER "BRender/Drivers"
)

add_library(BRender::Drivers::VKRend ALIAS vkrend)
