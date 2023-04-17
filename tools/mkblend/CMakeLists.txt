project(mkblend)

add_executable(mkblend ./mkblend.c)
target_link_libraries(mkblend PRIVATE BRender::Core)

install(TARGETS mkblend)
