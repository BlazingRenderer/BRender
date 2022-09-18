project(mkranges)

add_executable(mkranges ./mkranges.c)
target_link_libraries(mkranges PRIVATE brender)
