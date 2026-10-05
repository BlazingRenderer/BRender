#!/usr/bin/env python3
"""Write /tmp/apply/<header>.md: for each undocumented transcribed function,
its real declaration plus the verbatim transcript fields."""
import glob, os, re, collections, sys

# <repo>/contrib/trm/dump.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DIRS = ["core/inc", "core/fmt/include", "core/fw/include", "core/math/include"]
HDRS = [os.path.join(r, f) for d in DIRS for r, _, fs in os.walk(os.path.join(REPO, d)) for f in fs if f.endswith(".h")]

def parse(path):
    entries, name, buf = {}, None, []
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
    return {k: v for k, v in entries.items()
            if re.match(r"Br[A-Z]", k) or re.match(r"br[a-z0-9_]*_cbfn$", k)}

def score(buf):
    return sum(1 for l in buf if re.match(r"\s*(description|declaration|arguments|result|effects|preconditions|remarks|see_also):", l))

def find_decl(name):
    pat = re.compile(r"\b" + re.escape(name) + r"\s*\(")
    for h in HDRS:
        lines = open(h, encoding="utf-8", errors="replace").read().splitlines()
        in_comment = False
        for i, line in enumerate(lines):
            s = line.strip()
            if in_comment:
                if "*/" in s:
                    in_comment = False
                continue
            if s.startswith("/*"):
                if "*/" not in s:
                    in_comment = True
                continue
            if pat.search(line) and not s.startswith("#"):
                sig = [s]
                j = i
                while not sig[-1].endswith(";") and j + 1 < len(lines):
                    j += 1
                    sig.append(lines[j].strip())
                if not sig[-1].endswith(";"):
                    continue
                doc = False
                k = i - 1
                while k >= 0 and lines[k].strip() == "":
                    k -= 1
                if k >= 0 and lines[k].strip().endswith("*/"):
                    m = k
                    while m >= 0 and not lines[m].strip().startswith("/*"):
                        m -= 1
                    doc = m >= 0 and lines[m].strip().startswith("/**")
                return os.path.relpath(h, REPO), i + 1, line.strip(), doc, " ".join(sig)
    return None

def main():
    all_entries = {}
    for f in sorted(glob.glob("/tmp/trm-extract/w*.md")):
        for name, occs in parse(f).items():
            for buf in occs:
                if name not in all_entries or score(buf) > score(all_entries[name]):
                    all_entries[name] = buf
    include_doc = "--all" in sys.argv
    by_hdr = collections.defaultdict(list)
    for name in sorted(all_entries):
        d = find_decl(name)
        if d and (include_doc or not d[3]):
            by_hdr[d[0]].append((name, d[1], d[2], d[4], all_entries[name]))
    os.makedirs("/tmp/apply", exist_ok=True)
    for h, items in by_hdr.items():
        out = "/tmp/apply/" + h.replace("/", "__") + ".md"
        with open(out, "w", encoding="utf-8") as fh:
            fh.write(f"# {h} — {len(items)} functions to document\n")
            for name, ln, decl, sig, buf in items:
                fh.write(f"\n===== {name}  (line {ln}) =====\n")
                fh.write(f"DECLARATION: {decl}\n")
                fh.write(f"SIGNATURE: {sig}\n")
                fh.write("".join(buf).rstrip() + "\n")
        print(f"{out}  ({len(items)})")

if __name__ == "__main__":
    main()
