#!/usr/bin/env bash
#
# Run every check this project has.
#
# There is no unit-test suite and none is wanted; the checks are:
#   1. the tree builds
#   2. `mkres scenes` reproduces examples/rendertest/dat/ byte-for-byte
#   3. the render regression corpus passes at every device/bpp/depth
#   4. every checked-in .gltf loads
#   5. every scene the harness names has a fixture, and with --history, that
#      this held at every commit in a range
#
# Nothing here writes to the repository. The fixture check runs in a
# temporary directory; rendertest compares unless --bless is passed.
#
# Usage:
#   contrib/run-tests.sh [build-dir] [--bless] [--no-build]
#                        [--history <range>]
#
#   build-dir   defaults to cmake-build.
#   --bless     rewrite the reference table instead of comparing. Only for
#               accepting a deliberate change; see the note at the end.
#   --no-build  skip the build step (for re-running after no source change).
#   --history <range>
#               also check the scene/fixture invariant at every commit in
#               <range>, e.g. upstream/master..HEAD. Slower, and the only way
#               to catch a commit that named a fixture it did not have.

set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
build="cmake-build"
bless=0
do_build=1
history=""

while [ $# -gt 0 ]; do
    case "$1" in
        --bless)            bless=1 ;;
        --no-build)         do_build=0 ;;
        --history)          history="${2:-}"; shift ;;
        -h|--help)          awk 'NR>1 && /^#/ {sub(/^# ?/, ""); print; next} NR>1 {exit}' "$0"; exit 0 ;;
        -*)                 echo "unknown option: $1" >&2; exit 2 ;;
        *)                  build="$1" ;;
    esac
    shift
done

if [ -n "$history" ] && ! git rev-parse --git-dir >/dev/null 2>&1; then
    echo "--history needs a git repository" >&2; exit 2
fi

cd "$repo" || exit 1

pass=0
fail=0
skipped=0

ok()   { printf '  \033[32mPASS\033[0m  %s\n' "$1"; pass=$((pass + 1)); }
bad()  { printf '  \033[31mFAIL\033[0m  %s\n' "$1"; fail=$((fail + 1)); }
skip() { printf '  \033[33mSKIP\033[0m  %s\n' "$1"; skipped=$((skipped + 1)); }
hdr()  { printf '\n== %s\n' "$1"; }

# ---------------------------------------------------------------- build

hdr "build ($build)"

if [ "$do_build" = 0 ]; then
    skip "build (--no-build)"
elif [ ! -d "$build" ]; then
    bad "$build does not exist - configure it first, see the note at the end of this script"
else
    if cmake --build "$build" -j"$(nproc)" >/tmp/run-tests-build.log 2>&1; then
        # A build that fails silently leaves the old binary in place. The
        # "Built target" line this used to grep for is a Makefile-generator
        # message and Ninja says something else, so ask for the target by
        # name instead and trust that exit status, which means the same thing
        # under either generator.
        if cmake --build "$build" --target rendertest >/tmp/run-tests-target.log 2>&1; then
            ok "built (log: /tmp/run-tests-build.log)"
        else
            bad "cmake reported success but could not build rendertest - old binary in place?"
        fi
    else
        bad "build failed (log: /tmp/run-tests-build.log)"
    fi
fi

# ------------------------------------------------- scene/fixture invariant

# Every scene the harness names must exist as a fixture. A name that does
# not resolve is a dangling reference: rendertest reports NO-REFERENCE for
# it, so the run looks merely unverified rather than broken.
#
# This is also worth checking across history, which no working-tree check
# can do. Twelve commits in the history of this repository named scenes
# they did not have, all invisible at the tip because the tip was
# consistent - a bisect through one of them silently skipped a scene.

hdr "references: every scene the harness names has a fixture"

rt_main=examples/rendertest/main.c
rt_dat=examples/rendertest/dat

# Empty argument = the working tree; otherwise a revision. Prints nothing
# when consistent, and one name per line when not.
read_main() {
    if [ -n "$1" ]; then git show "$1:$rt_main" 2>/dev/null; else cat "$rt_main" 2>/dev/null; fi
}
read_dat() {
    if [ -n "$1" ]; then git ls-tree --name-only "$1" "$rt_dat/" 2>/dev/null; else ls "$rt_dat" 2>/dev/null; fi
}
dangling() { # $1 = revision, or empty for the working tree
    names=$(read_main "$1" | grep -oE '"scene-[a-z0-9_-]+"' | tr -d '"' | sort -u)
    present=$(read_dat "$1" | sed 's|.*/||;s|\.gltf$||' | sort -u)
    for x in $names; do
        printf '%s\n' "$present" | grep -qx "$x" || echo "$x"
    done
}

missing=$(dangling "")
if [ -z "$missing" ]; then
    ok "$(read_main "" | grep -oE '"scene-[a-z0-9_-]+"' | sort -u | wc -l) scene names, all with fixtures"
else
    bad "$rt_main names scenes with no fixture under $rt_dat/"
    printf '%s\n' "$missing" | head -5 | sed 's/^/        /'
