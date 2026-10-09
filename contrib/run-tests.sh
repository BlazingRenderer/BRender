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
#                        [--reference-driver <token>] [--history <range>]
#
#   build-dir   defaults to cmake-build.
#   --bless     rewrite the reference table instead of comparing. Only for
#               accepting a deliberate change; see the note at the end.
#   --no-build  skip the build step (for re-running after no source change).
#   --reference-driver <token>
#               score against another driver's entries, e.g. 'software' to
#               compare a new rasteriser against the recorded ones.
#   --history <range>
#               also check the scene/fixture invariant at every commit in
#               <range>, e.g. upstream/master..HEAD. Slower, and the only way
#               to catch a commit that named a fixture it did not have.

set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
build="cmake-build"
bless=0
do_build=1
ref_driver=""
history=""

while [ $# -gt 0 ]; do
    case "$1" in
        --bless)            bless=1 ;;
        --no-build)         do_build=0 ;;
        --reference-driver) ref_driver="${2:-}"; shift ;;
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

# ------------------------------------------------------- fixture invariant

hdr "fixtures: mkres scenes reproduces examples/rendertest/dat/"

if [ ! -x "$mkres" ]; then
    skip "mkres not built"
else
    # The build dir may be relative or absolute; mktemp runs the binary from
    # elsewhere, so an absolute path is needed either way.
    case "$mkres" in /*) mkres_bin="$mkres" ;; *) mkres_bin="$repo/$mkres" ;; esac
    tmp=$(mktemp -d) || exit 1
    ( cd "$tmp" && "$mkres_bin" scenes >/dev/null 2>&1 )
    if diff -rq "$tmp" examples/rendertest/dat/ >/tmp/run-tests-mkres.log 2>&1; then
        ok "$(ls "$tmp" | wc -l) fixtures byte-identical to dat/"
    else
        n=$(grep -c '^Files ' /tmp/run-tests-mkres.log 2>/dev/null || echo '?')
        bad "$n fixture(s) differ from dat/ (log: /tmp/run-tests-mkres.log)"
        sed 's/^/        /' /tmp/run-tests-mkres.log | head -10
    fi
    rm -rf "$tmp"
fi

# ------------------------------------------------------------- the corpus

hdr "corpus: rendertest"

ref_args=""
[ "$bless" = 1 ] && ref_args="--bless"
[ -n "$ref_driver" ] && ref_args="$ref_args --reference-driver $ref_driver"

# bpp N renders through BrZbSceneRender; --no-depth renders through
# BrZsSceneRender, the Z-sort path. Both are distinct reference keys.
for bpp in 8 15 16 24; do
    for mode in zb zs; do
        if [ "$mode" = zs ]; then depth="--no-depth"; else depth=""; fi

        out=$("$rendertest" --device softrend --bpp "$bpp" $depth $ref_args 2>&1)
        # Match FAIL as well as PASS. A failed run does print a result line,
        # and grepping only for PASS reported every failure as a run that
        # produced no result at all - which is what hid a whole
        # architecture's worth of failures here.
        line=$(echo "$out" | grep -oE 'result=(PASS|FAIL) failures=[0-9]+' | tail -1)
        match=$(echo "$out" | grep -c 'MATCH')
        nref=$(echo "$out" | grep -c 'NO-REFERENCE')

        if [ -z "$line" ]; then
            bad "softrend $bpp/$mode: no result line - the run did not finish"
        elif [ "$line" = "result=PASS failures=0" ]; then
            # NO-REFERENCE is not a failure, but it is unverified - say so.
            if [ "$nref" -gt 0 ]; then
                ok "softrend $bpp/$mode: $line (${match} match, ${nref} NO-REFERENCE)"
            else
                ok "softrend $bpp/$mode: $line (${match} match)"
            fi
        else
            bad "softrend $bpp/$mode: $line"
            echo "$out" | grep -iE 'CHANGED|FAIL' | head -5 | sed 's/^/        /'
        fi
    done
done

# glrend is the arbiter for anything the software paths disagree on. The
# checked-in glrend references are llvmpipe-keyed, so force software GL.
out=$(LIBGL_ALWAYS_SOFTWARE=true "$rendertest" --device glrend --bpp 8 $ref_args 2>&1)
line=$(echo "$out" | grep -oE 'result=(PASS|FAIL) failures=[0-9]+' | tail -1)
if [ "$line" = "result=PASS failures=0" ]; then
    ok "glrend 8/zb: $line ($(echo "$out" | grep -c MATCH) match)"
elif [ -z "$line" ]; then
    bad "glrend 8/zb: no result line - the run did not finish (no GL? try LIBGL_ALWAYS_SOFTWARE=true)"
else
    bad "glrend 8/zb: $line"
fi

# ------------------------------------------------------------- gltf loads

hdr "every checked-in .gltf loads"

if [ ! -x "$gltfview" ]; then
    skip "gltfview not built"
else
    n=0; nbad=0
    for f in examples/rendertest/dat/*.gltf resources/gltf-reference/*.gltf; do
        [ -e "$f" ] || continue
        n=$((n + 1))
        if ! timeout -s KILL 60 "$gltfview" --force-software --software-bpp 8 \
                -w 320 -h 240 --no-stats "$f" >/dev/null 2>&1; then
            bad "load: $f"
            nbad=$((nbad + 1))
        fi
    done
    [ "$nbad" = 0 ] && ok "$n/$n files load"
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
