# Texconv, Colour Keying, and Indexed Transparency in BRender

## Summary

BRender palette-indexed textures use **palette index 0 as the transparent
colour**. This is a hardcoded convention enforced by:
- pentprim's 8-bit assembly rasterisers (unconditionally skip index 0)
- glrend's shader (`texColour.rgb == vec3(0,0,0)` check)
- texconv and pmclone (threshold-based pixel keying)

## How pentprim handles it

All 8-bit textured rasterisers (`drivers/pentprim/t_pia.asm`,
`ti8_pi.asm`, `ti8_pip.asm`, `ti8_piz.asm`, `fti8_pip.asm`, etc.) have a
hardcoded transparency check in the pixel-draw loop:

```asm
mov     bl, [texel]          ; raw palette index from texture
test    bl, bl               ; is it 0? (some variants use and bl,bl)
jz      skip_pixel           ; skip pixel (label varies: pixel_behind, nodraw, loopb, etc.)
```

This happens **before** the CLUT/shade-table lookup — it checks the raw
index, not the resulting RGB.

**Non-textured** rasterisers (flat PI, gouraud PII) have no transparency check.

The `BRT_COLOUR_KEY_B` material token is **completely ignored** by pentprim.
It has no `PRIMF_COLOUR_KEY_BIT` in its flag enum and no `BRT_COLOUR_KEY_B`
template entry. Transparency is unconditional for textured 8-bit.

## How glrend handles it

glrend DOES support `BRT_COLOUR_KEY_B` via `PRIMF_COLOUR_KEY` in its state.
The shader at `brender.frag.glsl:146` checks post-CLUT RGB:

```glsl
if(!disable_colour_key && texColour.rgb == vec3(0.0, 0.0, 0.0))
    discard;
```

Controlled by `disable_colour_key` boolean derived from `PRIMF_COLOUR_KEY`.
Checks **RGB result** after palette lookup, not the raw palette index.

## How softrend bakes opacity into C_A

`drivers/softrend/v1model.c:804-807` merges the surface opacity into the
colour's alpha byte before vertex functions run:

```c
scache.colour = renderer->state.surface.colour & 0xFFFFFF;
scache.colour |= BrScalarToInt(BR_CONST_MUL(renderer->state.surface.opacity, 256)) << 24;
```

This means `SurfaceAlpha` (`drivers/softrend/alpha.c`) computes C_A from
the baked opacity value, not the surface colour's original alpha.

## pmclone.c — palette cloning with transparency

`core/pixelmap/pmclone.c` provides per-format pixel read/write converters
used by `DevicePixelmapMemCloneTyped` and `BrPixelmapCloneTyped`.

### index_8_read (line 10)

```c
colour = BrPixelmapPixelGet(pm->map, 0, index - pm->map->origin_y);
if(index == 0)
    return BR_COLOUR_RGBA(r, g, b, 0);  // transparent
return BR_COLOUR_RGBA(r, g, b, 255);     // opaque
```

### index_8_write (line 44)

```c
if(BR_ALPHA(colour) <= opts->index_alpha_threshold) {
    *pixels = 0;      // write index 0 (transparent)
    return;
}
BrQuantMapColours(opts->target_clut ? 0 : 1, rgb, pixels, 1);  // quantize into entries
```

### BrPixelmapConvert (core/pixelmap/pmclone.c, line 478+)

The main conversion function, replacing the old `DevicePixelmapMemCloneTyped` / `BRT_CLONE` dispatch chain. When cloning TO `BR_PMT_INDEX_8`:

1. **Quantizer input**: skips pixels with `BR_ALPHA(colour) <= opts->index_alpha_threshold`
2. **Palette**: built via `BrQuantMakePalette(1, 255, palette)` — reserves slot 0
3. **Slot 0**: set to fully transparent `BrPixelmapPixelSet(palette, 0, 0, 0)`
4. **Mapping**: `BrQuantPrepareMapping(1, 255, palette)` — entries 1..255
5. **Output**: `index_8_write` handles transparent pixels directly (writes 0)

## texconv — `tools/texconv/palquant.c`

### BrQuantAddPixelmap (line 78)

```c
threshold = colour >> 24;         // alpha byte
if(threshold < alpha_threshold)    // strict less-than: alpha < threshold added to quantizer
    threshold = 0;
if(!threshold)
    BrQuantAddColours(rgb, 1);     // add to quantizer
```

Pixels with alpha < alpha_threshold are added to the quantizer. Pixels with alpha >= alpha_threshold
are skipped.

### BrQuantMapPixelmap (line 126)

```c
threshold = colour >> 24;
if(threshold < alpha_threshold)
    threshold = 0;
if(!threshold)
    BrQuantMapColours(base, rgb, ..., 1);  // map to palette index
else
    pixels[y * row_bytes + x] = 0;         // write index 0
```

### texconv `convert.c` — T_ConvertIndexedPixelmap (line 612)

Similar threshold logic to `palquant.c`, but note **convert.c uses `<=` (inclusive) while palquant.c uses `<` (exclusive)**:
1. Source converted to `BR_PMT_RGBA_8888` internally
2. `BR_ALPHA <= alpha_threshold` → added to quantizer
3. `BR_ALPHA > alpha_threshold` → mapped to index 0 in output
4. Palette built via `BrQuantMakePalette(base, range, palette)`
5. Index 0 is the "transparent" key colour

The `alpha_threshold` default (127) and the `alpha_channel` toggle control
whether alpha-as-opacity or alpha-as-keying-mask is used.

## Quantizer API (`core/pixelmap/quantize.c`)

Wu's color quantizer. Key functions:

- `BrQuantBegin()` — initialise
- `BrQuantAddColours(rgb, count)` — collect source pixel colours
- `BrQuantMakePalette(base, num_entries, palette)` — build palette at `base` offset
- `BrQuantPrepareMapping(base, num_entries, palette)` — build inverse LUT
- `BrQuantMapColours(base, rgb_in, index_out, count)` — map RGB to palette indices
- `BrQuantEnd()` — cleanup

The `base` parameter offsets where palette entries are written and where
mapping output starts. Using `base=1, num_entries=255` reserves palette
slot 0 for the transparent key colour.

## Colour keying in glrend1x

glrend1x uses `GL_ALPHA_TEST` for colour keying, implemented via:

1. **Texture upload**: two copies of each texture — one with alpha=0 for transparent
   pixels (palette index 0 / RGB=(0,0,0)), one with alpha=255 everywhere. Selected
   based on whether colour-keying is active (`sbuffer.c:97-117`).
2. **In `pushGLState`** (`pstate.c:368-372`): when `!is_blended && !disable_colour_key`, enable
   `GL_ALPHA_TEST` with `AlphaFunc(GL_GREATER, 0.5f)`
3. **For blended primitives**: alpha test stays disabled (alpha is blend factor)

This matches pentprim's unconditional transparency for textured 8-bit,
and glrend's toggleable `PRIMF_COLOUR_KEY` for non-indexed.
