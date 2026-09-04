#!/usr/bin/env python3
"""Deterministic TU splitter for god-class .cpp files.

Splits one translation unit into per-concern TUs plus optional shared headers by
moving whole blocks (member functions, file-scope statics, structs, typedefs,
namespaces, using-declarations, clang-format directives) verbatim, driven by a
key-based manifest. Refuses to cut unless the parsed blocks reconstruct the
original file exactly (non-blank lines, conditional directives excluded), and
verifies brace and #if balance on every emitted file.

Usage:
    python3 scripts/tu-cutter.py inventory <file.cpp>
    python3 scripts/tu-cutter.py cut <file.cpp> <manifest.json>

Paths (CLI args and manifest entries) may be absolute or repo-relative; both are
resolved against the repository root, so the script is CWD-independent.

Manifest format:
{
 "dest_dir": "core/Pipeline/DataModel/Project",
 "residual": "core/Pipeline/DataModel/ProjectModel.cpp",
 "static_namespace": "DataModel",
 "headers": [{"name": "SHARED", "path": ".../ProjectModelShared.h",
              "namespace": "DataModel", "promote_inline": true,
              "includes": ["<QString>"]}],
 "tu_order": ["ProjectModelPersistence.cpp", "..."],
 "assignments": {"DataModel::ProjectModel::saveJsonFile": "ProjectModelPersistence.cpp",
                 "static:nextDuplicateTitle": "SHARED",
                 "DataModel::ProjectModel::openJsonFile#1": "ProjectModelLoading.cpp"}
}
Blocks not in assignments stay in the residual. "key#N" targets the Nth overload
(file order); a bare key targets all overloads. Destination "SHARED"-style names
must match a header name; "RESIDUAL" is implicit for unassigned blocks.
"""

from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def resolve(path):
    """Repo-relative or absolute path -> absolute string path."""
    p = Path(path)
    return str(p if p.is_absolute() else ROOT / p)


COND_RE = re.compile(r"^#\s*(if|ifdef|ifndef|elif|else|endif)\b")


def read(path):
    with open(resolve(path), "r", encoding="utf-8") as fh:
        return fh.read().split("\n")


def strip_code(line, in_block):
    """Return (code_only, in_block_after): line minus strings/comments."""
    out = []
    i, n = 0, len(line)
    while i < n:
        if in_block:
            j = line.find("*/", i)
            if j < 0:
                return ("".join(out), True)
            in_block = False
            i = j + 2
            continue
        c = line[i]
        if line.startswith("//", i):
            break
        if line.startswith("/*", i):
            in_block = True
            i += 2
            continue
        if c in ('"', "'"):
            q = c
            i += 1
            while i < n:
                if line[i] == "\\":
                    i += 2
                    continue
                if line[i] == q:
                    break
                i += 1
            i += 1
            continue
        out.append(c)
        i += 1
    return ("".join(out), in_block)


INCLUDE_RE = re.compile(r"^#\s*include\b")


def find_preamble_end(lines):
    """Index one past the initial SPDX/include region, with cond-stack closed."""
    last = 0
    for i, ln in enumerate(lines[:400]):
        s = ln.strip()
        if INCLUDE_RE.match(s):
            last = i
        elif s and not s.startswith(("//", "/*", "*", "*/", "#")):
            break
    end = last + 1
    depth = 0
    for i in range(end):
        s = lines[i].strip()
        m = COND_RE.match(s)
        if not m:
            continue
        w = m.group(1)
        if w in ("if", "ifdef", "ifndef"):
            depth += 1
        elif w == "endif":
            depth -= 1
    while depth > 0 and end < len(lines):
        s = lines[end].strip()
        m = COND_RE.match(s)
        if m and m.group(1) == "endif":
            depth -= 1
        elif s and not m:
            break
        end += 1
    return end


