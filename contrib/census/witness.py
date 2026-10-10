#!/usr/bin/env python3
"""
Witness census over the softprim matcher.

Joins three things:

  * the generated matcher list (softprim_matchers.inc) from a build - it names
    the emitted blocks and their axis tuples;
  * the per-scene decision logs run_corpus.sh wrote, where every spFindMatch call
    is one line "<index>\\t<match|refused|nomatch>\\t<identifier>";
  * the reachability walk in reachable.py, which decides which emitted tuples any
    state could select at all.

An emitted tuple nothing selects is a kernel no fixture exercises; an emitted
tuple no state can select is dead code in the table. The two need different
answers, so they are reported separately.

Usage: witness.py <build-dir> <census-dir> [--json out.json]
"""

import collections
import glob
import itertools
import json
import os
import re
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import reachable  # noqa: E402

AXES = ["fmt", "top", "depth", "shade", "tex", "addr", "persp", "blend", "fog", "dith"]


def parse_inc(path):
    """-> list of entries, in matcher order."""
    line_re = re.compile(r"SOFTPRIM_(BLOCK_TRI|BLOCK_LINE|BLOCK_POINT|REFUSED)\(([^,]+),\s*(.*)\)\s*$")
    entries = []
    for line in open(path):
        m = line_re.match(line.strip())
        if not m:
            continue
        kind, name, rest = m.group(1), m.group(2), m.group(3)
        args = [a.strip() for a in rest.split(",")]
        # The axes are the trailing ten SP_* arguments; the earlier ones are the
        # per-map type fields, which are also spelled SP_PMT_NONE when the block
        # requires no map of that kind.
        axes = tuple(t for t in re.findall(r"SP_[A-Z0-9_]+", rest))[-10:]
        entries.append({
            "index": len(entries),
            "kind": "refused" if kind == "REFUSED" else "emitted",
            "identifier": name,
            "axes": axes,
            "flags_mask": args[0],
            "flags_cmp": args[1],
            "msize": int(args[9]),
            "types": args[10:15],
        })
    return entries


def parse_log(path):
    """-> (n_decisions, n_match, n_refused, n_nomatch, set-of-emitted-ids)"""
    n = n_m = n_r = n_n = 0
    ids = set()
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if len(parts) < 3:
            continue
        ident = parts[2]
        outcome = parts[1]
        n += 1
        if outcome == "match":
            n_m += 1
            ids.add(ident)
        elif outcome == "refused":
            n_r += 1
        else:
            n_n += 1
    return n, n_m, n_r, n_n, ids


def short(shape):
    return "/".join(
        t.replace("SP_FMT_", "").replace("SP_TOP_", "").replace("SP_DEPTH_", "")
        .replace("SP_SHADE_", "").replace("SP_TEX_", "").replace("SP_ADDR_", "")
        .replace("SP_PERSP_", "").replace("SP_BLEND_", "").replace("SP_FOG_", "")
        .replace("SP_DITH_", "")
        for t in shape)


