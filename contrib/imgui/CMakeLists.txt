#
# Dear ImGui 1.91.9b, as a library for the examples that draw a UI.
#
# The vendored files are ImGui's core, its SDL3 platform backend for input, and
# imgui_impl_brender, this tree's renderer backend. See LICENSE.txt.
#
add_library(brender-imgui STATIC
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

        # The BRender renderer backend.
        imgui_impl_brender.cpp
        imgui_impl_brender.h

        LICENSE.txt
)

target_compile_features(brender-imgui PUBLIC cxx_std_17)
target_include_directories(brender-imgui PUBLIC .)
target_link_libraries(brender-imgui PUBLIC BRender::Full SDL3::SDL3)
set_property(TARGET brender-imgui PROPERTY FOLDER "BRender/Contrib")