def sig_key(flat_upto_paren):
    t = flat_upto_paren.strip()
    m = re.search(r"([A-Za-z_][\w:]*)::(~?[A-Za-z_]\w*|operator\S*)\s*\($", t)
    if m:
        return f"{m.group(1)}::{m.group(2)}"
    m = re.search(r"([A-Za-z_]\w*)\s*\($", t)
    if m:
        head = t.split(m.group(1))[0]
        tag = "static" if re.search(r"\bstatic\b", head) else "free"
        return f"{tag}:{m.group(1)}"
    return None


def close_brace(lines, open_line, in_block=False):
    """Line index of the brace closing the first '{' at/after open_line."""
    depth = 0
    started = False
    for k in range(open_line, len(lines)):
        code, in_block = strip_code(lines[k], in_block)
        for ch in code:
            if ch == "{":
                depth += 1
                started = True
            elif ch == "}":
                depth -= 1
                if started and depth == 0:
                    return k
    return len(lines) - 1


def scan_blocks(lines, start):
    blocks = []
    cond_stack = []
    i, n = start, len(lines)
    prefix = None
    while i < n:
        s = lines[i].strip()
        if COND_RE.match(s):
            word = COND_RE.match(s).group(1)
            if word in ("if", "ifdef", "ifndef"):
                k = i + 1
                while k < n and not lines[k].strip():
                    k += 1
                if k < n and re.match(r"^#\s*endif\b", lines[k].strip()):
                    blocks.append(
                        dict(
                            kind="cond-empty",
                            key=f"condempty:{i + 1}",
                            start=i,
                            end=k + 1,
                            cond=list(cond_stack),
                        )
                    )
                    prefix = None
                    i = k + 1
                    continue
                cond_stack.append(lines[i])
            elif word in ("else", "elif") and cond_stack:
                cond_stack[-1] += "\x00" + lines[i]
            elif word == "endif" and cond_stack:
                cond_stack.pop()
            prefix = None
            i += 1
            continue
        if not s:
            i += 1
            continue
        if s.startswith("/*"):
            if prefix is None:
                prefix = i
            while i < n and "*/" not in lines[i]:
                i += 1
            i += 1
            continue
        if s in ("// clang-format off", "// clang-format on"):
            blocks.append(
                dict(
                    kind="directive",
                    key=f"clangfmt:{s.split()[-1]}",
                    start=prefix if prefix is not None else i,
                    end=i + 1,
                    cond=list(cond_stack),
                )
            )
            prefix = None
            i += 1
            continue
        if s.startswith("//"):
            if prefix is None:
                prefix = i
            i += 1
            continue
        if s.startswith(("using ", "Q_LOGGING", "extern ", "typedef ")) and s.endswith(
            ";"
        ):
            name = s.rstrip(";").split()[-1].split("::")[-1]
            blocks.append(
                dict(
                    kind="using",
                    key=f"using:{name}",
                    start=prefix if prefix is not None else i,
                    end=i + 1,
                    cond=list(cond_stack),
                )
            )
            prefix = None
            i += 1
            continue
        m_struct = re.match(r"(?:struct|class|enum(?:\s+class)?)\s+([A-Za-z_]\w*)", s)
        if m_struct and (
            s.rstrip().endswith("{") or (i + 1 < n and lines[i + 1].strip() == "{")
        ):
            open_l = i if "{" in s else i + 1
            j = close_brace(lines, open_l)
            blocks.append(
                dict(
                    kind="struct",
                    key=f"struct:{m_struct.group(1)}",
                    start=prefix if prefix is not None else i,
                    end=j + 1,
                    cond=list(cond_stack),
                )
            )
            prefix = None
            i = j + 1
            continue
        if re.match(r"typedef\s+(enum|struct)\b", s):
            if "{" in s and s.rstrip().endswith(";"):
                j = i
            else:
                open_l = i if "{" in s else i + 1
                j = close_brace(lines, open_l)
            name_m = re.search(r"}\s*([A-Za-z_]\w*)\s*;", lines[j])
            key = f"typedef:{name_m.group(1) if name_m else i + 1}"
            blocks.append(
                dict(
                    kind="typedef",
                    key=key,
                    start=prefix if prefix is not None else i,
                    end=j + 1,
                    cond=list(cond_stack),
                )
            )
            prefix = None
            i = j + 1
            continue
        if s.startswith("namespace") and s.rstrip().endswith("{"):
            j = close_brace(lines, i)
            name = s.split("{")[0].replace("namespace", "").strip() or "(anon)"
            blocks.append(
                dict(
                    kind="namespace",
                    key=f"namespace:{name}",
                    start=prefix if prefix is not None else i,
                    end=j + 1,
                    cond=list(cond_stack),
                )
            )
            prefix = None
            i = j + 1
            continue
        sig, j, found, in_blk = [], i, False, False
        op = cl = 0
        while j < n and j < i + 60:
            code, in_blk = strip_code(lines[j], in_blk)
            sig.append(code)
            op += code.count("(")
            cl += code.count(")")
            flat = " ".join(x.strip() for x in sig)
            if "(" in flat and op == cl:
                if code.rstrip().endswith("{") or (
                    j + 1 < n and lines[j + 1].strip() == "{"
                ):
                    found = True
                    break
                if code.rstrip().endswith(";"):
                    break
                if "{" in code and "}" not in code:
                    found = True
                    break
            if "(" not in flat and (
                code.rstrip().endswith(";") or code.rstrip().endswith("}")
            ):
                break
            j += 1
        if not found:
            flat_all = " ".join(x.strip() for x in sig)
            if "(" in flat_all and flat_all.rstrip().endswith(";"):
                key = sig_key(flat_all.split("(")[0] + "(") or f"unknown:{i + 1}"
                blocks.append(
                    dict(
                        kind="decl",
                        key=f"decl>{key}",
                        start=prefix if prefix is not None else i,
                        end=j + 1,
                        cond=list(cond_stack),
                    )
                )
            prefix = None
            i = j + 1
            continue
        brace_line = j if "{" in strip_code(lines[j], False)[0] else j + 1
        endl = close_brace(lines, brace_line)
        flat = " ".join(x.strip() for x in sig)
        key = sig_key(flat.split("(")[0] + "(") or f"unknown:{i + 1}"
        blocks.append(
            dict(
                kind="func",
                key=key,
                start=prefix if prefix is not None else i,
                end=endl + 1,
                cond=list(cond_stack),
            )
        )
        prefix = None
        i = endl + 1
    return blocks


