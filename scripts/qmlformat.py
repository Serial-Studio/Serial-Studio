#!/usr/bin/env python3
"""QML property-ordering linter/formatter for Serial Studio.

Enforces a subset of the QML style rules documented in CLAUDE.md:
- `id: <name>` is the first non-comment line inside each object block,
  followed by exactly one blank line before the next content.
- Contiguous runs of single-assignment properties are sorted by total
  rendered length of the full logical line (shortest first) — the
  "christmas tree" rule — without crossing blank lines, signal handlers,
  functions, or nested object blocks.

The tokenizer groups physical lines into "logical properties": a simple
property line may own following continuation lines (ternary fragments,
trailing-operator continuations, etc.).  When sorting a run, logical
properties move as atomic units, so ternary continuations never get
orphaned from their owner.

Any block containing ambiguous constructs (nested object literals,
inline `Rectangle { ... }` children as property values, ScrollBar /
background inline blocks) is skipped — the whole run containing the
ambiguity is left untouched.

Usage:
    python3 scripts/qmlformat.py --check app/qml               # report-only
    python3 scripts/qmlformat.py --fix app/qml                 # rewrite files
    python3 scripts/qmlformat.py --check app/qml/Foo.qml       # single file
    python3 scripts/qmlformat.py --fix --diff app/qml          # show changes

Exit codes:
    0 - clean (check) or rewrote files (fix)
    1 - violations found (check)
    2 - argument error
"""

from __future__ import annotations

import argparse
import difflib
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable


# ---------------------------------------------------------------------------
# Line classification
# ---------------------------------------------------------------------------

_SIMPLE_PROP = re.compile(
    r"""
    ^                          # start of line
    \s*
    (?:readonly\s+)?
    (?:property\s+\w+\s+)?
    [A-Za-z_][\w.]*            # identifier (possibly dotted)
    \s* : \s*
    .+                         # at least some value on this line
    $
    """,
    re.VERBOSE,
)

_HANDLER = re.compile(r"^\s*on[A-Z]\w*\s*:\s*")
_ID_LINE = re.compile(r"^\s*id\s*:\s*(\w+)\s*$")

# A continuation of the previous physical line — starts with an operator
# that cannot begin a new statement.
_CONTINUATION_PREFIX = re.compile(
    r"""
    ^ \s*
    (?:
        [?:]            # ternary
      | [+\-*/%&|^]     # arithmetic / bitwise
      | && | \|\|       # logical
      | ,               # list continuation
      | \. [A-Za-z_]    # dotted-member continuation
      | \) | ]          # closing bracket
    )
    """,
    re.VERBOSE,
)


@dataclass
class LogicalLine:
    """One or more physical lines that together form one logical unit."""

    raws: list[str] = field(default_factory=list)  # raw source lines (no newlines)
    kind: str = "other"  # "id" | "prop" | "blank" | "comment" | "open" | "close" | "handler" | "other"
    start_idx: int = 0   # index of first physical line in the source

    @property
    def length(self) -> int:
        """Total rendered length = first physical line's length (for sorting)."""
        return len(self.raws[0]) if self.raws else 0

    @property
    def text(self) -> str:
        """Concatenated stripped first-line text, mainly for classification."""
        return self.raws[0].strip() if self.raws else ""


def physical_kind(raw: str) -> str:
    """First-pass classification of a single physical line."""
    stripped = raw.rstrip()
    text = stripped.strip()

    if not text:
        return "blank"
    if text.startswith("//") or text.startswith("/*") or text.startswith("*"):
        return "comment"
    if _ID_LINE.match(stripped):
        return "id"
    # A line ending in `{` opens a nested block
    if text.endswith("{"):
        return "open"
    # `}` or `} ...` closes a block
    if text == "}" or text.rstrip().endswith("}") and not _SIMPLE_PROP.match(stripped):
        return "close"
    if _HANDLER.match(stripped):
        return "handler"
    if _SIMPLE_PROP.match(stripped):
        return "prop"
    return "other"


def is_continuation(prev_kind: str, raw: str) -> bool:
    """True if `raw` is a continuation of the previous logical line."""
    if prev_kind not in ("prop", "other"):
        return False
    stripped = raw.rstrip()
    if not stripped.strip():
        return False
    return bool(_CONTINUATION_PREFIX.match(stripped))


