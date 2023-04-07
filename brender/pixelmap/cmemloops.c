/**
 * Inner loops for operations on in-memory pixelmaps.
 * Ported from memloops.asm
 */
#include <string.h>
#include <brender.h>
#include <brassert.h>
#include "memloops.h"

br_uint_8 bit_to_mask_s[] = {
    0xFF, /* 0b11111111 */
    0x7F, /* 0b01111111 */
    0x3F, /* 0b00111111 */
    0x1F, /* 0b00011111 */
    0x0F, /* 0b00001111 */
    0x07, /* 0b00000111 */
    0x03, /* 0b00000011 */
    0x01, /* 0b00000001 */
    0x00  /* 0b00000000 */
};

br_uint_8 bit_to_mask_e[] = {
    0x00, /* 0b00000000 */
    0x80, /* 0b10000000 */
    0xC0, /* 0b11000000 */
    0xE0, /* 0b11100000 */
    0xF0, /* 0b11110000 */
    0xF8, /* 0b11111000 */
    0xFC, /* 0b11111100 */
    0xFE, /* 0b11111110 */
    0xFF  /* 0b11111111 */
};

static br_uint_32 *MemFill32(br_uint_32 *dest, br_uint_32 value, br_uint_32 num)
{
    while(num--)
        *(dest++) = value;

    return dest;
}

static br_uint_16 *MemFill16(br_uint_16 *dest, br_uint_16 value, br_uint_32 num)
{
    while(num--)
        *(dest++) = value;

    return dest;
}

static br_uint_8 *MemFill8(br_uint_8 *dest, br_uint_8 value, br_uint_32 num)
{
    memset(dest, value, num);

    return dest + num;
}

static void bc_inner_loop(char *dest, br_uint_32 bpp, br_uint_32 colour, br_uint_8 c)
{
    br_int_16  carry_check;
    br_uint_32 i;
    br_uint_8  oldC;

    for(i = 0; i < 8; ++i) {
        /* Carry check is working, don't touch it. */
        oldC = c;
        c += c;
        if((carry_check = oldC + oldC) <= 255)
            continue;

        switch(bpp) {
            case 1:
                dest[(i * 1) + 0] = (br_uint_8)(colour & 0xFF); /* mov [edi+(x*1)],bl */
                break;
            case 2:
                dest[(i * 2) + 0] = (br_uint_8)(colour & 0x00FF);        /* mov [edi+(x*2)+0],bl */
                dest[(i * 2) + 1] = (br_uint_8)((colour & 0xFF00) >> 8); /* mov [edi+(x*2)+1],bh */
                break;
            case 3:
                dest[(i * 3) + 0] = (br_uint_8)(colour & 0x0000FF);               /* mov [edi+(x*3)+0],bl */
                dest[(i * 3) + 1] = (br_uint_8)((colour & 0x00FF00) >> 8);        /* mov [edi+(x*3)+1],bh */
                dest[(i * 3) + 2] = (br_uint_8)(((colour >> 8) & 0x00FF00) >> 8); /* mov [edi+(x*3)+2],ah */
                break;
            case 4:
                *((br_uint_32 *)(&dest[(i * 4) + 0])) = colour; /* mov [edi+(x*4)],ebx */
                break;
        }
    }
}

static void COPY_BITS_CORE_1(char *dest, br_uint_32 d_stride, br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
                             br_uint_32 bpp, br_uint_32 colour, br_uint_8 himask, br_uint_8 lomask)
{
    br_uint_8 comb_mask = himask & lomask;

    for(; nrows; --nrows, dest += d_stride, src += s_stride) {
        /* We're 1 pixel wide, loop vertically (i.e. through each row) */
        bc_inner_loop(dest, bpp, colour, (br_uint_8)(*src & comb_mask));
    }
}

static void COPY_BITS_CORE_N(char *dest, br_int_32 d_stride, br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
                             br_uint_32 bpp, br_uint_32 colour, br_uint_8 himask, br_uint_8 lomask)
{
    char      c;
    br_int_32 bppt = (br_int_32)(bpp << 3);
    do {
        c = *(src)&lomask;

h_loop:
        ++src;
h_loop_last:
        // Set destination pixels according to byte mask
        bc_inner_loop(dest, bpp, colour, c);

        c = *src;
        dest += 8 * bpp;
        --bppt;

        if(bppt > 0)
            goto h_loop;
        if(bppt < 0)
            goto done_row;

        // If it is the last byte, mask and look one final time
        c &= lomask;
        goto h_loop_last;

done_row:
        dest += d_stride;
        src += s_stride;
        --nrows;
    } while(nrows);
}

