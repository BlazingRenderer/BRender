#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>
#include <brender.h>
#include <brddi.h>
#include <brsdl3dev.h>

#include "brdemo.h"
#include "bench.h"

/*
 * We deliberately do not reach into the driver's glad loader; SDL gives us the
 * entry points, and we only need a handful.
 */
#define BENCH_GL_TIME_ELAPSED 0x88BFu
#define BENCH_GL_QUERY_RESULT 0x8866u

typedef void (*bench_pfn_GenQueries)(int n, unsigned int *ids);
typedef void (*bench_pfn_DeleteQueries)(int n, const unsigned int *ids);
typedef void (*bench_pfn_BeginQuery)(unsigned int target, unsigned int id);
typedef void (*bench_pfn_EndQuery)(unsigned int target);
typedef void (*bench_pfn_GetQueryObjectui64v)(unsigned int id, unsigned int pname, unsigned long long *params);
typedef void (*bench_pfn_Finish)(void);
typedef void (*bench_pfn_ReadPixels)(int x, int y, int w, int h, unsigned int fmt, unsigned int type, void *pixels);
typedef void (*bench_pfn_ReadBuffer)(unsigned int mode);
typedef void (*bench_pfn_BindTexture)(unsigned int target, unsigned int texture);
typedef void (*bench_pfn_GetTexImage)(unsigned int target, int level, unsigned int fmt, unsigned int type, void *pixels);
typedef void (*bench_pfn_PixelStorei)(unsigned int pname, int param);

/*
 * Ring of queries so we can read back an older frame's result while the current
 * frame is in flight, rather than stalling the pipeline on every frame.
 */
#define BENCH_QUERY_RING 4

typedef struct bench_sample {
    br_uint_64 cpu_frame_ns;
    br_uint_64 gpu_ns[BENCH_STAGE_COUNT];
    br_uint_64 cpu_scene_ns;
} bench_sample;

const char *const bench_stage_names[BENCH_STAGE_COUNT] = {
    [BENCH_STAGE_CLEAR] = "clear",
    [BENCH_STAGE_SCENE] = "scene",
};

struct gltfview_bench {
    br_demo    *demo;
    SDL_Window *window;

    br_uint_32  warmup_frames;
    br_uint_32  total_frames;
    br_boolean  finish;
    const char *csv_path;

    /* GL entry points; NULL when there is no GL context. */
    bench_pfn_GenQueries          GenQueries;
    bench_pfn_DeleteQueries       DeleteQueries;
    bench_pfn_BeginQuery          BeginQuery;
    bench_pfn_EndQuery            EndQuery;
    bench_pfn_GetQueryObjectui64v GetQueryObjectui64v;
    bench_pfn_Finish              Finish;
    bench_pfn_ReadPixels          ReadPixels;
    bench_pfn_ReadBuffer          ReadBuffer;
    bench_pfn_BindTexture         BindTexture;
    bench_pfn_GetTexImage         GetTexImage;
    bench_pfn_PixelStorei         PixelStorei;

    unsigned int query_ring[BENCH_QUERY_RING];
    br_boolean   query_pending[BENCH_QUERY_RING];
    br_int_32    query_sample[BENCH_QUERY_RING];
    bench_stage  query_stage[BENCH_QUERY_RING];
    br_uint_32   query_head;
    br_boolean   stage_query_open;

    bench_sample *samples;
    br_uint_32    nsamples;

    br_uint_32 frame_index;
    br_uint_64 last_frame_end_ns;
    br_uint_64 scene_start_ns;
    br_uint_64 scene_cpu_ns;

    br_boolean done;
    br_boolean reported;
};

static br_uint_32 bench_env_u32(const char *name, br_uint_32 fallback)
{
    const char *value = BrGetEnv(name);

    if(value == NULL || value[0] == '\0')
        return fallback;

    return (br_uint_32)BrAToI(value);
}

static br_boolean bench_env_bool(const char *name)
{
    const char *value = BrGetEnv(name);

    if(value == NULL || value[0] == '\0')
        return BR_FALSE;

    if(value[0] == '0' && value[1] == '\0')
        return BR_FALSE;

    return BR_TRUE;
}

