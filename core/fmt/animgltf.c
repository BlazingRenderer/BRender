/*
 * animgltf.c - glTF animation loading for BRender
 *
 * Parses glTF animation data (keyframes, channels, samplers) and produces
 * a br_animation_set (immutable data) and br_animation_instance (bound to
 * actors) that can be evaluated with BrAnimationInstanceUpdate().
 *
 * Called internally by BrFmtGLTFActorLoadMany() (loadgltf.c) with the
 * already-parsed cgltf_data and actor array.
 *
 * Supports STEP and LINEAR interpolation with quaternion SLERP for
 * rotations. All clips default to looping; set clip->looping = BR_FALSE
 * after loading to clamp at the end.
 */
#include <string.h>
#include <brender.h>

#include "cgltf.h"

/* ------------------------------------------------------------------ */
/* Animation parsing                                                   */
/* ------------------------------------------------------------------ */

static void parse_one_animation(const cgltf_animation *src, const cgltf_data *data, br_animation_clip *dst, void *res)
{
    br_scalar  max_time = BR_SCALAR(0.0);
    cgltf_size ci;

    dst->identifier = (src->name != NULL) ? BrResStrDup(res, src->name) : NULL;
    dst->looping    = BR_TRUE;

    dst->nchannels = (br_int_32)src->channels_count;
    dst->channels  = BrResAllocate(res, src->channels_count * sizeof(br_animation_channel), BR_MEMORY_APPLICATION);
    BrMemSet(dst->channels, 0, src->channels_count * sizeof(br_animation_channel));

    for(ci = 0; ci < src->channels_count; ++ci) {
        const cgltf_animation_channel *ch   = &src->channels[ci];
        const cgltf_animation_sampler *samp = ch->sampler;
        br_animation_channel          *ac   = &dst->channels[ci];
        int                            components;
        br_int_32                      ki;

        if(!ch->target_node) {
            ac->path = -1;
            continue;
        }

        ac->target = (br_int_32)(ch->target_node - data->nodes);

        switch(ch->target_path) {
            case cgltf_animation_path_type_translation:
                ac->path = BR_ANIM_PATH_TRANSLATION;
                break;
            case cgltf_animation_path_type_rotation:
                ac->path = BR_ANIM_PATH_ROTATION;
                break;
            case cgltf_animation_path_type_scale:
                ac->path = BR_ANIM_PATH_SCALE;
                break;
            default:
                ac->path = -1;
                continue;
        }

        switch(samp->interpolation) {
            case cgltf_interpolation_type_step:
                ac->interpolation = BR_ANIM_INTERP_STEP;
                break;
            case cgltf_interpolation_type_linear:
                ac->interpolation = BR_ANIM_INTERP_LINEAR;
                break;
            default:
                ac->interpolation = BR_ANIM_INTERP_STEP;
                break;
        }

        ac->nkeys = (br_int_32)samp->input->count;
        ac->times = BrResAllocate(res, sizeof(br_scalar) * ac->nkeys, BR_MEMORY_APPLICATION);

        components = (ac->path == BR_ANIM_PATH_ROTATION) ? 4 : 3;
        ac->values = BrResAllocate(res, sizeof(br_scalar) * ac->nkeys * components, BR_MEMORY_APPLICATION);

        for(ki = 0; ki < ac->nkeys; ++ki) {
            cgltf_accessor_read_float(samp->input, ki, &ac->times[ki], 1);
            if(ac->times[ki] > max_time)
                max_time = ac->times[ki];
        }

        for(ki = 0; ki < ac->nkeys; ++ki) {
            cgltf_accessor_read_float(samp->output, ki, &ac->values[ki * components], components);
        }
    }

    dst->duration = max_time;
}

/* ------------------------------------------------------------------ */
/* Internal API (called from loadgltf.c)                               */
/* ------------------------------------------------------------------ */

br_animation_set *BrFmtGLTFAnimBuildSet(const cgltf_data *data, br_uint_32 nnodes, void *res_parent)
{
    br_animation_set *set;
    br_uint_32        i;

    if(data->animations_count == 0)
        return NULL;

    set = BrResAllocate(res_parent, sizeof(br_animation_set), BR_MEMORY_APPLICATION);
    BrMemSet(set, 0, sizeof(br_animation_set));

    set->nactors    = (br_int_32)nnodes;
    set->rest_poses = BrResAllocate(set, nnodes * sizeof(br_animation_pose), BR_MEMORY_APPLICATION);
    BrMemSet(set->rest_poses, 0, nnodes * sizeof(br_animation_pose));

    /*
     * Extract rest-pose TRS from each node. cgltf initialises defaults
     * (T=0, R=identity, S=1) even when the node uses a matrix form.
     */
    for(i = 0; i < nnodes; ++i) {
        br_animation_pose *pose = &set->rest_poses[i];
        pose->type              = BR_ANIMATION_POSE_TRS;
        pose->t.trs.trn.v[0]    = BR_SCALAR(data->nodes[i].translation[0]);
        pose->t.trs.trn.v[1]    = BR_SCALAR(data->nodes[i].translation[1]);
        pose->t.trs.trn.v[2]    = BR_SCALAR(data->nodes[i].translation[2]);
        pose->t.trs.rot.x       = BR_SCALAR(data->nodes[i].rotation[0]);
        pose->t.trs.rot.y       = BR_SCALAR(data->nodes[i].rotation[1]);
        pose->t.trs.rot.z       = BR_SCALAR(data->nodes[i].rotation[2]);
        pose->t.trs.rot.w       = BR_SCALAR(data->nodes[i].rotation[3]);
        pose->t.trs.scale.v[0]  = BR_SCALAR(data->nodes[i].scale[0]);
        pose->t.trs.scale.v[1]  = BR_SCALAR(data->nodes[i].scale[1]);
        pose->t.trs.scale.v[2]  = BR_SCALAR(data->nodes[i].scale[2]);
    }

    set->nclips = (br_int_32)data->animations_count;
    set->clips  = BrResAllocate(set, data->animations_count * sizeof(br_animation_clip), BR_MEMORY_APPLICATION);
    BrMemSet(set->clips, 0, data->animations_count * sizeof(br_animation_clip));

    for(i = 0; i < data->animations_count; ++i) {
        parse_one_animation(&data->animations[i], data, &set->clips[i], set);
    }

    return set;
}

br_animation_instance *BrFmtGLTFAnimBuildInstance(br_animation_set *set, br_actor **all_actors, br_uint_32 nnodes, void *res_parent)
{
    br_animation_instance *inst;
    br_uint_32             i;

    if(set == NULL)
        return NULL;

    inst = BrResAllocate(res_parent, sizeof(br_animation_instance), BR_MEMORY_APPLICATION);
    BrMemSet(inst, 0, sizeof(br_animation_instance));

    inst->set     = set;
    inst->nactors = (br_int_32)nnodes;
    inst->active  = 0;
    inst->actors  = BrResAllocate(inst, nnodes * sizeof(br_actor *), BR_MEMORY_APPLICATION);

    for(i = 0; i < nnodes; ++i)
        inst->actors[i] = all_actors[i];

    return inst;
}
