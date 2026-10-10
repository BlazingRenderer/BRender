#!/usr/bin/env python3
"""
Witness census over the pentprim matcher.

Joins two things:

  * the generated block tables in the build tree, which say what entries exist;
  * the per-scene selection logs run_corpus.sh wrote, which say which of them
    the corpus selected.

An entry nothing selected is a block no fixture exercises, which is the question
this exists to answer.

Usage: witness.py <build-dir> <census-dir> [--all] [--scenes] [--mmx]

  --all      list every entry, not only the unwitnessed ones
  --scenes   per scene, what it selected
  --mmx      per rasteriser symbol, the blocks that reach it and whether any
             scene selected one of them
"""

import collections
import glob
import os
import re
import sys

# Table name as the instrument logs it -> the generated sources that can carry
# it, and the output format and topology the .ifg names give it. The generator
# does not name them uniformly: the 15 and 16 bpp triangle tables are prm_t15f.c
# and prm_t16f.c, the 8 bpp one is prim_t8f.c, the 24 bpp one has no class
# suffix at all (prim_t24.c), the lines and points carry none either, and only
# the triangle tables are generated per floating-point class. This is round 1's
# mapping; deriving it from the table names is how it was got wrong once.
FMT = {8: "INDEX_8", 15: "RGB_555", 16: "RGB_565", 24: "RGB_888"}
TRI = {8: "prim_t8", 15: "prm_t15", 16: "prm_t16", 24: "prim_t24"}

TABLES = {}
for _bpp in (8, 15, 16, 24):
    TABLES["prim_p%d" % _bpp] = (["prim_p%d.c" % _bpp], FMT[_bpp], "point")
    TABLES["prim_l%d" % _bpp] = (["prim_l%d.c" % _bpp], FMT[_bpp], "line")
    TABLES["prim_t%d" % _bpp] = (["%s.c" % TRI[_bpp], "%sf.c" % TRI[_bpp], "%sx.c" % TRI[_bpp]], FMT[_bpp], "triangle")
for _bpp in (15, 16):
    TABLES["mmx_t%d" % _bpp] = (["mmx_t%df.c" % _bpp, "mmx_t%dx.c" % _bpp], FMT[_bpp], "triangle")

FIELD = lambda name, text, pat=r"([^,\n]+)": (
    m.group(1).strip() if (m := re.search(r"\.%s\s*=\s*%s," % (name, pat), text)) else None
)

# The fields match_block() (drivers/pentprim/match.c) tests besides the flags
# predicate. They are read out here so reachable.py walks the same entries this
# census joins against - one parser, one spelling of "what a block requires".
PREDICATES = [
    "flags_mask", "flags_cmp",
    "depth_type", "texture_type", "shade_type", "bump_type",
    "lighting_type", "screendoor_type", "blend_type", "fog_type",
    "input_colour_type",
]


def parse_tables(build):
    """-> {(table, index): entry} for the tables this build generated."""
    entries = {}
    for table, (names, fmt, topo) in TABLES.items():
        path = next((os.path.join(build, "drivers/pentprim", n) for n in names
                     if os.path.exists(os.path.join(build, "drivers/pentprim", n))), None)
        if path is None:
            continue
        # The generator writes one `{ ... },` per entry in table order, so the
        # chunk index is the entry's position - the index the instrument logs.
        for i, chunk in enumerate(open(path).read().split("\n},\n")):
            ident = FIELD("identifier", chunk, r'"([^"]*)"')
            if ident is None:
                continue
            entries[(table, i)] = {
                "table": table,
                "index": i,
                "identifier": ident,
                "fmt": fmt,
                "topo": topo,
                # The four rasterise slots are filled with the same pair for
                # every block, so the distinct set is what a primitive reaching
                # this block can call.
                "kernels": sorted(set(re.findall(
                    r"\.rasterise_(?:rl|lr)_[ls]\s*=\s*([A-Za-z_][A-Za-z0-9_]*),", chunk))),
                # The requirements match_block() compares against the
                # primitive state, in the generator's own spelling.
                "map_width": int(FIELD("map_width", chunk) or 0),
                "map_height": int(FIELD("map_height", chunk) or 0),
                **{name: FIELD(name, chunk) for name in PREDICATES},
            }
    return entries


def load_logs(census):
    """-> [(corpus, mode, bpp, scene, table, index, verdict, identifier)]"""
    rows = []
    for path in sorted(glob.glob(os.path.join(census, "logs", "*.log"))):
        base = os.path.basename(path)
        m = re.match(r"^(\w+)_(zb|zs)_bpp(\d+)_(.+)\.log$", base)
        if not m:
            print("witness: ignoring %s (not a run_corpus.sh log name)" % base, file=sys.stderr)
            continue
        corpus, mode, bpp, scene = m.group(1), m.group(2), int(m.group(3)), m.group(4)
        for line in open(path):
            parts = line.rstrip("\n").split("\t")
            if len(parts) != 4:
                continue
            table, index, verdict, ident = parts
            rows.append((corpus, mode, bpp, scene, table, int(index), verdict, ident))
    return rows


