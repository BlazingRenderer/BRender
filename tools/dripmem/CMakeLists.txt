project(dripmem)

add_library(dripmem STATIC
        ./dripmem.c
        ./dripmem.h
        ./stdmem.c
)
target_link_libraries(dripmem PRIVATE BRender::Core)
