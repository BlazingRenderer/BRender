/*
 * Dear ImGui renderer backend for BRender.
 *
 * ImGui's renderer contract is a batch of indexed triangles in one vertex buffer
 * with a per-vertex colour, one texture and one clip rectangle per draw command,
 * alpha blending and no depth. Br2DBegin()/Br2DVertices()/Br2DEnd() take exactly
 * that - a triangle list in framebuffer pixels - and turn it into the engine's
 * immediate geometry path, which the software, GL and Vulkan renderers already
 * implement. So one ImGui draw command is one batch, and there is nothing
 * device-specific here.
 *
 * The surface is a material. In colour mode its colour map is the font atlas,
 * converted to BR_PMT_RGBA_8888 because that is the type the software
 * rasteriser's sprite blocks sample, and the batch draws in its vertices' colours
 * with PRELIT and PREALPHA.
 *
 * Indexed mode is special-cased, because an INDEX_8 destination has no colour to
 * interpolate: it draws index values. The application prepares a palette and
 * hands it over; the atlas is converted once against that palette with index 0 as
 * the transparent key - which the indexed kernels mask on already - and a solid
 * fill becomes a one-texel INDEX_8 map of the fill's index, so both shapes are the
 * same textured index triangle.
 *
 * In both modes a draw command's texture handle is a br_material, which is also
 * what a Br2D batch draws with.
 */
#include "imgui_impl_brender.h"

#include <brender.h>

#include <math.h>
#include <algorithm>
#include <utility>
#include <vector>

static br_pixelmap *g_atlas    = NULL;
static br_material *g_material = NULL;
static br_pixelmap *g_palette  = NULL;
static bool         g_indexed  = false;

/* Indexed mode: one material per distinct fill colour, each with its own texel. */
static std::vector<std::pair<ImU32, br_material *>> g_fills;

static std::vector<br_vertex_2d> g_batch;

/* The engine's native word is ARGB and its bytes run B,G,R,A; ImGui's run R,G,B,A. */
static void atlas_convert_rgba(br_pixelmap *atlas, const unsigned char *pixels, int width, int height)
{
    for(int y = 0; y < height; ++y) {
        const br_uint_8 *src = pixels + (br_size_t)y * width * 4;
        br_uint_8       *dst = (br_uint_8 *)atlas->pixels + (br_size_t)y * atlas->row_bytes;

        for(int x = 0; x < width; ++x, src += 4, dst += 4) {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = src[3];
        }
    }
}

/* Nearest palette entry to a colour, over the palette the application prepared. */
static int palette_index(ImU32 colour)
{
    const int        r     = (colour >> IM_COL32_R_SHIFT) & 0xFF;
    const int        g     = (colour >> IM_COL32_G_SHIFT) & 0xFF;
    const int        b     = (colour >> IM_COL32_B_SHIFT) & 0xFF;
    const br_colour *entry = (const br_colour *)g_palette->pixels;
    int              best = 0, best_distance = INT32_MAX;

    for(int i = 0; i < g_palette->height; ++i) {
        const int dr       = (int)BR_RED(entry[i]) - r;
        const int dg       = (int)BR_GRN(entry[i]) - g;
        const int db       = (int)BR_BLU(entry[i]) - b;
        const int distance = dr * dr + dg * dg + db * db;

        if(distance < best_distance) {
            best_distance = distance;
            best          = i;
        }
    }

    return best;
}

/*
 * Indexed mode's atlas: the glyphs are white with alpha, so a texel becomes the
 * nearest palette grey to the alpha - the antialiasing is baked into the index
 * instead of being blended - and anything below a quarter covered is the key.
 */
static void atlas_convert_indexed(br_pixelmap *atlas, const unsigned char *pixels, int width, int height)
{
    const br_colour *entry = (const br_colour *)g_palette->pixels;

    for(int y = 0; y < height; ++y) {
        const br_uint_8 *src = pixels + (br_size_t)y * width * 4;
        br_uint_8       *dst = (br_uint_8 *)atlas->pixels + (br_size_t)y * atlas->row_bytes;

        for(int x = 0; x < width; ++x, src += 4, dst += 1) {
            const int alpha = src[3];

            if(alpha < 64) {
                *dst = 0;
                continue;
            }

            /* White over black is the grey the palette's ramp carries. */
            *dst = (br_uint_8)palette_index(BR_COLOUR_RGB(alpha, alpha, alpha));
        }
    }
}

