/*
 * Copyright (c) 1992,1993-1995 by Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: brfont.h 1.1 1997/12/10 16:41:16 jon Exp $
 * $Locker: $
 *
 */
#ifndef _BRFONT_H_
#define _BRFONT_H_

/**
 * \brief The font data structure, describing a BRender font.
 *
 * Up to 223 bit mapped characters are supported. The font does not necessarily need to accord with
 * ASCII character codes, however the non-printing ASCII codes (0-31 & 127) of the 256 codes
 * possible are reserved for such a purpose. Three ASCII fonts (covering codes 32-126) are
 * predefined by BRender:
 *
 * \li BrFontFixed3x5 — 3 pixels wide by 5 high, fixed pitch font
 * \li BrFontProp4x6 — 4 pixels wide by 6 high, pixel proportional font
 * \li BrFontProp7x9 — 7 pixels wide by 9 high, pixel proportional font
 */
typedef struct br_font {
    /**
     * \brief The `flags` member contains various details about the properties of the font such as
     *        whether it is proportional or not.
     *
     * The presence of the flag whose value is defined by the symbol `BR_FONTF_PROPORTIONAL` indicates
     * that the spacing between characters is proportional to their widths (`width` is used), otherwise
     * each character is regularly spaced (`glyph_x` is used).
     */
    br_uint_32        flags;
    /**
     * \brief The width of characters in fixed pitch fonts.
     *
     * Note that there is an implicit single pixel gap between characters.
     */
    br_uint_16        glyph_x;
    /**
     * \brief The height of the font in pixels (the number of pixel rows making up the largest
     *        character).
     */
    br_uint_16        glyph_y;
    /**
     * \brief The width in pixels between horizontally adjacent character co-ordinates.
     *
     * This is not currently implemented by BRender, but could be used to determine the spacing between
     * columns of text when interpreting ASCII HTAB say.
     */
    br_int_16         spacing_x;
    /**
     * \brief The height in pixels between vertically adjacent character co-ordinates.
     *
     * This is not currently implemented by BRender, but could be used to determine the spacing of
     * between rows of text when interpreting ASCII VTAB or CRLF say.
     */
    br_int_16         spacing_y;
    /**
     * \brief Pointer to an array of 256 widths in pixels of each character (in proportional fonts).
     *
     * Values for ASCII control codes (0-31 & 127) have reserved meanings. Note that there is an
     * implicit single pixel gap between characters.
     */
    const br_int_8   *width;
    /**
     * \brief Pointer to an array of 256 offsets to each character's bit map (values for ASCII control
     *        codes (0-31 & 127) have reserved meanings), with a set bit indicating character foreground
     *        colour, and a cleared bit indicating transparent.
     *
     * Each character is formed of a number of rows of bytes. The number of rows in each character is
     * equal to `glyph_y`. The number of bytes in each row is given by the integer formula below, where
     * Width is the pixel width of the character (`glyph_x` in the case of fixed pitch fonts and
     * `width[char]` in the case of proportional fonts). Number of bytes in each row = \f$(\mathit{Width} + 7) / 8\f$.
     * Therefore, in proportional fonts, it is possible that some characters will have a different
     * number of bytes per row. The character's pixels are arranged in memory such that the character's
     * top left hand corner occupies the most significant bit of the first byte. Naturally a character
     * row may use fewer pixels than all of the bits of the bytes it occupies, in such a case the least
     * significant bits of the last byte of each row will be unused. For example, say the ASCII letter
     * 'E' was stored in a font called `my_font` as a 10x18 character. It would be stored in 18 pairs of
     * bytes (36 bytes) at an address pointed to by `my_font.glyphs+my_font.encoding['E']`. The first
     * byte would contain the pixels for the left hand 8 dots of the top row. The second byte's two most
     * significant bits would contain the remaining two dots of the top row. Subsequent bytes would
     * contain pixels for lower rows in a similar fashion.
     */
    const br_uint_8 **encoding;
} br_font;

/*
 * Flags
 */
#define BR_FONTF_PROPORTIONAL 1

/*
 * Default fonts that are available in framework
 */
#ifdef __cplusplus
extern "C" {
#endif
extern struct br_font *BR_ASM_DATA BrFontFixed3x5;
extern struct br_font *BR_ASM_DATA BrFontProp4x6;
extern struct br_font *BR_ASM_DATA BrFontProp7x9;
#ifdef __cplusplus
};
#endif

#endif
