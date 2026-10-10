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

#
# The corpus is this project's test suite. Each configuration is its own test so
# that a failure names the configuration, and nothing here passes --bless:
# accepting the current output is a deliberate act, not a test.
#
# --history is deliberately not registered - it walks commits and needs a git
# checkout, so it belongs to contrib/run-tests.sh rather than to the build.
#
if (BUILD_TESTING)
    foreach (bpp IN ITEMS 8 15 16 24)
        foreach (mode IN ITEMS zb zs)
            if (mode STREQUAL "zs")
                set(depth --no-depth)
            else ()
                set(depth)
            endif ()

            add_test(NAME "softrend-${bpp}-${mode}"
                     COMMAND rendertest --device softrend --bpp ${bpp} ${depth})
            set_tests_properties("softrend-${bpp}-${mode}" PROPERTIES
                                 ENVIRONMENT "SDL_VIDEODRIVER=offscreen")
        endforeach ()
    endforeach ()

    #
    # The stored glrend reference is llvmpipe's, so ask for software GL.
    #
    # There is no bpp axis and the name carries none: --bpp is inert on a GL
    # device, because the pixel type a key is built from is the colour
    # buffer's, which the driver chooses, not the flag.
    #
    # Both depth modes are registered. 15 of the 102 fixtures place a different
    # frame under --no-depth than under the depth buffer - the same 15 in each
    # fp class - so the z-sorted half is coverage, not a duplicate of the
    # z-buffered one.
    add_test(NAME "glrend-zb" COMMAND rendertest --device glrend)
    set_tests_properties("glrend-zb" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")

    add_test(NAME "glrend-zs" COMMAND rendertest --device glrend --no-depth)
    set_tests_properties("glrend-zs" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")

    #
    # glrend1x is softrend's transform-and-lighting over glrend's rasteriser. It
    # is a GL device too, so it shares the bpp-less name and the software-GL
    # environment, but not the reference: its pixel type is 7 against glrend's
    # 8, and 15 of the 102 fixtures put a different frame under --no-depth - a
    # different 15 from glrend's, so this is coverage and not a copy.
    #
    # Unlike glrend, the fp class is not inert here: the T&L runs in this
    # binary, so the key records it, and scene-blend-p2-persp differs between
    # x87 and declared where the other 101 fixtures agree.
    add_test(NAME "glrend1x-zb" COMMAND rendertest --device glrend1x)
    set_tests_properties("glrend1x-zb" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")

    add_test(NAME "glrend1x-zs" COMMAND rendertest --device glrend1x --no-depth)
    set_tests_properties("glrend1x-zs" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")

    add_test(NAME "fixtures"
             COMMAND ${CMAKE_COMMAND}
                     "-DMKRES=$<TARGET_FILE:mkres>"
                     "-DDAT=${CMAKE_CURRENT_SOURCE_DIR}/dat"
                     "-DWORK=${CMAKE_CURRENT_BINARY_DIR}/fixture-check"
                     -P "${CMAKE_CURRENT_SOURCE_DIR}/fixture-check.cmake")

    add_test(NAME "gltf-loads"
             COMMAND ${CMAKE_COMMAND}
                     "-DGLTFVIEW=$<TARGET_FILE:gltfview>"
                     "-DDIR1=${CMAKE_CURRENT_SOURCE_DIR}/dat"
                     "-DDIR2=${CMAKE_SOURCE_DIR}/resources/gltf-reference"
                     -P "${CMAKE_CURRENT_SOURCE_DIR}/gltf-check.cmake")
    set_tests_properties("gltf-loads" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;GLTFVIEW_BENCH_FRAMES=5;GLTFVIEW_BENCH_NOVSYNC=1"
                         TIMEOUT 900)
endif ()
