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
by the `software` driver token, so a run scores directly against the checked-in references.

pentprim's pixels are the oracle, and they are frozen: `examples/rendertest/rendertest.txt`
holds the 768 `softrend/software/...` keys and `scratch/gltfview-baseline{,-zs}.txt` the
reference scenes, and with pentprim gone they are the only record of its output.

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

## What is implemented

All of pentprim's 393 live blocks are in the matcher's walk; 337 are emitted as blocks with
a kernel (221 distinct kernels after the tuple collapse) and 56 are refused. Every
`INDEX_8` and every `RGB_888` shape is implemented; both refusal families are entirely
555/565. The implemented set includes the `INDEX_8` ROPs (indexed blend, fog, decal,
dithered map), the shade-table family, the MMX 15/16bpp family including screendoor and
colour dither and its packed 20.12 texture addressing, the arbitrary-width and perfect-scan
RGB paths, and lines and points.

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

56 of the 393 entries (40 distinct tuples), in two families. Every one of them is refused
rather than omitted, and the first thing to establish about any of them is whether the
matcher can reach it at all - which the generated table answers on its own, since
`scratch/census/reachable2.py` walks it the way `spFindMatch` does and stops on a refused
entry. The split is **40 unreachable, 16 reachable but with no kernel**. The rule is
`infogen.pl`'s `softprim_implemented()`, which carries its reasoning in comments, plus the
guards in `raster.cpp`.

**Unreachable: every z-buffered 555/565 shape, in both families (40 entries).**

- the 555/565-typed colour maps (`SP_TEX_555`/`SP_TEX_565`): 12 entries per format, all
  z-buffered.
- the RGB-output shade-table shapes at 15/16bpp (`SP_SHADE_CONST_I_RGB`/
  `SP_SHADE_INTERP_I_RGB`): 8 entries per format, all z-buffered.

The mechanism is one thing: the MMX table is tried first for a 555/565 output, its textured
rows all require an `INDEX_8` map with a palette, and one of its untextured rows then takes
the primitive - so a 555/565 map is never sampled and the primitive is drawn untextured
instead. That is measured, not assumed: the corpus's own `scene-tex-rgb555` and
`scene-tex-rgb565` fixtures draw untextured at 15/16bpp z-buffered. pentprim's table has the
same order and the same MMX-first rule, so those kernels do not run there either: refusing
them costs nothing that can be observed.

**Reachable, but no kernel exists (16 entries).**

- the 555/565-typed line and point shapes, 8 entries. The line/point family here is
  arbitrary-width only, so they would need kernels of their own. A fixture can ask for a
  line or a point - the topology comes from `BR_actors.render_style`, not from glTF's
  `mode`, and `scene-lines-*` and `scene-points` do reach these tables - but none pairs an
  edges or points actor with a 555/565-typed colour map, which is what these entries
  require.
- the z-sorted power-of-two RGB shade-table shapes, 8 entries. These are `perspi.h`'s
  separate perspective mapper, which packs its base texel from a compiled-in width. Four of
  the eight also carry `SP_BLEND_ALPHA`, the 50/50 blend against the destination, and are
  refused because that axis value is not named in `SP_SPEC` at all - the blend has no
  kernel.

So the reasons are four kinds of thing: unreachable under the matcher's own rules, the 40;
no kernel for the line/point family, the 8; no kernel for the power-of-two shade-table
mapper, the 4; and no kernel for the 50/50 destination blend, the other 4. Only the last two
groups are a gap a reader might expect to see filled one day - a shape that could be given a
kernel and witnessed - and the `blendrgb` half of that needs an axis value in `SP_SPEC`
first.

## How it is verified

Both instruments score softprim against **pentprim's frozen pixels**, never against its own
output.

- **The fixture corpus.** `examples/rendertest` renders 96 fixtures at four pixel formats
  (8/15/16/24bpp) x {z-buffered, z-sorted} and compares each against
  `examples/rendertest/rendertest.txt`, whose 768 `softrend/software/...` keys are
  pentprim's. Measured: `PASS failures=0`, 96 comparisons, in all eight configurations.
- **Nine reference scenes.** `resources/gltf-reference`, rendered with
  `gltfview --force-software` (`GLTFVIEW_ZSORT=1` selects the z-sorted set) and scored
  against `scratch/gltfview-baseline.txt` (z-buffered) and `scratch/gltfview-baseline-zs.txt`
  (z-sorted): 9 scenes x 8/15/16/24bpp x both modes. Measured: all 72 reproduce pentprim's
  frozen checksums exactly, byte-for-byte.

Both figures are from the 32-bit i386 softrend build, the configuration pentprim itself was
built in. softprim is portable and compiles for other targets, but the pixel claims are
measured on i386 only: the x87 register stack rounds differently from SSE, so another target
is expected to move last bits and would need references of its own.

### Known limitation: a Debug build is expected to differ

The frozen references are a **Release** artefact. The optimisation level changes how often
an intermediate value is spilled from the x87 register stack to memory, and a spilled value
rounds differently from the register, so a Debug (`-O0`) build of the same C stages computes
different last bits from the Release one. Measured: a Debug build differs from the stored
references by **one pixel in eleven of the 96 fixtures at 24bpp** (both depth modes), by one
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

Neither condition is one a fixture happens to produce. The corpus is the cheaper instrument
and the one that runs by default; it is not a superset of the scenes.

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
