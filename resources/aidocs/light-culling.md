# Light culling

*Hand-written design note, not generated like the other files here. Written
2026-10-03 while investigating the scene UBO / renderer-state CPU cost, and
revised 2026-10-10. The revision matters more than the note: everything it
described as missing work landed the day after it was written, so its summary and
its glrend and softrend sections were describing a tree that no longer existed.
The measurements are unchanged, and they are the reason the glrend half has the
shape it does.*

## Summary

The core evaluates the cull; the drivers only honour the flag.

`BrSetupLights()` derives a per-light policy and view-space geometry from each
light's own data, `BrLightCullReset()` tests that geometry against the model about
to be drawn, and the result reaches the renderer as `CULLED_B`. No driver derives
the test.

- **softrend** reads `culled` and skips those lights.
- **glrend** reads `culled` and leaves those lights out of the affine list it
  packs for the draw — but only when the draw names a model. The per-triangle path
  carries no model, so nothing is culled on it.

That second bullet is all that is still open here, and it is not a port: the port
happened, and by the time glrend honoured the token the compaction underneath it
had already landed.

## What the core provides

`BrSetupLights()` (`core/v1db/enables.c:310`), called during scene setup, decides
the per-light *policy* from the light's own data and pushes it into renderer
state:

```c
/* Enable radius culling if outer radius given */
tvp->t   = BRT_RADIUS_CULL_B;
tvp->v.b = light->radius_outer != BR_SCALAR(0.0);

/* direct/spot lights, or lights with volume regions: all of these have a
 * direction, which the shading needs */
if((light->type & BR_LIGHT_TYPE) == BR_LIGHT_DIRECT ||
   (light->type & BR_LIGHT_TYPE) == BR_LIGHT_SPOT ||
   light->volume.regions != NULL) {
    /* (writes DIRECTION here, transformed into view space) */

    /* but only a spot has a cone, so only a spot is angle-culled */
    if((light->type & BR_LIGHT_TYPE) == BR_LIGHT_SPOT) {
        tvp->t   = BRT_ANGLE_CULL_B;
        tvp->v.b = BR_TRUE;
    } else {
        tvp->t   = BRT_ANGLE_CULL_B;
        tvp->v.b = BR_FALSE;
    }
}
```

The two flags describe independent volumes, not one test written twice. A spot can
be close enough to reach a model with its radius and still miss it with its cone;
a long thin cone can reach a model the outer radius does not. So the radius test
runs first and the cone test only when it did not already cull the light
(`enables.c:168-175`), and `sphereIntersectsCone()` (`:85`) is the cone test.
Direct lights take the `else` above and are never cone-culled: `scene-directional`
differing from `scene-directional-miss` is a lighting difference, not a cull.

`radius_cull` needs two things rather than one: the light has to be of a kind
that has a position and a radius at all (`BR_LIGHT_POINT`, `BR_LIGHT_SPOT`, a
volume region, or `BR_LIGHT_LINEAR_FALLOFF`), and then `radius_outer` has to be
non-zero. `examples/mkres/scene.c:155-160` sets the falloff bit whenever it is
given a radius for exactly that reason - a fixture that set one without the other
would test nothing.

It also transforms position, radius, direction and cone angles **into view space**,
and stores them in `v1db.light_cull[]` (`core/v1db/v1db.h:107-120`) for the cull to
read.

Lifecycle:

- `BrLightCullReset(br_model *model)` (`enables.c:134`) **evaluates** the cull; it
  no longer only clears the flags. It scales the model's bounding radius into view
  space (`modelToViewRadiusScale()`, `:112`), tests each light's reach or cone
  against it (`sphereIntersectsCone()`, `:85`, for the cone), and sets `CULLED_B`
  through `RendererPartSetMany`. It is called from `BrDbModelRender()`
  (`core/v1db/render.c:52`) rather than from `actorRender()`, so that
  `BrZ{b,s}ModelRender()` reaches it too.
