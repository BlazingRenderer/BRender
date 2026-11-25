# Recipes

## Floating-point evaluation class

The reference key is
`<device>/<driver>/<fp-class>/<zb|zs>/<pixel-type>/<WxH>/<scene>`, and
`<fp-class>` is the width `float` and `double` expressions are evaluated at by
the build that produced the entry:

- `x87` — evaluated in x87 extended precision (`FLT_EVAL_METHOD` 2), as in
  the i686 GCC build with `-mfpmath=387`.
- `declared` — expressions are evaluated at their declared type's width
  (`FLT_EVAL_METHOD` 0), as in the x86-64 build.

Intermediate rounding can change pixels before the rasteriser sees the scene.
Rendertest derives the class from the build rather than choosing it at runtime.

## Running the corpus

```
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build -j
ctest --test-dir cmake-build --output-on-failure
```

Each configuration is its own test, so a failure names the configuration.
CI runs this for both classes: `brender-samples` (declared) and
`brender-samples-linux32` (x87).

A `fixtures` failure can come from a stale `mkres`: rebuild `mkres` and
`rendertest` in the directory under test before comparing its output.

`contrib/run-tests.sh [build-dir] [--no-build] [--history <range>]` is the
human-facing gate. It builds, runs CTest, and loads the checked-in glTF files.
`--history` also checks that every commit in the range carries the fixtures
its harness names.

## Checking a series, not only its tip

```
nix develop .#rendertest -c contrib/verify-series.sh upstream/master..HEAD
```

The script builds each commit from scratch in a separate worktree and runs the
current gate against that commit's fixtures and references. The `rendertest`
shell supplies the software GL and Vulkan runtime used by CI. A tip-only
build can miss a broken intermediate commit even if a history rewrite leaves
its final tree unchanged.

## Updating render references

1. Build the current tree in Release for the class you want to update. Use an
   i686 `x87` build and an x86-64 `declared` build to update both classes.
2. List the registered test commands with
   `ctest --test-dir <build-dir> --show-only=json-v1`. Run the affected corpus
   commands with their CTest working directories and environments, appending
   `--bless`. The GL commands use offscreen SDL and llvmpipe. On Linux, use
   the software runtime for the target architecture; the `rendertest` shell
   supplies it for native builds, and
   `nix eval --raw .#brender-samples-linux32.glRuntimeEnv` prints the exports
   for an i686 shell.
3. Keep the `refscenes-*` commands separate: each specifies its own scene under
   `resources/gltf-reference`. Rendering the scenes together can change the
   checksums. `--reference <file>` lets you bless a copy of the table first.
4. Diff `tools/rendertest/rendertest.txt` and verify that only the intended keys
   changed. For pixel changes, use `--ppm-dir <dir>` to inspect affected frames
   before accepting them. Rerun the registered tests **without** `--bless`;
   `NO-REFERENCE` means a frame was not compared, even if the test passed.

The software reference in both classes is softprim's output. Update the table
in the commit that introduces a scene or changes its pixels, and explain any
intentional pixel change there.
