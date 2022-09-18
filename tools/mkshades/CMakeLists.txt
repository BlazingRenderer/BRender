project(mkshades)

add_executable(mkshades ./mkshades.c)
target_link_libraries(mkshades PRIVATE brender)
