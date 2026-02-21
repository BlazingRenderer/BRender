#version 450 core

#extension GL_GOOGLE_include_directive : require

#include "render.lighting.glsl"

/*
 * Vertex shader for the br_renderer scene draw.
 *
 * The model-view matrix is pushed per draw and the projection comes from the scene block,
 * because the lighting model shades in view space and needs the model-view for both the
 * view position and the normal, not just the combined MVP. A br_matrix4 copied into a GLSL
 * mat4 is the transpose of the BRender matrix, so `model_view * vec4(pos, 1)` computes the
 * BRender row-vector `pos * model_view`, and `projection * model_view` computes the BRender
 * `model_to_view * view_to_screen`.
 *
 * A BRender normal transforms by the inverse transpose of the model-view linear part. The
 * model-view is already here, so the normal matrix is derived instead of pushed, keeping
 * the push constant block inside the 128 bytes Vulkan guarantees.
 *
 * Flat shading samples the normal at the provoking vertex, so the normal is emitted both
 * interpolated and flat; the lit fragment shader picks by the surface's shading mode.
 *
 * A GOURAUD surface is lit here, per vertex, exactly as glrend's vertex stage does
 * (drivers/glrend/brender.vert.glsl): the vertex normal is normalised and the lights are
 * accumulated into ambient/diffuse/specular, which are then interpolated to the fragment
 * stage. The PHONG (and out-of-scope FLAT) paths leave the light varyings zero and the lit
 * fragment shader accumulates per fragment instead, so the accumulation model stays in
 * render.lighting.glsl and the stage that runs it is what the shading mode selects.
 *
 * BRender's view_to_screen produces OpenGL clip space - y up, z in [-w, w] - but with
 * +1 (not -1) as the near plane's normalised z: here the camera's hither is negative in
 * view space (the view volume is along -z), so near maps to NDC z = +1 and far to -1.
 * Vulkan takes y down and z in [0, w], so flip y and remap z so that near becomes 0 and far
 * becomes w, the Vulkan convention the depth test's compare ops assume; mapping row 0 to
 * the top matches the rect draw and the readback.
 *
 * The model's map coordinate is transformed by the surface's map transform and passed on
 * as a varying. The transform is affine, so transforming here and interpolating is the
 * same as interpolating and transforming per fragment, and it leaves the fragment shaders
 * with a plain texture fetch. The BRender row-vector convention (softrend/mapping.c) is
 * `u' = m00*u + m10*v + m20`, `v' = m01*u + m11*v + m21`; map_linear holds m00, m01, m10,
 * m11 and map_translate holds m20, m21.
 *
 * The model's per-vertex colour is passed interpolated to the fragment stage. A surface
 * whose colour_source is the geometry draws with it instead of the surface colour, and a
 * pre-lit surface adds it to the shaded colour (see render.common.glsl).
 *
 * Clip planes ride in the scene block as view-space half-spaces. glrend enables
 * GL_CLIP_DISTANCE0 + i for each plane the core set to BRT_PLANE; Vulkan has no glEnable,
 * so the equivalent is to write gl_ClipDistance[i]. Vulkan enables one half-space per
 * element of the array the shader declares, so every element must be written: an enabled
 * plane gets its signed view-space distance and a disabled one a positive constant that
 * clips nothing.
 */

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aMap;
layout(location = 3) in vec4 aColour;

layout(location = 0) out vec3 vViewPosition;
layout(location = 1) out vec3 vNormal;
layout(location = 2) flat out vec3 vFlatNormal;
layout(location = 3) out vec2 vUV;
layout(location = 4) out vec4 vColour;
layout(location = 5) out vec3 vLightAmbient;
layout(location = 6) out vec3 vLightDiffuse;
layout(location = 7) out vec3 vLightSpecular;

out float gl_ClipDistance[MAX_CLIP_PLANES];

void main()
{
    vec4 view_position = pc.model_view * vec4(aPosition, 1.0);

    vViewPosition = view_position.xyz;

    vec3 normal = transpose(inverse(mat3(pc.model_view))) * aNormal;
    vNormal     = normal;
    vFlatNormal = normal;

    vColour = aColour;

    /*
     * The GOURAUD light terms, accumulated against the normalised vertex normal the way
     * glrend's vertex stage does. Any other shading mode leaves them zero, and the lit
     * fragment shader accumulates from the interpolated normal instead.
     */
    vLightAmbient  = vec3(0.0);
    vLightDiffuse  = vec3(0.0);
    vLightSpecular = vec3(0.0);

    if(gouraudShading())
        accumulateSceneLights(view_position.xyz, normalize(normal), vLightAmbient, vLightDiffuse, vLightSpecular);

    vUV = vec2(pc.map_linear.x * aMap.x + pc.map_linear.z * aMap.y + pc.map_translate.x,
               pc.map_linear.y * aMap.x + pc.map_linear.w * aMap.y + pc.map_translate.y);

    gl_Position = scene.projection * view_position;

    gl_Position.y = -gl_Position.y;
    /*
     * BRender's view_to_screen produces OpenGL clip space (z in [-w, w], near at +w);
     * Vulkan's clip volume is z in [0, w], so remap into it. The near/far *reversal* is the
     * viewport's minDepth/maxDepth (the glDepthRange() equivalent - see sceneBegin), so this
     * is a pure [-w, w] -> [0, w] map and does not reorder near and far.
     */
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;

    /*
     * glrend computes dot(position, clip_planes[i]) in the same view space for i below
     * num_clip_planes. The distance is kept for a positive value, so a disabled plane is
     * written as a positive constant rather than left undefined - Vulkan enables every
     * element of the array, not just the ones the core set.
     */
    for(int i = 0; i < MAX_CLIP_PLANES; ++i) {
        if(clipPlaneEnabled(i))
            gl_ClipDistance[i] = dot(view_position, scene.clip_planes[i]);
        else
            gl_ClipDistance[i] = 1.0;
    }
}