def tokenize(raw_lines: list[str]) -> list[LogicalLine]:
    """Group physical lines into logical lines.

    A simple-property or `other` line absorbs subsequent continuation lines
    (ones starting with `?`, `:`, `+`, `&&`, etc.) until a non-continuation
    physical line is seen.
    """
    logical: list[LogicalLine] = []
    i = 0
    n = len(raw_lines)

    while i < n:
        raw = raw_lines[i]
        kind = physical_kind(raw)
        line = LogicalLine(raws=[raw], kind=kind, start_idx=i)
        i += 1

        # Absorb continuations onto prop/other lines
        if kind in ("prop", "other"):
            while i < n and is_continuation(kind, raw_lines[i]):
                line.raws.append(raw_lines[i])
                i += 1

        logical.append(line)

    return logical


# ---------------------------------------------------------------------------
# Block walking
# ---------------------------------------------------------------------------

def _brace_delta(line: LogicalLine) -> int:
    """Net change in brace depth this logical line contributes."""
    if line.kind in ("comment", "blank"):
        return 0
    opens = 0
    closes = 0
    for raw in line.raws:
        # Fast-and-loose — string literals inside prop values could fool us,
        # but property values ending in `{` are already classified as "open"
        # which is fine for our needs.
        opens += raw.count("{")
        closes += raw.count("}")
    return opens - closes


# ---------------------------------------------------------------------------
# Safety check — does a run contain anything unsafe to reorder?
# ---------------------------------------------------------------------------

def is_run_safe(run: list[LogicalLine]) -> bool:
    """Sorting is only safe when every logical line is a single physical
    'prop' line with no inline braces.  Multi-line props (ternaries,
    continuations) and property values opening nested blocks stay put."""
    for line in run:
        if line.kind != "prop":
            return False
        if len(line.raws) != 1:
            return False
        text = line.raws[0]
        # Reject inline nested blocks on prop values (e.g. `background: Rectangle { ... }` spread across lines is already multi-line; this guards against any single-line case too)
        if "{" in text or "}" in text:
            return False
    return True


# ---------------------------------------------------------------------------
# Property-run extraction
# ---------------------------------------------------------------------------

def find_property_runs(lines: list[LogicalLine]) -> list[tuple[int, int]]:
    """Return (start, end_exclusive) ranges of consecutive 'prop' logical
    lines (length >= 2).  Blank lines, comments, handlers, or anything
    non-prop terminates a run."""
    runs: list[tuple[int, int]] = []
    i = 0
    n = len(lines)
    while i < n:
        if lines[i].kind == "prop":
            j = i
            while j < n and lines[j].kind == "prop":
                j += 1
            if j - i >= 2:
                runs.append((i, j))
            i = j
        else:
            i += 1
    return runs


def is_sorted_ascending(run: list[LogicalLine]) -> bool:
    for a, b in zip(run, run[1:]):
        if a.length > b.length:
            return False
    return True


def sort_run(run: list[LogicalLine]) -> list[LogicalLine]:
    return sorted(run, key=lambda line: line.length)


# ---------------------------------------------------------------------------
# `id:` placement check
# ---------------------------------------------------------------------------

@dataclass
class Violation:
    path: Path
    line: int
    kind: str
    message: str


def check_id_placement(lines: list[LogicalLine], path: Path) -> list[Violation]:
    """Inside each object body, `id:` must be the first non-comment/blank
    content line and must be followed by one blank line."""
    violations: list[Violation] = []
    depth = 0
    # Track body-start indices: the index of the logical line immediately
    # after each 'open'.  We only check the shallow (top-of-body) level.
    body_stack: list[int] = []

    for i, line in enumerate(lines):
        if line.kind == "open":
            body_stack.append(i + 1)
            depth += _brace_delta(line)
            continue

        delta = _brace_delta(line)
        if delta < 0 and body_stack:
            # Closing a body we started; process its shallow id placement
            start = body_stack.pop()
            _check_shallow_id(lines, start, i, path, violations)
        depth += delta

    return violations


