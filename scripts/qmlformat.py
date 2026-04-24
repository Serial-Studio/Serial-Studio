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
    python3 scripts/qmlformat.py                               # fix everything under app/qml
    python3 scripts/qmlformat.py --check                       # report-only, whole tree
    python3 scripts/qmlformat.py --check app/qml               # report-only, explicit path
    python3 scripts/qmlformat.py --fix app/qml                 # rewrite files
    python3 scripts/qmlformat.py --check app/qml/Foo.qml       # single file
    python3 scripts/qmlformat.py --fix --diff app/qml          # show changes

Called with no arguments the script defaults to --fix on <repo>/app/qml.

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

# Trailing tokens that imply the next physical line continues this one.
# The base set omits `:` because a bare `:` at end-of-line on the first
# physical line of a prop is the property separator after a blank value
# (rare but possible).  Once we've already absorbed at least one
# continuation line, a trailing `:` is almost certainly a ternary middle,
# so _TRAILING_OPERATOR_INNER adds `:` back in.
_TRAILING_OPERATOR_FIRST = re.compile(
    r"""
    (?:
        [?+\-*/%&|^]      # single-char operators (no `:`)
      | && | \|\|
    )
    \s* $
    """,
    re.VERBOSE,
)

_TRAILING_OPERATOR_INNER = re.compile(
    r"""
    (?:
        [?:+\-*/%&|^]     # includes `:` for ternary continuations
      | && | \|\|
    )
    \s* $
    """,
    re.VERBOSE,
)


def _strip_strings_and_comments(line: str) -> str:
    """Return `line` with string literals and `//` comments blanked out, so
    bracket-counting on the result ignores brackets that live inside strings
    or end-of-line comments."""
    result: list[str] = []
    i = 0
    n = len(line)
    while i < n:
        ch = line[i]

        # Line comment — drop everything to end-of-line
        if ch == "/" and i + 1 < n and line[i + 1] == "/":
            break

        # Double-quoted string
        if ch == '"':
            result.append(" ")
            i += 1
            while i < n:
                if line[i] == "\\" and i + 1 < n:
                    i += 2
                    result.append("  ")
                    continue
                if line[i] == '"':
                    result.append(" ")
                    i += 1
                    break
                result.append(" ")
                i += 1
            continue

        # Single-quoted string (JS string literal)
        if ch == "'":
            result.append(" ")
            i += 1
            while i < n:
                if line[i] == "\\" and i + 1 < n:
                    i += 2
                    result.append("  ")
                    continue
                if line[i] == "'":
                    result.append(" ")
                    i += 1
                    break
                result.append(" ")
                i += 1
            continue

        result.append(ch)
        i += 1

    return "".join(result)


def _bracket_delta(line: str) -> int:
    """Signed open/close bracket count, ignoring strings and line comments."""
    sanitized = _strip_strings_and_comments(line)
    opens = sanitized.count("(") + sanitized.count("[") + sanitized.count("{")
    closes = sanitized.count(")") + sanitized.count("]") + sanitized.count("}")
    return opens - closes


def _brace_delta_raw(line: str) -> int:
    """Signed curly-brace count on a raw string, ignoring strings/comments."""
    sanitized = _strip_strings_and_comments(line)
    return sanitized.count("{") - sanitized.count("}")


