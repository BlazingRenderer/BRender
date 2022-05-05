project(host)

add_library(host-headers INTERFACE)
target_include_directories(host-headers INTERFACE .)
target_link_libraries(host-headers INTERFACE inc)
target_link_libraries(host-headers INTERFACE fw-headers)

set(HOST_FILES
	./himage.c
	./hostcfg.c
	./hstsetup.c
	./posixcompat.cpp
	./host.h
	./host_ip.h
	./host_p.h
	./hostimg.h
	./posixcompat.h
)

add_library(host STATIC ${HOST_FILES})
target_link_libraries(host host-headers)

set_property(TARGET host PROPERTY FOLDER ${BRENDER_FOLDER})