void _MemCopyBits_A(char *dest, br_int_32 d_stride, br_uint_8 *src, br_uint_32 s_stride, br_uint_32 start_bit,
                    br_uint_32 end_bit, br_uint_32 nrows, br_uint_32 bpp, br_uint_32 colour)
{
    br_uint_8 himask = bit_to_mask_e[end_bit & 7];
    br_uint_8 lomask = bit_to_mask_s[start_bit];

    if((end_bit >> 3) && (bpp >= 1 && bpp <= 4))
        COPY_BITS_CORE_N(dest, d_stride, src, s_stride, nrows, bpp, colour, himask, lomask);
    else if(bpp >= 1 && bpp <= 4)
        COPY_BITS_CORE_1(dest, d_stride, src, s_stride, nrows, bpp, colour, himask, lomask);
}

void _MemFill_A(char *dest, br_uint_32 pixels, br_uint_32 bpp, br_uint_32 colour)
{
    /* 32-bit */
    if(bpp == 4) {
        MemFill32((br_uint_32 *)dest, colour, pixels);
    }
    /* 16-bit */
    else if(bpp == 2) {
        MemFill16((br_uint_16 *)dest, (br_uint_16)colour, pixels);

    }
    /* 24-bit */
    else if(bpp == 3) {
        char *cols = (char *)&colour;

        /* If 3 fill bytes are the same, just use 1bpp fill */
        if((cols[0] == cols[1]) && (cols[0] == cols[2])) {
            _MemFill_A(dest, pixels * 3, 1, colour);
        } else
            for(br_uint_32 i = 0; i < pixels; ++i) {
                *dest++ = BR_BLU(colour);
                *dest++ = BR_GRN(colour);
                *dest++ = BR_RED(colour);
            }
    } else if(bpp == 1) {
        MemFill8(dest, (br_uint_8)(colour & 0xFF), pixels);
    }
}

void _MemRectCopy_A(char *dest, const char *src, br_uint_16 pwidth, br_uint_16 pheight, br_int_32 d_stride,
                    br_int_32 s_stride, br_uint_32 bpp)
{
    const br_uint_32 linediff = pwidth * bpp;
    s_stride -= linediff;
    d_stride -= linediff;

    for(; pheight-- > 0; src += s_stride, dest += d_stride) {
        for(br_uint_32 w = 0; w < pwidth; ++w, src += bpp, dest += bpp)
            _MemPixelSet(dest, bpp, _MemPixelGet(src, bpp));
    }
}

/* From IDA */
void _MemCopy_A(char *dest, const char *src, br_uint_32 pixels, br_uint_32 bpp)
{
    int          j;  // [sp+0h] [bp-24h]@4
    int          i;  // [sp+10h] [bp-14h]@1
    unsigned int v6; // [sp+14h] [bp-10h]@1
    void        *v8; // [sp+1Ch] [bp-8h]@1
    const char  *v9; // [sp+20h] [bp-4h]@1

    v9      = src;
    v8      = dest;
    char v7 = bpp * pixels;
    v6      = bpp * pixels;
    for(i = (br_uint_8)dest & 3; i; --i) {
        v7             = *v9;
        *(uint8_t *)v8 = *v9++;
        v8             = (char *)v8 + 1;
        --v6;
    }
    memcpy(v8, v9, 4 * (v6 >> 2));
    for(j = v6 & 3; j; --j)
        *((uint8_t *)v8 + 4 * (v6 >> 2)) = v7;
}

void _MemPixelSet(char *dest, br_uint_32 bytes, br_uint_32 colour)
{
    switch(bytes) {
        case 1:
            *((br_uint_8 *)dest) = (br_uint_8)colour;
            break;
        case 2:
            *((br_uint_16 *)dest) = (br_uint_16)colour;
            break;
        case 3:
            *dest       = (br_uint_8)colour;
            *(dest + 1) = (br_uint_8)((colour & 0x00FF00) >> 8);
            *(dest + 2) = (br_uint_8)((colour & 0xFF0000) >> 16);
            break;
        case 4:
            *((br_uint_32 *)dest) = colour;
            break;
        default:
            ASSERT_MESSAGE("invalid pixel size", 0);
    }
}

br_uint_32 _MemPixelGet(const char *dest, br_uint_32 bytes)
{
    switch(bytes) {
        case 1:
            return (br_uint_8)*dest;
        case 2:
            return *((br_uint_16 *)dest);
        case 3:
            return (*(dest + 2) << 16) | (*(dest + 1) << 8) | (*dest);
        case 4:
            return *((br_uint_32 *)dest);
        default:
            ASSERT_MESSAGE("invalid pixel size", 0);
            return 0;
    }
}
