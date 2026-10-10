#!/usr/bin/env bash
#
# Witness census corpus runner (softprim).
#
# One process per scene, so a log file is exactly one scene's decisions and no
# marker is needed from the harness. Two corpora:
#
#   fix   the fixtures, in the order the harness names them (`--list`), so a
#         list that has drifted from the corpus cannot go unnoticed
#   ref   the nine resources/gltf-reference scenes, through --scene-dir
#
# Both at 4 bpp x {zb, zs}. Z-sorted runs go through rendertest --no-depth
# rather than gltfview, which has GLTFVIEW_ZSORT but would reach the same path.
#
# Usage: run_corpus.sh <build-dir> <out-dir>
#
#   <build-dir>  a built tree configured with -DBRENDER_SOFT_WITNESS_LOG=ON
#                (see README.md); with the option off the logs come out empty
#   <out-dir>    where the logs and transcripts go; <out-dir>/logs is cleared
#                first, so a stale log cannot be counted as a selection
#
# Environment:
#
#   CENSUS_DAT        fixture directory, default this tree's own
#   CENSUS_REFERENCE  reference table to score against, default this tree's own.
#                     The nine reference scenes have no entries in it, so their
#                     runs report NO-REFERENCE either way; the selection log is
#                     what this script is for.
#   JOBS              scenes in parallel, default 8.
set -u

BUILD=${1:?usage: run_corpus.sh <build-dir> <out-dir>}
OUT=${2:?usage: run_corpus.sh <build-dir> <out-dir>}

ROOT=$(cd "$(dirname "$0")/../.." && pwd)
RT=$BUILD/examples/rendertest/rendertest
DAT=${CENSUS_DAT:-$ROOT/examples/rendertest/dat}
REF=${CENSUS_REFERENCE:-}
LOGS=$OUT/logs

[ -x "$RT" ] || { echo "no rendertest at $RT - build the tree first" >&2; exit 1; }

mkdir -p "$LOGS"
rm -f "$LOGS"/*.log "$LOGS"/*.txt
mapfile -t FIXTURES < <("$RT" --list)

REFSCENES="croc-island-forest croc-mp001-00 croc-mp001-00-nose croc-mp010-03 croc-mp032_00 croc-mp033_01 croc-mp038_00 croc-mp047_02 croc-title-ice"

rt_run() { # <corpus> <mode> <bpp> <scene> <scene-dir>
    local corpus=$1 mode=$2 bpp=$3 scene=$4 dir=$5
    local log=$LOGS/${corpus}_${mode}_bpp${bpp}_${scene}.log
    local args=(--device softrend --bpp "$bpp")
    [ "$mode" = zs ] && args+=(--no-depth)
    [ "$dir" != - ] && args+=(--scene-dir "$dir")
    [ -n "$REF" ] && args+=(--reference "$REF")
    : > "$log"
    BR_WITNESS_LOG="$log" SDL_VIDEODRIVER=offscreen timeout -s KILL 300 \
        "$RT" "${args[@]}" "$scene" > "${log%.log}.txt" 2>&1
}

export -f rt_run
export RT LOGS REF

emit_jobs() {
    local bpp mode s
    for bpp in 8 15 16 24; do
        for mode in zb zs; do
            for s in "${FIXTURES[@]}"; do echo "fix $mode $bpp $s $DAT"; done
            for s in $REFSCENES; do echo "ref $mode $bpp $s $ROOT/resources/gltf-reference"; done
        done
    done
}

# One shell invocation per (configuration, scene), dispatched through the
# exported function so xargs can parallelise without a temporary command file.
emit_jobs | xargs -P "${JOBS:-8}" -L1 bash -c 'rt_run "$@"' _

echo "logs in $LOGS ($(ls "$LOGS"/*.log 2>/dev/null | wc -l) files)"
echo "transcripts with no result line: $(grep -L 'result=' "$LOGS"/*.txt 2>/dev/null | wc -l)  (a run that died)"
echo "transcripts reporting failures:  $(grep -l 'result=FAIL' "$LOGS"/*.txt 2>/dev/null | wc -l)  (the 'ref' corpus always does - no table holds those scenes)"