/*
 * The one material shape this backend uses: unculled, no depth, colour keying off,
 * and whatever else the mode needs - colour mode takes its colours and opacity from
 * the vertices, indexed mode has neither.
 */
static br_material *new_material(const char *identifier, br_pixelmap *map, br_uint_32 flags)
{
    br_material *material = BrMaterialAllocate(identifier);

    if(material == NULL)
        return NULL;

    material->flags      = flags | BR_MATF_ALWAYS_VISIBLE | BR_MATF_DISABLE_COLOUR_KEY;
    material->mode       = BR_MATM_SHADING_MODE_GOURAUD | BR_MATM_DEPTH_TEST_AL;
    material->colour_map = map;

    BrMaterialUpdate(material, BR_MATU_ALL);

    return material;
}

/*
 * Indexed mode has no blending, so a fill's alpha has to go somewhere: composite it
 * over the window background, which is what it is nearly always drawn on, and let
 * the nearest palette entry carry the result. A fully opaque fill is unchanged.
 */
static ImU32 fill_composite(ImU32 colour)
{
    const ImVec4& bg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    const int     a  = (colour >> IM_COL32_A_SHIFT) & 0xFF;
    const int     r  = (colour >> IM_COL32_R_SHIFT) & 0xFF;
    const int     g  = (colour >> IM_COL32_G_SHIFT) & 0xFF;
    const int     b  = (colour >> IM_COL32_B_SHIFT) & 0xFF;
    const int     ia = 255 - a;

    return IM_COL32((r * a + (int)(bg.x * 255.0f) * ia) / 255, (g * a + (int)(bg.y * 255.0f) * ia) / 255,
                    (b * a + (int)(bg.z * 255.0f) * ia) / 255, 255);
}

/* Indexed mode's solid fill: a one-texel map holding the fill's palette index. */
static br_material *fill_material(ImU32 colour)
{
    for(const auto& fill : g_fills) {
        if(fill.first == colour)
            return fill.second;
    }

    br_pixelmap *map      = BrPixelmapAllocate(BR_PMT_INDEX_8, 1, 1, NULL, 0);
    br_material *material = NULL;

    if(map != NULL) {
        /* BrPixelmapAllocate() references a supplied buffer, so this map owns its texel. */
        *(br_uint_8 *)map->pixels = (br_uint_8)palette_index(fill_composite(colour));
        BrMapUpdate(map, BR_MAPU_ALL);
        material = new_material("imgui-fill", map, 0);
    }

    if(material != NULL)
        g_fills.push_back(std::make_pair(colour, material));

    return material;
}

bool ImGui_ImplBrender_Init(br_pixelmap *palette)
{
    unsigned char *pixels = NULL;
    int            width = 0, height = 0;

    IMGUI_CHECKVERSION();

    ImGuiIO& io            = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_brender";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_palette = palette;
    g_indexed = palette != NULL;

    br_pixelmap *atlas = BrPixelmapAllocate(g_indexed ? BR_PMT_INDEX_8 : BR_PMT_RGBA_8888, width, height, NULL, 0);

    if(atlas == NULL)
        return false;

    if(g_indexed)
        atlas_convert_indexed(atlas, pixels, width, height);
    else
        atlas_convert_rgba(atlas, pixels, width, height);

    /*
     * The batch binds the pixelmap's stored buffer rather than the pixelmap, so the
     * contents have to reach the device before the first batch draws.
     */
    BrMapUpdate(atlas, BR_MAPU_ALL);

    const br_uint_32 flags = g_indexed ? 0 : (BR_MATF_PRELIT | BR_MATF_PREALPHA);

    if((g_material = new_material("imgui-atlas", atlas, flags)) == NULL) {
        BrPixelmapFree(atlas);
        return false;
    }

    g_atlas = atlas;
    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_material);

    return true;
}