def annotate(blocks):
    seen = {}
    for b in blocks:
        b["ordinal"] = seen.get(b["key"], 0)
        seen[b["key"]] = b["ordinal"] + 1
        b["nlines"] = b["end"] - b["start"]
    return blocks


def coverage_gaps(lines, pre_end, blocks):
    covered = [False] * len(lines)
    for k in range(pre_end):
        covered[k] = True
    for b in blocks:
        for k in range(b["start"], b["end"]):
            covered[k] = True
    gaps = []
    for k, c in enumerate(covered):
        if not c and lines[k].strip() and not COND_RE.match(lines[k].strip()):
            gaps.append((k + 1, lines[k][:90]))
    return gaps


def inventory(path):
    lines = read(path)
    pre = find_preamble_end(lines)
    blocks = annotate(scan_blocks(lines, pre))
    gaps = coverage_gaps(lines, pre, blocks)
    return dict(
        file=path,
        total_lines=len(lines),
        preamble_end=pre,
        block_count=len(blocks),
        uncovered_nonblank=[dict(line=g[0], text=g[1]) for g in gaps],
        blocks=[
            dict(
                key=b["key"],
                ordinal=b["ordinal"],
                kind=b["kind"],
                start=b["start"] + 1,
                end=b["end"],
                nlines=b["nlines"],
                cond=bool(b["cond"]),
            )
            for b in blocks
        ],
    )


