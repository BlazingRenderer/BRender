project(sample-glfw3-opengl)

add_executable(sample-glfw3-opengl
        main.c
)

target_link_libraries(sample-glfw3-opengl BRender::Core BRender::Drivers::GLRend glfw)
set_property(TARGET sample-glfw3-opengl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../dat")
set_property(TARGET sample-glfw3-opengl PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
        install(TARGETS sample-glfw3-opengl DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
