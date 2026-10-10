# The pentprim oracle

This branch is the tree as it was before the portable rasteriser replaced
pentprim, kept so that pentprim can still be **asked questions by hand** — what
does it do with this scene, which kernel handles this primitive, what pixels does
it produce — when softprim does not yet answer the same way.

It is not a CI gate and nothing automated depends on it. Cherry-picking more of
the harness forward when you need it is expected to be manual.

## What is on it

`076e6db6` plus two commits:

- **`examples/{brdemo,rendertest}: let pentprim bless the current x87 reference`**
  — three files, each carrying one thing the oracle depends on: the primitive
  heap size (`brdemo.h`), the registered driver name (`rendertest/CMakeLists.txt`),
  and the floating-point class in the reference key (`rendertest/main.c`). This
  is a snapshot of the harness as it stood when the `x87` half of the reference
  was regenerated, not a copy that tracks master.
- **`drivers/pentprim: log every block selection for a census`** — the
  instrument. With `BR_WITNESS_LOG` naming a file, every selection appends
  `<table>\t<index>\t<match|default>\t<identifier>`; inert with it unset.

`nix/uasm` and `nix/h2inc.nix` are on this branch, so the assembler toolchain is
reproducible without hunting for store paths. Configure as
`resources/aidocs/recipes.md` says.

## Before you trust an answer

Rebuild, then check that the tree still reproduces the checked-in `x87` entries.
If it does not, the harness on this branch has drifted and its answers are about
a different scene than master's.

```
M=/path/to/CrocDE-BRender      # the master checkout
cmake --build <build> -j
for bpp in 8 15 16 24; do for mode in zb zs; do
    d=; [ "$mode" = zs ] && d=--no-depth
    printf '%-3s %-3s ' "$bpp" "$mode"
    SDL_VIDEODRIVER=offscreen <build>/examples/rendertest/rendertest \
        --device softrend --bpp "$bpp" $d \
        --reference "$M/examples/rendertest/rendertest.txt" \
        --scene-dir "$M/examples/rendertest/dat" \
        2>&1 | grep -oE 'result=(PASS|FAIL) failures=[0-9]+' | tail -1
done; done
```

Expect eight `result=PASS failures=0`. That is 768 `x87` entries, and it is what
the tree looked like when this was written.

To see pixels rather than a checksum, add `--ppm-dir <dir>`; it writes one PPM
per fixture.

## The witness census

Which of pentprim's rasterisers does the reference actually pin? An entry no
fixture selects is a kernel the oracle does not test, so its softprim
counterpart is unverified by it.

```
contrib/census/run_corpus.sh <build> <out>        # 840 runs, one process per scene
contrib/census/witness.py <build> <out>           # what was selected, what was not
contrib/census/witness.py <build> <out> --mmx     # per rasteriser symbol
contrib/census/witness.py <build> <out> --scenes  # per scene
```

`run_corpus.sh` defaults to this tree's own fixtures and reference; set
`CENSUS_DAT` and `CENSUS_REFERENCE` to master's when the run has to be comparable
with the current reference (the tree's reference keys predate the floating-point
class field, so they compare nothing).

Measured on 2026-10-10, at the 96 fixtures: 7424 selections, 235 distinct
entries, of 393 entries in the fourteen tables. 158 have no witness. The MMX
tables are fully witnessed (32 of 32 each); `prim_l15`, `prim_p15`, `prim_l16`
and `prim_p16` each leave two, both the textured shapes; `prim_t15` and
`prim_t16` leave 26 of 34; `prim_t8` leaves 98 of 189.

Eight of those 26 are only unwitnessed *on a build with MMX*. Re-run with
`BRENDER_USE_MMX=FALSE` and they are selected: the MMX tables are tried first,
so they shadow entries a machine without MMX would reach and the reference build
never does. The remaining 18 in each of `prim_t15` and `prim_t16`, and all 98 in
`prim_t8`, no configuration selects either way.

`BRENDER_USE_MMX` is a system-config entry read from the environment
(`core/fw/sys_conf.c:64` into `core/host/hstsetup.c:97`), so no rebuild is
needed. Nothing changes at 8 or 24 bpp, where the MMX row shares the general
arrays: 420 logs byte-identical.

`witness.py` checks the join as it goes: every logged index has to name the entry
the generated table has at that index, and every `(table, index)` has to exist.
It says so loudly rather than reporting a census built on the wrong rows — which
is how the table-name mapping was found to be wrong when this was written.

## Which entries no state can select

An unwitnessed entry is either one no fixture happens to hit, or one no
primitive state can hit at all. `reachable.py` decides the second without a
renderer, by running the walk `match_block()` performs over the finite state
space its comparisons can distinguish, once per (format, topology) and once for
each of the two MMX configurations:

```
contrib/census/reachable.py <build>                       # dead per configuration
contrib/census/reachable.py <build> --census <out>        # + selected / gap / dead
```

It reads the same generated tables `witness.py` joins against
(`witness.parse_tables`) and, with `--census`, the same selection logs. The
one caveat worth remembering is that it enumerates every assignment of the
`PRIMF_*` tokens the entries name, while the engine derives several of those
bits from the bound buffers; the flag space is therefore a superset, which makes
"dead" sound and "selectable" an over-count. See the module docstring for the
rest of the model and for what it deliberately cannot see.

The nine `resources/gltf-reference` scenes are dumps from Croc, kept because
they have historically tripped the lighting, so it can be tested without
starting the game. They are a coverage corpus rather than a reference one: no
table holds entries for them, in this tree or in master's, so their runs always
report NO-REFERENCE and `result=FAIL`. Read their logs and ignore the verdict.
The fixture corpus is the one that compares, and it passes at all eight
configurations.

## What is deliberately not here

Round 1 of the census compared **which block pentprim selects** against which one
refprim selects, and half of that tooling needs a second build of a tree with
`drivers/refprim` in it. That is `dev/pentprim-nondet`, where the whole tool is
tracked, and where the findings documents are. Nothing is lost by its absence
here: this branch answers pentprim questions, and comparing two matchers is not
one of them.

## The instrument's blind spots

Both are deliberate and both are in the commit message:

- A match the cache at the top of `renderBegin()` still holds for the same
  timestamps is not re-walked, so a run records distinct selections rather than
  primitives.
- The fall-through is logged as `default`, but nothing in the current corpus
  reaches it: every pixel type the harness can key has a real entry that matches.

## The line and point fixtures

`scene-lines-rgb555` and `scene-lines-rgb565` (master's
`examples/rendertest/dat`) carry an edge actor and a point actor over one
material, so each witnesses both topologies. At `--bpp 15`/`--bpp 16` they select
the 555/565-typed line and point kernels - `prim_l15#0`, `#3`, `prim_l16#0`,
`#3`, `prim_p15#0`, `#3`, `prim_p16#0`, `#3` - which are the eight entries
`reachable.py` reports as selectable but selected by no corpus scored here.

Point `CENSUS_DAT` at master's `examples/rendertest/dat` and name the two
scenes, one process per scene; nothing else in the tree reaches those cells
except `scene-points` and the `scene-lines-*` family at their own output type.
