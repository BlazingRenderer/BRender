#!/usr/bin/env python3
"""Find stale comments above .c definitions of functions documented from the TRM."""
import re, glob, os

# <repo>/contrib/trm/cscan.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

names = set()
for dm in glob.glob("/tmp/apply/*.md"):
    for b in re.split(r"\n===== ", open(dm, encoding="utf-8").read())[1:]:
        head, _, _ = b.partition("\n")
        names.add(head.split("  (line")[0].strip())

def defs(f):
    L = open(f, encoding="utf-8", errors="replace").read().splitlines()
    for i, line in enumerate(L):
        if re.match(r"^\S", line) and "(" in line and ";" not in line:
            for nm in names:
                if re.search(r"\b" + re.escape(nm) + r"\s*\(", line):
                    yield nm, i, L
                    break

hits = []
for f in sorted(glob.glob(os.path.join(REPO, "core", "**", "*.c"), recursive=True)):
    for nm, i, L in defs(f):
        j = i - 1
        while j >= 0 and L[j].strip() == "":
            j -= 1
        if j >= 0 and L[j].strip().endswith("*/"):
            k = j
            while k >= 0 and not L[k].strip().startswith("/*"):
                k -= 1
            block = "\n".join(L[k:j + 1])
            if not L[k].strip().startswith("/**"):
                hits.append((os.path.relpath(f, REPO), k + 1, nm, block))

print(f"stale .c comment blocks above documented definitions: {len(hits)}\n")
import collections
c = collections.Counter(h[0] for h in hits)
for f, n in c.most_common():
    print(f"{f}: {n}")
print()
for f, ln, nm, block in hits[:40]:
    first = block.splitlines()[0][:70]
    print(f"{f}:{ln}  [{nm}]  {first}")
