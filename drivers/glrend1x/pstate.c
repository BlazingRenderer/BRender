/*
 * Primitive state methods
 *
 * We're basically a dumb rasteriser. BRender's software pipeline
 * handles transforms, lighting, fog, and clipping. The only state
 * we need to push to GL is rasterization control: blend mode, depth test,
 * texture binding, shade model, dither, and depth write masks.
 */
#include "drv.h"

static const struct br_primitive_state_dispatch primitiveStateDispatch;

static const state_primitive default_state_primitive = {
    .flags            = PRIMF_DEPTH_WRITE,
    .colour_type      = BRT_DEFAULT,
    .depth_test       = BRT_LESS,
    .blend_mode       = BRT_NONE,
    .shading_mode     = BRT_FLAT,
    .perspective_type = BRT_NONE,
    .colour_map       = NULL,
    .filter           = BRT_NONE,
    .fog_type         = BRT_NONE,
    .fog_min          = BR_SCALAR(256),
    .fog_max          = BR_SCALAR(512),
    .fog_colour       = BR_COLOUR_RGBA(255, 255, 255, 0),
};

static const state_output default_state_output = {
    .colour = NULL,
    .depth  = NULL,
};

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define F(f) offsetof(br_primitive_state, f)
static br_tv_template_entry primitiveStateTemplateEntries[] = {
    {BRT(IDENTIFIER_CSTR), F(identifier),                      Q | A,            BRTV_CONV_COPY},
    {BRT(PARTS_TL),        (br_uintptr_t)&PrimPartsGL1xTokens, Q | A | BRTV_ABS, BRTV_CONV_LIST},
};
#undef F

#define F(f) offsetof(br_primitive_state, f)
#define P(f) ((br_uintptr_t)(&(f)))

static br_tv_template_entry partPrimitiveTemplateEntries[] = {
    {BRT(DEPTH_WRITE_B),       F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_DEPTH_WRITE,   1},
    {BRT(BLEND_B),             F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_BLEND,         1},
    {BRT(MODULATE_B),          F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_MODULATE,      1},
    {BRT(COLOUR_KEY_B),        F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_COLOUR_KEY,    1},
    {BRT(DITHER_COLOUR_B),     F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_DITHER_COLOUR, 1},

    {BRT(COLOUR_T),            F(prim.colour_type),      Q | S | A, BRTV_CONV_COPY,         0,                   0},

    {BRT(DEPTH_TEST_T),        F(prim.depth_test),       Q | S | A, BRTV_CONV_COPY,         0,                   0},
    {BRT(BLEND_MODE_T),        F(prim.blend_mode),       Q | S | A, BRTV_CONV_COPY,         0,                   0},
    {BRT(MAP_WIDTH_LIMIT_T),   F(prim.map_width_limit),  Q | S | A, BRTV_CONV_COPY,         0,                   1},
    {BRT(MAP_HEIGHT_LIMIT_T),  F(prim.map_height_limit), Q | S | A, BRTV_CONV_COPY,         0,                   1},
    {BRT(SHADING_MODE_T),      F(prim.shading_mode),     Q | S | A, BRTV_CONV_COPY,         0,                   0},

    {BRT(PERSPECTIVE_T),       F(prim.perspective_type), Q | S | A, BRTV_CONV_COPY,         0,                   0},
    {BRT(PERSPECTIVE_B),       F(prim.perspective_type), S,         BRTV_CONV_BOOL_TOKEN,   BRT_DEFAULT,         1},

    {BRT(COLOUR_MAP_O),        F(prim.colour_map),       Q | S | A, BRTV_CONV_COPY,         0,                   0},
    {BRT(TEXTURE_O),           F(prim.colour_map),       Q | S,     BRTV_CONV_COPY,         0,                   0},

    {BRT(MAP_INTERPOLATION_T), F(prim.filter),           Q | S | A, BRTV_CONV_COPY,         0,                   0},

    {BRT(FOG_B),               F(prim.flags),            Q | S | A, BRTV_CONV_BIT,          PRIMF_FOG,           1},
    {BRT(FOG_T),               F(prim.fog_type),         Q | S | A, BRTV_CONV_COPY,         0,                   0},
    {BRT(FOG_MIN_X),           F(prim.fog_min),          Q | S | A, BRTV_CONV_FIXED_SCALAR, 0,                   0},
    {BRT(FOG_MIN_F),           F(prim.fog_min),          Q | S | A, BRTV_CONV_FLOAT_SCALAR, 0,                   0},
    {BRT(FOG_MAX_X),           F(prim.fog_max),          Q | S | A, BRTV_CONV_FIXED_SCALAR, 0,                   0},
    {BRT(FOG_MAX_F),           F(prim.fog_max),          Q | S | A, BRTV_CONV_FLOAT_SCALAR, 0,                   0},
    {BRT(FOG_RGB),             F(prim.fog_colour),       Q | S | A, BRTV_CONV_COPY,         0,                   0},
};

