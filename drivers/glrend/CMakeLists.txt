project(glrend)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

add_custom_command(
        OUTPUT brender.vert.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_DefaultVertexShader" "${CMAKE_CURRENT_SOURCE_DIR}/brender.vert.glsl" "${CMAKE_CURRENT_BINARY_DIR}/brender.vert.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/brender.vert.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "${CMAKE_CURRENT_SOURCE_DIR}/brender.common.glsl" "${CMAKE_CURRENT_SOURCE_DIR}/common.glsl"
)

add_custom_command(
        OUTPUT brender.frag.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_DefaultFragmentShader" "${CMAKE_CURRENT_SOURCE_DIR}/brender.frag.glsl" "${CMAKE_CURRENT_BINARY_DIR}/brender.frag.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/brender.frag.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "${CMAKE_CURRENT_SOURCE_DIR}/brender.common.glsl" "${CMAKE_CURRENT_SOURCE_DIR}/common.glsl"
)

add_custom_command(
        OUTPUT rect.vert.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_RectVertexShader" "${CMAKE_CURRENT_SOURCE_DIR}/rect.vert.glsl" "${CMAKE_CURRENT_BINARY_DIR}/rect.vert.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/rect.vert.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
        OUTPUT rect.frag.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_RectFragmentShader" "${CMAKE_CURRENT_SOURCE_DIR}/rect.frag.glsl" "${CMAKE_CURRENT_BINARY_DIR}/rect.frag.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/rect.frag.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
        OUTPUT text.vert.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_TextVertexShader" "${CMAKE_CURRENT_SOURCE_DIR}/text.vert.glsl" "${CMAKE_CURRENT_BINARY_DIR}/text.vert.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/text.vert.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
        OUTPUT text.frag.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_TextFragmentShader" "${CMAKE_CURRENT_SOURCE_DIR}/text.frag.glsl" "${CMAKE_CURRENT_BINARY_DIR}/text.frag.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/text.frag.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
        OUTPUT line.vert.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_LineVertexShader" "${CMAKE_CURRENT_SOURCE_DIR}/line.vert.glsl" "${CMAKE_CURRENT_BINARY_DIR}/line.vert.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/line.vert.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
        OUTPUT line.frag.glsl.h
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_LineFragmentShader" "${CMAKE_CURRENT_SOURCE_DIR}/line.frag.glsl" "${CMAKE_CURRENT_BINARY_DIR}/line.frag.glsl.h"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/line.frag.glsl"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
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
