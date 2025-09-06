#include <string.h>
#include "drv.h"

static void setup_quirks(br_gl_context_state *ctx)
{
    ctx->quirks.value = 0;

    /*
     * Disable anisotropic filtering on llvmpipe. It is _slow_.
     */
    if(BrStrCmp(ctx->gl_vendor, "Mesa") == 0 && strstr(ctx->gl_renderer, "llvmpipe (") == ctx->gl_renderer) {
        BrLogInfo("GLREND", "Quirk - using llvmpipe, disabling anisotropic filtering.");
        ctx->quirks.disable_anisotropic_filtering = 1;
    }

    if(strstr(ctx->gl_renderer, "Apple M") == ctx->gl_renderer) {
        BrLogInfo("GLREND", "Quirk - using Apple Silicon, forcing model uniform buffer orphaning.");
        ctx->quirks.orphan_model_buffers = 1;
    }
}

static char **build_extensions_list(void *vparent, const GladGLContext *gl, GLint *p_num_extensions)
{
    char **extensions     = NULL;
    GLint  num_extensions = 0;

    gl->GetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);

    extensions = BrResAllocate(vparent, sizeof(char *) * (num_extensions + 1), BR_MEMORY_DRIVER);
    for(GLuint i = 0; i < num_extensions; ++i) {
        const GLubyte *ext = gl->GetStringi(GL_EXTENSIONS, i);
        extensions[i]      = BrResStrDup(extensions, (const char *)ext);
    }
    extensions[num_extensions] = NULL;

    *p_num_extensions = num_extensions;
    return extensions;
}

static void GLAD_API_PTR gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message,
                                           const void *user)
{
    const char *source_string, *type_string, *severity_string;

    (void)user;

    // clang-format off
    switch(source) {
        case GL_DEBUG_SOURCE_API:             source_string = "API";             break;
        case GL_DEBUG_SOURCE_OTHER:           source_string = "OTHER";           break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     source_string = "THIRD_PARTY";     break;
        case GL_DEBUG_SOURCE_APPLICATION:     source_string = "APPLICATION";     break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   source_string = "WINDOW_SYSTEM";   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: source_string = "SHADER_COMPILER"; break;
        default:                              source_string = "UNKNOWN";         break;
    }

    switch(type) {
        case GL_DEBUG_TYPE_ERROR:               type_string = "ERROR";               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_string = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_string = "UNDEFINED_BEHAVIOR";  break;
        case GL_DEBUG_TYPE_PORTABILITY:         type_string = "PORTABILITY";         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         type_string = "PERFORMANCE";         break;
        case GL_DEBUG_TYPE_MARKER:              type_string = "MARKER";              break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          type_string = "PUSH_GROUP";          break;
        case GL_DEBUG_TYPE_POP_GROUP:           type_string = "POP_GROUP";           break;
        case GL_DEBUG_TYPE_OTHER:               type_string = "OTHER";               break;
        default:                                type_string = "UNKNOWN";             break;
    }

    switch(severity) {
        case GL_DEBUG_SEVERITY_LOW:          severity_string = "LOW";          break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severity_string = "MEDIUM";       break;
        case GL_DEBUG_SEVERITY_HIGH:         severity_string = "HIGH";         break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severity_string = "NOTIFICATION"; break;
        default:                             severity_string = "UNKNOWN";      break;
    }
    // clang-format on

    if(length < 0) {
        BrLogDebug("GLREND", "glDebug: source=%s, type=%s, id=%u, severity=%s: %.*s", source_string, type_string, id, severity_string,
                   (int)length, message);
    } else {
        BrLogDebug("GLREND", "glDebug: source=%s, type=%s, id=%u, severity=%s: %s", source_string, type_string, id, severity_string, message);
    }
}

static GLuint build_white_texture(const GladGLContext *gl)
{
    const static uint8_t white_rgba[] = {255, 255, 255, 255};

    GLuint tex;

    gl->GenTextures(1, &tex);

    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white_rgba);

    DeviceGLObjectLabel(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "white");

    return tex;
}

static GLuint build_checkerboard_texture(const GladGLContext *gl)
{
    // clang-format off
    const static br_uint_8 checkerboard_rgba[] = {
        0x00, 0x00, 0x00, 0xFF,   0xFF, 0x00, 0xFF, 0xFF,
        0xFF, 0x00, 0xFF, 0xFF,   0x00, 0x00, 0x00, 0xFF,
    };
    // clang-format on

    GLuint tex;

    gl->GenTextures(1, &tex);

    gl->BindTexture(GL_TEXTURE_2D, tex);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerboard_rgba);

    DeviceGLObjectLabel(gl, GL_TEXTURE, tex, BR_GLREND_DEBUG_INTERNAL_PREFIX "checkerboard");

    return tex;
}

