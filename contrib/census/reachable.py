#!/usr/bin/env python3
"""
Reachability of each emitted block tuple under the matcher's own semantics.

"Unwitnessed" splits into two very different things: no state the corpus builds
happens to select the block, and no state can select it at all. The second is
decidable without a renderer. It is the same walk spFindMatch performs, over the
finite space of states the flag predicates and the type/size requirements can
distinguish:

  * flags are only ever tested through a mask/compare pair, so the space that
    matters is the assignment of the PRIMF_* tokens any entry names;
  * a type requirement is exact equality against the bound buffer's type (or
    "any" when the entry names SP_PMT_NONE), so one representative value per
    named type plus one value no entry names covers every case;
  * a map-size requirement is "width == N and height == N", so one
    representative size per named N plus one unnamed size, crossed with itself
    to catch the mismatch case, covers every case.

For each flag assignment the walk is run over every representative state, marking
the first entry whose predicate holds - exactly the fall-through spFindMatch
implements. A tuple no (flags, state) pair selects is dead code in the table: a
candidate for deletion rather than a fixture.

Usage:  reachable.py <build-dir>            (prints the dead tuples)
        (import as a module: reachable_shapes(entries))
"""

import collections
import itertools
import os
import sys

# A value no entry requires, used to represent "any other type/size". It only
# has to be distinct from every value the entries name.
OTHER = "SP_OTHER_TYPE"
OTHER_SIZE = 1000


def _tokens(field):
    return {t.strip() for t in field.split("|") if t.strip() and t.strip() != "0"}


def reachable_shapes(entries):
    """-> set of emitted axis tuples some state selects."""
    emitted, _ = _reachable(entries)
    return emitted


def reachable_refused_shapes(entries):
    """-> set of refused axis tuples some state selects.

    A refused entry stops the walk, so a state that selects one draws nothing.
    These are reachable-by-construction the same way the emitted ones are; the
    difference is that no kernel answers them.
    """
    _, refused = _reachable(entries)
    return refused


def _reachable(entries):
    """-> (emitted tuples some state selects, refused tuples some state selects)"""
    walks = collections.defaultdict(list)
    for e in entries:
        walks[(e["axes"][0], e["axes"][1])].append(e)

    emitted = set()
    refused = set()
    for walk in walks.values():
        e, r = _walk_reachable(walk)
        emitted |= e
        refused |= r
    return emitted, refused


def _walk_reachable(walk):
    # Distinct flag tokens named anywhere, in a fixed order so a bit position is
    # stable within this walk.
    bits = []
    for e in walk:
        for t in itertools.chain(_tokens(e["flags_mask"]), _tokens(e["flags_cmp"])):
            if t not in bits:
                bits.append(t)
    bit_index = {t: i for i, t in enumerate(bits)}

    def pred(e):
        mask = 0
        for t in _tokens(e["flags_mask"]):
            mask |= 1 << bit_index[t]
        cmpv = 0
        for t in _tokens(e["flags_cmp"]):
            cmpv |= 1 << bit_index[t]
        return mask, cmpv

    preds = [pred(e) for e in walk]

    def type_values(field):
        vals = {e["types"][field] for e in walk if e["types"][field] != "SP_PMT_NONE"}
        return sorted(vals) + [OTHER]

    depth_vals = ["SP_PMT_NONE"] + type_values(0)
    tex_vals = ["SP_PMT_NONE"] + type_values(1)
    shade_vals = ["SP_PMT_NONE"] + type_values(2)
    blend_vals = ["SP_PMT_NONE"] + type_values(3)
    fog_vals = ["SP_PMT_NONE"] + type_values(4)

    named_sizes = sorted({e["msize"] for e in walk if e["msize"]})
    sizes = named_sizes + [OTHER_SIZE]
    # every (w, h) combination the exact-size predicate can distinguish.
    pairs = [(w, h) for w in sizes for h in sizes]

    # ---- build the representative states and a bitmask per requirement value.
    dim_masks = {k: collections.defaultdict(int) for k in ("depth", "tex", "shade", "blend", "fog", "size")}
    size_masks = collections.defaultdict(int)
    n = 0
    for depth, tex, shade, blend, fog in itertools.product(depth_vals, tex_vals, shade_vals, blend_vals, fog_vals):
        # An unbound buffer is NONE with no map; a bound one has a positive size.
        ss = [(0, 0)] if tex == "SP_PMT_NONE" else pairs
        for w, h in ss:
            bit = 1 << n
            dim_masks["depth"][depth] |= bit
            dim_masks["tex"][tex] |= bit
            dim_masks["shade"][shade] |= bit
            dim_masks["blend"][blend] |= bit
            dim_masks["fog"][fog] |= bit
            dim_masks["size"][(w, h)] |= bit
            if w == h:
                size_masks[w] |= bit
            n += 1
    all_states = (1 << n) - 1

    def type_states(e):
        m = all_states
        req = e["types"]
        for k, v in zip(("depth", "tex", "shade", "blend", "fog"), req):
            if v != "SP_PMT_NONE":
                m &= dim_masks[k][v]
        if e["msize"]:
            m &= size_masks.get(e["msize"], 0)
        return m

    tstates = [type_states(e) for e in walk]

    reachable = set()
    refused_reachable = set()
    for fv in range(1 << len(bits)):
        available = all_states
        for k, e in enumerate(walk):
            mask, cmpv = preds[k]
            if (fv & mask) != cmpv:
                continue
            sel = available & tstates[k]
            if sel:
                if e["kind"] == "emitted":
                    reachable.add(e["axes"])
                else:
                    refused_reachable.add(e["axes"])
                # A refused entry stops the walk: its states can select nothing.
                available &= ~sel
            if not available:
                break
    return reachable, refused_reachable


def main():
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from witness import parse_inc, short  # noqa

    entries = parse_inc(os.path.join(sys.argv[1], "drivers/softprim/softprim_matchers.inc"))
    emitted = {e["axes"] for e in entries if e["kind"] == "emitted"}
    reach = reachable_shapes(entries)
    dead = emitted - reach
    print("emitted tuples:       %d" % len(emitted))
    print("reachable by a state: %d" % len(reach & emitted))
    print("unreachable (dead):   %d" % len(dead))
    for s in sorted(dead, key=short):
        ids = [e["identifier"] for e in entries if e["axes"] == s]
        print("  %-70s x%d" % (short(s), len(ids)))


if __name__ == "__main__":
    main()