static br_tv_template_entry partOutputTemplateEntries[] = {
    {BRT(COLOUR_BUFFER_O), F(out.colour), Q | S | A, BRTV_CONV_COPY},
    {BRT(DEPTH_BUFFER_O),  F(out.depth),  Q | S | A, BRTV_CONV_COPY},
};

#undef F
#undef P

#undef S
#undef Q
#undef A

#define BLOCK_VERTEX_COMPONENTS CM_SX | CM_SY | CM_SZ | CM_R | CM_G | CM_B | CM_A | CM_U | CM_V
#define BLOCK_CONVERT_MASK \
    (1 << C_SX) | (1 << C_SY) | (1 << C_SZ) | (1 << C_W) | (1 << C_R) | (1 << C_G) | (1 << C_B) | (1 << C_A) | (1 << C_U) | (1 << C_V)

#define MAKE_BLOCK(render_, type_, flags_)             \
    {                                                  \
        .render            = (brp_render_fn *)render_, \
        .type              = type_,                    \
        .flags             = flags_,                   \
        .vertex_components = BLOCK_VERTEX_COMPONENTS,  \
        .convert_mask_f    = BLOCK_CONVERT_MASK,       \
    }

enum {
    BR_BLOCKF_BLENDED = 1 << 0,
    BR_BLOCKF_SUBDIV  = 1 << 1,
};

#define MAKE_BLOCKS(name, render_, type_)                                                                               \
    static brp_block block_##name                  = MAKE_BLOCK(render_, type_, 0);                                     \
    static brp_block block_##name##_blended        = MAKE_BLOCK(render_, type_, BR_PRIMF_BLENDED);                      \
    static brp_block block_##name##_subdiv         = MAKE_BLOCK(render_, type_, BR_PRIMF_SUBDIVIDE);                    \
    static brp_block block_##name##_blended_subdiv = MAKE_BLOCK(render_, type_, BR_PRIMF_BLENDED | BR_PRIMF_SUBDIVIDE); \
                                                                                                                        \
    static brp_block *const blockmap_##name[] = {                                                                       \
        [0]                                    = &block_##name,                                                         \
        [BR_BLOCKF_BLENDED]                    = &block_##name##_blended,                                               \
        [BR_BLOCKF_SUBDIV]                     = &block_##name##_subdiv,                                                \
        [BR_BLOCKF_BLENDED | BR_BLOCKF_SUBDIV] = &block_##name##_blended_subdiv,                                        \
    };

MAKE_BLOCKS(triangle, TriangleRenderGL1x, BRT_TRIANGLE);
MAKE_BLOCKS(line, LineRenderGL1x, BRT_LINE);
MAKE_BLOCKS(point, PointRenderGL1x, BRT_POINT);

static brp_block block_null = {};

typedef struct br_state_info_gl {
    GLuint colour_map;

    struct {
        br_uint_8 is_blended : 1;
        br_uint_8 is_filtered : 1;
        br_uint_8 is_indexed : 1;
        br_uint_8 disable_colour_key : 1;
        br_uint_8 write_colour : 1;
        br_uint_8 write_depth : 1;
        br_uint_8 fog : 1;
        br_uint_8 dither : 1;
        br_uint_8 perspective : 1;
    };
    GLenum depth_func;
    GLint  texture_env_mode;
    GLint  filter_min;
    GLint  filter_mag;
    GLint  wrap_s;
    GLint  wrap_t;

    br_vector4 fog_colour;
    br_scalar  fog_min;
    br_scalar  fog_max;

    br_token shading_mode;
} br_primitive_state_info_gl;

