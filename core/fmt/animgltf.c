/*
 * animgltf.c - glTF animation loading and playback for BRender
 *
 * Parses glTF animation data (keyframes, channels, samplers) and
 * provides per-frame evaluation that writes TRS-composed matrices
 * to BRender actor transforms.
 *
 * Designed to work alongside loadgltf.c: call BrFmtGLTFActorLoadMany()
 * first for scene geometry, then BrFmtGLTFAnimLoad() with the returned
 * actors to add animation support.
 *
 * Supports STEP and LINEAR interpolation with quaternion SLERP for
 * rotations. Animations named *_oneshot clamp at their end; others loop.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <brender.h>

#include "cgltf.h"

/* ------------------------------------------------------------------ */
/* cgltf allocator callbacks (same pattern as loadgltf.c)              */
/* ------------------------------------------------------------------ */

static void *cgltf_alloc_anim(void *user, cgltf_size size)
{
    return BrResAllocate(user, size, BR_MEMORY_SCRATCH);
}

static void cgltf_free_anim(void *user, void *ptr)
{
    if(ptr == NULL)
        return;

    (void)user;
    BrResFree(ptr);
}

static cgltf_result cgltf_load_brfile_anim(const cgltf_memory_options *memory_options,
                                           const cgltf_file_options *file_options,
                                           const char *path, cgltf_size *size, void **data)
{
    (void)file_options;

    if((*data = BrFileLoad(memory_options->user_data, path, size)) == NULL)
        return cgltf_result_io_error;

    return cgltf_result_success;
}

static void cgltf_release_brfile_anim(const cgltf_memory_options *memory_options,
                                      const cgltf_file_options *file_options, void *data)
{
    (void)memory_options;
    (void)file_options;

    if(data != NULL)
        BrResFree(data);
}

/* ------------------------------------------------------------------ */
/* Math helpers                                                        */
/* ------------------------------------------------------------------ */

/*
 * Compose T(3) + R(4 quat xyzw) + S(3) into a BRender 3x4 matrix.
 * glTF spec 3.5.3: M = T * R * S
 */
static void compose_trs_to_br34(const float *t, const float *q, const float *s, br_matrix34 *out)
{
    float qx = q[0], qy = q[1], qz = q[2], qw = q[3];
    float xx = qx * qx, yy = qy * qy, zz = qz * qz;
    float xy = qx * qy, xz = qx * qz, yz = qy * qz;
    float wx = qw * qx, wy = qw * qy, wz = qw * qz;

    out->m[0][0] = BR_SCALAR((1.0f - 2.0f * (yy + zz)) * s[0]);
    out->m[0][1] = BR_SCALAR((2.0f * (xy + wz)) * s[0]);
    out->m[0][2] = BR_SCALAR((2.0f * (xz - wy)) * s[0]);

    out->m[1][0] = BR_SCALAR((2.0f * (xy - wz)) * s[1]);
    out->m[1][1] = BR_SCALAR((1.0f - 2.0f * (xx + zz)) * s[1]);
    out->m[1][2] = BR_SCALAR((2.0f * (yz + wx)) * s[1]);

    out->m[2][0] = BR_SCALAR((2.0f * (xz + wy)) * s[2]);
    out->m[2][1] = BR_SCALAR((2.0f * (yz - wx)) * s[2]);
    out->m[2][2] = BR_SCALAR((1.0f - 2.0f * (xx + yy)) * s[2]);

    out->m[3][0] = BR_SCALAR(t[0]);
    out->m[3][1] = BR_SCALAR(t[1]);
    out->m[3][2] = BR_SCALAR(t[2]);
}

/*
 * Quaternion spherical linear interpolation (shortest path).
 */
static void quat_slerp(float *out, const float *a, const float *b, float t)
{
    float dot = a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
    float b2[4];
    int   i;

    if(dot < 0.0f) {
        dot = -dot;
        for(i = 0; i < 4; i++)
            b2[i] = -b[i];
    } else {
        for(i = 0; i < 4; i++)
            b2[i] = b[i];
    }

    if(dot > 0.9995f) {
        float len;
        for(i = 0; i < 4; i++)
            out[i] = a[i] + t * (b2[i] - a[i]);
        len = (float)sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2] + out[3] * out[3]);
        if(len > 1e-6f)
            for(i = 0; i < 4; i++)
                out[i] /= len;
    } else {
        float theta     = (float)acos(dot);
        float sin_theta = (float)sin(theta);
        float wa        = (float)sin((1.0f - t) * theta) / sin_theta;
        float wb        = (float)sin(t * theta) / sin_theta;
        for(i = 0; i < 4; i++)
            out[i] = wa * a[i] + wb * b2[i];
    }
}