static void bench_load_gl(gltfview_bench *self)
{
    self->GenQueries          = (bench_pfn_GenQueries)SDL_GL_GetProcAddress("glGenQueries");
    self->DeleteQueries       = (bench_pfn_DeleteQueries)SDL_GL_GetProcAddress("glDeleteQueries");
    self->BeginQuery          = (bench_pfn_BeginQuery)SDL_GL_GetProcAddress("glBeginQuery");
    self->EndQuery            = (bench_pfn_EndQuery)SDL_GL_GetProcAddress("glEndQuery");
    self->GetQueryObjectui64v = (bench_pfn_GetQueryObjectui64v)SDL_GL_GetProcAddress("glGetQueryObjectui64v");
    self->Finish              = (bench_pfn_Finish)SDL_GL_GetProcAddress("glFinish");
    self->ReadPixels          = (bench_pfn_ReadPixels)SDL_GL_GetProcAddress("glReadPixels");
    self->ReadBuffer          = (bench_pfn_ReadBuffer)SDL_GL_GetProcAddress("glReadBuffer");
    self->BindTexture         = (bench_pfn_BindTexture)SDL_GL_GetProcAddress("glBindTexture");
    self->GetTexImage         = (bench_pfn_GetTexImage)SDL_GL_GetProcAddress("glGetTexImage");
    self->PixelStorei         = (bench_pfn_PixelStorei)SDL_GL_GetProcAddress("glPixelStorei");

    if(self->GenQueries == NULL || self->BeginQuery == NULL || self->EndQuery == NULL || self->GetQueryObjectui64v == NULL)
        BrLogWarn("BENCH", "GL timer queries unavailable; GPU timings will be reported as 0.");
}

/* Read back a slot's result, if one is outstanding. Blocks until it is ready. */
static void bench_reap_slot(gltfview_bench *self, br_uint_32 slot)
{
    unsigned long long ns = 0;

    if(!self->query_pending[slot])
        return;

    self->GetQueryObjectui64v(self->query_ring[slot], BENCH_GL_QUERY_RESULT, &ns);

    if(self->query_sample[slot] >= 0)
        self->samples[self->query_sample[slot]].gpu_ns[self->query_stage[slot]] = (br_uint_64)ns;

    self->query_pending[slot] = BR_FALSE;
    self->query_sample[slot]  = -1;
}

static void bench_checksum(gltfview_bench *bench);
static void bench_checksum_gl(gltfview_bench *bench);
static void bench_checksum_software(gltfview_bench *bench);

static void bench_drain(gltfview_bench *self)
{
    if(self->GenQueries == NULL)
        return;

    for(br_uint_32 i = 0; i < BENCH_QUERY_RING; ++i)
        bench_reap_slot(self, i);
}

gltfview_bench *GLTFViewBenchAllocate(br_demo *demo)
{
    gltfview_bench *self;
    br_uint_32      frames = bench_env_u32("GLTFVIEW_BENCH_FRAMES", 0);

    if(frames == 0)
        return NULL;

    self = BrResAllocate(demo, sizeof(*self), BR_MEMORY_APPLICATION);

    self->demo          = demo;
    self->total_frames  = frames;
    self->warmup_frames = bench_env_u32("GLTFVIEW_BENCH_WARMUP", 10);

    /* The first frame has no predecessor to measure against. */
    if(self->warmup_frames < 1)
        self->warmup_frames = 1;
    self->finish   = bench_env_bool("GLTFVIEW_BENCH_FINISH");
    self->csv_path = BrGetEnv("GLTFVIEW_BENCH_CSV");
    self->samples  = BrResAllocate(self, sizeof(bench_sample) * frames, BR_MEMORY_APPLICATION);

    for(br_uint_32 i = 0; i < BENCH_QUERY_RING; ++i)
        self->query_sample[i] = -1;

    if(demo->_screen != NULL)
        self->window = BrSDL3UtilGetWindow(demo->_screen);

    if(self->window == NULL) {
        BrLogError("BENCH", "No window; the benchmark needs a live GL context.");
        BrResFree(self);
        return NULL;
    }

    if(bench_env_bool("GLTFVIEW_BENCH_NOVSYNC")) {
        if(!SDL_GL_SetSwapInterval(0))
            BrLogWarn("BENCH", "Could not disable vsync: %s", SDL_GetError());
    }

    bench_load_gl(self);

    if(self->GenQueries != NULL) {
        self->GenQueries(BENCH_QUERY_RING, self->query_ring);
        BrMemSet(self->query_pending, 0, sizeof(self->query_pending));
    }

    self->last_frame_end_ns = SDL_GetTicksNS();

    int swap_interval = -1;

    SDL_GL_GetSwapInterval(&swap_interval);

    BrLogInfo("BENCH", "Benchmarking %u frames (%u warmup), swap interval %d, glFinish %d.", self->total_frames, self->warmup_frames,
              swap_interval, self->finish);

    return self;
}

