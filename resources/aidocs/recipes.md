# Recipes

Procedures that are not obvious from the code. Each was run; where a step is
order-dependent, it says so.

## Floating-point evaluation class

The reference key is
`<device>/<driver>/<fp-class>/<zb|zs>/<pixel-type>/<WxH>/<scene>`, and
`<fp-class>` is the width `float` and `double` expressions are evaluated at by
the build that produced the entry:

- `x87` — evaluated in 80-bit x87 registers and rounded only on a store
  (`FLT_EVAL_METHOD` 2): i686 with GCC's default `-mfpmath=387`, MSVC's 32-bit
  target, the DOS build.
- `declared` — each operation rounds at the width its type declares
  (`FLT_EVAL_METHOD` 0): x86-64, aarch64, and a 32-bit build with
  `-mfpmath=sse`.

Both are IEEE-754; what differs is the evaluation width, and computing in 80
bits then rounding once lands on a different `float` than rounding at every
step. That difference reaches the frame through softrend's setup *and* through
core's scene setup, so softprim and glrend alike produce different pixels from
the same scene — which is why every entry carries the class rather than only
the software ones. Rendertest derives its own class (`tools/rendertest/main.c`);
nothing chooses it by hand.

The `x87` half is pentprim's output, and it is the bit-exactness oracle for the
portable rasteriser that replaced it. **Only an x87 build can reproduce it.**
`-mfpmath=387` on x86-64 gets close but not exact, because the x86-64 ABI rounds
`float` and `double` at call boundaries and no flag undoes that.

## Running the corpus

```
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j
ctest --test-dir cmake-build --output-on-failure
```

Each configuration is its own test, so a failure names the configuration.
CI runs this for both classes: `brender-samples` (declared) and
`brender-samples-linux32` (x87).

A `fixtures` failure is almost always a stale `mkres` binary rather than a
divergent generator: rebuild the `mkres` target — and `rendertest` — in every
build directory in use before believing it. Each build generates its own `mkres`
from `tools/mkres/scene.c`, and one left behind by an earlier commit keeps
writing the fixtures that commit generated.

`contrib/run-tests.sh [build-dir] [--no-build] [--history <range>]` is the
human-facing gate. It runs the checks that do not belong in a build — the build
step itself, the `--history` walk below, and a load of every checked-in
`.gltf`. `--bless` is never a test argument: accepting current output is a
deliberate act.

## Checking a series, not only its tip

```
contrib/run-tests.sh cmake-build --no-build --history upstream/master..HEAD
```

At every commit in the range, this checks that each scene the harness names has
a fixture. It is the only check in `run-tests.sh` that looks at anything other
than the working tree, and it catches a commit that named a fixture it did not
have — invisible at the tip, because a tip can be self-consistent while an
intermediate commit is not.

`contrib/verify-series.sh <range>` is the heavier check: it builds every commit
in a range in its own worktree with a fresh configure, then runs the full gate
against it, so it also catches a commit that does not compile or that moves
pixels without re-blessing the reference. It has to be run inside
`nix develop .#brender-samples`: outside that shell the freshly-configured
builds do not match the reference.

The same blindness applies to any history rewrite. A reorder or a squash can
leave the tip tree correct while an intermediate commit is wrong, so verify by
scanning a value across *every* commit and not only by diffing trees:
`git diff <old-tip> <new-tip>` being empty is not sufficient. Squashing two
commits that rename a token leaves the later one reverting it unless its tree is
rebuilt as well.

## Regenerating the x87 half of the reference

The `x87` entries come from pentprim, which is in the tree up to `076e6db6`
("resources/aidocs: specify this fork's glTF extensions") — the commit before
the portable rasteriser replaced it. They are blessed with the *current*
harness, so the harness on that tree has to be brought forward: its key format
and the demo's primitive heap are both newer.

That tree is kept as the `dev/pentprim-verification` branch — the oracle. It is
built beside the tree it blesses, never merged into it, and its corpus, harness
and reference are brought forward from the blessed tree before each run. A
softrend change is carried onto it by cherry-pick, not by that sync.

1. Create the oracle worktree. The path is yours to choose; it is `<oracle>`
   here:

   ```
   git worktree add <oracle> dev/pentprim-verification
   ```

   (`brdemo.h` matters because the heap size is a demo constant that the frames
   depend on once it is too small to hold a scene — see below. The branch's
   `contrib/census/README.md` has the command that checks this worktree still
   reproduces the checked-in `x87` entries, which is worth running before
   trusting anything it produces.)

2. Bring its corpus, harness and reference forward from the ref you are
   blessing. `contrib/sync-oracle.sh --from <ref>` checks out that ref's
   `tools/rendertest`, `examples/brdemo`, `tools/mkres` and
   `resources/gltf-reference` into the worktree and commits them as
   `meta: sync oracle from <sha>`; it refuses a worktree that is dirty outside
   those paths. It finds the worktree on the branch or the sibling `<repo>-pent`
   one, and `--worktree DIR` names it otherwise. It deliberately does not sync
   `drivers/softrend`: carry a softrend change that moves pixels by
   cherry-picking its commit onto the worktree, which stays clean because the
   change is a commit of its own. Reset the worktree before the next sync.

