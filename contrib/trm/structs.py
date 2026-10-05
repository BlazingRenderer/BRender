#!/usr/bin/env python3
"""Apply the TRM's struct-level ("The Structure") descriptions to the headers.

Reads transcript files of the form

    ## br_pixelmap
    BRender's pixel map structure, used for texture maps, ...

and writes a Doxygen block above the matching `typedef struct <name> {`.

Unlike the member docs this is not inside a macro, so the block is a normal
decorated comment.  Existing plain comments above the typedef are replaced; a
comment followed by a blank line is left alone (it is a note, not the type
description) and the block is inserted above it.
"""
import os, re, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen, members

# structs whose manual name differs from the code's
TARGETS = {"br_bounds": ["br_bounds2", "br_bounds3", "br_bounds4"]}
# structs with a code-specific note above them: insert, do not replace
KEEP = {"br_euler", "br_model"}


def parse(paths):
    out = {}
    for path in paths:
        cur = None
        for line in open(path, encoding="utf-8").read().splitlines():
            m = re.match(r"^##\s+(\S+)\s*$", line)
            if m:
                cur = m.group(1)
                out[cur] = []
                continue
            if cur is not None:
                out[cur].append(line.rstrip())
    return {k: "\n".join(v).strip() for k, v in out.items()}


def pmatrix(lines):
    """Convert rows printed as `( m00  m01 )` into pmatrix rows, or None."""
    rows = []
    for line in lines:
        m = re.match(r"^\(\s*(.*?)\s*\)$", line.strip())
        if not m:
            return None
        cells = []
        for tok in m.group(1).split():
            mm = re.match(r"^([A-Za-z]+)([0-9]+)$", tok)
            cells.append("%s_{%s}" % (mm.group(1), mm.group(2)) if mm else tok)
        rows.append(" & ".join(cells) + r" \\")
    return rows


def render(text):
    seq = []
    for kind, data in members.segment(text.splitlines()):
        if kind in ("bullet", "ordered"):
            if seq and seq[-1][0] == "list":
                seq[-1][1].append(members.strip_pages(data))
            else:
                seq.append(("list", [members.strip_pages(data)]))
        elif kind == "prose":
            lines = [l.rstrip() for l in data]
            rows = pmatrix(lines) if len(lines) > 1 else None
            if rows:
                seq.append(("math", rows))
            else:
                seq.append(("prose", members.strip_pages(gen.oneline(data))))
        else:
            seq.append((kind, data))
    out = ["/**"]
    first = True
    for kind, data in seq:
        if kind == "prose":
            if first:
                brief, rest = gen.split_brief(data)
                gen.wrap(out, "\\brief ", brief)
                if rest.strip():
                    out.append(" *")
                    gen.wrap(out, "", rest)
            else:
                out.append(" *")
                gen.wrap(out, "", data)
        elif kind == "list":
            out.append(" *")
            for item in data:
                gen.wrap(out, "\\li ", item)
        elif kind == "math":
            out.append(" *")
            out.append(" * \\f[")
            out.append(" * \\begin{pmatrix}")
            for row in data:
                out.append(" * " + row)
            out.append(" * \\end{pmatrix}")
            out.append(" * \\f]")
        elif kind == "code":
            info, code = data
            out.append(" *")
            out.append(" * \\code")
            for line in code:
                out.append(" * " + line.rstrip())
            out.append(" * \\endcode")
        first = False
    out.append(" */")
    return [(l.replace("*/", "*\\/") if i < len(out) - 1 else l)
            for i, l in enumerate(out)]


def apply(name, text, do_write):
    for target in TARGETS.get(name, [name]):
        apply_one(target, text, do_write)


def apply_one(name, text, do_write):
    header = members.header_for(name)
    if header is None:
        print(f"{name}: struct not found")
        return 0
    path = os.path.join(members.REPO, header)
    lines = open(path, encoding="utf-8").read().splitlines()
    start, _ = members.find_struct(lines, name)
    if start is None:
        print(f"{name}: struct not found in {header}")
        return 0
    block = render(text)
    # Replace the comment immediately above unless it is a plain note to keep; a
    # doc block already there (a rerun) is always replaced.
    prev = lines[start - 1].strip() if start > 0 else ""
    if prev.endswith("*/"):
        k = start - 1
        while k >= 0 and "/*" not in lines[k]:
            k -= 1
        is_doc = k >= 0 and lines[k].strip().startswith("/**")
        if is_doc or name not in KEEP:
            lines[k:start] = block
            action = "replaced"
        else:
            lines[start:start] = block
            action = "inserted"
    else:
        lines[start:start] = block
        action = "inserted"
    print(f"{name} ({header}): {action}")
    if do_write:
        open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    return 1


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    do_write = "--apply" in sys.argv
    if not args:
        print(__doc__)
        return
    for name, text in parse(args).items():
        if not text:
            print(f"{name}: EMPTY")
            continue
        apply(name, text, do_write)


if __name__ == "__main__":
    main()