/*
 * Evaluate a single animation channel at the given time.
 */
static void evaluate_channel(br_gltf_channel *ch, float time, float *out)
{
    br_gltf_keyframes *keys = &ch->keys;
    int                components = (ch->path == 1) ? 4 : 3;
    int                lo, hi, mid;

    if(keys->count == 0)
        return;

    if(keys->count == 1 || time <= keys->times[0]) {
        BrMemCpy(out, keys->values, components * sizeof(float));
        return;
    }

    if(time >= keys->times[keys->count - 1]) {
        BrMemCpy(out, &keys->values[(keys->count - 1) * components], components * sizeof(float));
        return;
    }

    lo = 0;
    hi = keys->count - 1;
    while(lo + 1 < hi) {
        mid = (lo + hi) / 2;
        if(keys->times[mid] <= time)
            lo = mid;
        else
            hi = mid;
    }

    if(ch->interpolation == 0) {
        BrMemCpy(out, &keys->values[lo * components], components * sizeof(float));
    } else {
        float t_range  = keys->times[hi] - keys->times[lo];
        float t_factor = (t_range > 0.0f) ? (time - keys->times[lo]) / t_range : 0.0f;

        if(ch->path == 1) {
            quat_slerp(out, &keys->values[lo * 4], &keys->values[hi * 4], t_factor);
        } else {
            float *va = &keys->values[lo * 3];
            float *vb = &keys->values[hi * 3];
            int    i;
            for(i = 0; i < 3; i++)
                out[i] = va[i] + t_factor * (vb[i] - va[i]);
        }
    }
}

/* ------------------------------------------------------------------ */
/* DFS actor-to-node binding                                           */
/* ------------------------------------------------------------------ */

/*
 * Walk the cgltf node tree and the br_actor tree in parallel,
 * establishing the node_actors[] and rest_trs[] mappings.
 *
 * This works because loadgltf.c builds actors in identical tree
 * order to cgltf: it adds children in reverse to compensate for
 * BrActorAdd's prepend behavior, so actor children end up in the
 * same order as cgltf children.
 */
static void bind_actors_dfs(const cgltf_node *node, const cgltf_data *data,
                            br_actor *actor, br_gltf_anim_data *anim)
{
    br_int_32 node_idx = (br_int_32)(node - data->nodes);
    float    *trs;
    br_actor *child_actor;
    cgltf_size ci;

    if(node_idx < 0 || node_idx >= anim->nnodes)
        return;

    anim->node_actors[node_idx] = actor;
    fprintf(stderr, "ANIM:   bind node %d '%s' -> actor '%s'\n",
            node_idx, node->name ? node->name : "(null)",
            actor->identifier ? actor->identifier : "(null)");

    /*
     * Store rest-pose TRS. cgltf initialises defaults (T=0, R=identity, S=1)
     * even when the node uses a matrix form.
     */
    trs = &anim->rest_trs[node_idx * 10];
    BrMemCpy(trs,     node->translation, 3 * sizeof(float));
    BrMemCpy(trs + 3, node->rotation,    4 * sizeof(float));
    BrMemCpy(trs + 7, node->scale,       3 * sizeof(float));

    /*
     * Recurse: match cgltf children to actor children in order.
     */
    child_actor = actor->children;
    for(ci = 0; ci < node->children_count && child_actor != NULL; ++ci) {
        bind_actors_dfs(node->children[ci], data, child_actor, anim);
        child_actor = child_actor->next;
    }
}

/* ------------------------------------------------------------------ */
/* Animation parsing                                                   */
/* ------------------------------------------------------------------ */

