# Mip chains

A mip chain is a pixelmap that carries every level below its own in the same pixel buffer.
The format is Argonaut's; the accessors and the builder are this tree's.

## The layout

- Level 0 is the map's own pixels: `width` × `height`, at the map's `row_bytes`.
- Level *i* is `max(1, width >> i)` × `max(1, height >> i)`, **tightly packed** — its row is
  `width_i × bytes per pixel`, whatever alignment the type would otherwise get — and begins
  where the level above it ends.
- Level 1 begins `mip_offset` rows of level 0 into the buffer. `mip_offset` is a row count,
  not a byte count, and it is the only thing the header records.
- A chain is **complete**: it runs down to 1×1. Nothing stores a level count, so this is the
  contract rather than a convention, and `BrPixelmapMipLevelCount()` derives it from
  `max(width, height)`.
- One allocation holds every level. `PixelmapMipMemorySize()` (internal, `pm_ip.h`) is its
  size. The levels have no `br_pixelmap` and no header of their own.
- `.pix` writes a chain as one `FID_PIXELS` block covering every level, and requires level
  rows to be tight. Any size is legal: because `mip_offset` is a row count, a rectangle — or
  a map whose axes differ in length — is as representable as a square.

## Reading a chain

```c
br_uint_32 n = BrPixelmapMipLevelCount(pm); /* 1 when the map has no chain */

for(br_uint_32 i = 0; i < n; ++i) {
    br_pixelmap *level = BrPixelmapMipLevel(pm, i); /* NULL past the last level */

    upload(level->width, level->height, level->row_bytes, level->pixels);
    BrPixelmapFree(level); /* the header only */
}
```

`BrPixelmapMipLevel()` returns a header over the chain's own pixels: the pixels and the
`map` (palette) pointer are borrowed, so `BrPixelmapFree()` on it releases the header alone.
Level 0's header carries the map's own stride and flags; each level below it carries the
tight stride and says so in its flags.

## Building a chain

`BrPixelmapMakeMipMap(source, type, palette, base, range, method)` returns a new chain:

- Every level is the box filter of the texels above it, accumulated in exact integer sums and
  divided once per level: a level is `round(sum / count)`, rounded **half up**. Alpha is
  filtered like any other channel.
- The type is decoded and written through `core/pixelmap/pmclone.c`'s converter table, so
  every type with a reader and writer pair works, and a type without one is refused rather
  than misread. This matters: the memory pixelmap's `BrPixelmapPixelGet()` returns the pixel
  in the type's own encoding — an index, or a packed 16-bit word — not a decoded colour.
- An `INDEX_8` destination needs a palette. Every level is mapped to the nearest entry within
  `[base, base + range)`, in the `BR_QUANTIZE_RGB` or `BR_QUANTIZE_YIQ` metric.
- A source that is not a plain pixel buffer, or one whose type has no converter, is refused
  with a logged error, and so is one large enough that the filter's 32-bit sums would
  overflow: a level's sum is the whole image's channel total, so the ceiling is
  `width * height * 255` fitting in `int32_t`.

`RGB_888` output is bit-exact against an independently computed round-half-up box filter at
4×4, 8×8 and 16×16; that is the arithmetic the reference rows encode, and
`tools/rendertest --mip-witness` pins it.

## Who consumes a chain

- **glrend** and **vkrend** upload the chain for every non-indexed colour map, building one
  with `BrPixelmapMakeMipMap()` when the map has none, and reach it from
  `BR_MATF_MIP_INTERPOLATION`. Both sample the same texels, which is what lets the witness
  name the byte it expects. An indexed plane is a palette lookup rather than a filtered
  surface, so it keeps one level on both.
- **glrend1x** uploads level 0 and ignores `BR_MATF_MIP_INTERPOLATION`; see `renderers.md`.
- **softrend** textures per fragment but keeps no chain and selects no LOD, so the flag has
  nothing to act on.

`rendertest --mip-witness` (`mipw-glrend`) asserts the level a minified sample reaches: a
16×16 red/blue checkerboard minified four-to-one must read the 50/50 mix at `80 00 80 ff`
with the flag, and a hard texel without it.
