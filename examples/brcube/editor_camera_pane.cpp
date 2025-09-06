#include "gl.h"
#include <brddi.h>
#include <brglrend.h>

#include "editor_camera_pane.hpp"

#include "ImGuizmo/ImGuizmo.h"

Editor::CameraPane::CameraPane(const char *id, br_pixelmap *screen)
    : m_is_focused(false), m_screen(screen), m_camera(nullptr)
{
    this->m_id.reset(BrResSprintf(nullptr, "Camera##%s", id));
    this->m_camera = BrEditorCamAllocate(nullptr);
}

Editor::CameraPane::~CameraPane()
{
    BrEditorCamFree(this->m_camera);
}

br_editor_camera *Editor::CameraPane::GetCamera() const noexcept
{
    return this->m_camera;
}

void Editor::CameraPane::Update(float dt) noexcept
{
    BrEditorCamUpdate(this->m_camera, dt);
}

bool Editor::CameraPane::IsFocused() const noexcept
{
    return this->m_is_focused;
}

void Editor::CameraPane::Draw(br_actor *world)
{
    bool      visible;
    br_actor *camroot = this->m_camera->actor;

    /* FIXME: Add a bool to get an "X" */
    visible = ImGui::Begin(this->m_id.get(), nullptr);

    if(camroot->prev != nullptr)
        BrActorRemoveNoRenumber(camroot);

    if(world != nullptr)
        BrActorAdd(world, camroot);

    if(visible)
        this->DrawInternal(world);

    if(camroot->prev != nullptr)
        BrActorRemoveNoRenumber(camroot);

    ImGui::End();
}

br_token CameraToScreenMatrix4(br_matrix4 *mat, br_actor *camera);

void Editor::CameraPane::DrawInternal(br_actor *world)
{
    GLuint tex;
    ImGui::BeginChild("GameRender");

    // br_matrix4 proj;
    // br_matrix34 view;
    // BrActorToScreenMatrix4(&proj, this->m_camera->camera, this->m_camera->camera);
    //
    // BrActorToActorMatrix34(&view, world, this->m_camera->camera);

    // FIXME: only if not dragging
    if(ImGui::BeginPopupContextWindow()) {
        if(ImGui::MenuItem("Reset"))
            BrEditorCamReset(this->m_camera);

        ImGui::EndPopup();
    }

    ImVec2 wsize = ImGui::GetWindowSize();
    this->MaybeResize(wsize.x, wsize.y);

    if((tex = GetPixelmapTexture(this->m_colour_buffer.get())) == 0)
        return;

    {
        BrRendererFrameBegin();
        BrPixelmapFill(this->m_colour_buffer.get(), BR_COLOUR_RGBA(0, 0, 0, 255));
        BrPixelmapFill(this->m_depth_buffer.get(), 0xFFFFFFFF);
        BrZbSceneRender(world, this->m_camera->camera, this->m_colour_buffer.get(), this->m_depth_buffer.get());
        BrRendererFrameEnd();
    }

    ImGui::Image(static_cast<ImTextureID>(tex), wsize, ImVec2(0, 1), ImVec2(1, 0));

    if((this->m_is_focused = ImGui::IsWindowFocused())) {
        CameraPane::CheckInput(this->m_camera);
    }

    ImGui::EndChild();
}

void Editor::CameraPane::MaybeResize(float width, float height) noexcept
{
    br_uint_16 iwidth  = static_cast<br_uint_16>(width);
    br_uint_16 iheight = static_cast<br_uint_16>(height);

    br_pixelmap *colour_buffer = this->m_colour_buffer.release();
    br_pixelmap *depth_buffer = this->m_depth_buffer.release();

    // TODO: error checking
    BrPixelmapResizeBuffers(this->m_screen, &colour_buffer, &depth_buffer);
    this->m_colour_buffer.reset(colour_buffer);
    this->m_depth_buffer.reset(depth_buffer);

    BrEditorCamSetViewportSize(this->m_camera, iwidth, iheight);
}

typedef struct KeyMap {
    ImGuiKey   key;
    br_uint_32 flag;
} KeyMap;

const static KeyMap keymap[] = {
    {.key = ImGuiKey_W,         .flag = BR_ECAMF_MOVE_FORWARD},
    {.key = ImGuiKey_S,         .flag = BR_ECAMF_MOVE_BACK   },
    {.key = ImGuiKey_A,         .flag = BR_ECAMF_MOVE_LEFT   },
    {.key = ImGuiKey_D,         .flag = BR_ECAMF_MOVE_RIGHT  },
    {.key = ImGuiKey_Q,         .flag = BR_ECAMF_MOVE_DOWN   },
    {.key = ImGuiKey_E,         .flag = BR_ECAMF_MOVE_UP     },
    {.key = ImGuiKey_LeftShift, .flag = BR_ECAMF_FASTER      },
};

typedef struct ButtonMap {
    ImGuiMouseButton button;
    br_uint_32       flag;
} ButtonMap;

const static ButtonMap buttonmap[] = {
    {.button = ImGuiMouseButton_Middle, .flag = BR_ECAMF_MODE_PAN },
    {.button = ImGuiMouseButton_Right,  .flag = BR_ECAMF_MODE_LOOK},
};

void Editor::CameraPane::CheckInput(br_editor_camera *cam) noexcept
{
    const ImGuiIO& io = ImGui::GetIO();

    for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
        const KeyMap *km = keymap + i;

        if(ImGui::IsKeyDown(km->key))
            cam->flags |= km->flag;
        else
            cam->flags &= ~km->flag;
    }

    // if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
    //     cam->flags |= BR_ECAMF_MODE_LOOK;
    // } else {
    //     cam->flags &= ~BR_ECAMF_MODE_LOOK;
    // }

    // if(ImGui::IsKeyPressed(ImGuiKey_Space)) {
    //     BrEditorCamReset(cam);
    // }

    for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
        const ButtonMap *bm = buttonmap + i;
        if(ImGui::IsMouseDown(bm->button)) {
            cam->flags |= bm->flag;
        } else {
            cam->flags &= ~bm->flag;
        }
    }

    cam->mx_rel += static_cast<br_int_32>(io.MouseDelta.x);
    cam->my_rel += static_cast<br_int_32>(io.MouseDelta.y);
    cam->mwheel_rel += io.MouseWheel;
}