void GLTFViewBenchBeginStage(gltfview_bench *bench, bench_stage stage)
{
    br_uint_32 slot;

    if(bench == NULL)
        return;

    bench->scene_start_ns = SDL_GetTicksNS();

    if(bench->GenQueries == NULL)
        return;

    slot = bench->query_head;
    bench_reap_slot(bench, slot);

    if(bench->frame_index >= bench->warmup_frames && bench->nsamples < bench->total_frames)
        bench->query_sample[slot] = (br_int_32)bench->nsamples;

    bench->query_stage[slot] = stage;
    bench->BeginQuery(BENCH_GL_TIME_ELAPSED, bench->query_ring[slot]);
    bench->stage_query_open = BR_TRUE;
}

void GLTFViewBenchEndStage(gltfview_bench *bench, bench_stage stage)
{
    if(bench == NULL)
        return;

    bench->scene_cpu_ns = SDL_GetTicksNS() - bench->scene_start_ns;

    if(!bench->stage_query_open)
        return;

    bench->EndQuery(BENCH_GL_TIME_ELAPSED);
    bench->query_pending[bench->query_head] = BR_TRUE;
    bench->query_head                       = (bench->query_head + 1) % BENCH_QUERY_RING;
    bench->stage_query_open                 = BR_FALSE;
}

void GLTFViewBenchEndFrame(gltfview_bench *bench)
{
    br_uint_64 cpu_ns;

    if(bench == NULL)
        return;

    if(bench->Finish != NULL && bench->finish)
        bench->Finish();

    /*
     * Measured from the same point in the previous frame, so this is the full
     * frame period: it includes the present and the event pump.
     */
    cpu_ns = SDL_GetTicksNS() - bench->last_frame_end_ns;

    if(bench->frame_index >= bench->warmup_frames && bench->nsamples < bench->total_frames) {
        bench_sample *s = bench->samples + bench->nsamples;

        s->cpu_frame_ns = cpu_ns;
        s->cpu_scene_ns = bench->scene_cpu_ns;
        BrMemSet(s->gpu_ns, 0, sizeof(s->gpu_ns)); /* Filled in as queries are reaped. */
        ++bench->nsamples;
    }

    ++bench->frame_index;

    if(bench->frame_index >= bench->warmup_frames + bench->total_frames) {
        bench->done = BR_TRUE;
        bench_drain(bench);
        bench_checksum(bench);
    }

    bench->last_frame_end_ns = SDL_GetTicksNS();
}

/*
 * Hash the final frame - from the colour buffer's offscreen texture when there
 * is a context, or from the demo's own colour buffer when there is not
 * (softrend). The two are not comparable with each other; each guards its own
 * render path.
 */
static void bench_checksum(gltfview_bench *bench)
{
    /*
     * hw_accel, not GetTexImage: SDL_GL_GetProcAddress succeeds whenever libGL
     * can be loaded, context or not, so testing a GL entry point alone sends a
     * softrend run down the GL path and hashes a readback from nothing.
     */
    if(bench->demo != NULL && bench->demo->hw_accel && (bench->GetTexImage != NULL || bench->ReadPixels != NULL) && bench->window != NULL)
        bench_checksum_gl(bench);
    else
        bench_checksum_software(bench);
}

