add_subdirectory(brdemo)

add_subdirectory(brintro)
add_subdirectory(robot)
add_subdirectory(cube)
add_subdirectory(mkres)
add_subdirectory(devpmtest)
add_subdirectory(tutorials)
add_subdirectory(samples)

if(BRENDER_BUILD_SOFT)
    add_subdirectory(softcube)
endif()

find_package(glfw3 3)
if(TARGET glfw)
    add_subdirectory(glfw3-opengl)
endif ()