3. Build it with the assembler toolchain. `nix/uasm` and `nix/h2inc.nix` are on
   that branch, and were dropped from master in `2e8b44e0`; `h2inc` is a public
   fetch from GitHub. A shell with `uasm` and `h2inc` on `PATH` is enough — the
   store paths are not worth recording because they are garbage-collected once
   nothing references them. Each worktree builds in its own directory; the
   oracle's is `cmake-build-32-pent2`:

   ```
   nix develop .#brender-samples-linux32          # i686, to match pentprim
   cmake -S . -B cmake-build-32-pent2 -DCMAKE_BUILD_TYPE=Release \
         -DBRENDER_BUILD_SOFT=ON \
         -DBRENDER_H2INC_EXECUTABLE="$(command -v h2inc)" \
         -DCMAKE_ASM_MASM_COMPILER="$(command -v uasm)"
   cmake --build cmake-build-32-pent2 --target rendertest -j
   ```

4. Bless the **softrend** `x87` half against the *current* fixtures and the
   *current* table, so the worktree's own copies are not used. Run this from the
   tree being blessed, with `<oracle>` the worktree from step 1. `--bless` only
   writes the class the build derives — `x87` from this i686 oracle, `declared`
   from an x86-64 build — and it merges: entries already present keep their
   position and only the covered keys are rewritten, so the order these run in
   does not matter.

   ```
   oracle=<the oracle worktree>
   for cfg in "8 zb" "8 zs" "15 zb" "15 zs" "16 zb" "16 zs" "24 zb" "24 zs"; do
       set -- $cfg; [ "$2" = zs ] && d=--no-depth || d=
       "$oracle/cmake-build-32-pent2/tools/rendertest/rendertest" \
           --device softrend --bpp "$1" $d \
           --scene-dir tools/rendertest/dat \
           --reference tools/rendertest/rendertest.txt --bless
   done
   ```

   **The nine reference scenes under `resources/gltf-reference` are blessed
   separately.** They are a second corpus — a different `--scene-dir` and an
   explicit scene list — and `contrib/run-tests.sh --bless` skips them by
   design, because it rebuilds each configuration from its ctest name and a name
   cannot carry either. Bless them one scene per process: a frame there is not a
   function of its scene alone but of the scenes that ran before it, so the nine
   in one process do not reproduce the stored checksums. Point `--scene-dir` at
   `resources/gltf-reference` and add `--reference` when the bless runs from
   another tree; `drivers/softprim/README.md` has the loop.

   **The GL devices are not blessed here.** This worktree's `glrend` does not
   reproduce the `scene-light-volume{,-cut}` entries — it renders both identical
   to `scene-light-volume-off` — so blessing it here rewrites those keys to the
   `-off` value; and `glrend1x` does not involve pentprim at all. Bless their
   `x87` halves from the fork's own i686 build instead (`cmake-build-32`), which
   reproduces them; their `declared` halves come from an x86-64 build as usual.
   Bless both depth modes — glrend's `zb` and `zs` differ — and note that
   glrend's two fp classes agree (its T&L is in the shader) while glrend1x's do
   not (its T&L is softrend's):

   ```
   for cfg in zb zs; do
       [ "$cfg" = zs ] && d=--no-depth || d=
       for dev in glrend glrend1x; do
           SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=true \
               cmake-build-32/tools/rendertest/rendertest --device "$dev" $d \
                   --scene-dir tools/rendertest/dat \
                   --reference tools/rendertest/rendertest.txt --bless
       done
   done
   ```

5. Dump a frame and look at it before accepting a bless. A bless records
   whatever the renderer just produced, so render the changed scene with
   `--ppm-dir <dir>` (or `GLTFVIEW_BENCH_SOFT_PNG=<file>` through `gltfview`)
   and open the image. A black render has been blessed into the reference and
   survived a full gate; a green run is not evidence the pixels are right.

6. Verify by scoring the current tree's x87 build against the new table; it
   should be 100%. Confirm the checksums of entries you did not expect to move
   are unchanged — `git diff` on the table is the check.

### The primitive heap is part of the oracle

`brdemo`'s primitive heap is a fixed 8192 KiB, and the reference was blessed
with whatever fit in it. It was 1500 KiB until recently, which is smaller than
the z-sort corpus needs (it peaks near 4.97 MiB, and x86-64's larger
`br_primitive` needs more bytes for the same primitives), so primitives were
being dropped — and how many dropped depended on the word size, which made
64-bit frames differ from the reference for reasons that had nothing to do with
the rasteriser. If a scene grows past the heap, the overflow is reported once at
the point of failure (`drivers/softrend/heap.c`) rather than silently dropping
geometry, and the fixtures and reference have to be regenerated together.
