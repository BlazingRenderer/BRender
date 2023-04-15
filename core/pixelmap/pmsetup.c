/*
 * Copyright (c) 1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: pmsetup.c 1.2 1998/09/02 13:42:37 johng Exp $
 * $Locker: $
 *
 */
#include "pm.h"

static br_image Image_BRPMAP1;

/*
 * Global renderer state
 */
struct br_pixelmap_state BR_ASM_DATA _pixelmap;

/*
 * Resource classes
 */
static br_resource_class resourceClasses[] = {
    {0, "PIXELMAP", BR_MEMORY_PIXELMAP, NULL, 0 },
    {0, "PIXELS",   BR_MEMORY_PIXELS,   NULL, 32}, /* Pixels are cache-line  aligned  */
};

void BR_PUBLIC_ENTRY BrPixelmapBegin(void)
{
    int i;

    /*
     * Clear out static structure
     */
    BrMemSet(&_pixelmap, 0, sizeof(_pixelmap));

    /*
     * Allocate the resource anchor
     */
    _pixelmap.res = BrResAllocate(NULL, 0, BR_MEMORY_ANCHOR);

    /*
     * Register all our resource classes
     */
    for(i = 0; i < BR_ASIZE(resourceClasses); i++)
        BrResClassAdd(resourceClasses + i);

    /*
     * Export DLL
     */
    BrImageAdd(&Image_BRPMAP1);
}

void BR_PUBLIC_ENTRY BrPixelmapEnd(void)
{
    BrImageRemove(&Image_BRPMAP1);

    /*
     * Free all resources ...
     */
    BrResFree(_pixelmap.res);

    /*
     * Clear out static structure
     */
    BrMemSet(&_pixelmap, 0, sizeof(_pixelmap));
}

struct br_pixelmap_state *BR_RESIDENT_ENTRY PixelmapState(void)
{
    return &_pixelmap;
}

// clang-format off
#define REGMETHOD(m)   {.name = "_" BR_STR(m),           .proc = (br_image_proc)m }
#define DEVMETHOD(t,m) {.name = "_" BR_CMETHOD_STR(t,m), .proc = (br_image_proc)BR_CMETHOD_REF(t, m)}
// clang-format on

