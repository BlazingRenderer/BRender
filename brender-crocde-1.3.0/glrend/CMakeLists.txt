project(glrend)

find_package(OpenGL REQUIRED)

set(GLSTATE_FILES
	./glcache.cpp
	./glstate.cpp
	./glstate.hpp
	./glstate_cull.cpp
	./glstate_hidden.cpp
	./glstate_ip.hpp
	./glstate_light.cpp
	./glstate_matrix.cpp
	./glstate_output.cpp
	./glstate_primitive.cpp
	./glstate_surface.cpp
	./glstate_global.cpp
)
source_group("STATE" FILES ${GLSTATE_FILES})

set(VIDEO_FILES
	./checkboard64.cpp
	./shader_brender.cpp
	./shader_default.cpp
	./shader_text.cpp
	./video.cpp
	./video.h
	./video_fonts.cpp
	./video_ip.hpp
)
source_group("VIDEO" FILES ${VIDEO_FILES})

set(RENDERER_FILES
	./gprim.cpp
	./gprim.hpp

	./gv1buckt.cpp
	./gv1buckt.hpp

	./gv1model.cpp
	./gv1model.hpp

	./gv1stored.cpp

	./lighting.cpp
	./lighting.hpp

	./onscreen.cpp

	./renderer.cpp
	./renderer.hpp

	./sbuffer.cpp
	./sbuffer.hpp

	./sstate.cpp
	./sstate.hpp
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
	./device.hpp
	./devpixmp.hpp
	./drv.hpp
	./drv_ip.hpp
	./gl_core_3_2.h
	./object.hpp
	./rendfcty.hpp
	./template.hpp

	./device.cpp
	./devpixmp.cpp
	./driver.cpp
	./gl_core_3_2.c
	./outfcty.cpp
	./rendfcty.cpp
)

add_library(glrend STATIC
	${GLREND_FILES}
	${RENDERER_FILES}
	${VIDEO_FILES}
	${GLSTATE_FILES}
)

target_link_libraries(glrend glfw)
target_link_libraries(glrend log)
target_link_libraries(glrend fw)
target_link_libraries(glrend pixelmap)
target_link_libraries(glrend glm)
target_link_libraries(glrend OpenGL::GL)
set_property(TARGET glrend PROPERTY FOLDER ${BRENDER_FOLDER})
