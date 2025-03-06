function(add_example name)
	add_executable(${name}
			${name}.c
			)
	target_link_libraries(${name} PRIVATE brdemo)
	set_target_properties(${name} PROPERTIES
			VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dat"
			FOLDER "BRender/Examples/Tutorials"
	)

	if(TARGET SDL2::SDL2main)
		target_link_libraries(${name} PRIVATE SDL2::SDL2main)
	endif()

	if (NOT BRENDER_DISABLE_INSTALL)
		install(TARGETS ${name} DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif ()
endfunction()

add_example(brtutor1)
add_example(brtutor2)
add_example(brtutor3)
add_example(brtutor4)
add_example(brtutor5)
add_example(brtutor6)
add_example(brtutor7)
add_example(brtutor8)
add_example(brtutor9)
add_example(brtutor10)

set(TUTORIAL_FILES cube.mat cylinder.dat duck.3ds duck.dat duck.mat
		duck8.mat e.png e.tga earth.mat earth15.pix earth8.pix
		fork.3ds fork.asc fork.dat fork.mat gold.gif gold15.mat gold15.pix
		gold8.mat gold8.pix mkranges.txt refmap.gif refmap.pix refmap.png
		refmap.tga shade.tab sph16.dat sph32.dat sph4096.dat sph8.dat std.pal
		strippal.txt tile.pix tile0011.tga torus.dat
		)

foreach(file ${TUTORIAL_FILES})
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/dat/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
