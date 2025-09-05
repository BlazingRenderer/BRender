/*
 * Private device pixelmap structure
 */
#ifndef _DEVPIXMP_H_
#define _DEVPIXMP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef union br_quirks_gl {
    struct {
        br_uint_32 disable_anisotropic_filtering : 1;
        br_uint_32 orphan_model_buffers : 1;
        br_uint_32 reserved : 30;
    };
    br_uint_32 value;
} br_quirks_gl;

#ifdef BR_DEVICE_PIXELMAP_PRIVATE

/*
 * Private state of device pixelmap
 */
typedef struct br_device_pixelmap {

    /*
     * Dispatch table
     */
    const struct br_device_pixelmap_dispatch *dispatch;

    /*
     * Standard handle identifier
     */
    const char *pm_identifier;

    /** Standard pixelmap members (not including identifier) **/

    BR_PIXELMAP_MEMBERS

    /** End of br_pixelmap fields **/

    br_device          *device;
    br_output_facility *output_facility;

    /*
     * Type of buffer (when matched)
     */
    br_token use_type;

    /*
     * No. MSAA samples
     */
    br_int_32 msaa_samples;

    /*
     * Pointer to renderer currently opened on this pixelmap (N.B. This is only set on the screen
     * pixelmap)
     */
    struct br_renderer *renderer;

    /*
     * Current screen pixelmap. Valid on ALL types. The screen points to itself.
     * NB: This is mainly used to retrieve the context-level OpenGL state.
     */
    struct br_device_pixelmap *screen;

    /*
     * No. dependent pixelmaps.
     * For screens, this is everything.
     */
    br_int_32 num_refs;

    /* OpenGL crap */
    union {
        struct {
            /*
             * System-specific OpenGL function pointers.
             */
            br_device_gl_ext_procs ext_procs;

            /*
             * Device-wide VIDEO instance.
             */
            VIDEO video;

            /*
             * OpenGL context
             */
            void *gl_context;

            const char *gl_version;
            const char *gl_vendor;
            const char *gl_renderer;

            GLint  gl_num_extensions;
            char **gl_extensions;

            br_quirks_gl quirks;

            GLuint tex_white;
            GLuint tex_checkerboard;

            br_font_gl font_fixed3x5;
            br_font_gl font_prop4x6;
            br_font_gl font_prop7x9;
        } asFront;
        struct {
            struct br_device_pixelmap *depthbuffer;
            GLuint                     glFbo;
            GLuint                     glTex;
            br_device_clut            *clut;
        } asBack;
        struct {
            struct br_device_pixelmap *backbuffer;
            GLuint                     glDepth;
        } asDepth;
        struct {
            struct br_device_pixelmap *parent;
        } asSub;
    };
} br_device_pixelmap;

#endif

#ifdef __cplusplus
};
#endif
#endif
