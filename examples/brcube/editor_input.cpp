//#include "editor.hpp"
//
//typedef struct KeyMap {
//    SDL_KeyCode sdl_key;
//    ImGuiKey    key;
//    br_uint_32  flag;
//} KeyMap;
//
//const static KeyMap keymap[] = {
//    {.sdl_key = SDLK_w,      .key = ImGuiKey_W,         .flag = BR_ECAMF_MOVE_FORWARD},
//    {.sdl_key = SDLK_s,      .key = ImGuiKey_S,         .flag = BR_ECAMF_MOVE_BACK   },
//    {.sdl_key = SDLK_a,      .key = ImGuiKey_A,         .flag = BR_ECAMF_MOVE_LEFT   },
//    {.sdl_key = SDLK_d,      .key = ImGuiKey_D,         .flag = BR_ECAMF_MOVE_RIGHT  },
//    {.sdl_key = SDLK_q,      .key = ImGuiKey_Q,         .flag = BR_ECAMF_MOVE_DOWN   },
//    {.sdl_key = SDLK_e,      .key = ImGuiKey_E,         .flag = BR_ECAMF_MOVE_UP     },
//    {.sdl_key = SDLK_LSHIFT, .key = ImGuiKey_LeftShift, .flag = BR_ECAMF_FASTER      },
//};
//
//typedef struct ButtonMap {
//    Uint8            sdl_button;
//    ImGuiMouseButton button;
//    br_uint_32       flag;
//} ButtonMap;
//
//const static ButtonMap buttonmap[] = {
//    {.sdl_button = SDL_BUTTON_MIDDLE, .button = ImGuiMouseButton_Middle, .flag = BR_ECAMF_MODE_PAN },
//    {.sdl_button = SDL_BUTTON_RIGHT,  .button = ImGuiMouseButton_Right,  .flag = BR_ECAMF_MODE_LOOK},
//};
//
//void Editor::Editor::CheckCameraInput(br_editor_camera *cam)
//{
//    const ImGuiIO& io = ImGui::GetIO();
//
//    for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
//        const KeyMap *km = keymap + i;
//
//        if(ImGui::IsKeyDown(km->key))
//            cam->flags |= km->flag;
//        else
//            cam->flags &= ~km->flag;
//    }
//
//    if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
//        cam->flags |= BR_ECAMF_MODE_LOOK;
//    } else {
//        cam->flags &= ~BR_ECAMF_MODE_LOOK;
//    }
//
//    if(ImGui::IsKeyPressed(ImGuiKey_Space)) {
//        BrEditorCamReset(cam);
//    }
//
//    for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
//        const ButtonMap *bm = buttonmap + i;
//        if(ImGui::IsMouseDown(bm->button)) {
//            cam->flags |= bm->flag;
//        } else {
//            cam->flags &= ~bm->flag;
//        }
//    }
//
//    cam->mx_rel += static_cast<br_int_32>(io.MouseDelta.x);
//    cam->my_rel += static_cast<br_int_32>(io.MouseDelta.y);
//    cam->mwheel_rel += io.MouseWheel;
//}
//
//bool Editor::Editor::ProcessCameraEvent(br_editor_camera *cam, const SDL_Event *event)
//{
//    switch(event->type) {
//        case SDL_KEYDOWN:
//        case SDL_KEYUP:
//            for(size_t i = 0; i < BR_ASIZE(keymap); ++i) {
//                const KeyMap *km = keymap + i;
//                if(km->sdl_key != event->key.keysym.sym)
//                    continue;
//
//                if(event->type == SDL_KEYDOWN)
//                    cam->flags |= km->flag;
//                else if(event->type == SDL_KEYUP)
//                    cam->flags &= ~km->flag;
//            }
//
//            if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_LCTRL) {
//                cam->flags |= BR_ECAMF_MODE_LOOK;
//            }
//
//            if(event->type == SDL_KEYUP && event->key.keysym.sym == SDLK_LCTRL) {
//                cam->flags &= ~BR_ECAMF_MODE_LOOK;
//            }
//
//            if(event->type == SDL_KEYDOWN && event->key.keysym.sym == SDLK_SPACE) {
//                BrMatrix34Identity(&cam->actor->t.t.mat);
//                BrMatrix34Identity(&cam->camera->t.t.mat);
//            }
//
//            return true;
//
//        case SDL_MOUSEBUTTONDOWN:
//        case SDL_MOUSEBUTTONUP:
//            for(size_t i = 0; i < BR_ASIZE(buttonmap); ++i) {
//                const ButtonMap *bm = buttonmap + i;
//                if(event->button.button != bm->sdl_button)
//                    continue;
//
//                if(event->button.state == SDL_PRESSED) {
//                    cam->flags |= bm->flag;
//                } else if(event->button.state == SDL_RELEASED) {
//                    cam->flags &= ~bm->flag;
//                }
//
//                break;
//            }
//
//            return true;
//
//        case SDL_MOUSEMOTION:
//            /* Accumulate mouse motion until next Update(). */
//            cam->mx_rel += event->motion.xrel;
//            cam->my_rel += event->motion.yrel;
//            return true;
//
//        case SDL_MOUSEWHEEL:
//            cam->mwheel_rel += event->wheel.preciseY;
//            return true;
//
//        default:
//            break;
//    }
//
//    return false;
//}
