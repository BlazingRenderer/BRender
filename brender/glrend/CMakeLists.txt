project(glrend)

find_package(OpenGL REQUIRED)

add_subdirectory(glad)

add_custom_command(
		OUTPUT brender.vert.glsl.h
		COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_DefaultVertexShader" "${CMAKE_CURRENT_SOURCE_DIR}/brender.vert.glsl" brender.vert.glsl.h
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/brender.vert.glsl"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

add_custom_command(
		OUTPUT brender.frag.glsl.h
		COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl" "g_DefaultFragmentShader" "${CMAKE_CURRENT_SOURCE_DIR}/brender.frag.glsl" brender.frag.glsl.h
		MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/brender.frag.glsl"
		DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/embedshader.pl"
)

set(GLSTATE_FILES
	./glcache.c
	./glstate.c
	./glstate.h
	./glstate_cull.c
	./glstate_hidden.c
	./glstate_ip.h
	./glstate_light.c
	./glstate_matrix.c
	./glstate_output.c
	./glstate_primitive.c
	./glstate_surface.c
)
source_group("STATE" FILES ${GLSTATE_FILES})

set(VIDEO_FILES
	./checkboard64.c
	./shader_brender.c
	./shader_default.c
	./shader_text.c
	./video.c
	./video.h
)
source_group("VIDEO" FILES ${VIDEO_FILES})

set(RENDERER_FILES
	./gv1buckt.c
	./gv1buckt.h

	./gv1model.c
	./gv1model.h

	./gstored.c
	./gstored.h
	./v1model.c

	./onscreen.c

	./renderer.c
	./renderer.h

	./sbuffer.h
	./sbuffer.c

	./sstate.c
	./sstate.h
)
source_group("Renderer" FILES ${RENDERER_FILES})

##
# lua ../../../glLoadGen_2_0_2/LoadGen.lua -spec gl -version 3.2 \
#	-profile core -style pointer_c -indent tab \
#	-ext EXT_texture_filter_anisotropic \
#	-ext EXT_texture_lod_bias \
#	core_3_2
##
set(GLREND_FILES
	./drv.h
	./drv_ip.h

	./glad/src/glad.c
	./glad/include/glad/glad.h
	./glad/include/KHR/khrplatform.h

	./driver.c

	./outfcty.c
	./outfcty.h

	./ext_procs.c
	./ext_procs_sdl.c

	./rendfcty.c
	./rendfcty.h

	./device.c
	./device.h

	./devpixmp.c
	./devpixmp.h

	./font.c
	./font.h

	./template.h
)

add_library(glrend STATIC
	${GLREND_FILES}
	${RENDERER_FILES}
	${VIDEO_FILES}
	${GLSTATE_FILES}

	./include/brglrend.h
	./include/brglrend_p.h

	"${CMAKE_CURRENT_BINARY_DIR}/brender.vert.glsl.h"
	"${CMAKE_CURRENT_BINARY_DIR}/brender.frag.glsl.h"
)

get_target_property(target_type glrend TYPE)
if(NOT target_type STREQUAL STATIC_LIBRARY)
	target_compile_definitions(glrend PRIVATE -DDEFINE_BR_ENTRY_POINT)
endif()

target_include_directories(glrend PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(glrend PUBLIC include)

target_link_libraries(glrend glrend-glad)
target_link_libraries(glrend SDL2::SDL2)
target_link_libraries(glrend fw)
target_link_libraries(glrend v1db)
target_link_libraries(glrend nulldev)
target_link_libraries(glrend OpenGL::GL)
set_property(TARGET glrend PROPERTY FOLDER ${BRENDER_FOLDER})
