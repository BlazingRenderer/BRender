/*
 * animation.c - Core keyframe animation evaluation for BRender
 *
 * Implements BrAnimationInstanceUpdate(): evaluates a br_animation_instance
 * at a given time, interpolating keyframe channels and composing TRS into
 * br_matrix34 transforms on the bound actors.
 *
 * Format-specific loading (glTF, etc.) is handled by core/fmt; this file
 * contains only the format-agnostic runtime.
 */
#include <brender.h>
#include "brassert.h"

/*
 * Evaluate a single animation channel at the given time.
 */
static void anim_eval_channel(const br_animation_channel *ch, br_scalar time, br_scalar *out)
{
    int components = (ch->path == BR_ANIM_PATH_ROTATION) ? 4 : 3;
    int lo, hi, mid;

    if(ch->nkeys == 0)
        return;

    if(ch->nkeys == 1 || time <= ch->times[0]) {
        BrMemCpy(out, ch->values, components * sizeof(br_scalar));
        return;
    }

    if(time >= ch->times[ch->nkeys - 1]) {
        BrMemCpy(out, &ch->values[(ch->nkeys - 1) * components], components * sizeof(br_scalar));
        return;
    }

    lo = 0;
    hi = ch->nkeys - 1;
    while(lo + 1 < hi) {
        mid = (lo + hi) / 2;
        if(ch->times[mid] <= time)
            lo = mid;
        else
            hi = mid;
    }

    if(ch->interpolation == BR_ANIM_INTERP_STEP) {
        BrMemCpy(out, &ch->values[lo * components], components * sizeof(br_scalar));
    } else {
        br_scalar t_range  = BR_SUB(ch->times[hi], ch->times[lo]);
        br_scalar t_factor = (t_range > BR_SCALAR(0.0)) ? BR_DIV(BR_SUB(time, ch->times[lo]), t_range) : BR_SCALAR(0.0);

        if(ch->path == BR_ANIM_PATH_ROTATION) {
            br_quat qa, qb, qout;
            qa.x = ch->values[lo * 4 + 0];
            qa.y = ch->values[lo * 4 + 1];
            qa.z = ch->values[lo * 4 + 2];
            qa.w = ch->values[lo * 4 + 3];
            qb.x = ch->values[hi * 4 + 0];
            qb.y = ch->values[hi * 4 + 1];
            qb.z = ch->values[hi * 4 + 2];
            qb.w = ch->values[hi * 4 + 3];
            BrQuatSlerp(&qout, &qa, &qb, t_factor, 0);
            out[0] = qout.x;
            out[1] = qout.y;
            out[2] = qout.z;
            out[3] = qout.w;
        } else {
            br_scalar *va = &ch->values[lo * 3];
            br_scalar *vb = &ch->values[hi * 3];
            int        i;
            for(i = 0; i < 3; i++)
                out[i] = BR_ADD(va[i], BR_MUL(t_factor, BR_SUB(vb[i], va[i])));
        }
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

void BR_PUBLIC_ENTRY BrAnimationInstanceUpdate(br_animation_instance *inst, br_scalar time)
{
    br_animation_set *set;
    br_scalar        *trs;
    br_int_32         cli, chi, ni;

    if(inst == NULL || inst->set == NULL)
        return;

    set = inst->set;

    UASSERT(inst->nactors == set->nactors);

    if(set->nclips == 0)
        return;

    /*
     * Lazily allocate a working TRS buffer (nactors * BR_ANIM_TRS_STRIDE
     * scalars), a dirty flag array (nactors ints), and a per-(actor,path)
     * owner array (nactors * 3 ints) used to detect overlapping channels
     * across simultaneously-active clips when inst->active < 0.
     */
    if(inst->_eval_buf == NULL)
        inst->_eval_buf = BrResAllocate(
            inst, set->nactors * BR_ANIM_TRS_STRIDE * sizeof(br_scalar) + set->nactors * sizeof(br_int_32) + set->nactors * 3 * sizeof(br_int_32),
            BR_MEMORY_SCRATCH);

    trs = inst->_eval_buf;
    {
        br_int_32 *dirty  = (br_int_32 *)(trs + set->nactors * BR_ANIM_TRS_STRIDE);
        br_int_32 *owners = dirty + set->nactors;
        br_int_32  oi;

        /* Load rest poses into flat TRS buffer */
        for(ni = 0; ni < set->nactors; ++ni) {
            br_animation_pose *pose = &set->rest_poses[ni];
            br_scalar         *dst  = &trs[ni * BR_ANIM_TRS_STRIDE];

            ASSERT(pose->type == BR_ANIMATION_POSE_TRS);

            dst[0] = pose->t.trs.trn.v[0];
            dst[1] = pose->t.trs.trn.v[1];
            dst[2] = pose->t.trs.trn.v[2];
            dst[3] = pose->t.trs.rot.x;
            dst[4] = pose->t.trs.rot.y;
            dst[5] = pose->t.trs.rot.z;
            dst[6] = pose->t.trs.rot.w;
            dst[7] = pose->t.trs.scale.v[0];
            dst[8] = pose->t.trs.scale.v[1];
            dst[9] = pose->t.trs.scale.v[2];
        }

        BrMemSet(dirty, 0, set->nactors * sizeof(br_int_32));
        for(oi = 0; oi < set->nactors * 3; ++oi)
            owners[oi] = -1;

        for(cli = 0; cli < set->nclips; ++cli) {
            br_animation_clip *clip = &set->clips[cli];
            br_scalar          clip_time;

            if(inst->active >= 0 && cli != inst->active)
                continue;

            if(clip->nchannels == 0 || clip->duration <= BR_SCALAR(0.0))
                continue;

            if(clip->looping)
                clip_time = BR_FMOD(time, clip->duration);
            else
                clip_time = (time > clip->duration) ? clip->duration : time;

            for(chi = 0; chi < clip->nchannels; ++chi) {
                br_animation_channel *ch = &clip->channels[chi];
                br_scalar             value[4];
                br_scalar            *actor_trs;
                br_int_32             owner_idx;

                if(ch->path < 0 || ch->path > BR_ANIM_PATH_SCALE || ch->target < 0 || ch->target >= set->nactors)
                    continue;

                /*
                 * Two simultaneously-active clips writing the same
                 * (actor, path) is a content authoring bug: their order
                 * would be file-dependent. Same clip writing twice is
                 * allowed (idempotent overwrite).
                 */
                owner_idx = ch->target * 3 + ch->path;
                UASSERT(owners[owner_idx] < 0 || owners[owner_idx] == cli);
                owners[owner_idx] = cli;

                actor_trs         = &trs[ch->target * BR_ANIM_TRS_STRIDE];
                dirty[ch->target] = 1;

                anim_eval_channel(ch, clip_time, value);

                switch(ch->path) {
                    case BR_ANIM_PATH_TRANSLATION:
                        actor_trs[0] = value[0];
                        actor_trs[1] = value[1];
                        actor_trs[2] = value[2];
                        break;
                    case BR_ANIM_PATH_ROTATION:
                        actor_trs[3] = value[0];
                        actor_trs[4] = value[1];
                        actor_trs[5] = value[2];
                        actor_trs[6] = value[3];
                        break;
                    case BR_ANIM_PATH_SCALE:
                        actor_trs[7] = value[0];
                        actor_trs[8] = value[1];
                        actor_trs[9] = value[2];
                        break;
                    default:
                        break;
                }
            }
        }

        for(ni = 0; ni < set->nactors; ++ni) {
            br_scalar *n;
            br_vector3 t_vec, s_vec;
            br_quat    r_quat;

            if(!dirty[ni] || inst->actors[ni] == NULL)
                continue;

            n = &trs[ni * BR_ANIM_TRS_STRIDE];

            t_vec.v[0] = n[0];
            t_vec.v[1] = n[1];
            t_vec.v[2] = n[2];
            r_quat.x   = n[3];
            r_quat.y   = n[4];
            r_quat.z   = n[5];
            r_quat.w   = n[6];
            s_vec.v[0] = n[7];
            s_vec.v[1] = n[8];
            s_vec.v[2] = n[9];

            inst->actors[ni]->t.type = BR_TRANSFORM_MATRIX34;
            BrMatrix34FromTRS(&inst->actors[ni]->t.t.mat, &t_vec, &r_quat, &s_vec);
        }
    }
}

/*
 * Disconnect all actor references from an animation instance.
 * Call this before freeing actors independently of the instance.
 */
void BR_PUBLIC_ENTRY BrAnimationInstanceDetach(br_animation_instance *inst)
{
    br_int_32 i;

    if(inst == NULL || inst->set == NULL)
        return;

    for(i = 0; i < inst->nactors; ++i)
        inst->actors[i] = NULL;
}
