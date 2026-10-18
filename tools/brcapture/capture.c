/*
 * The frame-capture helpers a checksumming demo needs: the FNV-1a pixelmap
 * hash, the pixel reader behind its coverage count, the GL colour-buffer
 * readback and the PPM export of that readback.
 *
 * Nothing here may take an SDL dependency. The GL entry points arrive in a
 * br_capture_gl, so a tool that links this module without SDL3 still gets the
 * readback, and the frame size is the caller's to query.
 */
#include <brender.h>
#include <brddi.h> /* ObjectQuery(), for the colour buffer's GL texture. */

#include "brcapture.h"

#define BR_CAPTURE_GL_BACK           0x0405u
#define BR_CAPTURE_GL_PACK_ALIGNMENT 0x0CF5u
#define BR_CAPTURE_GL_RGBA           0x1908u
#define BR_CAPTURE_GL_TEXTURE_2D     0x0DE1u
#define BR_CAPTURE_GL_UNSIGNED_BYTE  0x1401u

br_uint_32 BrCapturePixelValue(const br_uint_8 *row, br_int_32 x, br_uint_32 bits)
{
    if(bits >= 8) {
        const br_uint_8 *p = row + (br_size_t)x * (bits / 8);
        br_uint_32       v = 0;

        for(br_uint_32 c = 0; c < bits / 8; ++c)
            v |= (br_uint_32)p[c] << (8 * c);

        return v;
    }

    switch(bits) {
        case 4:
            return (row[x / 2] >> (4 * (1 - (x % 2)))) & 0xf;
        case 2:
            return (row[x / 4] >> (2 * (3 - (x % 4)))) & 0x3;
        case 1:
            return (row[x / 8] >> (7 - (x % 8))) & 0x1;
        default:
            return 0;
    }
}

/*
 * BrPixelmapPixelSize() is in *bits*; a row is not width * pixel size bytes.
 * See gltfview/bench.c for the story of getting this wrong.
 */
br_uint_64 BrCaptureHashPixelmap(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered)
{
    br_uint_32 bits      = BrPixelmapPixelSize(pm);
    br_int_32  row_bytes = (br_int_32)(((br_uint_64)pm->width * bits + 7) / 8);
    br_int_32  stride    = pm->row_bytes != 0 ? pm->row_bytes : row_bytes;

    for(br_int_32 y = 0; y < pm->height; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)pm->pixels + (br_size_t)y * stride;

        for(br_int_32 i = 0; i < row_bytes; ++i) {
            hash ^= row[i];
            hash *= 1099511628211ull;
        }

        if(covered == NULL)
            continue;

        for(br_int_32 x = 0; x < pm->width; ++x) {
            if(BrCapturePixelValue(row, x, bits) != 0)
                ++*covered;
        }
    }

    return hash;
}

/*
 * The normalised mode: convert to the RGBA byte-array layout first, so the
 * same picture hashes the same way whatever type and channel order the front
 * screen read it back in. This asks whether two front screens agree, which is
 * a different question from BrCaptureHashPixelmap()'s "did this render path
 * change", whose answer depends on the pixel type - hence two functions and
 * not a mode flag.
 *
 * A pixel is covered when its red, green or blue channel is set. Alpha is
 * deliberately excluded: an opaque frame carries 255 there on every pixel, so
 * counting any non-zero channel would call every frame fully covered.
 */
