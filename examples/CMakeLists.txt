if(BRENDER_HAVE_SDL3 AND BRENDER_HAVE_GL)
    add_subdirectory(brdemo)

    add_subdirectory(brintro)
    add_subdirectory(robot)
    add_subdirectory(cube)
    add_subdirectory(gltfview)
    add_subdirectory(imgui-demo)
    add_subdirectory(tutorials)
    add_subdirectory(samples)

endif()

if(BRENDER_HAVE_SDL3)
    add_subdirectory(softcube)
endif()

if(BRENDER_HAVE_MCGA)
    add_subdirectory(doscube)
endif()

if(BRENDER_BUILD_GLFW_EXAMPLE AND BRENDER_HAVE_GL)
    find_package(glfw3 3)
    if(TARGET glfw)
        add_subdirectory(glfw3-opengl)
    endif ()
endif()
