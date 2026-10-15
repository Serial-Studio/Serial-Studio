#!/usr/bin/env python3
"""Serial Studio terminology gate (spec 0100 R17/R18).

One concept, one user-facing name. This scans every surface a user reads for
the banned synonyms listed in scripts/term-rules.json and names the canonical
term for each hit:

    qml   -- qsTr("...") literals under app/qml
    cpp   -- tr("..."), QObject::tr("...") and translate(ctx, "...") literals
             under core/ and app/src (generated forms included, tests excluded)
    json  -- "title" / "tooltip" / "label" values in app/rcc/commands/**
    docs  -- doc/help/*.md, README.md, examples/**/README.md
    ai    -- app/rcc/ai/**/*.md (the in-app assistant corpus)

Internal identifiers, JSON keys and API names are not user text: Markdown
code spans, fenced blocks and link URLs are masked before matching, and
term-rules.json lists the pages and line patterns that legitimately name an
old term (rename banners, the API Reference, the on-disk "Session Databases"
folder).

Usage:
    python3 scripts/term-verify.py                 # scan every surface
    python3 scripts/term-verify.py --surface qml   # one surface (repeatable)
    python3 scripts/term-verify.py --summary       # counts per rule only

Exit codes:
    0 - clean
    1 - banned synonyms found
    2 - argument or rules-file error
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Iterator

ROOT = Path(__file__).resolve().parent.parent
RULES_FILE = ROOT / "scripts" / "term-rules.json"
SURFACES = ("qml", "cpp", "json", "docs", "ai")

_QSTR_RE = re.compile(r'qsTr\(\s*"((?:[^"\\]|\\.)*)"')
_CPP_TR_RE = re.compile(
    r'\b(?:tr[A-Z][A-Za-z]*|tr|QT_TR_NOOP|(?:QT_TRANSLATE_NOOP|translate)\(\s*"[^"]*"\s*,)'
    r'\s*\(?\s*"((?:[^"\\]|\\.)*)"'
)
_CONT_RE = re.compile(r'^\s*\+?\s*"((?:[^"\\]|\\.)*)"')
_JSON_KEYS = ("title", "tooltip", "label", "description")
_FENCE_RE = re.compile(r"^\s*(```+|~~~+)")
_MASK_RES = (
    re.compile(r"`[^`]*`"),
    re.compile(r"\]\([^)]*\)"),
    re.compile(r"<[^>]+>"),
    re.compile(r"https?://\S+"),
)


@dataclass
class Rule:
    regex: re.Pattern
    use: str
    label: str


@dataclass
class Hit:
    path: Path
    line: int
    rule: Rule
    excerpt: str


def load_rules() -> tuple[list[Rule], set[str], list[re.Pattern]]:
    """Reads term-rules.json into compiled rules, allowed paths and allowed line patterns."""
    data = json.loads(RULES_FILE.read_text(encoding="utf-8"))
    rules = []
    for entry in data["rules"]:
        flags = re.IGNORECASE if "i" in entry.get("flags", "") else 0
        rules.append(
            Rule(re.compile(entry["pattern"], flags), entry["use"], entry["pattern"])
        )

    allow_paths = set(data.get("allow_paths", []))
    allow_lines = [re.compile(p) for p in data.get("allow_line_patterns", [])]
    return rules, allow_paths, allow_lines


def mask_markdown(line: str) -> str:
    """Blanks code spans, link targets, HTML tags and URLs, keeping the line length."""
    for pattern in _MASK_RES:
        line = pattern.sub(lambda m: " " * (m.end() - m.start()), line)
    return line


def markdown_lines(path: Path) -> Iterator[tuple[int, str]]:
    """Yields (line number, masked text) for prose lines outside fenced code blocks."""
    fenced = False
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        if _FENCE_RE.match(line):
            fenced = not fenced
            continue

        if not fenced:
            yield number, mask_markdown(line)


def literal_lines(path: Path, opener: re.Pattern) -> Iterator[tuple[int, str]]:
    """Yields (line number, literal) for translatable string literals, including the
    continuation literals of a string split across lines."""
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    open_call = False
    for number, line in enumerate(lines, 1):
        matches = list(opener.finditer(line))
        for match in matches:
            yield number, match.group(1)

        if matches:
            open_call = not line.rstrip().endswith((")", ");", "),", ")"))
            continue

        cont = _CONT_RE.match(line) if open_call else None
        if cont:
            yield number, cont.group(1)
        else:
            open_call = False


def json_lines(path: Path) -> Iterator[tuple[int, str]]:
    """Yields (line number, value) for the user-visible keys of a command manifest."""
    key_re = re.compile(r'"(%s)"\s*:\s*"((?:[^"\\]|\\.)*)"' % "|".join(_JSON_KEYS))
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), 1):
        for match in key_re.finditer(line):
            yield number, match.group(2)


def surface_files(surface: str) -> Iterable[Path]:
    """Lists the files one surface scans."""
    if surface == "qml":
        return sorted((ROOT / "app" / "qml").rglob("*.qml"))

    if surface == "cpp":
        files = []
        for base in (ROOT / "core", ROOT / "app" / "src"):
            files += [p for p in base.rglob("*.cpp") if "tests" not in p.parts]
        return sorted(files)

    if surface == "json":
        return sorted((ROOT / "app" / "rcc" / "commands").rglob("*.json"))

    if surface == "docs":
        files = sorted((ROOT / "doc" / "help").glob("*.md"))
        files += [ROOT / "README.md"]
        files += sorted((ROOT / "examples").rglob("README.md"))
        return files

    return sorted((ROOT / "app" / "rcc" / "ai").rglob("*.md"))


def scan_surface(surface, rules, allow_paths, allow_lines) -> list[Hit]:
    """Scans one surface and returns every banned-synonym hit."""
    hits = []
    for path in surface_files(surface):
        rel = path.relative_to(ROOT).as_posix()
        if rel in allow_paths or not path.exists():
            continue

        if surface == "qml":
            source = literal_lines(path, _QSTR_RE)
        elif surface == "cpp":
            source = literal_lines(path, _CPP_TR_RE)
        elif surface == "json":
            source = json_lines(path)
        else:
            source = markdown_lines(path)

        for number, text in source:
            if any(p.search(text) for p in allow_lines):
                continue

            for rule in rules:
                if rule.regex.search(text):
                    hits.append(Hit(path, number, rule, text.strip()[:110]))

    return hits


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description="Serial Studio terminology gate (spec 0100)."
    )
    parser.add_argument("--surface", action="append", choices=SURFACES)
    parser.add_argument(
        "--summary", action="store_true", help="print counts per rule only"
    )
    args = parser.parse_args(argv)

    try:
        rules, allow_paths, allow_lines = load_rules()
    except (OSError, ValueError, KeyError, re.error) as error:
        print(f"term-verify: cannot read {RULES_FILE.name}: {error}", file=sys.stderr)
        return 2

    hits = []
    for surface in args.surface or SURFACES:
        hits += scan_surface(surface, rules, allow_paths, allow_lines)

    if not args.summary:
        for hit in hits:
            rel = hit.path.relative_to(ROOT).as_posix()
            sys.stdout.write(f"{rel}:{hit.line}: use '{hit.rule.use}': {hit.excerpt}\n")

    counts: dict[str, int] = {}
    for hit in hits:
        counts[hit.rule.use] = counts.get(hit.rule.use, 0) + 1

    for use, count in sorted(counts.items(), key=lambda kv: -kv[1]):
        sys.stdout.write(f"  {count:5d}  -> {use}\n")

    sys.stdout.write(f"term-verify: {len(hits)} banned synonym(s)\n")
    return 1 if hits else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
