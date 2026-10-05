#!/usr/bin/env python3
"""Insert generated Doxygen blocks above their declarations."""
import re, glob, os

# <repo>/contrib/trm/apply.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

def parse_dump(dm):
    header = open(dm).readline().strip().lstrip("# ").split(" — ")[0].strip()
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

def already_doc(out):
    j = len(out) - 1
    while j >= 0 and out[j].strip() == "":
        j -= 1
    if j < 0 or not out[j].strip().endswith("*/"):
        return False
    k = j
    while k >= 0 and not out[k].strip().startswith("/*"):
        k -= 1
    return k >= 0 and out[k].strip().startswith("/**")

total = 0
for dm in sorted(glob.glob("/tmp/apply/*.md")):
    base = os.path.basename(dm)[:-3]
    bp = "/tmp/gen/" + base + ".blocks"
    if not os.path.exists(bp):
        continue
    header, decls = parse_dump(dm)
    blocks = parse_blocks(bp)
    hpath = os.path.join(REPO, header)
    L = open(hpath, encoding="utf-8").read().splitlines()
    out, inserted, skipped = [], 0, 0
    for line in L:
        s = line.strip()
        hit = None
        for name, decl in list(decls.items()):
            if s == decl and name in blocks and not blocks[name].startswith("FLAG:"):
                hit = name
                break
        if hit:
            if already_doc(out):
                skipped += 1
                out.append(line)
                del decls[hit]
                continue
            out.append(blocks[hit])
            out.append(line)
            inserted += 1
            del decls[hit]
        else:
            out.append(line)
    open(hpath, "w", encoding="utf-8").write("\n".join(out) + "\n")
    print(f"{header}: inserted {inserted}, skipped(already doc) {skipped}, unmatched {len(decls)}")
    total += inserted
print("total inserted:", total)