def _has_trailing_operator(line: str, is_inner: bool) -> bool:
    """True when the sanitized line ends with a continuation operator.

    `is_inner` distinguishes the first physical line of a logical property
    (where a trailing `:` is the prop separator) from absorbed continuation
    lines (where `:` usually means ternary middle).

    An extra guard handles ternary-middle on the FIRST line too:
    `icon.source: checked ? "a" :` has a trailing `:` that is not the prop
    separator (the prop-separator `:` comes after `icon.source` and is
    followed by a non-empty value).  We detect this by counting unbalanced
    `?`: if the sanitized text has more `?` than `:` after the prop
    separator, a trailing `:` must be completing a ternary and we absorb.
    """
    sanitized = _strip_strings_and_comments(line).rstrip()
    if not sanitized:
        return False

    pattern = _TRAILING_OPERATOR_INNER if is_inner else _TRAILING_OPERATOR_FIRST
    if pattern.search(sanitized):
        return True

    # First-line ternary-middle: `prop: ... ? ... :`
    if not is_inner and sanitized.endswith(":"):
        # Strip the property-separator `:` and whatever precedes it, then
        # check the remaining value text for unbalanced `?`.  The trailing
        # `:` is the operator we're evaluating, so drop it before counting.
        m = re.match(r"\s*(?:readonly\s+)?(?:property\s+\w+\s+)?[A-Za-z_][\w.]*\s*:(.*)$",
                     sanitized)
        if m:
            value = m.group(1).rstrip()
            if value.endswith(":"):
                value = value[:-1]
            q = value.count("?")
            c = value.count(":")
            if q > c:
                return True

    return False


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

    # Classify by net brace delta on the sanitized text so lines with
    # balanced inline braces (`const x = {}`) aren't mistaken for closers.
    delta = _brace_delta_raw(stripped)
    if delta > 0:
        return "open"
    if delta < 0:
        return "close"

    # Net-zero lines — treat `}` or `} else {` style only if the only brace
    # action on the line is a single close at the start.
    if text == "}" and not _SIMPLE_PROP.match(stripped):
        return "close"

    if _HANDLER.match(stripped):
        return "handler"
    if _SIMPLE_PROP.match(stripped):
        return "prop"
    return "other"


def opens_js_body(raw: str) -> bool:
    """True when `raw` ends with `{` and that `{` opens a JavaScript body.

    Heuristic: the line matches a QML handler pattern (`onSomething: {`),
    a Component.onXxx attachment, or a JavaScript `function ... {` or
    arrow-function `... => {` body.  Anything of the form `Foo {` or
    `Foo.Bar {` at the start of a line is a QML object declaration and is
    NOT a JS body.
    """
    stripped = raw.rstrip()
    if not stripped.endswith("{"):
        return False

    text = stripped.strip()
    sanitized = _strip_strings_and_comments(text)

    # JavaScript: `function name(args) {` or `function (args) {`
    if re.match(r"^\s*function\b", sanitized):
        return True

    # Arrow function body: `... => {`
    if "=>" in sanitized:
        return True

    # Handler or attachment with a body: `onClicked: {`, `Component.onCompleted: {`,
    # `Keys.onPressed: {`, etc.  Anything that has `: {` at the end.
    if re.search(r":\s*\{\s*$", sanitized):
        return True

    return False


def is_continuation(prev_kind: str, raw: str) -> bool:
    """True if `raw` begins with a token that can only be a continuation."""
    if prev_kind not in ("prop", "other"):
        return False
    stripped = raw.rstrip()
    if not stripped.strip():
        return False
    return bool(_CONTINUATION_PREFIX.match(stripped))


