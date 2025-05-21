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

static void MemFill0(void *dst, br_uint_32 value, br_size_t num)
{
}

static void MemFill32(void *dst, br_uint_32 value, br_size_t num)
{
    br_uint_32 *d = dst;
    while(num--)
        *(d++) = value;
}

static void MemFill16(void *dst, br_uint_32 value, br_size_t num)
{
    br_uint_16 *d = dst;
    while(num--)
        *(d++) = value;
}

static void MemFill24(void *dst, br_uint_32 val, br_size_t num)
{
    br_uint_8 *d = dst;
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

static void MemFill8(void *dst, br_uint_32 value, br_size_t num)
{
    memset(dst, (int)(value & 0xFF), num);
}

typedef void(br_filler_cbfn)(void *dst, br_uint_32 value, br_size_t num);

static br_filler_cbfn *const fillers[] = {
    // clang-format off
    [0] = MemFill0,
    [1] = MemFill8,
    [2] = MemFill16,
    [3] = MemFill24,
    [4] = MemFill32,
    // clang-format on
};

/**
 * \brief For each bit \c i set in \c c, set the corresponding pixel to a given colour.
 */
static void apply_mask(br_uint_8 *dst, br_uint_32 bpp, br_colour colour, br_uint_8 c)
{
    br_filler_cbfn *const filler = fillers[bpp];

    for(int i = 0; i < 8; ++i, dst += bpp) {
        /* If the bit is set, fill. */
        if((c << i) & 0x80)
            filler(dst, colour, 1);
    }
}

static void COPY_BITS_CORE_1(br_uint_8 *dst, br_uint_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
                             br_uint_32 bpp, br_uint_32 colour, br_uint_8 himask, br_uint_8 lomask)
{
    br_uint_8 comb_mask = himask & lomask;

    for(; nrows; --nrows, dst += d_stride, src += s_stride) {
        /* We're 1 pixel wide, loop vertically (i.e. through each row) */
        apply_mask(dst, bpp, colour, *src & comb_mask);
    }
}

static void COPY_BITS_CORE_N(br_uint_8 *dst, br_int_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 nrows,
                             br_uint_32 bpp, br_uint_32 colour, br_uint_8 himask, br_uint_8 lomask)
{
    for(size_t r = 0; r < nrows; ++r) {
        br_uint_8 *local_dst = dst;

        for(br_uint_32 n = 0; n < s_stride; ++n) {
            br_uint_8 c = *src++;

            if(n == 0)
                c &= lomask;
            else if(n == s_stride - 1)
                c &= himask;

            apply_mask(local_dst, bpp, colour, c);

            local_dst += 8 * bpp;
        }

        dst += d_stride;
    }
}

void _MemCopyBits_A(void *dst, br_int_32 d_stride, const br_uint_8 *src, br_uint_32 s_stride, br_uint_32 start_bit, br_uint_32 end_bit,
                    br_uint_32 nrows, br_uint_32 bpp, br_uint_32 colour)
{
    br_uint_8 himask = bit_to_mask_e[end_bit & 7];
    br_uint_8 lomask = bit_to_mask_s[start_bit];

    if((end_bit >> 3) && (bpp >= 1 && bpp <= 4))
        COPY_BITS_CORE_N(dst, d_stride, src, s_stride, nrows, bpp, colour, himask, lomask);
    else if(bpp >= 1 && bpp <= 4)
        COPY_BITS_CORE_1(dst, d_stride, src, s_stride, nrows, bpp, colour, himask, lomask);
}

void _MemFill_A(void *dst, br_uint_32 pixels, br_uint_32 bpp, br_uint_32 colour)
{
    ASSERT_MESSAGE("invalid pixel size", bpp < BR_ASIZE(fillers));
    fillers[bpp](dst, colour, pixels);
}

void _MemRectCopy_A(void *dst, const char *src, br_uint_16 pwidth, br_uint_16 pheight, br_int_32 d_stride, br_int_32 s_stride, br_uint_32 bpp)
{
    br_uint_8 *dest      = dst;
    size_t     row_bytes = pwidth * bpp;

    for(size_t r = 0; r < pheight; ++r) {
        BrMemCpy(dest, src, row_bytes);
        dest += d_stride;
        src += s_stride;
    }
}

/* From IDA */
void _MemCopy_A(void *dst, const char *src, br_uint_32 pixels, br_uint_32 bpp)
{
    BrMemCpy(dst, src, pixels * bpp);
}

void _MemPixelSet(void *dst, br_uint_32 bytes, br_uint_32 colour)
{
    switch(bytes) {
        case 1:
            *((br_uint_8 *)dst) = (br_uint_8)colour;
            break;
        case 2:
            *((br_uint_16 *)dst) = (br_uint_16)colour;
            break;
        case 3:
            *((br_uint_8 *)dst + 0) = (br_uint_8)((colour & 0x0000FF) >> 0);
            *((br_uint_8 *)dst + 1) = (br_uint_8)((colour & 0x00FF00) >> 8);
            *((br_uint_8 *)dst + 2) = (br_uint_8)((colour & 0xFF0000) >> 16);
            break;
        case 4:
            *((br_uint_32 *)dst) = colour;
            break;
        default:
            ASSERT_MESSAGE("invalid pixel size", bytes >= 1 && bytes <= 4);
    }
}

br_uint_32 _MemPixelGet(const void *dst, br_uint_32 bytes)
{
    switch(bytes) {
        case 1:
            return *((br_uint_8 *)dst);
        case 2:
            return *((br_uint_16 *)dst);
        case 3: {
            const br_uint_8 *d = dst;
            return d[0] | (d[1] << 8) | (d[2] << 16);
        }
        case 4:
            return *((br_uint_32 *)dst);
        default:
            ASSERT_MESSAGE("invalid pixel size", bytes >= 1 && bytes <= 4);
            return 0;
    }
}
