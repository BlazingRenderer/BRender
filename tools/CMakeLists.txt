foreach(target 3ds2br dquery dripmem mkblend mkfog mkranges mkshades paljoin texconv matconv texview)
	add_subdirectory(${target})
	set_property(TARGET ${target} PROPERTY FOLDER "BRender/Tools")
endforeach()

# Debug tools - enable/disable them as required.
# add_subdirectory(mathtest)
