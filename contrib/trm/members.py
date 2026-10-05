#!/usr/bin/env python3
"""Apply TRM member docs to a struct definition.

Reads the `## <struct>` sections of a wave-4 transcript and rewrites the
documentation comment above each member in the header that defines the struct.

A member is transcribed as

    - name — first paragraph
      <body>

    - name2, name3 — ...

and the body is grouped into blank-line-separated blocks.  A block that is
fenced (```), a list (`- ` or `N. `) or a Markdown table is kept structured;
anything else is prose.  Fenced blocks with a `c` info string become
\\code{.c}; all other fenced blocks become \\verbatim.

br_pixelmap is deliberately absent: its members live in the
BR_PIXELMAP_MEMBERS_PREFIXED() macro and are documented by hand.

Usage:
    members.py <transcript.md> [--struct NAME]... [--apply] [--print]
"""
import os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen

# <repo>/contrib/trm/members.py
REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
SEARCH = ["core/inc", "core/fmt/include", "core/fw/include", "core/math/include"]

# The transcript names a struct the manual documents; the code may name it
# differently, or split it into several.  br_pixelmap is handled by hand.
TARGETS = {
    "br_bounds": ["br_bounds2", "br_bounds3", "br_bounds4"],
}


def parse_transcript(path):
    """Return {struct: [{'names': [...], 'lines': [...]}]}."""
    sections, cur, in_members, member = {}, None, False, None
    for line in open(path, encoding="utf-8").read().splitlines():
        m = re.match(r"^##\s+(\S+)\s*$", line)
        if m:
            name = m.group(1)
            if name == "Notes":
                cur, in_members, member = None, False, None
            else:
                cur, in_members, member = name, False, None
                sections[cur] = []
            continue
        if cur is None:
            continue
        if line.strip() == "members:":
            in_members = True
            continue
        if not in_members:
            continue
        m = re.match(r"^- (.*?) — (.*)$", line)
        if m:
            member = {"names": [n.strip() for n in m.group(1).split(",")],
                      "lines": [m.group(2)]}
            sections[cur].append(member)
            continue
        if member is not None:
            member["lines"].append(line)
    for members in sections.values():
        for m in members:
            while m["lines"] and not m["lines"][-1].strip():
                m["lines"].pop()
    return sections


def indent(line):
    return len(line) - len(line.lstrip())


def segment(lines):
    """Walk body lines, yielding (kind, data) in document order."""
    items, para = [], []

    def flush():
        if para:
            items.append(("prose", list(para)))
            para.clear()

    i = 0
    while i < len(lines):
        line = lines[i]
        s = line.strip()
        if not s:
            flush()
            i += 1
            continue
        if s.startswith("```"):
            flush()
            j = i + 1
            while j < len(lines) and not lines[j].strip().startswith("```"):
                j += 1
            items.append(("code", (s[3:].strip(), lines[i + 1:j])))
            i = j + 1
            continue
        if s.startswith("|"):
            flush()
            j = i
            while j < len(lines) and lines[j].strip().startswith("|"):
                j += 1
            items.append(("table", lines[i:j]))
            i = j
            continue
        m = re.match(r"^\s*- (.*)$", line)
        if m:
            flush()
            items.append(("bullet", m.group(1)))
            i += 1
            continue
        if re.match(r"^\s*\d+\.\s", line):
            flush()
            items.append(("ordered", s))
            i += 1
            continue
        if indent(line) >= 4 and not para:
            flush()
            j = i
            run = []
            while j < len(lines) and (not lines[j].strip() or indent(lines[j]) >= 4):
                run.append(lines[j])
                j += 1
            items.append(("code", ("c", [x[4:] for x in run if x.strip()])))
            i = j
            continue
        para.append(s)
        i += 1
    flush()
    return items


def strip_code_comments(text):
    return re.sub(r"/\*.*?\*/", "", text, flags=re.S)


def strip_pages(s):
    """Drop the manual's page subscripts, but keep "page N" cross-references."""
    s = re.sub(r"(\bBr[A-Za-z0-9_\[\]|]+\([^)]*\))\s*\d{1,3}(?:/\d+)?\b", r"\1", s)
    s = re.sub(r"(\bbr_[a-z0-9_]+)\s+\d{2,3}\b", r"\1", s)
    s = re.sub(r"(\bbr_[a-z0-9_]+\(\))\s*\d{1,3}\b", r"\1", s)
    s = re.sub(r"\bCBFn([A-Za-z0-9_]+)\s*\(\s*\)\s*\d*",
               lambda m: gen.CBFN_MAP.get(m.group(1), ""), s)
    s = re.sub(r"\s*,\s*,", ", ", s)
    return re.sub(r"^[\s,;]+|[\s,;]+$", "", s)


def render(member):
    """Return the doc block lines for one member."""
    head = strip_pages(member["lines"][0].strip())
    brief, body = gen.split_brief(head)
    out = ["/**"]
    gen.wrap(out, "\\brief ", brief)
    seq = []
    if body.strip():
        seq.append(("prose", [body]))
    for kind, data in segment(member["lines"][1:]):
        if kind in ("bullet", "ordered"):
            if seq and seq[-1][0] == "list":
                seq[-1][1].append(strip_pages(data))
            else:
                seq.append(("list", [strip_pages(data)]))
        else:
            seq.append((kind, data))
    for kind, data in seq:
        if kind == "prose":
            text = strip_pages(gen.oneline(data))
            if not text:
                continue
            out.append(" *")
            m = re.match(r"^(?:For [Ee]xample|Example)\s*:\s*(.*)$", text)
            if m:
                out.append(" * \\par Example")
                gen.wrap(out, "", m.group(1))
            else:
                gen.wrap(out, "", text)
        elif kind == "list":
            out.append(" *")
            for item in data:
                gen.wrap(out, "\\li ", item)
        elif kind == "table":
            out.append(" *")
            for line in data:
                out.append(" * " + line.strip())
        elif kind == "code":
            info, code = data
            text = strip_code_comments("\n".join(code))
            is_c = info.startswith("c")
            out.append(" *")
            out.append(" * \\code{.c}" if is_c else " * \\verbatim")
            for line in text.splitlines():
                out.append(" * " + line.rstrip())
            out.append(" * \\endcode" if is_c else " * \\endverbatim")
    out.append(" */")
    return [(l.replace("*/", "*\\/") if i < len(out) - 1 else l)
            for i, l in enumerate(out)]


