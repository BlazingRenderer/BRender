/*
 * BrCalculateAspectRect() for BRender
 * https://github.com/BlazingRenderer/BRender
 *
 * SPDX-License-Identifier: MIT-0
 * Copyright(c) 2025 Zane van Iperen
 */
#include <brender.h>

/**
 * \brief Calculates a br_rectangle with the given aspect ratio, centered within a space of [width, height].
 *
 * The rect is either letterboxed or pillarboxed appropriately.
 *
 * \param aspect The target aspect ratio.
 * \param width The width of the destination.
 * \param height The height of the destination.
 */
br_rectangle BrCalculateAspectRect(br_scalar aspect, br_int_32 width, br_int_32 height)
{
    br_scalar dest_aspect = BR_DIV(BR_SCALAR(width), BR_SCALAR(height));
    br_scalar dest_width, dest_height;
    br_int_32 int_w, int_h;

    if(dest_aspect > aspect) {
        /*
         * Destination is wider than source: letterbox
         */
        dest_height = BR_SCALAR(height);
        dest_width  = BR_CEIL(BR_MUL(dest_height, aspect));
    } else {
        /*
         * Destination is taller or equal in width: pillarbox
         */
        dest_width  = BR_SCALAR(width);
        dest_height = BR_CEIL(BR_DIV(dest_width, aspect));
    }

    int_w = (br_int_32)dest_width;
    int_h = (br_int_32)dest_height;

    return (br_rectangle){
        .x = (width - int_w) / 2,
        .y = (height - int_h) / 2,
        .w = int_w,
        .h = int_h,
    };
}
