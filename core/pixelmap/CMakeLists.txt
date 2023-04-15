project(pixelmap)

add_library(pixelmap-headers INTERFACE)
target_include_directories(pixelmap-headers INTERFACE .)
target_link_libraries(pixelmap-headers INTERFACE fw-headers)
target_link_libraries(pixelmap-headers INTERFACE math-headers)

set(PIXELMAP_FILES
	./fontptrs.c
	./genclip.c
	./gencopy.c
	./mipsupt.c
	./pixelmap.c
	./pmdsptch.c
	./pmfail.c
	./pmfile.c
	./pmgen.c
	./pmmem.c
	./pmnull.c
	./pmsetup.c
	./pmutil.c
	./quantize.c
	./scale.c
	./cmemloops.c

	./pm.h
	./pm_ip.h
	./pmmem.h
)

set(FONT_FILES
	./fnt3x5.c
	./fntp4x6.c
	./fntp7x9.c
)
source_group("Fonts" FILES ${FONT_FILES})

add_library(pixelmap STATIC ${PIXELMAP_FILES} ${FONT_FILES})
target_include_directories(pixelmap PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(pixelmap pixelmap-headers)
target_link_libraries(pixelmap fw)
target_compile_definitions(pixelmap PRIVATE __BR_V1DB__=0)

set_property(TARGET pixelmap PROPERTY FOLDER ${BRENDER_FOLDER})

