project(mkranges)

add_executable(mkranges ./mkranges.c)
target_link_libraries(mkranges PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS mkranges DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
