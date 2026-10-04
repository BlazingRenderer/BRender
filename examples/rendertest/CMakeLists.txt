project(rendertest)

add_executable(rendertest
        main.c
)

target_link_libraries(rendertest PRIVATE BRender::Full BRender::DDI brdemo)

target_compile_definitions(rendertest PRIVATE
        RT_REFERENCE_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/rendertest.txt"
        RT_SCENE_DIR_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/dat"
)

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS rendertest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
