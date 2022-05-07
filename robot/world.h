/****************************************************************************
 *                                                                          *
 * SOURCE FILE NAME: world.h                                                *
 *                                                                          *
 * DESCRIPTIVE NAME: Header information for robot 3D world                  *
 *                                                                          *
 * COMPONENT: Robot                                                         *
 *                                                                          *
 * COPYRIGHT: (C) Copyright Argonaut Software. 1994.  All rights reserved.  *
 *                                                                          *
 ****************************************************************************/

/*
 * Global constants
 */
#define FRAMERATE_COUNT 20

/*
 * The 3D world
 */
typedef struct brwin_world {
/*
 * World hierarchy
 */
    struct br_actor *root;
    struct br_actor *camera;
    struct br_actor *camera_pivot;
    struct br_actor *light;
    struct br_actor *actor;

/*
 * What sort of update should be carried out
 */
    int update;

} brwin_world;

#define WORLD_UPDATE_ALWAYS     0
#define WORLD_UPDATE_FOREGROUND 1
#define WORLD_UPDATE_NEVER      2
#define WORLD_UPDATE_COUNT      3

#define MAX_DIRTY_RECTANGLES   16

/*
 * A view into the 3D world
 */
typedef struct brwin_dirty_rect {
    int min_x;
    int min_y;
    int max_x;
    int max_y;
} brwin_dirty_rect;

typedef struct brwin_view {
    brwin_world *world;                /* World into which this is a view    */

//    uint32_t ulScanLineBytes;          /* Number of bytes in scanline        */
//    uint32_t ulScanLines;              /* Height of the buffer               */
//    uint32_t ulBufferNumber;           /* Buffer number                      */
//    uint8_t  *pbBuffer;                /* Current off screen buffer          */

    uint32_t ulNumColor;               /* Number of palette entries          */
    char     *title;                   /* Title string                       */

    long aspect;                       /* Fixed point aspect ratio for camera*/

    uint32_t ulWidth, ulHeight;        /* Size of image                      */
    int      x, y;                     /* Position in window of image        */

    int blit_type;                     /* How to blit the window             */
    int force_all;                     /* Always update whole region	        */

    long start_time;                   /* State for working out render rate  */
    int  frame_count;                  /*  - the frame count                 */
    int  frame_rate;                   /*  - the frame rate                  */

    struct br_pixelmap *screen;        /* Front buffer                       */
    struct br_pixelmap *colour_buffer; /* Rendering buffers - for color      */
    struct br_pixelmap *depth_buffer;  /*  - for depth                       */
} brwin_view;

/*
 * World/view function prototypes
 */
brwin_world *WorldAllocate(void);

void WorldFree(brwin_world *world);

void WorldUpdate(brwin_world *world);

void ViewBufferSet(brwin_view *view);

void ViewRender(brwin_view *view);

void ViewScreenUpdate(brwin_view *view);
