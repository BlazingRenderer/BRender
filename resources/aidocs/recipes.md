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
the software ones. Rendertest derives its own class (`examples/rendertest/main.c`);
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
a fixture. It is the only check that looks at anything other than the working
tree, and it catches a commit that named a fixture it did not have — invisible
at the tip, because a tip can be self-consistent while an intermediate commit is
not.

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
harness, so the harness has to be brought forward onto that tree: its key
format and the demo's primitive heap are both newer.

1. A worktree at the pre-softprim commit, and the current harness copied in:

   ```
   git worktree add --detach ../brender-pent 076e6db6
   for f in examples/rendertest/main.c examples/rendertest/CMakeLists.txt \
            examples/brdemo/brdemo.h; do
       cp "$f" "../brender-pent/$f"
   done
   ```

   (`brdemo.h` matters because the heap size is a demo constant that the frames
   depend on once it is too small to hold a scene — see below.)

2. Build it with the assembler toolchain. `nix/uasm` and `nix/h2inc.nix` at
   `2e8b44e0^` are the derivations; `h2inc` is a public fetch from GitHub. A
   shell with `uasm` and `h2inc` on `PATH` is enough — the store paths are not
   worth recording because they are garbage-collected once nothing references
   them:

   ```
   nix develop .#brender-samples-linux32          # i686, to match pentprim
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
         -DBRENDER_BUILD_SOFT=ON -DBRENDER_SOFT_REFPRIM=OFF \
         -DBRENDER_H2INC_EXECUTABLE="$(command -v h2inc)" \
         -DCMAKE_ASM_MASM_COMPILER="$(command -v uasm)"
   cmake --build build -j
   ```

3. Bless against the *current* fixtures and the *current* table, so the
   worktree's own copies are not used. `--bless` merges: entries already
   present keep their position and only the covered keys are rewritten, so the
   order these run in does not matter.

   ```
   MAIN=/path/to/the/checkout
   for cfg in "8 zb" "8 zs" "15 zb" "15 zs" "16 zb" "16 zs" "24 zb" "24 zs"; do
       set -- $cfg; [ "$2" = zs ] && d=--no-depth || d=
       build/examples/rendertest/rendertest --device softrend --bpp "$1" $d \
           --scene-dir "$MAIN/examples/rendertest/dat" \
           --reference "$MAIN/examples/rendertest/rendertest.txt" --bless
   done
   SDL_VIDEODRIVER=offscreen LIBGL_ALWAYS_SOFTWARE=true \
       build/examples/rendertest/rendertest --device glrend --bpp 8 \
           --scene-dir "$MAIN/examples/rendertest/dat" \
           --reference "$MAIN/examples/rendertest/rendertest.txt" --bless
   ```

4. Verify by scoring the current tree's x87 build against the new table; it
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
