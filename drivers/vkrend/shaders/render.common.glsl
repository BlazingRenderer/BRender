/*
 * Declarations shared by every stage of the br_renderer scene draw.
 *
 * The scene block holds the per-scene resources the draw needs: the projection matrix, the
 * environment frame the environment-mapped fragment shaders rotate a reflection into, and
 * the frame's packed lights. It is bound as a push descriptor (set 0, binding 0) and
 * written once per scene, which is why the lights can be a full BR_MAX_LIGHTS array - a
 * push constant block cannot exceed the 128 bytes Vulkan guarantees, and one light alone
 * is most of that. The per-draw material block (fog for now) is a second uniform buffer at
 * set 0, binding 3, for the same reason: the push constant block is exactly the 128 bytes
 * Vulkan guarantees, so anything else the fragment stage needs per draw has to live
 * outside it. glslang resolves the include relative to this file's directory.
 *
 * The push constant block holds the per-draw state and is laid out to fit the 128 bytes
 * exactly: the model-view (64) + surface colour (16) + lighting factors (16) + the map
 * transform's linear part (16) + its translation (8) + a packed flag word pair (8). The
 * model-view, not the combined MVP, is what the lighting model needs - it shades in view
 * space, so it derives the normal matrix from the model-view here rather than carrying a
 * second matrix in the block. The map transform is applied to the model's map coordinates
 * by the vertex stage, so the fragment stage only reads the transformed UV.
 *
 * The flags word pair packs seven things into 64 bits: bit 0 of flags.x is flat shading,
 * bit 1 is the colour key, and bits 2..31 of flags.x plus bits 0..17 of flags.y are the
 * per-model culled-light mask - exactly the BR_MAX_LIGHTS bits it needs, one per light
 * part in the packed order. The remaining bits of flags.y carry the surface's colour source
 * (bit 18), its pre-lighting state (bit 19), whether the shading mode is GOURAUD (bit 20)
 * and the surface's mapping source (bits 21..22).
 */

#define MAX_LIGHTS 48 /* Must match BR_MAX_LIGHTS. */

/* Must match BR_MAX_CLIP_PLANES; the vertex stage redeclares gl_ClipDistance with it. */
#define MAX_CLIP_PLANES 6

/* Must match BR_VKREND_MAX_LIGHT_VOLUME_REGIONS/_PLANES in renderer.h. */
#define MAX_LIGHT_VOLUME_REGIONS 16
#define MAX_LIGHT_VOLUME_PLANES  64

/*
 * One packed light, mirroring br_vk_light_state in renderer.c. The field set is the one
 * glrend's shader consumes (drivers/glrend/brender.common.glsl), so the two renderers
 * shade the same.
 */
struct br_light {
    vec4  position;    /* (X, Y, Z, w); w is 0 for a directional light, 1 otherwise.        */
    vec4  direction;   /* (X, Y, Z, 0), view space; scaled by intensity when directional.     */
    vec4  half_vector; /* (X, Y, Z, 0), the directional light's halfway vector.                */
    vec4  colour;      /* (R, G, B, 0).                                                        */
    vec4  iclq;        /* (intensity, constant, linear, quadratic) attenuation.                */
    vec4  radii;       /* (cos(inner), cos(outer), radius_inner, radius_outer).                */
    uvec4 info;        /* (type, attenuation_type, 0, 0).                                      */
    vec4  volume;      /* (region_offset, region_count, falloff_distance, 0).                  */
};

layout(std140, set = 0, binding = 0) uniform SceneData {
    mat4 projection;     /* view_to_screen, transposed.                          */
    mat4 environment;    /* view_to_environment, transposed; identity when unset. */
    vec4 eye_view;       /* (0, 0, 1, 0): the view-space eye direction.          */
    vec4 ambient_colour; /* Sum of the non-radial ambient lights, clamped.       */
    uvec4 counts;        /* [0] num_lights, [1] use_ambient_colour.              */
    vec4  clip_planes[MAX_CLIP_PLANES]; /* View-space half-spaces, one per plane.  */
    uvec4 clip_mask;     /* [0] bit i => clip plane i was enabled in the state.  */
    br_light lights[MAX_LIGHTS];
    vec4 light_volume_regions[MAX_LIGHT_VOLUME_REGIONS]; /* (plane_offset, plane_count, 0, 0). */
    vec4 light_volume_planes[MAX_LIGHT_VOLUME_PLANES];   /* View-space half-spaces.            */
} scene;