static void DeviceGL1xExtractPrimitiveState(const br_primitive_state *state, br_primitive_state_info_gl *info, GLuint tex_white)
{
    const state_primitive *prim = &state->prim;

    *info = (br_primitive_state_info_gl){
        .is_blended         = BR_FALSE,
        .is_filtered        = BR_FALSE,
        .is_indexed         = BR_FALSE,
        .disable_colour_key = BR_FALSE,
        .write_colour       = BR_TRUE,
        .write_depth        = BR_TRUE,
        .perspective        = BR_FALSE,
        .depth_func         = GL_LESS,
        .texture_env_mode   = GL_MODULATE,
        .filter_min         = GL_NEAREST,
        .filter_mag         = GL_NEAREST,
        .wrap_s             = GL_REPEAT,
        .wrap_t             = GL_REPEAT,
        .fog                = BR_FALSE,
        .fog_colour         = BR_VECTOR4(BR_SCALAR(1), BR_SCALAR(1), BR_SCALAR(1), BR_SCALAR(1)),
        .fog_min            = 0,
        .fog_max            = 0,
        .shading_mode       = BRT_FLAT,
    };

    info->is_blended         = (prim->flags & PRIMF_BLEND) != 0;
    info->disable_colour_key = (prim->flags & PRIMF_COLOUR_KEY) == 0;
    info->write_colour       = 1;
    info->write_depth        = (prim->flags & PRIMF_DEPTH_WRITE) != 0;
    info->is_indexed         = prim->colour_map ? (prim->colour_map->fmt->indexed != 0) : 0;
    info->fog                = (prim->flags & PRIMF_FOG) && prim->fog_type != BRT_NONE;
    info->shading_mode       = prim->shading_mode;
    info->dither             = (prim->flags & PRIMF_DITHER_COLOUR) != 0;
    info->perspective        = prim->perspective_type == BRT_DEFAULT || prim->perspective_type == BRT_SUBDIVIDE;

    if(prim->flags & PRIMF_MODULATE)
        info->texture_env_mode = GL_MODULATE;

    if(prim->colour_map != NULL) {
        const br_buffer_stored *stored = prim->colour_map;

        info->is_indexed = stored->fmt->indexed;
        info->colour_map = BufferStoredGL1xGetTexture(stored, !info->disable_colour_key);
    } else {
        info->is_indexed = BR_FALSE;
        info->colour_map = tex_white;
    }

    /* Don't depth write with transparent primitives. */
    info->write_depth = !info->is_blended && info->write_depth;

    if(prim->filter == BRT_LINEAR) {
        info->filter_min  = GL_LINEAR;
        info->filter_mag  = GL_LINEAR;
        info->is_filtered = 1;
    } else {
        info->filter_min  = GL_NEAREST;
        info->filter_mag  = GL_NEAREST;
        info->is_filtered = 0;
    }

    /*
     * Apply wrapping.
     */
    switch(prim->map_width_limit) {
        case BRT_WRAP:
        case BRT_MIRROR: /* Unsupported */
        default:
            info->wrap_s = GL_REPEAT;
            break;

        case BRT_CLAMP:
            info->wrap_s = GL_CLAMP_TO_EDGE;
            break;
    }

    switch(prim->map_height_limit) {
        case BRT_WRAP:
        case BRT_MIRROR: /* Unsupported */
        default:
            info->wrap_t = GL_REPEAT;
            break;

        case BRT_CLAMP:
            info->wrap_t = GL_CLAMP_TO_EDGE;
            break;
    }

    switch(prim->depth_test) {
        case BRT_LESS:
        default:
            info->depth_func = GL_LESS;
            break;

        case BRT_GREATER:
            info->depth_func = GL_GREATER;
            break;

        case BRT_LESS_OR_EQUAL:
            info->depth_func = GL_LEQUAL;
            break;

        case BRT_GREATER_OR_EQUAL:
            info->depth_func = GL_GEQUAL;
            break;

        case BRT_EQUAL:
            info->depth_func = GL_EQUAL;
            break;

        case BRT_NOT_EQUAL:
            info->depth_func = GL_NOTEQUAL;
            break;

        case BRT_NEVER:
            info->depth_func = GL_NEVER;
            break;

        case BRT_ALWAYS:
            info->depth_func = GL_ALWAYS;
            break;
    }

    if(prim->fog_type != BRT_NONE) {
        info->fog_colour.v[0] = BR_RED(prim->fog_colour) / 255.0f;
        info->fog_colour.v[1] = BR_GRN(prim->fog_colour) / 255.0f;
        info->fog_colour.v[2] = BR_BLU(prim->fog_colour) / 255.0f;
        info->fog_colour.v[3] = 1.0f;

        info->fog_min = prim->fog_min;
        info->fog_max = prim->fog_max;

        if(info->fog_max == info->fog_min)
            info->fog_max += 0.01f;
    }
}

