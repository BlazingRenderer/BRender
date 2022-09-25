project(mkfog)

add_executable(mkfog ./mkfog.c)
target_link_libraries(mkfog PRIVATE BRender::Core)
