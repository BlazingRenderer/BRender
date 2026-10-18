project(gltfbench)

add_executable(gltfbench
        main.c
)

target_link_libraries(gltfbench PRIVATE BRender::Full BRender::DDI brcapture parg glrend-headers glrend1x-headers)
set_property(TARGET gltfbench PROPERTY FOLDER "BRender/Tools")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS gltfbench DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
