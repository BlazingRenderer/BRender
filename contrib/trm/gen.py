#!/usr/bin/env python3
"""Generate Doxygen blocks from the TRM transcripts.

Reads /tmp/apply/<header>.md (work items: real declaration + verbatim fields) and
writes /tmp/gen/<header>.blocks with one generated block per function, or a FLAG
line where the entry needs a human.
"""
import re, glob, os, sys, collections

FIELDS = ["description", "arguments", "preconditions", "effects", "result",
          "remarks", "see_also", "example", "notes"]
ABBR = {"e.g", "i.e", "etc", "cf", "vs", "no", "fig", "approx", "al", "resp"}

CBFN_MAP = {
    "ActorEnum": "br_actor_enum_cbfn", "DiagFailure": "br_diag_failure_cbfn",
    "DiagWarning": "br_diag_warning_cbfn", "MapEnum": "br_map_enum_cbfn",
    "MapFind": "br_map_find_cbfn", "MaterialEnum": "br_material_enum_cbfn",
    "MaterialFind": "br_material_find_cbfn", "ModeTest": "br_mode_test_cbfn",
    "ModelCustom": "br_model_custom_cbfn", "ModelEnum": "br_model_enum_cbfn",
    "ModelFind": "br_model_find_cbfn", "ModelPick2D": "br_modelpick2d_cbfn",
    "Pick2D": "br_pick2d_cbfn", "Pick3D": "br_pick3d_cbfn",
    "Primitive": "br_primitive_cbfn", "RenderBounds": "br_renderbounds_cbfn",
    "ResClassEnum": "br_resclass_enum_cbfn", "ResClassFind": "br_resclass_find_cbfn",
    "ResEnum": "br_resenum_cbfn", "ResourceFree": "br_resourcefree_cbfn",
    "ResFree": "br_resourcefree_cbfn", "TableEnum": "br_table_enum_cbfn",
    "TableFind": "br_table_find_cbfn", "FileAdvance": "brfile_advance_cbfn",
    "FileAttributes": "brfile_attributes_cbfn", "FileClose": "brfile_close_cbfn",
    "FileEOF": "brfile_eof_cbfn", "FileEof": "brfile_eof_cbfn",
    "FileGetChr": "brfile_getchr_cbfn", "FileGetLine": "brfile_getline_cbfn",
    "FileOpenRead": "brfile_open_read_cbfn", "FileOpenWrite": "brfile_open_write_cbfn",
    "FilePutChr": "brfile_putchr_cbfn", "FilePutLine": "brfile_putline_cbfn",
    "FileRead": "brfile_read_cbfn", "FileWrite": "brfile_write_cbfn",
    "MemAllocate": "brmem_allocate_cbfn", "MemFree": "brmem_free_cbfn",
    "MemInquire": "brmem_inquire_cbfn", "MemReallocate": "brmem_reallocate_cbfn",
    "MemAlign": "brmem_align_cbfn",
}

def parse_dump(path):
    text = open(path, encoding="utf-8").read()
    out = []
    for b in re.split(r"\n===== ", text)[1:]:
        head, _, body = b.partition("\n")
        name = head.split("  (line")[0].strip()
        decl, sig, fields, cur = "", "", collections.OrderedDict(), None
        for line in body.splitlines():
            if line.startswith("DECLARATION: "):
                decl = line[len("DECLARATION: "):].strip()
                continue
            if line.startswith("SIGNATURE: "):
                sig = line[len("SIGNATURE: "):].strip()
                continue
            m = re.match(r"(" + "|".join(FIELDS) + r"|page|declaration):\s?(.*)$", line)
            if m:
                cur = m.group(1)
                fields.setdefault(cur, []).append(m.group(2))
                continue
            if cur:
                fields[cur].append(line)
        out.append((name, sig or decl, fields))
    return out

def code_params(decl):
    i = decl.find("(")
    if i < 0:
        return None
    depth, j = 0, -1
    for k in range(i, len(decl)):
        if decl[k] == "(":
            depth += 1
        elif decl[k] == ")":
            depth -= 1
            if depth == 0:
                j = k
                break
    if j < 0:
        return None
    inner = decl[i + 1:j].strip()
    if inner in ("", "void"):
        return []
    parts, depth, cur = [], 0, ""
    for ch in inner:
        if ch in "([":
            depth += 1
        elif ch in ")]":
            depth -= 1
        if ch == "," and depth == 0:
            parts.append(cur); cur = ""
        else:
            cur += ch
    parts.append(cur)
    names = []
    for p in parts:
        p = p.strip()
        if p == "...":
            names.append("..."); continue
        toks = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", p)
        names.append(toks[-1] if toks else "?")
    return names

