project(std)

set(STD_FILES
        brexcept.c
        logwrite.c
        math.c
        stddiag.c
        stdfile.c
        stdlib.c
        stdlog.c
        stdmem.c
        )

add_library(std OBJECT ${STD_FILES})
target_compile_definitions(std PRIVATE __BR_V1DB__=0)
target_link_libraries(std PRIVATE brender-inc-ddi)
