add_library(dosbase
		OBJECT
		dosio.h
		dosio.c
		)
target_link_libraries(dosbase BRender::Full)
target_include_directories(dosbase PUBLIC .)