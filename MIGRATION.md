# BRender v1.4.0+ Migration Guide

## Upgrade Checklist

When migrating an existing project to **BRender v1.4.0+**, review and update the following:

### 1. `br_angle` type change

* **Search for:** `br_angle`, `br_fixed_luf`, direct arithmetic on angle values.
* **Replace with:** use `br_scalar` operations or `BR_*()` helpers.
* **If persisting:** wrap with `BrFixedLUFToScalar()` and `BrScalarToFixedLUF()` for compatibility.
* **ABI note:** rebuild all code that embeds `br_angle` in structs.
  Old binaries or object files are not link-compatible.

### 2. Material shading modes

* **Search for:** `BR_MATF_SMOOTH`.
* **Replace with:** `br_material::mode = BR_MATM_SHADING_MODE_GOURAUD`.
* **Default behaviour:**

    * Zero shading mode + `SMOOTH` → Gouraud.
    * Explicit non-zero shading mode overrides `SMOOTH`.
* **Driver caveats:** GL renderer ignores Flat; software renderer ignores Phong.

### 3. Explicit blending

* **Search for:** `BR_MATF_MODULATE`.
* **Replace with:** `BR_MATF_BLEND` if blending is desired.
* **Asset loaders:**

    * Set `BR_MATF_BLEND` when opacity < 255, alpha channel not fully opaque, or `PREALPHA` set.
    * Clear it when alpha is fully opaque.
* **Do not rely on:** alpha-channel existence alone.
* **Do not rely on:** `BR_MATM_BLEND_MODE_*` presence to enable blending.

### 4. Removed flags

* **Search for:** `BR_MATF_ZTRANSPARENCY`.
* **Remove:** entirely; the enum slot is reserved.

## Major Changes

### `br_angle` now uses `br_scalar` (was `br_fixed_luf`, 0.16 turns)

To support smooth rotation at higher refresh rates (120–240 Hz), `br_angle` is now `br_scalar`.
0.16-turn fixed (≈1/65536 turn ≈ 0.0055°) yields visible phase quantisation and long-run drift in high-Hz animation
loops.

If you already use the `BR_*()` math helpers (`BR_MUL`, `BR_ADD`, etc.), changes should be minimal.
Helpers `BrFixedLUFToScalar()` and `BrScalarToFixedLUF()` are provided for migration.

* **ABI/layout:** This is an ABI break for any structs embedding `br_angle`.
* **File I/O:** Older files load; angles are converted to `br_scalar` on read.
  Newer files are **not** readable by older BRender.

Relevant commit: `7a38ebde58960463fe5e753efa0997651206a1d6`.

**Effective shading mode**

| Inputs                                                                 | Result  |
|------------------------------------------------------------------------|---------|
| `mode & BR_MATM_SHADING_MODE_MASK == 0` **and** `BR_MATF_SMOOTH` set   | Gouraud |
| `mode & BR_MATM_SHADING_MODE_MASK == 0` **and** `BR_MATF_SMOOTH` clear | Flat    |
| `mode & BR_MATM_SHADING_MODE_MASK == BR_MATM_SHADING_MODE_GOURAUD`     | Gouraud |
| `mode & BR_MATM_SHADING_MODE_MASK == BR_MATM_SHADING_MODE_PHONG`       | Phong   |
| `mode & BR_MATM_SHADING_MODE_MASK == BR_MATM_SHADING_MODE_FLAT`        | Flat    |

---

### Material shading modes; deprecation of `BR_MATF_SMOOTH`

`br_material::mode` now explicitly selects the shading mode (not the lighting model):

* `BR_MATM_SHADING_MODE_FLAT` (default, value 0)
* `BR_MATM_SHADING_MODE_GOURAUD`
* `BR_MATM_SHADING_MODE_PHONG`

Compatibility rule: if the shading mode is zero (FLAT) **and** `BR_MATF_SMOOTH` is set, treat as **Gouraud**.
Any non-zero shading mode overrides `BR_MATF_SMOOTH`.

Driver support (current implementations):

* `glrend`: **no** Flat.
* `softrend`: **no** Phong.

> **For driver implementors**
>
> `BRT_SMOOTH_B` is set for both Gouraud and Phong (legacy semantics).  
> Use `BRT_SHADING_MODE_T` with `BRT_FLAT` / `BRT_GOURAUD` / `BRT_PHONG` to distinguish, and fail or degrade gracefully
> if unsupported.

---

### Blending: replace `BR_MATF_MODULATE` with `BR_MATF_BLEND`; remove "alpha channel ⇒ blend" behaviour

`BR_MATF_MODULATE` is retired.
`BR_MATF_BLEND` is (re)introduced as the explicit "this material is blended" flag.

Previously, drivers inferred blending via:

1. `opacity < 255`,
2. presence of an alpha channel in the pixelmap,
3. `PREALPHA` (premultiplied semantics).

Rule changes:

* **(2) is removed.** A pixelmap having an alpha channel -- especially PNGs with fully-opaque alpha -- no longer forces
  the blended path.
* **Blending enable** now depends **only** on: `opacity < 255` **or** `BR_MATF_BLEND` **or** `PREALPHA`.
* Blend equation/mode (`BR_MATM_BLEND_MODE_*`) still selects *how* to blend, but the **presence** of a mode (e.g.,
  `STANDARD` being zero) **does not** imply blending.

**Migration note:** On legacy asset load, set `BR_MATF_BLEND` _if and only if_:

* material opacity < 255, or
* pixelmap alpha is **not fully opaque**, or
* `PREALPHA` is set.

If the pixelmap alpha is fully opaque, **clear** `BR_MATF_BLEND`.


**Blend enable**

| Condition                                           | Blended?                          |
|-----------------------------------------------------|-----------------------------------|
| `opacity < 255`                                     | **Yes**                           |
| `BR_MATF_BLEND` set                                 | **Yes**                           |
| `PREALPHA` set                                      | **Yes** (premultiplied semantics) |
| Only pixelmap has an alpha channel (even if opaque) | **No** (post-change)              |

*(Drivers still choose the **blend equation/mode** via `BR_MATM_BLEND_MODE_*`; but **blend enable** comes from the table
above.)*


---

## Minor Changes

### Removal of `BR_MATF_ZTRANSPARENCY`

This flag was unused and is replaced with `BR_MATF_RESERVED_0`.
The value is reserved and shall not be reused.
