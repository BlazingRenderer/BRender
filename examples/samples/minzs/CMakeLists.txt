project(minzs)

add_executable(minzs
        minzs.c
)
target_link_libraries(minzs PRIVATE brdemo)
set_property(TARGET minzs PROPERTY FOLDER "BRender/Examples/Samples")

if(TARGET SDL2::SDL2main)
        target_link_libraries(minzs PRIVATE SDL2::SDL2main)
endif()

