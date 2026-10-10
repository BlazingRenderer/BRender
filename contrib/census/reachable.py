#!/usr/bin/env python3
"""
Reachability of every generated pentprim block under the matcher's semantics.

"Unwitnessed" has two very different meanings: no fixture happens to hit the
block, and no primitive state can hit it. The second is decidable without a
renderer: it is the walk match_block() in drivers/pentprim/match.c performs,
over the finite space of states the flag predicates and the buffer
requirements can distinguish.

Three things about pentprim make this walk different from the refprim/softprim
one (the round-1 tooling on dev/pentprim-nondet and master's
contrib/census/reachable.py):

  * the outer dispatch is a [format][topology] table, so the walk is per
    (format, topology) - and with MMX on for a 555/565 output the MMX table is
    walked *before* the general one, with the general table as the fall-back.
    An entry can therefore be live in one configuration and dead in the other,
    so the walk is run for both;
  * match_block() tests eight buffer types (depth, texture, shade, bump,
    lighting, screendoor, blend, fog) plus the primitive's input colour type,
    not refprim's five;
  * when nothing in the walk matches, renderBegin() uses the *last* entry of
    the general table as the default. A defaulting state selects that entry,
    so it is reachable even if no state ever matches it; the report keeps
    "matched" and "reachable" apart for this reason. A block reached only by
    the default is still a block the matcher can hand a primitive to.

The state space, exactly as the matcher's comparisons distinguish it:

  * flags are only ever read through the mask/compare pairs the entries carry,
    so the space that matters is the assignment of the PRIMF_* tokens named
    anywhere in the walk. All 2**n assignments are enumerated. This is a
    *superset* of what the engine can produce (PRIMF_POWER2, PRIMF_PALETTE,
    PRIMF_NO_SKIP, PRIMF_STRIDE_POSITIVE and PRIMF_RANGE_ZERO are derived from
    the bound buffers), so "dead" is sound and "reachable" may over-count;
  * a buffer-type requirement is exact equality, or "any" when the entry names
    PMT_NONE, so one representative per named type plus one unnamed type covers
    every case;
  * a size requirement is "width == map_width and height == map_height" with 0
    meaning "any", so one representative per named width/height plus one
    unnamed value on each axis covers every case. The two axes are independent:
    a 1024x96 texture matches map_width == 1024 only if map_height is also 0.
    A texture is unbound exactly when its type is PMT_NONE, and then both sizes
    are 0; the invalid (unbound, non-zero size) states are excluded by the
    validity mask below so they cannot stand in for a state no buffer can be
    in;
  * input_colour_type is 0 ("any") for every entry in these tables, so it is
    modelled but never discriminates.

The walk, for one flag assignment, is the fall-through match_block()
implements: the first entry whose flags and buffer requirements hold is the
match; entries before it that also hold are the ones it shadows.

Usage:  reachable.py <build-dir> [--census <dir>]... [--json <out>]

  --census   also join the run_corpus.sh selection logs under <dir> and split
             every entry into selected / selectable-but-not-selected / not
             selectable. Repeatable; the selected set is the union.

  (import as a module: walk_config(entries_by_table, mmx))
"""

import collections
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import witness  # noqa: E402

# The matcher's outer dispatch (match.c): format index i comes from the colour
# buffer type, topology j from the primitive type. The MMX row substitutes its
# own table for the 555/565 triangle only; every other cell shares the general
# array, which is why only (1, 2) and (2, 2) have a second table.
GENERAL = {
    (0, 0): "prim_p8",  (0, 1): "prim_l8",  (0, 2): "prim_t8",
    (1, 0): "prim_p15", (1, 1): "prim_l15", (1, 2): "prim_t15",
    (2, 0): "prim_p16", (2, 1): "prim_l16", (2, 2): "prim_t16",
    (3, 0): "prim_p24", (3, 1): "prim_l24", (3, 2): "prim_t24",
}
MMX = {(1, 2): "mmx_t15", (2, 2): "mmx_t16"}

TYPE_AXES = ["depth_type", "texture_type", "shade_type", "bump_type",
             "lighting_type", "screendoor_type", "blend_type", "fog_type"]

