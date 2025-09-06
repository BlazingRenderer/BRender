#ifndef EDITOR_COMMON_HPP_
#define EDITOR_COMMON_HPP_

#include <type_traits>
#include <memory>
#include <string>
#include <chrono>
#include <brender.h>
#include <brddi.h>

#include "imgui.h"
#include "misc/cpp//imgui_stdlib.h"

namespace ImGui
{
bool InputUInt16(const char *label, uint16_t *v, int16_t step = 1, int16_t step_fast = 100,
                 ImGuiInputTextFlags extra_flags = 0);
} // namespace ImGui

namespace Editor
{

template <typename T /*, typename = std::enable_if_t<std::is_trivial_v<T>>*/>
struct br_res_deleter {
    using pointer = T *;
    void operator()(pointer p) const noexcept
    {
        BrResFree(reinterpret_cast<void *>(p));
    }
};
template <typename T>
using br_res_ptr = std::unique_ptr<T, br_res_deleter<T>>;

struct br_pixelmap_deleter {
    using pointer = br_pixelmap *;
    void operator()(pointer p) const noexcept
    {
        BrPixelmapFree(p);
    }
};
using br_pixelmap_ptr = std::unique_ptr<br_pixelmap, br_pixelmap_deleter>;


struct br_actor_deleter {
    using pointer = br_actor *;
    void operator()(pointer p) const noexcept
    {
        BrActorFree(p);
    }
};
using br_actor_ptr = std::unique_ptr<br_actor, br_actor_deleter>;


struct imgui_context_deleter {
    using pointer = ImGuiContext *;
    void operator()(pointer p)
    {
        ImGui::DestroyContext(p);
    }
};
using imgui_context_ptr = std::unique_ptr<ImGuiContext, imgui_context_deleter>;

struct LogEntry {
    using TimePoint = std::chrono::system_clock::time_point;

    TimePoint   timestamp;
    br_uint_64  sdl_ticks;
    br_uint_8   level;
    std::string component;
    std::string message;
};

GLuint GetPixelmapTexture(br_pixelmap *pm);

} // namespace Editor

#endif /* EDITOR_COMMON_HPP_ */
