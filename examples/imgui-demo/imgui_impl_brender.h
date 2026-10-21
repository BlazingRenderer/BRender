/*
 * Dear ImGui renderer backend for BRender.
 *
 * Draws ImGui's draw data through the engine's immediate 2D API (Br2D*), so the
 * software, GL and Vulkan renderers all take the same call. A draw command's
 * texture handle is a br_material, which is also what a Br2D batch draws with.
 */
#ifndef IMGUI_IMPL_BRENDER_H
#define IMGUI_IMPL_BRENDER_H

#include <brender.h>

#include "imgui.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Create the font atlas as a br_pixelmap and register it with ImGui. Needs a
 * current ImGui context.
 */
bool ImGui_ImplBrender_Init(void);

/* Release the atlas. */
void ImGui_ImplBrender_Shutdown(void);

/*
 * Submit one frame's draw data into destination, between ImGui::Render() and the
 * buffer swap. The destination is a parameter rather than state the backend
 * caches, because a window resize replaces the colour buffer.
 */
void ImGui_ImplBrender_RenderDrawData(ImDrawData *draw_data, br_pixelmap *destination);

#ifdef __cplusplus
}
#endif

#endif /* IMGUI_IMPL_BRENDER_H */
