# Texconv, Colour Keying, and Indexed Transparency in BRender

## Summary

BRender palette-indexed textures use **palette index 0 as the transparent
colour**; a non-indexed texture uses an all-zero RGB texel. The key is tested on
the raw source texel, before any palette or shade lookup, and is enforced by:
- softprim's indexed 8-bit rasterisers (unconditionally skip index 0)
- glrend's shader (palette index 0 for an indexed map, black for an RGB one)
- texconv and pmclone (threshold-based pixel keying)

## How softprim handles it

All 8-bit textured rasterisers have a hardcoded transparency check in the
pixel-draw loop: the raw palette index is tested for zero and a zero texel is
skipped. This happens **before** the CLUT/shade-table lookup — it checks the raw
index, not the resulting RGB.

**Non-textured** rasterisers (flat PI, gouraud PII) have no transparency check.

The `BRT_COLOUR_KEY_B` material token is **completely ignored** by softprim.
It has no `PRIMF_COLOUR_KEY_BIT` in its flag enum and no `BRT_COLOUR_KEY_B`
template entry. Transparency is unconditional for textured 8-bit.

## How glrend handles it

glrend honours `BRT_COLOUR_KEY_B` via `PRIMF_COLOUR_KEY`. The shader tests the
**raw source texel, before the palette lookup** - palette index 0 for an indexed
map, an all-zero RGB texel for a non-indexed one:

```glsl
bool keyed = (texture_mode == TEXTURE_MODE_NORMAL) ? (texColour.rgb == vec3(0.0, 0.0, 0.0))
                                                   : (texture(index_texture, mappedUV).r == 0u);

if(!disable_colour_key && keyed)
    discard;
```

`index_texture` is the `usampler2D` the indexed modes already sample, so both
branches key the raw texel, as the software rasterisers do.

**Filtering is the gap, and it has no reference.** No software renderer in this
tree or in `BRender-1997`/`BRender-v1.3.2` ever filtered a texture
(`MAP_INTERPOLATION_T` reaches only the GL and hardware drivers), so `BRT_LINEAR`
texture filtering exists only in glrend and glrend1x. The key is therefore
sampled at the fragment's own texel while the colour is filtered, which leaves a
fringe at a key edge under `TEXTURE_MODE_INDEX_FILTER`. See issue #36.

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

1. **Texture upload** (`sbuffer.c:97-117`): two copies of each texture - one
   keyed, one not. An `INDEX_8` source needs no colour key on top of the
   conversion: `index_8_read()` already gives palette index 0 alpha 0 and every
   other index 255, so only index 0 is keyed. A non-indexed source carries no
   alpha of its own and gets the black colour key instead.
2. **In `pushGLState`** (`pstate.c:368-372`): when `!is_blended && !disable_colour_key`, enable
   `GL_ALPHA_TEST` with `AlphaFunc(GL_GREATER, 0.5f)`
3. **For blended primitives**: alpha test stays disabled (alpha is blend factor)

This matches softprim's unconditional transparency for textured 8-bit, and
glrend's toggleable `PRIMF_COLOUR_KEY`.

## Guidance: prefer alpha, and know where it works

The colour key is a legacy mechanism: a binary mask on the source texel. It
cannot be filtered coherently, it cannot be partial, and a palette that puts
black at a non-zero index does not mean what an author expects. New content
should carry transparency in the alpha channel and blend.

That advice is GL-only, and it is worth knowing before authoring against it:

- **The software rasterisers have no alpha-carrying texture format.** softprim's
  texture axis is `SP_TEX_I8/555/565/RGBX888/RGB888`
  (`drivers/softprim/softprim_axes.h`), and the pixelmap-type map in
  `drivers/softprim/infogen.pl` has no RGBA entry. An RGBA texture drawn by
  softrend renders its RGB **opaque** - the cutout is not soft, it is gone. On
  the software path the colour key is the only cutout mechanism.
- **glrend has no alpha test.** Its shader's only `discard` is the colour key, so
  texture alpha reaches the frame only through `GL_BLEND` - a blended draw, which
  does not write depth. A cutout authored as alpha that was an opaque,
  depth-writing keyed surface becomes a sorted blend.
- **glrend1x does alpha-test** (`GL_ALPHA_TEST`, `drivers/glrend1x/pstate.c`), so
  the same asset behaves differently on the two GL renderers.

So: alpha where the target is GL and the draw can blend; the colour key where the
target is the software rasteriser, or where the surface must be opaque and
depth-writing.
