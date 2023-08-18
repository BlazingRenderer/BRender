project(mkblend)

add_executable(mkblend ./mkblend.c)
target_link_libraries(mkblend PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS mkblend DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