br_error ContextStateGLInit(void *vparent, br_gl_context_state *self, const GladGLContext *gl)
{
    BrMemSet(self, 0, sizeof(br_gl_context_state));

    self->gl = gl;

    self->gl_version  = (const char *)gl->GetString(GL_VERSION);
    self->gl_vendor   = (const char *)gl->GetString(GL_VENDOR);
    self->gl_renderer = (const char *)gl->GetString(GL_RENDERER);

    /*
     * Always register the debug stuff, it needs to be explicitly glEnable(GL_DEBUG_OUTPUT)'d anyway.
     */
    if(gl->KHR_debug) {
        gl->DebugMessageCallback(gl_debug_callback, self);
        gl->DebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

#if BR_GLREND_DEBUG
        gl->Enable(GL_DEBUG_OUTPUT);
        // gl->Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
    }

    /*
     * Get a copy of the extension list.
     * NULL-terminate so we can expose it as a BRT_POINTER_LIST.
     */
    self->gl_extensions = build_extensions_list(vparent, gl, &self->gl_num_extensions);

    /*
     * Cache some limits.
     */
    gl->GetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &self->limits.max_uniform_block_size);
    gl->GetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &self->limits.max_uniform_buffer_bindings);
    gl->GetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &self->limits.max_vertex_uniform_blocks);
    gl->GetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &self->limits.max_fragment_uniform_blocks);
    gl->GetIntegerv(GL_MAX_SAMPLES, &self->limits.max_samples);

    if(gl->EXT_texture_filter_anisotropic)
        gl->GetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &self->limits.max_anisotropy);
    else
        self->limits.max_anisotropy = 1.0f;

    /*
     * Everything's init'd, quirkify.
     */
    setup_quirks(self);

    self->tex_white        = build_white_texture(gl);
    self->tex_checkerboard = build_checkerboard_texture(gl);

    /*
     * We can't use BRender's fonts directly, so build a POT texture with
     * glyph from left-to-right. All fonts have 256 possible characters.
     */

    BrLogTrace("GLREND", "Building fixed 3x5 font array.");
    (void)ShaderGLTextBuildFont(gl, &self->font_fixed3x5, BrFontFixed3x5);

    BrLogTrace("GLREND", "Building proportional 4x6 font array.");
    (void)ShaderGLTextBuildFont(gl, &self->font_prop4x6, BrFontProp4x6);

    BrLogTrace("GLREND", "Building proportional 7x9 font array.");
    (void)ShaderGLTextBuildFont(gl, &self->font_prop7x9, BrFontProp7x9);

    ShaderGLLineCompile(&self->line_shader, gl);
    ShaderGLRectCompile(&self->rect_shader, gl);
    ShaderGLTextCompile(&self->text_shader, gl);
    ShaderGLMainCompile(&self->main_shader, gl, NULL, NULL);

    return BRE_OK;
}

void ContextStateGLFini(br_gl_context_state *self)
{
    const GladGLContext *gl = self->gl;

    gl->UseProgram(0);
    gl->BindVertexArray(0);
    gl->BindBuffer(GL_ARRAY_BUFFER, 0);
    gl->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    gl->BindBuffer(GL_UNIFORM_BUFFER, 0);

    gl->DeleteProgram(self->main_shader.program);
    gl->DeleteBuffers(1, &self->main_shader.ubo_scene);

    gl->DeleteProgram(self->text_shader.program);
    gl->DeleteVertexArrays(1, &self->text_shader.vao_glyphs);
    gl->DeleteBuffers(1, &self->text_shader.ubo_glyphs);

    gl->DeleteProgram(self->rect_shader.program);
    gl->DeleteVertexArrays(1, &self->rect_shader.vao);
    gl->DeleteBuffers(1, &self->rect_shader.ubo);

    gl->DeleteProgram(self->line_shader.program);
    gl->DeleteVertexArrays(1, &self->line_shader.vao);
    gl->DeleteBuffers(1, &self->line_shader.ubo);

    gl->DeleteTextures(1, &self->font_prop7x9.tex);
    gl->DeleteTextures(1, &self->font_prop4x6.tex);
    gl->DeleteTextures(1, &self->font_fixed3x5.tex);
    gl->DeleteTextures(1, &self->tex_checkerboard);
    gl->DeleteTextures(1, &self->tex_white);
}