void ImGui_ImplBrender_Shutdown(void)
{
    ImGuiIO& io = ImGui::GetIO();

    g_batch.clear();

    for(auto& fill : g_fills) {
        BrPixelmapFree(fill.second->colour_map);
        BrMaterialFree(fill.second);
    }

    g_fills.clear();

    BrMaterialFree(g_material);
    BrPixelmapFree(g_atlas);
    g_material = NULL;
    g_atlas    = NULL;
    g_palette  = NULL;
    g_indexed  = false;

    io.BackendRendererName = NULL;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasVtxOffset;
}

static void batch_vertex(const ImDrawVert *v, const ImVec2& scale)
{
    br_vertex_2d out = {};
    const ImU32  c   = v->col;

    out.position.v[0] = BR_SCALAR(v->pos.x * scale.x);
    out.position.v[1] = BR_SCALAR(v->pos.y * scale.y);
    out.map.v[0]      = BR_SCALAR(v->uv.x);
    out.map.v[1]      = BR_SCALAR(v->uv.y);

    /* ImGui's colours are 0xAABBGGRR; br_colour is 0xAARRGGBB. */
    out.colour = BR_COLOUR_RGBA((c >> IM_COL32_R_SHIFT) & 0xFF, (c >> IM_COL32_G_SHIFT) & 0xFF, (c >> IM_COL32_B_SHIFT) & 0xFF,
                                (c >> IM_COL32_A_SHIFT) & 0xFF);

    g_batch.push_back(out);
}

static void batch_flush(br_pixelmap *destination, br_material *material, const br_rectangle *clip)
{
    if(g_batch.empty())
        return;

    Br2DBegin(destination, material, clip);
    Br2DVertices(g_batch.data(), (br_int_32)g_batch.size());
    Br2DEnd();

    g_batch.clear();
}

/*
 * A triangle whose three vertices share one texture coordinate is a solid shape:
 * ImGui draws those from a single texel of the font atlas, and the vertex colour is
 * what actually matters. A glyph samples a real region, so it takes the atlas.
 *
 * The two cannot be told apart per command - ImGui batches a whole window's quads
 * into one - so indexed mode walks the command a triangle at a time and flushes a
 * batch whenever the material changes.
 */
static bool triangle_is_solid(const ImDrawVert *a, const ImDrawVert *b, const ImDrawVert *c)
{
    return a->uv.x == b->uv.x && a->uv.y == b->uv.y && b->uv.x == c->uv.x && b->uv.y == c->uv.y;
}

static void render_command_indexed(const ImDrawCmd *cmd, const ImDrawVert *vertices, const ImDrawIdx *indices, br_pixelmap *destination,
                                   const br_rectangle *clip, const ImVec2& scale)
{
    br_material *current = NULL;

    g_batch.clear();

    for(unsigned int i = 0; i + 2 < cmd->ElemCount; i += 3) {
        const ImDrawVert *a = vertices + indices[i + 0];
        const ImDrawVert *b = vertices + indices[i + 1];
        const ImDrawVert *c = vertices + indices[i + 2];
        br_material      *material;

        if(triangle_is_solid(a, b, c))
            material = fill_material(a->col);
        else
            material = g_material;

        if(material != current) {
            batch_flush(destination, current, clip);
            current = material;
        }

        if(current == NULL)
            continue;

        batch_vertex(a, scale);
        batch_vertex(b, scale);
        batch_vertex(c, scale);
    }

    batch_flush(destination, current, clip);
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

            if(g_indexed) {
                render_command_indexed(cmd, vertices, indices, destination, &clip, scale);
                continue;
            }

            br_material *material = (br_material *)(intptr_t)cmd->TextureId;

            if(material == NULL)
                continue;

            g_batch.clear();

            for(unsigned int i = 0; i < cmd->ElemCount; ++i)
                batch_vertex(vertices + indices[i], scale);

            batch_flush(destination, material, &clip);
        }
    }
}
