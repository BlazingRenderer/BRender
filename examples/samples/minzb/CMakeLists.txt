project(minzb)

add_executable(minzb
        minzb.c
)
target_compile_definitions(minzb PUBLIC -DSDL_MAIN_HANDLED)
target_link_libraries(minzb BRender::Full brdemo)
set_property(TARGET minzb PROPERTY FOLDER "BRender/Examples/Samples")