static void apply_blend_mode(br_token blend_mode, const GladGLContext *gl)
{
    /* C_result = (C_source * F_Source) + (C_dest * F_dest) */

    switch(blend_mode) {
        default:
            /* fallthrough */
        case BRT_BLEND_STANDARD:
            /* fallthrough */
        case BRT_BLEND_DIMMED:
            gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;

        case BRT_BLEND_SUMMED:
            gl->BlendFunc(GL_SRC_ALPHA, GL_ONE);
            break;

        case BRT_BLEND_PREMULTIPLIED:
            gl->BlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
}

static void pushGLState(const GladGLContext *gl, br_primitive_state *self, const br_primitive_state_info_gl info)
{
    switch(info.shading_mode) {
        case BRT_FLAT:
            gl->ShadeModel(GL_FLAT);
            break;

        default:
            gl->ShadeModel(GL_SMOOTH);
            break;
    }

    if(info.write_colour) {
        gl->ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    } else {
        gl->ColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }

    if(info.is_blended) {
        gl->Enable(GL_BLEND);
        apply_blend_mode(self->prim.blend_mode, gl);
    } else {
        gl->Disable(GL_BLEND);
    }

    gl->DepthFunc(info.depth_func);
    gl->DepthMask(info.write_depth ? GL_TRUE : GL_FALSE);

    br_boolean depth_test = BR_TRUE;

    if(self->out.depth == NULL)
        depth_test = BR_FALSE;

    if(info.fog) {
        GLfloat fog_colour[4] = {
            (GLfloat)BR_RED(self->prim.fog_colour) / 255.0f,
            (GLfloat)BR_GRN(self->prim.fog_colour) / 255.0f,
            (GLfloat)BR_BLU(self->prim.fog_colour) / 255.0f,
            1.0f,
        };

        gl->Enable(GL_FOG);
        gl->Fogi(GL_FOG_MODE, GL_LINEAR);
        gl->Fogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
        gl->Fogfv(GL_FOG_COLOR, fog_colour);
        gl->Fogf(GL_FOG_START, info.fog_min);
        gl->Fogf(GL_FOG_END, info.fog_max);
    } else {
        gl->Disable(GL_FOG);
        gl->Fogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FRAGMENT_DEPTH_EXT);
    }

    if(info.disable_colour_key || info.is_blended)
        gl->Disable(GL_ALPHA_TEST);
    else {
        gl->Enable(GL_ALPHA_TEST);
        gl->AlphaFunc(GL_GREATER, 0.5f);
    }

    info.dither ? gl->Enable(GL_DITHER) : gl->Disable(GL_DITHER);

    gl->Enable(GL_TEXTURE_2D);
    gl->TexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, info.texture_env_mode);

    if(depth_test == BR_TRUE)
        gl->Enable(GL_DEPTH_TEST);
    else
        gl->Disable(GL_DEPTH_TEST);

    /*
     * This doesn't actually do anything on modern drivers.
     */
    if(info.perspective) {
        gl->Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    } else {
        gl->Hint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    }
}

