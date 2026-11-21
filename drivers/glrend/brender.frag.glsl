#version 430 core

#include "brender.common.glsl"
#include "common.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec4 colour;


layout(location=4) in mat3 vertexLighting;

layout(location=7) in vec3 rawPosition;
layout(location=8) in vec3 rawNormal;

layout(location=9) in float viewDistance;

layout(location=0) out vec4 mainColour;

uniform sampler2D  main_texture;
uniform usampler2D index_texture;
uniform sampler2D  bump_texture;

/*
 * The surface's tangent frame, from the derivatives of the mapping. Without a
 * tangent vertex attribute this is where a tangent-space normal map gets its
 * basis. The frame follows the interpolated normal on a smoothly shaded triangle.
 */
bool cotangentFrame(in vec3 p, in vec2 duv1, in vec2 duv2, in vec3 n, out mat3 frame)
{
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);

    /* Perspective interpolation can leave roundoff in an otherwise rank-one UV gradient. */
    float uvLengthSquared = max(dot(duv1, duv1), dot(duv2, duv2));
    if(!(abs(determinant(mat2(duv1, duv2))) > 1e-4 * uvLengthSquared))
        return false;

    vec3 dp2perp = cross(dp2, n);
    vec3 dp1perp = cross(n, dp1);
    vec3 t       = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 b       = dp2perp * duv1.y + dp1perp * duv2.y;

    float maxLengthSquared = max(dot(t, t), dot(b, b));
    if(!(maxLengthSquared > 0.0))
        return false;

    float invmax = inversesqrt(maxLengthSquared);

    frame = mat3(t * invmax, b * invmax, n);
    return abs(determinant(frame)) > 0.0;
}

/*
 * The bump map's texel: .xyz is the tangent-space normal and .a its height. The
 * derivatives belong to the unoffset co-ordinate, so the sampler picks the level
 * that co-ordinate would have chosen rather than one blurred by the offset.
 */
vec4 bumpTexel(in vec2 uv, in vec2 ddx, in vec2 ddy)
{
    return textureGrad(bump_texture, uv, ddx, ddy);
}

vec3 bumpNormal(in mat3 frame, in vec3 sampledNormal)
{
    vec3 sampled = sampledNormal * 2.0 - 1.0;

    /* A zero vector rounds to three RGB8 midpoints, not exact zero. */
    if(!(dot(sampled, sampled) > 4.0 / (255.0 * 255.0)))
        return frame[2];

    sampled.xy *= bump_scale;
    vec3 perturbed = frame * sampled;
    float lengthSquared = dot(perturbed, perturbed);
    return lengthSquared > 0.0 && !isinf(lengthSquared) ? normalize(perturbed) : frame[2];
}

vec4 applyFog(in vec4 inColour, in float dist)
{
    float fog_min = fog_range.x; /* NB: To match softrend, set this to 0. */
    float fog_max = max(fog_range.y + 0.001f, fog_range.y);

    float fog_factor = clamp((fog_max - dist) / (fog_max - fog_min), 0.0, 1.0);
    return mix(fog_colour, inColour, fog_factor);
}

vec2 SurfaceMapEnvironment(in vec3 eye, in vec3 normal, in mat4 model_to_environment) {
    vec3 r;
    float cu, cv;

    /*
     * Generate reflected vector
     */
    r = reflect(eye, normalize(normal));

    /*
     * Rotate vector into the environment frame.
     * This should be the identity matrix if no environment is set.
     */
    vec3 wr2 = normalize(mat3(model_to_environment) * r);

    /*
     * Convert vector to environment coordinates.
     *
     * wr2 is the antipode of softrend's reflected vector
     * (drivers/softrend/mapping.c, SurfaceMapEnvironment), so it adds half a
     * turn to cu and mirrors cv: the +0.5 on cu cancels the half turn, but
     * nothing cancels the mirror, leaving V = 1 - V_softrend. A framebuffer
     * texture is stored bottom-up, so the sampler flips V back and V_softrend
     * comes out; a top-down host upload does not, so it needs V_softrend
     * directly.
     */
    cu = atan(wr2.x, -wr2.z) * 0.159154943091895; /* 1/(2*PI) */
    cv = -wr2.y * 0.5;

    return 0.5 + vec2(cu, environment_bottom_up ? cv : -cv);
}

/*
 * The surface's own co-ordinate, before the surface's map transform: the vertex's,
 * or the one the mapping source generates.
 */
