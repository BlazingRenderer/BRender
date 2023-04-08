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

    /** Standard pixelmap members (not including identifier**/

    BR_PIXELMAP_MEMBERS_PREFIXED(pm_)

    /** End of br_pixelmap fields **/

    /*
     * Pointer to owning device - special case - has to be after public fields
     * because of backwards compatibility for the pixelmaps structure
     */
    struct br_device *device;

    struct br_output_facility *output_facility;

    br_boolean owned;

    /*
     * Our backing SDL surface.
     */
    SDL_Surface *surface;

    /*
     * SDL rendererer, used for line and point drawing.
     */
    SDL_Renderer *renderer;

    /*
     * The SDL window, if we were created for one.
     */
    SDL_Window *window;

    struct br_device_clut *clut;
} br_device_pixelmap;

#ifdef __cplusplus
};
#endif
#endif
