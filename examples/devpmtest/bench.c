#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <brender.h>

#include "bench.h"

/*
 * Command line and headless capture support for devpmtest, so each test can be
 * rendered once, deterministically, and its frame written out for the device
 * front screens to be compared.
 */

/* FNV-1a, the same constants gltfview's benchmark hashes its frames with. */
#define DEVPMTEST_FNV_OFFSET 1469598103934665603ull
#define DEVPMTEST_FNV_PRIME  1099511628211ull

struct devpmtest_bench {
    devpmtest_front_screen front_screen;
    const char            *opengl_device_name;
    br_boolean             run_all;
    br_boolean             have_run;
    br_int_32              test_index;
    br_uint_32             frames;
    float                  delta_ms;
    const char            *dump_path;
    const char            *dump_dir;
    br_boolean             checksum;
};

/* Process-lifetime options; parse fills this in and hands out a pointer to it. */
static struct devpmtest_bench bench_storage;

static void bench_usage(FILE *out)
{
    fprintf(out, "Usage: devpmtest [options]\n"
                 "\n"
                 "Without a run mode the tool runs its interactive test browser.\n"
                 "\n"
                 "  --run-test N        Render only tests[N] headlessly and exit.\n"
                 "  --run-all           Render every test headlessly and exit.\n"
                 "  --front-screen STR  Front screen: soft (default), gl or vulkan.\n"
                 "  --opengl-device-name NAME\n"
                 "                      OpenGL device driver for --front-screen gl (default\n"
                 "                      glrend; glrend1x is the legacy GL 1.2 one).\n"
                 "  --frames N          Frames to render per test before capture (default 1).\n"
                 "  --dt-ms MS          Fixed per-frame delta in ms (default 1000/60).\n"
                 "  --dump PATH         With --run-test, save the captured frame to PATH as PNG.\n"
                 "  --dump-dir DIR      With --run-all, save DIR/<NN>-<slug>.png per test.\n"
                 "  --checksum          Print the captured frame's FNV-1a checksum and coverage.\n"
                 "  --help              Display this message.\n");
}

static const char *bench_arg_value(int argc, char **argv, int *i, const char *opt)
{
    if(*i + 1 >= argc) {
        fprintf(stderr, "devpmtest: option %s requires an argument.\n", opt);
        return NULL;
    }

    return argv[++*i];
}

