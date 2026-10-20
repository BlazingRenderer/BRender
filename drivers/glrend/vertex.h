/*
 * The vertex layout both GL geometry paths use.
 */
#ifndef _VERTEX_H_
#define _VERTEX_H_

typedef struct br_vertex_gl {
    br_vector3_f p;
    br_vector2_f map;
    br_vector3_f n;

    /*
     * RGBA, in the byte order the colour attribute reads
     */
    br_uint_8 c[4];
} br_vertex_gl;

#endif /* _VERTEX_H_ */