NONE = "PMT_NONE"          # the generator's "no requirement" spelling
OTHER = "BR_OTHER_TYPE"    # a type token no entry in the walk names
ANY_INPUT = "0"            # input_colour_type "no requirement"
OTHER_SIZE = 1000          # a texture size no entry in the walk names
SIZE_ZERO = (0, 0)


def _tokens(field):
    return {t.strip() for t in field.split("|") if t.strip() and t.strip() != "0"}


def _axis_values(walk, field):
    """PMT_NONE, every value the walk names, then a value it does not."""
    return [NONE] + sorted({e[field] for e in walk if e[field] != NONE}) + [OTHER]


def _value_masks(axes):
    """-> (masks, n_states): masks[a][v] is the bitmask of states whose axis a
    has value index v, over a mixed-radix product with the last axis fastest."""
    strides, s = [0] * len(axes), 1
    for a in range(len(axes) - 1, -1, -1):
        strides[a] = s
        s *= len(axes[a])
    n = s
    allones = (1 << n) - 1
    masks = []
    for a, values in enumerate(axes):
        # Bits with axis a == v are `stride` ones every `len(values)*stride`
        # bits, offset by v*stride. Build the repeated block as a repunit
        # multiplication rather than one OR per block.
        period = len(values) * strides[a]
        repunit = allones // ((1 << period) - 1)
        block = (1 << strides[a]) - 1
        masks.append([((repunit * block) << (v * strides[a])) & allones
                      for v in range(len(values))])
    return masks, n


def _walk_reachable(walk):
    """-> (reachable keys, matched keys, default fired, state space size).

    `walk` is the entry list in matcher order for one (format, topology). The
    result is keyed by (table, index); `matched` are the entries some state's
    walk selected itself, `reachable` additionally carries the default
    fall-through to the last entry of the general table, and `default fired`
    says whether any state matched nothing and so took that fall-through.
    """
    keys = [(e["table"], e["index"]) for e in walk]

    bits = []
    for e in walk:
        for t in list(_tokens(e["flags_mask"])) + list(_tokens(e["flags_cmp"])):
            if t not in bits:
                bits.append(t)
    bit_index = {t: i for i, t in enumerate(bits)}

    axes = [_axis_values(walk, f) for f in TYPE_AXES]
    input_values = [ANY_INPUT] + sorted(
        {e["input_colour_type"] for e in walk if e["input_colour_type"] not in (None, ANY_INPUT)}) + [OTHER]
    i_input = len(axes)
    axes.append(input_values)
    i_size = len(axes)
    widths = sorted({e["map_width"] for e in walk if e["map_width"]})
    heights = sorted({e["map_height"] for e in walk if e["map_height"]})
    sizes = [SIZE_ZERO] + [(w, h) for w in widths + [OTHER_SIZE]
                           for h in heights + [OTHER_SIZE]]
    axes.append(sizes)
    i_tex = TYPE_AXES.index("texture_type")

    masks, n_states = _value_masks(axes)
    index_of = [{v: i for i, v in enumerate(values)} for values in axes]

    # A state is a real primitive only if an unbound texture (PMT_NONE) has no
    # size; every other texture value is a bound buffer and admits any size.
    valid = 0
    for v, m in enumerate(masks[i_tex]):
        valid |= (m & masks[i_size][0]) if axes[i_tex][v] == NONE else m

    width_mask = collections.defaultdict(int)
    height_mask = collections.defaultdict(int)
    for i, (w, h) in enumerate(sizes):
        width_mask[w] |= masks[i_size][i]
        height_mask[h] |= masks[i_size][i]

    def pred(e):
        mask = cmpmask = 0
        for t in _tokens(e["flags_mask"]):
            mask |= 1 << bit_index[t]
        for t in _tokens(e["flags_cmp"]):
            cmpmask |= 1 << bit_index[t]
        return mask, cmpmask

    def type_states(e):
        m = valid
        for a, f in enumerate(TYPE_AXES):
            v = e[f]
            if v != NONE:
                m &= masks[a][index_of[a][v]]
        ic = e["input_colour_type"]
        if ic != ANY_INPUT:
            m &= masks[i_input][index_of[i_input][ic]]
        if e["map_width"]:
            m &= width_mask.get(e["map_width"], 0)
        if e["map_height"]:
            m &= height_mask.get(e["map_height"], 0)
        return m

    preds = [pred(e) for e in walk]
    tstates = [type_states(e) for e in walk]

    reachable, matched = set(), set()
    default_fired = False
    for fv in range(1 << len(bits)):
        available = valid
        for k, e in enumerate(walk):
            mask, cmpmask = preds[k]
            if (fv & mask) != cmpmask:
                continue
            sel = available & tstates[k]
            if sel:
                reachable.add(keys[k])
                matched.add(keys[k])
                available &= ~sel
            if not available:
                break
        if available:
            # Nothing in the walk matched: renderBegin() falls through to the
            # last entry of the general table (the last element of `walk`).
            reachable.add(keys[-1])
            default_fired = True

    return reachable, matched, default_fired, n_states


