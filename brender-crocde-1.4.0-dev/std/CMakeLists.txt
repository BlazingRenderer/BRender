project(std)

add_library(std-headers INTERFACE)
target_include_directories(std-headers INTERFACE .)
target_link_libraries(std-headers INTERFACE fw-headers)

set(STD_FILES
	./brexcept.c
	./logwrite.c
	./stddiag.c
	./stdfile.c
	./stdlib.c
	./stdlog.c
	./stdmem.c
)
add_library(std STATIC ${STD_FILES})
target_link_libraries(std std-headers)
target_compile_definitions(std PRIVATE __BR_V1DB__=0)

set_property(TARGET std PROPERTY FOLDER ${BRENDER_FOLDER})

