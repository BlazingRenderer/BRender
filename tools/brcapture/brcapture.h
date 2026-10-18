#ifndef BRCAPTURE_H_
#define BRCAPTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <brender.h>

/*
 * The GL entry points BrCaptureReadbackGL() uses.
 *
 * brcapture.h must not include SDL, because a capture tool can link this module
 * without SDL3, so the caller hands over the addresses it has already looked
 * up however it likes. Any entry point may be NULL; the readback falls back in
 * exactly the order the driver allows.
 */
typedef void (*br_capture_pfn_ReadPixels)(int x, int y, int w, int h, unsigned int format, unsigned int type, void *pixels);
typedef void (*br_capture_pfn_ReadBuffer)(unsigned int mode);
typedef void (*br_capture_pfn_BindTexture)(unsigned int target, unsigned int texture);
typedef void (*br_capture_pfn_GetTexImage)(unsigned int target, int level, unsigned int format, unsigned int type, void *pixels);
typedef void (*br_capture_pfn_PixelStorei)(unsigned int pname, int param);

typedef struct br_capture_gl {
    br_capture_pfn_ReadPixels  ReadPixels;
    br_capture_pfn_ReadBuffer  ReadBuffer;
    br_capture_pfn_BindTexture BindTexture;
    br_capture_pfn_GetTexImage GetTexImage;
    br_capture_pfn_PixelStorei PixelStorei;
} br_capture_gl;

/**
 * \brief The value of pixel x within a packed pixelmap row.
 *
 * Sub-byte types pack the first pixel of a byte into its most significant
 * field. This is what decides whether a pixel counts as covered, as against
 * merely non-zero bytes.
 */
br_uint_32 BrCapturePixelValue(const br_uint_8 *row, br_int_32 x, br_uint_32 bits);

/**
 * \brief Fold a pixelmap's rows into hash, optionally counting the covered
 *        pixels.
 *
 * Every byte of every row goes in, and an indexed pixelmap's palette is folded
 * into the same value by passing it as another pixelmap. That is the raw mode:
 * the result depends on the pixel type as well as the picture. BrPixelmapPixelSize()
 * is in *bits*, so a row is not width * pixel size bytes long.
 *
 * \param hash    The FNV-1a state to continue, typically 1469598103934665603ull.
 * \param pm      The pixelmap to fold in.
 * \param covered If non-NULL, receives the number of pixels whose value is not zero.
 *
 * \returns The updated hash.
 */
br_uint_64 BrCaptureHashPixelmap(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered);

/**
 * \brief Fold a pixelmap's pixels into hash as RGBA8, optionally counting the
 *        covered pixels.
 *
 * The normalised mode: pm is converted to BR_PMT_RGBA_8888_ARR first unless it
 * is already in that layout, so the result depends on the picture and not on
 * the pixel type or channel order, and two front screens showing the same frame
 * hash alike. Use BrCaptureHashPixelmap() when the type is part of the question.
 *
 * \param hash    The FNV-1a state to continue, typically 1469598103934665603ull.
 * \param pm      The pixelmap to fold in.
 * \param covered If non-NULL, receives the number of pixels whose red, green or
 *                blue channel is non-zero. Alpha is ignored, so an opaque frame
 *                is only as covered as its colour is.
 *
 * \returns The updated hash, or hash unchanged if pm could not be converted.
 */
br_uint_64 BrCaptureHashPixelmapRGBA(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered);

/**
 * \brief Read the rendered frame back into px as tightly packed RGBA8.
 *
 * px receives w * h * 4 bytes, bottom row first, which is the order
 * glReadPixels and glGetTexImage both return them in.
 *
 * \param gl            The entry points to read through; not NULL.
 * \param colour_buffer The demo's colour buffer, whose offscreen texture is
 *                      preferred when the driver exposes one.
 * \param w             The frame width, in pixels.
 * \param h             The frame height, in pixels.
 * \param px            A buffer of at least w * h * 4 bytes.
 *
 * \returns BR_TRUE if the frame was read, BR_FALSE if there was no way to read it.
 */
br_boolean BrCaptureReadbackGL(const br_capture_gl *gl, br_pixelmap *colour_buffer, br_int_32 w, br_int_32 h, br_uint_8 *px);

/**
 * \brief Write a readback to path as a binary PPM.
 *
 * \param path The file to write.
 * \param px   A bottom-up, tightly packed RGBA8 block, as BrCaptureReadbackGL()
 *             returns.
 * \param w    The frame width, in pixels.
 * \param h    The frame height, in pixels.
 *
 * \returns BRE_OK if the file was written, BRE_FAIL otherwise.
 */
br_error BrCaptureExportPPM(const char *path, const br_uint_8 *px, br_int_32 w, br_int_32 h);

#ifdef __cplusplus
}
#endif

#endif /* BRCAPTURE_H_ */