def main():
    if len(sys.argv) < 3:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    build, census = sys.argv[1], sys.argv[2]
    show_all = "--all" in sys.argv
    show_scenes = "--scenes" in sys.argv
    show_mmx = "--mmx" in sys.argv

    tables = parse_tables(build)
    if not tables:
        print("no generated tables under %s/drivers/pentprim - is the build done?" % build, file=sys.stderr)
        return 2
    rows = load_logs(census)
    if not rows:
        print("no logs under %s/logs - run run_corpus.sh first" % census, file=sys.stderr)
        return 2

    # Integrity: an index in a log has to name the entry the table has there, or
    # the census is joining the wrong rows.
    bad = [r for r in rows
           if (r[4], r[5]) in tables and tables[(r[4], r[5])]["identifier"] != r[7]]
    unknown = sorted({(r[4], r[5]) for r in rows if (r[4], r[5]) not in tables})

    seen = collections.defaultdict(list)
    for corpus, mode, bpp, scene, table, index, verdict, ident in rows:
        seen[(table, index)].append((corpus, mode, bpp, scene, verdict))

    print("selections: %d   distinct entries: %d   entries in the tables: %d"
          % (len(rows), len(seen), len(tables)))
    if bad:
        print("\n!! %d log lines disagree with the table they index, e.g. %s#%d: logged %r, table has %r"
              % (len(bad), bad[0][4], bad[0][5], bad[0][7], tables[(bad[0][4], bad[0][5])]["identifier"]))
    if unknown:
        print("\n!! %d (table, index) pairs are not in the tables, e.g. %r" % (len(unknown), unknown[:3]))

    print("\n%-12s %-9s %-9s %5s %6s %7s" % ("table", "format", "topology", "blocks", "seen", "never"))
    for table in sorted({e["table"] for e in tables.values()}):
        blocks = [e for e in tables.values() if e["table"] == table]
        nseen = len([e for e in blocks if (e["table"], e["index"]) in seen])
        print("%-12s %-9s %-9s %5d %6d %7d"
              % (table, blocks[0]["fmt"], blocks[0]["topo"], len(blocks), nseen, len(blocks) - nseen))

    unwitnessed = [e for e in tables.values() if (e["table"], e["index"]) not in seen]
    wanted = sorted(tables.values(), key=lambda e: (e["table"], e["index"])) if show_all else \
        sorted(unwitnessed, key=lambda e: (e["table"], e["index"]))
    print("\n%s (%d):" % ("entries" if show_all else "entries nothing selected", len(wanted)))
    for e in wanted:
        where = seen.get((e["table"], e["index"]))
        if where:
            w = ", ".join(sorted({"%s/%s %dbpp" % (s, m, b) for _, m, b, s, _ in where})[:3])
            print("  %-10s %4d  %-64s %s" % (e["table"], e["index"], e["identifier"], w))
        else:
            print("  %-10s %4d  %-64s %s" % (e["table"], e["index"], e["identifier"],
                                             "(kernels: %s)" % ", ".join(e["kernels"]) if e["kernels"] else ""))

    if show_mmx:
        syms = collections.defaultdict(lambda: {"blocks": [], "scenes": set()})
        for e in tables.values():
            if not e["table"].startswith("mmx_"):
                continue
            for sym in e["kernels"]:
                syms[sym]["blocks"].append("%s#%d" % (e["table"], e["index"]))
                for _, m, b, s, _ in seen.get((e["table"], e["index"]), []):
                    syms[sym]["scenes"].add("%s/%s %dbpp" % (s, m, b))
        print("\nMMX symbols (%d):" % len(syms))
        for sym in sorted(syms):
            d = syms[sym]
            print("  %-40s %2d block(s)  %s" % (sym, len(d["blocks"]),
                  ", ".join(sorted(d["scenes"])[:3]) if d["scenes"] else "NOT SELECTED"))

    if show_scenes:
        per = collections.defaultdict(list)
        for corpus, mode, bpp, scene, table, index, verdict, ident in rows:
            per[(corpus, scene, mode, bpp)].append("%s#%d" % (table, index))
        print("\nper scene:")
        for key in sorted(per):
            print("  %-4s %-28s %-3s %2d  %s" % (key[0], key[1], key[2], key[3],
                                                  " ".join(sorted(set(per[key])))))
    return 0


if __name__ == "__main__":
    sys.exit(main())
