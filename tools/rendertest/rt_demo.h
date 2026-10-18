#ifndef RT_DEMO_H_
#define RT_DEMO_H_

#include <brender.h>

/*
 * What a run is handed: the subset of brdemo's br_demo that a harness needs.
 *
 * rendertest drives its own loop, so it owns these rather than borrowing the
 * framework's; the field names match br_demo's on purpose, so the code that
 * renders a scene reads the same either side of the line.
 */
typedef struct rt_demo {
    br_pixelmap *colour_buffer;
    br_pixelmap *depth_buffer;

    br_actor       *world;
    br_actor       *camera;
    br_order_table *order_table;

    br_colour  clear_colour;
    br_boolean hw_accel;

    br_pixelmap *_screen;

    void *user;
} rt_demo;

#endif /* RT_DEMO_H_ */
