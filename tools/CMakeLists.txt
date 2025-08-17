foreach(target 3ds2br dquery dripmem mkblend mkfog mkranges mkshades paljoin texconv matconv gltfview)
	add_subdirectory(${target})
	set_property(TARGET ${target} PROPERTY FOLDER "BRender/Tools")
endforeach()

if(TARGET SDL3::SDL3)
	add_subdirectory(texview)
	set_property(TARGET texview PROPERTY FOLDER "BRender/Tools")
endif()

# Debug tools - enable/disable them as required.
# add_subdirectory(mathtest)