static br_tv_template *findTemplate(br_primitive_state *self, br_token part)
{
    switch(part) {
        case BRT_PRIMITIVE:
            if(self->device->templates.partPrimitiveTemplate == NULL)
                self->device->templates.partPrimitiveTemplate = BrTVTemplateAllocate(self->device, partPrimitiveTemplateEntries,
                                                                                     BR_ASIZE(partPrimitiveTemplateEntries));
            return self->device->templates.partPrimitiveTemplate;

        case BRT_OUTPUT:
            if(self->device->templates.partOutputTemplate == NULL)
                self->device->templates.partOutputTemplate = BrTVTemplateAllocate(self->device, partOutputTemplateEntries,
                                                                                  BR_ASIZE(partOutputTemplateEntries));
            return self->device->templates.partOutputTemplate;

        default:
            break;
    }

    return NULL;
}

br_primitive_state *PrimitiveStateGL1xAllocate(br_primitive_library *plib)
{
    br_primitive_state *self;

    self = BrResAllocate(DeviceGLResource(ObjectDevice(plib)), sizeof(*self), BR_MEMORY_OBJECT);

    if(self == NULL)
        return NULL;

    self->plib       = plib;
    self->device     = ObjectDevice(plib);
    self->identifier = BrResStrDup(self, "Primitive-State");
    self->dispatch   = &primitiveStateDispatch;

    ObjectContainerAddFront(plib, (br_object *)self);

    self->prim = default_state_primitive;
    self->out  = default_state_output;

    return self;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, renderBegin)(br_primitive_state *self, brp_block **rpb, br_boolean *block_changed,
                                                                      br_boolean *ranges_changed, br_boolean no_render, br_token prim_type)
{
    const GladGLContext       *gl = self->plib->gl;
    br_primitive_state_info_gl info;

    DeviceGL1xExtractPrimitiveState(self, &info, self->plib->tex_white);

    if(block_changed)
        *block_changed = BR_TRUE;
    if(ranges_changed)
        *ranges_changed = BR_TRUE;

    if(!no_render) {
        if(self->out.colour != NULL) {
            br_device_pixelmap *pm = self->out.colour;
            br_rectangle        vp;
            GLsizei             w, h;

            vp = DevicePixelmapGL1xGetViewport(pm);
            w  = (GLsizei)vp.w;
            h  = (GLsizei)vp.h;

            gl->Viewport(vp.x, vp.y, w, h);
            gl->MatrixMode(GL_PROJECTION);
            gl->LoadIdentity();
            gl->Ortho(0.0, (double)w, (double)h, 0.0, -1.0, 1.0);
            gl->MatrixMode(GL_MODELVIEW);
            gl->LoadIdentity();
        }

        pushGLState(gl, self, info);

        gl->BindTexture(GL_TEXTURE_2D, info.colour_map);

        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, info.filter_min);
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, info.filter_mag);
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, info.wrap_s);
        gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, info.wrap_t);

        if(prim_type == BRT_TRIANGLE)
            gl->Begin(GL_TRIANGLES);
        else if(prim_type == BRT_LINE)
            gl->Begin(GL_LINES);
        else if(prim_type == BRT_POINT)
            gl->Begin(GL_POINTS);
    }

    brp_block        *block    = NULL;
    brp_block *const *blockmap = NULL;
    // clang-format off
    switch(prim_type) {
        case BRT_TRIANGLE: blockmap = blockmap_triangle; break;
        case BRT_LINE:     blockmap = blockmap_line;     break;
        case BRT_POINT:    blockmap = blockmap_point;    break;
        default:                                         break;
    }
    // clang-format on

    if(blockmap != NULL) {
        br_uint_32 ff = 0;
        ff |= info.is_blended ? BR_BLOCKF_BLENDED : 0;
        ff |= info.perspective ? BR_BLOCKF_SUBDIV : 0;

        block = blockmap[ff];
    }

    if(block == NULL)
        block = &block_null;

    *rpb = block;

    SetRendfuncGL1xContext(gl, self);

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, renderEnd)(br_primitive_state *self, brp_block *rpb)
{
    const GladGLContext *gl = self->plib->gl;

    if(rpb && rpb->render)
        gl->End();

    DeviceGL1xCheckErrors(gl);
    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, rangesQueryF)(br_primitive_state *self, br_float *offsets, br_float *scales, br_int_32 max_comp)
{
    br_int_32 c;
    for(c = 0; c < max_comp; c++) {
        offsets[c] = 0.0f;
        scales[c]  = 1.0f;
    }

    if(max_comp > C_SX && self->out.colour != NULL) {
        br_float w = (br_float)self->out.colour->pm_width;
        br_float h = (br_float)self->out.colour->pm_height;

        offsets[C_SX] = w * 0.5f;
        scales[C_SX]  = w * 0.5f;
        offsets[C_SY] = h * 0.5f;
        scales[C_SY]  = -h * 0.5f;
    }

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, stateCopy)(br_primitive_state *self, br_primitive_state *src, br_uint_32 mask)
{
    if(mask & MASK_STATE_PRIMITIVE)
        self->prim = src->prim;

    if(mask & MASK_STATE_OUTPUT)
        self->out = src->out;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, stateDefault)(br_primitive_state *self, br_uint_32 mask)
{
    if(mask & MASK_STATE_PRIMITIVE) {
        self->prim = default_state_primitive;
    }

    if(mask & MASK_STATE_OUTPUT) {
        self->out = default_state_output;
    }

    return BRE_OK;
}