/*
 * Read back the final frame and report a checksum plus the fraction of the
 * frame that is not the clear colour. The checksum is a regression guard: a
 * change to the render path must not alter it.
 */
static void bench_checksum_gl(gltfview_bench *bench)
{
    int        w = 0, h = 0;
    br_uint_8 *px;
    br_uint_64 hash    = 1469598103934665603ull;
    br_uint_32 covered = 0, total;

    SDL_GetWindowSizeInPixels(bench->window, &w, &h);

    if(w <= 0 || h <= 0)
        return;

    px = BrResAllocate(bench, (br_size_t)w * h * 4, BR_MEMORY_APPLICATION);

    /*
     * glrend renders into the colour buffer's own FBO and only blits it to the
     * window at swap time, which is after this runs, so on glrend the FBO
     * texture is the only correct source - reading GL_BACK there gives whatever
     * the swap left behind, i.e. undefined. glrend1x is GL 1.x with no FBOs and
     * exposes no texture token, so fall back to the window back buffer, which
     * is exactly right for it.
     */
    {
        br_uint_32 tex          = 0;
        br_boolean from_texture = BR_FALSE;

        if(bench->PixelStorei != NULL)
            bench->PixelStorei(0x0CF5 /* GL_PACK_ALIGNMENT */, 1);

        if(bench->BindTexture != NULL && bench->GetTexImage != NULL &&
           ObjectQuery(bench->demo->colour_buffer, &tex, BRT_OPENGL_TEXTURE_U32) == BRE_OK && tex != 0) {
            bench->BindTexture(0x0DE1 /* GL_TEXTURE_2D */, tex);
            bench->GetTexImage(0x0DE1 /* GL_TEXTURE_2D */, 0, 0x1908 /* GL_RGBA */, 0x1401 /* GL_UNSIGNED_BYTE */, px);
            from_texture = BR_TRUE;
        }

        if(!from_texture) {
            if(bench->ReadPixels == NULL) {
                BrLogWarn("BENCH", "No glReadPixels or colour texture; checksum unavailable.");
                BrResFree(px);
                return;
            }

            if(bench->ReadBuffer != NULL)
                bench->ReadBuffer(0x0405 /* GL_BACK */);

            bench->ReadPixels(0, 0, w, h, 0x1908 /* GL_RGBA */, 0x1401 /* GL_UNSIGNED_BYTE */, px);
        }
    }

    total = (br_uint_32)w * (br_uint_32)h;
    for(br_uint_32 i = 0; i < total; ++i) {
        const br_uint_8 *p = px + (br_size_t)i * 4;

        for(int c = 0; c < 4; ++c) {
            hash ^= p[c];
            hash *= 1099511628211ull;
        }

        if(p[0] || p[1] || p[2])
            ++covered;
    }

    printf("BENCH checksum=%016llx coverage=%.2f%% (%u/%u px at %dx%d)\n", (unsigned long long)hash,
           100.0 * (double)covered / (double)total, covered, total, w, h);

    {
        const char *path = BrGetEnv("GLTFVIEW_BENCH_PPM");

        if(path != NULL && path[0] != '\0') {
            br_pixelmap *pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, w, h, NULL, BR_PMAF_NORMAL);

            if(pm != NULL) {
                /*
                 * glReadPixels hands back bottom-up r,g,b,a and PPM wants
                 * top-down; BR_PMT_RGBA_8888 stores b,g,r,a, so copy the rows
                 * in reverse and swap red and blue.
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

                BrFmtImageSave(path, pm, BR_FMT_IMAGE_PPM);
                BrPixelmapFree(pm);
                printf("BENCH ppm=%s\n", path);
            } else {
                BrLogWarn("BENCH", "Could not allocate a pixelmap for %s.", path);
            }
        }
    }

    BrResFree(px);
}

/*
 * The value of pixel x within a packed row, used to decide whether the pixel
 * counts as covered. Sub-byte types pack the first pixel of a byte into its
 * most significant field.
 */
static br_uint_32 bench_pixel_value(const br_uint_8 *row, br_int_32 x, br_uint_32 bits)
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
 * Fold a pixelmap's pixels into hash, optionally counting the pixels that are
 * not zero.
 *
 * BrPixelmapPixelSize() returns the pixel size in *bits*, and a sub-byte type
 * packs several pixels into a byte, so a row is not width * pixel size bytes
 * long. Treating the bit count as a byte count walks off the end of the row
 * and hashes whatever follows the buffer, which makes the checksum depend on
 * the binary's memory layout instead of on what was rendered.
 */
static br_uint_64 bench_hash_pixelmap(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered)
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
            if(bench_pixel_value(row, x, bits) != 0)
                ++*covered;
        }
    }

    return hash;
}

