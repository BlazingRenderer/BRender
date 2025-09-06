#include "drv.h"

/*
 * Lightweight vertex/fragment shader for rendering text.
 */
#include "text.vert.glsl.h"
#include "text.frag.glsl.h"

br_boolean ShaderGLTextCompile(br_gl_text_shader *self, const GladGLContext *gl)
{
    GLuint vert, frag;

    if((vert = DeviceGLCreateAndCompileShader(gl, GL_VERTEX_SHADER, g_TextVertexShader, sizeof(g_TextVertexShader))) == 0)
        goto vert_failed;

    if((frag = DeviceGLCreateAndCompileShader(gl, GL_FRAGMENT_SHADER, g_TextFragmentShader, sizeof(g_TextFragmentShader))) == 0)
        goto frag_failed;

    if((self->program = DeviceGLCreateAndCompileProgram(gl, vert, frag)) == 0)
        goto prog_failed;

    DeviceGLObjectLabel(gl, GL_SHADER, vert, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:vertex");
    DeviceGLObjectLabel(gl, GL_SHADER, frag, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:shader:fragment");
    DeviceGLObjectLabel(gl, GL_PROGRAM, self->program, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:program");

    gl->GenVertexArrays(1, &self->vao_glyphs);
    gl->BindVertexArray(self->vao_glyphs);
    gl->BindVertexArray(0);

    DeviceGLObjectLabel(gl, GL_VERTEX_ARRAY, self->vao_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:vao");

    self->uSampler = gl->GetUniformLocation(self->program, "uSampler");

    self->block_index_font_data   = gl->GetUniformBlockIndex(self->program, "FontData");
    self->block_binding_font_data = 2;
    gl->UniformBlockBinding(self->program, self->block_index_font_data, self->block_binding_font_data);

    self->block_index_glyphs   = gl->GetUniformBlockIndex(self->program, "TextData");
    self->block_binding_glyphs = 3;
    gl->UniformBlockBinding(self->program, self->block_index_glyphs, self->block_binding_glyphs);

    gl->GenBuffers(1, &self->ubo_glyphs);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo_glyphs);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, self->block_binding_glyphs, self->ubo_glyphs);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabel(gl, GL_BUFFER, self->ubo_glyphs, BR_GLREND_DEBUG_INTERNAL_PREFIX "text:ubo");

    gl->BindFragDataLocation(self->program, 0, "main_colour");

prog_failed:
    gl->DeleteShader(frag);

frag_failed:
    gl->DeleteShader(vert);

vert_failed:
    DeviceGLCheckErrors(gl);

    return self->program != 0;
}

void ShaderGLTextBegin(br_gl_text_shader *self, const GladGLContext *gl, const br_gl_text_font *font)
{
    gl->Enable(GL_BLEND);
    gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl->Disable(GL_DEPTH_TEST);
    gl->Disable(GL_CULL_FACE);
    gl->DepthMask(GL_FALSE);

    gl->UseProgram(self->program);
    gl->BindBufferBase(GL_UNIFORM_BUFFER, self->block_binding_font_data, font->font_data);

    gl->ActiveTexture(GL_TEXTURE0);
    gl->BindTexture(GL_TEXTURE_2D_ARRAY, font->tex);
    gl->Uniform1i(self->uSampler, 0);

    gl->BindVertexArray(self->vao_glyphs);
    gl->BindBuffer(GL_UNIFORM_BUFFER, self->ubo_glyphs);
}

void ShaderGLTextDrawInstanced(br_gl_text_shader *self, const GladGLContext *gl, const br_gl_text_data *data, GLsizei instance_count)
{
    (void)self;

    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(br_gl_text_data), data, GL_STATIC_DRAW);
    gl->DrawArraysInstanced(GL_TRIANGLES, 0, 6, instance_count);
}

void ShaderGLTextEnd(br_gl_text_shader *self, const GladGLContext *gl)
{
    (void)self;

    gl->BindVertexArray(0);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);
}

br_error ShaderGLTextBuildFont(const GladGLContext *gl, br_gl_text_font *gl_font, br_font *font)
{
    GLuint                    tex;
    const br_pixelmap_gl_fmt *fmt;
    br_pixelmap              *pm;
    br_int_32                 max_width  = font->glyph_x;
    br_int_32                 max_height = font->glyph_y;

    br_gl_text_font_data fd = {};

    if((fmt = DeviceGLGetFormatDetails(BR_PMT_RGBA_8888)) == NULL)
        return BRE_FAIL;

    /*
     * Find the max width, and calculate the UV coords.
     */
    for(size_t i = 0; i < BR_GL_TEXT_FONT_GLYPH_COUNT; ++i) {
        br_int_32 width = (font->flags & BR_FONTF_PROPORTIONAL) ? font->width[i] : font->glyph_x;

        fd.widths[i] = (float)width;

        if(width > max_width)
            max_width = width;
    }

    for(size_t i = 0; i < BR_GL_TEXT_FONT_GLYPH_COUNT; ++i) {
        fd.widths[i] /= (float)max_width;
    }

    /*
     * Upload the font data.
     */
    gl->GenBuffers(1, &gl_font->font_data);
    gl->BindBuffer(GL_UNIFORM_BUFFER, gl_font->font_data);
    gl->BufferData(GL_UNIFORM_BUFFER, sizeof(br_gl_text_font_data), &fd, GL_STATIC_DRAW);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    DeviceGLObjectLabelF(gl, GL_BUFFER, gl_font->font_data, BR_GLREND_DEBUG_INTERNAL_PREFIX "Fnt%s%dx%d:Data",
                         (font->flags & BR_FONTF_PROPORTIONAL) ? "P" : "F", font->glyph_x, font->glyph_y);

    /*
     * Allocate a temporary pixelmap to draw text into.
     */
    if((pm = BrPixelmapAllocate(fmt->pm_type, max_width, max_height, NULL, BR_PMAF_NORMAL)) == NULL)
        return BRE_FAIL;

    gl->GenTextures(1, &tex);
    gl->BindTexture(GL_TEXTURE_2D_ARRAY, tex);
    gl->TexImage3D(GL_TEXTURE_2D_ARRAY, 0, fmt->internal_format, max_width, max_height, BR_GL_TEXT_FONT_GLYPH_COUNT, 0, fmt->format,
                   fmt->type, NULL);
    gl->TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    DeviceGLObjectLabelF(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "Fnt%s%dx%d:Texture",
                         (font->flags & BR_FONTF_PROPORTIONAL) ? "P" : "F", font->glyph_x, font->glyph_y);

    for(GLsizei i = 0; i < BR_GL_TEXT_FONT_GLYPH_COUNT; ++i) {
        char c[2] = {(char)i, '\0'};

        BrPixelmapFill(pm, BR_COLOUR_RGBA(0, 0, 0, 0));
        BrPixelmapText(pm, -pm->origin_x, -pm->origin_y, BR_COLOUR_RGBA(255, 255, 255, 255), font, c);

        gl->TexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, max_width, max_height, 1, fmt->format, fmt->type, pm->pixels);
    }

    gl->BindTexture(GL_TEXTURE_2D_ARRAY, 0);
    BrPixelmapFree(pm);

    gl_font->tex  = tex;
    gl_font->font = font;
    return BRE_OK;
}
