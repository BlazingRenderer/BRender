#include <ctype.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <malloc.h> /* alloca() */
#else
#include <alloca.h>
#endif

#include <brender.h>
#include <brcapture.h>
#include <parg.h>

#include "bench.h"

/*
 * Command line and headless capture support for devpmtest, so each test can be
 * rendered once, deterministically, and its frame written out for the device
 * front screens to be compared.
 */

/* FNV-1a's offset basis, the hash gltfview's benchmark starts its frames from. */
#define DEVPMTEST_FNV_OFFSET 1469598103934665603ull

/*
 * The long options. Values are arbitrary so long as they are positive and
 * distinct, and the table is walked to turn one back into a name.
 */
#define DEVPMTEST_OPT_RUN_TEST           301
#define DEVPMTEST_OPT_RUN_ALL            302
#define DEVPMTEST_OPT_FRONT_SCREEN       303
#define DEVPMTEST_OPT_OPENGL_DEVICE_NAME 304
#define DEVPMTEST_OPT_FRAMES             305
#define DEVPMTEST_OPT_DT_MS              306
#define DEVPMTEST_OPT_DUMP               307
#define DEVPMTEST_OPT_DUMP_DIR           308
#define DEVPMTEST_OPT_CHECKSUM           309
#define DEVPMTEST_OPT_HELP               310

static const struct parg_option bench_options[] = {
    {.name = "run-test",           .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_RUN_TEST          },
    {.name = "run-all",            .has_arg = PARG_NOARG,  .flag = NULL, .val = DEVPMTEST_OPT_RUN_ALL           },
    {.name = "front-screen",       .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_FRONT_SCREEN      },
    {.name = "opengl-device-name", .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_OPENGL_DEVICE_NAME},
    {.name = "frames",             .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_FRAMES            },
    {.name = "dt-ms",              .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_DT_MS             },
    {.name = "dump",               .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_DUMP              },
    {.name = "dump-dir",           .has_arg = PARG_REQARG, .flag = NULL, .val = DEVPMTEST_OPT_DUMP_DIR          },
    {.name = "checksum",           .has_arg = PARG_NOARG,  .flag = NULL, .val = DEVPMTEST_OPT_CHECKSUM          },
    {.name = "help",               .has_arg = PARG_NOARG,  .flag = NULL, .val = DEVPMTEST_OPT_HELP              },
    {.name = NULL,                 .has_arg = 0,           .flag = NULL, .val = 0                               },
};

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

/* The row of bench_options[] a value belongs to, for a message parg cannot spell. */
static const struct parg_option *bench_option_for_value(int val)
{
    for(const struct parg_option *o = bench_options; o->name != NULL; ++o) {
        if(o->val == val)
            return o;
    }

    return NULL;
}