def tokenize(raw_lines: list[str]) -> list[LogicalLine]:
    """Group physical lines into logical lines.

    A prop/other line absorbs subsequent physical lines while EITHER:
      - bracket depth is still > 0 (unclosed `(`, `[`, or `{` in the value),
      - the last absorbed line ends with a trailing operator (`?`, `:`, `+`, …),
      - OR the next physical line starts with a continuation operator.

    Comments and handlers are never absorbed into a property run. A `{` or `}`
    that appears on the *absorbed* lines contributes to the bracket count, but
    does not retrigger the `open`/`close` classification — the logical line is
    still treated as a prop for run-sorting purposes, just an unsafe one.
    """
    logical: list[LogicalLine] = []
    i = 0
    n = len(raw_lines)

    while i < n:
        raw = raw_lines[i]
        kind = physical_kind(raw)
        line = LogicalLine(raws=[raw], kind=kind, start_idx=i)
        i += 1

        if kind not in ("prop", "other"):
            logical.append(line)
            continue

        # Track open-bracket balance across absorbed lines. If the property's
        # value opens a list literal, object literal, or parenthesized group,
        # keep absorbing physical lines until brackets rebalance.
        depth = _bracket_delta(raw)
        last = raw
        inner = False  # True once we've absorbed at least one continuation line

        while i < n:
            next_raw = raw_lines[i]
            next_kind = physical_kind(next_raw)

            # Blank line always terminates the run — even an open bracket
            # followed by a blank line is malformed source we won't touch.
            if next_kind == "blank":
                break

            # A line whose net bracket delta is negative is a structural close
            # of an OUTER block, not a continuation of this prop's value.
            # Never absorb it, even if it happens to start with `)`, `]`, or `}`.
            # Exception: when we're still inside an unclosed bracket run of our
            # own — then the `)` / `]` / `}` closes our own literal.
            next_bracket_delta = _bracket_delta(next_raw)
            if next_bracket_delta < 0 and depth + next_bracket_delta < 0:
                break

            # Decide whether `next_raw` continues `last`.
            absorb = False
            if depth > 0:
                absorb = True
            elif _has_trailing_operator(last, is_inner=inner):
                absorb = True
            elif is_continuation(kind, next_raw):
                absorb = True

            if not absorb:
                break

            line.raws.append(next_raw)
            depth += _bracket_delta(next_raw)
            last = next_raw
            inner = True
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
    'prop' line with no inline braces or brackets.  The tokenizer already
    absorbs multi-line props into one LogicalLine; any line with more than
    one physical source line is therefore a multi-line value and skipped."""
    for line in run:
        if line.kind != "prop":
            return False
        if len(line.raws) != 1:
            return False
        sanitized = _strip_strings_and_comments(line.raws[0])
        # Reject inline nested blocks or collection literals on the value.
        if any(ch in sanitized for ch in "{}[]"):
            return False
    return True


# ---------------------------------------------------------------------------
# Property-run extraction
# ---------------------------------------------------------------------------

def find_property_runs(lines: list[LogicalLine]) -> list[tuple[int, int]]:
    """Return (start, end_exclusive) ranges of consecutive 'prop' logical
    lines (length >= 2) that are safe to reorder.

    A run is collected only when the surrounding block is a QML object
    declaration (`Foo { ... }` or `Rectangle { ... }`).  JavaScript bodies
    (handler functions like `onClicked: { ... }`, `function foo() { ... }`,
    arrow-body `=> { ... }`) are skipped entirely: the `identifier: value`
    pattern inside a JS object literal has dangling-comma semantics, and
    statements inside a JS statement block are not order-independent.

    The walk tracks a stack of (is_js, depth) entries so nested `{ ... }`
    inside a JS body stay in JS mode until brackets rebalance.
    """
    runs: list[tuple[int, int]] = []
    n = len(lines)

    # Stack entries: True for JS body, False for QML body.  An open that
    # occurs while the top of the stack is already JS inherits JS mode.
    js_stack: list[bool] = []

    def in_js() -> bool:
        return any(js_stack)

    i = 0
    while i < n:
        line = lines[i]

        if line.kind == "open":
            # Classify the new block: an open line inside an already-JS block
            # inherits JS mode (nested `{ ... }` inside a function body).
            is_js = opens_js_body(line.raws[0]) or in_js()

            # A single physical open line typically contributes one `{`, but
            # pathological sources may have several.  Push one frame per `{`;
            # all nested frames share the same JS classification.
            net = _brace_delta_raw(line.raws[0])
            frames_to_push = max(1, net)
            for _ in range(frames_to_push):
                js_stack.append(is_js)
            i += 1
            continue

        if line.kind == "close":
            net = _brace_delta_raw(line.raws[0])
            pops = max(1, -net) if net < 0 else 1
            for _ in range(pops):
                if js_stack:
                    js_stack.pop()
            i += 1
            continue

        if in_js():
            i += 1
            continue

        if line.kind == "prop":
            j = i
            while j < n and lines[j].kind == "prop":
                j += 1
            if j - i >= 2:
                runs.append((i, j))
            i = j
            continue

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

def default_target() -> Path:
    """Return the repo-level `app/qml` directory anchored to the script's location."""
    # scripts/qmlformat.py  →  <repo>/app/qml
    return Path(__file__).resolve().parent.parent / "app" / "qml"


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        epilog="With no arguments, runs --fix on the repo's app/qml tree.",
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="report only, no writes")
    group.add_argument("--fix", action="store_true", help="rewrite files in place (default)")
    parser.add_argument("--diff", action="store_true",
                        help="show unified diff of proposed changes")
    parser.add_argument("paths", nargs="*", type=Path,
                        help="files or directories to process (default: app/qml)")

    args = parser.parse_args(argv)

    # Default to --fix when neither mode was explicitly requested
    if not args.check and not args.fix:
        args.fix = True

    # Default to the repo's app/qml tree when no paths were supplied
    if not args.paths:
        target = default_target()
        if not target.exists():
            print(f"default target {target} does not exist; pass paths explicitly",
                  file=sys.stderr)
            return 2
        args.paths = [target]

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