def walk_config(entries_by_table, mmx):
    """-> per (format, topology) walk results for one MMX configuration.

    entries_by_table maps a census table name to its entries in table order.
    """
    result = {}
    for ij, general in GENERAL.items():
        walk = []
        if mmx and ij in MMX and MMX[ij] in entries_by_table:
            walk += entries_by_table[MMX[ij]]
        walk += entries_by_table[general]
        reachable, matched, default_fired, n_states = _walk_reachable(walk)
        result[ij] = {
            "tables": ([MMX[ij]] if mmx and ij in MMX and MMX[ij] in entries_by_table else []) + [general],
            "walk": [(e["table"], e["index"]) for e in walk],
            "reachable": reachable,
            "matched": matched,
            "default_fired": default_fired,
            "n_states": n_states,
        }
    return result


def config_reachable(entries_by_table, mmx):
    per = walk_config(entries_by_table, mmx)
    reachable = set()
    matched = set()
    for w in per.values():
        reachable |= w["reachable"]
        matched |= w["matched"]
    return reachable, matched, per


def _group(entries):
    by_table = collections.defaultdict(list)
    for e in entries.values():
        by_table[e["table"]].append(e)
    for table in by_table:
        by_table[table].sort(key=lambda e: e["index"])
    return by_table


def main():
    if len(sys.argv) < 2 or sys.argv[1].startswith("-"):
        print(__doc__.strip(), file=sys.stderr)
        return 2
    build = sys.argv[1]
    census_dirs = [sys.argv[i + 1] for i, a in enumerate(sys.argv) if a == "--census"]
    json_out = sys.argv[sys.argv.index("--json") + 1] if "--json" in sys.argv else None

    entries = witness.parse_tables(build)
    if not entries:
        print("no generated tables under %s/drivers/pentprim - is the build done?" % build,
              file=sys.stderr)
        return 2
    by_table = _group(entries)
    all_keys = sorted(entries, key=lambda k: (k[0], k[1]))

    configs = {}
    for name, mmx in (("mmx", True), ("nommx", False)):
        reachable, matched, per = config_reachable(by_table, mmx)
        configs[name] = {
            "reachable": reachable,
            "matched": matched,
            "per_walk": per,
            "dead": [k for k in all_keys if k not in reachable],
        }

    print("build:   %s" % build)
    print("entries: %d in %d tables" % (len(entries), len({t for t, _ in all_keys})))

    print("\n== per configuration ==")
    print("%-22s %9s %9s %9s" % ("configuration", "reachable", "matched", "dead"))
    for name, label in (("mmx", "MMX on (default)"), ("nommx", "MMX off")):
        c = configs[name]
        print("%-22s %9d %9d %9d"
              % (label, len(c["reachable"]), len(c["matched"]), len(c["dead"])))

    print("\n== per table ==")
    print("%-10s %6s %9s %9s %9s %9s" % ("table", "blocks", "mmx:live", "mmx:dead",
                                         "no:live", "no:dead"))
    for table in sorted(by_table):
        keys = [(table, e["index"]) for e in by_table[table]]
        row = [table, str(len(keys))]
        for name in ("mmx", "nommx"):
            live = len([k for k in keys if k in configs[name]["reachable"]])
            row += [str(live), str(len(keys) - live)]
        print("%-10s %6s %9s %9s %9s %9s" % tuple(row))

    mmx_on, no = configs["mmx"]["reachable"], configs["nommx"]["reachable"]
    both = sorted(mmx_on & no)
    only_on = sorted(mmx_on - no)
    only_off = sorted(no - mmx_on)
    dead = sorted(set(all_keys) - (mmx_on | no))

    print("\n== classification ==")
    print("reachable in both configurations: %d" % len(both))
    print("reachable only with MMX on:       %d" % len(only_on))
    print("reachable only with MMX off:      %d" % len(only_off))
    print("dead in both:                     %d" % len(dead))

    print("\n== default fall-through, per walk ==")
    for name, label in (("mmx", "MMX on"), ("nommx", "MMX off")):
        walks = configs[name]["per_walk"]
        fired = [(ij, w) for ij, w in walks.items() if w["default_fired"]]
        print("%-8s %d of %d walks reach the default" % (label, len(fired), len(walks)))
        for ij, w in sorted(fired):
            last = w["walk"][-1]
            print("   (format,topology)=(%d,%d) falls through to %s#%d" % (ij[0], ij[1], last[0], last[1]))

    def dump(title, keys):
        print("\n%s (%d):" % (title, len(keys)))
        for k in keys:
            print("  %-10s %4d  %s" % (k[0], k[1], entries[k]["identifier"]))

    dump("entries no state selects in either configuration (dead)", dead)
    dump("entries reachable only with MMX off (shadowed by the MMX table when on)", only_off)

    out = {
        "build": build,
        "entries": len(entries),
        "configs": {
            name: {"reachable": [list(k) for k in sorted(c["reachable"])],
                   "matched": [list(k) for k in sorted(c["matched"])],
                   "dead": [list(k) for k in c["dead"]],
                   "per_walk": {"%d,%d" % ij: {
                       "tables": w["tables"],
                       "default_fired": w["default_fired"],
                       "n_states": w["n_states"],
                       "reachable": [list(k) for k in sorted(w["reachable"])],
                       "matched": [list(k) for k in sorted(w["matched"])]}
                       for ij, w in c["per_walk"].items()}}
            for name, c in configs.items()},
    }

    if census_dirs:
        rows = []
        for d in census_dirs:
            rows += witness.load_logs(d)
        bad = [r for r in rows
               if (r[4], r[5]) in entries and entries[(r[4], r[5])]["identifier"] != r[7]]
        unknown = sorted({(r[4], r[5]) for r in rows if (r[4], r[5]) not in entries})
        selected = {(r[4], r[5]) for r in rows}
        if bad:
            print("\n!! %d log lines disagree with the table they index, e.g. %s#%d"
                  % (len(bad), bad[0][4], bad[0][5]))
        if unknown:
            print("\n!! %d (table, index) pairs are not in the tables, e.g. %r"
                  % (len(unknown), unknown[:3]))
        selectable = mmx_on | no
        sel_only = sorted(selected - selectable)
        gap = sorted(selectable - selected)
        print("\n== cross with the corpus ==")
        print("census:            %s" % ", ".join(census_dirs))
        print("selections:        %d   distinct entries selected: %d" % (len(rows), len(selected)))
        print("selected:          %d" % len(selected))
        print("selectable, not selected: %d" % len(gap))
        print("not selectable at all:    %d" % len(dead))
        if sel_only:
            print("!! selected but the walk calls dead: %d" % len(sel_only))
            for k in sel_only:
                print("   %s#%d" % k)
        dump("selectable but not selected", gap)
        out["census"] = {
            "dirs": census_dirs,
            "selections": len(rows),
            "selected": [list(k) for k in sorted(selected)],
            "selectable_not_selected": [list(k) for k in gap],
            "selected_but_dead": [list(k) for k in sel_only],
        }

    if json_out:
        json.dump(out, open(json_out, "w"), indent=1, sort_keys=True)
        print("\nwrote %s" % json_out)
    return 0


if __name__ == "__main__":
    sys.exit(main())
