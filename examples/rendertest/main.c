/*
 * rendertest - render the scene fixtures and check them against a stored
 * reference.
 *
 * One device per invocation. The device is fixed before the demo loop starts,
 * so a run covers every scene under that one device; run it again with a
 * different --device to cover another.
 *
 * The checksums are the same FNV-1a the gltfview benchmark reports. A hardware
 * context is read back from the colour buffer's offscreen texture, the software
 * path from the colour buffer itself; the scene setup mirrors gltfview's so
 * that the two agree pixel for pixel.
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <brender.h>
#include <brddi.h>
#include <brsdl3dev.h>

#include "brdemo.h"

/* ------------------------------------------------------------------ */
/* GL entry points; SDL hands us the addresses, we don't use glad.     */
/* ------------------------------------------------------------------ */

typedef void (*rt_pfn_ReadPixels)(int, int, int, int, unsigned int, unsigned int, void *);
typedef void (*rt_pfn_ReadBuffer)(unsigned int);
typedef void (*rt_pfn_BindTexture)(unsigned int, unsigned int);
typedef void (*rt_pfn_GetTexImage)(unsigned int, int, unsigned int, unsigned int, void *);
typedef void (*rt_pfn_PixelStorei)(unsigned int, int);
typedef const unsigned char *(*rt_pfn_GetString)(unsigned int);

#define RT_GL_BACK           0x0405u
#define RT_GL_PACK_ALIGNMENT 0x0CF5u
#define RT_GL_RGBA           0x1908u
#define RT_GL_TEXTURE_2D     0x0DE1u
#define RT_GL_UNSIGNED_BYTE  0x1401u
#define RT_GL_RENDERER       0x1F01u

/* ------------------------------------------------------------------ */
/* Configuration and results.                                         */
/* ------------------------------------------------------------------ */

/*
 * RT_MAX_SCENES is the most fixtures one run holds: the default set plus any
 * named on the command line. RT_MAX_ENTRIES is the size of the reference table
 * a run loads, which bounds a bless as well as a comparison. A run that needs
 * more of either fails rather than dropping what does not fit.
 *
 * The key names the device, the driver, the floating-point class, the pixel
 * type, the depth mode and the fixture, so a configuration costs one entry per
 * fixture in each class. 4096 holds every configuration the harness can key
 * across 128 fixtures in both classes, with room for the fixture set to grow;
 * 512 could not take the next blessing round. One file is what --reference,
 * --bless and the merge in rt_write_reference() are written against.
 *
 * RT_MAX_SCENES is the same problem one step out: 64 left no room for the
 * fixture set to grow. 256 is the room the cap was raised to.
 *
 * RT_MAX_KEY bounds a reference key and the rt_expect that holds one. A key
 * that would not fit is refused rather than truncated.
 */
#define RT_MAX_SCENES  256
#define RT_MAX_ENTRIES 4096
#define RT_MAX_KEY     192
#define RT_MAX_LINE    1024

/*
 * The floating-point evaluation class of this build.
 *
 * Both classes implement IEEE-754; what differs is the width float and double
 * expressions are evaluated at. i686, and MSVC's 32-bit x87, keep them in
 * 80-bit x87 registers and round only on a store (FLT_EVAL_METHOD 2); x86-64,
 * aarch64 and a 32-bit build with -mfpmath=sse evaluate each operation at the
 * width its type declares (FLT_EVAL_METHOD 0). The two round differently, and
 * the difference reaches the frame through softrend's setup and through core's
 * scene setup, so the software rasteriser and glrend alike produce different
 * pixels from the same scene - a key has to say which class produced the
 * entry.
 *
 * MSVC defines no __FLT_EVAL_METHOD__ before C11; _M_IX86_FP is 0 when x87 is
 * in use rather than SSE.
 */
#if (defined(__FLT_EVAL_METHOD__) && __FLT_EVAL_METHOD__ == 2) || (defined(_M_IX86_FP) && _M_IX86_FP == 0)
#define RT_FP_CLASS "x87"
#else
#define RT_FP_CLASS "declared"
#endif

/* Filled in by main() before the demo runs. */
static const char *rt_cfg_device       = "glrend";
static br_uint_8   rt_cfg_pm_type      = BR_PMT_INDEX_8;
static int         rt_cfg_width        = 320;
static int         rt_cfg_height       = 240;
static int         rt_cfg_warmup       = 2;
static int         rt_cfg_no_depth     = 0;
static int         rt_cfg_bless        = 0;
static int         rt_cfg_verbose      = 0;
static const char *rt_cfg_reference    = RT_REFERENCE_DEFAULT;
static const char *rt_cfg_ref_driver   = NULL; /* reference driver token; NULL: this binary's own */
static const char *rt_cfg_scene_dir    = RT_SCENE_DIR_DEFAULT;
static char        rt_cfg_ppm_dir[512] = "";
static const char *rt_cfg_scenes[RT_MAX_SCENES];
static int         rt_cfg_nscenes = 0;

typedef struct rt_scene {
    char name[64];
    char path[512];
} rt_scene;

typedef struct rt_result {
    char       scene[64];
    br_uint_64 hash;
    br_uint_32 coverage;
    br_uint_32 total;
} rt_result;

typedef struct rt_expect {
    char       key[RT_MAX_KEY];
    br_uint_64 hash;
} rt_expect;

typedef struct rt_state {
    /* Fixed before the demo runs. */
    const char *device;
    br_uint_8   pm_type;
    int         width;
    int         height;
    int         warmup;
    int         no_depth;
    int         bless;
    int         verbose;

    char reference[512];
    char ppm_dir[512];          /* empty: don't export */
    char driver[128];           /* short driver token, e.g. "llvmpipe" */
    char reference_driver[128]; /* empty: look up under driver above */

    rt_scene *scenes;
    int       nscenes;
    int       scene_index;

    /* Per-scene. */
    br_actor *scene_root;
    int       frame_index;

    /*
     * Grey shade ramp used only by the software PPM export. Built per scene
     * from the materials' index_base/index_range; NULL on the hardware path.
     */
    br_pixelmap *shade_palette;

    /* Output. */
    rt_result *results;
    int        nresults;
    int        compared; /* results scored against a stored entry, not NO-REFERENCE */

    rt_expect *expects;
    int        nexpects;

    SDL_Window        *window;
    rt_pfn_ReadPixels  ReadPixels;
    rt_pfn_ReadBuffer  ReadBuffer;
    rt_pfn_BindTexture BindTexture;
    rt_pfn_GetTexImage GetTexImage;
    rt_pfn_PixelStorei PixelStorei;
    rt_pfn_GetString   GetString;

    int env_ok;
    int failures;
} rt_state;

static int g_failed;

/* ------------------------------------------------------------------ */
/* Checksum: identical to gltfview's benchmark.                       */
/* ------------------------------------------------------------------ */

static br_uint_32 rt_pixel_value(const br_uint_8 *row, br_int_32 x, br_uint_32 bits)
{
    if(bits >= 8) {
        const br_uint_8 *p = row + (br_size_t)x * (bits / 8);
        br_uint_32       v = 0;

        for(br_uint_32 c = 0; c < bits / 8; ++c)
            v |= (br_uint_32)p[c] << (8 * c);

        return v;
    }

    switch(bits) {
        case 4:
            return (row[x / 2] >> (4 * (1 - (x % 2)))) & 0xf;
        case 2:
            return (row[x / 4] >> (2 * (3 - (x % 4)))) & 0x3;
        case 1:
            return (row[x / 8] >> (7 - (x % 8))) & 0x1;
        default:
            return 0;
    }
}

/*
 * BrPixelmapPixelSize() is in *bits*; a row is not width * pixel size bytes.
 * See gltfview/bench.c for the story of getting this wrong.
 */
static br_uint_64 rt_hash_pixelmap(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered)
{
    br_uint_32 bits      = BrPixelmapPixelSize(pm);
    br_int_32  row_bytes = (br_int_32)(((br_uint_64)pm->width * bits + 7) / 8);
    br_int_32  stride    = pm->row_bytes != 0 ? pm->row_bytes : row_bytes;

    for(br_int_32 y = 0; y < pm->height; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)pm->pixels + (br_size_t)y * stride;

        for(br_int_32 i = 0; i < row_bytes; ++i) {
            hash ^= row[i];
            hash *= 1099511628211ull;
        }

        if(covered == NULL)
            continue;

        for(br_int_32 x = 0; x < pm->width; ++x) {
            if(rt_pixel_value(row, x, bits) != 0)
                ++*covered;
        }
    }

    return hash;
}

/*
 * Read the rendered frame back into px as tightly-packed RGBA8, bottom row
 * first, which is the order glReadPixels and glGetTexImage both return.
 *
 * glrend renders into the colour buffer's own FBO and only blits it to the
 * window at swap time, which is after this runs, so on glrend the FBO texture
 * is the only correct source - reading GL_BACK there gives whatever the swap
 * left behind, i.e. undefined. glrend1x is GL 1.x with no FBOs: it renders
 * straight into the default framebuffer and exposes no texture token, so fall
 * back to the window back buffer, which is exactly right for it.
 */
static int rt_readback_gl(rt_state *st, br_demo *demo, int w, int h, br_uint_8 *px)
{
    br_uint_32 tex = 0;

    if(st->PixelStorei != NULL)
        st->PixelStorei(RT_GL_PACK_ALIGNMENT, 1);

    if(st->BindTexture != NULL && st->GetTexImage != NULL && ObjectQuery(demo->colour_buffer, &tex, BRT_OPENGL_TEXTURE_U32) == BRE_OK && tex != 0) {
        st->BindTexture(RT_GL_TEXTURE_2D, tex);
        st->GetTexImage(RT_GL_TEXTURE_2D, 0, RT_GL_RGBA, RT_GL_UNSIGNED_BYTE, px);
        return 1;
    }

    if(st->ReadPixels == NULL)
        return 0;

    if(st->ReadBuffer != NULL)
        st->ReadBuffer(RT_GL_BACK);

    st->ReadPixels(0, 0, w, h, RT_GL_RGBA, RT_GL_UNSIGNED_BYTE, px);

    return 1;
}

