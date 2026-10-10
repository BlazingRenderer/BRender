# Witness census

Which block does softprim's matcher select, which does nothing select, and which
can no state select at all? A kernel is not finished when it is implemented — it
is finished when something witnesses it — and this is the tool that says what is
witnessed.

`witness.py` joins the generated matcher list to per-scene selection logs, and
`reachable.py` decides which tuples *any* state could select, which separates
"I need a fixture" from "this is dead".

## The instrument

`drivers/softprim/match.c`, behind `BRENDER_SOFT_WITNESS_LOG`: with the option on
and `BR_WITNESS_LOG` naming a file, every matcher decision appends
`<index>\t<match|refused|nomatch>\t<identifier>`, where the index is the winning
entry's position in `softPrimBlocks` — the order of the generated
`softprim_matchers.inc` — so a line joins back to the emitted block.

It is off by default, and with the option off `spWitnessNote()` is empty, so the
matcher carries no conditional code either way. Build a tree for it:

```
cmake -S . -B cmake-build-witness -DCMAKE_BUILD_TYPE=Release \
      -DBRENDER_BUILD_SOFT=ON -DBRENDER_BUILD_EXAMPLES=ON \
      -DBRENDER_SOFT_WITNESS_LOG=ON
cmake --build cmake-build-witness -j
```

## Running it

```
contrib/census/run_corpus.sh cmake-build-witness /tmp/census
contrib/census/witness.py cmake-build-witness /tmp/census
```

840 runs: the fixtures the harness names (`--list`) and the nine
`resources/gltf-reference` scenes, at 4 bpp × {zb, zs}, one process per scene so
a log file is one scene's decisions. `witness.py --json out.json` for the
machine-readable form.

## Measured on 2026-10-10, at the 96 fixtures

```
matcher entries:      393
  emitted (kernel):   337  (221 distinct tuples)
  refused:            56  (40 distinct tuples)
matcher decisions:    162687   (match 162687, refused 0, nomatch 0)
witnessed tuples:     219  (99.1% of emitted)
unwitnessed tuples:   2
  of which: 2 unreachable under any state, 0 reachable
refused tuples:       40, of which 16 can be selected by some state
```

**There is no fixture gap on the emitted side.** Both unwitnessed tuples are
unreachable outright — `I8/TRI/{NONE,ZW}/NONE/NONE/NONE/NONE/AFFINE/NONE/NONE/NONE`,
an untextured, unshaded, unblended, unfogged triangle, captured by an earlier
entry for every state — so they are candidates for deletion, not for a fixture.
Softprim has no cache of the previous match, so every decision is walked and
logged; the decision count is the whole corpus, not a sample.

**The uncovered path is the refusals.** 16 tuples can be selected by some state
and the corpus selects none of them, so softprim's "this shape will not be drawn"
behaviour is defensive and untested. They are all 555/565:

- `555|565 / LINE|POINT / NONE|ZW / TEX_555|565 / DIVIDE` — a 555/565-typed map on
  a line or a point, at either depth mode;
- `555|565 / TRI / NONE / {CONST,INTERP}_I_RGB / I8 / SHIFT / CORRECT / {ALPHA,NONE}`
  — a `I8` texture with `SHIFT` addressing over an RGB shade table, perspective
  correct, no depth, blend `ALPHA` or `NONE`.

None of them is a wrong draw: the `.inc` entry exists because pentprim's tables
route those shapes to a neighbouring kernel, and drawing them there would be a
silent wrong draw — so softprim returns no block instead.

Both halves are port gaps, and both are reachable from a fixture:

- The **eight triangles** need the power-of-two perspective mapper for that
  family. Upstream 1.3.2 has it — `TriangleRenderPITIP256_RGB_555`,
  `drivers/pentprim/persp.c:133` — and softprim ported only the arbitrary-width
  (`DIVIDE`) variant, as `infogen.pl`'s comment says.
- The **eight line/point** shapes need the line and point kernels upstream has and
  softprim never took: `l_pi.c`, `p_pi.c`, `l_piz.c`, `p_piz.c` in
  `BRender-v1.3.2/drivers/pentprim/`.

The generator's comment gives the line/point half up with "no .glTF can reach a
line or a point anyway", and that is not so. A line or a point reaches the
rasteriser as a *draw-time* primitive: a mesh drawn under `BR_actors.render_style`
`BR_RSTYLE_POINTS` or `BR_RSTYLE_EDGES`, which the corpus already does in
`scene-lines-*`, `scene-points` and `scene-edges` — 1008 of the decisions above,
over 48 distinct line and point kernels and 12 of them textured, so the material's
map reaches the primitive library through that path intact. The render-style table
is the only thing that chooses the primitive type, handing
`BRT_POINT` or `BRT_LINE` for the same mesh (`core/v1db/modrend.c:31-50`).
What a `.gltf` cannot carry is *stored* line and point geometry — glTF modes 0..3,
which this reader refuses by design because BRender has no representation for it
(a face is three vertex indices and the model has no primitive type,
`core/inc/model.h:79`; `resources/aidocs/gltf-extensions.md`, *Hard failures*).
That distinction is the whole of it: the kernels these refusals stand in for are
reached by render style, and that path is one fixture away.

A 555/565-typed map is carried by the `brender=` marker on an image's data URI,
which the fixtures already use — `scene-tex-rgb555` is a 555-typed map on the one
shape of that family softprim does implement — so a 555-typed map on a line is a
fixture nobody has written rather than a shape no fixture can express.

What a caller does with a refusal is measured: on the scanline path it aborts the
rest of that model's groups and the error is dropped, `drivers/softrend/v1model.c:783`
returning it into `core/v1db/modrend.c:11`, which ignores it; on the z-sort path it
skips the one primitive. Either way there is no diagnostic, which is the case for
making a refusal fail loudly rather than truncate a model in silence.

## What the tool cannot see

- The nine `resources/gltf-reference` scenes have no entries in any reference
  table, so their runs report NO-REFERENCE and `result=FAIL`. They are dumps from
  Croc kept because they have historically tripped the lighting, so they widen
  the *states* the corpus builds even though they cannot be compared. Read their
  logs and ignore the verdict.
- `witness.py` refuses to report a census of empty logs. A build without the
  instrument produces one empty file per scene, which would otherwise read as
  "nothing is witnessed" — a wrong answer wearing the shape of a finding.

## Where this came from

This is the second round of the census written on `dev/pentprim-nondet`, reduced
to the half that runs here. That branch, and `dev/pentprim-verification`, carry
the pentprim side: which of *pentprim's* rasterisers the `x87` reference pins,
which is the same question asked of the oracle rather than of the shipping
renderer.
