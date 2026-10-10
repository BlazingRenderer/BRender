project(rendertest)

add_executable(rendertest
        main.c
)

target_link_libraries(rendertest PRIVATE BRender::Full BRender::DDI brdemo)

target_compile_definitions(rendertest PRIVATE
        RT_REFERENCE_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/rendertest.txt"
        RT_SCENE_DIR_DEFAULT="${CMAKE_CURRENT_SOURCE_DIR}/dat"
)

#
# The software rasteriser is chosen at link time, so which one produced a frame
# is a build fact and belongs in the reference key. softprim registers the name
# pentprim did.
#
set (RT_SOFT_DRIVER "softprmf")

target_compile_definitions(rendertest PRIVATE RT_SOFT_DRIVER="${RT_SOFT_DRIVER}")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS rendertest DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
