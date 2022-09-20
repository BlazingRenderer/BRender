project(fw)

add_custom_command(
	OUTPUT dev_objs.cgh
	# This will work with MSVC and GCC
	COMMAND "${CMAKE_C_COMPILER}" -D__CLASSGEN__ -E "${CMAKE_CURRENT_SOURCE_DIR}/dev_objs.hpp" > dev_objs.tmp
	COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/classgen.pl" < dev_objs.tmp > dev_objs.cgh
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/dev_objs.hpp"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/classgen.pl"
	BYPRODUCTS dev_objs.tmp
)

set(FW_DEV_FILES
	${CMAKE_CURRENT_BINARY_DIR}/dev_objs.cgh
	${CMAKE_CURRENT_SOURCE_DIR}/dev_objs.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/classgen.pl
)
source_group("Device" FILES ${FW_DEV_FILES})
set_source_files_properties(${FW_DEV_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

add_custom_target(fw-dev-headers DEPENDS ${FW_DEV_FILES})

add_library(fw-headers INTERFACE)
target_include_directories(fw-headers INTERFACE .)
target_include_directories(fw-headers INTERFACE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(fw-headers INTERFACE host-headers)
target_link_libraries(fw-headers INTERFACE fmt-headers)
add_dependencies(fw-headers fw-dev-headers)

add_custom_command(
	OUTPUT fwimg.h fwimg.def
	COMMAND "${PERL_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/../resgen.pl" "fwimg.h" "fwimg.def" < "${CMAKE_CURRENT_SOURCE_DIR}/fwimg.fns"
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/fwimg.fns"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../resgen.pl"
)

set(FW_FILES
	./assocarr.c
	./brbegin.c
	./brbhook.c
	./brlists.c
	./brqsort.c
	./bswap.c
	./datafile.c
	./defdiag.c
	./deffile.c
	./defmem.c
	./devlist.c
	./devquery.c
	./devsetup.c
	./diag.c
	./error.c
	./file.c
	./fwsetup.c
	./genfile.c
	./image.c
	./lexer.c
	./loader.c
	./log.c
	./mem.c
	./object.c
	./objectc.c
	./pattern.c
	./pool.c
	./printf.c
	./register.c
	./resource.c
	./resreg.c
	./scratch.c
	./scrstr.c
	./sys_conf.c
	./token.c
	./tokenval.c

	./blockops.h
	./brassert.h
	./brddi.h
	./brddi_p.h
	./brexcept.h
	./brhton.h
	./brimage.h
	./brlists.h
	./brstdlib.h
	./datafile.h
	./dataf_p.h
	./formats.h
	./fw.h
	./fw_ip.h
	./lexer.h
	./register.h
	./shortcut.h
	./tokenmap.h
	./wincoff.h

	${CMAKE_CURRENT_BINARY_DIR}/fwimg.h
	./fwimg.fns
)

add_library(fw STATIC ${FW_FILES} ${FW_DEV_FILES})
target_link_libraries(fw fw-headers)
target_link_libraries(fw host)
target_link_libraries(fw std)
target_link_libraries(fw math)
target_link_libraries(fw pixelmap)
target_compile_definitions(fw PRIVATE __BR_V1DB__=0)

set_property(TARGET fw PROPERTY FOLDER ${BRENDER_FOLDER})