def main():
    build = sys.argv[1]
    census = sys.argv[2]
    inc = os.path.join(build, "drivers/softprim/softprim_matchers.inc")
    if not os.path.exists(inc):
        print("no generated %s - configure and build the tree first" % inc, file=sys.stderr)
        return 2
    entries = parse_inc(inc)

    emitted = [e for e in entries if e["kind"] == "emitted"]
    refused = [e for e in entries if e["kind"] == "refused"]
    emitted_shapes = {e["axes"] for e in emitted}
    refused_shapes = {e["axes"] for e in refused}
    id_to_shape = {e["identifier"]: e["axes"] for e in emitted}

    log_paths = sorted(glob.glob(os.path.join(census, "logs", "*.log")))
    if not log_paths:
        print("no logs under %s/logs - run run_corpus.sh first" % census, file=sys.stderr)
        return 2

    per_run = {}
    total_decisions = total_match = total_refused = total_nomatch = 0
    witnessed_shapes = set()
    for path in log_paths:
        base = os.path.basename(path)[:-4]
        # <corpus>_<mode>_bpp<n>_<scene>
        corpus, mode, bpp, scene = base.split("_", 3)
        n, m, r, nn, ids = parse_log(path)
        total_decisions += n
        total_match += m
        total_refused += r
        total_nomatch += nn
        per_run[(corpus, mode, bpp, scene)] = (n, m, r, nn, ids)
        for i in ids:
            if i in id_to_shape:
                witnessed_shapes.add(id_to_shape[i])

    # A log that exists and says nothing is not a census of an unwitnessed tree:
    # it is a build without the instrument, or a variable that never reached it.
    # Both would otherwise read as "no tuple is witnessed", which is wrong in a
    # way that looks like a finding.
    if total_decisions == 0:
        print("the %d logs are empty: the build has no instrument, or BR_WITNESS_LOG did not reach it.\n"
              "Configure with -DBRENDER_SOFT_WITNESS_LOG=ON (see README.md)." % len(log_paths), file=sys.stderr)
        return 2

    unwitnessed_shapes = emitted_shapes - witnessed_shapes

    print("== headline ==")
    print("matcher entries:      %d" % len(entries))
    print("  emitted (kernel):   %d  (%d distinct tuples)" % (len(emitted), len(emitted_shapes)))
    print("  refused:            %d  (%d distinct tuples)" % (len(refused), len(refused_shapes)))
    print("matcher decisions:    %d" % total_decisions)
    print("  match:              %d" % total_match)
    print("  refused:            %d" % total_refused)
    print("  nomatch:            %d" % total_nomatch)
    print("witnessed tuples:     %d  (%.1f%% of emitted)" % (len(witnessed_shapes), 100.0 * len(witnessed_shapes) / len(emitted_shapes)))
    print("unwitnessed tuples:   %d" % len(unwitnessed_shapes))
    print("logs:                 %d" % len(log_paths))

    # witness provenance: which runs select each shape, for grouping
    prov = collections.defaultdict(set)
    for (corpus, mode, bpp, scene), (n, m, r, nn, ids) in per_run.items():
        for i in ids:
            if i in id_to_shape:
                prov[id_to_shape[i]].add((corpus, scene, mode, bpp))

    reach = reachable.reachable_shapes(entries)
    dead = sorted(unwitnessed_shapes - reach)
    reachable_gap = sorted(unwitnessed_shapes & reach)
    print()
    print("of the unwitnessed:   %d unreachable under any state, %d reachable (a fixture can give them a witness)"
          % (len(dead), len(reachable_gap)))

    # A refused entry is a state softprim will not draw. Whether the corpus ever
    # asks for one is a different question from whether one can be asked for, and
    # the second is what says whether the refusal path is tested or merely there.
    refused_reach = reachable.reachable_refused_shapes(entries)
    print("refused tuples:       %d, of which %d can be selected by some state and %d cannot"
          % (len(refused_shapes), len(refused_reach), len(refused_shapes) - len(refused_reach)))
    if refused_reach:
        print()
        print("== refused tuples a state can select (none of which the corpus selected) ==")
        for s in sorted(refused_reach, key=short):
            ids = sorted({e["identifier"] for e in refused if e["axes"] == s})
            print("  %s  x%d  %s" % (short(s), len(ids), ids[0]))

    print()
    print("== unreachable-under-any-state (dead) tuples ==")
    for s in dead:
        ids = sorted({i for i, sh in id_to_shape.items() if sh == s})
        print("  %s  x%d  %s" % (short(s), len(ids), ids[0]))

    print()
    print("== reachable but unwitnessed tuples ==")
    for s in reachable_gap:
        ids = sorted({i for i, sh in id_to_shape.items() if sh == s})
        seen = sorted({(c, sc) for (c, sc, mo, bp) in prov.get(s, set())})
        print("  %s  x%d  %s" % (short(s), len(ids), ids[0]))

    if "--json" in sys.argv:
        out = sys.argv[sys.argv.index("--json") + 1]
        json.dump({
            "entries": len(entries),
            "emitted": len(emitted),
            "emitted_tuples": len(emitted_shapes),
            "refused": len(refused),
            "decisions": total_decisions,
            "match": total_match,
            "refused_decisions": total_refused,
            "nomatch": total_nomatch,
            "witnessed": sorted(short(s) for s in witnessed_shapes),
            "dead": sorted(short(s) for s in dead),
            "reachable_gap": sorted(short(s) for s in reachable_gap),
            "witnessed_n": len(witnessed_shapes),
            "dead_n": len(dead),
            "reachable_gap_n": len(reachable_gap),
            "refused_reachable": sorted(short(s) for s in refused_reach),
            "refused_reachable_n": len(refused_reach),
            "prov": {"%s" % short(s): sorted("%s/%s@%s/%s" % x for x in v) for s, v in prov.items()},
            "per_run": {"_".join(k): v[0:4] for k, v in per_run.items()},
            "inc": inc,
        }, open(out, "w"), indent=1)
        print("\nwrote", out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
