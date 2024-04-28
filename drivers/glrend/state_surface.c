#include "drv.h"

#define S    BRTV_SET
#define Q    BRTV_QUERY
#define A    BRTV_ALL

#define AX   0
#define AF   BRTV_ALL

#define F(f) offsetof(state_stack, f)

static br_tv_template_entry template_entries[] = {
    {BRT_COLOUR_RGB,       NULL, F(surface.colour),        Q | S | AX, BRTV_CONV_COPY,             0, TM_PART                },

    {BRT_OPACITY_X,        NULL, F(surface.opacity),       Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT_OPACITY_F,        NULL, F(surface.opacity),       Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT_AMBIENT_X,        NULL, F(surface.ka),            Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT_AMBIENT_F,        NULL, F(surface.ka),            Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT_DIFFUSE_X,        NULL, F(surface.kd),            Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT_DIFFUSE_F,        NULL, F(surface.kd),            Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT_SPECULAR_X,       NULL, F(surface.ks),            Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT_SPECULAR_F,       NULL, F(surface.ks),            Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },
    {BRT_SPECULAR_POWER_X, NULL, F(surface.power),         Q | S | AX, BRTV_CONV_FIXED_SCALAR,     0, TM_PART                },
    {BRT_SPECULAR_POWER_F, NULL, F(surface.power),         Q | S | AF, BRTV_CONV_FLOAT_SCALAR,     0, TM_PART                },

    {BRT_LIGHTING_B,       NULL, F(surface.lighting),      Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT_FORCE_FRONT_B,    NULL, F(surface.force_front),   Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT_FORCE_BACK_B,     NULL, F(surface.force_back),    Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},
    {BRT_DEPTH_BIAS_X,     NULL, F(surface.depth_bias),    Q | S | A,  BRTV_CONV_FIXED_SCALAR,     0, TM_PART | TM_INVALID_CC},
    {BRT_DEPTH_BIAS_F,     NULL, F(surface.depth_bias),    Q | S | A,  BRTV_CONV_FLOAT_SCALAR,     0, TM_PART | TM_INVALID_CC},

    {BRT_COLOUR_SOURCE_T,  NULL, F(surface.colour_source), Q | S | A,  BRTV_CONV_COPY,             0, TM_PART | TM_INVALID_CC},

    {BRT_MAP_MATRIX_M23_X, NULL, F(surface.map_transform), Q | S | AX, BRTV_CONV_M23_FIXED_SCALAR, 0, TM_PART | TM_INVALID_CC},
    {BRT_MAP_MATRIX_M23_F, NULL, F(surface.map_transform), Q | S | AF, BRTV_CONV_M23_FLOAT_SCALAR, 0, TM_PART | TM_INVALID_CC},
};

static const state_surface default_state = {
    .colour  = BR_COLOUR_RGB(255, 255, 255),
    .opacity = BR_SCALAR(1.0),
    .ka      = BR_SCALAR(0.0),
    .kd      = BR_SCALAR(0.75),
    .ks      = BR_SCALAR(0.0),
    .power   = BR_SCALAR(20.0),

    .lighting      = BR_FALSE,
    .force_front   = BR_FALSE,
    .force_back    = BR_FALSE,
    .colour_source = BRT_SURFACE,

    .map_transform = {{
        BR_VECTOR2(1, 0),
        BR_VECTOR2(0, 1),
        BR_VECTOR2(0, 0),
    }},
};

void StateGLInitSurface(state_all *state)
{
    state->templates.surface = BrTVTemplateAllocate(state->res, template_entries, BR_ASIZE(template_entries));

    state->default_.surface = default_state;
    state->default_.valid |= MASK_STATE_SURFACE;
}
