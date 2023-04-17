project(dquery)

add_executable(dquery ./dquery.c)
target_link_libraries(dquery PRIVATE BRender::Core BRender::DDI)

install(TARGETS dquery)
