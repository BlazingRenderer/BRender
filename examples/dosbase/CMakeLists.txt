add_library(dosbase
		OBJECT
		dosio.h
		dosio.c
		)
target_link_libraries(dosbase BRender::Full brsdl)
target_include_directories(dosbase PUBLIC .)