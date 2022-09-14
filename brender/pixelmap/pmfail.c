/*
 * Stub methods for pixelmap rendering that always fail
 */
#include "pm.h"

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, allocateSub)(br_device_pixelmap *self,
                                                               br_device_pixelmap **newpm,
                                                               br_rectangle *rect)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rectangleStretchCopyFrom)(br_device_pixelmap *_self, br_rectangle *d,
                                                                            br_device_pixelmap *dest, br_rectangle *s)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelSet)(br_device_pixelmap *self, br_point *p, br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, line)(br_device_pixelmap *self, br_point *s, br_point *e,
                                                        br_uint_32 colour)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD(br_device_pixelmap_fail, copyBits)(br_device_pixelmap *_self, br_point *point, br_uint_8 *src,
                                                       br_uint_16 s_stride, br_rectangle *bit_rect, br_uint_32 colour)
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

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, rowSet)(br_device_pixelmap *self, void *buffer,
                                                          br_size_t buffer_size, br_uint_32 row)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelQuery)(br_device_pixelmap *self, br_uint_32 *pcolour,
                                                              br_point *p)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressQuery)(br_device_pixelmap *self, void **pptr,
                                                                     br_uint_32 *pqual, br_point *p)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, pixelAddressSet)(br_device_pixelmap *self, void *ptr,
                                                                   br_uint_32 *qual)
{
    return BRE_FAIL;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, flush)(br_device_pixelmap *self)
{
    return BRE_OK;
}

br_error BR_CMETHOD_DECL(br_device_pixelmap_fail, synchronise)(br_device_pixelmap *self,
                                                               br_token sync_type, br_boolean block)
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
