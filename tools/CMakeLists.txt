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

# Debug tools - enable/disable them as required.
# add_subdirectory(mathtest)
