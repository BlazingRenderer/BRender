# softprim

softprim is BRender's portable C/C++ software rasteriser. It replaced the x86
assembly driver `pentprim` without changing the entry point
(`BrDrv1SoftPrimBegin`), device identifier (`SOFTPRMF`), primitive library
(`Default-Primitives-Float`) or reference-table driver token (`softprmf`). It
builds with the other drivers when `BRENDER_BUILD_DRIVERS` is enabled.

## Block selection

`SP_TABLES` in `CMakeLists.txt` lists the `.ifg` tables in match order.
`infogen.pl` turns each block into kernel axes and matcher requirements;
`merge_blocks.pl` keeps every matcher row but emits one kernel per distinct
axis tuple. `match.c` picks the first row whose requirements are met. This
order matters even when two rows share a kernel: their buffer or flag
requirements may differ.

Every block in the current tables has a kernel. A shape outside `SP_SPEC`
still gets a `SOFTPRIM_REFUSED` row so it cannot fall through to a sibling
kernel and silently draw the wrong result. The assertions in `SoftPrimRender`
check that admitted shapes have dispatch and the necessary kernel modes.

`soft_t24.ifg` supplies the RGBA sprite path: affine sampling at RGB888,
RGB555 or RGB565 output, with texture alpha multiplied by vertex alpha.

### RGB555 and RGB565

`soft_t15.ifg` and `soft_t16.ifg` put the Z-buffered native colour maps and
indexed textures with an RGB shade table before the matching MMX-derived
table. The latter's untextured rows do not require a texture type, so they
would take those primitives before the general rows could see them. Paletted
textures without a matching RGB shade table still take the MMX-derived path.
softprim does not have an MMX toggle.

- **Native colour maps:** `RgbPow2Triangle` handles tight square maps at
  32, 64, 128, 256 and 1024 texels. Its rows precede `RgbAwtTriangle`, which
  handles the other dimensions, including 512×512 and rectangular maps. The
  Z-sorted table has no size-specific rows.
- **Indexed textures with an RGB shade table:** the table's format must match
  the destination. The 256×256 perspective rows use `PizShadeTriangle`; an
  affine 256×256 map or a different size uses `RgbAwtTriangle`. A blended
  shade fragment in the 256×256 cells tests depth without writing it; a zero
  texel writes neither colour nor depth.

These choices preserve the general kernels previously reachable only with
MMX disabled in pentprim, without replacing the paletted MMX-derived kernels.
The port uses the bound map dimensions rather than instantiating a separate
body for each supported size.

## Verification

`tools/rendertest` scores the fixture corpus and `resources/gltf-reference`
against `tools/rendertest/rendertest.txt`. CTest registers 8/15/16/24bpp with
and without depth. Run the reference scenes one per process, as CTest does:
rendering them sequentially in one process can change their checksums.
`contrib/census` records which blocks were selected; a passing render alone
does not identify the kernel that ran.

Both floating-point classes use softprim's own software reference. The older
MMX-off comparisons were made on isolated draws: pentprim cannot reproduce a
whole frame that combines its general and MMX paths. The corpus now checks
returned pixels; the transparent-depth, equal-depth and blend-depth-write
comparisons used separate overlap probes rather than the registered fixtures.

The references come from Release builds. On x87, changing optimisation can
change where intermediates spill and round, including in stages before the
rasteriser. A Debug mismatch is not necessarily a rasteriser regression.

When changing a kernel, update its axes and implementation together, verify
selection, and check both floating-point classes before accepting intentional
pixel changes. See the [reference recipe](../../resources/aidocs/recipes.md#updating-render-references).
