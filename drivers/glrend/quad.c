#include "drv.h"

const static GLsizei s_QuadStride = sizeof(br_device_pixelmap_gl_tri);

/* Quad GL_TRIANGES indices. */
static GLuint s_QuadIndices[] = {0, 1, 2, 2, 3, 0};

/* Default quad data. Format is XYZRGBUV. */
// clang-format off
static br_device_pixelmap_gl_tri s_QuadData[4] = {
    {
        /* Bottom-left */
        -1.0f, -1.0f, 0.5f,
         1.0f,  1.0f, 1.0f,
         0.0f,  0.0f,
    },
    {
        /* Top-left */
        -1.0f, 1.0f, 0.5f,
         1.0f, 1.0f, 1.0f,
         0.0f, 1.0f,
    },
    {
        /* Top-right */
        1.0f, 1.0f,  0.5f,
        1.0f, 1.0f,  1.0f,
        1.0f, 1.0f,
    },
    {
        /* Bottom-right */
        1.0f, -1.0f, 0.5f,
        1.0f,  1.0f, 1.0f,
        1.0f,  0.0f,
    }
};
// clang-format on

void DeviceGLInitQuad(br_device_pixelmap_gl_quad *self, HVIDEO hVideo)
{
    /* Create the crap needed for the operations quad. */
    BrMemCpy(self->tris, s_QuadData, sizeof(self->tris));
    BR_STATIC_ASSERT(sizeof(self->tris) == sizeof(s_QuadData), "Quad size mismatch");

    { /* Build the buffers */
        glGenBuffers(2, self->buffers);

        glBindBuffer(GL_ARRAY_BUFFER, self->buffers[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(self->tris), self->tris, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->buffers[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(s_QuadIndices), s_QuadIndices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    { /* Default Quad VAO */
        glGenVertexArrays(1, &self->defaultVao);
        glBindVertexArray(self->defaultVao);

        glBindBuffer(GL_ARRAY_BUFFER, self->buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->buffers[1]);

        /* Positions */
        glEnableVertexAttribArray(hVideo->defaultProgram.aPosition);
        glVertexAttribPointer(hVideo->defaultProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

        /* Colours */
        glEnableVertexAttribArray(hVideo->defaultProgram.aColour);
        glVertexAttribPointer(hVideo->defaultProgram.aColour, 3, GL_FLOAT, GL_FALSE, s_QuadStride,
                              (GLvoid *)(sizeof(GLfloat) * 3));

        /* UVs */
        glEnableVertexAttribArray(hVideo->defaultProgram.aUV);
        glVertexAttribPointer(hVideo->defaultProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride,
                              (GLvoid *)(sizeof(GLfloat) * 6));
    }

    { /* Text Quad VAO */
        glGenVertexArrays(1, &self->textVao);
        glBindVertexArray(self->textVao);

        glBindBuffer(GL_ARRAY_BUFFER, self->buffers[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self->buffers[1]);

        /* Positions */
        glEnableVertexAttribArray(hVideo->textProgram.aPosition);
        glVertexAttribPointer(hVideo->textProgram.aPosition, 3, GL_FLOAT, GL_FALSE, s_QuadStride, NULL);

        /* UVs */
        glEnableVertexAttribArray(hVideo->textProgram.aUV);
        glVertexAttribPointer(hVideo->textProgram.aUV, 2, GL_FLOAT, GL_FALSE, s_QuadStride, (GLvoid *)(sizeof(GLfloat) * 6));
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    /*
     * Some of the above can fail if the user changes the shader.
     * Drain all the errors to prevent them falling through to future checks.
     */
    while(glGetError() != GL_NO_ERROR)
        ;
}

void DeviceGLFiniQuad(br_device_pixelmap_gl_quad *self)
{
    glDeleteBuffers(2, self->buffers);
    glDeleteVertexArrays(1, &self->defaultVao);
    glDeleteVertexArrays(1, &self->textVao);
}

static void actually_patch_quad(br_device_pixelmap_gl_quad *self, float dx0, float dy0, float dx1, float dy1, float sx0,
                                float sy0, float sx1, float sy1)
{
    /* Bottom-left */
    self->tris[0].x = dx0;
    self->tris[0].y = dy0;
    self->tris[0].u = sx0;
    self->tris[0].v = sy0;

    /* Top-left */
    self->tris[1].x = dx0;
    self->tris[1].y = dy1;
    self->tris[1].u = sx0;
    self->tris[1].v = sy1;

    /* Top-right */
    self->tris[2].x = dx1;
    self->tris[2].y = dy1;
    self->tris[2].u = sx1;
    self->tris[2].v = sy1;

    /* Bottom right */
    self->tris[3].x = dx1;
    self->tris[3].y = dy0;
    self->tris[3].u = sx1;
    self->tris[3].v = sy0;

    /* Upload the data */
    glBindBuffer(GL_ARRAY_BUFFER, self->buffers[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(self->tris), self->tris);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DeviceGLPatchQuad(br_device_pixelmap_gl_quad *self, const br_pixelmap *dst, const br_rectangle *dr,
                       const br_pixelmap *src, const br_rectangle *sr)
{
    float dx0, dy0, dx1, dy1;
    float sx0, sy0, sx1, sy1;

    VIDEOI_BrRectToUVs(dst, dr, &dx0, &dy0, &dx1, &dy1);
    VIDEOI_BrRectToUVs(src, sr, &sx0, &sy0, &sx1, &sy1);

    actually_patch_quad(self, dx0, dy0, dx1, dy1, sx0, sy0, sx1, sy1);
}

void DeviceGLPatchQuadFont(br_device_pixelmap_gl_quad *self, const br_pixelmap *dst, const br_rectangle *dr,
                           const br_font_gl *font, br_uint_8 glyph)
{
    float dx0, dy0, dx1, dy1;
    VIDEOI_BrRectToUVs(dst, dr, &dx0, &dy0, &dx1, &dy1);

    actually_patch_quad(self, dx0, dy0, dx1, dy1, font->glyph[glyph].x0, font->glyph[glyph].y0, font->glyph[glyph].x1,
                        font->glyph[glyph].y1);
}

void DeviceGLDrawQuad(br_device_pixelmap_gl_quad *self)
{
    glBindVertexArray(self->defaultVao);
    glDrawElements(GL_TRIANGLES, (GLsizei)BR_ASIZE(s_QuadIndices), GL_UNSIGNED_INT, NULL);
}

void DeviceGLDrawQuadText(br_device_pixelmap_gl_quad *self)
{
    glBindVertexArray(self->textVao);
    glDrawElements(GL_TRIANGLES, (GLsizei)BR_ASIZE(s_QuadIndices), GL_UNSIGNED_INT, NULL);
}