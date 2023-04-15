#ifndef __MEMLOOPS_H__
#define __MEMLOOPS_H__

#pragma warning(error : 4020)

/*
 * cmemloops.c
 */

void       _MemCopyBits_A(char *dest, br_int_32 d_stride, br_uint_8 *src, br_uint_32 s_stride, br_uint_32 start_bit,
                          br_uint_32 end_bit, br_uint_32 nrows, br_uint_32 bpp, br_uint_32 colour);
void       _MemFill_A(char *dest, br_uint_32 pixels, br_uint_32 bpp, br_uint_32 colour);
void       _MemRectCopy_A(char *dest, const char *src, br_uint_16 pwidth, br_uint_16 pheight, br_int_32 d_stride,
                          br_int_32 s_stride, br_uint_32 bpp);
void       _MemCopy_A(char *dest, const char *src, br_uint_32 pixels, br_uint_32 bpp);
void       _MemPixelSet(char *dest, br_uint_32 bytes, br_uint_32 colour);
br_uint_32 _MemPixelGet(const char *dest, br_uint_32 bytes);

#endif /* __MEMLOOPS_H__ */