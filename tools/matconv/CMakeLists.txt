project(matconv)

add_executable(matconv matconv.c)
target_link_libraries(matconv PRIVATE BRender::Core)

if (NOT BRENDER_DISABLE_INSTALL)
	install(TARGETS matconv DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