static const br_image_function_info functions[] = {
    REGMETHOD(PixelmapCopyBitsClip),
    REGMETHOD(PixelmapLineClip),
    REGMETHOD(PixelmapPointClip),
    REGMETHOD(PixelmapRectangleClip),
    REGMETHOD(PixelmapRectangleClipTwo),
    DEVMETHOD(br_device_pixelmap_gen, copy),
    DEVMETHOD(br_device_pixelmap_gen, copyBits),
    DEVMETHOD(br_device_pixelmap_gen, copyDirty),
    DEVMETHOD(br_device_pixelmap_gen, copyFrom),
    DEVMETHOD(br_device_pixelmap_gen, copyFromDirty),
    DEVMETHOD(br_device_pixelmap_gen, copyTo),
    DEVMETHOD(br_device_pixelmap_gen, copyToDirty),
    DEVMETHOD(br_device_pixelmap_gen, directLock),
    DEVMETHOD(br_device_pixelmap_gen, directUnlock),
    DEVMETHOD(br_device_pixelmap_gen, doubleBuffer),
    DEVMETHOD(br_device_pixelmap_gen, doubleBufferDirty),
    DEVMETHOD(br_device_pixelmap_gen, fill),
    DEVMETHOD(br_device_pixelmap_gen, fillDirty),
    DEVMETHOD(br_device_pixelmap_gen, flush),
    DEVMETHOD(br_device_pixelmap_gen, line),
    DEVMETHOD(br_device_pixelmap_gen, rectangle),
    DEVMETHOD(br_device_pixelmap_gen, rectangle2),
    DEVMETHOD(br_device_pixelmap_gen, synchronise),
    DEVMETHOD(br_device_pixelmap_gen, text),
    DEVMETHOD(br_device_pixelmap_gen, textBounds),
    DEVMETHOD(br_device_pixelmap_mem, allocateSub),
    DEVMETHOD(br_device_pixelmap_mem, copyBits),
    DEVMETHOD(br_device_pixelmap_mem, copyDirty),
    DEVMETHOD(br_device_pixelmap_mem, copyFrom),
    DEVMETHOD(br_device_pixelmap_mem, copyFromDirty),
    DEVMETHOD(br_device_pixelmap_mem, copyTo),
    DEVMETHOD(br_device_pixelmap_mem, copyToDirty),
    DEVMETHOD(br_device_pixelmap_mem, directLock),
    DEVMETHOD(br_device_pixelmap_mem, directUnlock),
    DEVMETHOD(br_device_pixelmap_mem, doubleBuffer),
    DEVMETHOD(br_device_pixelmap_mem, doubleBufferDirty),
    DEVMETHOD(br_device_pixelmap_mem, fill),
    DEVMETHOD(br_device_pixelmap_mem, fillDirty),
    DEVMETHOD(br_device_pixelmap_mem, flush),
    DEVMETHOD(br_device_pixelmap_mem, line),
    DEVMETHOD(br_device_pixelmap_mem, match),
    DEVMETHOD(br_device_pixelmap_mem, originSet),
    DEVMETHOD(br_device_pixelmap_mem, pixelAddressQuery),
    DEVMETHOD(br_device_pixelmap_mem, pixelAddressSet),
    DEVMETHOD(br_device_pixelmap_mem, pixelQuery),
    DEVMETHOD(br_device_pixelmap_mem, pixelSet),
    DEVMETHOD(br_device_pixelmap_mem, rectangleCopyFrom),
    DEVMETHOD(br_device_pixelmap_mem, rectangleCopyTo),
    DEVMETHOD(br_device_pixelmap_mem, rectangleFill),
    DEVMETHOD(br_device_pixelmap_mem, rectangleStretchCopyFrom),
    DEVMETHOD(br_device_pixelmap_mem, rectangleStretchCopyTo),
    DEVMETHOD(br_device_pixelmap_mem, resize),
    DEVMETHOD(br_device_pixelmap_mem, rowQuery),
    DEVMETHOD(br_device_pixelmap_mem, rowSet),
    DEVMETHOD(br_device_pixelmap_mem, rowSize),
    DEVMETHOD(br_device_pixelmap_mem, synchronise),
    DEVMETHOD(br_device_pixelmap_mem, validSource),
    DEVMETHOD(br_device_pixelmap_null, copyBits),
    DEVMETHOD(br_device_pixelmap_null, copyDirty),
    DEVMETHOD(br_device_pixelmap_null, copyFrom),
    DEVMETHOD(br_device_pixelmap_null, copyFromDirty),
    DEVMETHOD(br_device_pixelmap_null, copyTo),
    DEVMETHOD(br_device_pixelmap_null, copyToDirty),
    DEVMETHOD(br_device_pixelmap_null, doubleBuffer),
    DEVMETHOD(br_device_pixelmap_null, doubleBufferDirty),
    DEVMETHOD(br_device_pixelmap_null, fill),
    DEVMETHOD(br_device_pixelmap_null, fillDirty),
    DEVMETHOD(br_device_pixelmap_null, pixelAddressQuery),
    DEVMETHOD(br_device_pixelmap_null, pixelAddressSet),
    DEVMETHOD(br_device_pixelmap_null, pixelQuery),
    DEVMETHOD(br_device_pixelmap_null, pixelSet),
    DEVMETHOD(br_device_pixelmap_null, rectangleCopyFrom),
    DEVMETHOD(br_device_pixelmap_null, rectangleCopyTo),
    DEVMETHOD(br_device_pixelmap_null, rectangleFill),
    DEVMETHOD(br_device_pixelmap_null, rectangleStretchCopyFrom),
    DEVMETHOD(br_device_pixelmap_null, rectangleStretchCopyTo),
    DEVMETHOD(br_device_pixelmap_null, resize),
};

static br_image Image_BRPMAP1 = {
    .node         = {0},
    .identifier   = "BRPMAP1",
    .type         = BR_IMG_RESIDENT,
    .ref_count    = 0,
    .n_functions  = BR_ASIZE(functions),
    .functions    = functions,
    .type_pointer = NULL,
};
