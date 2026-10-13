#!/usr/bin/env bash
#
# Bring the pentprim oracle worktree's corpus and rendertest harness forward
# from a master ref.
#
# The x87 half of examples/rendertest/rendertest.txt is pentprim's output, and
# pentprim only exists on dev/pentprim-verification - a snapshot of the tree
# from before the portable rasteriser replaced it. Its corpus, harness and
# reference therefore drift behind master, and every blessing run has to bring
# them forward first. This is that step.
#
# Usage:
#   contrib/sync-oracle.sh [--worktree DIR] [--from REF]
#
#   --worktree DIR   the oracle worktree; default is the one on
#                    dev/pentprim-verification, or the sibling <repo>-pent
#                    worktree the blessing recipe creates (it is normally
#                    checked out at a commit, not the branch, so the path is
#                    the reliable handle)
#   --from REF       ref to take the files from; default master
#
# A softrend change is not synced here: it is carried by cherry-picking its
# commit onto the worktree, which is clean because the softrend change is a
# commit of its own. examples/rendertest includes the reference table, so a
# cherry-pick that also carries a table change finds it already current and
# resolves to this sync's.
#
# examples/brdemo is synced with examples/rendertest because rendertest is
# built from the two together: a harness change that reaches one and not the
# other leaves the worktree's rendertest using a brdemo API the branch does not
# have, and it does not compile. (The require_accel change was the first one to
# do that.)
#
# The sync is written to the worktree and staged, not committed. The blessing
# run reads the working tree, so nothing has to land in the branch's history;
# reset the worktree before the next sync.

set -u

repo=$(cd "$(dirname "$0")/.." && pwd)
branch=dev/pentprim-verification
worktree=
from=master

while [ $# -gt 0 ]; do
    case "$1" in
        --worktree) worktree="${2:-}" ; shift ;;
        --from)     from="${2:-}" ; shift ;;
        -h|--help)  awk 'NR>1 && /^#/ {sub(/^# ?/, ""); print; next} NR>1 {exit}' "$0"; exit 0 ;;
        *)          echo "unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done

if [ -z "$worktree" ]; then
    worktree=$(git -C "$repo" worktree list --porcelain | awk -v b="refs/heads/$branch" '
        /^worktree / { w = $2 }
        /^branch /   { if ($2 == b) print w }')
fi

if [ -z "$worktree" ] && [ -e "${repo}-pent/.git" ] && git -C "${repo}-pent" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    worktree="${repo}-pent"
fi

if [ -z "$worktree" ] || [ ! -d "$worktree" ]; then
    echo "error: no oracle worktree found; pass --worktree DIR, or create one with:" >&2
    echo "       git worktree add ${repo}-pent $branch" >&2
    exit 2
fi

if ! git -C "$worktree" rev-parse --verify --quiet "$from^{commit}" >/dev/null; then
    echo "error: $from does not resolve to a commit" >&2
    exit 2
fi

paths=(examples/rendertest examples/brdemo examples/mkres resources/gltf-reference)

if ! git -C "$worktree" checkout "$from" -- "${paths[@]}"; then
    echo "error: could not sync from $from" >&2
    exit 1
fi

printf '\nsynced from %s into %s\n' "$from" "$worktree"
for p in "${paths[@]}"; do
    printf '  %s\n' "$p"
done

printf '\nbuild it in the linux32 shell, then bless the x87 half against this tree:\n'
printf '  nix develop .#brender-samples-linux32 --command bash -c \\\n'
printf "    'cd %s && cmake --build cmake-build-32-pent2 --target rendertest -j\$(nproc)'\n" "$worktree"
printf '\nresources/aidocs/recipes.md has the blessing loop.\n'