/*
 * The software path renders into the demo's colour buffer rather than a GL
 * framebuffer, so hash that. Its pixels are a raw buffer with no implicit
 * format conversion, and an indexed buffer means nothing without its palette,
 * so fold the palette into the same value.
 */
static void bench_checksum_software(gltfview_bench *bench)
{
    br_pixelmap *pm      = bench->demo != NULL ? bench->demo->colour_buffer : NULL;
    br_pixelmap *pal     = NULL;
    br_uint_64   hash    = 1469598103934665603ull;
    br_uint_32   covered = 0, total;

    if(pm == NULL || pm->pixels == NULL || pm->width == 0 || pm->height == 0) {
        BrLogWarn("BENCH", "No colour buffer to checksum - the software run has no guard.");
        return;
    }

    if(BrPixelmapPixelSize(pm) == 0) {
        BrLogWarn("BENCH", "Unknown pixel type %u - the software run has no guard.", (unsigned int)pm->type);
        return;
    }

    hash = bench_hash_pixelmap(hash, pm, &covered);

    if((pal = pm->map) != NULL && pal->pixels != NULL && pal->width != 0 && pal->height != 0 && BrPixelmapPixelSize(pal) != 0)
        hash = bench_hash_pixelmap(hash, pal, NULL);

    total = (br_uint_32)pm->width * (br_uint_32)pm->height;

    printf("BENCH checksum=%016llx coverage=%.2f%% (%u/%u px at %dx%d, software, type=%u)\n", (unsigned long long)hash,
           100.0 * (double)covered / (double)total, covered, total, (int)pm->width, (int)pm->height, (unsigned int)pm->type);

    /*
     * Local debug dump of the software colour buffer, so a textured frame can be
     * looked at without a GL context. The indices are written as greys (see
     * below) and saved through the engine's PNG writer. Scope is deliberately
     * one env var and one write; the real compare mode is being built
     * separately.
     */
    {
        const char *path = BrGetEnv("GLTFVIEW_BENCH_SOFT_PNG");

        if(path != NULL && path[0] != '\0') {
            /*
             * The software colour buffer is BR_PMT_INDEX_8 with its palette held
             * as a device CLUT (BrPixelmapPaletteSet), not as pm->map, so the
             * generic image writer has no palette to resolve and refuses the
             * conversion. Render the indices as greys instead: enough to see
             * whether texture detail is landing. This is a local debug view, not
             * the regression comparison, which is the checksum above.
             */
            br_pixelmap *dst = BrPixelmapAllocate(BR_PMT_RGBA_8888_ARR, pm->width, pm->height, NULL, BR_PMAF_NORMAL);

            if(dst != NULL) {
                br_uint_32 bits      = BrPixelmapPixelSize(pm);
                br_int_32  row_bytes = (br_int_32)(((br_uint_64)pm->width * bits + 7) / 8);
                br_int_32  stride    = pm->row_bytes != 0 ? pm->row_bytes : row_bytes;

                for(br_int_32 y = 0; y < pm->height; ++y) {
                    const br_uint_8 *src = (const br_uint_8 *)pm->pixels + (br_size_t)y * stride;
                    br_uint_8       *d   = (br_uint_8 *)dst->pixels + (br_size_t)y * dst->row_bytes;

                    for(br_int_32 x = 0; x < pm->width; ++x) {
                        br_uint_8 v = (br_uint_8)bench_pixel_value(src, x, bits);

                        d[x * 4 + 0] = v;
                        d[x * 4 + 1] = v;
                        d[x * 4 + 2] = v;
                        d[x * 4 + 3] = 0xff;
                    }
                }

                if(BrFmtImageSave(path, dst, BR_FMT_IMAGE_PNG))
                    printf("BENCH soft-png=%s\n", path);
                else
                    BrLogWarn("BENCH", "Could not write software frame to %s.", path);

                BrPixelmapFree(dst);
            } else {
                BrLogWarn("BENCH", "Could not allocate a pixelmap for %s.", path);
            }
        }
    }
}

