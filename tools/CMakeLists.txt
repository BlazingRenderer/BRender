foreach(target 3ds2br dquery dripmem mkblend mkfog mkranges mkshades paljoin texconv matconv)
	add_subdirectory(${target})
	set_property(TARGET ${target} PROPERTY FOLDER "BRender/Tools")
endforeach()

if(TARGET SDL3::SDL3)
	add_subdirectory(texview)
	set_property(TARGET texview PROPERTY FOLDER "BRender/Tools")
endif()

# The vendored argument parser, compiled into each tool that wants it. brdemo
# keeps its own copy: it must not depend on the tools to build.
add_subdirectory(parg)
set_property(TARGET parg PROPERTY FOLDER "BRender/Tools")

# mkres generates the render-test fixtures and needs only core.
add_subdirectory(mkres)
set_property(TARGET mkres PROPERTY FOLDER "BRender/Tools")

# The harnesses and the capture library they share. rendertest needs a renderer
# and devpmtest a device, so they follow the drivers rather than core alone.
if(BRENDER_HAVE_SDL3 AND BRENDER_HAVE_GL)
	foreach(target brcapture devpmtest)
		add_subdirectory(${target})
		set_property(TARGET ${target} PROPERTY FOLDER "BRender/Tools")
	endforeach()

	# The headless glTF scene benchmark; a tool, not a demo.
	add_subdirectory(gltfbench)

	# rendertest is the CI corpus: it covers the GL devices and the vkrend
	# witness, so it wants everything CI has. A tree without vkrend is not one
	# it is meant to run in, and the corpus is not built there rather than
	# registered in part.
	if(BRENDER_HAVE_VK)
		add_subdirectory(rendertest)
		set_property(TARGET rendertest PROPERTY FOLDER "BRender/Tools")
	endif()
endif()

# Debug tools - enable/disable them as required.
# add_subdirectory(mathtest)
