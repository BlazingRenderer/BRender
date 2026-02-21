/*
 * Private device CLUT state
 */
#ifndef DEVCLUT_H_
#define DEVCLUT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_DEVICE_CLUT_PRIVATE
/*
 * Private state of device CLUT
 */
typedef struct br_device_clut {
    /*
     * Dispatch table
     */
    const struct br_device_clut_dispatch *dispatch;

    /*
     * Standard handle identifier
     */
    const char *identifier;

    /*
     * Device pointer
     */
    br_device *device;

    /*
     * Entries exposed as a 1 x BR_VKREND_MAX_CLUT_ENTRIES host pixelmap whose pixels
     * are entries itself, so the rect draw can upload the palette exactly as it
     * uploads a host source.
     */
    br_pixelmap *pm;

    /*
     * Whether any entry has ever been set. A CLUT exists for every offscreen, so its
     * presence alone does not make it a palette; glrend keys off its CLUT texture
     * existing, which happens on the first entry set. The rect draw uses this to tell
     * an unpopulated CLUT from a real palette, so an indexed source whose chain finds
     * only unpopulated CLUTs falls back to white rather than the CLUT's zero entries.
     */
    br_boolean populated;

    /*
     * Storage.
     */
    br_colour entries[BR_VKREND_MAX_CLUT_ENTRIES];
} br_device_clut;

#endif

#ifdef __cplusplus
};
#endif
#endif /* DEVCLUT_H_ */
