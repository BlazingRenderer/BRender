#
# Dear ImGui over BRender's 2D primitives.
#
# The vendored files are Dear ImGui 1.91.9b: its core, and the SDL3 platform
# backend for input. imgui_impl_brender is this tree's renderer backend, and
# main.cpp is the demo. See LICENSE.txt.
#
project(imgui-demo CXX)

add_executable(imgui-demo
        # Dear ImGui 1.91.9b, unmodified.
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui.h
        imgui_internal.h
        imconfig.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h

        imgui_impl_sdl3.cpp
        imgui_impl_sdl3.h

        # The BRender renderer backend and the demo.
        imgui_impl_brender.cpp
        imgui_impl_brender.h
        main.cpp
)

target_compile_features(imgui-demo PRIVATE cxx_std_17)
target_include_directories(imgui-demo PRIVATE .)
target_link_libraries(imgui-demo PRIVATE BRender::Full brdemo SDL3::SDL3)
set_property(TARGET imgui-demo PROPERTY FOLDER "BRender/Examples")

if (NOT BRENDER_DISABLE_INSTALL)
    install(TARGETS imgui-demo DESTINATION ${CMAKE_INSTALL_BINDIR})
endif ()
