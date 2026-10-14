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
# The sync is committed as `meta: sync oracle from <sha>`, so the worktree is
# left clean and the branch is a self-consistent oracle. The blessing run reads
# the working tree either way. It refuses to run when the worktree carries
# changes outside the synced paths, so a sync cannot be mixed into an
# in-progress cherry-pick or a hand-applied patch.

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

# Refuse to run over changes outside the synced paths. The commit below is
# path-limited, so it cannot sweep them in, but a sync has no business sharing
# a tree with an in-progress cherry-pick or a hand-applied softrend patch.
unrelated=()
while IFS= read -r line; do
    f=${line:3}
    f=${f##* -> }

    keep=
    for p in "${paths[@]}"; do
        case "$f" in "$p" | "$p"/*) keep=1 ;; esac
    done

    [ -n "$keep" ] || unrelated+=("$f")
done < <(git -C "$worktree" status --porcelain)

if [ "${#unrelated[@]}" -gt 0 ]; then
    echo "error: the worktree has changes outside the synced paths:" >&2
    printf '  %s\n' "${unrelated[@]}" >&2
    echo "       commit or stash them first" >&2
    exit 2
fi

sha=$(git -C "$worktree" rev-parse --short "$from") || exit 2

if ! git -C "$worktree" checkout "$from" -- "${paths[@]}"; then
    echo "error: could not sync from $from" >&2
    exit 1
fi

printf '\nsynced from %s into %s\n' "$from" "$worktree"
for p in "${paths[@]}"; do
    printf '  %s\n' "$p"
done

if git -C "$worktree" diff --cached --quiet -- "${paths[@]}"; then
    printf '\nalready current at %s; nothing to commit\n' "$sha"
else
    git -C "$worktree" commit -q \
        -m "meta: sync oracle from $sha" \
        -m "Brings examples/rendertest, examples/brdemo, examples/mkres and resources/gltf-reference forward from $from." \
        -- "${paths[@]}" || {
        echo "error: could not commit the sync" >&2
        exit 1
    }

    printf '\ncommitted meta: sync oracle from %s\n' "$sha"
fi

printf '\nbuild it in the linux32 shell, then bless the x87 half against this tree:\n'
printf '  nix develop .#brender-samples-linux32 --command bash -c \\\n'
printf "    'cd %s && cmake --build cmake-build-32-pent2 --target rendertest -j\$(nproc)'\n" "$worktree"
printf '\nresources/aidocs/recipes.md has the blessing loop.\n'
