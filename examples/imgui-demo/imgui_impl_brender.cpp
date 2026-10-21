/*
 * Dear ImGui renderer backend for BRender.
 *
 * ImGui's renderer contract is a batch of indexed triangles in one vertex buffer
 * with a per-vertex colour, one texture and one clip rectangle per draw command,
 * alpha blending and no depth. Br2DBegin()/Br2DVertices()/Br2DEnd() take exactly
 * that - a textured, vertex-coloured triangle list in framebuffer pixels - and
 * turn it into the engine's immediate geometry path, which the software, GL and
 * Vulkan renderers already implement. So one ImGui draw command is one batch, and
 * there is nothing device-specific here.
 *
 * The texture is a br_pixelmap. ImGui's font atlas arrives as R,G,B,A bytes and
 * is converted to BR_PMT_RGBA_8888 (the engine's ARGB word, whose bytes in memory
 * run B,G,R,A) because that is the type the software rasteriser's sprite blocks
 * sample; glrend and vkrend take it as well, so the one atlas serves all three.
 */
#include "imgui_impl_brender.h"

#include <brender.h>

#include <math.h>
#include <algorithm>
#include <vector>

static br_pixelmap              *g_atlas    = NULL;
static br_material              *g_material = NULL;
static std::vector<br_vertex_2d> g_batch;

bool ImGui_ImplBrender_Init(void)
{
    unsigned char *pixels = NULL;
    int            width = 0, height = 0;

    IMGUI_CHECKVERSION();

    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_brender";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    if((g_atlas = BrPixelmapAllocate(BR_PMT_RGBA_8888, width, height, NULL, 0)) == NULL)
        return false;

    for(int y = 0; y < height; ++y) {
        const br_uint_8 *src = pixels + (br_size_t)y * width * 4;
        br_uint_8       *dst = (br_uint_8 *)g_atlas->pixels + (br_size_t)y * g_atlas->row_bytes;

        for(int x = 0; x < width; ++x, src += 4, dst += 4) {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = src[3];
        }
    }

    /*
     * The 2D batch binds the pixelmap's stored buffer rather than the pixelmap, so
     * the contents have to reach the device before the first batch draws.
     */
    BrMapUpdate(g_atlas, BR_MAPU_ALL);

    /*
     * The surface a batch draws with is a material, so the atlas gets one. A batch's
     * colours are its vertices', so PRELIT makes the surface read the geometry's
     * colours and turns modulation on, and PREALPHA reads the opacity the same way;
     * between them they are what makes a UI draw in its own colours rather than the
     * material's. ALWAYS_VISIBLE is cull NONE (a 2D triangle has no front), and colour
     * keying is off because the atlas is white where it is transparent.
     */
    if((g_material = BrMaterialAllocate("imgui-atlas")) == NULL)
        return false;

    g_material->flags = BR_MATF_PRELIT | BR_MATF_PREALPHA | BR_MATF_ALWAYS_VISIBLE | BR_MATF_DISABLE_COLOUR_KEY;
    g_material->mode  = BR_MATM_SHADING_MODE_GOURAUD | BR_MATM_DEPTH_TEST_AL | BR_MATM_BLEND_MODE_STANDARD | BR_MATM_MAP_WIDTH_LIMIT_CLAMP |
                       BR_MATM_MAP_HEIGHT_LIMIT_CLAMP;
    g_material->colour_map = g_atlas;

    BrMaterialUpdate(g_material, BR_MATU_ALL);

    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_material);

    return true;
}

void ImGui_ImplBrender_Shutdown(void)
{
    ImGuiIO& io = ImGui::GetIO();

    g_batch.clear();
    BrMaterialFree(g_material);
    BrPixelmapFree(g_atlas);
    g_material = NULL;
    g_atlas    = NULL;

    io.BackendRendererName = NULL;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
}

void ImGui_ImplBrender_RenderDrawData(ImDrawData *draw_data, br_pixelmap *destination)
{
    if(!draw_data->Valid || draw_data->CmdListsCount == 0)
        return;

    /*
     * Vertices, clip rectangles and the display size are in display points; the
     * batch is in framebuffer pixels. On a HiDPI window the two differ, and the
     * destination is the framebuffer.
     */
    const ImVec2 scale              = draw_data->FramebufferScale;
    const float  framebuffer_width  = draw_data->DisplaySize.x * scale.x;
    const float  framebuffer_height = draw_data->DisplaySize.y * scale.y;

    for(int n = 0; n < draw_data->CmdListsCount; ++n) {
        const ImDrawList *cmd_list = draw_data->CmdLists[n];

        for(int c = 0; c < cmd_list->CmdBuffer.Size; ++c) {
            const ImDrawCmd *cmd = &cmd_list->CmdBuffer[c];

            if(cmd->UserCallback == ImDrawCallback_ResetRenderState) {
                /* There is no cached render state to reset. */
                continue;
            }

            if(cmd->UserCallback != NULL) {
                cmd->UserCallback(cmd_list, cmd);
                continue;
            }

            if(cmd->ElemCount == 0)
                continue;

            const float left   = std::max(cmd->ClipRect.x * scale.x, 0.0f);
            const float top    = std::max(cmd->ClipRect.y * scale.y, 0.0f);
            const float right  = std::min(cmd->ClipRect.z * scale.x, framebuffer_width);
            const float bottom = std::min(cmd->ClipRect.w * scale.y, framebuffer_height);

            if(right <= left || bottom <= top)
                continue;

            const br_int_32 x0 = (br_int_32)left;
            const br_int_32 y0 = (br_int_32)top;
            const br_int_32 x1 = (br_int_32)ceilf(right);
            const br_int_32 y1 = (br_int_32)ceilf(bottom);

            const br_rectangle clip = {
                /* .x = */ x0,
                /* .y = */ y0,
                /* .w = */ x1 - x0,
                /* .h = */ y1 - y0,
            };

            const ImDrawIdx  *indices  = cmd_list->IdxBuffer.Data + cmd->IdxOffset;
            const ImDrawVert *vertices = cmd_list->VtxBuffer.Data + cmd->VtxOffset;

            g_batch.clear();
            g_batch.reserve(cmd->ElemCount);

            for(unsigned int i = 0; i < cmd->ElemCount; ++i) {
                const ImDrawVert& v = vertices[indices[i]];
                const ImU32       c = v.col;

                br_vertex_2d out;

                out.position.v[0] = BR_SCALAR(v.pos.x * scale.x);
                out.position.v[1] = BR_SCALAR(v.pos.y * scale.y);
                out.map.v[0]      = BR_SCALAR(v.uv.x);
                out.map.v[1]      = BR_SCALAR(v.uv.y);

                /* ImGui's colours are 0xAABBGGRR; br_colour is 0xAARRGGBB. */
                out.colour = BR_COLOUR_RGBA((c >> IM_COL32_R_SHIFT) & 0xFF, (c >> IM_COL32_G_SHIFT) & 0xFF, (c >> IM_COL32_B_SHIFT) & 0xFF,
                                            (c >> IM_COL32_A_SHIFT) & 0xFF);

                g_batch.push_back(out);
            }

            Br2DBegin(destination, (br_material *)(intptr_t)cmd->TextureId, &clip);
            Br2DVertices(g_batch.data(), (br_int_32)g_batch.size());
            Br2DEnd();
        }
    }
}
