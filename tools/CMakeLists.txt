foreach(target 3ds2br dquery dripmem mkblend mkfog mkranges mkshades paljoin texconv matconv texview)
	add_subdirectory(${target})
	set_property(TARGET ${target} PROPERTY FOLDER "BRender/Tools")
endforeach()

find_package(assimp)
if(TARGET assimp::assimp)
	add_subdirectory(brmc)
endif()

# Debug tools - enable/disable them as required.
# add_subdirectory(mathtest)
