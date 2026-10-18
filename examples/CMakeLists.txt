if(TARGET BRender::Drivers::SDL3Dev AND TARGET BRender::Drivers::GLRend)
    add_subdirectory(brdemo)

    add_subdirectory(brintro)
    add_subdirectory(robot)
    add_subdirectory(cube)
    add_subdirectory(mkres)
    add_subdirectory(devpmtest)
    add_subdirectory(gltfview)
    add_subdirectory(tutorials)
    add_subdirectory(samples)

endif()

# rendertest is the CI corpus: it covers the GL devices and the vkrend witness,
# so it wants everything CI has. A tree without vkrend is not one it is meant to
# run in, and it is not built there rather than registered in part.
if(TARGET BRender::Drivers::SDL3Dev AND TARGET BRender::Drivers::GLRend AND TARGET BRender::Drivers::VKRend)
    add_subdirectory(rendertest)
endif()

if(TARGET BRender::Drivers::SDL3Dev AND TARGET BRender::Drivers::SoftPrim)
    add_subdirectory(softcube)
endif()

if(TARGET BRender::Drivers::MCGA AND TARGET BRender::Drivers::SoftPrim)
    add_subdirectory(doscube)
endif()

if(BRENDER_BUILD_GLFW_EXAMPLE AND TARGET BRender::Drivers::GLRend)
    find_package(glfw3 3)
    if(TARGET glfw)
        add_subdirectory(glfw3-opengl)
    endif ()
endif()
