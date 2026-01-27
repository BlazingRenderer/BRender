project(dreamcast)

set(DREAMCAST_FILES
        brexcept.c
        logwrite.c
        math.c
        dcdiag.c
        dcfile.c
        dclib.c
        dclog.c
        dcmem.c
        )

add_library(dreamcast OBJECT ${DREAMCAST_FILES})
target_compile_definitions(dreamcast PRIVATE __BR_V1DB__=0)
target_link_libraries(dreamcast PRIVATE brender-inc-ddi)
