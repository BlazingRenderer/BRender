project(mkshades)

add_executable(mkshades ./mkshades.c)
target_link_libraries(mkshades PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS mkshades DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