def manual_args(arglines):
    items, cur = [], None
    for line in arglines:
        m = re.match(r"^-\s*(.*)$", line.strip())
        if m:
            if cur:
                items.append(cur)
            cur = [m.group(1)]
        elif cur is not None and line.strip():
            cur.append(line.strip())
    if cur:
        items.append(cur)
    out = []
    for it in items:
        head = it[0]; desc = ""
        for sep in (" — ", " - ", " – "):
            if sep in head:
                head, desc = head.split(sep, 1); break
        names = []
        if head.strip().startswith("..."):
            names = ["..."]
        else:
            for piece in head.split(","):
                toks = re.findall(r"[A-Za-z_][A-Za-z0-9_]*", piece)
                if toks:
                    names.append(toks[-1])
        tail = " ".join(x for x in it[1:] if x)
        if tail:
            desc = (desc + " " + tail).strip()
        out.append((names, desc.strip()))
    return out

def split_bullets(lines):
    """Split a field into its intro text and bullet items."""
    intro, bullets, cur = [], [], None
    for l in lines:
        m = re.match(r"^-\s+(.*)$", l.strip())
        if m:
            if cur is not None:
                bullets.append(cur)
            cur = m.group(1)
            continue
        if not l.strip():
            if cur is not None:
                bullets.append(cur)
                cur = None
            elif not bullets:
                intro.append("")
            continue
        if cur is not None:
            cur += " " + l.strip()
        else:
            intro.append(l.strip())
    if cur is not None:
        bullets.append(cur)
    return " ".join(x for x in intro if x).strip(), [b.strip() for b in bullets if b.strip()]

def oneline(lines):
    return re.sub(r"\s+", " ", " ".join(x.strip() for x in lines if x.strip())).strip()

def strip_pages(s):
    s = re.sub(r"(\bBr[A-Za-z0-9_\[\]|]+\([^)]*\))\s*\d{1,3}(?:/\d+)?\b", r"\1", s)
    s = re.sub(r"(\bBr[A-Za-z0-9_\[\]|]+\(\))\s*\d{1,3}(?:/\d+)?\b", r"\1", s)
    s = re.sub(r"(\bbr_[a-z0-9_]+)\s+\d{2,3}\b", r"\1", s)
    s = re.sub(r"(\bbr_[a-z0-9_]+\(\))\s*\d{1,3}\b", r"\1", s)
    s = re.sub(r",?\s*[Pp]age\s+\d+", "", s)
    s = re.sub(r"\bCBFn([A-Za-z0-9_]+)\s*\(\s*\)\s*\d*", lambda m: CBFN_MAP.get(m.group(1), ""), s)
    s = re.sub(r"\s*,\s*,", ", ", s)
    s = re.sub(r"^[\s,;]+|[\s,;]+$", "", s)
    return s

TYPE_RE = re.compile(r"^(?:br_[a-z0-9_]+(?:\s*\*)*|void|int|float|char|long|unsigned|signed)\s+(?=[A-Za-z0-9])")

def split_brief(text):
    text = text.strip()
    for m in re.finditer(r"\.\s+(?=[A-Z])", text):
        head = text[:m.start()]
        prev = head.split()[-1] if head.split() else ""
        if prev.lower() in ABBR:
            continue
        return head + ".", text[m.end():]
    return text, ""

def wrap(out, prefix, text, width=100):
    line = " * " + prefix
    indent = " * " + " " * len(prefix)
    first = True
    for w in text.split():
        if first:
            line = line + w
            first = False
            continue
        cand = line + " " + w
        if len(cand) <= width:
            line = cand
        else:
            out.append(line)
            line = indent + w
    out.append(line)

