project(math)

add_library(math-headers INTERFACE)
target_include_directories(math-headers INTERFACE .)
target_link_libraries(math-headers INTERFACE host-headers)

set(MATH_FILES
	./angles.c
	./fixed.c
	./matrix4.c
	./matrix23.c
	./matrix34.c
	./plane.c
	./quat.c
	./scalar.c
	./transfrm.c
	./vector.c

	./brmath.h
	./math_ip.h
	./vecifns.h
)

add_library(math STATIC ${MATH_FILES})
target_link_libraries(math PUBLIC math-headers)
target_compile_definitions(math PRIVATE __BR_V1DB__=0)

if(NOT WIN32)
	target_link_libraries(math PUBLIC m)
endif()

set_property(TARGET math PROPERTY FOLDER ${BRENDER_FOLDER})

