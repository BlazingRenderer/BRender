/*
 * Core animation types: format-agnostic keyframe animation for actors.
 */
#ifndef _ANIMATION_H_
#define _ANIMATION_H_

/*
 * Forward declaration: br_actor is defined in actor.h (via brv1db.h),
 * which is included after animation.h in brender.h.
 */
typedef struct br_actor br_actor;

/*
 * Path types for animation channels
 */
enum {
    BR_ANIM_PATH_TRANSLATION = 0,
    BR_ANIM_PATH_ROTATION    = 1,
    BR_ANIM_PATH_SCALE       = 2,
};

/*
 * Interpolation modes
 */
enum {
    BR_ANIM_INTERP_STEP   = 0,
    BR_ANIM_INTERP_LINEAR = 1,
};

/*
 * Pose variants. Only TRS is defined today; the tag+union shell is kept
 * so future variants (e.g. raw matrices) can be added without an ABI break.
 */
enum {
    BR_ANIMATION_POSE_TRS = 0,
};

/*
 * Number of br_scalar values in a packed TRS pose: T(3) + R_quat(4) + S(3)
 */
#define BR_ANIM_TRS_STRIDE 10

/*
 * A single keyframe track: one TRS path on one target actor.
 * values are packed br_scalar: 3 components for T/S, 4 (xyzw) for R.
 */
typedef struct br_animation_channel {
    br_int_32  target;        /* index into owning br_animation_set actors */
    br_int_32  path;          /* BR_ANIM_PATH_* or -1 to skip */
    br_int_32  interpolation; /* BR_ANIM_INTERP_* */
    br_int_32  nkeys;         /* no. keys */
    br_scalar *times;         /* [nkeys] */
    br_scalar *values;        /* [nkeys * (path==ROTATION ? 4 : 3)] */
} br_animation_channel;

/*
 * A named animation clip: a set of channels sharing a timeline.
 */
typedef struct br_animation_clip {
    char                 *identifier;
    br_scalar             duration;
    br_boolean            looping;
    br_int_32             nchannels;
    br_animation_channel *channels;
} br_animation_clip;

/*
 * Rest pose for a single actor. Tag+union matches br_transform for
 * consistency; today only BR_ANIMATION_POSE_TRS is defined.
 */
typedef struct br_animation_pose {
    br_uint_8 type;

    union {
        struct {
            br_vector3 trn;
            br_quat    rot;
            br_vector3 scale;
        } trs;
    } t;
} br_animation_pose;

/*
 * A scene animation set: immutable data (rest poses + clips).
 * Shareable across multiple instances. All sub-allocations are children
 * of this resource; BrResFree() cleans up.
 */
typedef struct br_animation_set {
    br_uintptr_t       _reserved;
    char              *identifier;
    br_int_32          nactors;
    br_animation_pose *rest_poses; /* [nactors] */
    br_animation_clip *clips;      /* [nclips] */
    br_int_32          nclips;
} br_animation_set;

/*
 * A live animation binding: ties an animation set to a specific
 * actor tree for playback. Multiple instances can share one set.
 */
typedef struct br_animation_instance {
    br_animation_set *set;
    br_actor        **actors;    /* weak refs [nactors] */
    br_int_32         nactors;   /* must equal set->nactors */
    br_int_32         active;    /* clip index (0..nclips-1), or -1 for all */
    br_scalar        *_eval_buf; /* internal: lazily allocated working TRS buffer */
} br_animation_instance;

#endif
