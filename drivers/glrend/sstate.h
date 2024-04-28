/*
 * Stored renderer state
 */
#ifndef _SSTATE_H_
#define _SSTATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BR_RENDERER_STATE_STORED_PRIVATE

/*
 * Private state of geometry format
 */
typedef struct br_renderer_state_stored {
    /*
     * Dispatch table
     */
    const struct br_renderer_state_stored_dispatch *dispatch;

    /*
     * Standard object identifier
     */
    const char *identifier;

    /*
     * Pointer to owning device
     */
    struct br_device *device;

    /*
     * Saved state
     */

    /*
     * Pointer to renderer that this state is asociated with
     */
    struct br_renderer *renderer;

    state_stack state;
} br_renderer_state_stored;

#endif

#ifdef __cplusplus
};
#endif
#endif
