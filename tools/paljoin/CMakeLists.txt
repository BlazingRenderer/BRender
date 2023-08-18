project(paljoin)

add_executable(paljoin ./paljoin.c)
target_link_libraries(paljoin PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS paljoin DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