static void BR_CMETHOD_DECL(br_primitive_state_gl, free)(br_object *_self)
{
    br_primitive_state *self = (br_primitive_state *)_self;
    ObjectContainerRemove(self->plib, (br_object *)self);
    BrResFreeNoCallback(self);
}

static const char *BR_CMETHOD_DECL(br_primitive_state_gl, identifier)(br_object *self)
{
    return ((br_primitive_state *)self)->identifier;
}
static br_device *BR_CMETHOD_DECL(br_primitive_state_gl, device)(br_object *self)
{
    return ((br_primitive_state *)self)->device;
}

static br_token BR_CMETHOD_DECL(br_primitive_state_gl, type)(br_object *_self)
{
    return BRT_PRIMITIVE_STATE;
}

static br_boolean BR_CMETHOD_DECL(br_primitive_state_gl, isType)(br_object *_self, br_token t)
{
    return (t == BRT_PRIMITIVE_STATE) || (t == BRT_OBJECT);
}

static br_size_t BR_CMETHOD_DECL(br_primitive_state_gl, space)(br_object *_self)
{
    return sizeof(br_primitive_state);
}

static br_tv_template *BR_CMETHOD_DECL(br_primitive_state_gl, templateQuery)(br_object *_self)
{
    br_primitive_state *self = (br_primitive_state *)_self;

    if(self->device->templates.primitiveStateTemplate == NULL) {
        self->device->templates.primitiveStateTemplate = BrTVTemplateAllocate(self->device, primitiveStateTemplateEntries,
                                                                              BR_ASIZE(primitiveStateTemplateEntries));
    }

    return self->device->templates.primitiveStateTemplate;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partSet)(br_primitive_state *self, br_token part, br_int_32 index, br_token t, br_value value)
{
    br_tv_template *tp = findTemplate(self, part);
    br_uint_32      m;

    if(tp == NULL)
        return BRE_FAIL;

    m = 0;
    return BrTokenValueSet(self, &m, t, value, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partSetMany)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                      br_token_value *tv, br_int_32 *pcount)
{
    br_tv_template *tp = findTemplate(self, part);
    br_uint_32      m;

    if(tp == NULL)
        return BRE_FAIL;

    m = 0;
    return BrTokenValueSetMany(self, pcount, &m, tv, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQuery)(br_primitive_state *self, br_token part, br_int_32 index, void *pvalue,
                                                                    br_token t)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, NULL, 0, t, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryBuffer)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                          void *pvalue, void *buffer, br_size_t buffer_size, br_token t)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQuery(pvalue, buffer, buffer_size, t, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryMany)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                        br_token_value *tv, void *extra, br_size_t extra_size, br_int_32 *pcount)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryMany(tv, extra, extra_size, pcount, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryManySize)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                            br_size_t *pextra_size, br_token_value *tv)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryManySize(pextra_size, tv, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryAll)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                       br_token_value *buffer, br_size_t buffer_size)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAll(buffer, buffer_size, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryAllSize)(br_primitive_state *self, br_token part, br_int_32 index, br_size_t *psize)
{
    br_tv_template *tp = findTemplate(self, part);

    if(tp == NULL)
        return BRE_FAIL;

    return BrTokenValueQueryAllSize(psize, self, tp);
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partQueryCapability)(br_primitive_state *self, br_token part, br_int_32 index,
                                                                              br_token_value *buffer, br_size_t buffer_size)
{
    return BRE_FAIL;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, partIndexQuery)(br_primitive_state *self, br_token part, br_int_32 *pnindex)
{
    if(pnindex == NULL)
        return BRE_FAIL;

    if(part == BRT_OUTPUT || part == BRT_PRIMITIVE)
        *pnindex = 1;
    else
        *pnindex = 0;

    return BRE_OK;
}

