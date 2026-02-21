/*
 * BRender's ambient/diffuse/specular lighting model, the same one glrend evaluates
 * (drivers/glrend/brender.common.glsl) and softrend implements per pixel
 * (drivers/softrend/light24.c). The lit fragment shaders and the vertex shader (for the
 * GOURAUD path) include this, so every stage that shades shares one model.
 *
 * It shades in view space, where the core publishes the light geometry (BrSetupLights() in
 * core/v1db/enables.c), so the interpolated normal and view position the vertex shader
 * produced are used as they are.
 *
 * accumulateSceneLights() returns the separate ambient, diffuse and specular terms: the
 * caller composes them differently for a textured surface (where the texel modulates the
 * ambient and diffuse terms and, in glrend, the specular is added unmodulated too).
 *
 * A light's cutoff volume is applied by lightVolumeAttenuation() below, matching glrend's
 * of the same name (drivers/glrend/brender.common.glsl). Fog is not part of this stage; it
 * is applied by the fragment shaders after the terms below are composed
 * (render.common.glsl, applyFog()).
 *
 * This file includes render.common.glsl, so a shader that lights includes this one and not
 * render.common.glsl directly. glslang resolves the nested include relative to this file.
 */

#extension GL_GOOGLE_include_directive : require
#include "render.common.glsl"

#define LIGHT_AMBIENT 0u
#define LIGHT_DIRECT  1u
#define LIGHT_POINT   2u
#define LIGHT_SPOT    3u

#define ATTENUATION_QUADRATIC 0u
#define ATTENUATION_RADII     1u

float calculateAttenuation(uint i, float dist)
{
    const float attenuation_c = scene.lights[i].iclq.y;
    const float attenuation_l = scene.lights[i].iclq.z;
    const float attenuation_q = scene.lights[i].iclq.w;

    return 1.0 / (attenuation_c + (attenuation_l * dist) + (attenuation_q * dist * dist));
}

float calculateAttenuationRadii(uint i, float dist, float intensity)
{
    const float radius_inner = scene.lights[i].radii.z;
    const float radius_outer = scene.lights[i].radii.w;

    /* radius_outer != radius_inner is enforced CPU-side. */
    float t = clamp((dist - radius_inner) / (radius_outer - radius_inner), 0.0, 1.0);
    return intensity * (1.0 - t);
}

/*
 * Radial ambient lights, from softrend/light24.c lightingColourAmbientRadii().
 */
void lightingColourAmbientRadii(in vec3 p, in uint i, inout vec3 outA)
{
    const float intensity    = scene.lights[i].iclq.x;
    const float radius_outer = scene.lights[i].radii.w;
    const vec3  position     = scene.lights[i].position.xyz;

    vec3  dirn = position - p;
    float dist = length(dirn);

    if(dist >= radius_outer)
        return;

    float atten = calculateAttenuationRadii(i, dist, intensity);

    outA += pc.factors.x * intensity * scene.lights[i].colour.xyz * atten;
}

void lightingColourDirect(in vec3 n, in uint i, inout vec3 outD, inout vec3 outS)
{
    const float intensity = scene.lights[i].iclq.x;
    const vec3  colour    = scene.lights[i].colour.xyz;
    const vec3  direction = scene.lights[i].direction.xyz;

    float diffDot = max(dot(n, direction), 0.0);
    outD += diffDot * pc.factors.y * colour; /* Intensity is scaled into the direction CPU-side. */

    if(pc.factors.z > 0.0) {
        float specDot = max(dot(n, scene.lights[i].half_vector.xyz), 0.0);
        if(specDot > 0.0)
            outS += pc.factors.z * intensity * colour * pow(specDot, pc.factors.w);
    }
}

void lightingColourPoint(in vec3 p, in vec3 n, in uint i, inout vec3 outD, inout vec3 outS)
{
    const uint  attenuation_type = scene.lights[i].info.y;
    const float intensity        = scene.lights[i].iclq.x;
    const vec3  colour           = scene.lights[i].colour.xyz;
    const float radius_outer     = scene.lights[i].radii.w;
    const vec3  position         = scene.lights[i].position.xyz;

    vec3  dirn     = position - p;
    float dist_sqr = dot(dirn, dirn);

    if(attenuation_type == ATTENUATION_RADII && dist_sqr >= radius_outer * radius_outer)
        return;

    float dist  = sqrt(dist_sqr);
    float atten = 0.0;

    if(attenuation_type == ATTENUATION_RADII)
        atten = calculateAttenuationRadii(i, dist, intensity);
    else
        atten = calculateAttenuation(i, dist);

    if(atten <= 0.0)
        return;

    vec3 dirn_norm = dirn / dist;

    float diffDot = max(dot(n, dirn_norm), 0.0);
    outD += diffDot * pc.factors.y * colour * atten;

    if(pc.factors.z > 0.0) {
        float specDot = max(dot(n, normalize(scene.eye_view.xyz + dirn_norm)), 0.0);
        if(specDot > 0.0)
            outS += pc.factors.z * colour * pow(specDot, pc.factors.w) * atten;
    }
}