/*
 * One draw's material state, mirroring br_vk_material_state in renderer.h. The push
 * constant block is full, so this rides in its own uniform buffer bound as a push
 * descriptor at set 0, binding 3. The reserved word keeps the block 16-byte aligned so
 * more material state can join the fog fields without moving them.
 */
layout(std140, set = 0, binding = 3) uniform MaterialData {
    vec4 fog_colour;  /* (R, G, B, 1): glrend's fog_colour with alpha 1.        */
    vec2 fog_range;   /* (fog_min, fog_max), in view-space units.               */
    uint fog_enabled; /* 1 when the primitive's fog_type is not BRT_NONE.       */
    uint reserved;    /* 16-byte padding for material state to join later.      */
} material;

/*
 * Whether the core enabled state clip plane i. glrend keys the same decision off
 * state_clip::type == BRT_PLANE in StateGLUpdateScene(); the mask is that test, carried to
 * the shader because Vulkan has no glEnable and an unwritten gl_ClipDistance element would
 * clip against an undefined half-space.
 */
bool clipPlaneEnabled(int i)
{
    return (scene.clip_mask.x & (1u << i)) != 0u;
}

layout(push_constant) uniform RenderData {
    mat4  model_view;
    vec4  colour;         /* (R, G, B, opacity) of the surface.                  */
    vec4  factors;        /* ka, kd, ks, power.                                  */
    vec4  map_linear;     /* m00, m01, m10, m11.                                 */
    vec2  map_translate;  /* m20, m21.                                           */
    uvec2 flags;          /* [0] bit 0 flat, bit 1 colour key, bits 2..31 cull 0..29;
                             [1] bits 0..17 cull 30..47, bit 18 vertex colour source,
                             bit 19 prelighting, bit 20 GOURAUD, bits 21..22 mapping
                             source.                                                */
} pc;

/* Whether the surface is flat shaded, i.e. the normal is taken at the provoking vertex. */
bool flatShading()
{
    return (pc.flags.x & 1u) != 0u;
}

/*
 * Whether the surface shades per vertex (GOURAUD) rather than per fragment (PHONG, and the
 * out-of-scope FLAT). The vertex stage accumulates the lights when this is set and the lit
 * fragment shaders shade with the interpolated result; otherwise the fragment stage
 * accumulates. This is the shader-side split of the shading mode the pipeline key carries.
 */
bool gouraudShading()
{
    return (pc.flags.y & (1u << 20)) != 0u;
}

/* Whether the colour key is enabled, i.e. a black texel discards the fragment. */
bool colourKeyEnabled()
{
    return (pc.flags.x & 2u) != 0u;
}

/*
 * Whether the primitive's fog is on. The core publishes BRT_FOG_T as BRT_NONE for a
 * material without BR_MATF_FOG_LOCAL and BRT_LINEAR otherwise (core/v1db/prepmatl.c);
 * glrend only distinguishes NONE from not-NONE, so neither does vkrend.
 */
bool fogEnabled()
{
    return material.fog_enabled != 0u;
}

/*
 * Fog, matching glrend's applyFog() exactly (drivers/glrend/brender.frag.glsl): a linear
 * fade toward fog_colour between fog_range.x and fog_range.y, with glrend's 0.001 guard on
 * the far distance and the same clamp-then-mix order. dist is the fragment's view-space
 * distance, -vViewPosition.z, the same quantity glrend's vertex stage passes as
 * viewDistance.
 *
 * NB: this deliberately follows glrend, not softprim/pentprim. softprim's fog is an
 * indexed-output-only blend - pentprim has no fogged RGB block at all
 * (drivers/softprim/softprim_axes.h) - and it ignores fog_min, effectively starting the
 * fade at 0. glrend accounts for fog_min and vkrend follows glrend; do not "fix" this
 * toward softprim. The primitive's fog_scale is carried in state but glrend declares and
 * sets it without ever applying it, so vkrend ignores it too.
 */
vec4 applyFog(in vec4 inColour, in float dist)
{
    float fog_min = material.fog_range.x;
    float fog_max = max(material.fog_range.y + 0.001f, material.fog_range.y);

    float fog_factor = clamp((fog_max - dist) / (fog_max - fog_min), 0.0, 1.0);
    return mix(material.fog_colour, inColour, fog_factor);
}

/*
 * Whether the surface colour is the interpolated vertex colour rather than pc.colour.
 * The core sets BRT_COLOUR_SOURCE_T to BRT_GEOMETRY (or BRT_VERTEX) for a material whose
 * colours are per-vertex - a pre-lit one - and to BRT_SURFACE otherwise, the same split
 * glrend's apply_stored_properties() reads out of colour_source.
 */
