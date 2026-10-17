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

#
# The rect draw pipeline's shaders. glslang compiles GLSL to SPIR-V, embedspv.pl turns
# the binary into a C array (see drivers/glrend for the same shape of build). Both are
# build tools, so a build without them is a configuration error rather than a silently
# reduced driver.
#
find_program(GLSLANG_EXECUTABLE glslang)
if (NOT GLSLANG_EXECUTABLE)
    message(STATUS "glslang not found, not building vkrend.")
    return()
endif ()

function(compile_shader_spv shader varname stage destination)
    cmake_path(GET shader FILENAME shader_filename)
    set(spv_file "${CMAKE_CURRENT_BINARY_DIR}/${shader_filename}.spv")

    # Extra arguments are additional dependencies, e.g. a shared #include.
    add_custom_command(
            OUTPUT              ${destination}
            WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}
            MAIN_DEPENDENCY     ${shader}
            BYPRODUCTS          "${spv_file}"
            DEPENDS             "${CMAKE_CURRENT_SOURCE_DIR}/embedspv.pl" ${ARGN}

            COMMAND "${GLSLANG_EXECUTABLE}" -V -S "${stage}" -o "${spv_file}" "${shader}"
            COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedspv.pl" "${varname}" "${spv_file}" "${destination}"
    )
endfunction()

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/rect.vert.glsl"
        "vkrend_rect_vert_spv"
        "vert"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.vert.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/rect.frag.glsl"
        "vkrend_rect_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.frag.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/rect.indexed.frag.glsl"
        "vkrend_rect_indexed_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.indexed.frag.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/line.vert.glsl"
        "vkrend_line_vert_spv"
        "vert"
        "${CMAKE_CURRENT_BINARY_DIR}/line.vert.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/line.frag.glsl"
        "vkrend_line_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/line.frag.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/line_int.frag.glsl"
        "vkrend_line_int_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/line_int.frag.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/text.vert.glsl"
        "vkrend_text_vert_spv"
        "vert"
        "${CMAKE_CURRENT_BINARY_DIR}/text.vert.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/text.frag.glsl"
        "vkrend_text_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/text.frag.spv.h"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.vert.glsl"
        "vkrend_render_vert_spv"
        "vert"
        "${CMAKE_CURRENT_BINARY_DIR}/render.vert.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lighting.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.frag.glsl"
        "vkrend_render_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lit.frag.glsl"
        "vkrend_render_lit_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lighting.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.textured.frag.glsl"
        "vkrend_render_textured_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.textured.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lit.textured.frag.glsl"
        "vkrend_render_lit_textured_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.textured.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lighting.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.textured.indexed.frag.glsl"
        "vkrend_render_textured_indexed_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.textured.indexed.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.indexed.glsl"
)

compile_shader_spv(
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lit.textured.indexed.frag.glsl"
        "vkrend_render_lit_textured_indexed_frag_spv"
        "frag"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.textured.indexed.frag.spv.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.lighting.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render.indexed.glsl"
)

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

        devclut.c
        devclut.h

        devpixmp.h
        devpixmp.c
        devpixmpf.c
        devpmsub.c

        rect.c

        "${CMAKE_CURRENT_BINARY_DIR}/rect.vert.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.indexed.frag.spv.h"

        line.c

        "${CMAKE_CURRENT_BINARY_DIR}/line.vert.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/line.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/line_int.frag.spv.h"

        text.c

        "${CMAKE_CURRENT_BINARY_DIR}/text.vert.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/text.frag.spv.h"

        rendfcty.c
        rendfcty.h

        renderer.c
        renderer.h
        state.c
        state.h

        gstored.c
        gstored.h
        gv1model.c
        gv1model.h
        gv1buckt.c
        gv1buckt.h

        sbuffer.c
        sbuffer.h

        "${CMAKE_CURRENT_BINARY_DIR}/render.vert.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.textured.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.textured.indexed.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.textured.frag.spv.h"
        "${CMAKE_CURRENT_BINARY_DIR}/render.lit.textured.indexed.frag.spv.h"

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
