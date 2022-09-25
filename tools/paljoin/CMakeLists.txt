project(paljoin)

add_executable(paljoin ./paljoin.c)
target_link_libraries(paljoin PRIVATE BRender::Core)
