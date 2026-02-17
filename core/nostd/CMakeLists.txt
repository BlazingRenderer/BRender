project(nostd)

set(NOSTD_FILES
        brexcept.c
        logwrite.c
        math.c
        diag.c
        file.c
        lib.c
        log.c
        mem.c
        )

add_library(nostd OBJECT ${NOSTD_FILES})
target_compile_definitions(nostd PRIVATE __BR_V1DB__=0)
target_link_libraries(nostd PRIVATE brender-inc-ddi)