static void rt_checksum_gl(rt_state *st, br_demo *demo, br_uint_64 *out_hash, br_uint_32 *out_covered, br_uint_32 *out_total)
{
    int        w = 0, h = 0;
    br_uint_8 *px;
    br_uint_64 hash    = 1469598103934665603ull;
    br_uint_32 covered = 0;

    SDL_GetWindowSizeInPixels(st->window, &w, &h);

    if(w <= 0 || h <= 0) {
        *out_hash    = hash;
        *out_covered = *out_total = 0;
        return;
    }

    px = BrResAllocate(NULL, (br_size_t)w * h * 4, BR_MEMORY_APPLICATION);

    if(!rt_readback_gl(st, demo, w, h, px)) {
        BrResFree(px);
        *out_hash    = hash;
        *out_covered = *out_total = 0;
        return;
    }

    for(br_uint_32 i = 0; i < (br_uint_32)w * (br_uint_32)h; ++i) {
        const br_uint_8 *p = px + (br_size_t)i * 4;

        for(int c = 0; c < 4; ++c) {
            hash ^= p[c];
            hash *= 1099511628211ull;
        }

        if(p[0] || p[1] || p[2])
            ++covered;
    }

    BrResFree(px);

    *out_hash    = hash;
    *out_covered = covered;
    *out_total   = (br_uint_32)w * (br_uint_32)h;
}

static void rt_checksum_software(rt_state *st, br_demo *demo, br_uint_64 *out_hash, br_uint_32 *out_covered, br_uint_32 *out_total)
{
    br_pixelmap *pm      = demo->colour_buffer;
    br_pixelmap *pal     = NULL;
    br_uint_64   hash    = 1469598103934665603ull;
    br_uint_32   covered = 0;

    if(pm == NULL || pm->pixels == NULL || pm->width == 0 || pm->height == 0 || BrPixelmapPixelSize(pm) == 0) {
        *out_hash    = hash;
        *out_covered = *out_total = 0;
        return;
    }

    hash = rt_hash_pixelmap(hash, pm, &covered);

    if((pal = pm->map) != NULL && pal->pixels != NULL && pal->width != 0 && pal->height != 0 && BrPixelmapPixelSize(pal) != 0)
        hash = rt_hash_pixelmap(hash, pal, NULL);

    *out_hash    = hash;
    *out_covered = covered;
    *out_total   = (br_uint_32)pm->width * (br_uint_32)pm->height;
}

/* ------------------------------------------------------------------ */
/* Scene construction.                                                */
/* ------------------------------------------------------------------ */

static br_actor *rt_find_camera(br_actor *a)
{
    if(a->type == BR_ACTOR_CAMERA)
        return a;

    for(br_actor *c = a->children; c != NULL; c = c->next) {
        br_actor *r = rt_find_camera(c);

        if(r != NULL)
            return r;
    }

    return NULL;
}

/*
 * A material's lookup tables (index_shade/index_blend/index_fog/screendoor) are
 * BR_PMT_INDEX_8 pixelmaps with no palette: their pixels are indices into the
 * table itself, not colours. There is no palette to requantise through and the
 * rasterisers read the table's bytes directly, so they must be left untouched.
 */
static br_boolean rt_is_lookup_table(const br_pixelmap *pm)
{
    return pm->type == BR_PMT_INDEX_8 && pm->map == NULL;
}

/*
 * Build an optimal CLUT for every pixelmap in the scene; gltfview does this.
 *
 * The loader leaves a NULL entry for an image it could not obtain - a file the
 * URI names but that is not there, or that it cannot read; an image that is
 * present but broken refuses the load instead - so a table it filled can have
 * holes in it.
 */
static br_pixelmap *rt_build_clut(br_pixelmap *const *maps, size_t nmaps)
{
    br_pixelmap *clut;

    clut = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL);
    BrPixelmapPixelSet(clut, 0, 0, 0);

    BrQuantBegin();
    for(size_t i = 0; i < nmaps; ++i) {
        br_pixelmap *pm = maps[i];

        if(pm == NULL || rt_is_lookup_table(pm))
            continue;

        for(int y = 0; y < pm->height; y++) {
            for(int x = 0; x < pm->width; x++) {
                br_colour col = BrPixelmapPixelGet(pm, -pm->origin_x + x, -pm->origin_y + y);

                br_uint_8 rgb[3] = {
                    [0] = BR_RED(col),
                    [1] = BR_GRN(col),
                    [2] = BR_BLU(col),
                };

                BrQuantAddColours(rgb, 1);
            }
        }
    }

    BrQuantMakePalette(1, 255, clut);
    BrQuantEnd();

    return clut;
}

/*
 * Build the grey ramp the software PPM export applies to an INDEX_8 frame.
 *
 * The software indexed path does not store colours: softrend's indexed
 * lighting (drivers/softrend/light8.c) scales the surface intensity into
 * [index_base, index_base + index_range] and the rasteriser writes that shade
 * index straight into the colour buffer. The index is only meaningful through
 * a palette that maps the band to a ramp, and an untextured scene leaves the
 * CLUT built above completely empty (rt_build_clut() only sees pixelmaps).
 *
 * Read index_base/index_range from the materials, the same fields the indexed
 * lighting path consumes, and lay a linear grey ramp across the band they
 * cover. This makes the frame readable as a grey shape - a bpp8 software
 * export is a SHAPE only, hue is lost, so it is not colour-comparable with
 * bpp15/16/24. This is deliberately not a real CLUT: a faithful colour export
 * needs a palette built from the scene's textures/materials and a re-bless of
 * the reference.
 */
static br_pixelmap *rt_build_shade_palette(br_material *const *mats, size_t nmats)
{
    br_pixelmap *pal;
    int          base = 256, top = -1;

    for(size_t i = 0; i < nmats; ++i) {
        int b = mats[i]->index_base;
        int r = mats[i]->index_range;

        if(b < 0)
            b = 0;

        if(r <= 0)
            continue;

        if(b < base)
            base = b;

        if(b + r > top)
            top = b + r;
    }

    /*
     * No usable material range (or materials absent): fall back to the whole
     * index space so the shape is still visible.
     */
    if(top <= base) {
        base = 0;
        top  = 255;
    }

    if((pal = BrPixelmapAllocate(BR_PMT_RGBX_888, 1, 256, NULL, BR_PMAF_NORMAL)) == NULL)
        return NULL;

    for(int i = 0; i < 256; ++i) {
        br_uint_8 grey;

        if(i < base)
            grey = 0;
        else if(i >= top)
            grey = 255;
        else
            grey = (br_uint_8)(255 * (i - base + 1) / (top - base + 1));

        BrPixelmapPixelSet(pal, 0, i, BR_COLOUR_RGB(grey, grey, grey));
    }

    return pal;
}

static br_error rt_load_scene(rt_state *st, br_demo *demo, int index)
{
    const rt_scene *s = &st->scenes[index];
    br_fmt_results *results;
    br_actor       *root;

    br_gltf_options opts = {
        .pm_type = demo->colour_buffer->type,
    };

    if(demo->hw_accel) {
        opts.pm_type = BR_PMT_RGBA_8888;
    } else {
        switch(demo->colour_buffer->type) {
            case BR_PMT_INDEX_8:
                opts.pm_type = BR_PMT_RGBA_8888;
                break;
            case BR_PMT_RGB_565:
            case BR_PMT_RGB_555:
                opts.pm_type = BR_PMT_INDEX_8;
                break;
            default:
                break;
        }
    }

    if((results = BrFmtGLTFActorLoadMany(s->path, &opts)) == NULL || results->nactors == 0) {
        BrLogError("RT", "Failed to load %s", s->path);
        return BRE_FAIL;
    }

    if((root = BrActorAllocate(BR_ACTOR_NONE, NULL)) == NULL) {
        BrLogError("RT", "Failed to allocate scene root");
        return BRE_FAIL;
    }

    root->identifier = BrResStrDup(root, s->name);
    BrActorAdd(demo->world, root);
    st->scene_root = root;

    for(br_size_t i = 0; i < results->nactors; ++i)
        BrActorAdd(root, results->actors[i]);

    br_pixelmap *clut = rt_build_clut(results->pixelmaps, results->npixelmaps);
    BrPixelmapPaletteSet(demo->colour_buffer, clut);

    if(!demo->hw_accel && demo->colour_buffer->type == BR_PMT_INDEX_8)
        st->shade_palette = rt_build_shade_palette(results->materials, results->nmaterials);

    for(size_t i = 0; i < results->npixelmaps; ++i) {
        br_pixelmap *pm = results->pixelmaps[i];

        if(pm == NULL)
            continue;

        if(demo->colour_buffer->type == BR_PMT_INDEX_8 && !rt_is_lookup_table(pm)) {
            const br_pixelmap_convert_options cvtopts = {
                .index_alpha_threshold = 0,
                .target_clut           = clut,
            };
            br_pixelmap *tmp = BrPixelmapConvert(pm, BR_PMT_INDEX_8, &cvtopts);

            pm->type      = BR_PMT_INDEX_8;
            pm->row_bytes = tmp->row_bytes;

            BrResFree(pm->pixels);
            pm->pixels = BrResAdd(pm, BrResRemove(tmp->pixels));

            if(pm->map != NULL)
                BrResFree(pm->map);

            pm->map = clut;

            BrPixelmapFree(tmp);
        } else {
            if(pm->map != NULL)
                BrTableUpdate(pm->map, BR_TABU_ALL);
        }

        BrMapUpdate(pm, BR_MAPU_ALL);
    }

    for(br_size_t i = 0; i < results->nmaterials; ++i)
        BrMaterialUpdate(results->materials[i], BR_MATU_ALL);

    for(br_size_t i = 0; i < results->nmodels; ++i) {
        if(results->models[i] != NULL)
            BrModelUpdate(results->models[i], BR_MODU_ALL);
    }

    demo->camera = rt_find_camera(root);

    if(demo->camera == NULL) {
        BrLogError("RT", "%s has no camera", s->name);
        return BRE_FAIL;
    }

    /*
     * The order table's depth bounds come from the camera; without them it
     * divides by a zero z range. The aspect has to be set per scene rather
     * than left to BrDemoDefaultOnResize(), which runs once and so only ever
     * touches the first scene's camera.
     */
    {
        br_camera *cam = demo->camera->type_data;

        demo->order_table->min_z = cam->hither_z;
        demo->order_table->max_z = cam->yon_z;
        cam->aspect              = BR_DIV(BR_SCALAR(demo->colour_buffer->width), BR_SCALAR(demo->colour_buffer->height));
    }

    st->frame_index = 0;
    return BRE_OK;
}