static void parse_one_animation(const cgltf_animation *src, const cgltf_data *data,
                                br_gltf_animation *dst, void *res)
{
    float      max_time = 0.0f;
    const char *name    = src->name ? src->name : "";
    size_t      name_len;
    cgltf_size  ci;

    name_len = BrStrLen(name);
    if(name_len >= sizeof(dst->identifier))
        name_len = sizeof(dst->identifier) - 1;
    BrMemCpy(dst->identifier, name, name_len);
    dst->identifier[name_len] = '\0';

    dst->loop = BR_TRUE;
    if(name_len >= 7 && BrStrCmp(name + name_len - 7, "oneshot") == 0)
        dst->loop = BR_FALSE;

    dst->nchannels = (br_int_32)src->channels_count;
    dst->channels  = BrResAllocate(res, src->channels_count * sizeof(br_gltf_channel), BR_MEMORY_APPLICATION);
    BrMemSet(dst->channels, 0, src->channels_count * sizeof(br_gltf_channel));

    for(ci = 0; ci < src->channels_count; ++ci) {
        const cgltf_animation_channel *ch   = &src->channels[ci];
        const cgltf_animation_sampler *samp = ch->sampler;
        br_gltf_channel               *gc   = &dst->channels[ci];
        int                            components;
        br_int_32                      ki;

        if(!ch->target_node) {
            gc->path = -1;
            continue;
        }

        gc->node_index = (br_int_32)(ch->target_node - data->nodes);

        switch(ch->target_path) {
            case cgltf_animation_path_type_translation:
                gc->path = 0;
                break;
            case cgltf_animation_path_type_rotation:
                gc->path = 1;
                break;
            case cgltf_animation_path_type_scale:
                gc->path = 2;
                break;
            default:
                gc->path = -1;
                continue;
        }

        switch(samp->interpolation) {
            case cgltf_interpolation_type_step:
                gc->interpolation = 0;
                break;
            case cgltf_interpolation_type_linear:
                gc->interpolation = 1;
                break;
            default:
                gc->interpolation = 0;
                break;
        }

        gc->keys.count = (br_int_32)samp->input->count;
        gc->keys.times = BrResAllocate(res, sizeof(float) * gc->keys.count, BR_MEMORY_APPLICATION);

        components      = (gc->path == 1) ? 4 : 3;
        gc->keys.values = BrResAllocate(res, sizeof(float) * gc->keys.count * components, BR_MEMORY_APPLICATION);

        for(ki = 0; ki < gc->keys.count; ++ki) {
            cgltf_accessor_read_float(samp->input, ki, &gc->keys.times[ki], 1);
            if(gc->keys.times[ki] > max_time)
                max_time = gc->keys.times[ki];
        }

        for(ki = 0; ki < gc->keys.count; ++ki) {
            cgltf_accessor_read_float(samp->output, ki, &gc->keys.values[ki * components], components);
        }
    }

    dst->duration = max_time;
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

br_gltf_anim_data *BR_PUBLIC_ENTRY BrFmtGLTFAnimLoad(const char *name, br_actor **actors, br_uint_32 nactors)
{
    void              *scratch;
    cgltf_data        *data;
    br_gltf_anim_data *anim;
    cgltf_size         i;

    cgltf_options opts = {
        .type   = cgltf_file_type_invalid,
        .memory = {
            .alloc_func = cgltf_alloc_anim,
            .free_func  = cgltf_free_anim,
        },
        .file = {
            .read    = cgltf_load_brfile_anim,
            .release = cgltf_release_brfile_anim,
        },
    };

    /*
     * Scratch resource: parent for all cgltf allocations (freed at the end).
     */
    scratch = BrResAllocate(NULL, sizeof(int), BR_MEMORY_SCRATCH);
    opts.memory.user_data = scratch;

    if(cgltf_parse_file(&opts, name, &data) != cgltf_result_success) {
        BrResFree(scratch);
        return NULL;
    }

    if(cgltf_load_buffers(&opts, data, name) != cgltf_result_success) {
        BrResFree(scratch);
        return NULL;
    }

    if(data->animations_count == 0) {
        BrResFree(scratch);
        return NULL;
    }

    /*
     * Allocate the result structure. All sub-allocations are children of
     * this resource, so BrFmtGLTFAnimFree() is a single BrResFree().
     */
    anim = BrResAllocate(NULL, sizeof(br_gltf_anim_data), BR_MEMORY_APPLICATION);
    BrMemSet(anim, 0, sizeof(br_gltf_anim_data));

    anim->nnodes      = (br_int_32)data->nodes_count;
    anim->node_actors = BrResAllocate(anim, data->nodes_count * sizeof(br_actor *), BR_MEMORY_APPLICATION);
    anim->rest_trs    = BrResAllocate(anim, data->nodes_count * 10 * sizeof(float), BR_MEMORY_APPLICATION);
    BrMemSet(anim->node_actors, 0, data->nodes_count * sizeof(br_actor *));

    fprintf(stderr, "ANIM: nnodes=%d, nactors=%u, scene_roots=%d\n",
            anim->nnodes, (unsigned)nactors,
            data->scene ? (int)data->scene->nodes_count : -1);

    /*
     * DFS-walk the scene tree and actor tree in parallel to establish
     * the node-to-actor mapping and extract rest-pose TRS.
     */
    if(data->scene != NULL) {
        cgltf_size ri;
        cgltf_size nroots = data->scene->nodes_count;

        if(nroots > nactors)
            nroots = nactors;

        for(ri = 0; ri < nroots; ++ri) {
            bind_actors_dfs(data->scene->nodes[ri], data, actors[ri], anim);
        }
    }

    /*
     * Parse animation channels.
     */
    anim->nanims      = (br_int_32)data->animations_count;
    anim->active_anim = 0;
    anim->anims       = BrResAllocate(anim, data->animations_count * sizeof(br_gltf_animation), BR_MEMORY_APPLICATION);
    BrMemSet(anim->anims, 0, data->animations_count * sizeof(br_gltf_animation));

    for(i = 0; i < data->animations_count; ++i) {
        parse_one_animation(&data->animations[i], data, &anim->anims[i], anim);
        fprintf(stderr, "ANIM:   anim[%d] '%s' dur=%.3f channels=%d\n",
                (int)i, anim->anims[i].identifier,
                (double)anim->anims[i].duration, anim->anims[i].nchannels);
    }

    {
        br_int_32 bound = 0;
        for(i = 0; i < (cgltf_size)anim->nnodes; ++i)
            if(anim->node_actors[i] != NULL) bound++;
        fprintf(stderr, "ANIM: bound %d/%d nodes, active_anim=%d\n",
                bound, anim->nnodes, anim->active_anim);
    }

    BrResFree(scratch);
    return anim;
}

void BR_PUBLIC_ENTRY BrFmtGLTFAnimUpdate(br_gltf_anim_data *anim, float time)
{
    float     *trs;
    br_int_32  ai, ci, ni;

    if(anim == NULL || anim->nanims == 0)
        return;

    /*
     * Heap-allocate working buffers (no fixed MAX_ANIM_NODES limit).
     */
    trs = BrScratchAllocate(anim->nnodes * 10 * sizeof(float) + anim->nnodes * sizeof(br_int_32));
    {
        br_int_32 *dirty = (br_int_32 *)(trs + anim->nnodes * 10);

        BrMemCpy(trs, anim->rest_trs, anim->nnodes * 10 * sizeof(float));
        BrMemSet(dirty, 0, anim->nnodes * sizeof(br_int_32));

        for(ai = 0; ai < anim->nanims; ++ai) {
            if(anim->active_anim >= 0 && ai != anim->active_anim)
                continue;
            br_gltf_animation *ga = &anim->anims[ai];
            float              anim_time;

            if(ga->nchannels == 0 || ga->duration <= 0.0f)
                continue;

            if(ga->loop)
                anim_time = (float)fmod(time, ga->duration);
            else
                anim_time = (time > ga->duration) ? ga->duration : time;

            for(ci = 0; ci < ga->nchannels; ++ci) {
                br_gltf_channel *ch = &ga->channels[ci];
                float            value[4];
                float           *node_trs;

                if(ch->path < 0 || ch->node_index < 0 || ch->node_index >= anim->nnodes)
                    continue;

                node_trs = &trs[ch->node_index * 10];
                dirty[ch->node_index] = 1;

                evaluate_channel(ch, anim_time, value);

                switch(ch->path) {
                    case 0:
                        node_trs[0] = value[0];
                        node_trs[1] = value[1];
                        node_trs[2] = value[2];
                        break;
                    case 1:
                        node_trs[3] = value[0];
                        node_trs[4] = value[1];
                        node_trs[5] = value[2];
                        node_trs[6] = value[3];
                        break;
                    case 2:
                        node_trs[7] = value[0];
                        node_trs[8] = value[1];
                        node_trs[9] = value[2];
                        break;
                }
            }
        }

        {
            static int dbg_count = 0;
            int dirty_count = 0, applied_count = 0;
            for(ni = 0; ni < anim->nnodes; ++ni) {
                float *n;
                if(dirty[ni]) dirty_count++;
                if(!dirty[ni] || anim->node_actors[ni] == NULL)
                    continue;
                n = &trs[ni * 10];
                applied_count++;
                if(dbg_count < 3) {
                    fprintf(stderr, "ANIM:   apply node %d T=(%.3f,%.3f,%.3f) R=(%.3f,%.3f,%.3f,%.3f)\n",
                            ni, (double)n[0], (double)n[1], (double)n[2],
                            (double)n[3], (double)n[4], (double)n[5], (double)n[6]);
                }
                anim->node_actors[ni]->t.type = BR_TRANSFORM_MATRIX34;
                compose_trs_to_br34(n, n + 3, n + 7, &anim->node_actors[ni]->t.t.mat);
            }
            if(dbg_count < 3) {
                fprintf(stderr, "ANIM: update t=%.3f dirty=%d applied=%d\n",
                        (double)time, dirty_count, applied_count);
            }
            dbg_count++;
        }
    }

    BrScratchFree(trs);
}

void BR_PUBLIC_ENTRY BrFmtGLTFAnimFree(br_gltf_anim_data *anim)
{
    if(anim != NULL)
        BrResFree(anim);
}