bool vertexColourSource()
{
    return (pc.flags.y & (1u << 18)) != 0u;
}

/*
 * Whether the vertex colour is a pre-computed lighting result to add to the shaded
 * colour, glrend's `prelighting` term (BR_MATF_PRELIT together with BR_MATF_LIGHT).
 */
bool prelightingEnabled()
{
    return (pc.flags.y & (1u << 19)) != 0u;
}

/*
 * The core's per-model cull, published as BRT_CULLED_B on the light part and carried in
 * the packed flag pair. A culled light is dropped before the light is evaluated, matching
 * glrend's StateGLBuildLightLists().
 */
bool lightCulled(uint i)
{
    const uint bit = i + 2u;

    if(bit < 32u)
        return (pc.flags.x & (1u << bit)) != 0u;

    return (pc.flags.y & (1u << (bit - 32u))) != 0u;
}

/*
 * Where the surface's U,V come from, as the core publishes it on BRT_MAPPING_SOURCE_T:
 * BR_MATF_ENVIRONMENT_L and BR_MATF_ENVIRONMENT_I make it BRT_ENVIRONMENT_LOCAL and
 * BRT_ENVIRONMENT_INFINITE and everything else leaves it at BRT_GEOMETRY_MAP
 * (core/v1db/prepmatl.c). The two environment modes ride in the free flag bits above the
 * shading bit; the plain map-transform path needs no bit because it is the default, so
 * zero is "none".
 */
#define MAPPING_SOURCE_NONE                 0u
#define MAPPING_SOURCE_ENVIRONMENT_LOCAL    1u
#define MAPPING_SOURCE_ENVIRONMENT_INFINITE 2u

uint mappingSource()
{
    return (pc.flags.y >> 21) & 3u;
}

/*
 * Sphere-map U,V for an environment-mapped surface, from softrend's SurfaceMapEnvironment()
 * (drivers/softrend/mapping.c). softrend builds the reflected vector r = 2N(N.E) - E from
 * the fragment-to-eye vector E, which is GLSL's reflect(-E, N), rotates it into the
 * environment frame by model_to_environment and converts it to turns:
 *
 *     cu = atan2(r.x, -r.z) / (2*PI)      cv = 0.5 - r.y / 2
 *
 * and returns (cu, cv). BrAngleToScalar() is the identity, so there is no 0.5 on cu; the
 * +0.5 belongs only to cv. vkrend shades in view space, where the eye is the view-space
 * origin, so the same reflection is view_to_environment * reflect(incident, N) for a
 * model-to-view that is length preserving. The local eye points from the fragment and the
 * infinite eye from the model origin - the model-space eye the core caches
 * (drivers/softrend/setup.c eyeInModel()) maps to that view-space direction. The incident
 * is therefore -E, the negation of the outgoing eye direction, which is what makes
 * reflect() produce softrend's r rather than its antipode.
 *
 * NB: glrend's SurfaceMapEnvironment() (drivers/glrend/brender.frag.glsl) passes reflect()
 * the eye direction E itself, not -E, and so turns the ray through the origin; its
 * `0.5 + vec2(cu, cv)` compensates the pi that adds to cu (the antipode shifts the angle
 * by pi, i.e. 0.5 turn) and leaves V mirrored. vkrend reflects -E, like softrend, so it
 * must NOT carry that +0.5: doing so lands the sample half a turn around the sphere map.
 * The vkrend witness's environment step (tools/rendertest/vkwitness.c) reads the texel the
 * fragment stage fetched, with the anchor rotated 60 degrees, so a half turn lands it in a
 * different one.
 *
 * The result then goes through the surface's map transform, which is what the vertex stage
 * applied to the plain map coordinate.
 */
vec2 surfaceMapEnvironment(vec3 view_position, vec3 normal, uint source)
{
    vec3 incident = (source == MAPPING_SOURCE_ENVIRONMENT_LOCAL)
                        ? normalize(view_position)
                        : normalize((pc.model_view * vec4(0.0, 0.0, 0.0, 1.0)).xyz);

    vec3 r = reflect(incident, normalize(normal));

    vec3 wr = normalize(mat3(scene.environment) * r);

    vec2 uv = vec2(atan(wr.x, -wr.z) * 0.159154943091895, 0.5 - wr.y * 0.5);

    return vec2(pc.map_linear.x * uv.x + pc.map_linear.z * uv.y + pc.map_translate.x,
                pc.map_linear.y * uv.x + pc.map_linear.w * uv.y + pc.map_translate.y);
}
