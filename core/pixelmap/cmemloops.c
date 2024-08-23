/**
 * Inner loops for operations on in-memory pixelmaps.
 * Ported from memloops.asm
 */
#include <string.h>
#include <brender.h>
#include <brassert.h>
#include "memloops.h"

// clang-format off
br_uint_8 bit_to_mask_s[] = {
    0b11111111,
    0b01111111,
    0b00111111,
    0b00011111,
    0b00001111,
    0b00000111,
    0b00000011,
    0b00000001,
    0b00000000,
};

br_uint_8 bit_to_mask_e[] = {
    0b00000000,
    0b10000000,
    0b11000000,
    0b11100000,
    0b11110000,
    0b11111000,
    0b11111100,
    0b11111110,
    0b11111111,
};
// clang-format on

static void MemFill0(void *dest, br_uint_32 value, br_size_t num)
{
}

static void MemFill32(void *dest, br_uint_32 value, br_size_t num)
{
    br_uint_32 *d = dest;
    while(num--)
        *(d++) = value;
}

static void MemFill16(void *dest, br_uint_32 value, br_size_t num)
{
    br_uint_16 *d = dest;
    while(num--)
        *(d++) = value;
}

static void MemFill24(void *dest, br_uint_32 val, br_size_t num)
{
    br_uint_8 *d = dest;
    br_uint_8  r = BR_RED(val);
    br_uint_8  g = BR_GRN(val);
    br_uint_8  b = BR_BLU(val);

    if((r == g) && (r == b)) {
        memset(d, r, num * 3);
    } else {
        for(br_uint_32 i = 0; i < num; ++i) {
            *d++ = r;
            *d++ = g;
            *d++ = b;
        }
    }
}

static void MemFill8(void *dest, br_uint_32 value, br_size_t num)
{
    memset(dest, (int)(value & 0xFF), num);
}

typedef void(br_filler_cbfn)(void *dest, br_uint_32 value, br_size_t num);

static br_filler_cbfn *const fillers[] = {
    // clang-format off
    [0] = MemFill0,
    [1] = MemFill8,
    [2] = MemFill16,
    [3] = MemFill24,
    [4] = MemFill32,
    // clang-format on
};

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

        fillers[bpp](dest + (i * bpp), colour, 1);
    }
}

static void COPY_BITS_CORE_1(char *dest, br_uint_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
                             br_uint_32 bpp, br_uint_32 colour, br_uint_8 himask, br_uint_8 lomask)
{
    br_uint_8 comb_mask = himask & lomask;

    for(; nrows; --nrows, dest += d_stride, src += s_stride) {
        /* We're 1 pixel wide, loop vertically (i.e. through each row) */
        bc_inner_loop(dest, bpp, colour, (br_uint_8)(*src & comb_mask));
    }
}

static void COPY_BITS_CORE_N(char *dest, br_int_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
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

void _MemCopyBits_A(char *dest, br_int_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 start_bit,
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
    ASSERT_MESSAGE("invalid pixel size", bpp < BR_ASIZE(fillers));
    fillers[bpp](dest, colour, pixels);
}

void _MemRectCopy_A(char *dest, const char *src, br_uint_16 pwidth, br_uint_16 pheight, br_int_32 d_stride,
                    br_int_32 s_stride, br_uint_32 bpp)
{
    size_t row_bytes = pwidth * bpp;

    for(size_t r = 0; r < pheight; ++r) {
        BrMemCpy(dest, src, row_bytes);
        dest += d_stride;
        src += s_stride;
    }
}

/* From IDA */
void _MemCopy_A(char *dest, const char *src, br_uint_32 pixels, br_uint_32 bpp)
{
    BrMemCpy(dest, src, pixels * bpp);
}

void _MemPixelSet(void *dest, br_uint_32 bytes, br_uint_32 colour)
{
    switch(bytes) {
        case 1:
            *((br_uint_8 *)dest) = (br_uint_8)colour;
            break;
        case 2:
            *((br_uint_16 *)dest) = (br_uint_16)colour;
            break;
        case 3:
            *((br_uint_8 *)dest + 0) = (br_uint_8)((colour & 0x0000FF) >> 0);
            *((br_uint_8 *)dest + 1) = (br_uint_8)((colour & 0x00FF00) >> 8);
            *((br_uint_8 *)dest + 2) = (br_uint_8)((colour & 0xFF0000) >> 16);
            break;
        case 4:
            *((br_uint_32 *)dest) = colour;
            break;
        default:
            ASSERT_MESSAGE("invalid pixel size", bytes >= 1 && bytes <= 4);
    }
}

br_uint_32 _MemPixelGet(const void *dest, br_uint_32 bytes)
{
    switch(bytes) {
        case 1:
            return *((br_uint_8 *)dest);
        case 2:
            return *((br_uint_16 *)dest);
        case 3: {
            const br_uint_8 *d = dest;
            return d[0] | (d[1] << 8) | (d[2] << 16);
        }
        case 4:
            return *((br_uint_32 *)dest);
        default:
            ASSERT_MESSAGE("invalid pixel size", bytes >= 1 && bytes <= 4);
            return 0;
    }
}
