#include "drv.h"

typedef struct br_gl_state_cache {
    PFNGLENABLEPROC            Enable;
    PFNGLENABLEPROC            Disable;
    PFNGLISENABLEDPROC         IsEnabled;
    PFNGLFRONTFACEPROC         FrontFace;
    PFNGLPOLYGONMODEPROC       PolygonMode;
    PFNGLCULLFACEPROC          CullFace;
    PFNGLDEPTHFUNCPROC         DepthFunc;
    PFNGLDEPTHMASKPROC         DepthMask;
    PFNGLCOLORMASKPROC         ColorMask;
    PFNGLBINDSAMPLERPROC       BindSampler;
    PFNGLBLENDFUNCSEPARATEPROC BlendFuncSeparate;

    struct {
        int depth_test;
        int blend;
        int cull_face;
        int scissor_test;
    } caps;

    GLenum    front_face;
    GLenum    polygon_mode;
    GLenum    cull_face;
    GLenum    depth_func;
    GLboolean depth_mask;
    GLuint    colour_mask;

    GLuint samplers[1];

    struct {
        GLenum src_rgb;
        GLenum dst_rgb;
        GLenum src_alpha;
        GLenum dst_alpha;
    } blend_func;
} br_gl_state_cache;

static br_gl_state_cache gl_state_cache;

static int *glcGetState(GLenum cap)
{
    br_gl_state_cache *cache = &gl_state_cache;

    switch(cap) {
        case GL_DEPTH_TEST:
            return &cache->caps.depth_test;

        case GL_BLEND:
            return &cache->caps.blend;

        case GL_CULL_FACE:
            return &cache->caps.cull_face;

        case GL_SCISSOR_TEST:
            return &cache->caps.scissor_test;

        default:
            return NULL;
    }
}

static void glcEnable(GLenum cap)
{
    br_gl_state_cache *cache = &gl_state_cache;
    int               *pcap  = glcGetState(cap);

    if(pcap == NULL) {
        cache->Enable(cap);
        return;
    }

    if(!(*pcap)) {
        cache->Enable(cap);
        *pcap = 1;
    }
}

static void glcDisable(GLenum cap)
{
    br_gl_state_cache *cache = &gl_state_cache;
    int               *pcap  = glcGetState(cap);

    if(pcap == NULL) {
        cache->Disable(cap);
        return;
    }

    if(*pcap) {
        cache->Disable(cap);
        *pcap = 0;
    }
}

static GLboolean glcIsEnabled(GLenum cap)
{
    br_gl_state_cache *cache = &gl_state_cache;
    int               *pcap  = glcGetState(cap);

    if(pcap == NULL)
        return cache->IsEnabled(cap);

    return *pcap;
}

static void glcFrontFace(GLenum mode)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(cache->front_face != mode) {
        cache->FrontFace(mode);
        cache->front_face = mode;
    }
}

static void glcCullFace(GLenum mode)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(cache->cull_face != mode) {
        cache->CullFace(mode);
        cache->cull_face = mode;
    }
}

static void glcPolygonMode(GLenum face, GLenum mode)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(face != GL_FRONT_AND_BACK) {
        cache->PolygonMode(face, mode);
        return;
    }

    if(cache->polygon_mode != mode) {
        cache->PolygonMode(face, mode);
        cache->polygon_mode = mode;
    }
}

static void glcDepthFunc(GLenum func)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(cache->depth_func != func) {
        cache->DepthFunc(func);
        cache->depth_func = func;
    }
}

static void glcDepthMask(GLboolean flag)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(cache->depth_mask != flag) {
        cache->DepthMask(flag);
        cache->depth_mask = flag;
    }
}

void glcColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    br_gl_state_cache *cache = &gl_state_cache;

    GLuint mask = (red << 3) | (green << 2) | (blue << 1) | (alpha << 0);

    if(cache->colour_mask != mask) {
        cache->ColorMask(red, green, blue, alpha);
        cache->colour_mask = mask;
    }
}

void glcBindSampler(GLuint unit, GLuint sampler)
{
    br_gl_state_cache *cache = &gl_state_cache;

    if(unit >= BR_ASIZE(cache->samplers)) {
        cache->BindSampler(unit, sampler);
        return;
    }

    if(sampler != cache->samplers[unit]) {
        cache->BindSampler(unit, sampler);
        cache->samplers[unit] = sampler;
    }
}

void glcBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    br_gl_state_cache *cache = &gl_state_cache;

    int diff = 0;

    diff = diff || srcRGB != cache->blend_func.src_rgb;
    diff = diff || dstRGB != cache->blend_func.dst_rgb;
    diff = diff || srcAlpha != cache->blend_func.src_alpha;
    diff = diff || dstAlpha != cache->blend_func.dst_alpha;

    if(diff) {
        cache->BlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
        cache->blend_func.src_rgb   = srcRGB;
        cache->blend_func.dst_rgb   = dstRGB;
        cache->blend_func.src_alpha = srcAlpha;
        cache->blend_func.dst_alpha = dstAlpha;
    }
}

void GLStateCacheSync(GladGLContext *gl)
{
    br_gl_state_cache *cache = &gl_state_cache;
    GLint              vals[8];

    gl->GetIntegerv(GL_DEPTH_TEST, vals);
    cache->caps.depth_test = vals[0];

    gl->GetIntegerv(GL_BLEND, vals);
    cache->caps.blend = vals[0];

    gl->GetIntegerv(GL_CULL_FACE, vals);
    cache->caps.cull_face = vals[0];

    gl->GetIntegerv(GL_SCISSOR_TEST, vals);
    cache->caps.cull_face = vals[0];

    gl->GetIntegerv(GL_FRONT_FACE, vals);
    cache->front_face = vals[0];

    /*
     * GL_POLYGON_MODE returns (front, back) despite Core only supporting GL_FRONT_AND_BACK.
     */
    gl->GetIntegerv(GL_POLYGON_MODE, vals);
    cache->polygon_mode = vals[0];

    gl->GetIntegerv(GL_CULL_FACE_MODE, vals);
    cache->cull_face = vals[0];

    gl->GetIntegerv(GL_DEPTH_FUNC, vals);
    cache->depth_func = vals[0];

    gl->GetIntegerv(GL_DEPTH_WRITEMASK, vals);
    cache->depth_mask = vals[0];

    gl->GetIntegerv(GL_COLOR_WRITEMASK, vals);
    cache->colour_mask = (vals[0] << 3) | (vals[1] << 2) | (vals[2] << 1) | (vals[3] << 0);

    gl->BindSampler(0, 0);
    cache->samplers[0] = 0;

    gl->GetIntegerv(GL_BLEND_SRC_RGB, vals);
    cache->blend_func.src_rgb = vals[0];

    gl->GetIntegerv(GL_BLEND_SRC_ALPHA, vals);
    cache->blend_func.src_alpha = vals[0];

    gl->GetIntegerv(GL_BLEND_DST_RGB, vals);
    cache->blend_func.dst_rgb = vals[0];

    gl->GetIntegerv(GL_BLEND_DST_ALPHA, vals);
    cache->blend_func.dst_alpha = vals[0];
}

void GLStateCacheAttach(GladGLContext *gl)
{
    br_gl_state_cache *cache = &gl_state_cache;

    *cache = (br_gl_state_cache){
        .Enable            = gl->Enable,
        .Disable           = gl->Disable,
        .IsEnabled         = gl->IsEnabled,
        .FrontFace         = gl->FrontFace,
        .PolygonMode       = gl->PolygonMode,
        .CullFace          = gl->CullFace,
        .DepthFunc         = gl->DepthFunc,
        .DepthMask         = gl->DepthMask,
        .ColorMask         = gl->ColorMask,
        .BindSampler       = gl->BindSampler,
        .BlendFuncSeparate = gl->BlendFuncSeparate,
    };

    gl->Enable            = glcEnable;
    gl->Disable           = glcDisable;
    gl->IsEnabled         = glcIsEnabled;
    gl->FrontFace         = glcFrontFace;
    gl->PolygonMode       = glcPolygonMode;
    gl->CullFace          = glcCullFace;
    gl->DepthFunc         = glcDepthFunc;
    gl->DepthMask         = glcDepthMask;
    gl->ColorMask         = glcColorMask;
    gl->BindSampler       = glcBindSampler;
    gl->BlendFuncSeparate = glcBlendFuncSeparate;

    GLStateCacheSync(gl);
}

void GLStateCacheDetach(GladGLContext *gl)
{
    br_gl_state_cache *cache = &gl_state_cache;

    gl->Enable            = cache->Enable;
    gl->Disable           = cache->Disable;
    gl->IsEnabled         = cache->IsEnabled;
    gl->FrontFace         = cache->FrontFace;
    gl->PolygonMode       = cache->PolygonMode;
    gl->CullFace          = cache->CullFace;
    gl->DepthFunc         = cache->DepthFunc;
    gl->DepthMask         = cache->DepthMask;
    gl->ColorMask         = cache->ColorMask;
    gl->BindSampler       = cache->BindSampler;
    gl->BlendFuncSeparate = cache->BlendFuncSeparate;
}