def spdx_banner(lines):
    out = []
    for ln in lines:
        out.append(ln)
        if ln.strip() == "*/" or (
            ln.strip().startswith("//") and "SPDX" in ln and len(out) < 4
        ):
            if ln.strip() == "*/":
                break
    if out and out[-1].strip() != "*/":
        out = [l for l in lines[:6] if "SPDX" in l or l.startswith("//")]
    return out


def block_text(lines, b, promote_inline=False):
    seg = list(lines[b["start"] : b["end"]])
    if promote_inline:
        is_template = False
        for i, ln in enumerate(seg):
            s = ln.strip()
            if not s or s.startswith(("//", "/*", "*")):
                continue
            if s.startswith("template"):
                is_template = True
                continue
            if ln.startswith("static "):
                seg[i] = ("" if is_template else "inline ") + ln[len("static ") :]
            elif not is_template:
                seg[i] = "inline " + ln
            break
    parts, closes = [], 0
    for cond in b["cond"]:
        parts.append(cond.split("\x00")[0])
        closes += 1
    parts.extend(seg)
    parts.extend(["#endif"] * closes)
    return parts


def cut(path, manifest_path):
    man = json.load(open(resolve(manifest_path)))
    lines = read(path)
    pre_end = find_preamble_end(lines)
    preamble = lines[:pre_end]
    blocks = annotate(scan_blocks(lines, pre_end))
    gaps = coverage_gaps(lines, pre_end, blocks)
    if gaps:
        print(
            json.dumps(
                {"error": "uncovered nonblank lines; refuse to cut", "gaps": gaps[:20]},
                indent=1,
            )
        )
        return 1

    def keep(ln):
        s = ln.strip()
        return bool(s) and not COND_RE.match(s)

    orig_nonblank = [ln for ln in lines if keep(ln)]
    recon = [ln for ln in preamble if keep(ln)]
    for b in blocks:
        recon.extend(ln for ln in lines[b["start"] : b["end"]] if keep(ln))
    if recon != orig_nonblank:
        for k, (a, c) in enumerate(zip(recon, orig_nonblank)):
            if a != c:
                print(
                    json.dumps(
                        {
                            "error": "selfcheck reconstruction mismatch",
                            "at": k,
                            "recon": a[:90],
                            "orig": c[:90],
                        }
                    )
                )
                return 1
        print(
            json.dumps(
                {
                    "error": "selfcheck length mismatch",
                    "recon": len(recon),
                    "orig": len(orig_nonblank),
                }
            )
        )
        return 1

    assign = man["assignments"]
    headers = man.get("headers", [])
    hdr_names = {h["name"] for h in headers}
    known = set(man["tu_order"]) | {"RESIDUAL"} | hdr_names
    bad = [v for v in assign.values() if v not in known]
    if bad:
        print(json.dumps({"error": "unknown destinations", "bad": sorted(set(bad))}))
        return 1

    def dest_for(b):
        return assign.get(
            f"{b['key']}#{b['ordinal']}", assign.get(b["key"], "RESIDUAL")
        )

    tus = {name: [] for name in man["tu_order"]}
    hdr_blocks = {h["name"]: [] for h in headers}
    resid = []
    for b in blocks:
        d = dest_for(b)
        if d == "RESIDUAL":
            resid.append(b)
        elif d in hdr_blocks:
            hdr_blocks[d].append(b)
        else:
            tus[d].append(b)

    written = []
    hdr_includes = []
    for h in headers:
        bl = hdr_blocks[h["name"]]
        if not bl:
            continue
        hdr = spdx_banner(lines)
        hdr += ["", "#pragma once", ""]
        hdr += [f"#include {inc}" for inc in h.get("includes", [])]
        ns = h.get("namespace")
        if ns:
            hdr += ["", f"namespace {ns}", "{", ""]
        else:
            hdr += [""]
        for b in bl:
            hdr.extend(
                block_text(lines, b, promote_inline=h.get("promote_inline", False))
            )
            hdr.append("")
        if ns:
            hdr += [f"}} // namespace {ns}"]
        os.makedirs(os.path.dirname(resolve(h["path"])), exist_ok=True)
        with open(resolve(h["path"]), "w", encoding="utf-8", newline="\n") as fh:
            fh.write("\n".join(hdr).rstrip() + "\n")
        written.append(h["path"])
        hdr_includes.append(h["path"])

    NONMEMBER = (
        "static:",
        "free:",
        "namespace:",
        "struct:",
        "typedef:",
        "using:",
        "decl>",
        "unknown:",
    )
    static_ns = man.get("static_namespace")

    def emit_blocks(out, bl):
        ns_open = False
        for b in bl:
            nonmember = b["key"].startswith(NONMEMBER)
            directive = b["key"].startswith("clangfmt:")
            if static_ns and nonmember and not ns_open:
                out.extend([f"namespace {static_ns}", "{", ""])
                ns_open = True
            if static_ns and ns_open and not nonmember and not directive:
                out.extend([f"}} // namespace {static_ns}", ""])
                ns_open = False
            out.extend(block_text(lines, b))
            out.append("")
        if ns_open:
            out.extend([f"}} // namespace {static_ns}", ""])

    os.makedirs(resolve(man["dest_dir"]), exist_ok=True)
    for name in man["tu_order"]:
        bl = tus[name]
        if not bl:
            continue
        out = list(preamble)
        for hp in hdr_includes:
            rel = os.path.relpath(hp, os.path.join(man["dest_dir"]))
            out.append(
                f'#include "{os.path.basename(hp)}"'
                if "/" not in rel
                else f'#include "{rel}"'
            )
        out.append("")
        emit_blocks(out, bl)
        p = os.path.join(resolve(man["dest_dir"]), name)
        with open(p, "w", encoding="utf-8", newline="\n") as fh:
            fh.write("\n".join(out).rstrip() + "\n")
        written.append(p)

    res = list(preamble)
    if man.get("residual_includes_headers", True):
        for hp in hdr_includes:
            rel = os.path.relpath(hp, os.path.dirname(man["residual"]))
            res.append(f'#include "{rel}"')
    res.append("")
    emit_blocks(res, resid)
    with open(resolve(man["residual"]), "w", encoding="utf-8", newline="\n") as fh:
        fh.write("\n".join(res).rstrip() + "\n")
    written.append(man["residual"])

    balance_fail = []
    for p in written:
        txt = read(p)
        depth, in_blk = 0, False
        for ln in txt:
            code, in_blk = strip_code(ln, in_blk)
            depth += code.count("{") - code.count("}")
        cdepth = 0
        for ln in txt:
            m = COND_RE.match(ln.strip())
            if not m:
                continue
            w = m.group(1)
            if w in ("if", "ifdef", "ifndef"):
                cdepth += 1
            elif w == "endif":
                cdepth -= 1
        if depth != 0 or cdepth != 0:
            balance_fail.append(dict(file=p, brace_depth=depth, cond_depth=cdepth))

    moved_lines = sum(b["end"] - b["start"] for bl in tus.values() for b in bl)
    report = dict(
        selfcheck="PASS",
        written=written,
        total_blocks=len(blocks),
        per_tu={n: len(bl) for n, bl in tus.items()},
        per_header={n: len(bl) for n, bl in hdr_blocks.items()},
        residual_blocks=len(resid),
        moved_lines=moved_lines,
        balance="PASS" if not balance_fail else balance_fail,
    )
    print(json.dumps(report, indent=1))
    return 0 if not balance_fail else 1


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    if sys.argv[1] == "inventory":
        print(json.dumps(inventory(sys.argv[2]), indent=1))
        return 0
    if sys.argv[1] == "cut":
        return cut(sys.argv[2], sys.argv[3])
    return 2


if __name__ == "__main__":
    sys.exit(main())
