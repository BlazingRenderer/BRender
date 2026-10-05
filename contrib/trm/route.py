#!/usr/bin/env python3
"""Route TRM transcript entries to the header that declares them.

Reads /tmp/trm-extract/*.md transcripts (one '# FunctionName' or '## FunctionName'
heading per entry), finds each function's declaration in the tree, and groups by
header so the docs can be applied module by module.
"""
import glob, os, re, subprocess, sys, collections

# <repo>/contrib/trm/route.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SEARCH_DIRS = ["core/inc", "core/fmt/include", "core/fw/include",
               "core/math/include", "drivers"]

def headers():
    out = []
    for d in SEARCH_DIRS:
        for root, _, files in os.walk(os.path.join(REPO, d)):
            if "cmake-build" in root:
                continue
            for f in files:
                if f.endswith(".h"):
                    out.append(os.path.join(root, f))
    return out

HDRS = headers()

def parse(path):
    entries = {}
    name = None
    buf = []
    for line in open(path, encoding="utf-8", errors="replace"):
        m = re.match(r"#{1,3}\s+([A-Za-z_][A-Za-z0-9_]*)\s*$", line)
        if m:
            if name:
                entries[name] = buf
            name = m.group(1)
            buf = []
        elif name is not None:
            buf.append(line)
    if name:
        entries[name] = buf
    return entries

def decl_of(name):
    pat = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    hits = []
    for h in HDRS:
        try:
            for i, line in enumerate(open(h, encoding="utf-8", errors="replace"), 1):
                if pat.search(line) and ("BR_PUBLIC_ENTRY" in line or "BR_RESIDENT_ENTRY" in line
                                         or line.lstrip().startswith(("void ", "br_", "int ", "float "))):
                    hits.append((os.path.relpath(h, REPO), i, line.strip()))
        except OSError:
            pass
    return hits

def main():
    files = sorted(glob.glob("/tmp/trm-extract/*.md"))
    all_entries = {}
    for f in files:
        for name, buf in parse(f).items():
            if not re.match(r"Br[A-Z]", name):
                continue
            all_entries.setdefault(name, []).append((os.path.basename(f), buf))
    print(f"transcripts: {len(files)}  distinct functions: {len(all_entries)}")
    by_hdr = collections.defaultdict(list)
    missing = []
    multi = []
    for name, occs in sorted(all_entries.items()):
        hits = decl_of(name)
        if not hits:
            missing.append(name)
            continue
        hdrs = {h[0] for h in hits}
        if len(hdrs) > 1:
            multi.append((name, sorted(hdrs)))
        by_hdr[hits[0][0]].append(name)
    print(f"\n=== by header ({len(by_hdr)}) ===")
    for h in sorted(by_hdr):
        print(f"{h}: {len(by_hdr[h])}")
    print(f"\n=== no declaration ({len(missing)}) ===")
    print(" ".join(missing))
    if multi:
        print(f"\n=== declared in >1 header ({len(multi)}) ===")
        for n, hs in multi:
            print(f"  {n}: {hs}")

if __name__ == "__main__":
    main()