fi

if [ -n "$history" ]; then
    hdr "references: the same check at every commit in $history"
    ncommits=0
    for c in $(git rev-list --reverse "$history"); do
        ncommits=$((ncommits + 1))
        commit_missing=$(dangling "$c")
        if [ -n "$commit_missing" ]; then
            bad "$(git rev-parse --short "$c") names a scene it does not have"
            printf '%s\n' "$commit_missing" | head -3 | sed 's/^/        /'
        fi
    done
    if [ "$ncommits" = 0 ]; then
        bad "$history resolved to no commits"
    else
        ok "$ncommits commits checked"
    fi
fi

# ---------------------------------------------------------------- binaries

rendertest="$build/examples/rendertest/rendertest"
mkres="$build/examples/mkres/mkres"
gltfview="$build/examples/gltfview/gltfview"

if [ ! -x "$rendertest" ]; then
    echo; echo "no rendertest binary at $rendertest - cannot continue" >&2
    exit 1
fi

export SDL_VIDEODRIVER=offscreen
export GLTFVIEW_BENCH_FRAMES=5
export GLTFVIEW_BENCH_NOVSYNC=1

# ------------------------------------------------------------- the checks

# The corpus, the fixture reproduction and the glTF sweep are registered as
# tests by the build (examples/rendertest/CMakeLists.txt), so they are written
# once. This script used to carry a second copy of the corpus loop, and the two
# drifted: a run that finished and failed was reported here as one that had
# produced no result at all.
#
# --history and the build step are what this script adds. Neither belongs in a
# build: one walks git history, the other is the build.

if [ "$bless" = 1 ]; then
    hdr "bless: rewriting the reference"

    # Blessing is not a test, and must not become one - accepting current
    # output is the one thing a test cannot be allowed to do. The
    # configurations still come from the build, via `ctest -N', rather than
    # from a second copy of the list.
    blessed=0
    for name in $(ctest --test-dir "$build" -N 2>/dev/null | sed -n 's/.*Test *#[0-9]*: *\([a-z0-9-]*\).*/\1/p'); do
        device=${name%%-*}
        rest=${name#*-}
        type=${rest%%-*}
        mode=${rest#*-}

        if [ "$mode" = zs ]; then depth="--no-depth"; else depth=""; fi

        # The name grammar differs by device. A software rasteriser's name is
        # `<device>-<type>-<mode>`; glrend's is `<device>-<mode>`, because the
        # GL device has no bpp axis - --bpp is inert there, the pixel type in
        # the key being the colour buffer's type, which the driver chooses.
        # The device is the first field either way and the mode the last, so
        # the split above parses both; only the `--bpp' the softrend command is
        # built from is absent, and glrend still takes the mode's depth flag.
        case "$device" in
            softrend) out=$("$rendertest" --device softrend --bpp "$type" $depth --bless 2>&1) ;;
            glrend)   out=$(LIBGL_ALWAYS_SOFTWARE=true "$rendertest" --device glrend $depth --bless 2>&1) ;;
            *)        continue ;;
        esac

        line=$(echo "$out" | grep -oE 'result=(PASS|FAIL) failures=[0-9]+' | tail -1)
        ok "$name: ${line:-no result line}"
        blessed=$((blessed + 1))
    done

    [ "$blessed" != 0 ] || bad "no configurations found to bless"
fi

hdr "corpus: ctest"

# --no-tests=error: with no test to run ctest exits 0, and reporting that as a
# pass says the corpus is clean when nothing looked at it. A build registers no
# tests either because BUILD_TESTING is off or because the commit predates the
# corpus being registered with CTest.
if ctest --test-dir "$build" -j"$(nproc)" --output-on-failure --no-tests=error >/tmp/run-tests-ctest.log 2>&1; then
    ok "$(grep -oE '[0-9]+% tests passed[^,]*' /tmp/run-tests-ctest.log | tail -1) (log: /tmp/run-tests-ctest.log)"
else
    if grep -q 'tests passed' /tmp/run-tests-ctest.log; then
        bad "ctest failed (log: /tmp/run-tests-ctest.log)"
        grep -E '\*\*\*Failed|Failed ' /tmp/run-tests-ctest.log | head -5 | sed 's/^/        /'
    else
        bad "this build registered no tests, so the corpus was not run (log: /tmp/run-tests-ctest.log)"
    fi
fi

# ------------------------------------------------------------------ done

hdr "summary"
printf '  %d passed, %d failed, %d skipped\n' "$pass" "$fail" "$skipped"

if [ "$fail" != 0 ]; then
    exit 1
fi
exit 0

# Notes
# -----
# A fresh configure is all that is needed:
#
#   cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
#
# Any call to --bless is a deliberate act: it accepts the current output as
# correct. This script never passes it unless asked.
#
# --history walks every commit in the range. Slow, but it is the only check
# here that looks at anything other than the working tree, and the one thing
# it has caught - commits naming fixtures that did not exist yet - cannot be
# seen any other way.