- `BrLightModelCull(br_actor *light)` (`enables.c:187`, declared
  `core/inc/v1db_p.h:2304`) sets `CULLED_B = TRUE` for one light and is
  `BR_PUBLIC_ENTRY`. It still **has no callers anywhere in this tree** — it is an
  application hook.

## softrend

`drivers/softrend/setup.c` starts each light from the engine's flag and skips it:

- `:521`, `:598` — `alp->culled = alp->s->culled;` then skip when set.
- `:529`, `:541` — `radius_cull` gates the view→model position transform. This is
  not a test; the test is the core's. A light without a radius never needs its
  position in model space, which is all the condition asks.
- `:482` — `if(lp->culled) continue;`, so the render loop skips culled lights.

## glrend

- `drivers/glrend/state.h:120-125` — `radius_cull`, `angle_cull`, `culled`.
- `drivers/glrend/state_light.c:52,53,57` — `Q | S` templates for
  `RADIUS_CULL_B`, `ANGLE_CULL_B`, `CULLED_B`; defaults `BR_FALSE` at `:77,78,80`.
- `drivers/glrend/cache.c:189` — `StateGLBuildLightLists()` skips a light whose
  `culled` is set, leaving it out of the affine list packed into the model block
  for this draw.

The guard is conditional on the draw naming a model:

```c
/*
 * The flag describes the model being drawn, so it only applies to a
 * model draw. The per-triangle path carries no model and so still
 * holds whichever model was culled last, which must not be used.
 */
if(v11m != NULL && lights[cache->light_part[i]].culled)
    continue;
```

So `radius_cull` and `angle_cull` remain write-only in glrend, and that is now
deliberate: the core owns the test, and a driver-side one would be a second copy
of it. What glrend cannot do is cull the per-triangle path, for want of a model to
test against.

## Why the shape is what it is

The token is a **decision input, not a mechanism**. Measured on
`dev/glrend-lightcull-wip`, while this was still a driver-side port:

- per-light `uvec4` bitmask + `continue`: **+0.38 ms of test, −0.01 ms saved** —
  the light bodies get *predicated*, not branched around, so the `sqrt`,
  `normalize` and dots still execute.
- per-draw index list: **−17% on the GTX 960M, +2% on the HD 530** — making the
  array accesses data-dependent turns affine addressing into indirect fetches,
  which on NVIDIA cost more than 15 extra affine lights.
- affine compaction: **1.49× HD 530, 1.14× 960M, 1.01× 780M**. Only this shape
  pays, and it is the one that landed.

`dev/glrend-lightcull` (tip `510a70a8`, notably `8b18c8a9` *"carry only the lights
a draw can see"*) is where that was worked out. Both of its commits are on master,
as `2a51de2f` and `dce28f8d`. Its message says its cull "follows
softrend/setup.c", and the model→view radius transform it derived for itself is
the one that later moved into the core — that derivation is where the over-cull
bug came from, culling in the wrong space.

The rest of the sequence:

- `47d99a58`, `de200230` — softrend honours the core's cull, including the indexed
  path.
- `643577da` — the core evaluates the per-model cull in view space, so drivers only
  honour the flag.
- `8ac4ba03` — glrend reads the core's `CULLED_B` instead of testing again.

## Open questions

1. **Does Croc's game call `BrLightModelCull()`?** Still not in this tree. It is
   additive now rather than load-bearing, since the engine culls regardless, but it
   is the only way an application can cull a light the engine thinks reaches the
   model, and nothing here exercises it.
2. **glrend's per-triangle path is never culled**, because it carries no model to
   test against.

## Related, but separate

The CPU profile that led here found the scene's ~0.6 ms of CPU is dominated by
`__memmove` (~26% of the profile), and that ~90% of that is BRender copying
`state_stack` (6376 bytes, of which `state_light[48]` is 5760 — 90%) **per
primitive**. `MASK_STATE_STORED` (`drivers/glrend/state.h:28`) is only
`CULL|SURFACE|PRIMITIVE` (224 bytes). That is a distinct problem from light culling
and is tracked separately.