vec2 SurfaceUV(in vec3 position, in vec3 normal, in vec2 uv)
{
    switch(uv_source) {
        case UV_SOURCE_MODEL: {
            /*
             * NB: We need this no-op branch explicitly first because
             * NX Homebrew (Mesa 20.1.0-rc3 + nouveau) is much faster
             * with it here.
             */
            break;
        }

        case UV_SOURCE_ENV_L: {
            /*
             * Generate U,V for environment assuming local eye.
             *
             * softrend/mapping.c - SurfaceMapEnvironmentLocal()
             */
            vec3 eye = normalize(eye_m.xyz - position);
            uv = SurfaceMapEnvironment(eye, normal, environment);
            break;
        }

        case UV_SOURCE_ENV_I: {
            /*
             * Generate U,V for environment assuming infinite eye.
             *
             * softrend/mapping.c - SurfaceMapEnvironmentInfinite()
             */
            vec3 eye = normalize(eye_m.xyz);
            uv = SurfaceMapEnvironment(eye, normal, environment);
            break;
        }

        case UV_SOURCE_GEOMETRY_X: {
            /*
             * Take U,V from vertex Y,Z
             *
             * softrend/mapping.c - SurfaceMapGeometryX()
             */
            uv = position.yz;
            break;
        }

        case UV_SOURCE_GEOMETRY_Y: {
            /*
             * Take U,V from vertex Z,X
             *
             * softrend/mapping.c - SurfaceMapGeometryY()
             */
            uv = position.zx;
            break;
        }

        case UV_SOURCE_GEOMETRY_Z: {
            /*
             * Take U,V from vertex X,Y
             *
             * softrend/mapping.c - SurfaceMapGeometryZ()
             */
            uv = position.xy;
            break;
        }
    }

    return uv;
}

/* The surface's map transform, from its own co-ordinate to the one its maps sample. */
vec2 applyMapTransform(in vec2 uv)
{
    return (mat3(map_transform) * vec3(uv, 1.0)).xy;
}

/* The bump map's modifier, applied before the surface's map transform. */
vec2 applyBumpMapTransform(in vec2 uv)
{
    return (mat3(bump_map_transform) * vec3(uv, 1.0)).xy;
}

bool bumpTransformInvertible(mat2 transform)
{
    float det = abs(determinant(transform));
    float normSquared = dot(transform[0], transform[0]) + dot(transform[1], transform[1]);

    return det >= 1.17549435e-38 && !isinf(det) && det > 1e-4 * normSquared;
}

void applyBump(in vec3 p, in vec2 surfaceUV, inout vec3 n)
{
    if(!bump_enabled || shading_mode != SHADING_MODE_PHONG || bump_scale == 0.0)
        return;

    if(!bumpTransformInvertible(mat2(map_transform)) || !bumpTransformInvertible(mat2(bump_map_transform)))
        return;

    /* Take derivatives before the colour key's discard. */
    vec2 bumpUV = applyMapTransform(applyBumpMapTransform(surfaceUV));
    vec2 bumpDDX = dFdx(bumpUV);
    vec2 bumpDDY = dFdy(bumpUV);
    mat3 frame;

    if(!cotangentFrame(p, bumpDDX, bumpDDY, normalize(n), frame))
        return;

    n = bumpNormal(frame, bumpTexel(bumpUV, bumpDDX, bumpDDY).xyz);
}

vec4 getTexColour(in vec2 uv)
{
    switch(texture_mode) {
        case TEXTURE_MODE_NORMAL:
            return texture(main_texture, uv);
        case TEXTURE_MODE_INDEX:
            return texturei(index_texture, main_texture, uv);
        case TEXTURE_MODE_INDEX_FILTER:
            return bilinearFilter(index_texture, main_texture, uv);
        default:
            return vec4(1, 0, 1, 1);
    }
}

void main()
{
    vec2 surfaceUV = SurfaceUV(rawPosition, rawNormal, uv);
    vec2 mappedUV  = applyMapTransform(surfaceUV);

    vec3 shadeNormal = normal;
    applyBump(position.xyz, surfaceUV, shadeNormal);

    vec4 texColour = getTexColour(mappedUV);

    /*
     * The colour key is tested on the raw source texel, before the palette or
     * shade lookup: palette index 0 for an indexed map, black for an RGB one.
     * See resources/aidocs/pixelmap-colour-keying.md.
     */
    bool keyed = (texture_mode == TEXTURE_MODE_NORMAL) ? (texColour.rgb == vec3(0.0, 0.0, 0.0))
                                                       : (texture(index_texture, mappedUV).r == 0u);

    if(!disable_colour_key && keyed)
        discard;

    vec3 lightA = vertexLighting[0];
    vec3 lightD = vertexLighting[1];
    vec3 lightS = vertexLighting[2];

    if(shading_mode == SHADING_MODE_PHONG) {
        accumulateLights(position.xyz, shadeNormal, lightA, lightD, lightS);
    }

    vec4 surfaceColour = colour_source == COLOUR_SOURCE_GEOMETRY ? colour : surface_colour;

    /*
     * If no lighting at all, fullbright baby.
     */
    lightD += (!prelighting && !lighting) ? 1.0 : 0.0;

#if DEBUG_DISABLE_LIGHTS
    lightA = vec3(0);
    lightD = vec3(1);
    lightS = vec3(0);
#endif

    vec4 fragColour = vec4(0);
    fragColour += prelighting ? vec4(colour.rgb, 1) : vec4(0);
    fragColour += ((vec4(lightA + lightD, 1.0)) * (surfaceColour * texColour)) + vec4(lightS, 0.0);

    if(enable_fog) {
        fragColour = applyFog(fragColour, viewDistance);
    }

    fragColour = clamp(fragColour, 0, 1);

    mainColour = fragColour;

    return;
}