def comment_mask(lines, start, end):
    """True for lines that lie inside a block comment."""
    mask, in_comment = [False] * len(lines), False
    for i in range(start, end + 1):
        s = lines[i].strip()
        mask[i] = in_comment or s.startswith("/*")
        if in_comment:
            if "*/" in s:
                in_comment = False
        elif s.startswith("/*") and "*/" not in s:
            in_comment = True
    return mask


def find_decl(lines, start, end, name, mask):
    base = re.sub(r"\[.*\]$", "", name)
    pat = re.compile(r"\b" + re.escape(base) + r"\s*(\[[^\]]*\])*\s*;")
    for i in range(start, end + 1):
        if not mask[i] and pat.search(lines[i]):
            return i
    return None


def find_path_decl(lines, start, end, parts, mask):
    leaf = parts[-1]
    sub = parts[-2] if len(parts) == 3 else None
    if sub is None:
        return find_decl(lines, start, end, leaf, mask)
    close = None
    for i in range(start, end + 1):
        if re.match(r"^\s*\}\s*" + re.escape(sub) + r"\s*;", lines[i]):
            close = i
            break
    if close is None:
        return None
    open_ = None
    for j in range(close - 1, start - 1, -1):
        if re.match(r"^\s*struct\s*\{", lines[j]):
            open_ = j
            break
    if open_ is None:
        return None
    return find_decl(lines, open_ + 1, close - 1, leaf, mask)


def strip_inline_comment(line):
    """Drop a `foo; /* comment */` trailing comment (the docs supersede it)."""
    m = re.match(r"^(.*?\S)\s*/\*.*?\*/\s*$", line)
    if m and "/*" not in m.group(1):
        return m.group(1)
    return line


def replace_above(lines, idx, block):
    j = idx - 1
    while j >= 0 and lines[j].strip() == "":
        j -= 1
    # Only a comment that is the whole line counts: a member declaration with a
    # trailing /* ... */ comment must not be mistaken for the block above the
    # next member.
    s = lines[j].strip() if j >= 0 else ""
    if s == "*/" or s.startswith("/*"):
        k = j
        while k >= 0 and "/*" not in lines[k]:
            k -= 1
        if k >= 0:
            lines[k:j + 1] = block
            return "replaced"
    lines[idx:idx] = block
    return "inserted"


def find_struct(lines, name):
    start = end = None
    open_pat = re.compile(r"\bstruct\s+" + re.escape(name) + r"\s*\{")
    close_pat = re.compile(r"^\}\s*" + re.escape(name) + r"\s*;")
    for i, line in enumerate(lines):
        if start is None and open_pat.search(line):
            start = i
        elif start is not None and close_pat.match(line):
            end = i
            break
    return start, end


def header_for(name):
    for d in SEARCH:
        for root, _, files in os.walk(os.path.join(REPO, d)):
            for f in files:
                if not f.endswith(".h"):
                    continue
                path = os.path.join(root, f)
                lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
                if find_struct(lines, name)[0] is not None:
                    return os.path.relpath(path, REPO)
    return None


def apply_struct(struct, members, do_write):
    header = header_for(struct)
    if header is None:
        print(f"{struct}: struct not found in the tree")
        return 0
    path = os.path.join(REPO, header)
    lines = open(path, encoding="utf-8").read().splitlines()
    start, end = find_struct(lines, struct)
    if start is None:
        print(f"{struct}: struct not found in {header}")
        return 0
    mask = comment_mask(lines, start, end)
    jobs, missing = [], []
    for member in members:
        for name in member["names"]:
            idx = (find_path_decl(lines, start, end, name.split("."), mask)
                   if "." in name else
                   find_decl(lines, start, end, name, mask))
            if idx is None:
                missing.append(name)
            else:
                pad = re.match(r"^(\s*)", lines[idx]).group(1)
                block = [pad + l if l.strip() else l for l in render(member)]
                jobs.append((idx, name, block))
    print(f"{struct} ({header}): {len(jobs)} members"
          + (f", MISSING {missing}" if missing else ""))
    for idx, name, block in sorted(jobs, reverse=True):
        lines[idx] = strip_inline_comment(lines[idx])
    for idx, name, block in sorted(jobs, reverse=True):
        action = replace_above(lines, idx, block)
        print(f"  {action:8} {name}")
    if do_write:
        open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    return len(jobs)


def main():
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return
    path = args[0]
    only = [args[i + 1] for i, a in enumerate(args) if a == "--struct"]
    do_write = "--apply" in args
    show = "--print" in args
    sections = parse_transcript(path)
    for struct, members in sections.items():
        if only and struct not in only:
            continue
        if show:
            for member in members:
                print(f"===== {struct}: {', '.join(member['names'])} =====")
                print("\n".join(render(member)))
        else:
            for target in TARGETS.get(struct, [struct]):
                apply_struct(target, members, do_write)


if __name__ == "__main__":
    main()
