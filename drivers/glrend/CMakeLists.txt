project(glrend)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# NB: Don't use this, it's fragile as fuck.
# find_package(glslang CONFIG REQUIRED)
# set(GLSLANG_EXECUTABLE "$<TARGET_FILE:glslang::glslang-standalone>")

find_program(GLSLANG_EXECUTABLE glslang REQUIRED)
message(STATUS "Found glslang: ${GLSLANG_EXECUTABLE}")

find_program(SPIRV_CROSS_EXECUTABLE spirv-cross REQUIRED)
message(STATUS "Found spirv-cross: ${SPIRV_CROSS_EXECUTABLE}")

function(compile_shader)
    set(shaderpp    "${CMAKE_CURRENT_SOURCE_DIR}/shaderpp.pl")
    set(embedshader "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl")

    cmake_parse_arguments(PARSE_ARGV 0 arg "" "SOURCE;DESTINATION;VARNAME;STAGE" "DEPENDS")

    cmake_path(GET arg_SOURCE FILENAME source_filename)

    set(processed_file "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}.processed.glsl")
    set(spv_file       "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}.spv")
    set(optimised_file "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}.optimised.glsl")

    add_custom_command(
            OUTPUT              ${arg_DESTINATION}
            WORKING_DIRECTORY   ${CMAKE_CURRENT_SOURCE_DIR}
            MAIN_DEPENDENCY     ${arg_SOURCE}
            BYPRODUCTS          "${processed_file}"
                                "${spv_file}"
                                "${optimised_file}"
            DEPENDS             "${shaderpp}"
                                "${embedshader}"
                                ${arg_DEPENDS}

            COMMAND "${PERL_EXECUTABLE}" "${shaderpp}" "${arg_SOURCE}" "${processed_file}"
            COMMAND "${GLSLANG_EXECUTABLE}" -G100 -S "${arg_STAGE}" -o "${spv_file}" "${processed_file}"
            COMMAND "${SPIRV_CROSS_EXECUTABLE}" "${spv_file}" --version 300 --es --output "${optimised_file}"
            COMMAND "${PERL_EXECUTABLE}" "${embedshader}" "${arg_VARNAME}" "${optimised_file}" "${arg_DESTINATION}"
    )
endfunction()

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/brender.vert.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/brender.vert.glsl.h"
        STAGE       "vert"
        VARNAME     "g_DefaultVertexShader"
        DEPENDS     "${CMAKE_CURRENT_SOURCE_DIR}/brender.common.glsl"
                    "${CMAKE_CURRENT_SOURCE_DIR}/common.glsl"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/brender.frag.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/brender.frag.glsl.h"
        STAGE       "frag"
        VARNAME     "g_DefaultFragmentShader"
        DEPENDS     "${CMAKE_CURRENT_SOURCE_DIR}/brender.common.glsl"
        "${CMAKE_CURRENT_SOURCE_DIR}/common.glsl"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/rect.vert.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/rect.vert.glsl.h"
        STAGE       "vert"
        VARNAME     "g_RectVertexShader"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/rect.frag.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/rect.frag.glsl.h"
        STAGE       "frag"
        VARNAME     "g_RectFragmentShader"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/text.vert.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/text.vert.glsl.h"
        STAGE       "vert"
        VARNAME     "g_TextVertexShader"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/text.frag.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/text.frag.glsl.h"
        STAGE       "frag"
        VARNAME     "g_TextFragmentShader"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/line.vert.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/line.vert.glsl.h"
        STAGE       "vert"
        VARNAME     "g_LineVertexShader"
)

compile_shader(
        SOURCE      "${CMAKE_CURRENT_SOURCE_DIR}/line.frag.glsl"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/line.frag.glsl.h"
        STAGE       "frag"
        VARNAME     "g_LineFragmentShader"
)

set(GLSTATE_FILES
        cache.c
        state.c
        state.h
        state_clip.c
        state_cull.c
        state_hidden.c
        state_light.c
        state_matrix.c
        state_output.c
        state_primitive.c
        state_surface.c
        )
source_group("STATE" FILES ${GLSTATE_FILES})

set(VIDEO_FILES
        shader_brender.c
        shader_rect.c
        shader_text.c
        shader_line.c
        video.c
        video.h
        )
source_group("VIDEO" FILES ${VIDEO_FILES})

set(RENDERER_FILES
        gv1buckt.c
        gv1buckt.h

        gv1model.c
        gv1model.h

        gstored.c
        gstored.h
        v1model.c

        onscreen.c

        renderer.c
        renderer.h

        sbuffer.h
        sbuffer.c

        sstate.c
        sstate.h
        )
source_group("Renderer" FILES ${RENDERER_FILES})

set(GLREND_FILES
        drv.h
        drv_ip.h

        driver.c

        outfcty.c
        outfcty.h

        ext_procs.c

        rendfcty.c
        rendfcty.h

        device.c
        device.h

        devpixmp.c
        devpixmp.h
        devpmglf.c
        devpmsub.c

        devclut.c
        devclut.h

        font.c
        font.h

        template.h

        util.c

        glad.c
        )

set(INC_FILES
        include/brglrend.h
        include/brglrend_p.h

        include/glad/glad.h
        include/KHR/khrplatform.h
        )

add_library(glrend-headers INTERFACE)
target_sources(glrend-headers INTERFACE
        FILE_SET include
        TYPE HEADERS
        BASE_DIRS include
        FILES ${INC_FILES}
        )

target_include_directories(glrend-headers INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/brender/glrend>
        )

set_target_properties(glrend-headers PROPERTIES
        EXPORT_NAME "Drivers::GLRend::Headers"
        FOLDER "BRender/Drivers"
        )

add_library(glrend
        ${GLREND_FILES}
        ${RENDERER_FILES}
        ${VIDEO_FILES}
        ${GLSTATE_FILES}
        ${INC_FILES}

        buffer_ring.c

        "${CMAKE_CURRENT_BINARY_DIR}/brender.vert.glsl.h"
        "${CMAKE_CURRENT_BINARY_DIR}/brender.frag.glsl.h"

        "${CMAKE_CURRENT_BINARY_DIR}/rect.vert.glsl.h"
        "${CMAKE_CURRENT_BINARY_DIR}/rect.frag.glsl.h"

        "${CMAKE_CURRENT_BINARY_DIR}/text.vert.glsl.h"
        "${CMAKE_CURRENT_BINARY_DIR}/text.frag.glsl.h"

        "${CMAKE_CURRENT_BINARY_DIR}/line.vert.glsl.h"
        "${CMAKE_CURRENT_BINARY_DIR}/line.frag.glsl.h"
        )

get_target_property(target_type glrend TYPE)
if (target_type STREQUAL SHARED_LIBRARY)
    target_compile_definitions(glrend PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif ()

target_include_directories(glrend PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(glrend PRIVATE OpenGL::GL BRender::DDI)
target_link_libraries(glrend PUBLIC glrend-headers)

set_target_properties(glrend PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED TRUE
        EXPORT_NAME "Drivers::GLRend"
        FOLDER "BRender/Drivers"
        )

add_library(BRender::Drivers::GLRend::Headers ALIAS glrend-headers)
add_library(BRender::Drivers::GLRend ALIAS glrend)