br_uint_64 BrCaptureHashPixelmapRGBA(br_uint_64 hash, br_pixelmap *pm, br_uint_32 *covered)
{
    br_pixelmap *rgba  = pm;
    br_pixelmap *conv  = NULL;
    br_uint_32   bytes = 0;

    if(pm->type != BR_PMT_RGBA_8888_ARR) {
        if((conv = BrPixelmapCloneTyped(pm, BR_PMT_RGBA_8888_ARR)) == NULL)
            return hash;

        rgba = conv;
    }

    bytes = BrPixelmapPixelSize(rgba) / 8;

    for(br_int_32 y = 0; y < rgba->height; ++y) {
        const br_uint_8 *row = (const br_uint_8 *)rgba->pixels + (br_size_t)y * rgba->row_bytes;

        for(br_int_32 x = 0; x < rgba->width; ++x) {
            const br_uint_8 *p = row + (br_size_t)x * bytes;

            for(br_uint_32 c = 0; c < bytes; ++c) {
                hash ^= p[c];
                hash *= 1099511628211ull;
            }

            if(covered != NULL && (p[0] || p[1] || p[2]))
                ++*covered;
        }
    }

    if(conv != NULL)
        BrPixelmapFree(conv);

    return hash;
}

/*
 * Read the rendered frame back into px as tightly-packed RGBA8, bottom row
 * first, which is the order glReadPixels and glGetTexImage both return.
 *
 * glrend renders into the colour buffer's own FBO and only blits it to the
 * window at swap time, which is after this runs, so on glrend the FBO texture
 * is the only correct source - reading GL_BACK there gives whatever the swap
 * left behind, i.e. undefined. glrend1x is GL 1.x with no FBOs: it renders
 * straight into the default framebuffer and exposes no texture token, so fall
 * back to the window back buffer, which is exactly right for it.
 */
br_boolean BrCaptureReadbackGL(const br_capture_gl *gl, br_pixelmap *colour_buffer, br_int_32 w, br_int_32 h, br_uint_8 *px)
{
    br_uint_32 tex = 0;

    if(gl->PixelStorei != NULL)
        gl->PixelStorei(BR_CAPTURE_GL_PACK_ALIGNMENT, 1);

    if(gl->BindTexture != NULL && gl->GetTexImage != NULL && ObjectQuery(colour_buffer, &tex, BRT_OPENGL_TEXTURE_U32) == BRE_OK && tex != 0) {
        gl->BindTexture(BR_CAPTURE_GL_TEXTURE_2D, tex);
        gl->GetTexImage(BR_CAPTURE_GL_TEXTURE_2D, 0, BR_CAPTURE_GL_RGBA, BR_CAPTURE_GL_UNSIGNED_BYTE, px);
        return BR_TRUE;
    }

    if(gl->ReadPixels == NULL)
        return BR_FALSE;

    if(gl->ReadBuffer != NULL)
        gl->ReadBuffer(BR_CAPTURE_GL_BACK);

    gl->ReadPixels(0, 0, w, h, BR_CAPTURE_GL_RGBA, BR_CAPTURE_GL_UNSIGNED_BYTE, px);

    return BR_TRUE;
}

br_error BrCaptureExportPPM(const char *path, const br_uint_8 *px, br_int_32 w, br_int_32 h)
{
    br_pixelmap *pm;
    br_uint_32   saved;

    if((pm = BrPixelmapAllocate(BR_PMT_RGBA_8888, w, h, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    /*
     * The readback is bottom-up and PPM is top-down. BR_PMT_RGBA_8888 is
     * ARGB-on-the-wire: its bytes are b,g,r,a, while the readback hands
     * back r,g,b,a, so swap red and blue.
     */
    for(int y = 0; y < h; ++y) {
        const br_uint_8 *src = px + (br_size_t)(h - 1 - y) * w * 4;
        br_uint_8       *dst = (br_uint_8 *)pm->pixels + (br_size_t)y * pm->row_bytes;

        for(int x = 0; x < w; ++x) {
            dst[x * 4 + 0] = src[x * 4 + 2];
            dst[x * 4 + 1] = src[x * 4 + 1];
            dst[x * 4 + 2] = src[x * 4 + 0];
            dst[x * 4 + 3] = src[x * 4 + 3];
        }
    }

    saved = BrFmtImageSave(path, pm, BR_FMT_IMAGE_PPM);
    BrPixelmapFree(pm);

    return saved ? BRE_OK : BRE_FAIL;
}
