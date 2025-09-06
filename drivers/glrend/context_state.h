#ifndef CONTEXT_STATE_H_
#define CONTEXT_STATE_H_

/*
 * Context/Front screen data.
 */
typedef struct br_gl_context_state {
    const GladGLContext *gl;

    const char *gl_version;
    const char *gl_vendor;
    const char *gl_renderer;

    GLint  gl_num_extensions;
    char **gl_extensions;

    struct {
        struct {
            br_uint_32 disable_anisotropic_filtering : 1;
            br_uint_32 orphan_model_buffers : 1;
            br_uint_32 reserved : 30;
        };
        br_uint_32 value;
    } quirks;

    struct {
        GLint   max_uniform_block_size;
        GLint   max_uniform_buffer_bindings;
        GLint   max_vertex_uniform_blocks;
        GLint   max_fragment_uniform_blocks;
        GLint   max_samples;
        GLfloat max_anisotropy;
    } limits;

    GLuint tex_white;
    GLuint tex_checkerboard;

    br_gl_text_font font_fixed3x5;
    br_gl_text_font font_prop4x6;
    br_gl_text_font font_prop7x9;

    br_gl_line_shader line_shader;
    br_gl_rect_shader rect_shader;
    br_gl_text_shader text_shader;
    br_gl_main_shader main_shader;
} br_gl_context_state;

#endif /* CONTEXT_STATE_H_ */
