# softprim

The software rasteriser: a portable C/C++ port of the x86 assembly rasteriser that was
`drivers/pentprim/`, written to reproduce pentprim's output byte-for-byte rather than to be
a new renderer. 8,893 lines of C and C++, plus pentprim's `.ifg` block descriptions.

## Why it replaced pentprim

pentprim was hand-written 1990s x86 assembly. Building it needed a 32-bit x86 target, a
MASM-compatible assembler (`uasm`, or MSVC's `ml.exe` on Windows), Perl, and a generated
header (`drv.h` -> `drv.inc`, via `h2inc`, or `H2INC.EXE` under Wine before that).

softprim is portable C/C++ and needs none of it: `-DBRENDER_BUILD_SOFT=ON` builds it on any
architecture. It presents the same entry point (`BrDrv1SoftPrimBegin`), device and
primitive-library identifiers (`SOFTPRMF`, `Default-Primitives-Float`) and install name
(`softprmf`) pentprim did, so nothing outside `drivers/softprim/` changed. Frames are keyed
by the `softprmf` driver token, so a run scores directly against the checked-in references.

pentprim's pixels are the oracle, and they are frozen: `examples/rendertest/rendertest.txt`
holds the `softrend/softprmf/x87` keys - fixtures and reference scenes alike - and with
pentprim gone they are the only record of its output.

## The block table, and what a kernel is allowed to be

- `SP_TABLES` (`CMakeLists.txt`) names the 14 `.ifg` block descriptions in `tables/`.
  These are pentprim's own, carried here because they are the specification both
  rasterisers implement; the generator `infogen.pl` is theirs too.
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
examples/rendertest/rendertest.txt` when the bless runs from another tree, whose own
default reference is not this one.

## What is implemented

All of pentprim's 393 live blocks are in the matcher's walk; 353 are emitted as blocks with
a kernel (237 distinct tuples after the collapse) and 40 are refused, every one of them a
z-buffered 555/565 shape. Every `INDEX_8` and every `RGB_888` shape is implemented. The
implemented set includes the `INDEX_8` ROPs (indexed blend, fog, decal, dithered map), the
shade-table family including the 256x256 RGB cells, the MMX 15/16bpp family including
screendoor and colour dither and its packed 20.12 texture addressing, the arbitrary-width
and perfect-scan RGB paths, and lines and points.

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

40 of the 393 entries (24 distinct tuples). Every one of them is refused rather than
omitted, and the first thing to establish about any of them is whether the matcher can reach
it at all - which the generated table answers on its own, since
`contrib/census/reachable.py` walks it the way `spFindMatch` does and stops on a refused
entry. `contrib/census/witness.py` then says whether *any* state can select the rest. The
answer now is none: **all 40 are unreachable in softprim's walk**, so there is no reachable
shape without a kernel and no shape a fixture could ask for that softprim declines to draw.
The rule is `infogen.pl`'s `softprim_implemented()`, which carries its reasoning in comments,
plus the guards in `raster.cpp`.

**Unreachable in softprim: every z-buffered 555/565 shape, in both families (40 entries).**

- the 555/565-typed colour maps (`SP_TEX_555`/`SP_TEX_565`): 12 entries per format, all
  z-buffered.
- the RGB-output shade-table shapes at 15/16bpp (`SP_SHADE_CONST_I_RGB`/
  `SP_SHADE_INTERP_I_RGB`): 8 entries per format, all z-buffered.

The mechanism is one thing: the MMX table is first in softprim's walk for a 555/565 output,
its textured rows all require an `INDEX_8` map with a palette, and one of its untextured rows
then takes the primitive - so a 555/565 map is never sampled and the primitive is drawn
untextured instead. That is measured, not assumed: the corpus's own `scene-tex-rgb555` and
`scene-tex-rgb565` fixtures draw untextured at 15/16bpp z-buffered.

That is a property of softprim, not of pentprim. pentprim walks the MMX table only while MMX
is in use (`BRENDER_USE_MMX`, or the CPU's own capability); with it off the general table is
what answers, and those kernels are neither unreachable nor equivalent to their MMX twins -
measured on `dev/pentprim-verification` with `BRENDER_USE_MMX=0`, 70 of its 96 fixtures
change at each of 15bpp and 16bpp z-buffered, `scene-flat` alone by 211 of 76800 pixels, and
the census there counts 8 entries of `prim_t15` and 8 of `prim_t16` that only the general
configuration selects. **The reference is the MMX configuration** - the one the frozen
`x87` entries record - and softprim reproduces it on every architecture; it has no MMX
toggle and does not reproduce the general path.


## How it is verified

Both instruments score softprim against **pentprim's frozen pixels**, never against its own
output.

- **The fixture corpus.** `examples/rendertest` renders 102 fixtures at four pixel formats
  (8/15/16/24bpp) x {z-buffered, z-sorted} and compares each against
  `examples/rendertest/rendertest.txt`, whose 888 `softrend/softprmf/x87` keys are
  pentprim's. Measured: `PASS failures=0`, 102 comparisons, in all eight configurations.
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

The working record is in `scratch/` (mostly untracked, so not a lasting reference):
`witness-gaps.md` and its runner/analyser in `scratch/census/` for which shapes are
witnessed, unreachable or simply refused; `witness-census.md` for the previous round of the
same (and a snapshot of an older tree); `mmx-port-plan.md` for the MMX family;
`raster-buffer-order.md` for pentprim's rasteriser buffer and draw order; and
`deletion-gate.md` for the dependency set that pentprim's removal had to satisfy.

The design rationale that will outlive `scratch/` is in the sources, deliberately:
`raster.cpp`'s file and kernel comments (the x87 setup, the packed U/V, the signed
multiply, the per-block colour scale), `infogen.pl`'s `softprim_implemented()` and
`softprim_tuple()`, `softprim_axes.h` (what an axis is and is not), `match.c` (the walk and
the metadata it tests) and `CMakeLists.txt` (the table and spec).