def gen(name, decl, fields):
    log = []
    brief_src = strip_pages(oneline(fields.get("description", [])))
    if not brief_src:
        return None, ["no description"]
    brief, body = split_brief(brief_src)
    out = ["/**"]
    wrap(out, "\\brief ", brief)
    if body:
        out.append(" *")
        wrap(out, "", body)
    cps = code_params(decl)
    if cps is None:
        return None, ["cannot parse declaration"]
    margs = manual_args(fields.get("arguments", []))
    mnames = [n for names, _ in margs for n in names]
    if mnames and len(mnames) != len(cps):
        log.append(f"params {len(mnames)} manual vs {len(cps)} code ({mnames} / {cps})")
    params, pos = [], 0
    for names, desc in margs:
        for _ in names:
            if pos < len(cps):
                params.append((cps[pos], strip_pages(desc)))
            else:
                log.append("dropped manual arg beyond declaration")
            pos += 1
    if params:
        out.append(" *")
        w = max(len(p[0]) for p in params)
        for cn, desc in params:
            wrap(out, "\\param " + cn.ljust(w) + " ", desc)
    def sec(tag, key):
        intro, bullets = split_bullets(fields.get(key, []))
        if not intro and not bullets:
            return
        out.append(" *")
        lead = "\\" + tag + " "
        if intro:
            wrap(out, lead, strip_pages(intro))
            for b in bullets:
                wrap(out, "\\li ", strip_pages(b))
        else:
            wrap(out, "\\li ", strip_pages(bullets[0]))
            for b in bullets[1:]:
                wrap(out, "\\li ", strip_pages(b))
    sec("pre", "preconditions")
    sec("post", "effects")
    rv = strip_pages(oneline(fields.get("result", [])))
    if rv:
        for sep in (" — ", " - ", " – "):
            if sep in rv:
                rv = rv.split(sep, 1)[1]; break
        rv = TYPE_RE.sub("", rv)
        out.append(" *")
        wrap(out, "\\return ", rv)
    sec("remark", "remarks")
    sa = strip_pages(oneline(fields.get("see_also", [])))
    sa = re.sub(r"[,;]\s*$", "", sa).strip()
    if sa:
        out.append(" *")
        wrap(out, "\\sa ", sa)
    ex = [x for x in fields.get("example", []) if x.strip()]
    if any("*/" in x for x in ex):
        ex = [re.sub(r"/\*.*?\*/", "", x).rstrip() for x in ex]
        ex = [x for x in ex if x.strip()]
        if any("*/" in x for x in ex):
            log.append("escaped */ in example (non-comment)")
    if ex:
        is_list = (any(x.strip().startswith("- ") for x in ex)
                   and not any((";" in x or "{" in x or "}" in x) for x in ex))
        out.append(" *")
        out.append(" * \\par Example")
        if is_list:
            intro, bullets = split_bullets(ex)
            if intro:
                wrap(out, "", intro)
            for b in bullets:
                wrap(out, "\\li ", b)
        else:
            out.append(" * \\code{.c}")
            for l in ex:
                out.append(" * " + l.rstrip())
            out.append(" * \\endcode")
        if len(ex) > 10:
            log.append("long example")
    nv = strip_pages(oneline(fields.get("notes", [])))
    if nv:
        out.append(" *")
        wrap(out, "\\note ", nv)
    out.append(" */")
    out = [(l.replace("*/", "*\\/") if i < len(out) - 1 else l) for i, l in enumerate(out)]
    return "\n".join(out), log

def main():
    os.makedirs("/tmp/gen", exist_ok=True)
    only = sys.argv[1] if len(sys.argv) > 1 else None
    for path in sorted(glob.glob("/tmp/apply/*.md")):
        base = os.path.basename(path)[:-3]
        if only and only not in base:
            continue
        items = parse_dump(path)
        flagged = 0
        with open("/tmp/gen/" + base + ".blocks", "w", encoding="utf-8") as out:
            for name, decl, fields in items:
                block, log = gen(name, decl, fields)
                out.write(f"\n===== {name} =====\n")
                if block is None:
                    out.write("FLAG: " + "; ".join(log) + "\n"); flagged += 1
                else:
                    out.write(block + "\n")
                    if log:
                        out.write("// FLAG: " + "; ".join(log) + "\n"); flagged += 1
        print(f"{base}: {len(items)} entries, {flagged} flagged")

if __name__ == "__main__":
    main()
