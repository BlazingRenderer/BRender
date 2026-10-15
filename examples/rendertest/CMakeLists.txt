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
    # The reference scenes: the .gltf files under resources/gltf-reference. They
    # are a second corpus - the same bpp x depth configurations, but a different
    # --scene-dir and an explicit scene list, because they are not fixtures this
    # binary links in. They are kept because they reach states the fixtures do
    # not; drivers/softprim/README.md says what for.
    #
    # The directory is the source of truth for which scenes these are: the list
    # below is a glob of it, never a copy, and contrib/census/run_corpus.sh reads
    # the same set through rendertest --list-scene-dir. CONFIGURE_DEPENDS so a
    # scene added or removed re-configures rather than leaving a stale list.
    #
    # One test per (bpp, mode, scene), so that a failure names the scene.
    #
    # The name deliberately does not start with a device token. The --bless loop
    # in contrib/run-tests.sh rebuilds each corpus test from its name, and it
    # cannot rebuild one that carries a different --scene-dir and scene list, so
    # these are skipped there (as `fixtures' and `gltf-loads' are) rather than
    # blessed as the wrong configuration. Blessing them is deliberate and
    # separate; drivers/softprim/README.md says how.
    #
    file(GLOB RT_REFERENCE_SCENE_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/resources/gltf-reference/*.gltf")
    if (NOT RT_REFERENCE_SCENE_FILES)
        message(FATAL_ERROR "no .gltf under ${CMAKE_SOURCE_DIR}/resources/gltf-reference; the reference-scene tests would register nothing")
    endif ()

    set(RT_REFERENCE_SCENES)
    foreach (f IN LISTS RT_REFERENCE_SCENE_FILES)
        get_filename_component(f "${f}" NAME)
        string(REGEX REPLACE "\\.gltf$" "" f "${f}")
        list(APPEND RT_REFERENCE_SCENES "${f}")
    endforeach ()
    list(SORT RT_REFERENCE_SCENES)

    foreach (bpp IN ITEMS 8 15 16 24)
        foreach (mode IN ITEMS zb zs)
            if (mode STREQUAL "zs")
                set(ref_depth --no-depth)
            else ()
                set(ref_depth)
            endif ()

            foreach (scene IN LISTS RT_REFERENCE_SCENES)
                add_test(NAME "refscenes-${bpp}-${mode}-${scene}"
                         COMMAND rendertest --device softrend --bpp ${bpp} ${ref_depth}
                                 --scene-dir "${CMAKE_SOURCE_DIR}/resources/gltf-reference"
                                 ${scene})
                set_tests_properties("refscenes-${bpp}-${mode}-${scene}" PROPERTIES
                                     ENVIRONMENT "SDL_VIDEODRIVER=offscreen")
            endforeach ()
        endforeach ()
    endforeach ()

    #
    # The same nine scenes through the GL devices, at the driver's own colour
    # buffer: these pass no --bpp, so glrend keys type 8 and glrend1x type 7.
    # One test per (mode, scene) shape, so a failure names the scene. The
    # entries are llvmpipe's, as the fixture ones are, so a run on real
    # hardware reports NO-REFERENCE.
    #
    foreach (mode IN ITEMS zb zs)
        if (mode STREQUAL "zs")
            set(ref_gl_depth --no-depth)
        else ()
            set(ref_gl_depth)
        endif ()

        foreach (dev IN ITEMS glrend glrend1x)
            foreach (scene IN LISTS RT_REFERENCE_SCENES)
                add_test(NAME "refscenes-${dev}-${mode}-${scene}"
                         COMMAND rendertest --device ${dev} ${ref_gl_depth}
                                 --scene-dir "${CMAKE_SOURCE_DIR}/resources/gltf-reference"
                                 ${scene})
                set_tests_properties("refscenes-${dev}-${mode}-${scene}" PROPERTIES
                                     ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")
            endforeach ()
        endforeach ()
    endforeach ()

    # The GL fixture corpus. On glrend the colour buffer's pixel type is an axis
    # here as it is for softrend, so its name is `<device>-<bpp>-<mode>` and the
    # key records the type. --bpp 32 is RGBA_8888, glrend's own colour buffer,
    # and reproduces the stored keys; 15/16/24 are new coverage. --bpp 8 is
    # refused on a GL device, the indexed colour buffer being a GL_R8UI integer
    # attachment the scene's vec4 shader output cannot write.
    #
    # glrend1x has no type axis. Its colour buffer is the window framebuffer,
    # which is RGBA whatever type the token asks for - GL 1.2 has no FBOs, so
    # the driver cannot honour a requested type and all five of its per-type
    # keys were the same picture. Its name is `<device>-<mode>` and it passes no
    # --bpp, keying its own RGBX_888 (7) as the refscenes-glrend1x-* tests below
    # already do.
    #
    # The stored reference is llvmpipe's, so ask for software GL.
    #
    # Both depth modes are registered. 15 of the 106 fixtures place a different
    # frame under --no-depth than under the depth buffer - the same 15 in each
    # fp class for glrend, where the T&L is in the shader - so the z-sorted half
    # is coverage, not a duplicate of the z-buffered one.
    #
    # Unlike glrend, glrend1x's fp class is not inert: the T&L runs in this
    # binary, so the key records it, and scene-blendfog-p2 and
    # scene-blend-p2-persp differ between x87 and declared. Its z-sorted half is
    # likewise coverage: 15 of the 106 fixtures differ under --no-depth, a
    # different 15 from glrend's.
    #
    foreach (mode IN ITEMS zb zs)
        if (mode STREQUAL "zs")
            set(gl_depth --no-depth)
        else ()
            set(gl_depth)
        endif ()

        foreach (bpp IN ITEMS 15 16 24 32)
            add_test(NAME "glrend-${bpp}-${mode}"
                     COMMAND rendertest --device glrend --bpp ${bpp} ${gl_depth})
            set_tests_properties("glrend-${bpp}-${mode}" PROPERTIES
                                 ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")
        endforeach ()

        add_test(NAME "glrend1x-${mode}"
                 COMMAND rendertest --device glrend1x ${gl_depth})
        set_tests_properties("glrend1x-${mode}" PROPERTIES
                             ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")
    endforeach ()

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

    #
    # The environment-map orientation witness (main.c --env-witness). It is not
    # a fixture: a device offscreen texture cannot be carried through a .gltf,
    # so the witness builds the host and the device map at runtime and asserts
    # the sampled texel against softrend's sphere map. It is a self-check, not a
    # stored checksum, so it has nothing to bless.
    #
    # glrend only. It is the device whose shader projects the environment map,
    # and so the only one that reads the host/device distinction at all:
    # glrend1x's map coordinates come from softrend's transform-and-lighting,
    # which does not use glrend's shader. glrend1x also cannot run the witness -
    # GL 1.x has no FBOs, so its device allows exactly one offscreen and the
    # witness needs two - and softrend cannot render to the RGBA_8888 offscreen
    # the texel readback needs, its RGB rasterisers being 555/565/888 only.
    #
    add_test(NAME "envw-glrend" COMMAND rendertest --env-witness --device glrend)
    set_tests_properties("envw-glrend" PROPERTIES
                         ENVIRONMENT "SDL_VIDEODRIVER=offscreen;LIBGL_ALWAYS_SOFTWARE=true")
endif ()
