/*
 * Stub methods for pixelmap rendering that always fail
 */
#include "pm.h"

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, allocateSub)(br_device_pixelmap *self, br_device_pixelmap **newpm,
                                                               br_rectangle *rect)
{
    return BRE_FAIL;
}


br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copy)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyTo)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyFrom)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, fill)(br_device_pixelmap *self, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, doubleBuffer)(br_device_pixelmap *self, br_device_pixelmap *src)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyDirty)(br_device_pixelmap *self, br_device_pixelmap *src,
                                                             br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyToDirty)(br_device_pixelmap *self, br_device_pixelmap *src,
                                                               br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, copyFromDirty)(br_device_pixelmap *self, br_device_pixelmap *src,
                                                                 br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, fillDirty)(br_device_pixelmap *self, br_uint_32 colour,
                                                             br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, doubleBufferDirty)(br_device_pixelmap *self, br_device_pixelmap *src,
                                                                     br_rectangle *dirty, br_int_32 num_rects)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangle)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangle2)(br_device_pixelmap *self, br_rectangle *rect,
                                                              br_uint_32 colour_tl, br_uint_32 colour_br)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleCopyTo)(br_device_pixelmap *self, br_point *p,
                                                                   br_device_pixelmap *src, br_rectangle *r)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleCopyFrom)(br_device_pixelmap *self, br_point *p,
                                                                     br_device_pixelmap *dest, br_rectangle *r)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail,
                         rectangleStretchCopyTo)(br_device_pixelmap *self, br_rectangle *destinationRectangle,
                                                 br_device_pixelmap *src, br_rectangle *sourceRectangle)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleStretchCopyFrom)(br_device_pixelmap *self, br_rectangle *d,
                                                                            br_device_pixelmap *dest, br_rectangle *s)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleFill)(br_device_pixelmap *self, br_rectangle *rect, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, line)(br_device_pixelmap *self, br_point *s, br_point *e, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD(br_device_pixelmap_fail, copyBits)(br_device_pixelmap *_self, br_point *point, const br_uint_8 *src,
                                                       br_uint_16 s_stride, br_rectangle *bit_rect, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, text)(br_device_pixelmap *self, br_point *point, br_font *font,
                                                        const char *text, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowSize)(br_device_pixelmap *self, br_size_t *sizep)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowQuery)(br_device_pixelmap *self, void *buffer,
                                                            br_size_t buffer_size, br_uint_32 row)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowSet)(br_device_pixelmap *self, void *buffer, br_size_t buffer_size,
                                                          br_uint_32 row)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour, br_point *p)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressQuery)(br_device_pixelmap *self, void **pptr,
                                                                     br_uint_32 *pqual, br_point *p)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressSet)(br_device_pixelmap *self, void *ptr, br_uint_32 *qual)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, flush)(br_device_pixelmap *self)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, synchronise)(br_device_pixelmap *self, br_token sync_type, br_boolean block)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, directLock)(br_device_pixelmap *self, br_boolean block)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, directUnlock)(br_device_pixelmap *self)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, getControls)(br_device_pixelmap *self, br_display_controls *controls)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, setControls)(br_device_pixelmap *self, br_display_controls *controls)
{
    return BRE_FAIL;
}
