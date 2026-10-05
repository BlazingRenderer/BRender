#!/usr/bin/env python3
"""Build the apply plan from the TRM transcripts.

For every Br* function transcribed, find the header that declares it and whether
it already carries a /** doc block. Report, per header, what still needs writing.
"""
import glob, os, re, collections

# <repo>/contrib/trm/plan.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DIRS = ["core/inc", "core/fmt/include", "core/fw/include", "core/math/include"]

def headers():
    out = []
    for d in DIRS:
        for root, _, files in os.walk(os.path.join(REPO, d)):
            for f in files:
                if f.endswith(".h"):
                    out.append(os.path.join(root, f))
    return out
HDRS = headers()

def parse(path):
    entries = {}
    name, buf = None, []
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"#{1,3}\s+([A-Za-z_][A-Za-z0-9_]*)\s*$", line)
        if m:
            if name:
                entries.setdefault(name, []).append(buf)
            name, buf = m.group(1), []
        elif name is not None:
            buf.append(line)
    if name:
        entries.setdefault(name, []).append(buf)
    return {k: v for k, v in entries.items() if re.match(r"Br[A-Z]", k)}

def score(buf):
    return sum(1 for l in buf if re.match(r"\s*(description|declaration|arguments|result|effects|preconditions|remarks|see_also):", l))

def find_decl(name):
    pat = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    res = []
    for h in HDRS:
        lines = open(h, encoding="utf-8", errors="replace").read().splitlines()
        for i, line in enumerate(lines):
            if pat.search(line) and line.rstrip().endswith(";") and not line.lstrip().startswith("#"):
                documented = False
                j = i - 1
                while j >= 0 and lines[j].strip() == "":
                    j -= 1
                if j >= 0 and lines[j].strip().endswith("*/"):
                    k = j
                    while k >= 0 and not lines[k].strip().startswith("/*"):
                        k -= 1
                    if k >= 0 and lines[k].strip().startswith("/**"):
                        documented = True
                res.append((os.path.relpath(h, REPO), i + 1, documented))
    return res

def main():
    all_entries = {}
    for f in sorted(glob.glob("/tmp/trm-extract/w1-*.md")):
        for name, occs in parse(f).items():
            for buf in occs:
                if name not in all_entries or score(buf) > score(all_entries[name]):
                    all_entries[name] = buf
    todo = collections.defaultdict(list)
    done = collections.defaultdict(list)
    missing = []
    for name in sorted(all_entries):
        hits = find_decl(name)
        if not hits:
            missing.append(name)
            continue
        hdr, ln, doc = hits[0]
        (done if doc else todo)[hdr].append(name)
    print(f"transcribed {len(all_entries)} Br* functions\n")
    print("=== TO DOCUMENT ===")
    for h in sorted(todo):
        print(f"  {h}: {len(todo[h])}")
    for h in sorted(todo):
        print(f"\n--- {h} ---")
        print("  " + " ".join(todo[h]))
    print("\n=== ALREADY DOCUMENTED (cross-check only) ===")
    for h in sorted(done):
        print(f"  {h}: {len(done[h])}  ({' '.join(done[h])})")
    print(f"\n=== NO DECLARATION ({len(missing)}) ===")
    print("  " + " ".join(missing))

if __name__ == "__main__":
    main()