def _check_shallow_id(
    lines: list[LogicalLine],
    start: int,
    end: int,
    path: Path,
    violations: list[Violation],
) -> None:
    # Find the first non-blank, non-comment logical line at depth 0 of this body
    inner_depth = 0
    first_content_idx = None
    shallow_id_idx = None

    for i in range(start, end):
        line = lines[i]
        # Update inner depth before recording this line so nested-block
        # content is excluded from the shallow content walk
        if inner_depth == 0:
            if line.kind in ("blank", "comment"):
                continue
            if first_content_idx is None:
                first_content_idx = i
            if line.kind == "id":
                shallow_id_idx = i
                break
            # A non-id content line before the id (or no id) — stop looking at shallow depth
            break
        # Move through nested block content
        if line.kind == "close" and inner_depth > 0:
            inner_depth += _brace_delta(line)
            continue
        inner_depth += _brace_delta(line)

    if shallow_id_idx is None:
        return

    # If the first content line wasn't the id, flag it
    if first_content_idx != shallow_id_idx:
        violations.append(
            Violation(
                path,
                lines[shallow_id_idx].start_idx + 1,
                "id-placement",
                "`id:` must be the first content line inside the block",
            )
        )
        return

    # Ensure blank line after the id
    next_idx = shallow_id_idx + 1
    if next_idx < end and lines[next_idx].kind != "blank":
        violations.append(
            Violation(
                path,
                lines[shallow_id_idx].start_idx + 1,
                "id-blank-line",
                "expected a blank line after `id:` before the next content",
            )
        )


# ---------------------------------------------------------------------------
# File-level processing
# ---------------------------------------------------------------------------

def process_file(path: Path, fix: bool) -> tuple[list[Violation], str | None]:
    raw_text = path.read_text(encoding="utf-8")
    raw_lines = raw_text.splitlines()
    lines = tokenize(raw_lines)

    violations: list[Violation] = []
    violations.extend(check_id_placement(lines, path))

    runs = find_property_runs(lines)
    sortable_runs: list[tuple[int, int]] = []
    for start, end in runs:
        run_lines = lines[start:end]
        if not is_run_safe(run_lines):
            # Skip silently — we can't safely sort this run
            continue
        if is_sorted_ascending(run_lines):
            continue
        sortable_runs.append((start, end))
        violations.append(
            Violation(
                path,
                run_lines[0].start_idx + 1,
                "christmas-tree",
                f"{end - start} properties not sorted shortest→longest",
            )
        )

    if not fix or not sortable_runs:
        return violations, None

    # Apply sorts in reverse so indices stay valid
    new_lines = list(lines)
    for start, end in reversed(sortable_runs):
        sorted_block = sort_run(new_lines[start:end])
        new_lines[start:end] = sorted_block

    # Flatten back to raw text
    out_raws: list[str] = []
    for line in new_lines:
        out_raws.extend(line.raws)

    new_text = "\n".join(out_raws)
    if raw_text.endswith("\n") and not new_text.endswith("\n"):
        new_text += "\n"

    if new_text == raw_text:
        return violations, None

    return violations, new_text


def iter_qml_files(targets: list[Path]) -> Iterable[Path]:
    """Yield .qml files under each target path (recursive for directories)."""
    for target in targets:
        if target.is_file():
            if target.suffix == ".qml":
                yield target
            continue
        for root, _, files in os.walk(target):
            parts = Path(root).parts
            if "build" in parts or "_deps" in parts:
                continue
            for name in files:
                if name.endswith(".qml"):
                    yield Path(root) / name


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("--check", action="store_true", help="report only, no writes")
    group.add_argument("--fix", action="store_true", help="rewrite files in place")
    parser.add_argument("--diff", action="store_true",
                        help="show unified diff of proposed changes")
    parser.add_argument("paths", nargs="+", type=Path,
                        help="files or directories to process")

    args = parser.parse_args(argv)

    files = sorted(set(iter_qml_files(args.paths)))
    if not files:
        print("no QML files found", file=sys.stderr)
        return 2

    total_violations = 0
    total_changed = 0

    for path in files:
        violations, new_text = process_file(path, fix=args.fix)
        total_violations += len(violations)

        for v in violations:
            print(f"{v.path}:{v.line}: {v.kind}: {v.message}")

        if new_text is not None:
            if args.diff:
                before = path.read_text(encoding="utf-8").splitlines(keepends=True)
                after = new_text.splitlines(keepends=True)
                sys.stdout.writelines(
                    difflib.unified_diff(before, after, str(path), str(path))
                )
            if args.fix:
                path.write_text(new_text, encoding="utf-8")
                total_changed += 1
                print(f"{path}: rewrote", file=sys.stderr)

    if args.check:
        print(f"\n{len(files)} files scanned, {total_violations} violations", file=sys.stderr)
        return 1 if total_violations else 0

    print(f"\n{len(files)} files scanned, {total_changed} rewritten", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
