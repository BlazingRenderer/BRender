/*
 * Private device pixelmap structure
 */
#ifndef _DEVPIXMP_H_
#define _DEVPIXMP_H_

#ifdef __cplusplus
extern "C" {
#endif

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

    struct br_primitive_library *primitive_library;

    /*
     * Type of buffer (when matched)
     */
    br_token use_type;

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
             * The native context handle.
             */
            void *native_context;

            /*
             * GLAD OpenGL context.
             */
            GladGLContext glad_gl_context;

            const char *gl_version;
            const char *gl_vendor;
            const char *gl_renderer;
            GLint       gl_num_extensions;
            char      **gl_extensions;
        } asFront;
        struct {
            struct br_device_pixelmap *depthbuffer;
            br_device_clut            *clut;
        } asBack;
        struct {
            struct br_device_pixelmap *backbuffer;
        } asDepth;
        struct {
            struct br_device_pixelmap *parent;
        } asSub;
    };
} br_device_pixelmap;

#ifdef __cplusplus
};
#endif
#endif
