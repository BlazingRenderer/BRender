project(3ds2br)

add_executable(3ds2br ./3ds2br.c)
target_link_libraries(3ds2br PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS 3ds2br DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
