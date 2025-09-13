project(gltfview)

add_executable(gltfview
        main.c
        ../../contrib/editorcam/editorcam.c
        ../../contrib/editorcam/editorcam.h
        ../../contrib/editorcam/editorcam_p.h
)

target_include_directories(gltfview PRIVATE ../../contrib/editorcam)

target_link_libraries(gltfview PRIVATE BRender::Core BRender::Drivers::SDL3Dev BRender::Drivers::GLRend)

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS gltfview DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
