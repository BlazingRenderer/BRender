project(editorcam)

add_executable(editorcam
        main.c
        editorcam.c
        editorcam.h
        editorcam_p.h
)

target_link_libraries(editorcam brdemo)
