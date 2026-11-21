/*
 * A tangent-space bump map for the lit fragment shaders.
 *
 * Fragments only: the frame comes from dFdx/dFdy of the mapping, which a vertex shader
 * does not have, so this is included by the lit fragment shaders and not by
 * render.lighting.glsl, which the vertex stage also includes for its GOURAUD path.
 *
 * glrend's bumpNormal() (drivers/glrend/brender.frag.glsl) is the same construction and
 * the same reasoning: br_vertex carries no tangent, so the frame is derived from the
 * mapping derivatives and follows the interpolated normal on a smoothly shaded triangle.
 */
#include "render.lighting.glsl"

layout(set = 0, binding = 4) uniform sampler2D bump_map;

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

    /*
     * Vulkan's clip space puts y the other way up from GL's, so dFdy runs backwards here and
     * the frame comes out rotated 180 degrees about the normal. Negating the two tangent
     * vectors restores the surface's own handedness: without it this driver samples the
     * parallax offset the other way and reads every map's green channel mirrored.
     */
    frame = mat3(-t * invmax, -b * invmax, n);
    return abs(determinant(frame)) > 0.0;
}

/*
 * The bump map's texel: .xyz is the tangent-space normal and .a its height. The
 * derivatives belong to the unoffset co-ordinate, so the sampler picks the level that
 * co-ordinate would have chosen rather than one blurred by the offset.
 */
vec4 bumpTexel(in vec2 uv, in vec2 ddx, in vec2 ddy)
{
    return textureGrad(bump_map, uv, ddx, ddy);
}

vec3 bumpNormal(in mat3 frame, in vec3 sampledNormal)
{
    vec3 sampled = sampledNormal * 2.0 - 1.0;

    /* A zero vector rounds to three RGB8 midpoints, not exact zero. */
    if(!(dot(sampled, sampled) > 4.0 / (255.0 * 255.0)))
        return frame[2];

    sampled.xy *= pc.material.bump_scale;
    vec3 perturbed = frame * sampled;
    float lengthSquared = dot(perturbed, perturbed);
    return lengthSquared > 0.0 && !isinf(lengthSquared) ? normalize(perturbed) : frame[2];
}

bool bumpTransformInvertible(mat2 transform)
{
    float det = abs(determinant(transform));
    float normSquared = dot(transform[0], transform[0]) + dot(transform[1], transform[1]);

    return det >= 1.17549435e-38 && !isinf(det) && det > 1e-4 * normSquared;
}

void applyBump(in vec3 p, in vec2 surfaceUV, inout vec3 n)
{
    if(pc.material.bump_enabled == 0u || gouraudShading() || pc.material.bump_scale == 0.0)
        return;

    vec4 bumpLinear = bumpMapLinear();
    if(!bumpTransformInvertible(mat2(pc.map_linear.xy, pc.map_linear.zw)) || !bumpTransformInvertible(mat2(bumpLinear.xy, bumpLinear.zw)))
        return;

    /* Take derivatives before the colour key's discard. */
    vec2 bumpUV  = applyMapTransform(applyBumpMapTransform(surfaceUV));
    vec2 bumpDDX = dFdx(bumpUV);
    vec2 bumpDDY = dFdy(bumpUV);
    mat3 frame;

    if(!cotangentFrame(p, bumpDDX, bumpDDY, normalize(n), frame))
        return;

    n = bumpNormal(frame, bumpTexel(bumpUV, bumpDDX, bumpDDY).xyz);
}