static br_error BR_CMETHOD_DECL(br_primitive_state_gl1x, stateQueryPerformance)(br_primitive_state *self, br_fixed_lu *speed)
{
    return BRE_FAIL;
}

static const struct br_primitive_state_dispatch primitiveStateDispatch = {
    .__reserved0 = NULL,
    .__reserved1 = NULL,
    .__reserved2 = NULL,
    .__reserved3 = NULL,
    ._free       = BR_CMETHOD_REF(br_primitive_state_gl, free),
    ._identifier = BR_CMETHOD_REF(br_primitive_state_gl, identifier),
    ._type       = BR_CMETHOD_REF(br_primitive_state_gl, type),
    ._isType     = BR_CMETHOD_REF(br_primitive_state_gl, isType),
    ._device     = BR_CMETHOD_REF(br_primitive_state_gl, device),
    ._space      = BR_CMETHOD_REF(br_primitive_state_gl, space),

    ._templateQuery = BR_CMETHOD_REF(br_primitive_state_gl, templateQuery),
    ._query         = BR_CMETHOD_REF(br_object, query),
    ._queryBuffer   = BR_CMETHOD_REF(br_object, queryBuffer),
    ._queryMany     = BR_CMETHOD_REF(br_object, queryMany),
    ._queryManySize = BR_CMETHOD_REF(br_object, queryManySize),
    ._queryAll      = BR_CMETHOD_REF(br_object, queryAll),
    ._queryAllSize  = BR_CMETHOD_REF(br_object, queryAllSize),

    ._partSet           = BR_CMETHOD_REF(br_primitive_state_gl1x, partSet),
    ._partSetMany       = BR_CMETHOD_REF(br_primitive_state_gl1x, partSetMany),
    ._partQuery         = BR_CMETHOD_REF(br_primitive_state_gl1x, partQuery),
    ._partQueryBuffer   = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryBuffer),
    ._partQueryMany     = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryMany),
    ._partQueryManySize = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryManySize),
    ._partQueryAll      = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryAll),
    ._partQueryAllSize  = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryAllSize),

    ._partIndexQuery = BR_CMETHOD_REF(br_primitive_state_gl1x, partIndexQuery),

    ._stateDefault = BR_CMETHOD_REF(br_primitive_state_gl1x, stateDefault),
    ._stateCopy    = BR_CMETHOD_REF(br_primitive_state_gl1x, stateCopy),

    ._renderBegin = BR_CMETHOD_REF(br_primitive_state_gl1x, renderBegin),
    ._renderEnd   = BR_CMETHOD_REF(br_primitive_state_gl1x, renderEnd),

    ._rangesQuery = BR_CMETHOD_REF(br_primitive_state_gl1x, rangesQueryF),

    ._partQueryCapability   = BR_CMETHOD_REF(br_primitive_state_gl1x, partQueryCapability),
    ._stateQueryPerformance = BR_CMETHOD_REF(br_primitive_state_gl1x, stateQueryPerformance),
};