static void rt_unload_scene(rt_state *st, br_demo *demo)
{
    if(st->shade_palette != NULL) {
        BrPixelmapFree(st->shade_palette);
        st->shade_palette = NULL;
    }

    if(st->scene_root == NULL)
        return;

    BrActorRemove(st->scene_root);
    BrActorFree(st->scene_root);
    st->scene_root = NULL;
    demo->camera   = NULL;
}

/* ------------------------------------------------------------------ */
/* Reference file.                                                    */
/* ------------------------------------------------------------------ */

/*
 * The key names the driver that produced the frame and the floating-point
 * class it was built with, so a reference records a build fact and bit-
 * exactness across rasterisers stays visible. --reference-driver substitutes a
 * different token for lookup only, so a run can score against another
 * rasteriser's pixels without writing a second copy of the file; --bless still
 * records the driver that actually rendered.
 *
 * A key that does not fit the buffer is refused rather than truncated: a
 * shortened key names a different entry, or none at all, and the run would
 * score against that without ever saying so.
 */
static br_error rt_make_key(char *dst, size_t n, rt_state *st, const char *driver, const char *scene)
{
    const char *mode = st->no_depth ? "zs" : "zb";
    int len = snprintf(dst, n, "%s/%s/%s/%s/%u/%dx%d/%s", st->device, driver, RT_FP_CLASS, mode, (unsigned)st->pm_type, st->width, st->height, scene);

    if(len < 0 || (size_t)len >= n) {
        BrLogError("RT",
                   "reference key `%s/%s/%s/%s/%u/%dx%d/%s' is %d characters, more than the %zu this harness holds; the key is not "
                   "truncated to fit",
                   st->device, driver, RT_FP_CLASS, mode, (unsigned)st->pm_type, st->width, st->height, scene, len, n);
        return BRE_FAIL;
    }

    return BRE_OK;
}

/* The driver token the reference is looked up under. */
static const char *rt_lookup_driver(rt_state *st)
{
    return st->reference_driver[0] != '\0' ? st->reference_driver : st->driver;
}

/*
 * Load the reference table.
 *
 * A line that is not "<key> <hash>" names no entry, and an entry that is
 * missing is scored as NO-REFERENCE - which is not a failure, so a reference
 * file that half-parsed is a run that reports PASS with nothing behind it. A
 * key longer than this harness holds is the same defect from the other side:
 * shortened, it names a different entry.
 *
 * A file that will not open, or a read that stops on anything other than
 * end-of-file (fopen() succeeds on a directory, fgets() then fails with
 * EISDIR), is the same defect again: every fixture is scored NO-REFERENCE and
 * the run passes. --bless is the one run that may have nothing to read.
 */
static br_error rt_load_reference(rt_state *st)
{
    FILE *fp = fopen(st->reference, "r");
    char  line[RT_MAX_LINE];
    int   lineno = 0;

    if(fp == NULL) {
        if(st->bless && errno == ENOENT)
            return BRE_OK;

        BrLogError("RT", "Could not open reference `%s': %s; an absent reference is not a table with no entry for this key", st->reference,
                   strerror(errno));
        return BRE_FAIL;
    }

    while(fgets(line, sizeof(line), fp) != NULL) {
        char *p = line;
        char *nl;

        ++lineno;

        while(*p == ' ' || *p == '\t')
            ++p;

        if(*p == '#' || *p == '\n' || *p == '\r' || *p == '\0')
            continue;

        /*
         * Refuse rather than drop. An entry that does not fit is a reference
         * the run would score against as if it had never been written.
         */
        if(st->nexpects >= RT_MAX_ENTRIES) {
            BrLogError("RT", "%s holds more than RT_MAX_ENTRIES (%d) entries; raise it rather than score against a short table",
                       st->reference, RT_MAX_ENTRIES);
            fclose(fp);
            return BRE_FAIL;
        }

        {
            rt_expect *e  = &st->expects[st->nexpects];
            char      *sp = strchr(p, ' ');
            char      *end;

            if((nl = strchr(p, '\n')) != NULL)
                *nl = '\0';

            if((nl = strchr(p, '\r')) != NULL)
                *nl = '\0';

            /* "<key> <hash>" */
            if(sp == NULL || sp == p || sp[1] == '\0') {
                BrLogError("RT", "%s:%d: not `<key> <hash>': a reference that does not parse is scored as if the entry were absent",
                           st->reference, lineno);
                fclose(fp);
                return BRE_FAIL;
            }

            *sp = '\0';

            if(strlen(p) >= sizeof(e->key)) {
                BrLogError("RT", "%s:%d: key is %zu characters, more than the %zu this harness holds; it is not truncated to fit",
                           st->reference, lineno, strlen(p), sizeof(e->key));
                fclose(fp);
                return BRE_FAIL;
            }

            e->hash = strtoull(sp + 1, &end, 16);

            while(*end == ' ' || *end == '\t')
                ++end;

            if(end == sp + 1 || *end != '\0') {
                BrLogError("RT", "%s:%d: `%s' is not a hexadecimal checksum", st->reference, lineno, sp + 1);
                fclose(fp);
                return BRE_FAIL;
            }

            snprintf(e->key, sizeof(e->key), "%s", p);
            ++st->nexpects;
        }
    }

    /*
     * fgets() returns NULL on error as well as at end-of-file, and errno holds
     * the read error; take it before fclose() can overwrite it.
     */
    if(ferror(fp)) {
        int err = errno;

        BrLogError("RT", "Could not read reference `%s': %s; a reference that failed to read is not a table with no entry for this key",
                   st->reference, strerror(err));
        fclose(fp);
        return BRE_FAIL;
    }

    fclose(fp);

    return BRE_OK;
}

static rt_expect *rt_find_expect(rt_state *st, const char *key)
{
    for(int i = 0; i < st->nexpects; ++i) {
        if(strcmp(st->expects[i].key, key) == 0)
            return &st->expects[i];
    }

    return NULL;
}

/*
 * Merge this run's results into the reference so blessing one environment does
 * not drop another's entries.
 *
 * A bless that would not fit, or a result whose key does not fit, is refused
 * outright rather than writing the entries that do: a reference that quietly
 * lost an entry is a run that quietly stopped checking it.
 *
 * A bless that cannot open the file for writing is a failed run, not a logged
 * note: the reference on disk would not be the one the next run scores against.
 */
static void rt_write_reference(rt_state *st)
{
    FILE *fp;
    int   wanted = st->nexpects;

    for(int i = 0; i < st->nresults; ++i) {
        char key[RT_MAX_KEY];

        if(rt_make_key(key, sizeof(key), st, st->driver, st->results[i].scene) != BRE_OK) {
            ++st->failures;
            return;
        }

        if(rt_find_expect(st, key) == NULL)
            ++wanted;
    }

    if(wanted > RT_MAX_ENTRIES) {
        BrLogError("RT", "%s would need %d entries, more than the %d this harness holds; raise RT_MAX_ENTRIES", st->reference, wanted,
                   RT_MAX_ENTRIES);
        ++st->failures;
        return;
    }

    for(int i = 0; i < st->nresults; ++i) {
        char       key[RT_MAX_KEY];
        rt_expect *e;

        if(rt_make_key(key, sizeof(key), st, st->driver, st->results[i].scene) != BRE_OK) {
            ++st->failures;
            return;
        }

        e = rt_find_expect(st, key);

        if(e != NULL) {
            e->hash = st->results[i].hash;
        } else {
            e = &st->expects[st->nexpects++];
            snprintf(e->key, sizeof(e->key), "%s", key);
            e->hash = st->results[i].hash;
        }
    }

    if((fp = fopen(st->reference, "w")) == NULL) {
        BrLogError("RT", "Could not write %s: %s", st->reference, strerror(errno));
        ++st->failures;
        return;
    }

    fprintf(fp, "# rendertest reference\n");
    fprintf(fp, "# key=<device>/<driver>/<fp-class>/<zb|zs>/<pixel-type>/<WxH>/<scene> value=checksum\n");

    for(int i = 0; i < st->nexpects; ++i)
        fprintf(fp, "%s %016llx\n", st->expects[i].key, (unsigned long long)st->expects[i].hash);

    fclose(fp);
    printf("RT reference=%s written (%d entries)\n", st->reference, st->nexpects);
}

/* ------------------------------------------------------------------ */
/* Per-scene completion.                                              */
/* ------------------------------------------------------------------ */

/*
 * Dump the frame as a PPM. The software path hands the facade its colour
 * buffer directly and lets it normalise whatever pixel type that is; the GL
 * path has to build one, because the frame lives in the framebuffer.
 */
