#!/usr/bin/env python3
"""Sync generated blocks into headers: replace the existing doc block above a
declaration if one is present, otherwise insert."""
import re, glob, os, sys

# <repo>/contrib/trm/sync.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ONLY = set(sys.argv[1:])

def parse_dump(dm):
    header = open(dm, encoding="utf-8").readline().strip().lstrip("# ").split(" — ")[0].strip()
    decls = {}
    for b in re.split(r"\n===== ", open(dm, encoding="utf-8").read())[1:]:
        head, _, body = b.partition("\n")
        name = head.split("  (line")[0].strip()
        for line in body.splitlines():
            if line.startswith("DECLARATION: "):
                decls[name] = line[len("DECLARATION: "):].strip()
                break
    return header, decls

def parse_blocks(bp):
    blocks = {}
    for b in re.split(r"\n===== ", open(bp, encoding="utf-8").read())[1:]:
        head, _, body = b.partition("\n")
        name = head.split(" =====")[0].strip()
        body = "\n".join(l for l in body.splitlines() if not l.startswith("// FLAG:"))
        blocks[name] = body.rstrip("\n")
    return blocks

def find_anchor(L, anchor):
    in_c = False
    for i, line in enumerate(L):
        s = line.strip()
        if in_c:
            if "*/" in s:
                in_c = False
            continue
        if s.startswith("/*"):
            if "*/" not in s:
                in_c = True
            continue
        if s == anchor and not s.startswith("#"):
            return i
    return None

for dm in sorted(glob.glob("/tmp/apply/*.md")):
    base = os.path.basename(dm)[:-3]
    bp = "/tmp/gen/" + base + ".blocks"
    if not os.path.exists(bp):
        continue
    header, decls = parse_dump(dm)
    blocks = parse_blocks(bp)
    hpath = os.path.join(REPO, header)
    L = open(hpath, encoding="utf-8").read().splitlines()
    jobs = []
    for name, anchor in decls.items():
        if ONLY and name not in ONLY:
            continue
        if name not in blocks or blocks[name].startswith("FLAG:"):
            continue
        i = find_anchor(L, anchor)
        if i is not None:
            jobs.append((i, name))
    replaced = inserted = 0
    for i, name in sorted(jobs, reverse=True):
        j = i - 1
        while j >= 0 and L[j].strip() == "":
            j -= 1
        k = -1
        if j >= 0 and L[j].strip().endswith("*/"):
            k = j
            while k >= 0 and not L[k].strip().startswith("/*"):
                k -= 1
            if k >= 0 and not L[k].strip().startswith("/**"):
                k = -1
        if k >= 0:
            L[k:j + 1] = blocks[name].splitlines()
            replaced += 1
        else:
            L[i:i] = blocks[name].splitlines()
            inserted += 1
    open(hpath, "w", encoding="utf-8").write("\n".join(L) + "\n")
    print(f"{header}: replaced {replaced}, inserted {inserted}")
