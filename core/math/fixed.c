/*
 * Copyright (c) 1992,1993-1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: fixed.c 1.1 1997/12/10 16:41:21 jon Exp $
 * $Locker: $
 *
 * Various fixed point support functions
 */

#include "brender.h"
#include "brmath.h"

/*
 * Lookup tables for our various functions.
 * 257 because 256+1 = 257 and saves a modulo.
 * 321 because cos(x) = sin(x + pi/2), meaning they can share a (larger) table.
 *
 * These are unsigned because there's no safe way
 * initialise a signed number with a hex digit.
 */
const static br_uint_16 sincos_table[321];
const static br_uint_16 arcsin_table[257];
const static br_uint_16 arccos_table[257];
const static br_uint_16 arctan_table[257];

br_fixed_ls BR_PUBLIC_ENTRY BrFixedAbs(br_fixed_ls a)
{
    if(a < 0)
        return -a;

    return a;
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMul(br_fixed_ls a, br_fixed_ls b)
{
    br_int_64 tmp = a * (br_int_64)b;
    return (br_fixed_ls)(tmp >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac2(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e, br_fixed_ls f)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d) + (e * (br_int_64)f)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedMac4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d, br_fixed_ls e,
                                        br_fixed_ls f, br_fixed_ls g, br_fixed_ls h)
{
    return (br_fixed_ls)(((a * (br_int_64)b) + (c * (br_int_64)d) + (e * (br_int_64)f) + (g * (br_int_64)h)) >> 16);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr(br_fixed_ls a)
{
    return BrFixedMul(a, a);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr2(br_fixed_ls a, br_fixed_ls b)
{
    return BrFixedSqr(a) + BrFixedSqr(b);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c)
{
    return BrFixedSqr(a) + BrFixedSqr(b) + BrFixedSqr(c);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqr4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d)
{
    return BrFixedSqr(a) + BrFixedSqr(b) + BrFixedSqr(c) + BrFixedSqr(d);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength2(br_fixed_ls a, br_fixed_ls b)
{
    return BrFixedSqrt(BrFixedSqr2(a, b));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength3(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c)
{
    return BrFixedSqrt(BrFixedSqr3(a, b, c));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedLength4(br_fixed_ls a, br_fixed_ls b, br_fixed_ls c, br_fixed_ls d)
{
    return BrFixedSqrt(BrFixedSqr4(a, b, c, d));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedDiv(br_fixed_ls numerator, br_fixed_ls denominator)
{
    if(denominator == 0)
        return 0;

    return (br_fixed_ls)(((br_uint_64)numerator << 16) / denominator);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedRcp(br_fixed_ls a)
{
    return BrFixedDiv(BR_ONE_LS, a);
}

/*
 * Given a 16-bit input, generate the output value of a function
 * using a 256-word lookup table with interpolation between entries.
 *
 * Table must have all deltas: -32768 <= delta <= 32767
 */
static br_int_16 interp(br_int_16 input, const br_uint_16 *table)
{
    uint8_t index        = (input & 0xFF00) >> 8;
    int16_t base_value   = (br_int_16)table[index];
    int16_t next_value   = (br_int_16)table[index + 1];
    int     delta        = next_value - base_value;
    int     scaled_delta = (delta * (input & 0xFF)) >> 8;

    return (br_int_16)(base_value + scaled_delta);
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSin(br_fixed_luf a)
{
    return interp((int16_t)a, sincos_table + 0) * 2;
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedCos(br_fixed_luf a)
{
    return interp((int16_t)a, sincos_table + 64) * 2;
}

br_fixed_luf BR_PUBLIC_ENTRY BrFixedASin(br_fixed_ls s)
{
    br_int_16 offset_input = (br_int_16)((s + BR_ONE_LS) >> 1);
    return interp(offset_input, arcsin_table);
}

br_fixed_luf BR_PUBLIC_ENTRY BrFixedACos(br_fixed_ls c)
{
    br_int_16 offset_input = (br_int_16)((c + BR_ONE_LS) >> 1);
    return interp(offset_input, arccos_table);
}

/*
 * angle FixedATan2(_F y, _F x)
 *
 * Break in 8 octants, based on sign of X, sign of Y and relative magnitude
 * of X to Y.
 *
 *    0  x>0  y>0  |x| > |y|    =       +atan(|y|/|x|)
 *    1  x>0  y>0  |x| < |y|    = PI/2  -atan(|x|/|y|)
 *    2  x<0  y>0  |x| < |y|    = PI/2  +atan(|x|/|y|)
 *    3  x<0  y>0  |x| > |y|    = PI    -atan(|y|/|x|)
 *    4  x<0  y<0  |x| > |y|    = PI    +atan(|y|/|x|)
 *    5  x<0  y<0  |x| < |y|    = PI*3/2-atan(|x|/|y|)
 *    6  x>0  y<0  |x| < |y|    = PI*3/2+atan(|x|/|y|)
 *    7  x>0  y<0  |x| > |y|    = PI*2  -atan(|y|/|x|)
 */

#define X_GT_0_SHIFT   0
#define Y_GT_0_SHIFT   1
#define XA_GT_YA_SHIFT 2

#define X_GT_0         (1 << X_GT_0_SHIFT)
#define Y_GT_0         (1 << Y_GT_0_SHIFT)
#define XA_GT_YA       (1 << XA_GT_YA_SHIFT)

typedef struct octant_info {
    br_fixed_luf offset;
    int          mul;
    br_uint_32   nidx;
} octant_info;

// clang-format off
static const octant_info OctantInfo[] = {
    /* Q0 */ [X_GT_0 | Y_GT_0 | XA_GT_YA] = {.offset = 0,             .mul =  1, .nidx = 1},
    /* Q1 */ [X_GT_0 | Y_GT_0           ] = {.offset = BR_PI_2_LUF,   .mul = -1, .nidx = 0},
    /* Q2 */ [         Y_GT_0           ] = {.offset = BR_PI_2_LUF,   .mul =  1, .nidx = 0},
    /* Q3 */ [         Y_GT_0 | XA_GT_YA] = {.offset = BR_PI_LUF,     .mul = -1, .nidx = 1},
    /* Q4 */ [                  XA_GT_YA] = {.offset = BR_PI_LUF,     .mul =  1, .nidx = 1},
    /* Q5 */ [                         0] = {.offset = BR_3_PI_2_LUF, .mul = -1, .nidx = 0},
    /* Q6 */ [X_GT_0                    ] = {.offset = BR_3_PI_2_LUF, .mul =  1, .nidx = 0},
    /* Q7 */ [X_GT_0          | XA_GT_YA] = {.offset = 0, /* 2*pi */  .mul = -1, .nidx = 1},
};
// clang-format on

br_fixed_luf BrFixedATan2(br_fixed_ls y, br_fixed_ls x)
{
    const octant_info *octant;
    br_fixed_ls        abs_y  = BrFixedAbs(y);
    br_fixed_ls        abs_x  = BrFixedAbs(x);
    br_fixed_ls        num[2] = {abs_x, abs_y};
    br_fixed_ls        r;

    /* Handle the boundaries between quadrants. */
    if(abs_x == abs_y) {
        if(x > 0 && y < 0)
            return BR_7_PI_4_LUF;

        if(x < 0 && y < 0)
            return BR_5_PI_4_LUF;

        if(x < 0 && y > 0)
            return BR_3_PI_4_LUF;

        if(x > 0 && y > 0)
            return BR_PI_4_LUF;

        /* (0, 0) */
        return BR_ZERO_LUF;
    }

    octant = OctantInfo + (((x > 0) << X_GT_0_SHIFT) | ((y > 0) << Y_GT_0_SHIFT) | ((abs_x > abs_y) << XA_GT_YA_SHIFT));
    r      = BrFixedDiv(num[octant->nidx], num[1 - octant->nidx]);
    return octant->offset + (octant->mul * interp((br_int_16)r, arctan_table));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedSqrt(br_fixed_ls a)
{
    return BrFloatToFixed(BrFloatSqrt(BrFixedToFloat(a)));
}

br_fixed_ls BR_PUBLIC_ENTRY BrFixedPow(br_fixed_ls a, br_fixed_ls b)
{
    return BrFloatToFixed(BrFloatPow(BrFixedToFloat(a), BrFixedToFloat(b)));
}

// clang-format off
const static br_uint_16 sincos_table[] = {
    0x00000, 0x00324, 0x00647, 0x0096A, 0x00C8B, 0x00FAB, 0x012C7, 0x015E1,
    0x018F8, 0x01C0B, 0x01F19, 0x02223, 0x02527, 0x02826, 0x02B1E, 0x02E10,
    0x030FB, 0x033DE, 0x036B9, 0x0398C, 0x03C56, 0x03F16, 0x041CD, 0x0447A,
    0x0471C, 0x049B3, 0x04C3F, 0x04EBF, 0x05133, 0x0539A, 0x055F4, 0x05842,
    0x05A81, 0x05CB3, 0x05ED6, 0x060EB, 0x062F1, 0x064E7, 0x066CE, 0x068A5,
    0x06A6C, 0x06C23, 0x06DC9, 0x06F5E, 0x070E1, 0x07254, 0x073B5, 0x07503,
    0x07640, 0x0776B, 0x07883, 0x07989, 0x07A7C, 0x07B5C, 0x07C29, 0x07CE2,
    0x07D89, 0x07E1C, 0x07E9C, 0x07F08, 0x07F61, 0x07FA6, 0x07FD7, 0x07FF5,
    0x07FFF, 0x07FF5, 0x07FD7, 0x07FA6, 0x07F61, 0x07F08, 0x07E9C, 0x07E1C,
    0x07D89, 0x07CE2, 0x07C29, 0x07B5C, 0x07A7C, 0x07989, 0x07883, 0x0776B,
    0x07640, 0x07503, 0x073B5, 0x07254, 0x070E1, 0x06F5E, 0x06DC9, 0x06C23,
    0x06A6C, 0x068A5, 0x066CE, 0x064E7, 0x062F1, 0x060EB, 0x05ED6, 0x05CB3,
    0x05A81, 0x05842, 0x055F4, 0x0539A, 0x05133, 0x04EBF, 0x04C3F, 0x049B3,
    0x0471C, 0x0447A, 0x041CD, 0x03F16, 0x03C56, 0x0398C, 0x036B9, 0x033DE,
    0x030FB, 0x02E10, 0x02B1E, 0x02826, 0x02527, 0x02223, 0x01F19, 0x01C0B,
    0x018F8, 0x015E1, 0x012C7, 0x00FAB, 0x00C8B, 0x0096A, 0x00647, 0x00324,
    0x00000, 0x0FCDC, 0x0F9B9, 0x0F696, 0x0F375, 0x0F055, 0x0ED39, 0x0EA1F,
    0x0E708, 0x0E3F5, 0x0E0E7, 0x0DDDD, 0x0DAD9, 0x0D7DA, 0x0D4E2, 0x0D1F0,
    0x0CF05, 0x0CC22, 0x0C947, 0x0C674, 0x0C3AA, 0x0C0EA, 0x0BE33, 0x0BB86,
    0x0B8E4, 0x0B64D, 0x0B3C1, 0x0B141, 0x0AECD, 0x0AC66, 0x0AA0C, 0x0A7BE,
    0x0A57F, 0x0A34D, 0x0A12A, 0x09F15, 0x09D0F, 0x09B19, 0x09932, 0x0975B,
    0x09594, 0x093DD, 0x09237, 0x090A2, 0x08F1F, 0x08DAC, 0x08C4B, 0x08AFD,
    0x089C0, 0x08895, 0x0877D, 0x08677, 0x08584, 0x084A4, 0x083D7, 0x0831E,
    0x08277, 0x081E4, 0x08164, 0x080F8, 0x0809F, 0x0805A, 0x08029, 0x0800B,
    0x08001, 0x0800B, 0x08029, 0x0805A, 0x0809F, 0x080F8, 0x08164, 0x081E4,
    0x08277, 0x0831E, 0x083D7, 0x084A4, 0x08584, 0x08677, 0x0877D, 0x08895,
    0x089C0, 0x08AFD, 0x08C4B, 0x08DAC, 0x08F1F, 0x090A2, 0x09237, 0x093DD,
    0x09594, 0x0975B, 0x09932, 0x09B19, 0x09D0F, 0x09F15, 0x0A12A, 0x0A34D,
    0x0A57F, 0x0A7BE, 0x0AA0C, 0x0AC66, 0x0AECD, 0x0B141, 0x0B3C1, 0x0B64D,
    0x0B8E4, 0x0BB86, 0x0BE33, 0x0C0EA, 0x0C3AA, 0x0C674, 0x0C947, 0x0CC22,
    0x0CF05, 0x0D1F0, 0x0D4E2, 0x0D7DA, 0x0DAD9, 0x0DDDD, 0x0E0E7, 0x0E3F5,
    0x0E708, 0x0EA1F, 0x0ED39, 0x0F055, 0x0F375, 0x0F696, 0x0F9B9, 0x0FCDC,
    0x00000, 0x00324, 0x00647, 0x0096A, 0x00C8B, 0x00FAB, 0x012C7, 0x015E1,
    0x018F8, 0x01C0B, 0x01F19, 0x02223, 0x02527, 0x02826, 0x02B1E, 0x02E10,
    0x030FB, 0x033DE, 0x036B9, 0x0398C, 0x03C56, 0x03F16, 0x041CD, 0x0447A,
    0x0471C, 0x049B3, 0x04C3F, 0x04EBF, 0x05133, 0x0539A, 0x055F4, 0x05842,
    0x05A81, 0x05CB3, 0x05ED6, 0x060EB, 0x062F1, 0x064E7, 0x066CE, 0x068A5,
    0x06A6C, 0x06C23, 0x06DC9, 0x06F5E, 0x070E1, 0x07254, 0x073B5, 0x07503,
    0x07640, 0x0776B, 0x07883, 0x07989, 0x07A7C, 0x07B5C, 0x07C29, 0x07CE2,
    0x07D89, 0x07E1C, 0x07E9C, 0x07F08, 0x07F61, 0x07FA6, 0x07FD7, 0x07FF5,
    0x07FFF,
};

const static br_uint_16 arcsin_table[] = {
    0x0C001, 0x0C519, 0x0C737, 0x0C8D7, 0x0CA37, 0x0CB6D, 0x0CC87, 0x0CD8A,
    0x0CE7C, 0x0CF5F, 0x0D037, 0x0D104, 0x0D1C9, 0x0D286, 0x0D33C, 0x0D3ED,
    0x0D498, 0x0D53E, 0x0D5DF, 0x0D67C, 0x0D716, 0x0D7AC, 0x0D83F, 0x0D8CF,
    0x0D95C, 0x0D9E7, 0x0DA6F, 0x0DAF4, 0x0DB78, 0x0DBF9, 0x0DC79, 0x0DCF7,
    0x0DD73, 0x0DDED, 0x0DE66, 0x0DEDD, 0x0DF53, 0x0DFC8, 0x0E03B, 0x0E0AD,
    0x0E11E, 0x0E18D, 0x0E1FC, 0x0E26A, 0x0E2D6, 0x0E342, 0x0E3AC, 0x0E416,
    0x0E47F, 0x0E4E7, 0x0E54E, 0x0E5B4, 0x0E61A, 0x0E67F, 0x0E6E3, 0x0E746,
    0x0E7A9, 0x0E80C, 0x0E86D, 0x0E8CE, 0x0E92F, 0x0E98F, 0x0E9EE, 0x0EA4D,
    0x0EAAB, 0x0EB09, 0x0EB66, 0x0EBC3, 0x0EC20, 0x0EC7C, 0x0ECD7, 0x0ED33,
    0x0ED8D, 0x0EDE8, 0x0EE42, 0x0EE9C, 0x0EEF5, 0x0EF4E, 0x0EFA7, 0x0EFFF,
    0x0F057, 0x0F0AF, 0x0F106, 0x0F15D, 0x0F1B4, 0x0F20B, 0x0F261, 0x0F2B8,
    0x0F30D, 0x0F363, 0x0F3B9, 0x0F40E, 0x0F463, 0x0F4B8, 0x0F50C, 0x0F561,
    0x0F5B5, 0x0F609, 0x0F65D, 0x0F6B1, 0x0F704, 0x0F758, 0x0F7AB, 0x0F7FE,
    0x0F851, 0x0F8A4, 0x0F8F7, 0x0F949, 0x0F99C, 0x0F9EE, 0x0FA41, 0x0FA93,
    0x0FAE5, 0x0FB37, 0x0FB89, 0x0FBDB, 0x0FC2D, 0x0FC7F, 0x0FCD1, 0x0FD23,
    0x0FD74, 0x0FDC6, 0x0FE17, 0x0FE69, 0x0FEBA, 0x0FF0C, 0x0FF5E, 0x0FFAF,
    0x00000, 0x00051, 0x000A2, 0x000F4, 0x00146, 0x00197, 0x001E9, 0x0023A,
    0x0028C, 0x002DD, 0x0032F, 0x00381, 0x003D3, 0x00425, 0x00477, 0x004C9,
    0x0051B, 0x0056D, 0x005BF, 0x00612, 0x00664, 0x006B7, 0x00709, 0x0075C,
    0x007AF, 0x00802, 0x00855, 0x008A8, 0x008FC, 0x0094F, 0x009A3, 0x009F7,
    0x00A4B, 0x00A9F, 0x00AF4, 0x00B48, 0x00B9D, 0x00BF2, 0x00C47, 0x00C9D,
    0x00CF3, 0x00D48, 0x00D9F, 0x00DF5, 0x00E4C, 0x00EA3, 0x00EFA, 0x00F51,
    0x00FA9, 0x01001, 0x01059, 0x010B2, 0x0110B, 0x01164, 0x011BE, 0x01218,
    0x01273, 0x012CD, 0x01329, 0x01384, 0x013E0, 0x0143D, 0x0149A, 0x014F7,
    0x01555, 0x015B3, 0x01612, 0x01671, 0x016D1, 0x01732, 0x01793, 0x017F4,
    0x01857, 0x018BA, 0x0191D, 0x01981, 0x019E6, 0x01A4C, 0x01AB2, 0x01B19,
    0x01B81, 0x01BEA, 0x01C54, 0x01CBE, 0x01D2A, 0x01D96, 0x01E04, 0x01E73,
    0x01EE2, 0x01F53, 0x01FC5, 0x02038, 0x020AD, 0x02123, 0x0219A, 0x02213,
    0x0228D, 0x02309, 0x02387, 0x02407, 0x02488, 0x0250C, 0x02591, 0x02619,
    0x026A4, 0x02731, 0x027C1, 0x02854, 0x028EA, 0x02984, 0x02A21, 0x02AC2,
    0x02B68, 0x02C13, 0x02CC4, 0x02D7A, 0x02E37, 0x02EFC, 0x02FC9, 0x030A1,
    0x03184, 0x03276, 0x03379, 0x03493, 0x035C9, 0x03729, 0x038C9, 0x03AE7,
    0x04000,
};

const static br_uint_16 arccos_table[] = {
    0x07FFF, 0x07AE7, 0x078C9, 0x07729, 0x075C9, 0x07493, 0x07379, 0x07276,
    0x07184, 0x070A1, 0x06FC9, 0x06EFC, 0x06E37, 0x06D7A, 0x06CC4, 0x06C13,
    0x06B68, 0x06AC2, 0x06A21, 0x06984, 0x068EA, 0x06854, 0x067C1, 0x06731,
    0x066A4, 0x06619, 0x06591, 0x0650C, 0x06488, 0x06407, 0x06387, 0x06309,
    0x0628D, 0x06213, 0x0619A, 0x06123, 0x060AD, 0x06038, 0x05FC5, 0x05F53,
    0x05EE2, 0x05E73, 0x05E04, 0x05D96, 0x05D2A, 0x05CBE, 0x05C54, 0x05BEA,
    0x05B81, 0x05B19, 0x05AB2, 0x05A4C, 0x059E6, 0x05981, 0x0591D, 0x058BA,
    0x05857, 0x057F4, 0x05793, 0x05732, 0x056D1, 0x05671, 0x05612, 0x055B3,
    0x05555, 0x054F7, 0x0549A, 0x0543D, 0x053E0, 0x05384, 0x05329, 0x052CD,
    0x05273, 0x05218, 0x051BE, 0x05164, 0x0510B, 0x050B2, 0x05059, 0x05001,
    0x04FA9, 0x04F51, 0x04EFA, 0x04EA3, 0x04E4C, 0x04DF5, 0x04D9F, 0x04D48,
    0x04CF3, 0x04C9D, 0x04C47, 0x04BF2, 0x04B9D, 0x04B48, 0x04AF4, 0x04A9F,
    0x04A4B, 0x049F7, 0x049A3, 0x0494F, 0x048FC, 0x048A8, 0x04855, 0x04802,
    0x047AF, 0x0475C, 0x04709, 0x046B7, 0x04664, 0x04612, 0x045BF, 0x0456D,
    0x0451B, 0x044C9, 0x04477, 0x04425, 0x043D3, 0x04381, 0x0432F, 0x042DD,
    0x0428C, 0x0423A, 0x041E9, 0x04197, 0x04146, 0x040F4, 0x040A2, 0x04051,
    0x03FFF, 0x03FAE, 0x03F5D, 0x03F0B, 0x03EB9, 0x03E68, 0x03E16, 0x03DC5,
    0x03D73, 0x03D22, 0x03CD0, 0x03C7E, 0x03C2C, 0x03BDA, 0x03B88, 0x03B36,
    0x03AE4, 0x03A92, 0x03A40, 0x039ED, 0x0399B, 0x03948, 0x038F6, 0x038A3,
    0x03850, 0x037FD, 0x037AA, 0x03757, 0x03703, 0x036B0, 0x0365C, 0x03608,
    0x035B4, 0x03560, 0x0350B, 0x034B7, 0x03462, 0x0340D, 0x033B8, 0x03362,
    0x0330C, 0x032B7, 0x03260, 0x0320A, 0x031B3, 0x0315C, 0x03105, 0x030AE,
    0x03056, 0x02FFE, 0x02FA6, 0x02F4D, 0x02EF4, 0x02E9B, 0x02E41, 0x02DE7,
    0x02D8C, 0x02D32, 0x02CD6, 0x02C7B, 0x02C1F, 0x02BC2, 0x02B65, 0x02B08,
    0x02AAA, 0x02A4C, 0x029ED, 0x0298E, 0x0292E, 0x028CD, 0x0286C, 0x0280B,
    0x027A8, 0x02745, 0x026E2, 0x0267E, 0x02619, 0x025B3, 0x0254D, 0x024E6,
    0x0247E, 0x02415, 0x023AB, 0x02341, 0x022D5, 0x02269, 0x021FB, 0x0218C,
    0x0211D, 0x020AC, 0x0203A, 0x01FC7, 0x01F52, 0x01EDC, 0x01E65, 0x01DEC,
    0x01D72, 0x01CF6, 0x01C78, 0x01BF8, 0x01B77, 0x01AF3, 0x01A6E, 0x019E6,
    0x0195B, 0x018CE, 0x0183E, 0x017AB, 0x01715, 0x0167B, 0x015DE, 0x0153D,
    0x01497, 0x013EC, 0x0133B, 0x01285, 0x011C8, 0x01103, 0x01036, 0x00F5E,
    0x00E7B, 0x00D89, 0x00C86, 0x00B6C, 0x00A36, 0x008D6, 0x00736, 0x00518,
    0x00000,
};

const static br_uint_16 arctan_table[] = {
    0x00000, 0x00028, 0x00051, 0x0007A, 0x000A2, 0x000CB, 0x000F4, 0x0011D,
    0x00145, 0x0016E, 0x00197, 0x001BF, 0x001E8, 0x00211, 0x00239, 0x00262,
    0x0028B, 0x002B3, 0x002DC, 0x00304, 0x0032D, 0x00355, 0x0037E, 0x003A6,
    0x003CE, 0x003F7, 0x0041F, 0x00448, 0x00470, 0x00498, 0x004C0, 0x004E8,
    0x00511, 0x00539, 0x00561, 0x00589, 0x005B1, 0x005D9, 0x00601, 0x00628,
    0x00650, 0x00678, 0x006A0, 0x006C7, 0x006EF, 0x00716, 0x0073E, 0x00765,
    0x0078D, 0x007B4, 0x007DB, 0x00803, 0x0082A, 0x00851, 0x00878, 0x0089F,
    0x008C6, 0x008ED, 0x00913, 0x0093A, 0x00961, 0x00987, 0x009AE, 0x009D4,
    0x009FB, 0x00A21, 0x00A47, 0x00A6D, 0x00A94, 0x00ABA, 0x00AE0, 0x00B05,
    0x00B2B, 0x00B51, 0x00B77, 0x00B9C, 0x00BC2, 0x00BE7, 0x00C0C, 0x00C32,
    0x00C57, 0x00C7C, 0x00CA1, 0x00CC6, 0x00CEB, 0x00D0F, 0x00D34, 0x00D58,
    0x00D7D, 0x00DA1, 0x00DC6, 0x00DEA, 0x00E0E, 0x00E32, 0x00E56, 0x00E7A,
    0x00E9E, 0x00EC1, 0x00EE5, 0x00F08, 0x00F2C, 0x00F4F, 0x00F72, 0x00F95,
    0x00FB8, 0x00FDB, 0x00FFE, 0x01021, 0x01044, 0x01066, 0x01089, 0x010AB,
    0x010CD, 0x010EF, 0x01111, 0x01133, 0x01155, 0x01177, 0x01199, 0x011BA,
    0x011DC, 0x011FD, 0x0121E, 0x0123F, 0x01260, 0x01281, 0x012A2, 0x012C3,
    0x012E4, 0x01304, 0x01325, 0x01345, 0x01365, 0x01385, 0x013A5, 0x013C5,
    0x013E5, 0x01405, 0x01424, 0x01444, 0x01463, 0x01483, 0x014A2, 0x014C1,
    0x014E0, 0x014FF, 0x0151E, 0x0153C, 0x0155B, 0x01579, 0x01598, 0x015B6,
    0x015D4, 0x015F2, 0x01610, 0x0162E, 0x0164C, 0x0166A, 0x01687, 0x016A5,
    0x016C2, 0x016DF, 0x016FC, 0x01719, 0x01736, 0x01753, 0x01770, 0x0178C,
    0x017A9, 0x017C5, 0x017E2, 0x017FE, 0x0181A, 0x01836, 0x01852, 0x0186E,
    0x0188A, 0x018A5, 0x018C1, 0x018DC, 0x018F7, 0x01913, 0x0192E, 0x01949,
    0x01964, 0x0197F, 0x01999, 0x019B4, 0x019CE, 0x019E9, 0x01A03, 0x01A1D,
    0x01A37, 0x01A51, 0x01A6B, 0x01A85, 0x01A9F, 0x01AB9, 0x01AD2, 0x01AEC,
    0x01B05, 0x01B1E, 0x01B37, 0x01B50, 0x01B69, 0x01B82, 0x01B9B, 0x01BB4,
    0x01BCC, 0x01BE5, 0x01BFD, 0x01C16, 0x01C2E, 0x01C46, 0x01C5E, 0x01C76,
    0x01C8E, 0x01CA5, 0x01CBD, 0x01CD5, 0x01CEC, 0x01D04, 0x01D1B, 0x01D32,
    0x01D49, 0x01D60, 0x01D77, 0x01D8E, 0x01DA5, 0x01DBB, 0x01DD2, 0x01DE9,
    0x01DFF, 0x01E15, 0x01E2C, 0x01E42, 0x01E58, 0x01E6E, 0x01E84, 0x01E99,
    0x01EAF, 0x01EC5, 0x01EDA, 0x01EF0, 0x01F05, 0x01F1B, 0x01F30, 0x01F45,
    0x01F5A, 0x01F6F, 0x01F84, 0x01F99, 0x01FAD, 0x01FC2, 0x01FD7, 0x01FEB,
    0x02000,
};
// clang-format on
