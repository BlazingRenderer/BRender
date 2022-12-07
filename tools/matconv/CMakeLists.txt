project(matconv)

add_executable(matconv matconv.c)
target_link_libraries(matconv PRIVATE BRender::Core)
