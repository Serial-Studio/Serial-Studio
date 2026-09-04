#!/usr/bin/env python3
"""Expand single-line `/** text */` doxygen blocks to the canonical 3-line form.

C/C++ only. Walks app/src and rewrites every line that is, in its entirety,
`<indent>/** <text> */` into:

    <indent>/**
    <indent> * <text>
    <indent> */

Empty `/** */` and `/**  */` (only whitespace inside) are left alone -- they
are not real doc comments. Lines where the closer is part of a wider statement
(e.g. `int x; /** ... */`) are not matched because the regex anchors the
opener at the start of the line and the closer at end-of-line.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

ROOTS = [ROOT / "app" / "src", ROOT / "core"]
EXTS = {".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hh"}

PATTERN = re.compile(r"^([ \t]*)/\*\*[ \t]+(.+?)[ \t]+\*/[ \t]*$")


def expand(text: str) -> str:
    out_lines: list[str] = []
    changed = False
    for line in text.splitlines(keepends=True):
        eol = ""
        body = line
        if body.endswith("\r\n"):
            eol = "\r\n"
            body = body[:-2]
        elif body.endswith("\n"):
            eol = "\n"
            body = body[:-1]
        elif body.endswith("\r"):
            eol = "\r"
            body = body[:-1]

        m = PATTERN.match(body)
        if not m:
            out_lines.append(line)
            continue

        indent, inner = m.group(1), m.group(2)
        if not inner.strip() or inner.strip() == "*":
            out_lines.append(line)
            continue

        out_lines.append(f"{indent}/**{eol}")
        out_lines.append(f"{indent} * {inner}{eol}")
        out_lines.append(f"{indent} */{eol}")
        changed = True

    return "".join(out_lines) if changed else text


def iter_files() -> list[Path]:
    files: list[Path] = []
    for root in ROOTS:
        if not root.exists():
            continue
        for p in root.rglob("*"):
            if p.is_file() and p.suffix.lower() in EXTS:
                files.append(p)
    return files


def main() -> int:
    rewrote = 0
    for path in iter_files():
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        new_text = expand(text)
        if new_text != text:
            path.write_text(new_text, encoding="utf-8", newline="")
            rewrote += 1

    print(f"[expand-doxygen] rewrote {rewrote} file(s)", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
