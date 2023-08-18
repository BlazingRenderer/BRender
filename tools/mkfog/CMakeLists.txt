project(mkfog)

add_executable(mkfog ./mkfog.c)
target_link_libraries(mkfog PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS mkfog DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