br_boolean GLTFViewBenchShouldQuit(gltfview_bench *bench)
{
    return bench != NULL && bench->done;
}

static int bench_cmp_u64(const void *a, const void *b)
{
    br_uint_64 va = *(const br_uint_64 *)a;
    br_uint_64 vb = *(const br_uint_64 *)b;

    if(va < vb)
        return -1;

    if(va > vb)
        return 1;

    return 0;
}

typedef struct bench_stats {
    br_uint_64 min;
    br_uint_64 p50;
    br_uint_64 p95;
    br_uint_64 p99;
    br_uint_64 max;
    double     mean;
} bench_stats;

static bench_stats bench_compute_stats(br_uint_64 *values, br_uint_32 n, br_uint_64 *scratch)
{
    bench_stats st  = {0};
    br_uint_64  sum = 0;

    if(n == 0)
        return st;

    BrMemCpy(scratch, values, sizeof(br_uint_64) * n);
    BrQsort(scratch, n, sizeof(br_uint_64), bench_cmp_u64);

    for(br_uint_32 i = 0; i < n; ++i)
        sum += values[i];

    st.min  = scratch[0];
    st.max  = scratch[n - 1];
    st.p50  = scratch[(br_uint_32)(n * 0.50)];
    st.p95  = scratch[(br_uint_32)((n - 1) * 0.95)];
    st.p99  = scratch[(br_uint_32)((n - 1) * 0.99)];
    st.mean = (double)sum / (double)n;

    return st;
}

/*
 * Ground truth on how many draw calls the frame actually issued, straight from
 * the driver's own counters. Only meaningful after a render.
 */
static void bench_report_renderer_stats(void)
{
    br_renderer   *renderer = BrV1dbRendererQuery();
    br_token_value tv[]     = {
        {BRT_TRIANGLES_RENDERED_COUNT_U32, {.i32 = 0}},
        {BRT_VERTICES_RENDERED_COUNT_U32,  {.i32 = 0}},
        {BRT_TRIANGLES_DRAWN_COUNT_U32,    {.i32 = 0}},
        {BRT_OPAQUE_DRAW_COUNT_U32,        {.i32 = 0}},
        {BRT_TRANSPARENT_DRAW_COUNT_U32,   {.i32 = 0}},
        {BRT_FACE_GROUP_COUNT_U32,         {.i32 = 0}},
        {BR_NULL_TOKEN,                    {.i32 = 0}},
    };
    br_int_32 count = 0;

    if(renderer == NULL)
        return;

    if(ObjectQueryMany(renderer, tv, NULL, 0, &count) != BRE_OK)
        return;

    printf("BENCH renderer tris_rendered=%d verts_rendered=%d tris_drawn=%d opaque_draws=%d transparent_draws=%d face_groups=%d\n",
           tv[0].v.i32, tv[1].v.i32, tv[2].v.i32, tv[3].v.i32, tv[4].v.i32, tv[5].v.i32);
}