devpmtest_bench_parse DevpmTestBenchParse(int argc, char **argv, devpmtest_bench **bench)
{
    struct devpmtest_bench *b                   = &bench_storage;
    br_boolean              front_set           = BR_FALSE;
    br_boolean              used_capture_option = BR_FALSE;
    br_boolean              run_test_set        = BR_FALSE;
    struct parg_state       ps;
    char                  **tmp_argv;
    int                     optend;

    *bench = NULL;
    BrMemSet(b, 0, sizeof(*b));
    b->frames   = 1;
    b->delta_ms = 1000.0f / 60.0f;

    /*
     * parg_reorder() permutes the array it is given, so parse a copy and leave
     * the caller's argv alone. A command line is small and this runs before
     * BrBegin(), so alloca() is enough; nothing here allocates.
     */
    tmp_argv = alloca(sizeof(char *) * (argc + 1));
    BrMemCpy(tmp_argv, argv, argc * sizeof(char *));
    tmp_argv[argc] = NULL;

    parg_init(&ps);

    if((optend = parg_reorder(argc, tmp_argv, ":", bench_options)) < 0)
        goto bad;

    /*
     * parg_reorder() leaves a "--" at the end of the option region, where the
     * parse then stops. This tool has never accepted one, so put it back among
     * the stray arguments below rather than let it end the options silently.
     */
    if(optend > 1 && strcmp(tmp_argv[optend - 1], "--") == 0)
        --optend;

    for(int c; (c = parg_getopt_long(&ps, optend, tmp_argv, ":", bench_options, NULL)) != -1;) {
        switch(c) {
            case DEVPMTEST_OPT_HELP:
                bench_usage(stdout);
                return DEVPMTEST_BENCH_HELP;

            case DEVPMTEST_OPT_RUN_TEST:
                b->test_index = BrAToI(ps.optarg);
                b->have_run   = BR_TRUE;
                run_test_set  = BR_TRUE;
                break;

            case DEVPMTEST_OPT_RUN_ALL:
                b->run_all  = BR_TRUE;
                b->have_run = BR_TRUE;
                break;

            case DEVPMTEST_OPT_FRONT_SCREEN:
                if(strcmp(ps.optarg, "soft") == 0)
                    b->front_screen = DEVPMTEST_FRONT_SOFT;
                else if(strcmp(ps.optarg, "gl") == 0)
                    b->front_screen = DEVPMTEST_FRONT_GL;
                else if(strcmp(ps.optarg, "vulkan") == 0)
                    b->front_screen = DEVPMTEST_FRONT_VULKAN;
                else {
                    fprintf(stderr, "devpmtest: unknown front screen \"%s\" (want soft, gl or vulkan).\n", ps.optarg);
                    goto bad;
                }

                front_set = BR_TRUE;
                break;

            case DEVPMTEST_OPT_OPENGL_DEVICE_NAME:
                b->opengl_device_name = ps.optarg;
                break;

            case DEVPMTEST_OPT_FRAMES:
                b->frames = (br_uint_32)BrAToI(ps.optarg);
                if(b->frames == 0) {
                    fprintf(stderr, "devpmtest: --frames must be at least 1.\n");
                    goto bad;
                }

                used_capture_option = BR_TRUE;
                break;

            case DEVPMTEST_OPT_DT_MS:
                b->delta_ms         = BrAToF(ps.optarg);
                used_capture_option = BR_TRUE;
                break;

            case DEVPMTEST_OPT_DUMP:
                b->dump_path        = ps.optarg;
                used_capture_option = BR_TRUE;
                break;

            case DEVPMTEST_OPT_DUMP_DIR:
                b->dump_dir         = ps.optarg;
                used_capture_option = BR_TRUE;
                break;

            case DEVPMTEST_OPT_CHECKSUM:
                b->checksum         = BR_TRUE;
                used_capture_option = BR_TRUE;
                break;

            case ':': {
                /*
                 * parg reports an option missing its required argument and an
                 * option given an argument it does not take the same way; the
                 * table says which this was, and only one has a message of its
                 * own.
                 */
                const struct parg_option *opt = bench_option_for_value(ps.optopt);

                if(opt != NULL && opt->has_arg == PARG_REQARG)
                    fprintf(stderr, "devpmtest: option --%s requires an argument.\n", opt->name);
                else
                    fprintf(stderr, "devpmtest: unknown option \"%s\".\n", tmp_argv[ps.optind - 1]);
                goto bad;
            }

            case 1:
            case '?':
            default:
                /*
                 * '?' is an option parg could not match, and for it optarg is
                 * NULL while optind has already stepped past the token; 1 is
                 * an element that is not an option at all, and optarg points
                 * at it. This tool has no argument either way.
                 */
                fprintf(stderr, "devpmtest: unknown option \"%s\".\n", ps.optarg != NULL ? ps.optarg : tmp_argv[ps.optind - 1]);
                goto bad;
        }
    }

    /* Arguments rather than options are just as unknown as they ever were. */
    if(optend < argc) {
        fprintf(stderr, "devpmtest: unknown option \"%s\".\n", tmp_argv[optend]);
        goto bad;
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
 * Hash a pixelmap and count the pixels that are not the clear colour. brcapture
 * hashes it as RGBA8, the layout the PNG writer consumes, so the same frame
 * hashes the same way whatever type and channel order the front screen read it
 * back in.
 */
static void bench_capture_checksum(br_pixelmap *pm)
{
    br_uint_64 hash    = DEVPMTEST_FNV_OFFSET;
    br_uint_32 covered = 0;
    br_uint_32 total;

    if(BrPixelmapPixelSize(pm) == 0) {
        BrLogWarn("DEVPMTEST", "Unknown captured pixel type %u; no checksum.", (unsigned)pm->type);
        return;
    }

    hash  = BrCaptureHashPixelmapRGBA(hash, pm, &covered);
    total = (br_uint_32)pm->width * (br_uint_32)pm->height;

    printf("DEVPMTEST checksum=%016llx coverage=%.2f%% (%u/%u px at %dx%d, capture type=%u)\n", (unsigned long long)hash,
           100.0 * (double)covered / (double)total, covered, total, (int)pm->width, (int)pm->height, (unsigned)pm->type);
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
