project(fmt)

add_library(fmt-headers INTERFACE)
target_include_directories(fmt-headers INTERFACE .)
target_link_libraries(fmt-headers INTERFACE inc)

set(FMT_FILES
	./load3ds.c
	./loadasc.c
	./loadbmp.c
	./loadgif.c
	./loadiff.c
	./loadnff.c
	./loadscr.c
	./loadtga.c
	./loadvue.c
	./savescr.c

	./fmt.h
	./fmt_p.h
	./load3ds.h
)

add_library(fmt STATIC ${FMT_FILES})

target_link_libraries(fmt inc)
target_link_libraries(fmt fw-headers)
target_link_libraries(fmt v1db)

set_property(TARGET fmt PROPERTY FOLDER ${BRENDER_FOLDER})