void GLTFViewBenchReport(gltfview_bench *bench)
{
    br_uint_64 *scratch;
    br_uint_64 *col;
    bench_stats frame_st, scene_cpu_st, stage_st[BENCH_STAGE_COUNT];
    br_uint_32  n;

    if(bench == NULL || bench->reported)
        return;

    bench->reported = BR_TRUE;

    if(!bench->done)
        bench_drain(bench);

    n = bench->nsamples;

    if(n == 0) {
        BrLogWarn("BENCH", "No samples recorded.");
        return;
    }

    scratch = BrResAllocate(bench, sizeof(br_uint_64) * n, BR_MEMORY_APPLICATION);
    col     = BrResAllocate(bench, sizeof(br_uint_64) * n, BR_MEMORY_APPLICATION);

    for(br_uint_32 i = 0; i < n; ++i)
        col[i] = bench->samples[i].cpu_frame_ns;
    frame_st = bench_compute_stats(col, n, scratch);

    for(br_uint_32 i = 0; i < n; ++i)
        col[i] = bench->samples[i].cpu_scene_ns;
    scene_cpu_st = bench_compute_stats(col, n, scratch);

    for(br_uint_32 s = 0; s < BENCH_STAGE_COUNT; ++s) {
        for(br_uint_32 i = 0; i < n; ++i)
            col[i] = bench->samples[i].gpu_ns[s];

        stage_st[s] = bench_compute_stats(col, n, scratch);
    }

    int win_w = 0, win_h = 0;

    if(bench->window != NULL)
        SDL_GetWindowSizeInPixels(bench->window, &win_w, &win_h);

    printf("BENCH frames=%u warmup=%u\n", n, bench->warmup_frames);
    printf("BENCH render_target=%dx%d window=%dx%d\n", bench->demo->colour_buffer != NULL ? bench->demo->colour_buffer->width : -1,
           bench->demo->colour_buffer != NULL ? bench->demo->colour_buffer->height : -1, win_w, win_h);
    printf("BENCH %-14s %10s %10s %10s %10s %10s %10s\n", "metric", "min_ms", "mean_ms", "p50_ms", "p95_ms", "p99_ms", "max_ms");
    printf("BENCH %-14s %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n", "frame_cpu", frame_st.min / 1e6, frame_st.mean / 1e6,
           frame_st.p50 / 1e6, frame_st.p95 / 1e6, frame_st.p99 / 1e6, frame_st.max / 1e6);
    printf("BENCH %-14s %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n", "scene_cpu", scene_cpu_st.min / 1e6, scene_cpu_st.mean / 1e6,
           scene_cpu_st.p50 / 1e6, scene_cpu_st.p95 / 1e6, scene_cpu_st.p99 / 1e6, scene_cpu_st.max / 1e6);
    for(br_uint_32 s = 0; s < BENCH_STAGE_COUNT; ++s)
        printf("BENCH %-14s %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f\n", bench_stage_names[s], stage_st[s].min / 1e6,
               stage_st[s].mean / 1e6, stage_st[s].p50 / 1e6, stage_st[s].p95 / 1e6, stage_st[s].p99 / 1e6, stage_st[s].max / 1e6);

    printf("BENCH mean_fps=%.2f\n", frame_st.mean > 0 ? 1e9 / frame_st.mean : 0.0);

    bench_report_renderer_stats();

    if(bench->csv_path != NULL) {
        FILE *fp = fopen(bench->csv_path, "w");

        if(fp == NULL) {
            BrLogWarn("BENCH", "Could not open %s for writing.", bench->csv_path);
        } else {
            fprintf(fp, "frame,cpu_frame_ns,cpu_scene_ns");
            for(br_uint_32 s = 0; s < BENCH_STAGE_COUNT; ++s)
                fprintf(fp, ",gpu_%s_ns", bench_stage_names[s]);
            fprintf(fp, "\n");

            for(br_uint_32 i = 0; i < n; ++i) {
                fprintf(fp, "%u,%llu,%llu", i, (unsigned long long)bench->samples[i].cpu_frame_ns,
                        (unsigned long long)bench->samples[i].cpu_scene_ns);

                for(br_uint_32 s = 0; s < BENCH_STAGE_COUNT; ++s)
                    fprintf(fp, ",%llu", (unsigned long long)bench->samples[i].gpu_ns[s]);

                fprintf(fp, "\n");
            }

            fclose(fp);
            printf("BENCH csv=%s\n", bench->csv_path);
        }
    }

    BrResFree(scratch);
    BrResFree(col);
}
