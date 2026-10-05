#!/usr/bin/env python3
"""Remove .c comments above definitions that are now redundant with the TRM docs."""
import re, glob, os, sys, collections

# <repo>/contrib/trm/cpurge.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
KEEP = ["Graphics Gems", "Carling", "Andrew", "float intermediates", "Based on",
        "Given:", "│", "┌", "┐", "└", "┘"]

names = set()
for dm in glob.glob("/tmp/apply/*.md"):
    for b in re.split(r"\n===== ", open(dm, encoding="utf-8").read())[1:]:
        head, _, _ = b.partition("\n")
        names.add(head.split("  (line")[0].strip())

def find_hits(path):
    L = open(path, encoding="utf-8", errors="replace").read().splitlines()
    hits = []
    for i, line in enumerate(L):
        if not re.match(r"^\S", line) or "(" not in line or ";" in line:
            continue
        nm = next((n for n in names if re.search(r"\b" + re.escape(n) + r"\s*\(", line)), None)
        if not nm:
            continue
        j = i - 1
        while j >= 0 and L[j].strip() == "":
            j -= 1
        if j < 0 or not L[j].strip().endswith("*/"):
            continue
        k = j
        while k >= 0 and not L[k].strip().startswith("/*"):
            k -= 1
        if k < 0 or L[k].strip().startswith("/**"):
            continue
        hits.append((k, j, nm, "\n".join(L[k:j + 1])))
    return L, hits

dry = "--apply" not in sys.argv
purged = kept = 0
for f in sorted(glob.glob(os.path.join(REPO, "core", "**", "*.c"), recursive=True)):
    L, hits = find_hits(f)
    if not hits:
        continue
    remove = []
    for k, j, nm, block in hits:
        if any(x in block for x in KEEP):
            print(f"KEEP  {os.path.relpath(f, REPO)}:{k+1}  {nm}")
            kept += 1
            continue
        remove.append((k, j, nm))
    if dry or not remove:
        purged += len(remove)
        continue
    for k, j, nm in sorted(remove, reverse=True):
        del L[k:j + 1]
    open(f, "w", encoding="utf-8").write("\n".join(L) + "\n")
    purged += len(remove)
print(f"\n{'would purge' if dry else 'purged'}: {purged}, kept: {kept}")
