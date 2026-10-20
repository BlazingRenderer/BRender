/*
 * The vertex layout both GL geometry paths use
 */
#include "drv.h"

/*
 * The engine's colour is a br_colour, 0xAARRGGBB, whose bytes in memory run
 * B,G,R,A; the attribute reads four unsigned bytes as R,G,B,A.
 */
void VertexGLSetColour(br_vertex_gl *v, br_colour colour)
{
    v->c[0] = BR_RED(colour);
    v->c[1] = BR_GRN(colour);
    v->c[2] = BR_BLU(colour);
    v->c[3] = BR_ALPHA(colour);
}

/*
 * Point the current vertex array's attributes at the bound array buffer. Each
 * attribute has its own guard: a shader that wants position without mapping
 * coordinates has to be able to leave aUV at -1.
 */
void VertexGLSetAttribs(const GladGLContext *gl, const br_gl_main_shader *shader)
{
    if(shader->attributes.aPosition >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aPosition);
        gl->VertexAttribPointer(shader->attributes.aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(br_vertex_gl), (void *)offsetof(br_vertex_gl, p));
    }

    if(shader->attributes.aUV >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aUV);
        gl->VertexAttribPointer(shader->attributes.aUV, 2, GL_FLOAT, GL_FALSE, sizeof(br_vertex_gl), (void *)offsetof(br_vertex_gl, map));
    }

    if(shader->attributes.aNormal >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aNormal);
        gl->VertexAttribPointer(shader->attributes.aNormal, 3, GL_FLOAT, GL_FALSE, sizeof(br_vertex_gl), (void *)offsetof(br_vertex_gl, n));
    }

    if(shader->attributes.aColour >= 0) {
        gl->EnableVertexAttribArray(shader->attributes.aColour);
        gl->VertexAttribPointer(shader->attributes.aColour, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(br_vertex_gl), (void *)offsetof(br_vertex_gl, c));
    }
}
