# softprim

The software rasteriser: a portable C/C++ port of the x86 assembly rasteriser that was
`drivers/pentprim/`, written to reproduce pentprim's output byte-for-byte rather than to be
a new renderer. 8,893 lines of C and C++, plus pentprim's `.ifg` block descriptions.

## Why it replaced pentprim

pentprim was hand-written 1990s x86 assembly. Building it needed a 32-bit x86 target, a
MASM-compatible assembler (`uasm`, or MSVC's `ml.exe` on Windows), Perl, and a generated
header (`drv.h` -> `drv.inc`, via `h2inc`, or `H2INC.EXE` under Wine before that).

softprim is portable C/C++ and builds with the drivers (`BRENDER_BUILD_DRIVERS`). It
presents the same entry point (`BrDrv1SoftPrimBegin`), device and
primitive-library identifiers (`SOFTPRMF`, `Default-Primitives-Float`) and install name
(`softprmf`) pentprim did, so nothing outside `drivers/softprim/` changed. Frames are keyed
by the `softprmf` driver token, so a run scores directly against the checked-in references.

pentprim's pixels are the oracle, and they are frozen: `tools/rendertest/rendertest.txt`
holds the `softrend/softprmf/x87` keys - fixtures and reference scenes alike - and with
pentprim gone they are the only record of its output.

## The block table, and what a kernel is allowed to be

- `SP_TABLES` (`CMakeLists.txt`) names the `.ifg` block descriptions in `tables/`.
  Fourteen are pentprim's own, carried here because they are the specification both
  rasterisers implement; the generator `infogen.pl` is theirs too. The others are
  softprim's: `soft_t24.ifg` is the alpha-blended RGBA sprite family, which pentprim never
  had, and `soft_t15.ifg`/`soft_t16.ifg` restore the z-buffered native 555/565 triangle rows
  of the general table ahead of the MMX table.
- `infogen.pl` in its `softprim` mode projects each block onto the axis vocabulary in
  `softprim_axes.h` and emits one line per block carrying the axis tuple (which names the
  kernel) plus the metadata pentprim's matcher tests.
- The generated list is expanded twice. `match.c` builds the matcher's ordered table from
  it, in pentprim's table order, first match wins, including the fall-through; `raster.cpp`
  defines one kernel per distinct tuple (`merge_blocks.pl` collapses blocks that share a
  kernel). The same tokens feed both, so a block cannot name a kernel that does not exist.
- `SP_SPEC` is the explicit register of what is implemented. A shape outside it is emitted
  `SOFTPRIM_REFUSED` rather than omitted, because pentprim spells most variants as a match
  flag on a block ordered before its twin: dropping the variant would let the state it
  exists to take fall through to the twin and be drawn as though the variant had been
  applied. Refused blocks stay in the walk so the matcher stops on them and the primitive
  is refused. **A shape whose kernel does not exist is refused, not drawn by a sibling's
  kernel.**
- The guards are compile-time, not runtime: `SoftPrimRender`'s `static_assert`s in
  `raster.cpp`, plus the two assertions at its dispatch tail, which name the shapes the
  generator refuses. Widening `SP_SPEC` without writing the kernel stops the build, as
  does a new `.ifg` block that reintroduces a shape nothing implements.

## Porting a kernel from pentprim

Every kernel here came from pentprim, and the requirement is bit-exactness, so the target
is **pentprim's arithmetic, not a clean reimplementation** of the same algorithm. These are
the rules the first 221 kernels were ported to, recovered from the session that did it.

- **Read the source the block actually uses.** pentprim has more than one implementation of
a path and the `.ifg` says which one a block gets: `awtmi.h` for arbitrary-width maps,
`perspi.h`/`perspzi.h` for the power-of-two and perspective-scan ones, the `tt*.asm`
family for perfect scan. Port the vertex sort, the `g_divisor`/`g_inverse` setup, the u/v
fractional parts, the `(1<<28)/maxuv` normalisation, the truncating `(br_int_32)` casts and
`PDIVIDE` as they are written, not as they would be written afresh.
- **Where a C reference and the assembly disagree, the assembly is the authority.** It is
what the branch builds and what the reference entries were blessed from. The 256-wide RGB
destination blend is the case: `perspi.h` has no blend arm at all, so its `PITIPB256`
instantiation is the same code as the plain one - while `t15_pip.asm`'s `ScanLinePITIP`
takes a `BLEND` operand and halves source and destination through per-format masks.
- **One sanctioned divergence: map dimensions are runtime.** pentprim instantiates a kernel
per texture size (`texture8x8` … `texture1024x1024`); here one kernel takes its shift and
mask from the bound map. That is the only deliberate structural difference - anything else
that looks like one is a bug in the port.
- **Widen the spec and write the kernel in the same commit.** `SP_SPEC` may only name axes
whose kernels exist, and the `static_assert`s in `SoftPrimRender` enforce it in both
directions: a kernel with no spec entry is dead code, and a spec entry with no kernel stops
the build.
- **Refuse cleanly rather than guess.** A shape with no kernel returns no block. Drawing it
with a sibling's kernel is the failure the refusal mechanism exists to prevent, not a
cheaper way to make a fixture pass.
- **Witness it before trusting it,** and write the fixture first. The corpus is what says
the kernel ran and `contrib/census` is what says which tuple the state selected; a kernel
with neither is untested code that happens to compile.
- **Fix pentprim first if pentprim is wrong.** Two families here were ported only after
pentprim itself was corrected - the RGB_888 shade table's four-byte stride over a
three-byte entry, and the 32x32 block's packing. A pixel-affecting pentprim fix, its
re-bless and the port that follows are one change, with the reference entries moving in the
same commit.
- **Validate in this order.** It builds - check for `Built target`, because a failed build
leaves the previous binary in place and every measurement of it is meaningless;
`rendertest` reports `result=PASS failures=0` at 8/15/16/24bpp x {zb, zs}; the
`refscenes-*` tests reproduce the nine reference scenes' frozen checksums; `mkres scenes`
still writes `dat/` byte-for-byte. Bless from pentprim only: a softprim `--bless` records softprim's own output
and the comparison stops meaning anything. Bless the reference scenes the same way and one
scene per process - `rendertest --device softrend --bpp <bpp> [--no-depth] --scene-dir
resources/gltf-reference --bless <scene>` - adding `--reference
tools/rendertest/rendertest.txt` when the bless runs from another tree, whose own
default reference is not this one.

## What is implemented

All of pentprim's 393 live blocks are in the matcher's walk and every one of them is
emitted as a block with a kernel; there are no refusals left. Every `INDEX_8` and every
`RGB_888` shape is implemented. The implemented set includes the `INDEX_8` ROPs (indexed
blend, fog, decal, dithered map), the shade-table family including the 256x256 RGB cells at
both depths and both perspective settings, the MMX 15/16bpp family including screendoor and
colour dither and its packed 20.12 texture addressing, the arbitrary-width and perfect-scan
RGB paths, the size-specialised native 555/565 colour-map cells, and lines and points.

The one block family pentprim never had is `tables/soft_t24.ifg`: RGB output, unlit, a
`BR_PMT_RGBA_8888` colour map sampled with arbitrary-width addressing, affine, z-sorted or
z-buffered, the texel modulated by the vertex colour and composited as
`dst = texel.rgb*vertex.rgb * a + dst * (1-a)` per channel, with `a` the product of the
texel's own alpha and the vertex alpha.
It is six blocks - one per output format (888/555/565) and depth mode - and six tuples, and
the counts above, which are pentprim's, do not include it. Its table is first in `SP_TABLES`
because the only state that reaches it is one with an RGBA map bound; every pentprim
textured row fails its texture-type requirement and falls through, and no pentprim block
requires that map type, so no existing state's selection moves. `scene-rgba-blend` in
`tools/rendertest` exercises this family.

Two families were refused for a different reason and are implemented now, because pentprim
itself was corrected first:

- the RGB_888 shade-table family. pentprim's reader indexed a 256x256 RGB_888 table at a
  four-byte stride while its own matcher required a three-byte-entry table, so every
  intensity of 192 or more read past the allocation and "bit-exact" was not well defined.
  With the read corrected to three bytes an entry, softprim implemented the family.
- the z-sorted 32x32 perspective block. pentprim built the base texel for that one block as
  if the map were 64 texels wide; the port followed the corrected packing, and added
  `scene-tex-32` to witness it.

## What is deliberately not implemented

Nothing. The last gap was the z-buffered 256x256 RGB shade-table cells (8 entries:
`SP_SHADE_CONST_I_RGB`/`SP_SHADE_INTERP_I_RGB` over `SP_TEX_I8` at `SP_ADDR_SHIFT`, the
z-buffered `TriangleRenderPIZ2TIP256_RGB_555` and `...PIZ2TIPB256...` cells and their 565
twins), and they are `PizShadeTriangle` at both depths now. `contrib/census/reachable.py`
and `witness.py` over the generated table report 0 refused entries, 0 refused tuples, and
the only dead tuples are the two that predate this family.

The native 555/565 colour maps are all implemented, at both depths. The arbitrary-width
cells come from the general table when z-sorted and from `tables/soft_t15.ifg` and
`soft_t16.ifg` when z-buffered; the size-specialised cells come from the same two soft tables
(`RgbPow2Triangle`, `zb16p2ul.asm`). Those two are softprim-only tables listed before the
MMX table, because softprim's single walk would otherwise let an MMX untextured row - which
requires no texture type - take the state before the general table was reached. They
restore the answer MMX-off pentprim got from the general table for the two states the MMX
table has no kernel to serve: an RGB 555/565 colour map, and an `INDEX_8` map with an RGB
555/565 shade table. A normal paletted MMX primitive still reaches the MMX textured rows
first, and an untextured one still reaches the MMX untextured rows. The files carry the
match contract: the colour-map rows list the general table's five size rows ahead of the
arbitrary-width one, so a size-specific cell keeps its map and every other map - a 512x512,
a 64x32, the 96x48 the corpus ships - falls through to the arbitrary-width row exactly as
it does in pentprim; the shade-table rows list the exact 256x256 perspective rows first,
with a positive `texture256x256`, and the arbitrary-width rows after them. An affine
256x256 map fails the exact rows' PERSPECTIVE requirement and reaches the arbitrary-width
row, as it does in the general table.


## How it is verified

Both instruments score softprim against **pentprim's frozen pixels**, never against its own
output, with the fourteen mixed-walk keys listed below as the explicit exception.

- **The fixture corpus.** `tools/rendertest` renders 119 fixtures at four pixel formats
  (8/15/16/24bpp) x {z-buffered, z-sorted} and compares each against
  `tools/rendertest/rendertest.txt`, whose `softrend/softprmf/x87` keys are pentprim's
  except for the fourteen mixed-walk keys listed below. Measured: `PASS failures=0`, 119
  comparisons, in all eight configurations.
- **Nine reference scenes.** `resources/gltf-reference`, rendered by the same harness with
  `--scene-dir resources/gltf-reference` and scored against the same table: 9 scenes x
  8/15/16/24bpp x {z-buffered, z-sorted}, registered as the
  `refscenes-<bpp>-<mode>-<scene>` ctest tests. Measured: all 72 reproduce pentprim's
  frozen checksums exactly, byte-for-byte. One scene per process, and not the nine in one:
  a frame here is not a function of its scene alone - it depends on which scenes ran before
  it in the same process (measured: `croc-mp033_01` at 24bpp z-buffered is
  `6d12bbdcba22365e` alone and `d2ea20cea7920cdd` behind `croc-mp032_00`), and pentprim and
  softprim then disagree on 7 of the 72 configurations. One scene per process is how these
  were always scored - the gltfview baselines and `contrib/census/run_corpus.sh` both do it
  - and it is the only form in which the stored checksum means anything.

The x87 figures are from the 32-bit i386 softrend build, the configuration pentprim itself
was built in. softprim is portable and compiles for other targets: the `declared` keys are a
build that rounds every operation at its type's width, and they are softprim's own output,
because pentprim was never built for x86-64 - which is why a key names the class that
produced it.

### The restored cells and the mixed-walk keys

The z-buffered arbitrary-width 555/565 rows are pentprim's MMX-off answer, and the eight
single-cell fixtures reproduce it exactly: `scene-tex-rgb555`, its affine twin and the 565
pair, and `scene-shade-rgb555`, its flat twin and the 565 pair, at their own pixel type,
z-buffered. Those checksums are the retired tree's i686 build with `BRENDER_USE_MMX=0`, and
the 32-bit softprim build records the same values.

Fourteen further keys move because their fixture mixes a restored primitive with actors the
restoration does not claim. Six are `scene-shade-arb-flat`, `scene-shade-arb-flat-persp` and
`scene-shade-arb-smooth-persp`, at 15 and 16bpp z-buffered: in each frame the 555/565 cube
takes the restored shade-table row and the other two cubes keep the MMX untextured rows they
selected before. The other eight are the four 256x256 fixtures, `scene-shade-p256-flat-persp`,
`-smooth-persp`, `-flat-persp-blend` and `-smooth-persp-blend`, at 15 and 16bpp z-buffered:
the cube whose map and shade table match the output format now takes the restored exact
256x256 row (the witness names `...SHADE_*_I_RGB_TEX_I8_ADDR_SHIFT_PERSP_CORRECT...`), while
the two cubes of the other formats and the backdrop keep the MMX untextured rows. pentprim
never produced either mixed frame - it walked one table for a whole render, so its MMX-off
frame drew every actor from the general table - and there is no pentprim run to score these
fourteen against. They are softprim's own output, recorded from the same 32-bit build, and
they are the only x87 keys in the table that are not pentprim's. The witness logs name the
restored matches, and the frame differs from the old one only where the restored cube draws
(for `scene-shade-p256-flat-persp` at 15bpp, every 5-bit-depth difference between the old
MMX-on frame and the new one lies inside the union of the old and new silhouettes of the
restored cube).

The restored draw itself is scored against MMX-off pentprim separately, by isolated
single-cube probes outside the corpus: the eight perspective cells, and for the depth rules
a front/rear pair that distinguishes a zero texel (writes neither buffer), an opaque texel
(writes depth, occludes the rear), a 50/50 blend (tests depth, does not write it, so the rear
still passes) and an equal-depth second draw (admitted); an affine 256x256 map and a
non-256 power-of-two map reach the arbitrary-width row in both renderers. All of those
probes' checksums equal the oracle's, in both `fp-class`es.

### The size-specialised cells

The five `textureNxN` rows are witnessed by `scene-tex-rgb555-p32`, its affine twin,
`scene-tex-rgb565-p64`, `scene-tex-rgb555-p512` and `scene-tex-rgb565-64x32` - one size cell
per format at one perspective setting, plus both shapes no size row takes. Their `x87` keys
are pentprim's MMX-off output, checked against the i686 oracle's size sweep: all twenty
size-matrix cells (five sizes x two formats x two perspective settings), the two rectangular
fall-throughs and the two 512x512 ones reproduce its checksums exactly, in both
`fp-class`es. Their witnesses name the size row in the tuple
(`...TEX_555_ADDR_SHIFT...`), which is what separates them from the arbitrary-width
fall-through (`...ADDR_DIVIDE...`) in the same frame.

Two existing keys move with the removal of the `PRIMF_POWER2`-clear requirement from the
colour-map rows: `scene-mip` and `scene-mip-off` at 15bpp z-buffered, whose map is 32x8. A
power-of-two map of a size no row declares now reaches the arbitrary-width row, as it does
in MMX-off pentprim, instead of staying on the MMX untextured row; measured, the new frame
is MMX-off pentprim's (`256b45d7bd137b34`, the value its z-sorted twin already carried).

The 512x512 fall-through is the reason the size rows come first rather than the
arbitrary-width row being told to exclude power-of-two maps: pentprim's own precedence is
the size rows, and a 512 map belongs on the generic row. There is no `texture512x512` row in
`prm_t15.ifg`/`prm_t16.ifg` even though `infogen.pl` can emit one.

The fixture corpus scores the textured output. A zero texel leaving depth unchanged
and an equal-depth draw winning were checked with separate overlap probes; the
registered fixtures do not test those conditions directly.

### Known limitation: a Debug build is expected to differ

The frozen references are a **Release** artefact. The optimisation level changes how often
an intermediate value is spilled from the x87 register stack to memory, and a spilled value
rounds differently from the register, so a Debug (`-O0`) build of the same C stages computes
different last bits from the Release one. Measured: a Debug build differs from the stored
references by **one pixel in eleven of the then-96 fixtures at 24bpp** (both depth modes), by one
pixel in two fixtures at 8bpp and one at 16bpp (across the two depth modes), and by between
**38 and 570 pixels** in three more 8bpp fixtures, the `scene-shade-arb-*` cells, where a
sub-LSB intensity change moves a whole shade-index band. The difference is not softprim's
rasteriser: a Debug build of pentprim, whose rasteriser is fixed assembly and so cannot
change with `-O0`, diverges on exactly the same fixtures, and the two Debug builds agree
pixel for pixel - a rasteriser that cannot change diverging identically, and one that can
agreeing with it bit for bit, is itself evidence that softprim's rasteriser is not the source
of any difference. Those differences are not a regression: the keys were taken from a
Release build, and a Debug one is expected to differ.

### The honest limitation of the corpus

The fixture corpus has twice been blind to defects the nine reference scenes caught:

- the sub-LSB vertex-sort tie: it can only move a triangle whose SY pair ties as 16.16
  values but not as floats, and no fixture was moved by the fix;
- pentprim's 32x32 packing contradiction: only the nine scenes reached that block, and no
  fixture had a 32x32 map on the z-sorted perspective entry until `scene-tex-32` was added
  for it.

Neither condition is one a fixture happens to produce. The fixture corpus is the cheaper
instrument, and it is not a superset of the scenes - the nine are scored beside it now,
one process per scene, rather than only run for the states they build.

## Where the rest of it lives

`raster.cpp` explains the arithmetic. `infogen.pl` and `softprim_axes.h` define the
vocabulary, `CMakeLists.txt` selects the supported shapes, and `match.c` walks the matcher
table. `contrib/census` checks which entries that walk can select.
