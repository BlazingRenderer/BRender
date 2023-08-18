project(dquery)

add_executable(dquery ./dquery.c)
target_link_libraries(dquery PRIVATE BRender::Core BRender::DDI)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS dquery DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