devpmtest_bench_parse DevpmTestBenchParse(int argc, char **argv, devpmtest_bench **bench)
{
    struct devpmtest_bench *b                   = &bench_storage;
    br_boolean              front_set           = BR_FALSE;
    br_boolean              used_capture_option = BR_FALSE;
    br_boolean              run_test_set        = BR_FALSE;

    *bench = NULL;
    BrMemSet(b, 0, sizeof(*b));
    b->frames   = 1;
    b->delta_ms = 1000.0f / 60.0f;

    for(int i = 1; i < argc; ++i) {
        const char *opt = argv[i];
        const char *val;

        if(strcmp(opt, "--help") == 0) {
            bench_usage(stdout);
            return DEVPMTEST_BENCH_HELP;
        } else if(strcmp(opt, "--run-test") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->test_index = BrAToI(val);
            b->have_run   = BR_TRUE;
            run_test_set  = BR_TRUE;
        } else if(strcmp(opt, "--run-all") == 0) {
            b->run_all  = BR_TRUE;
            b->have_run = BR_TRUE;
        } else if(strcmp(opt, "--front-screen") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            if(strcmp(val, "soft") == 0)
                b->front_screen = DEVPMTEST_FRONT_SOFT;
            else if(strcmp(val, "gl") == 0)
                b->front_screen = DEVPMTEST_FRONT_GL;
            else if(strcmp(val, "vulkan") == 0)
                b->front_screen = DEVPMTEST_FRONT_VULKAN;
            else {
                fprintf(stderr, "devpmtest: unknown front screen \"%s\" (want soft, gl or vulkan).\n", val);
                goto bad;
            }

            front_set = BR_TRUE;
        } else if(strcmp(opt, "--opengl-device-name") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->opengl_device_name = val;
        } else if(strcmp(opt, "--frames") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->frames = (br_uint_32)BrAToI(val);
            if(b->frames == 0) {
                fprintf(stderr, "devpmtest: --frames must be at least 1.\n");
                goto bad;
            }

            used_capture_option = BR_TRUE;
        } else if(strcmp(opt, "--dt-ms") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->delta_ms         = BrAToF(val);
            used_capture_option = BR_TRUE;
        } else if(strcmp(opt, "--dump") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->dump_path        = val;
            used_capture_option = BR_TRUE;
        } else if(strcmp(opt, "--dump-dir") == 0) {
            if((val = bench_arg_value(argc, argv, &i, opt)) == NULL)
                goto bad;

            b->dump_dir         = val;
            used_capture_option = BR_TRUE;
        } else if(strcmp(opt, "--checksum") == 0) {
            b->checksum         = BR_TRUE;
            used_capture_option = BR_TRUE;
        } else {
            fprintf(stderr, "devpmtest: unknown option \"%s\".\n", opt);
            goto bad;
        }
    }

    if(!b->have_run) {
        if(used_capture_option) {
            fprintf(stderr, "devpmtest: --frames, --dt-ms, --dump, --dump-dir and --checksum need --run-test or --run-all.\n");
            goto bad;
        }

        /*
         * Keep the argument-less interactive launch on the soft front screen, with
         * no accelerator token, as master does.
         */
        if(!front_set)
            b->front_screen = DEVPMTEST_FRONT_SOFT;

        *bench = b;
        return DEVPMTEST_BENCH_INTERACTIVE;
    }

    if(b->run_all && run_test_set) {
        fprintf(stderr, "devpmtest: --run-all and --run-test are mutually exclusive.\n");
        goto bad;
    }

    if(!front_set)
        b->front_screen = DEVPMTEST_FRONT_SOFT;

    *bench = b;
    return DEVPMTEST_BENCH_CAPTURE;

bad:
    bench_usage(stderr);
    return DEVPMTEST_BENCH_ERROR;
}

devpmtest_front_screen DevpmTestBenchFrontScreen(const devpmtest_bench *bench)
{
    return bench->front_screen;
}

const char *DevpmTestBenchOpenGLDeviceName(const devpmtest_bench *bench)
{
    return bench->opengl_device_name;
}

br_boolean DevpmTestBenchRunAll(const devpmtest_bench *bench)
{
    return bench->run_all;
}

br_int_32 DevpmTestBenchTestIndex(const devpmtest_bench *bench)
{
    return bench->test_index;
}

br_uint_32 DevpmTestBenchFrames(const devpmtest_bench *bench)
{
    return bench->frames;
}

float DevpmTestBenchDeltaMs(const devpmtest_bench *bench)
{
    return bench->delta_ms;
}

const char *DevpmTestBenchDumpPath(const devpmtest_bench *bench)
{
    return bench->dump_path;
}

const char *DevpmTestBenchDumpDir(const devpmtest_bench *bench)
{
    return bench->dump_dir;
}

br_boolean DevpmTestBenchChecksum(const devpmtest_bench *bench)
{
    return bench->checksum;
}

/* A test name as a file name: lower case, runs of other characters as one '-'. */
static void bench_slug(const char *name, char *out, br_size_t out_size)
{
    br_size_t j = 0;

    for(br_size_t i = 0; name[i] != '\0' && j + 1 < out_size; ++i) {
        unsigned char c = (unsigned char)name[i];

        if(isalnum(c))
            out[j++] = (char)tolower(c);
        else if(j > 0 && out[j - 1] != '-')
            out[j++] = '-';
    }

    while(j > 0 && out[j - 1] == '-')
        --j;

    out[j] = '\0';
}