static void rt_export_ppm(rt_state *st, br_demo *demo)
{
    char path[600];

    if(st->ppm_dir[0] == '\0')
        return;

    snprintf(path, sizeof(path), "%s/%s.ppm", st->ppm_dir, st->scenes[st->scene_index].name);

    if(!demo->hw_accel) {
        br_pixelmap *save = demo->colour_buffer->map;

        /*
         * The software INDEX_8 colour buffer is a buffer of shade indices, not
         * colours (see rt_build_shade_palette()); BrFmtImageSave() only exports
         * it by applying a palette. Lend it the grey ramp for the length of the
         * save and put the colour buffer back exactly as it was: the checksum
         * has already been taken from the un-paletted buffer and the next scene
         * must not render against this diagnostic palette.
         */
        if(save == NULL && st->shade_palette != NULL)
            demo->colour_buffer->map = st->shade_palette;

        BrFmtImageSave(path, demo->colour_buffer, BR_FMT_IMAGE_PPM);

        demo->colour_buffer->map = save;
        return;
    }

    {
        int          w = 0, h = 0;
        br_uint_8   *px;
        br_pixelmap *pm;

        SDL_GetWindowSizeInPixels(st->window, &w, &h);

        if(w <= 0 || h <= 0)
            return;

        if((pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, w, h, NULL, BR_PMAF_NORMAL)) == NULL)
            return;

        if((px = BrResAllocate(NULL, (br_size_t)w * h * 4, BR_MEMORY_APPLICATION)) == NULL) {
            BrPixelmapFree(pm);
            return;
        }

        if(!rt_readback_gl(st, demo, w, h, px)) {
            BrResFree(px);
            BrPixelmapFree(pm);
            return;
        }

        /*
         * The readback is bottom-up and PPM is top-down. BR_PMT_RGBA_8888 is
         * ARGB-on-the-wire: its bytes are b,g,r,a, while the readback hands
         * back r,g,b,a, so swap red and blue.
         */
        for(int y = 0; y < h; ++y) {
            const br_uint_8 *src = px + (br_size_t)(h - 1 - y) * w * 4;
            br_uint_8       *dst = (br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes;

            for(int x = 0; x < w; ++x) {
                dst[x * 4 + 0] = src[x * 4 + 2];
                dst[x * 4 + 1] = src[x * 4 + 1];
                dst[x * 4 + 2] = src[x * 4 + 0];
                dst[x * 4 + 3] = src[x * 4 + 3];
            }
        }

        BrResFree(px);
        BrFmtImageSave(path, pm, BR_FMT_IMAGE_PPM);
        BrPixelmapFree(pm);
    }
}

static void rt_finish_scene(rt_state *st, br_demo *demo)
{
    br_uint_64  hash    = 0;
    br_uint_32  covered = 0, total = 0;
    char        key[RT_MAX_KEY];
    rt_expect  *e;
    const char *status;

    if(demo->hw_accel)
        rt_checksum_gl(st, demo, &hash, &covered, &total);
    else
        rt_checksum_software(st, demo, &hash, &covered, &total);

    rt_export_ppm(st, demo);

    /*
     * Cannot fire while the scene list fits in RT_MAX_SCENES, which main()
     * enforces; refuse rather than drop all the same, because a scene with no
     * result is a scene with no relation and no coverage check.
     */
    if(st->nresults >= RT_MAX_SCENES) {
        BrLogError("RT", "%s: more results than the %d this harness holds; raise RT_MAX_SCENES", st->scenes[st->scene_index].name, RT_MAX_SCENES);
        ++st->failures;
    } else {
        rt_result *r = &st->results[st->nresults++];

        snprintf(r->scene, sizeof(r->scene), "%s", st->scenes[st->scene_index].name);
        r->hash     = hash;
        r->coverage = covered;
        r->total    = total;
    }

    /*
     * A key that does not fit is a failed run, not a shortened key: the lookup
     * would name a different entry, or none, and score against that.
     */
    if(rt_make_key(key, sizeof(key), st, rt_lookup_driver(st), st->scenes[st->scene_index].name) != BRE_OK) {
        status = "BAD-KEY";
        ++st->failures;
    } else if(st->bless) {
        status = "BLESS";
    } else if(!st->env_ok) {
        status = "NO-REFERENCE";
    } else if((e = rt_find_expect(st, key)) == NULL) {
        status = "NO-REFERENCE";
    } else if(e->hash == hash) {
        status = "MATCH";
        ++st->compared;
    } else {
        status = "CHANGED";
        ++st->compared;
        ++st->failures;
    }

    printf("RT device=%s driver=%s mode=%s type=%u size=%dx%d scene=%s checksum=%016llx coverage=%.2f%% (%u/%u) %s\n", st->device,
           st->driver, st->no_depth ? "zs" : "zb", (unsigned)st->pm_type, st->width, st->height, st->scenes[st->scene_index].name,
           (unsigned long long)hash, total != 0 ? 100.0 * (double)covered / (double)total : 0.0, covered, total, status);
}

/* ------------------------------------------------------------------ */
/* Relations: cases that must differ from each other.                 */
/* ------------------------------------------------------------------ */

typedef struct rt_relation {
    const char *a;
    const char *b;
    br_boolean  equal; /* BR_TRUE: must match. BR_FALSE: must differ. */

    /*
     * Only check under this colour buffer type, or RT_ANY_TYPE for any. The
     * indexed lookup tables, the indexed fog and dithered_map are INDEX_8 only:
     * a shade table of the wrong type simply does not match an RGB block and no
     * RGB block declares a fog type at all, so the renderer drops both - which
     * is the behaviour, not a failure, and asserting the relation there would be
     * asserting something false.
     */
    br_uint_8 pm_type;

    /*
     * Only check in this depth mode, or RT_ANY_MODE. pentprim has no
     * z-sorted (no depth buffer) block that carries an indexed shade table, an
     * indexed blend table or a fog table at all, so in that mode those three
     * pieces of state are dropped and the relation would be asserting the
     * opposite of what the renderer does.
     */
    br_uint_8 mode;

    /*
     * Only check under this device, or NULL for any. glrend has no indexed
     * rasteriser and is always perspective correct, so the shading mode and the
     * affine/perspective distinction do not exist there: asserting either would
     * be asserting a softrend property of a hardware frame.
     */
    const char *device;
} rt_relation;

/*
 * A colour buffer type, or RT_ANY_TYPE for any; a depth mode, or RT_ANY_MODE
 * for either. Used by both the relation table and the blank table below.
 *
 * Both have to be spelled out on every entry. An unset .pm_type is 0, which is
 * BR_PMT_INDEX_1 - a type this harness never runs - and an unset .mode is 0,
 * which is RT_MODE_ZB, a real mode, so an entry that omits either is checked in
 * the configurations that field happens to select and skipped everywhere else,
 * without a word.
 */
#define RT_ANY_TYPE 0xff
#define RT_ANY_MODE 0xff
#define RT_MODE_ZB  0
#define RT_MODE_ZS  1

/*
 * An MMX-family relation. The MMX rasterisers only exist for the 555 and 565
 * outputs, so the same pair has to be asserted at both types - and neither of
 * the other two depths, where the family's flags have no block to select and
 * the frames agree for reasons that say nothing about the kernel.
 */
#define RT_MMX_NE(a_, b_)                                                                                       \
    {.a = a_, .b = b_, .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZB, .device = "softrend"}, \
        {.a = a_, .b = b_, .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZB, .device = "softrend"}

/*
 * The lighting fixtures. Each pair differs by one property of one light -
 * whether it hits, how far it reaches, whether it is directional, ambient-only
 * or view-space - so a light path that stops applying one fails here rather
 * than only moving a checksum.
 *
 * The type is RT_ANY_TYPE and the mode RT_ANY_MODE, and both have to be spelled
 * out: an unset field is 0, which for the type is BR_PMT_INDEX_1 and for the
 * mode RT_MODE_ZB, so a relation that omits either is skipped rather than
 * checked. Neither the lighting nor the material is decided by the output type
 * or the depth mode.
 *
 * The columns are hand-aligned, so the table is held out of clang-format.
 */
