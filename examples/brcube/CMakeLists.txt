project(brcube)

add_executable(brcube
        brcube.cpp

        editorcam.c
        editorcam_p.h
        editorcam.h

        editor.hpp
        editor.cpp

        editor_common.hpp

        editor_config.cpp
        editor_config.hpp

        editor_input.cpp

        editor_resources.cpp
        editor_resources.hpp

        editor_log_pane.cpp
        editor_log_pane.hpp

        editor_actor.cpp
        editor_actor.hpp

        editor_material.cpp
        editor_material.hpp

        editor_graph_pane.cpp
        editor_graph_pane.hpp

        editor_inspector.cpp
        editor_inspector.hpp

        editor_camera_pane.hpp
        editor_camera_pane.cpp

        editor_material_preview.cpp
        editor_material_preview.hpp

        editor_resources_pane.cpp
        editor_resources_pane.hpp

        editor_about_dialog.cpp
        editor_about_dialog.hpp

        imgui_config_brender.h
        gl.cpp
        gl.h

        imgui/imgui.h
        imgui/imgui_internal.h
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_demo.cpp

        imgui/misc/cpp/imgui_stdlib.cpp

        imgui/backends/imgui_impl_opengl3.cpp
        imgui/backends/imgui_impl_opengl3.h
        imgui/backends/imgui_impl_sdl3.cpp
        imgui/backends/imgui_impl_sdl3.h

        ImGuizmo/GraphEditor.cpp
        ImGuizmo/GraphEditor.h

        ImGuizmo/ImCurveEdit.cpp
        ImGuizmo/ImCurveEdit.h

        ImGuizmo/ImGradient.cpp
        ImGuizmo/ImGradient.h

        ImGuizmo/ImGuizmo.cpp
        ImGuizmo/ImGuizmo.h

        ImGuizmo/ImSequencer.cpp
        ImGuizmo/ImSequencer.h

        ImGuizmo/ImZoomSlider.h
)
target_compile_definitions(brcube PRIVATE "-DIMGUI_USER_CONFIG=\"${CMAKE_CURRENT_SOURCE_DIR}/imgui_config_brender.h\"")
target_link_libraries(brcube BRender::Full BRender::DDI)
target_include_directories(brcube PRIVATE imgui)

target_compile_features(brcube PRIVATE cxx_std_17)
