#!/usr/bin/env bash
#
# Build and check every commit in a range.
#
# `--history' in run-tests.sh only checks that each commit's scene names have
# fixtures. It does not build anything, so a commit that does not compile, or
# that moves pixels without re-blessing the reference, passes it. This is the
# check that every commit compiles and passes.
#
# Each commit gets its own configure and build in a separate worktree, then the
# full gate from contrib/run-tests.sh. Doing it in a worktree keeps the caller's
# tree alone, and configuring per commit is what makes it work across a commit
# that changes the build's own options - it costs a full build per commit, which
# is why this is not part of the normal run.
#
# The gate script is copied in from the calling tree, so a commit is checked by
# today's harness against its own fixtures and reference. It has to be: this
# script is newer than the swap, so the commits near the start of a range do not
# have one of their own.
#
# The range must start after the portable rasteriser replaced pentprim
# (74176fa9). Earlier commits build the assembler rasteriser, which needs uasm
# and h2inc and configures differently; it is regenerable (see
# resources/aidocs/recipes.md) but not by this script.
#
# It must also start at or after 3c9f1394, where the corpus became a CTest test.
# The gate is copied in from the calling tree and it runs the corpus through
# ctest, and a build that registers no tests is reported as a failure rather
# than as a pass - so a commit that predates the CTest wiring cannot be checked
# by it, and never will be by a later version of it.
#
# Usage:
#   contrib/verify-series.sh [range] [--keep] [-j N]
#
#   range    defaults to upstream/master..HEAD
#   --keep   leave the worktree and its builds in place for inspection

set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
range="upstream/master..HEAD"
keep=0
jobs=$(nproc)

while [ $# -gt 0 ]; do
    case "$1" in
        --keep) keep=1 ;;
        -j)     jobs="${2:-}" ; shift ;;
        -h|--help) awk 'NR>1 && /^#/ {sub(/^# ?/, ""); print; next} NR>1 {exit}' "$0"; exit 0 ;;
        -*)     echo "unknown option: $1" >&2; exit 2 ;;
        *)      range="$1" ;;
    esac
    shift
done

cd "$repo" || exit 1

first=$(git rev-list --reverse "$range" | head -1)
if [ -z "$first" ]; then
    echo "error: $range resolved to no commits" >&2
    exit 2
fi

# Refuse a range that reaches back past the swap rather than failing per commit
# with an assembler error nobody can read.
if ! git merge-base --is-ancestor 74176fa9 "$first" 2>/dev/null && [ "$first" != 74176fa9 ]; then
    echo "error: $range starts before 74176fa9 (the portable rasteriser); see the note above" >&2
    exit 2
fi

# And refuse one that reaches back past the corpus becoming a test, before
# which there is no test for the copied-in gate to run.
if ! git merge-base --is-ancestor 3c9f1394 "$first" 2>/dev/null; then
    echo "error: $range starts before 3c9f1394 (the corpus became a CTest test); see the note above" >&2
    exit 2
fi

worktree=${repo}-series
if [ -e "$worktree" ]; then
    echo "error: $worktree already exists; remove it or pass a different path" >&2
    exit 2
fi

cleanup() {
    if [ "$keep" = 1 ]; then
        echo "worktree kept at $worktree"
    else
        git -C "$repo" worktree remove --force "$worktree" 2>/dev/null
    fi
}
trap cleanup EXIT

git worktree add --detach "$worktree" >/dev/null 2>&1 || exit 1

build=cmake-build-series
pass=0
fail=0

printf '\n== %s\n' "$range"

for c in $(git rev-list --reverse "$range"); do
    short=$(git rev-parse --short "$c")
    subject=$(git log -1 --format=%s "$c" | cut -c1-58)
    log=/tmp/verify-series-$short.log

    git -C "$worktree" checkout --detach --force --quiet "$c" 2>/dev/null || {
        printf '  %-9s %-58s CHECKOUT FAILED\n' "$short" "$subject"
        fail=$((fail + 1))
        continue
    }

    # A fresh configure per commit: an option's meaning can change between
    # commits, and a stale cache would answer for the wrong build.
    rm -rf "$worktree/$build"

    if ! cmake -S "$worktree" -B "$worktree/$build" -DCMAKE_BUILD_TYPE=Release >"$log" 2>&1; then
        printf '  %-9s %-58s CONFIGURE FAILED (%s)\n' "$short" "$subject" "$log"
        fail=$((fail + 1))
        continue
    fi

    if ! cmake --build "$worktree/$build" -j"$jobs" >>"$log" 2>&1; then
        printf '  %-9s %-58s BUILD FAILED (%s)\n' "$short" "$subject" "$log"
        fail=$((fail + 1))
        continue
    fi

    cp "$repo/contrib/run-tests.sh" "$worktree/contrib/run-tests.sh"

    if ! ( cd "$worktree" && contrib/run-tests.sh "$build" --no-build ) >>"$log" 2>&1; then
        printf '  %-9s %-58s CHECKS FAILED (%s)\n' "$short" "$subject" "$log"
        fail=$((fail + 1))
        continue
    fi

    printf '\033[32m  %-9s %-58s ok\033[0m\n' "$short" "$subject"
    pass=$((pass + 1))
done

printf '\n  %d ok, %d failed\n' "$pass" "$fail"

[ "$fail" = 0 ] || exit 1
exit 0