// clang-format off
static const rt_relation rt_relations[] = {
    {.a = "scene-spot-hit",       .b = "scene-spot-miss",        .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-radius-near",    .b = "scene-radius-far",       .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-directional",    .b = "scene-directional-miss", .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-directional",    .b = "scene-unlit",            .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-colour",         .b = "scene-unlit",            .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-colour",         .b = "scene-colour-two",       .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-colour-ambient", .b = "scene-unlit",            .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-specular",       .b = "scene-directional",      .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    {.a = "scene-scale-spot",     .b = "scene-unlit",            .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    /* The turn has to reach the light, or the fixture is back on-axis. */
    {.a = "scene-scale-spot",     .b = "scene-scale-spot-off",   .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},
    /* The negative control: an unlit material ignores lights entirely. */
    {.a = "scene-unlit",          .b = "scene-unlit-plain",      .equal = BR_TRUE,  .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE},

    /*
     * The render-feature fixtures. Each pair differs by one piece of material
     * state, so a path that silently stops firing fails here rather than only
     * moving a checksum - which for state the renderer ignores looks like a
     * perfectly good frame.
     *
     * scene-persp and scene-tex-arb are pinned to RT_MODE_ZB, and the pin is a
     * measurement rather than a default: in the Z-sort mode at 555 and 565 the
     * frames each names are identical to scene-textured's (measured at both
     * types), so the premise
     * each asserts - that the perspective flag and the arbitrary-UV flag reach
     * the rasteriser - is false in that mode. Waking either would fail for a
     * reason that has nothing to do with the renderer. Every other pair in the
     * block holds in both modes and is asserted at RT_ANY_MODE.
     */
    {.a = "scene-smooth",         .b = "scene-flat",             .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-textured-shade", .b = "scene-textured",         .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB},
    {.a = "scene-persp",          .b = "scene-textured",         .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-persp-shade",    .b = "scene-persp",            .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB},
    {.a = "scene-tex-arb",        .b = "scene-textured",         .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_MODE_ZB},
    /*
     * The 32x32 map in the Z-sort mode: the mode is where pentprim's
     * TriangleRender_PT_I8_32 packs the base texel from the wrong width, and
     * scene-persp is the same rig one map size away, so a fixture that quietly
     * kept a 64 map fails here by name rather than by moving a checksum. The
     * z-buffered pair is the same shape through TriangleRender_ZPT_I8_D16_32,
     * which never had the slip; this relation is checked in the mode that did.
     */
    {.a = "scene-tex-32",         .b = "scene-persp",            .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade",          .b = "scene-smooth",           .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB},
    {.a = "scene-decal",          .b = "scene-textured-shade",   .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE},
    {.a = "scene-fog",            .b = "scene-smooth",           .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB},
    {.a = "scene-blend",          .b = "scene-blend-off",        .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB},
    {.a = "scene-dither",         .b = "scene-persp",            .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE},

    /*
     * The RGB-output shade-table witness. The shape is only reachable at the
     * z-sorted 555/565 paths: a z-buffered textured primitive matches an MMX
     * block first, and an 888 output has no such kernel. scene-textured-shade
     * is the control for the smooth pair (the
     * same rig with an INDEX_8 table, which at these types matches no shaded
     * block and draws the untextured one instead); scene-flat is the control for
     * the flat pair, whose constant-intensity cousin it is.
     */
    {.a = "scene-shade-rgb555",      .b = "scene-textured-shade", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-rgb565",      .b = "scene-textured-shade", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-rgb555-flat", .b = "scene-flat",           .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-rgb565-flat", .b = "scene-flat",           .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},

    /*
     * The two refused families' witnesses. Each differs from a neighbour that
     * cannot carry the family's defining type, so a fixture that quietly selects
     * a sibling - the failure mode both fixtures exist to rule out - fails here
     * by name instead of only moving a checksum.
     *
     * Family A (an RGB-typed colour map) is asserted in the Z-sort mode alone,
     * because the z-buffered 555/565 textured blocks are unreachable: pentprim
     * walks the MMX table first, its textured rows all require an INDEX_8 map
     * with a palette, and its untextured rows match any 555/565 triangle, so a
     * 555/565 map is drawn untextured and the fixture reaches nothing. Measured
     * at both types in both modes: the two frames differ in exactly the mode
     * asserted. scene-textured is the control
     * - the same rig with an INDEX_8 map.
     *
     * Family B (a textured primitive with an RGB-typed shade table) is
     * asserted at 888, where the block is reachable, against
     * scene-textured-shade - the same rig with an INDEX_8 table, which at 888
     * matches no shaded block and falls through to the untextured one.
     */
    {.a = "scene-tex-rgb555",        .b = "scene-textured",       .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-tex-rgb565",        .b = "scene-textured",       .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-rgb888",      .b = "scene-textured-shade", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_MODE_ZB,
     .device = "softrend"},

    /*
     * The topology witnesses. Each is scene-flat's rig with the style changed
     * and nothing else, so a dropped render_style leaves the actor drawing
     * faces and the frame equal to scene-flat's. How the style reaches the
     * renderer is softrend's business, and glrend has no indexed rasteriser to
     * compare, so the pair is asserted there only.
     *
     * Both fields have to be spelled out; see the note on RT_ANY_TYPE above.
     * Measured, each witness differs from scene-flat in every type and both
     * depth modes, so they are asserted at RT_ANY_MODE rather than left at the
     * RT_MODE_ZB an unset field means.
     */
    {.a = "scene-edges",          .b = "scene-flat",             .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-points",         .b = "scene-flat",             .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE,
     .device = "softrend"},

    /*
     * The line and point material witnesses. Each scene holds an edge actor and
     * a point actor sharing one material, so a pair that differs by one knob of
     * that material fails here by name whatever the topology - and a pair that
     * is one actor short cannot stand in, which is why each fixture's control
     * is another two-actor scene rather than scene-edges or scene-points.
     *
     * The textured fixtures are asserted at INDEX_8 and RGB_888 only, and that
     * is a measurement: neither the 555 nor the 565 line/point table has a
     * textured entry at all, so at those types the map is dropped and both
     * sides of the relation draw the same untextured frame. Asserting it there
     * would be asserting a property the renderer does not have.
     *
     * At 24bpp the loader re-types the indexed map as RGB_888 and the general
     * 888 table does have a textured entry, so a relation against a fixture
     * without a map holds in both depth modes. It does not hold between
     * scene-lines-map-gouraud and scene-lines-map: the 888 textured line block
     * carries no shading at all, so both select the same unshaded cell there and
     * the frames are equal - measured, and why that pair is asserted at 8bpp
     * alone while the map/no-map pairs are asserted at both.
     *
     * Both fields are spelled out; see the note on RT_ANY_TYPE above.
     */
    {.a = "scene-lines-gouraud",     .b = "scene-lines-plain",         .equal = BR_FALSE, .pm_type = RT_ANY_TYPE, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map",         .b = "scene-lines-plain",         .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map",         .b = "scene-lines-plain",         .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map-gouraud", .b = "scene-lines-map",           .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map-gouraud", .b = "scene-lines-gouraud",       .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map-gouraud", .b = "scene-lines-gouraud",       .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map-unlit",   .b = "scene-lines-plain-unlit",   .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-lines-map-unlit",   .b = "scene-lines-plain-unlit",   .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_ANY_MODE,
     .device = "softrend"},

    /*
     * The RGB-output arbitrary-width shade-table witnesses. The perspective
     * pairs are asserted at 555 and 565 in the Z-sort mode only: with a depth
     * buffer the 555/565 walk reaches an untextured block first, and the flag
     * does not change that frame - the Z-sort tables are where the CORRECT and
     * AFFINE cells of this family sit at all. At 888 both modes reach them. The
     * flat/gouraud pair is asserted in the same places for the same reason. The
     * affine twins of the two RGB-typed colour maps are one flag away from the
     * perspective fixtures already in the corpus, so the pair is asserted in
     * the mode those fixtures are witnesses in.
     */
    {.a = "scene-shade-arb-flat-persp",   .b = "scene-shade-arb-flat",   .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-arb-flat-persp",   .b = "scene-shade-arb-flat",   .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-arb-flat-persp",   .b = "scene-shade-arb-flat",   .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-shade-arb-smooth-persp", .b = "scene-shade-arb-flat-persp", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-arb-smooth-persp", .b = "scene-shade-arb-flat-persp", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-shade-arb-smooth-persp", .b = "scene-shade-arb-flat-persp", .equal = BR_FALSE, .pm_type = BR_PMT_RGB_888, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-tex-rgb555-affine",      .b = "scene-tex-rgb555",      .equal = BR_FALSE, .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZS,
     .device = "softrend"},
    {.a = "scene-tex-rgb565-affine",      .b = "scene-tex-rgb565",      .equal = BR_FALSE, .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZS,
     .device = "softrend"},

    /*
     * The MMX family. Every pair differs by exactly one property of one cube:
     * the perspective flag, the smoothing flag, or one of the two ROP flags
     * (`BR_MATF_DITHER`, opacity below full). A kernel that stops firing, or
     * that is selected by the wrong twin, moves one of these frames.
     */
    RT_MMX_NE("scene-mmx-uv-dither-persp", "scene-mmx-uv-dither-affine"),
    RT_MMX_NE("scene-mmx-uv-screen-persp", "scene-mmx-uv-screen-affine"),
    RT_MMX_NE("scene-mmx-uv-ditherscreen-persp", "scene-mmx-uv-ditherscreen-affine"),
    RT_MMX_NE("scene-mmx-uvc-dither-persp", "scene-mmx-uvc-dither-affine"),
    RT_MMX_NE("scene-mmx-uvc-screen-persp", "scene-mmx-uvc-screen-affine"),
    RT_MMX_NE("scene-mmx-uvc-ditherscreen-persp", "scene-mmx-uvc-ditherscreen-affine"),

    RT_MMX_NE("scene-mmx-rgb-dither-smooth", "scene-mmx-rgb-dither-flat"),
    RT_MMX_NE("scene-mmx-rgb-ditherscreen-smooth", "scene-mmx-rgb-ditherscreen-flat"),

    RT_MMX_NE("scene-mmx-rgb-dither-flat", "scene-mmx-rgb-screen-flat"),
    RT_MMX_NE("scene-mmx-rgb-ditherscreen-flat", "scene-mmx-rgb-dither-flat"),
    RT_MMX_NE("scene-mmx-uv-dither-affine", "scene-mmx-uv-screen-affine"),
    RT_MMX_NE("scene-mmx-uv-dither-affine", "scene-mmx-uv-ditherscreen-affine"),
    RT_MMX_NE("scene-mmx-uvc-dither-affine", "scene-mmx-uvc-ditherscreen-affine"),
    RT_MMX_NE("scene-mmx-uvc-screen-affine", "scene-mmx-uvc-ditherscreen-affine"),
    RT_MMX_NE("scene-mmx-uvc-dither-persp", "scene-mmx-uvc-ditherscreen-persp"),
    RT_MMX_NE("scene-mmx-uvc-screen-persp", "scene-mmx-uvc-ditherscreen-persp"),
    RT_MMX_NE("scene-mmx-uvrgb-dither-affine", "scene-mmx-uvrgb-ditherscreen-affine"),
    RT_MMX_NE("scene-mmx-uvrgb-screen-persp", "scene-mmx-uvrgb-ditherscreen-persp"),

    /*
     * The INDEX_8 ROP cross-product. The blend+fog scenes are the only fixtures
     * where all three operators meet in one primitive, so each is asserted
     * against the same rig with one of the two tables dropped - the same map,
     * the same cubes, the same camera: a kernel that applies fog but not the
     * blend table (or the reverse), or that applies them in the other order,
     * moves one of these frames.
     *
     * The z-buffered mode is where those cells are: the z-sorted half of the
     * table carries no blend or fog entry at all, so the pairs are not asserted
     * there. They cannot be asserted equal, either, and that is the fixtures'
     * own doing rather than the renderer's: a scene that binds a fog table is
     * drawn at the narrowed 4..8 camera range the fog levels need (scene.c), so
     * the two frames of a pair are different projections of the same rig and
     * comparing them in a mode that applies neither table says nothing.
     */
    {.a = "scene-blendfog-arb-persp", .b = "scene-blend-arb-persp", .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-blendfog-arb-persp", .b = "scene-fog-arb-persp",   .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-blendfog-arb",       .b = "scene-blend-arb",       .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-blendfog-p2",        .b = "scene-blend-p2-persp",  .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},

    /*
     * The rest of the cross-product, each against the neighbour it is one knob
     * away from: the perspective flag on the fog rows, the decal flag against
     * the same rig without it, the empty index band's flat twin, and the two
     * plain DIVIDE columns. The rows the z-sorted table cannot select are
     * asserted in the z-buffered mode alone.
     */
    {.a = "scene-fog-p2-persp",       .b = "scene-fog-p2",          .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-fog-arb-persp",      .b = "scene-fog-arb",         .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-fog-flat",           .b = "scene-flat",            .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-shade-flat",         .b = "scene-shade",           .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-idx-arb-persp",      .b = "scene-idx-arb",         .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-decal-p2-256",       .b = "scene-idx-p2-256",      .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_ANY_MODE,
     .device = "softrend"},
    {.a = "scene-blend-p2-smooth",    .b = "scene-blend",            .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    /*
     * The three cells the previous round left out, now bought;
     * see scene.c. Each is the cube of a cell whose other cubes are the scene it
     * is asserted against, so a cube that stopped selecting the entry it was
     * built for moves one of these frames.
     */
    {.a = "scene-blend-p2-flat",      .b = "scene-blend-p2-smooth", .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-decal-arb-flat",     .b = "scene-decal-arb",       .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
    {.a = "scene-fog-p2-tex",         .b = "scene-fog-p2",          .equal = BR_FALSE, .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZB,
     .device = "softrend"},
};
// clang-format on

static rt_result *rt_find_result(rt_state *st, const char *scene)
{
    for(int i = 0; i < st->nresults; ++i) {
        if(strcmp(st->results[i].scene, scene) == 0)
            return &st->results[i];
    }

    return NULL;
}

/*
 * Fixtures that must draw something.
 *
 * A frame that drew nothing is not a frame that drew correctly, and neither of
 * the checks above can tell the difference. A checksum only separates the two
 * when the stored reference is not itself blank: where it is - a fixture whose
 * feature the renderer under test refuses, or one whose surface quantises to
 * zero in five-bit channels - a blank frame matches it and reports PASS. And a
 * negative relation has the same hole from the other side: drawing nothing
 * differs from drawing something, so "scene-shade != scene-smooth" passes on
 * the most broken frame in the corpus.
 *
 * So every scene has to light at least one pixel, unless it is one of the
 * expected-blank cases below. Those are listed rather than the assertion being
 * narrowed to the scenes that pass today, so a fixture that starts rendering
 * blank fails here by name instead of moving a checksum.
 */
typedef struct rt_blank {
    const char *scene;
    br_uint_8   pm_type;
    br_uint_8   mode;

    /*
     * Only excuse the blank frame under this device, or NULL for any. A device
     * that refuses a fixture says nothing about the device under test, and the
     * refusal is a property of one driver, not of the scene.
     */
    const char *device;
} rt_blank;

/* Hand-aligned columns; held out of clang-format for the same reason as rt_relations above. */
// clang-format off
static const rt_blank rt_expected_blank[] = {
    /*
     * pentprim's own frame is blank for each of these, so there is nothing to
     * require of the driver under test. Measured with the pentprim build at
     * every bpp x mode; the light-cull fixtures are 8.3% of the screen at
     * 8bpp and 16bpp but quantise to zero in five-bit channels, which is why
     * 15bpp is the odd one.
     */
    {.scene = "scene-spot-miss",         .pm_type = BR_PMT_RGB_555, .mode = RT_ANY_MODE},
    {.scene = "scene-radius-far",        .pm_type = BR_PMT_RGB_555, .mode = RT_ANY_MODE},
    {.scene = "scene-directional-miss",  .pm_type = BR_PMT_RGB_555, .mode = RT_ANY_MODE},
    {.scene = "scene-scale-spot-off",    .pm_type = BR_PMT_RGB_555, .mode = RT_ANY_MODE},

    /*
     * The blend fixture has no RGB blend block to select: pentprim draws
     * nothing for it at 15/16bpp z-buffered, and its stored reference is the
     * empty frame. The 8bpp configurations are not here - pentprim draws the
     * fixture there, so a blank frame is a failure.
     */
    {.scene = "scene-blend",             .pm_type = BR_PMT_RGB_555, .mode = RT_MODE_ZB},
    {.scene = "scene-blend",             .pm_type = BR_PMT_RGB_565, .mode = RT_MODE_ZB},

    /*
     * pentprim draws nothing for the shade fixture in the Z-sort mode either -
     * no z-sorted block carries an indexed shade table - so the 8bpp z-sorted
     * key is two blank frames matching. scene-shade-flat is the same rig without
     * BR_MATF_SMOOTH, and it selects the z-sorted constant-intensity block there,
     * whose whole intensity is the material's index band: that band is empty
     * (index_range zero, which is what makes the untextured family reachable at
     * all), so the frame is index 0 and draws nothing.
     */
    {.scene = "scene-shade",             .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZS},
    {.scene = "scene-shade-flat",        .pm_type = BR_PMT_INDEX_8, .mode = RT_MODE_ZS},

    /*
     * The line topology witness under glrend, and only there.
     *
     * glrend's stored-geometry entry points refuse everything but triangles and
     * points - `br_geometry_stored_gl::render` and `renderOnScreen` return
     * BRE_FAIL for BRT_LINE - so the driver
     * draws nothing for BR_RSTYLE_EDGES. That is a glrend gap the fixture now
     * makes reachable from a file, not a property of the scene: every other
     * device here draws it, and the softrend keys in rendertest.txt record
     * that. The glrend reference entry is the blank frame the driver draws;
     * this expected-blank entry is what stops a blank frame failing the
     * draw check.
     */
    {.scene = "scene-edges",             .pm_type = RT_ANY_TYPE,    .mode = RT_ANY_MODE, .device = "glrend"},
};
// clang-format on

static void rt_check_drawn(rt_state *st)
{
    for(int i = 0; i < st->nresults; ++i) {
        rt_result *r        = &st->results[i];
        br_boolean expected = BR_FALSE;

        for(size_t j = 0; j < BR_ASIZE(rt_expected_blank); ++j) {
            if(strcmp(rt_expected_blank[j].scene, r->scene) != 0)
                continue;

            if(rt_expected_blank[j].pm_type != RT_ANY_TYPE && rt_expected_blank[j].pm_type != st->pm_type)
                continue;

            if(rt_expected_blank[j].mode != RT_ANY_MODE && rt_expected_blank[j].mode != (st->no_depth ? RT_MODE_ZS : RT_MODE_ZB))
                continue;

            if(rt_expected_blank[j].device != NULL && strcmp(rt_expected_blank[j].device, st->device) != 0)
                continue;

            expected = BR_TRUE;
            break;
        }

        if(expected || r->coverage != 0) {
            continue;
        }

        printf("RT blank %s drew nothing of %u px where the fixture must draw\n", r->scene, r->total);
        ++st->failures;
    }
}

static void rt_check_relations(rt_state *st)
{
    for(size_t i = 0; i < BR_ASIZE(rt_relations); ++i) {
        rt_result *a = rt_find_result(st, rt_relations[i].a);
        rt_result *b = rt_find_result(st, rt_relations[i].b);

        if(a == NULL || b == NULL)
            continue;

        if(rt_relations[i].pm_type != RT_ANY_TYPE && rt_relations[i].pm_type != st->pm_type)
            continue;

        if(rt_relations[i].mode != RT_ANY_MODE && rt_relations[i].mode != (st->no_depth ? RT_MODE_ZS : RT_MODE_ZB))
            continue;

        if(rt_relations[i].device != NULL && strcmp(rt_relations[i].device, st->device) != 0)
            continue;

        br_boolean same = (a->hash == b->hash);

        if(same != rt_relations[i].equal) {
            printf("RT relation %s %s %s FAIL (%016llx vs %016llx)\n", a->scene, rt_relations[i].equal ? "==" : "!=", b->scene,
                   (unsigned long long)a->hash, (unsigned long long)b->hash);
            ++st->failures;
        } else {
            printf("RT relation %s %s %s ok\n", a->scene, rt_relations[i].equal ? "==" : "!=", b->scene);
        }
    }
}

/*
 * A softrend run must score against at least one stored entry.
 *
 * The key carries the driver token, so a softrend run that forgets
 * --reference-driver software, or names a token the table does not hold, finds
 * no key at all and scores every fixture NO-REFERENCE, which is not a failure:
 * the run reports PASS with nothing behind it. NO-REFERENCE for one key is
 * legitimate - a new fixture, or a first --bless - so the check is on the
 * total, and only a run that compared against nothing anywhere is refused.
 *
 * Only softrend is checked. A glrend run on a real GPU legitimately has no
 * stored reference for any key - the glrend entries are llvmpipe's - so the
 * same rule would refuse a valid run.
 */
static void rt_check_compared(rt_state *st)
{
    if(st->bless || strcmp(st->device, "softrend") != 0)
        return;

    if(st->compared != 0)
        return;

    BrLogError("RT",
               "softrend compared against none of the %d fixtures' stored entries: every one is NO-REFERENCE, so PASS asserts nothing "
               "about the reference. Check that --reference-driver names a token the reference holds (this run looked up `%s').",
               st->nresults, rt_lookup_driver(st));
    ++st->failures;
}

/* ------------------------------------------------------------------ */
/* Demo callbacks.                                                    */
/* ------------------------------------------------------------------ */

static br_error rt_init(br_demo *demo)
{
    rt_state *st = BrResAllocate(demo, sizeof(rt_state), BR_MEMORY_APPLICATION);
    demo->user   = st;

    if(demo->hw_accel) {
        st->window = BrSDL3UtilGetWindow(demo->_screen);

        st->ReadPixels  = (rt_pfn_ReadPixels)SDL_GL_GetProcAddress("glReadPixels");
        st->ReadBuffer  = (rt_pfn_ReadBuffer)SDL_GL_GetProcAddress("glReadBuffer");
        st->BindTexture = (rt_pfn_BindTexture)SDL_GL_GetProcAddress("glBindTexture");
        st->GetTexImage = (rt_pfn_GetTexImage)SDL_GL_GetProcAddress("glGetTexImage");
        st->PixelStorei = (rt_pfn_PixelStorei)SDL_GL_GetProcAddress("glPixelStorei");
        st->GetString   = (rt_pfn_GetString)SDL_GL_GetProcAddress("glGetString");

        if(st->ReadPixels == NULL && st->GetTexImage == NULL) {
            BrLogError("RT", "No glReadPixels or glGetTexImage; hardware checksum unavailable");
            return BRE_FAIL;
        }

        if(st->GetString != NULL) {
            const char *r = (const char *)st->GetString(RT_GL_RENDERER);

            if(r != NULL) {
                size_t n = strcspn(r, " (");

                /*
                 * The token is a component of every key this run builds, so one
                 * that does not fit is refused rather than shortened: shortened,
                 * it records another driver's reference and the run passes on a
                 * table that was never written for it.
                 */
                if(n >= sizeof(st->driver)) {
                    BrLogError("RT", "GL_RENDERER `%s' is %zu characters, more than the %zu a driver token holds", r, n, sizeof(st->driver));
                    return BRE_FAIL;
                }

                memcpy(st->driver, r, n);
                st->driver[n] = '\0';
                BrLogInfo("RT", "GL_RENDERER=%s", r);
            }
        }

        if(st->driver[0] == '\0')
            snprintf(st->driver, sizeof(st->driver), "%s", "unknown");
    } else {
        snprintf(st->driver, sizeof(st->driver), "%s", RT_SOFT_DRIVER);
    }

    /*
     * The configuration was parsed in main(); the demo owns the lifecycle, so
     * copy it into the state that will live for the run.
     */
    {
        st->device   = rt_cfg_device;
        st->pm_type  = demo->colour_buffer->type;
        st->width    = rt_cfg_width;
        st->height   = rt_cfg_height;
        st->warmup   = rt_cfg_warmup;
        st->no_depth = rt_cfg_no_depth;
        st->bless    = rt_cfg_bless;
        st->verbose  = rt_cfg_verbose;
        st->env_ok   = 1;
        st->scenes   = BrResAllocate(st, sizeof(rt_scene) * RT_MAX_SCENES, BR_MEMORY_APPLICATION);
        st->results  = BrResAllocate(st, sizeof(rt_result) * RT_MAX_SCENES, BR_MEMORY_APPLICATION);
        st->expects  = BrResAllocate(st, sizeof(rt_expect) * RT_MAX_ENTRIES, BR_MEMORY_APPLICATION);
        st->nscenes  = rt_cfg_nscenes;
        st->nresults = 0;
        st->compared = 0;
        st->nexpects = 0;
        st->failures = 0;

        /*
         * The path is a fixed buffer and this is the last point it can be
         * refused. Truncated, it names a different file - one that may exist,
         * in which case the run scores against a table it never asked for - and
         * if it does not, the failed open is the absent reference that
         * rt_load_reference() no longer treats as usable.
         */
        if(strlen(rt_cfg_reference) >= sizeof(st->reference)) {
            BrLogError("RT", "--reference `%s' is %zu characters, more than the %zu a reference path holds", rt_cfg_reference,
                       strlen(rt_cfg_reference), sizeof(st->reference));
            return BRE_FAIL;
        }

        snprintf(st->reference, sizeof(st->reference), "%s", rt_cfg_reference);
        snprintf(st->ppm_dir, sizeof(st->ppm_dir), "%s", rt_cfg_ppm_dir);

        /* The same refusal for the substituted token; see the GL_RENDERER one. */
        {
            const char *token = rt_cfg_ref_driver != NULL ? rt_cfg_ref_driver : "";

            if(strlen(token) >= sizeof(st->reference_driver)) {
                BrLogError("RT", "--reference-driver `%s' is %zu characters, more than the %zu a driver token holds", token, strlen(token),
                           sizeof(st->reference_driver));
                return BRE_FAIL;
            }

            memcpy(st->reference_driver, token, strlen(token) + 1);
        }

        for(int i = 0; i < st->nscenes; ++i) {
            /*
             * The name is a fixed buffer and it is what the reference key is
             * built from, so a name that does not fit is refused rather than
             * shortened. Shortened, the key names a different fixture, and the
             * key-length check in rt_make_key() cannot see it: the key built
             * from the shortened name is inside RT_MAX_KEY and passes.
             */
            if(strlen(rt_cfg_scenes[i]) >= sizeof(st->scenes[i].name)) {
                BrLogError("RT", "scene `%s' is %zu characters, more than the %zu a fixture name holds; it is not truncated to fit",
                           rt_cfg_scenes[i], strlen(rt_cfg_scenes[i]), sizeof(st->scenes[i].name));
                return BRE_FAIL;
            }

            snprintf(st->scenes[i].name, sizeof(st->scenes[i].name), "%s", rt_cfg_scenes[i]);
            snprintf(st->scenes[i].path, sizeof(st->scenes[i].path), "%s/%s.gltf", rt_cfg_scene_dir, rt_cfg_scenes[i]);
        }

        if(demo->hw_accel)
            st->env_ok = 1; /* the driver token above keys the reference */
    }

    if(st->env_ok && rt_load_reference(st) != BRE_OK)
        return BRE_FAIL;

    if(rt_load_scene(st, demo, 0) != BRE_OK)
        return BRE_FAIL;

    return BRE_OK;
}

static void rt_render(br_demo *demo)
{
    rt_state *st = demo->user;

    BrRendererFrameBegin();

    BrPixelmapFill(demo->colour_buffer, demo->clear_colour);

    /*
     * --no-depth takes the Z-sort path, which binds no depth buffer. That is
     * the path the MMX 555/565 rasterisers have to be kept off: they are all
     * z rasterisers and fault when the depth base is left unset.
     */
    if(st->no_depth) {
        BrZsSceneRender(demo->world, demo->camera, demo->colour_buffer);
    } else {
        BrPixelmapFill(demo->depth_buffer, 0xFFFFFFFF);
        BrZbSceneRender(demo->world, demo->camera, demo->colour_buffer, demo->depth_buffer);
    }

    BrRendererFrameEnd();

    if(st->frame_index < st->warmup) {
        ++st->frame_index;
        return;
    }

    rt_finish_scene(st, demo);

    ++st->scene_index;

    if(st->scene_index >= st->nscenes) {
        rt_check_drawn(st);
        rt_check_relations(st);
        rt_check_compared(st);

        if(st->bless)
            rt_write_reference(st);

        if(st->failures != 0)
            g_failed = 1;

        printf("RT result=%s failures=%d\n", st->failures == 0 ? "PASS" : "FAIL", st->failures);
        SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
        return;
    }

    rt_unload_scene(st, demo);

    if(rt_load_scene(st, demo, st->scene_index) != BRE_OK) {
        g_failed = 1;
        SDL_PushEvent(&(SDL_Event){.type = SDL_EVENT_QUIT});
    }
}

static void rt_destroy(br_demo *demo)
{
    rt_state *st = demo->user;

    if(st != NULL && st->shade_palette != NULL) {
        BrPixelmapFree(st->shade_palette);
        st->shade_palette = NULL;
    }

    BrDemoDefaultDestroy(demo);
}

static const br_demo_dispatch rt_dispatch = {
    .init          = rt_init,
    .process_event = BrDemoDefaultProcessEvent,
    .update        = BrDemoDefaultUpdate,
    .render        = rt_render,
    .on_resize     = BrDemoDefaultOnResize,
    .destroy       = rt_destroy,
};

/* ------------------------------------------------------------------ */
/* Argument parsing.                                                  */
/* ------------------------------------------------------------------ */

/* Laid out as a grid so the set reads as one list; held out of clang-format, which would put one name per line. */
// clang-format off
static const char *const rt_default_scenes[] = {
    "scene-spot-hit",   "scene-spot-miss", "scene-spot-hit-miss",  "scene-radius-near", "scene-radius-far",      "scene-scaled",
    "scene-view-space", "scene-colour",    "scene-colour-ambient", "scene-colour-two",  "scene-directional",     "scene-directional-miss",
    "scene-specular",   "scene-unlit",     "scene-unlit-plain",    "scene-scale-spot",  "scene-scale-direction", "scene-scale-spot-off",

    /* The render-feature fixtures; see scene.c. */
    "scene-flat",       "scene-smooth",    "scene-textured",       "scene-textured-shade", "scene-persp",        "scene-persp-shade",
    "scene-tex-arb",    "scene-tex-32",     "scene-shade",          "scene-decal",       "scene-fog",             "scene-blend",
    "scene-dither",     "scene-alpha",
    "scene-shade-rgb555", "scene-shade-rgb555-flat", "scene-shade-rgb565", "scene-shade-rgb565-flat",
    "scene-shade-rgb888", "scene-tex-rgb555", "scene-tex-rgb565",
    "scene-blend-off",

    /*
     * The RGB-output arbitrary-width shade-table fixtures and the affine twins
     * of the two RGB-typed colour maps; see scene.c. The first three carry
     * one cube per output format, so each is the witness at its own --bpp.
     */
    "scene-shade-arb-flat", "scene-shade-arb-flat-persp", "scene-shade-arb-smooth-persp",
    "scene-tex-rgb555-affine", "scene-tex-rgb565-affine",

    /*
     * The topology witnesses; see scene.c. Non-triangle topologies have no
     * witness without them, because the style they need is actor state and no
     * glTF path carried it before BR_actors.
     */
    "scene-edges",
    "scene-points",

    /*
     * The line and point material states; see scene.c. Each scene carries an
     * edge actor and a point actor sharing one material, so one scene witnesses
     * both topologies - and the two plain fixtures are the controls the others
     * are one knob away from.
     */
    "scene-lines-plain",       "scene-lines-gouraud", "scene-lines-map", "scene-lines-map-gouraud", "scene-lines-plain-unlit",
    "scene-lines-map-unlit",

    /*
     * The MMX 555/565 family. pentprim walks the MMX table before the general
     * one, and its sixteen kernel names cover dithered, screendoor and
     * dithered-screendoor twins of the textured, flat, gouraud and untextured
     * blocks - seven of which had no witness anywhere in the tree. One scene
     * per cube state, so that a frame names the one state that moved it: a
     * scene can carry several materials - each cube's actor material
     * round-trips through cgltf_node::brender_material - but then its one
     * checksum would say only that one of the cubes changed. See scene.c.
     */
    "scene-mmx-rgb-dither-smooth",        "scene-mmx-rgb-dither-flat",        "scene-mmx-rgb-screen-flat",
    "scene-mmx-rgb-ditherscreen-smooth",  "scene-mmx-rgb-ditherscreen-flat",

    "scene-mmx-uv-dither-persp",          "scene-mmx-uv-dither-affine",
    "scene-mmx-uv-screen-persp",          "scene-mmx-uv-screen-affine",
    "scene-mmx-uv-ditherscreen-persp",    "scene-mmx-uv-ditherscreen-affine",

    "scene-mmx-uvc-persp",                "scene-mmx-uvc-dither-persp",        "scene-mmx-uvc-dither-affine",
    "scene-mmx-uvc-screen-persp",         "scene-mmx-uvc-screen-affine",
    "scene-mmx-uvc-ditherscreen-persp",   "scene-mmx-uvc-ditherscreen-affine",

    "scene-mmx-uvrgb-dither-affine",      "scene-mmx-uvrgb-screen-persp",
    "scene-mmx-uvrgb-ditherscreen-persp", "scene-mmx-uvrgb-ditherscreen-affine",

    /*
     * The INDEX_8 ROP cross-product; see scene.c. The blend+fog group is the
     * only place the `shade -> fog -> blend` order runs inside one
     * primitive, so it is bought before the rest of the cross-product, and each
     * of those three scenes carries the three shading modes as three cubes.
     */
    "scene-blendfog-p2",           "scene-blendfog-arb-persp",    "scene-blendfog-arb",
    "scene-blend-p2-persp",        "scene-blend-p2-smooth",       "scene-blend-p2-flat",
    "scene-blend-arb-persp",       "scene-blend-arb",             "scene-fog-p2-persp",
    "scene-fog-p2",                "scene-fog-p2-tex",            "scene-fog-arb-persp",
    "scene-fog-arb",               "scene-fog-flat",              "scene-idx-arb-persp",
    "scene-idx-arb",               "scene-idx-p2-256",            "scene-decal-p2-256",
    "scene-decal-arb",             "scene-decal-arb-flat",        "scene-shade-flat",
};
// clang-format on

/* The default set has to fit: rt_init() copies it into an RT_MAX_SCENES array. */
BR_STATIC_ASSERT(BR_ASIZE(rt_default_scenes) <= RT_MAX_SCENES, "the default fixture set does not fit in RT_MAX_SCENES");

static br_uint_8 rt_bpp_to_type(int bpp)
{
    switch(bpp) {
        case 15:
            return BR_PMT_RGB_555;
        case 16:
            return BR_PMT_RGB_565;
        case 24:
            return BR_PMT_RGB_888;
        default:
        case 8:
            return BR_PMT_INDEX_8;
    }
}

static void rt_usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [options] [scene ...]\n"
            "\n"
            "  --device <name>    glrend (default), glrend1x or softrend\n"
            "  --bpp <n>          software bit depth: 8 (default), 15, 16, 24\n"
            "  -w, --width <n>    render width (default 320)\n"
            "  -h, --height <n>   render height (default 240)\n"
            "  --warmup <n>       frames to render before checksumming (default 2)\n"
            "  --no-depth         render with Z-sort and no depth buffer\n"
            "  --scene-dir <dir>  where the scenes live\n"
            "  --reference <file> reference file to compare against\n"
            "  --reference-driver <token>\n"
            "                     score against another driver's entries (e.g.\n"
            "                     'software' to read another rasteriser's entries);\n"
            "                     --bless still records this binary's own driver\n"
            "  --ppm-dir <dir>    write each frame as <dir>/<scene>.ppm\n"
            "  --bless            write the reference instead of comparing\n"
            "  -v, --verbose      log more\n"
            "  -l, --list         list the scenes and exit\n"
            "\n"
            "With no scene arguments the default fixture set is used. glrend can be\n"
            "forced onto llvmpipe with LIBGL_ALWAYS_SOFTWARE=true.\n",
            argv0);
}

int main(int argc, char **argv)
{
    br_demo_run_args args;

    BrDemoDefaultArgs(&args);

    for(int i = 1; i < argc; ++i) {
        const char *a = argv[i];
        const char *v = (i + 1 < argc) ? argv[i + 1] : NULL;

        if(strcmp(a, "--device") == 0 && v != NULL) {
            rt_cfg_device = argv[++i];
        } else if(strcmp(a, "--bpp") == 0 && v != NULL) {
            rt_cfg_pm_type = rt_bpp_to_type(BrAToI(argv[++i]));
        } else if((strcmp(a, "-w") == 0 || strcmp(a, "--width") == 0) && v != NULL) {
            rt_cfg_width = BrAToI(argv[++i]);
        } else if((strcmp(a, "-h") == 0 || strcmp(a, "--height") == 0) && v != NULL) {
            rt_cfg_height = BrAToI(argv[++i]);
        } else if(strcmp(a, "--warmup") == 0 && v != NULL) {
            rt_cfg_warmup = BrAToI(argv[++i]);
        } else if(strcmp(a, "--no-depth") == 0) {
            rt_cfg_no_depth = 1;
        } else if(strcmp(a, "--scene-dir") == 0 && v != NULL) {
            rt_cfg_scene_dir = argv[++i];
        } else if(strcmp(a, "--reference") == 0 && v != NULL) {
            rt_cfg_reference = argv[++i];
        } else if(strcmp(a, "--reference-driver") == 0 && v != NULL) {
            rt_cfg_ref_driver = argv[++i];
        } else if(strcmp(a, "--ppm-dir") == 0 && v != NULL) {
            snprintf(rt_cfg_ppm_dir, sizeof(rt_cfg_ppm_dir), "%s", argv[++i]);
        } else if(strcmp(a, "--bless") == 0) {
            rt_cfg_bless = 1;
        } else if(strcmp(a, "-v") == 0 || strcmp(a, "--verbose") == 0) {
            ++rt_cfg_verbose;
        } else if(strcmp(a, "-l") == 0 || strcmp(a, "--list") == 0) {
            for(size_t k = 0; k < BR_ASIZE(rt_default_scenes); ++k)
                printf("%s\n", rt_default_scenes[k]);

            return 0;
        } else if(strcmp(a, "--help") == 0) {
            rt_usage(argv[0]);
            return 0;
        } else if(a[0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", a);
            rt_usage(argv[0]);
            return 2;
        } else {
            /*
             * Refuse rather than drop: a scene the run quietly leaves out is a
             * fixture with no result, and so nothing asserted about it at all.
             */
            if(rt_cfg_nscenes >= RT_MAX_SCENES) {
                fprintf(stderr, "More than RT_MAX_SCENES (%d) scenes named; raise it rather than run a short set\n", RT_MAX_SCENES);
                return 2;
            }

            rt_cfg_scenes[rt_cfg_nscenes++] = a;
        }
    }

    if(rt_cfg_nscenes == 0) {
        for(size_t k = 0; k < BR_ASIZE(rt_default_scenes); ++k)
            rt_cfg_scenes[rt_cfg_nscenes++] = rt_default_scenes[k];
    }

    if(strcmp(rt_cfg_device, "glrend") != 0 && strcmp(rt_cfg_device, "glrend1x") != 0 && strcmp(rt_cfg_device, "softrend") != 0) {
        fprintf(stderr, "Unknown device: %s\n", rt_cfg_device);
        return 2;
    }

    args.title    = "BRender render test";
    args.width    = rt_cfg_width;
    args.height   = rt_cfg_height;
    args.no_stats = 1;
    args.verbose  = rt_cfg_verbose;

    if(strcmp(rt_cfg_device, "softrend") == 0) {
        args.force_software   = 1;
        args.software_pm_type = rt_cfg_pm_type;
    } else {
        args.force_software     = 0;
        args.opengl_device_name = rt_cfg_device;
    }

    /*
     * An init failure - a scene that will not load, or a reference that does not
     * fit - is a failed run, not a quiet one.
     */
    if(BrDemoRunArg(&rt_dispatch, &args) != 0)
        g_failed = 1;

    return g_failed ? 1 : 0;
}