const char *DevpmTestBenchAllPath(const devpmtest_bench *bench, const char *name, br_int_32 index, char *buf, br_size_t buf_size)
{
    char slug[64];
    int  n;
    int  need_sep;

    if(bench->dump_dir == NULL)
        return NULL;

    bench_slug(name, slug, sizeof(slug));

    need_sep = bench->dump_dir[0] != '\0' && bench->dump_dir[strlen(bench->dump_dir) - 1] != '/';

    n = snprintf(buf, (size_t)buf_size, "%s%s%02d-%s.png", bench->dump_dir, need_sep ? "/" : "", (int)index, slug);

    if(n < 0 || (br_size_t)n >= buf_size)
        return NULL;

    return buf;
}

/*
 * Hash a pixelmap and count the pixels that are not the clear colour. The
 * captured buffer is converted to the RGBA byte-array layout the PNG writer
 * consumes, so the same frame hashes the same way whatever type and channel
 * order the front screen read it back in.
 */
static void bench_capture_checksum(br_pixelmap *pm)
{
    br_pixelmap *rgba    = pm;
    br_pixelmap *conv    = NULL;
    br_uint_64   hash    = DEVPMTEST_FNV_OFFSET;
    br_uint_32   covered = 0;
    br_uint_32   total;
    br_uint_32   bytes;

    if(BrPixelmapPixelSize(pm) == 0) {
        BrLogWarn("DEVPMTEST", "Unknown captured pixel type %u; no checksum.", (unsigned)pm->type);
        return;
    }

    if(pm->type != BR_PMT_RGBA_8888_ARR) {
        if((conv = BrPixelmapCloneTyped(pm, BR_PMT_RGBA_8888_ARR)) == NULL) {
            BrLogWarn("DEVPMTEST", "Could not convert the captured type %u to RGBA_8888_ARR; no checksum.", (unsigned)pm->type);
            return;
        }

        rgba = conv;
    }

    bytes = BrPixelmapPixelSize(rgba) / 8;
    total = (br_uint_32)rgba->width * (br_uint_32)rgba->height;

    for(br_int_32 y = 0; y < rgba->height; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)rgba->pixels + (br_size_t)y * rgba->row_bytes;

        for(br_int_32 x = 0; x < rgba->width; ++x) {
            const br_uint_8 *p = row + (br_size_t)x * bytes;

            for(br_uint_32 c = 0; c < bytes; ++c) {
                hash ^= p[c];
                hash *= DEVPMTEST_FNV_PRIME;
            }

            if(p[0] || p[1] || p[2])
                ++covered;
        }
    }

    printf("DEVPMTEST checksum=%016llx coverage=%.2f%% (%u/%u px at %dx%d, capture type=%u)\n", (unsigned long long)hash,
           100.0 * (double)covered / (double)total, covered, total, (int)pm->width, (int)pm->height, (unsigned)pm->type);

    if(conv != NULL)
        BrPixelmapFree(conv);
}

void DevpmTestBenchCapture(br_pixelmap *colour_buffer, const char *png_path, br_boolean checksum)
{
    br_pixelmap *pm;

    if(colour_buffer == NULL || colour_buffer->width <= 0 || colour_buffer->height <= 0) {
        BrLogWarn("DEVPMTEST", "No colour buffer to capture.");
        return;
    }

    /*
     * The same copy F5 makes: a device framebuffer cannot be locked for direct
     * access on every driver, so take a host copy first.
     */
    pm = BrPixelmapAllocate(colour_buffer->type, colour_buffer->width, colour_buffer->height, NULL, BR_PMAF_NORMAL);
    if(pm == NULL) {
        BrLogError("DEVPMTEST", "Could not allocate the capture pixelmap (%dx%d, type %u).", (int)colour_buffer->width,
                   (int)colour_buffer->height, (unsigned)colour_buffer->type);
        return;
    }

    BrPixelmapCopy(pm, colour_buffer);

    if(checksum)
        bench_capture_checksum(pm);

    if(png_path != NULL) {
        if(BrFmtImageSave(png_path, pm, BR_FMT_IMAGE_PNG))
            printf("DEVPMTEST png=%s\n", png_path);
        else
            BrLogError("DEVPMTEST", "Could not write the captured frame to %s.", png_path);
    }

    BrPixelmapFree(pm);
}