void lightingColourSpot(in vec3 p, in vec3 n, in uint i, inout vec3 outD, inout vec3 outS)
{
    const float spot_inner_cos = scene.lights[i].radii.x;
    const float spot_outer_cos = scene.lights[i].radii.y;
    const vec3  position       = scene.lights[i].position.xyz;
    const vec3  direction      = scene.lights[i].direction.xyz;

    vec3 dirn_norm = normalize(position - p);

    float spotDot = dot(dirn_norm, direction);
    if(spotDot <= spot_outer_cos)
        return;

    float cutoff          = 1.0;
    float innerOuterDiff  = spot_inner_cos - spot_outer_cos;

    if(innerOuterDiff != 0.0)
        cutoff = clamp((spotDot - spot_outer_cos) / innerOuterDiff, 0.0, 1.0);

    /* A spot light is a point light with a cutoff. */
    vec3 d = vec3(0.0);
    vec3 s = vec3(0.0);
    lightingColourPoint(p, n, i, d, s);

    outD += d * cutoff;
    outS += s * cutoff;
}

/*
 * A light's cutoff volume: a point inside any region is lit in full, and outside a region's
 * boundary the contribution fades linearly to nothing over falloff_distance. Mirrors
 * glrend's lightVolumeAttenuation() (drivers/glrend/brender.common.glsl), including the hard
 * cutoff a zero falloff distance produces.
 *
 * Returns 1.0 when the light has no volume.
 */
float lightVolumeAttenuation(in vec3 p, in uint i)
{
    const float region_offset = scene.lights[i].volume.x;
    const float region_count  = scene.lights[i].volume.y;
    const float falloff       = scene.lights[i].volume.z;

    if(region_count < 0.5)
        return 1.0;

    float atten_dist = 3.40282347e+38; /* BR_SCALAR_MAX */

    for(uint r = 0u; r < uint(region_count); ++r) {
        const vec4 region = scene.light_volume_regions[uint(region_offset) + r];

        const uint plane_offset = uint(region.x);
        const uint plane_count  = uint(region.y);

        bool  inside   = true;
        bool  outside  = false;
        float max_dist = 0.0;

        for(uint j = 0u; j < plane_count; ++j) {
            const vec4  plane = scene.light_volume_planes[plane_offset + j];
            const float dist  = -(dot(p, plane.xyz) - plane.w);

            if(dist >= 0.0) {
                inside = false;

                if(dist >= falloff)
                    outside = true;
                else if(dist > max_dist)
                    max_dist = dist;
            }
        }

        if(inside)
            return 1.0;

        if(!outside && max_dist < atten_dist)
            atten_dist = max_dist;
    }

    if(atten_dist >= falloff)
        return 0.0;

    return 1.0 - atten_dist / falloff;
}

/*
 * Accumulate every light the scene packed, in the order sceneBegin() packed them so the
 * cull flag's light index still matches. With no non-radial ambient light the ambient
 * constant applies flat; with one it modulates the accumulated colour. See
 * softrend/light24.c SurfaceColourLit().
 *
 * Each light's contribution is scaled by its cutoff-volume attenuation, and a light fully
 * outside its volume is skipped, as glrend's accumulateLights() does.
 */
void accumulateSceneLights(in vec3 p, in vec3 n, inout vec3 ambient, inout vec3 diffuse, inout vec3 specular)
{
    if(scene.counts.y != 0u)
        diffuse += pc.factors.x * scene.ambient_colour.xyz;
    else
        diffuse += pc.factors.x;

    for(uint i = 0u; i < scene.counts.x; ++i) {
        if(lightCulled(i))
            continue;

        const float volume_atten = lightVolumeAttenuation(p, i);

        if(volume_atten <= 0.0)
            continue;

        vec3 a = vec3(0.0);
        vec3 d = vec3(0.0);
        vec3 s = vec3(0.0);

        switch(scene.lights[i].info.x) {
            case LIGHT_AMBIENT:
                lightingColourAmbientRadii(p, i, a);
                break;

            case LIGHT_DIRECT:
                lightingColourDirect(n, i, d, s);
                break;

            case LIGHT_POINT:
                lightingColourPoint(p, n, i, d, s);
                break;

            case LIGHT_SPOT:
                lightingColourSpot(p, n, i, d, s);
                break;

            default:
                break;
        }

        ambient  += a * volume_atten;
        diffuse  += d * volume_atten;
        specular += s * volume_atten;
    }
}
