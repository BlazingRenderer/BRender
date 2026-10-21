/*
 * Immediate 2D drawing.
 *
 * A batch is a vertex-coloured triangle list in framebuffer pixels, origin
 * top-left and y down, drawn with a material's surface into the destination it
 * names with no depth buffer. It is the shape a UI renderer (ImGui, a HUD, debug
 * text) submits in, and it is device-agnostic: the software, GL and Vulkan
 * renderers all take it.
 *
 * It is built on the renderer's immediate geometry path - the same one
 * br_geometry_v1_model::render() provides for a prepared model - so it does not
 * add a draw path of its own.
 */
#ifndef _BR2D_H_
#define _BR2D_H_

/**
 * \brief One vertex of a 2D triangle.
 *
 * \c position is in framebuffer pixels with the origin at the top-left, \c map
 * is the texture coordinate (top-left origin, like a host upload) and \c colour
 * modulates the texel, alpha included.
 */
typedef struct br_vertex_2d {
    br_vector2 position;
    br_vector2 map;
    br_colour  colour;
} br_vertex_2d;

#endif /* _BR2D_H_ */
