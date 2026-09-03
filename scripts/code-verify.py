#!/usr/bin/env python3
"""Serial Studio source-tree formatter (QML + C++).

Enforces the style rules from CLAUDE.md that clang-format itself
cannot express:

- `id: <name>` is the first non-comment line inside each QML object
  block, followed by exactly one blank line before the next content.
- Contiguous runs of single-assignment QML properties are sorted by
  total rendered length of the full logical line (shortest first) —
  the "christmas tree" rule — without crossing blank lines, signal
  handlers, functions, or nested object blocks.
- A brace-free single-statement body (`if (x)\\n  foo();`) on its own
  line is followed by exactly one blank line before the next content.
  Applies to `if`, `else if`, `else`, `for`, and `while`. The blank
  line is suppressed when the body is followed by `else`, `}`, or
  `while` (do-while), or when it is already the last line in a block.

The QML tokenizer groups physical lines into "logical properties": a
simple property line may own following continuation lines (ternary
fragments, trailing-operator continuations, etc.).  When sorting a run,
logical properties move as atomic units, so ternary continuations never
get orphaned from their owner.

Any block containing ambiguous constructs (nested object literals,
inline `Rectangle { ... }` children as property values, ScrollBar /
background inline blocks) is skipped — the whole run containing the
ambiguity is left untouched.

The brace-free body rule applies to QML, C++ headers (.h), and C++
sources (.cpp). The id-placement and christmas-tree rules apply only to
QML files. CRLF/CR line endings are normalized to LF on every processed
file, regardless of suffix.

Comment-style and AI-narration findings (multi-line `//` runs, tutorial
voice, hedging, rot-references, restate-the-code openers, emoji, …) are
flag-only — collapsing or rewriting comments is a judgement call. Banner
shapes (`//---` rules, QML `//` / `// label` / `//` sandwich) and tooling
pragmas (`// clang-format off/on`, `// NOLINT`, `// cppcheck-suppress`,
`// fallthrough`) are skipped. Findings are grouped by rule into
`.code-report` at the repo root for a follow-up human or LLM pass.

Wrap a region with `// code-verify off` / `// code-verify on` to disable
every rule between the fences (the `/* ... */` equivalent works too).
`code-format off/on` is accepted as a legacy synonym.

Usage:
    python3 scripts/code-verify.py                          # fix everything under app/qml + app/src
    python3 scripts/code-verify.py --check                  # report-only, whole tree
    python3 scripts/code-verify.py --check app/qml          # report-only, explicit path
    python3 scripts/code-verify.py --fix app/src            # rewrite C++ files
    python3 scripts/code-verify.py --check app/qml/Foo.qml  # single file
    python3 scripts/code-verify.py --fix --diff app/qml     # show changes
    python3 scripts/code-verify.py --tu-census --check      # TU-size ratchet

Called with no arguments the script defaults to --fix on the entire
<repo>/app/qml and <repo>/app/src trees.

Exit codes:
    0 - clean (check) or rewrote files (fix)
    1 - errors found (check) -- advisories alone don't fail
    2 - argument error
"""

from __future__ import annotations

import argparse
import difflib
import hashlib
import importlib.util
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

# Optional semantic-rules module (scripts/code_verify_rules.py). Loaded by
# absolute path so the dashed `code-verify.py` filename doesn't trip up
# Python's import system. None when tree-sitter isn't installed.
_SEMANTIC_RULES = None
try:
    _rules_spec = importlib.util.spec_from_file_location(
        "code_verify_rules",
        Path(__file__).with_name("code_verify_rules.py"),
    )
    if _rules_spec is not None and _rules_spec.loader is not None:
        _rules_mod = importlib.util.module_from_spec(_rules_spec)
        # Register before exec so @dataclass can resolve cls.__module__.
        sys.modules["code_verify_rules"] = _rules_mod
        _rules_spec.loader.exec_module(_rules_mod)
        _SEMANTIC_RULES = _rules_mod
except Exception:
    _SEMANTIC_RULES = None


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

# `} Foo {` / `} Foo.Bar {` — the sibling-pairing idiom that closes one object
# body and opens the next on a single physical line. Net brace delta is zero,
# so the block walk needs this to tell the two bodies apart.
_CLOSE_THEN_OPEN = re.compile(r"^\s*\}\s*[A-Za-z_][\w.]*\s*\{$")

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


def _strip_strings_and_comments(line: str, fill: str = " ") -> str:
    """Return `line` with string literals and `//` comments replaced by `fill`,
    one character per source character so column positions survive, letting
    bracket-counting ignore brackets that live inside strings or end-of-line
    comments. A caller asking whether the line *ends* on an operator passes a
    non-blank `fill`: blanked out, `+ ":"` reads as a dangling `+`."""
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
            result.append(fill)
            i += 1
            while i < n:
                if line[i] == "\\" and i + 1 < n:
                    i += 2
                    result.append(fill * 2)
                    continue
                if line[i] == '"':
                    result.append(fill)
                    i += 1
                    break
                result.append(fill)
                i += 1
            continue

        # Single-quoted string (JS string literal)
        if ch == "'":
            result.append(fill)
            i += 1
            while i < n:
                if line[i] == "\\" and i + 1 < n:
                    i += 2
                    result.append(fill * 2)
                    continue
                if line[i] == "'":
                    result.append(fill)
                    i += 1
                    break
                result.append(fill)
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
    sanitized = _strip_strings_and_comments(line, fill="0").rstrip()
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
        m = re.match(
            r"\s*(?:readonly\s+)?(?:property\s+\w+\s+)?[A-Za-z_][\w.]*\s*:(.*)$",
            sanitized,
        )
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
    kind: str = (
        "other"  # "id" | "prop" | "blank" | "comment" | "open" | "close" | "handler" | "other"
    )
    start_idx: int = 0  # index of first physical line in the source

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


def closes_then_opens(line: LogicalLine) -> bool:
    """True for a single physical line of the form `} Foo {`, which ends one
    object body and starts the next sibling's."""
    if line.kind in ("comment", "blank") or len(line.raws) != 1:
        return False
    return bool(_CLOSE_THEN_OPEN.match(line.raws[0].rstrip()))


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
# Brace-free body blank-line rule (applies to QML + C++)
# ---------------------------------------------------------------------------

# Lines that open a control statement whose body lives on the next line.
# Matches `if (...)`, `else if (...)`, `for (...)`, `while (...)`, and bare
# `else`. The line must end exactly at `)` or after `else` — anything that
# ends with `{`, `;`, or any other token is not a brace-free control opener.
_BRACE_FREE_CONTROL_RE = re.compile(
    r"""
    ^ \s*
    (?:
        (?: else \s+ )? if \s* \( .* \)
      | for          \s* \( .* \)
      | while        \s* \( .* \)
      | else
    )
    \s* $
    """,
    re.VERBOSE,
)


def _strip_eol_comment(line: str) -> str:
    """Return `line` without any trailing `//` line comment, ignoring `//`
    that lives inside a string literal."""
    return _strip_strings_and_comments(line).rstrip()


def _is_brace_free_control(line: str) -> bool:
    """True when `line` opens a control statement whose body must be on the
    next physical line (i.e. it does not end with `{` and is not a one-liner
    with a `;` body)."""
    sanitized = _strip_eol_comment(line)
    if not sanitized:
        return False
    if sanitized.endswith("{") or sanitized.endswith(";"):
        return False
    if not _BRACE_FREE_CONTROL_RE.match(sanitized):
        return False
    # Reject pathological matches where the regex's greedy `.*` swallowed
    # an unbalanced paren run (e.g. `if (cond` continued on next line).
    return sanitized.count("(") == sanitized.count(")")


def _line_indent(line: str) -> int:
    """Return the count of leading whitespace columns on `line` (tabs count
    as one column — the codebase uses spaces, so this is a simple len-diff)."""
    return len(line) - len(line.lstrip())


def _find_brace_free_body_end(lines: list[str], control_idx: int) -> int | None:
    """Locate the last physical line of the brace-free body that follows the
    control statement at `control_idx`. Returns None when the body is itself
    a `{ ... }` block, when no body is found, or when source layout does not
    match the brace-free pattern (body must be more indented than the
    control)."""
    n = len(lines)
    control_line = lines[control_idx]
    control_indent = _line_indent(control_line)

    # Skip any incidental blank / comment lines between control and body
    j = control_idx + 1
    while j < n:
        stripped = lines[j].strip()
        if stripped == "" or stripped.startswith("//"):
            j += 1
            continue
        break
    if j >= n:
        return None

    body_first = j
    body_indent = _line_indent(lines[body_first])
    if body_indent <= control_indent:
        return None

    sanitized_first = _strip_eol_comment(lines[body_first])
    # Body that opens with `{` is a brace block, not brace-free
    if sanitized_first.lstrip().startswith("{") or sanitized_first.endswith("{"):
        return None

    # Walk forward absorbing continuation lines. A continuation is any line
    # that either keeps bracket depth above zero, sits at a deeper indent
    # than the body line (multi-line argument list, ternary continuation),
    # or follows a C++ attribute line like `[[unlikely]]`.
    depth = _bracket_delta(lines[body_first])
    last = body_first
    while last < n:
        sanitized = _strip_eol_comment(lines[last])
        if depth <= 0 and sanitized.endswith(";"):
            return last

        if last + 1 >= n:
            return last

        next_line = lines[last + 1]
        next_stripped = next_line.strip()
        if next_stripped == "":
            return last

        # C++ attribute lines (e.g. `[[unlikely]]`) sit between the control
        # statement and the actual statement; the attribute applies to the
        # next line, so always absorb it.
        if depth <= 0 and sanitized.endswith("]]") and not sanitized.endswith(";"):
            last += 1
            depth += _bracket_delta(lines[last])
            continue

        if depth <= 0:
            next_indent = _line_indent(next_line)
            if next_indent <= body_indent:
                return last

        last += 1
        depth += _bracket_delta(lines[last])

    return last


def find_brace_free_violations(
    lines: list[str], path: Path
) -> tuple[list[Violation], list[int]]:
    """Return (violation list, indices that need a blank line inserted after
    them). Each integer in the second list is the index of the LAST physical
    line of a brace-free body whose successor is not blank, `else`, `}`, or
    `while` (do-while continuation)."""
    violations: list[Violation] = []
    insert_after: list[int] = []

    n = len(lines)
    i = 0
    while i < n:
        if not _is_brace_free_control(lines[i]):
            i += 1
            continue

        body_end = _find_brace_free_body_end(lines, i)
        if body_end is None:
            i += 1
            continue

        nxt = body_end + 1
        if nxt >= n:
            i = body_end + 1
            continue

        nxt_stripped = lines[nxt].strip()
        if nxt_stripped == "":
            i = body_end + 1
            continue

        # Natural terminators that already separate the body from the next
        # logical chunk — no blank line needed. `#` covers every preprocessor
        # directive (#endif / #else / #elif / etc.) that may legitimately sit
        # between the body and the next statement without introducing prose.
        if (
            nxt_stripped.startswith("}")
            or nxt_stripped.startswith("#")
            or nxt_stripped.startswith("else")
            or nxt_stripped.startswith("while")
        ):
            i = body_end + 1
            continue

        violations.append(
            Violation(
                path,
                body_end + 1,
                "brace-free-blank",
                "missing blank line after brace-free single-statement body",
            )
        )
        insert_after.append(body_end)
        i = body_end + 1

    return violations, insert_after


def apply_brace_free_fixes(lines: list[str], insert_after: list[int]) -> list[str]:
    """Return a new list of lines with a blank line inserted after each
    index in `insert_after`."""
    if not insert_after:
        return lines

    inserts = set(insert_after)
    out: list[str] = []
    for i, line in enumerate(lines):
        out.append(line)
        if i in inserts:
            out.append("")
    return out


# ---------------------------------------------------------------------------
# `id:` placement check
# ---------------------------------------------------------------------------


@dataclass
class Violation:
    path: Path
    line: int
    kind: str
    message: str


def check_id_placement(
    lines: list[LogicalLine], path: Path
) -> tuple[list[Violation], list[int]]:
    """Inside each object body, `id:` must be the first non-comment/blank
    content line and must be followed by one blank line. Returns
    (violations, raw_line_indices) — the second list contains indices in
    physical-line space after which a blank line needs inserting to satisfy
    the id-blank-line rule."""
    violations: list[Violation] = []
    blanks_after: list[int] = []
    body_stack: list[int] = []

    for i, line in enumerate(lines):
        if line.kind == "open":
            body_stack.append(i + 1)
            continue

        if closes_then_opens(line):
            if body_stack:
                start = body_stack.pop()
                _check_shallow_id(lines, start, i, path, violations, blanks_after)
            body_stack.append(i + 1)
            continue

        delta = _brace_delta(line)
        if delta < 0 and body_stack:
            start = body_stack.pop()
            _check_shallow_id(lines, start, i, path, violations, blanks_after)

    return violations, blanks_after


def _check_shallow_id(
    lines: list[LogicalLine],
    start: int,
    end: int,
    path: Path,
    violations: list[Violation],
    blanks_after: list[int],
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
            if line.kind == "id":
                shallow_id_idx = i
                break
            if first_content_idx is None:
                first_content_idx = i
            # Keep walking at shallow depth: an `id:` further down is exactly the
            # misplacement this rule exists to catch, so stopping here made it dead
        # Move through nested block content
        if line.kind == "close" and inner_depth > 0:
            inner_depth += _brace_delta(line)
            continue
        inner_depth += _brace_delta(line)

    if shallow_id_idx is None:
        return

    # If the first content line wasn't the id, flag it (no auto-fix — moving
    # arbitrary content above the id would risk reordering side effects).
    if first_content_idx is not None:
        violations.append(
            Violation(
                path,
                lines[shallow_id_idx].start_idx + 1,
                "id-placement",
                "`id:` must be the first content line inside the block",
            )
        )
        return

    # Ensure blank line after the id — auto-fixable by inserting one.
    next_idx = shallow_id_idx + 1
    if next_idx < end and lines[next_idx].kind != "blank":
        id_line = lines[shallow_id_idx]
        violations.append(
            Violation(
                path,
                id_line.start_idx + 1,
                "id-blank-line",
                "expected a blank line after `id:` before the next content",
            )
        )
        # Insert blank after the id's last physical line
        blanks_after.append(id_line.start_idx + len(id_line.raws) - 1)


# ---------------------------------------------------------------------------
# Comment-style checks (flag-only — humans collapse / refactor)
# ---------------------------------------------------------------------------

# `// code-verify off` and `// code-verify on` fence raw lines out of every
# rule the script enforces. The fence text is matched on a stripped line --
# both `//` and `/*…*/` forms are accepted so QML and C++ can both opt out.
# `code-format off/on` is accepted as a legacy synonym; the fixer uses
# `code-verify` going forward.
_FENCE_OFF_RE = re.compile(r"^\s*(?://|/\*)\s*code-(?:verify|format)\s+off\b")
_FENCE_ON_RE = re.compile(r"^\s*(?://|/\*)\s*code-(?:verify|format)\s+on\b")


def _compute_fence_mask(lines: list[str]) -> list[bool]:
    """Return a per-line bitmap; True means the line sits inside a
    `// code-format off` / `// code-format on` fence and must be skipped by
    every rule. The fence lines themselves are masked too so they don't get
    flagged as multi-line comment runs."""
    mask = [False] * len(lines)
    fenced = False
    for i, line in enumerate(lines):
        if _FENCE_OFF_RE.match(line):
            fenced = True
            mask[i] = True
            continue
        if _FENCE_ON_RE.match(line):
            fenced = False
            mask[i] = True
            continue
        mask[i] = fenced
    return mask


def _is_line_comment(line: str) -> bool:
    """True when `line` is a `//` comment line (ignoring leading whitespace).
    Doxygen `///` and `//!` lines count — they are still single-line `//`
    style. Block comments (`/* */`, `/** */`) are not flagged here."""
    stripped = line.lstrip()
    return stripped.startswith("//")


_TOOLING_PRAGMA_RE = re.compile(
    r"^\s*(?:clang-format|code-(?:verify|format)|NOLINT|cppcheck-suppress|fallthrough)\b",
    re.IGNORECASE,
)


def _is_tooling_pragma(line: str) -> bool:
    """True when `line` is a `//` comment that carries a tooling pragma
    (`// clang-format off/on`, `// NOLINT...`, `// cppcheck-suppress ...`,
    `// fallthrough`). Pragmas are never prose, so they neither start a
    multi-line `//` run nor extend one.  Treating `// foo` followed by
    `// clang-format off` as a 2-line comment block is wrong — the second
    line is a directive, not narration."""
    payload = _comment_payload(line)
    if payload is None:
        return False
    return bool(_TOOLING_PRAGMA_RE.match(payload))


def _is_banner_payload(payload: str) -> bool:
    """True when a comment payload is a banner decorator: empty after the
    `//`, or punctuation-only (`---`, `===`, `***`).  These are intentional
    section markers per CLAUDE.md and are stripped from prose runs."""
    s = payload.strip()
    return s == "" or set(s) <= {"-", "=", "*"}


def _is_banner_run(payloads: list[str]) -> bool:
    """A run of consecutive `//` comment payloads is a banner when it follows
    the project's section-marker shapes:

      A) Decorators only — `//---`, `//===`, `//` (blank).  Flagging these
         as multi-line narration is wrong; they're rules between sections.
      B) Sandwich — the run starts AND ends with a decorator and every
         non-decorator line is adjacent to one.  Catches both the C++ form
         (`//---` / `// Section` / `//---`) and the QML form
         (`//` / `// Section` / `//`)."""
    if not payloads:
        return False

    if all(_is_banner_payload(p) for p in payloads):
        return True

    if not _is_banner_payload(payloads[0]) or not _is_banner_payload(payloads[-1]):
        return False

    for idx, p in enumerate(payloads):
        if _is_banner_payload(p):
            continue
        prev_dec = idx > 0 and _is_banner_payload(payloads[idx - 1])
        next_dec = idx + 1 < len(payloads) and _is_banner_payload(payloads[idx + 1])
        if not (prev_dec or next_dec):
            return False
    return True


def find_comment_style_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag the AI-typical multi-line `//` narration pattern.

    A `multi-line-comment` is ≥2 consecutive `//` comment lines that are
    NOT a banner.  CLAUDE.md caps in-body `//` comments at one line, so a
    real prose run is flagged for human collapse or removal.

    Banner shapes are intentional and skipped:
      - `//---` / `//===` decorator rules (with or without a label),
      - QML `//` / `// label` / `//` sandwich.

    EDGE CASE FOR LLMs FIXING THESE FLAGS — QML SANDWICH PRESERVATION
    ----------------------------------------------------------------
    QML files use a 3-line sandwich convention to label the type/section
    that follows:

        //
        // Plot area
        //
        Item { ... }

    This shape is INTENTIONAL and the linter skips it (see `_is_banner_run`
    sandwich rule). When you collapse a flagged 5+ line QML prose block
    (multi-line text wrapped between `//` decorators), DO NOT drop the
    decorators — keep the sandwich, just shorten the inner label to a
    single line. Wrong fix: replace the whole block with `// Plot area`
    above the `Item { }`. Right fix:

        //
        // Plot area
        //
        Item { ... }

    For C++/.cpp/.h/.mm files, the convention is the opposite — a one-line
    `// Section header` directly above the block, no decorators:

        // Reset the forward-fill cache for the new file
        m_lastFinalValues.clear();

    Banners with `//---` or `//===` decorators are also intentional in C++
    and separate concern groups (see CLAUDE.md "98-dash banners separate
    concern groups"). Don't touch those.

    HEADER FILES (.h)
    -----------------
    CLAUDE.md forbids `//` comments before member-variable / signal /
    function declarations in headers. When you encounter a flagged
    multi-line `//` block above a member variable in a header, DELETE
    the whole block — don't shorten it. Names and types are the
    documentation. The only `//` allowed in headers is the SPDX banner
    at the top; the only block-doc allowed is a `/** @brief */` directly
    above a TYPE-LEVEL definition (class, struct, enum, top-level
    typedef/using). If the flagged block is above an enum or struct, you
    can convert it to a one-line `/** @brief ... */`.

    Tooling pragmas (`// clang-format off/on`, `// NOLINT`,
    `// cppcheck-suppress`, `// fallthrough`) are run-breakers — they're
    directives, not prose, so a label line directly above one is still a
    one-line comment, not a 2-line block.

    Not auto-fixable: collapsing a real multi-line comment loses
    information, so judgement belongs to a human or LLM follow-up."""
    violations: list[Violation] = []
    n = len(lines)

    i = 0
    while i < n:
        if (
            fence_mask[i]
            or not _is_line_comment(lines[i])
            or _is_tooling_pragma(lines[i])
        ):
            i += 1
            continue

        run_start = i
        while (
            i < n
            and not fence_mask[i]
            and _is_line_comment(lines[i])
            and not _is_tooling_pragma(lines[i])
        ):
            i += 1
        run_len = i - run_start

        if run_len < 2:
            continue

        payloads = [
            _comment_payload(lines[k]) or ""
            for k in range(run_start, run_start + run_len)
        ]
        if _is_banner_run(payloads):
            continue

        violations.append(
            Violation(
                path,
                run_start + 1,
                "multi-line-comment",
                f"{run_len} consecutive `//` lines — collapse to one line "
                "or drop (CLAUDE.md: in-body comments are one-line section headers)",
            )
        )

    return violations


# AI-narration patterns — phrases and shapes that ship in AI-generated code
# but rarely survive a hand-review against CLAUDE.md's comment guidance
# ("Code is the spec. Comments label sections; they do not narrate.").
# Matched case-insensitively against the text after the leading `//` markers.
# Keep this list HIGH-PRECISION: a false positive that flags hand-written
# comments will train people to ignore the report.
_AI_PATTERNS: tuple[tuple[str, "re.Pattern[str]"], ...] = (
    # First-person tutorial voice — "we", "let's", "let us"
    (
        "first-person",
        re.compile(
            r"\b(?:we'(?:ll|ve|re)|we\s+(?:need|want|have|will|should|now|first|then|can)|let'?s\b|let\s+us\b)",
            re.IGNORECASE,
        ),
    ),
    # Throat-clearing prefixes that add no information
    (
        "throat-clearing",
        re.compile(
            r"^\s*(?:note(?:\s+that)?|important|keep\s+in\s+mind|remember(?:\s+that)?|please\s+note|fyi|n\.?b\.?)\s*[:,]",
            re.IGNORECASE,
        ),
    ),
    # Tutorial step markers
    (
        "tutorial-voice",
        re.compile(
            r"^\s*(?:now|here|first(?:ly)?|second(?:ly)?|next|then|finally)\s*,?\s*(?:we\b|you\b|i\b)",
            re.IGNORECASE,
        ),
    ),
    # "This is..." / "This does..." / "This function..." narration
    (
        "this-is-narration",
        re.compile(
            r"^\s*this\s+(?:is|does|function|method|class|file|module|code|block|line)\b",
            re.IGNORECASE,
        ),
    ),
    # PR/fix/change self-references that rot the moment they're committed
    (
        "rot-reference",
        re.compile(
            r"\b(?:this\s+(?:pr|patch|fix|commit|change|cl)|the\s+(?:recent|previous|above|aforementioned)\s+(?:change|fix|pr|commit)|as\s+(?:mentioned|noted|described)\s+above|see\s+(?:above|below))\b",
            re.IGNORECASE,
        ),
    ),
    # Hedging vocabulary — "for now", "for clarity", "for readability",
    # "in theory", "ideally", "perhaps", "maybe should"
    (
        "hedging",
        re.compile(
            r"\b(?:for\s+(?:now|clarity|readability|reference|completeness)|in\s+theory|ideally|perhaps|maybe\s+(?:should|we|this)|might\s+want\s+to)\b",
            re.IGNORECASE,
        ),
    ),
    # Restating-the-code openers — strongest signal of AI prose
    (
        "restate-obvious",
        re.compile(
            r"^\s*(?:loop(?:s)?\s+(?:over|through)|iterate(?:s)?\s+(?:over|through)|check(?:s)?\s+(?:if|whether)|set(?:s)?\s+\w+\s+to|return(?:s)?\s+the|get(?:s)?\s+the|create(?:s)?\s+(?:a|an|the)|initialize(?:s)?\s+(?:a|an|the)|store(?:s)?\s+the|update(?:s)?\s+the)\b",
            re.IGNORECASE,
        ),
    ),
    # AI-typical TODO/FIXME without a ticket reference — "TODO: implement this"
    (
        "todo-no-context",
        re.compile(
            r"^\s*(?:todo|fixme|xxx|hack)\s*[:!\-]?\s*(?:implement|handle|add|fix|consider|figure\s+out|deal\s+with|come\s+back\s+to)\b",
            re.IGNORECASE,
        ),
    ),
    # Emoji in comments — CLAUDE.md bans them outright
    (
        "emoji",
        re.compile(
            r"[\U0001F300-\U0001FAFF\U00002600-\U000027BF\U0001F000-\U0001F2FF]"
        ),
    ),
    # "Used by X" / "Called from Y" — caller references rot
    (
        "caller-reference",
        re.compile(
            r"\b(?:used\s+by|called\s+(?:by|from)|invoked\s+(?:by|from)|added\s+for|needed\s+for)\s+(?:the\s+)?[A-Z]?\w",
            re.IGNORECASE,
        ),
    ),
    # Trailing ellipsis on `//` lines — typically AI hand-waving "...etc"
    (
        "trailing-ellipsis",
        re.compile(r"\.{3,}\s*$"),
    ),
)


# `--` used as a sentence dash: the spaced double-hyphen an AI reaches for
# when told to strip em dashes (ASCII-only source bans the U+2014 glyph). It
# is a mechanical glyph swap, not the rewrite the rule actually asks for. The
# fix is to recast the sentence with a comma, colon, period, or parentheses,
# never to trade one dash glyph for another. Spaces on both sides keep `i--`,
# `--i`, and `//---` banners out of the match.
_DASH_SUBSTITUTE_RE = re.compile(r"\S -- \S")


def _comment_payload(line: str) -> str | None:
    """Return the text after `//`/`///`/`//!` markers on a `//` comment line,
    or None when the line isn't a single-line comment. Leading/trailing
    whitespace is preserved on the payload so anchored patterns still see
    the start of the prose."""
    stripped = line.lstrip()
    if not stripped.startswith("//"):
        return None
    # Strip up to three slashes plus an optional `!` (`///`, `//!`, `///!`)
    # then a single space if present.
    j = 2
    while j < len(stripped) and stripped[j] == "/":
        j += 1
    if j < len(stripped) and stripped[j] == "!":
        j += 1
    if j < len(stripped) and stripped[j] == " ":
        j += 1
    return stripped[j:]


def find_ai_narration_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag `//` comment lines that match AI-narration patterns.

    These are heuristics, not proof. The aim is to surface the comments
    most likely to violate CLAUDE.md's "label, don't narrate" rule for a
    human or LLM to review. The `.code-report` header explains the rules
    so a follow-up LLM pass can decide per-line."""
    violations: list[Violation] = []

    for i, line in enumerate(lines):
        if fence_mask[i]:
            continue
        payload = _comment_payload(line)
        if payload is None:
            continue
        # Skip section-header style "//---" banners — those are intentional
        # per CLAUDE.md "98-dash banners separate concern groups".
        stripped_payload = payload.strip()
        if not stripped_payload or set(stripped_payload) <= {"-", "="}:
            continue
        # Skip `// clang-format off/on` and other tooling pragmas.
        if re.match(
            r"^\s*(?:clang-format|code-(?:verify|format)|NOLINT|cppcheck-suppress)",
            payload,
            re.IGNORECASE,
        ):
            continue

        for kind, pattern in _AI_PATTERNS:
            if pattern.search(payload):
                violations.append(
                    Violation(
                        path,
                        i + 1,
                        f"ai-{kind}",
                        f"AI-narration smell ({kind}): {payload.strip()[:80]}",
                    )
                )
                # One violation per line is enough — the worst pattern wins
                break

        # Dash-substitute is orthogonal to the tone patterns: a comment can
        # restate the code AND lean on a spaced double-hyphen. Report it on
        # its own so the worst-pattern break above doesn't mask it.
        if _DASH_SUBSTITUTE_RE.search(payload):
            violations.append(
                Violation(
                    path,
                    i + 1,
                    "comment-dash-substitute",
                    "`--` as a sentence dash; rewrite the sentence (comma / "
                    "colon / period / parentheses), don't swap em dash for "
                    f"`--`: {payload.strip()[:80]}",
                )
            )

    return violations


# Names for the most common non-ASCII characters that show up in AI-written
# code, so the report can point at them instead of just listing codepoints.
_NON_ASCII_NAMES: dict[str, str] = {
    "—": "em dash (U+2014, rewrite the sentence; don't swap in `--`)",
    "–": "en dash (U+2013, type `-`)",
    "‘": "left single quote (U+2018, type `'`)",
    "’": "right single quote / apostrophe (U+2019, type `'`)",
    "“": 'left double quote (U+201C, type `"`)',
    "”": 'right double quote (U+201D, type `"`)',
    "…": "ellipsis (U+2026, type `...`)",
    "→": "right arrow (U+2192, type `->`)",
    "←": "left arrow (U+2190, type `<-`)",
    "↔": "two-way arrow (U+2194, type `<->`)",
    "⇒": "double right arrow (U+21D2, type `=>`)",
    " ": "non-breaking space (U+00A0, type a regular space)",
    "·": "middle dot (U+00B7)",
    "×": "multiplication sign (U+00D7, type `x` or `*`)",
    "µ": "micro sign (U+00B5, type `u` or `micro`)",
    "μ": "greek mu (U+03BC, type `u` or `micro`)",
    "°": "degree sign (U+00B0, type `deg` or `degrees`)",
    "−": "minus sign (U+2212, type `-`)",
    "±": "plus-minus sign (U+00B1, type `+/-`)",
    "≈": "approximately equal (U+2248, type `~=`)",
    "≤": "less-or-equal (U+2264, type `<=`)",
    "≥": "greater-or-equal (U+2265, type `>=`)",
    "≠": "not-equal (U+2260, type `!=`)",
    "²": "superscript 2 (U+00B2, type `^2`)",
    "³": "superscript 3 (U+00B3, type `^3`)",
    "√": "square root (U+221A, type `sqrt`)",
    "∞": "infinity (U+221E, type `inf`)",
    "•": "bullet (U+2022, type `*` or `-`)",
    "½": "one half (U+00BD, type `1/2`)",
    "¼": "one quarter (U+00BC, type `1/4`)",
    "¾": "three quarters (U+00BE, type `3/4`)",
}


def _describe_non_ascii(ch: str) -> str:
    """Return a short human label for a non-ASCII character. Falls back to
    the codepoint when the character isn't in the curated table — that way
    the report names every offender without needing an exhaustive map."""
    if ch in _NON_ASCII_NAMES:
        return _NON_ASCII_NAMES[ch]
    cp = ord(ch)
    return f"U+{cp:04X} ({ch!r})"


# Translation calls — non-ASCII inside their string args is allowed because
# the strings are user-facing localized text (em dashes, ellipses, ×, °, ²
# all show up in UI labels and unit suffixes). Matched by name; the regex
# below scrubs the call's argument span before scanning the line.
_TRANSLATION_CALL_RE = re.compile(
    r"\b(?:qsTr|qsTrId|qsTranslate|qsTrNoOp|QT_TR_NOOP|QT_TRANSLATE_NOOP|"
    r"QT_TRID_NOOP|tr|trUtf8|translate)\s*\("
)

# Copyright-style year-range lines (`Copyright (C) 2020-2025 Name`) — the
# en-dash there is conventional and lives in the SPDX banner, not in code.
_COPYRIGHT_LINE_RE = re.compile(r"\bCopyright\b.*?\b\d{4}", re.IGNORECASE)


def _strip_translation_args(line: str) -> str:
    """Replace the inside of `qsTr(...)` / similar calls with ASCII filler so
    only non-ASCII outside the call body is reported. Handles balanced parens,
    string-aware so nested parens inside string literals don't end the call
    early."""
    out: list[str] = []
    i = 0
    n = len(line)
    while i < n:
        m = _TRANSLATION_CALL_RE.match(line, i)
        if not m:
            out.append(line[i])
            i += 1
            continue

        out.append(line[i : m.end()])
        i = m.end()
        depth = 1
        in_str: str | None = None
        escape = False
        while i < n and depth > 0:
            ch = line[i]
            if in_str is not None:
                if escape:
                    escape = False
                elif ch == "\\":
                    escape = True
                elif ch == in_str:
                    in_str = None
                # Replace any non-ASCII *inside* the string with a placeholder
                # so the outer scan ignores it.
                out.append("." if ord(ch) >= 128 else ch)
                i += 1
                continue

            if ch in ("'", '"', "`"):
                in_str = ch
                out.append(ch)
            elif ch == "(":
                depth += 1
                out.append(ch)
            elif ch == ")":
                depth -= 1
                out.append(ch)
            else:
                out.append("." if ord(ch) >= 128 else ch)
            i += 1
    return "".join(out)


def find_non_ascii_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag non-ASCII characters in code, comments, and non-translation
    strings. Em dashes, smart quotes, arrows, and non-breaking spaces are
    AI-prose smells that also break older toolchains: MSVC without `/utf-8`
    mis-decodes them, some legacy editors mojibake them, and grep/diff tools
    render them as escape goo. Words and ASCII operators read fine for both
    humans and LLMs — type `->`, `<=`, `1/2` instead of arrows, less-or-equal
    glyphs, fraction glyphs.

    Lines INTENTIONALLY skipped:
      - `// code-verify off` fences,
      - `Copyright ... 20YY-20YY ...` SPDX banner lines (en-dash year range),
      - text inside `qsTr(...)`, `tr(...)`, `QT_TR_NOOP(...)`, etc. — those
        are user-facing localized strings where em dashes, ellipses, ×, °,
        and superscripts are conventional. Only non-ASCII OUTSIDE the
        translation call's argument span is reported."""
    violations: list[Violation] = []
    for i, line in enumerate(lines):
        if fence_mask[i]:
            continue
        if _COPYRIGHT_LINE_RE.search(line):
            continue

        scan_target = _strip_translation_args(line) if "(" in line else line

        offenders: list[str] = []
        seen: set[str] = set()
        for ch in scan_target:
            if ord(ch) < 128 or ch == "\t":
                continue
            if ch in seen:
                continue
            seen.add(ch)
            offenders.append(_describe_non_ascii(ch))
        if not offenders:
            continue
        violations.append(
            Violation(
                path,
                i + 1,
                "non-ascii",
                "non-ASCII character(s) — replace with ASCII equivalents: "
                + "; ".join(offenders),
            )
        )
    return violations


def find_qml_inline_comment_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag single-line `//` comments that sit inside a QML object body
    (`Item { }`, `Rectangle { }`, etc.) but NOT inside a JavaScript body
    (`function () { }`, `onClicked: { }`, `() => { }`).

    QML object bodies hold declarative property bindings — labelling them
    with inline `//` notes is the AI-narration smell CLAUDE.md bans. JS
    function bodies are imperative code and follow the same one-line
    section-header rule as `.cpp` files; flagging those would double-tax
    the multi-line-comment rule, so they're left alone here.

    Banner shapes (`//`, `//---`, `//===`, sandwich runs) are NOT flagged —
    they're handled by the multi-line / banner rules. Tooling pragmas
    (`// clang-format off/on`, `// NOLINT`, `// code-verify off/on`) are
    directives, not prose, and skipped."""
    if path.suffix != ".qml":
        return []

    violations: list[Violation] = []
    n = len(lines)

    # Stack of body kinds — "qml" for an object body, "js" for a JS body.
    # We push when a line opens a body and pop on the matching close. The
    # top of the stack tells us where the current line lives.
    body_stack: list[str] = []

    for i, line in enumerate(lines):
        if fence_mask[i]:
            continue

        stripped = line.strip()

        # Skip comment-only lines for stack maintenance — they don't open
        # or close bodies.
        if (
            stripped.startswith("//")
            or stripped.startswith("/*")
            or stripped.startswith("*")
        ):
            payload = _comment_payload(line)
            if payload is not None and not _is_tooling_pragma(line):
                p = payload.strip()
                # Skip blank `//` lines and `//---`/`//===` decorator rules —
                # those are banner pieces handled by the multi-line rule.
                is_decorator = (not p) or set(p) <= {"-", "=", "*"}
                if not is_decorator and body_stack and body_stack[-1] == "qml":
                    # Lookahead: only flag a SINGLE-line `//` block. If the
                    # next or prev non-blank line is also a `//` comment,
                    # the multi-line-comment rule covers it instead.
                    nxt = i + 1
                    while nxt < n and lines[nxt].strip() == "":
                        nxt += 1
                    next_is_comment = (
                        nxt < n
                        and lines[nxt].lstrip().startswith("//")
                        and not _is_tooling_pragma(lines[nxt])
                    )
                    prev = i - 1
                    while prev >= 0 and lines[prev].strip() == "":
                        prev -= 1
                    prev_is_comment = (
                        prev >= 0
                        and lines[prev].lstrip().startswith("//")
                        and not _is_tooling_pragma(lines[prev])
                    )
                    if not next_is_comment and not prev_is_comment:
                        violations.append(
                            Violation(
                                path,
                                i + 1,
                                "qml-inline-comment",
                                "single-line `//` comment inside a QML object "
                                "body — use the `//\\n// Label\\n//` sandwich "
                                "above the declaration, or drop the comment "
                                f"({p[:60]})",
                            )
                        )
            continue

        # Body open / close tracking on non-comment lines.
        if stripped.endswith("{"):
            body_stack.append("js" if opens_js_body(line) else "qml")
        elif stripped.startswith("}"):
            if body_stack:
                body_stack.pop()

    return violations


# Matches QML property declarations:
#   property bool foo: ...
#   readonly property int bar: ...
#   default property var baz: ...
#   property alias qux: ...
#   readonly property list<Item> items: ...
# Captures the property NAME (group 1) so we can flag leading underscores.
_QML_PROPERTY_DECL = re.compile(
    r"^\s*(?:default\s+)?(?:readonly\s+)?property\s+"
    r"(?:alias|list<[^>]+>|[A-Za-z_][\w.]*)\s+"
    r"([A-Za-z_]\w*)\s*[:{]"
)


def find_qml_underscore_property_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag any QML property declared with a leading underscore.

    QML's auto-derived signal handler naming (`onFooChanged` for property
    `foo`) does not work reliably when the property starts with an
    underscore: handlers like `on_FooChanged` may never fire, which silently
    breaks change tracking. The Ribbon* widgets hit this exact bug -- a
    cache binding never updated because its `on_LiveExpandedWidthChanged`
    handler was a dead form. The fix is a project-wide convention: no
    leading underscores on QML properties.

    Signal-handler/JS-variable leading underscores are NOT flagged here --
    they're plain JS identifiers, not declarative QML properties, and
    don't trip the handler-naming auto-derivation."""
    if path.suffix != ".qml":
        return []

    violations: list[Violation] = []
    for i, line in enumerate(lines):
        if fence_mask[i]:
            continue

        # Strip an EOL comment so a `// _foo` note doesn't confuse the match.
        stripped = _strip_eol_comment(line)
        match = _QML_PROPERTY_DECL.match(stripped)
        if match is None:
            continue

        name = match.group(1)
        if not name.startswith("_"):
            continue

        violations.append(
            Violation(
                path,
                i + 1,
                "qml-underscore-property",
                f"QML property `{name}` starts with `_`. QML's auto-derived "
                "`on<Name>Changed` handler can silently skip underscore-"
                "prefixed properties, breaking change tracking. FIX BY HAND, "
                "ONE AT A TIME -- do NOT batch-rename with sed/regex. "
                "Each occurrence needs the call sites audited: a corresponding "
                "`on_FooChanged` handler may exist and be quietly dead, in "
                "which case the fix is not just a rename but also re-wiring "
                "the handler logic that never ran. Bindings that read the "
                "property must also be updated. Have an LLM (or a careful "
                "human) review every use, decide whether the handler was "
                "ever meant to fire, and rewrite accordingly.",
            )
        )

    return violations


# ---------------------------------------------------------------------------
# File-level processing
# ---------------------------------------------------------------------------


def _is_first_party(path: Path) -> bool:
    """True when `path` lives under app/qml or app/src — the only trees whose
    sources own the project's structural style. Vendored libraries (lib/),
    embedded examples, and generated artifacts keep their upstream layout
    even when they happen to match a tracked suffix."""
    parts = path.resolve().parts
    return ("app", "qml") in zip(parts, parts[1:]) or ("app", "src") in zip(
        parts, parts[1:]
    )


# QJSEngine interruption may only be triggered from the dedicated watchdog
# thread. A QTimer on the same thread as a blocking QJSValue::call() can never
# fire (the event loop is blocked), so `setInterrupted(true)` driven from a
# same-thread timer is a silent no-op against `while(true){}` -- the original
# JS-watchdog bug. DataModel::JsWatchdogThread is the one place allowed to flip
# the flag; everything else arms a DataModel::JsWatchdog instead.
_INTERRUPT_TRUE_RE = re.compile(r"\bsetInterrupted\s*\(\s*true\s*\)")
_INTERRUPT_GUARD_ALLOWED = "JsWatchdogThread.cpp"


def find_interrupt_guard_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag `setInterrupted(true)` outside the watchdog thread. The interrupt
    must come from a thread other than the one running the (blocking) engine
    call, so JsWatchdogThread.cpp is the only correct site."""
    if path.name == _INTERRUPT_GUARD_ALLOWED:
        return []

    violations: list[Violation] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue
        if _INTERRUPT_TRUE_RE.search(line):
            violations.append(
                Violation(
                    path,
                    i + 1,
                    "js-interrupt-off-thread",
                    "setInterrupted(true) outside JsWatchdogThread.cpp -- a same-thread "
                    "timer cannot interrupt a blocked QJSEngine; arm a "
                    "DataModel::JsWatchdog instead",
                )
            )
    return violations


# Every ProjectModel slot that dirties the project document (calls
# setModified(true)) must open a ProjectUndoScope so the mutation lands in the
# undo history (spec 0031) -- a mutation path that bypasses history is a
# defect. Whitelisted names are the intentional exceptions: the history
# machinery itself, history boundaries (lock/load), and the presentation /
# workspace surfaces the spec keeps outside undo history.
_UNDO_SCOPE_FILES = re.compile(
    r"Project(?:Model|Entities|OutputWidgets|BulkOps|Sources|Tables|Folders|"
    r"Workspaces|Loader|Persistence|Presentation)(\.cpp$|[A-Z]\w*\.cpp$)"
)
_UNDO_FUNC_RE = re.compile(
    r"^[A-Za-z_][\w:<>&*\s]*DataModel::Project(?:Model|Entities|OutputWidgets|BulkOps|"
    r"Sources|Tables|Folders|Workspaces|Loader|Persistence|Presentation)::(\w+)\s*\("
)
_UNDO_SET_MODIFIED_RE = re.compile(r"\bsetModified\s*\(\s*true\s*\)")
_UNDO_SCOPE_RE = re.compile(
    r"\bProjectUndoScope\b|\bProjectUndoFrame\b|\bcommitPending\b"
)
_UNDO_SCOPE_WHITELIST = frozenset(
    {
        # History machinery + boundaries
        "setModified",
        "applyHistorySnapshot",
        "lockProject",
        "unlockProject",
        # Presentation-state setters (outside undo history by spec 0031)
        "saveWidgetSetting",
        "stageDisplayTitle",
        "setDisplayTitle",
        "setWidgetDisplayTitle",
        "setFreezeTitleMode",
        "setExternalWindows",
        "setTreeExpansion",
        "setDiagramCollapse",
        "savePluginState",
        "setActiveGroupId",
        "setGroupLayout",
        # Workspace surface (outside undo history by spec 0031)
        "addWorkspace",
        "deleteWorkspace",
        "clearAllWorkspaces",
        "renameWorkspace",
        "updateWorkspace",
        "setWorkspaceIcon",
        "reorderWorkspaces",
        "addWidgetToWorkspace",
        "removeWidgetFromWorkspace",
        "cleanupWorkspaceWidgetRefs",
        "setCustomizeWorkspaces",
        "autoGenerateWorkspaces",
        "resetWorkspacesToAuto",
        "mergeAutoWorkspaceUpdates",
        "moveWorkspace",
        "hideGroup",
        "showGroup",
        "showAllHiddenGroups",
        "addWorkspaceFolder",
        "renameWorkspaceFolder",
        "deleteWorkspaceFolder",
        "moveWorkspaceToFolder",
        "moveFolderToFolder",
        "moveWorkspaceInFolder",
        "moveWorkspaceFolderInParent",
        # Nested private helpers (always run under a scoped public slot)
        "duplicateTableByPath",
        "appendTableCopyToFolder",
        "setGroupsInFolderEnabled",
        "duplicateGroupFolderSubtree",
        "duplicateTableFolderSubtree",
    }
)


def find_undo_scope_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag ProjectModel functions that call setModified(true) without opening
    a ProjectUndoScope/ProjectUndoFrame (and are not whitelisted): the edit
    would mutate the document while bypassing the undo history."""
    if not _UNDO_SCOPE_FILES.search(path.name) or "Editor" in path.name:
        return []

    violations: list[Violation] = []
    func_name = None
    func_line = 0
    body: list[str] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue

        m = _UNDO_FUNC_RE.match(line)
        if m is not None:
            func_name = m.group(1)
            func_line = i + 1
            body = []
            continue

        if func_name is None:
            continue

        body.append(line)
        if line.startswith("}"):
            joined = "\n".join(body)
            if (
                _UNDO_SET_MODIFIED_RE.search(joined)
                and not _UNDO_SCOPE_RE.search(joined)
                and func_name not in _UNDO_SCOPE_WHITELIST
            ):
                violations.append(
                    Violation(
                        path,
                        func_line,
                        "undo-scope-missing",
                        f"{func_name}() calls setModified(true) without a "
                        "ProjectUndoScope -- the mutation bypasses undo history "
                        "(spec 0031); open a scope or whitelist it in "
                        "code-verify.py",
                    )
                )
            func_name = None

    return violations


# Driver configuration setters must be idempotent: re-applying the current value has to be a
# complete no-op (no lookup, no emit). The UI/live/project sync fabric replays identical
# settings constantly, and the one unguarded setter (Network::setRemoteAddress) turned that
# echo into the 2026-08-10 reconnect loop that got an IP banned. A setter may instead gate on
# isOpen() when the value cannot apply to a live device at all (Audio). setDriverProperty
# dispatchers are exempt: they fan out to the concrete setters, which are the guarded surface.
_DRIVER_SETTER_RE = re.compile(r"^(?:void|bool)\s+IO::Drivers::\w+::(set[A-Z]\w*)\s*\(")
_DRIVER_SETTER_EXEMPT_PARAM_RE = re.compile(r"\b(?:QVariant|QJsonObject|QJsonValue)\b")
_DRIVER_SETTER_GUARD_RE = re.compile(
    r"\bm_\w+(?:\.\w+\(\))?\s*[=!]=[^=]|[=!]=\s*m_\w+|\bisOpen\s*\(\s*\)"
)
_DRIVER_SETTER_MUTATION_RE = re.compile(r"\bm_\w+\s*=[^=]|\bQ_EMIT\b")


def find_driver_setter_guard_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag a concrete driver setter with a scalar/QString parameter whose body
    has neither a same-value early return nor an isOpen() gate."""
    if "IO/Drivers" not in path.as_posix():
        return []

    violations: list[Violation] = []
    func_name = None
    func_line = 0
    body: list[str] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue

        m = _DRIVER_SETTER_RE.match(line)
        if m is not None:
            name = m.group(1)
            signature = line
            for extra in raw_lines[i + 1 : i + 4]:
                if ")" in signature:
                    break
                signature += " " + extra.strip()

            params = signature[signature.find("(") + 1 :]
            params = params[: params.find(")")] if ")" in params else params
            if (
                name != "setDriverProperty"
                and params.strip()
                and not _DRIVER_SETTER_EXEMPT_PARAM_RE.search(params)
            ):
                func_name = name
                func_line = i + 1
                body = []
            continue

        if func_name is None:
            continue

        body.append(line)
        if line.startswith("}"):
            joined = "\n".join(body)
            mutates = _DRIVER_SETTER_MUTATION_RE.search(joined) is not None
            guarded = _DRIVER_SETTER_GUARD_RE.search(joined) is not None
            if mutates and not guarded:
                violations.append(
                    Violation(
                        path,
                        func_line,
                        "driver-setter-guard",
                        f"{func_name}() mutates or emits without a same-value "
                        "comparison or isOpen() gate -- the settings sync "
                        "fabric replays identical values, and an unguarded "
                        "setter turns that echo into lookups/emits (spec 0050; "
                        "the telehack reconnect loop). Guard it or gate it.",
                    )
                )
            func_name = None

    return violations


# Every string-to-number parse goes through the SerialStudio::toDouble()
# overload set (fast_float-backed). Qt's QString/QVariant toDouble() walks the
# full locale + double-conversion pipeline even for plainly non-numeric text
# (a measured >2x frame-parse throughput loss with string columns), and
# QJsonValue::toDouble() silently drops string-typed numbers. SerialStudio.h
# hosts the canonical implementation, so its internal Qt fallback is the one
# allowed call site.
_TODOUBLE_RE = re.compile(r"(?:\.|->)\s*toDouble\s*\(")
_TODOUBLE_ALLOWED = "SerialStudio.h"
_TODOUBLE_SUFFIXES = (".cpp", ".h", ".c", ".mm")


def find_todouble_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag direct Qt `.toDouble()` / `->toDouble()` calls outside
    SerialStudio.h. SerialStudio::toDouble() is the project-wide numeric
    parse: fast_float-backed, string-payload aware, and total (never fails)."""
    if path.name == _TODOUBLE_ALLOWED or path.suffix not in _TODOUBLE_SUFFIXES:
        return []

    violations: list[Violation] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue
        if _TODOUBLE_RE.search(line):
            violations.append(
                Violation(
                    path,
                    i + 1,
                    "qt-todouble-direct",
                    "direct Qt .toDouble() call -- use the SerialStudio::toDouble() "
                    "overload set (QStringView / QByteArrayView / QVariant / "
                    "QJsonValue; fast_float-backed, parses string payloads, never "
                    "fails). Deliberate locale-aware parsing belongs behind a "
                    "`// code-verify off` fence.",
                )
            )
    return violations


# Q_ASSERT is compiled out under QT_NO_DEBUG, so every precondition it guards
# is unchecked in the shipped binary — the assert reads as a guard while release
# performs the unchecked subscript / shift / divide anyway. SS_ASSERT (from
# app/src/SSAssert.h) keeps the debug abort and adds a release path that reports
# once per site and runs a caller-supplied recovery action. SSAssert.h defines
# the wrapper; HotpathOptimization.h holds SS_ASSUME's debug fallback.
_QASSERT_RE = re.compile(r"\bQ_ASSERT(?:_X)?\s*\(")
_QASSERT_ALLOWED = frozenset({"SSAssert.h", "HotpathOptimization.h"})
_QASSERT_SUFFIXES = (".cpp", ".h", ".c", ".mm")


def find_qassert_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag direct `Q_ASSERT` / `Q_ASSERT_X` calls. Advisory while the ~980-site
    migration lands; promote to error in the commit that clears the report."""
    if path.name in _QASSERT_ALLOWED or path.suffix not in _QASSERT_SUFFIXES:
        return []

    violations: list[Violation] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue
        if _QASSERT_RE.search(line):
            violations.append(
                Violation(
                    path,
                    i + 1,
                    "qt-qassert-direct",
                    "`Q_ASSERT` is compiled out of release builds -- the "
                    "precondition is unchecked in the shipped binary. Use "
                    "`SS_ASSERT(cond, <recovery>)` from SSAssert.h, "
                    "`SS_ASSERT_LOG(cond)` when no recovery is meaningful, or "
                    "`SS_ASSUME(cond)` for a guard that provably already ran in "
                    "a zero-branch hot kernel. An assert whose condition is too "
                    "expensive to evaluate in release belongs behind a "
                    "`// code-verify off` fence with a one-line why.",
                )
            )
    return violations


# SS_ASSERT_HOTPATH compiles out of release builds, trading the release-safety
# contract of SS_ASSERT for zero per-frame cost. That trade is only sound inside
# the per-frame/per-cell kernels where the condition restates a guard that
# provably already ran (the 2026-07 campaign's wholesale SS_ASSERT swap cost ~5%
# of hotpath throughput). Everywhere else SS_ASSERT stays the default, so the
# macro is pinned to the hotpath TUs; growing this list is a review decision.
_HOTPATH_ASSERT_RE = re.compile(r"\bSS_ASSERT_HOTPATH\s*\(")
_HOTPATH_ASSERT_ALLOWED = (
    "app/src/SSAssert.h",
    "app/src/DSPSimd.h",
    "app/src/DataModel/Frame.h",
    "app/src/DataModel/Frame.cpp",
    "app/src/DataModel/DataBlock.h",
    "app/src/DataModel/FrameBuilder.h",
    "app/src/DataModel/FrameBuilder.cpp",
    "app/src/DataModel/FrameBuilder/BlockPublisher.cpp",
    "app/src/DataModel/FrameBuilder/BlockStager.cpp",
    "app/src/DataModel/FrameBuilder/ReplayIngest.cpp",
    "app/src/IO/CircularBuffer.h",
    "app/src/IO/CircularBuffer.cpp",
    "app/src/IO/FrameReader.h",
    "app/src/IO/FrameReader.cpp",
    "app/src/IO/PipelineHost.h",
    "app/src/IO/PipelineHost.cpp",
    "app/src/IO/StreamWorker.h",
    "app/src/IO/StreamWorker.cpp",
    "app/src/UI/Dashboard.h",
    "app/src/UI/Dashboard.cpp",
)


def find_hotpath_assert_scope_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag `SS_ASSERT_HOTPATH` outside the hotpath TU whitelist (error)."""
    posix = path.as_posix()
    if any(posix.endswith(allowed) for allowed in _HOTPATH_ASSERT_ALLOWED):
        return []

    violations: list[Violation] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue
        if _HOTPATH_ASSERT_RE.search(line):
            violations.append(
                Violation(
                    path,
                    i + 1,
                    "hotpath-assert-scope",
                    "`SS_ASSERT_HOTPATH` compiles out of release builds and is "
                    "reserved for the per-frame/per-cell hotpath TUs where its "
                    "condition restates a guard that provably already ran. Use "
                    "`SS_ASSERT(cond, <recovery>)` / `SS_ASSERT_LOG(cond)` here, "
                    "or extend the whitelist in a reviewed commit if this file "
                    "genuinely joined the hotpath.",
                )
            )
    return violations


# Trial parity: an active trial installs a valid CommercialToken, so every Pro
# feature gate must count trial users as entitled -- SerialStudio::activated()
# in C++, or the paid probe paired with Cpp_Licensing_Trial state in QML.
# Gating a feature on a paid-only probe (LemonSqueezy isActivated, offline
# activation, FeatureTier comparisons) silently strips it from trial users, who
# then report the feature as broken instead of evaluating it. A paid probe with
# a Trial probe within _TRIAL_PARITY_WINDOW lines counts as a deliberate
# trial-aware expression; licence-management and entitlement-reporting surfaces
# are allowlisted (growing that list is a review decision). The doc-side twin
# below holds user-facing Markdown to the same bar: a licence-requirement
# claim must mention the trial nearby, because "needs an activated licence"
# reads as "buy first" to the very user the trial exists to convert.
_TRIAL_PARITY_PAID_RE = re.compile(
    r"\bisActivated\b|\bfeatureTier\s*\(|\bFeatureTier::"
    r"|Cpp_Licensing_OfflineLicense\s*\.\s*activated"
)
_TRIAL_PARITY_TRIAL_RE = re.compile(
    r"\btrialEnabled\b|\btrialExpired\b|\btrialAvailable\b|\bdaysRemaining\b"
    r"|\bLicensing::Trial\b|\bCpp_Licensing_Trial\b"
)
_TRIAL_PARITY_WINDOW = 4
_TRIAL_PARITY_ALLOWED = (
    "app/src/Misc/Translator.cpp",
    "app/src/Misc/CLI.cpp",
    "app/src/API/Handlers/LicensingHandler.cpp",
    "app/qml/main.qml",
    "app/qml/Dialogs/LicenseManagement.qml",
    "app/qml/Dialogs/Welcome.qml",
    "app/qml/Dialogs/About.qml",
)

_TRIAL_PARITY_DOC_RE = re.compile(
    r"(?:needs?|requires?)\s+(?:an?\s+|the\s+)?"
    r"(?:valid\s+|activated\s+|commercial\s+)*licen[cs]e"
    r"|licen[cs]e\s+(?:key\s+)?(?:is\s+)?required",
    re.IGNORECASE,
)
_TRIAL_PARITY_DOC_TRIAL_RE = re.compile(r"\btrial\b", re.IGNORECASE)
_DOC_FENCE_OFF_RE = re.compile(r"<!--\s*doc-verify\s+off\s*-->")
_DOC_FENCE_ON_RE = re.compile(r"<!--\s*doc-verify\s+on\s*-->")


def find_trial_parity_violations(
    raw_lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag paid-licence entitlement probes outside the licensing surfaces
    with no trial-aware complement nearby: an active trial counts as
    activated, so such a gate silently excludes trial users."""
    posix = path.as_posix()
    if "app/src/Licensing/" in posix:
        return []
    if any(posix.endswith(allowed) for allowed in _TRIAL_PARITY_ALLOWED):
        return []

    violations: list[Violation] = []
    for i, line in enumerate(raw_lines):
        if i < len(fence_mask) and fence_mask[i]:
            continue
        if not _TRIAL_PARITY_PAID_RE.search(line):
            continue

        lo = max(0, i - _TRIAL_PARITY_WINDOW)
        hi = min(len(raw_lines), i + _TRIAL_PARITY_WINDOW + 1)
        if _TRIAL_PARITY_TRIAL_RE.search("\n".join(raw_lines[lo:hi])):
            continue

        violations.append(
            Violation(
                path,
                i + 1,
                "trial-parity",
                "paid-licence probe (isActivated / FeatureTier / offline "
                "activation) with no trial-aware complement -- an active "
                "trial installs a valid CommercialToken, so Pro gates go "
                "through SerialStudio::activated() in C++ or pair the probe "
                "with Cpp_Licensing_Trial state in QML. A genuine "
                "licence-management surface belongs in the allowlist in "
                "code-verify.py.",
            )
        )
    return violations


def _is_user_facing_markdown(path: Path) -> bool:
    """True for the Markdown a customer reads: the doc/help manual, the
    examples READMEs, and the repo-root README. doc/claude and other
    AI-facing material stay out of scope."""
    posix = path.as_posix()
    if "/doc/claude/" in posix or posix.startswith("doc/claude/"):
        return False
    for marker in ("/doc/help/", "/examples/"):
        if marker in posix or posix.startswith(marker.strip("/") + "/"):
            return True
    repo_root = Path(__file__).resolve().parent.parent
    return path.name == "README.md" and path.resolve().parent == repo_root


def find_trial_parity_doc_violations(
    raw_lines: list[str], path: Path
) -> list[Violation]:
    """Flag a licence-requirement claim in user-facing Markdown with no trial
    mention within the window: trial users read it as 'buy first' and either
    walk away or misreport a working feature as broken. Honors the Markdown
    `<!-- doc-verify off/on -->` fences."""
    violations: list[Violation] = []
    fenced = False
    for i, line in enumerate(raw_lines):
        if _DOC_FENCE_OFF_RE.search(line):
            fenced = True
            continue
        if _DOC_FENCE_ON_RE.search(line):
            fenced = False
            continue
        if fenced or not _TRIAL_PARITY_DOC_RE.search(line):
            continue

        lo = max(0, i - _TRIAL_PARITY_WINDOW)
        hi = min(len(raw_lines), i + _TRIAL_PARITY_WINDOW + 1)
        if _TRIAL_PARITY_DOC_TRIAL_RE.search("\n".join(raw_lines[lo:hi])):
            continue

        violations.append(
            Violation(
                path,
                i + 1,
                "trial-parity",
                "licence-requirement claim with no trial mention nearby -- "
                "every Pro feature is fully available during the free trial, "
                "and wording that hides this reads as 'buy first' to the "
                "user the trial exists to convert. Say the feature works "
                "with a licence or during the free trial (link "
                "Pro-vs-Free.md), or fence a deliberate exception with "
                "<!-- doc-verify off -->.",
            )
        )
    return violations


def process_file(path: Path, fix: bool) -> tuple[list[Violation], str | None]:
    # Read as bytes first so CRLF detection isn't masked by Python's universal
    # newline translation in text mode — read_text() silently rewrites \r\n
    # and bare \r to \n, hiding the very thing we want to flag.
    raw_bytes = path.read_bytes()
    raw_text = raw_bytes.decode("utf-8")
    raw_lines = raw_text.splitlines()

    first_party = _is_first_party(path)
    is_qml = first_party and path.suffix == ".qml"
    violations: list[Violation] = []
    new_raws: list[str] = list(raw_lines)
    qml_changed = False
    id_blanks: list[int] = []

    fence_mask = _compute_fence_mask(raw_lines)

    # CRLF/CR line endings get normalized to LF on rewrite. `splitlines()`
    # already discards \r\n / \r / \n uniformly, so the round-trip via
    # "\n".join() at the bottom of this function strips them — we just need
    # to register the change so the file actually gets written out.
    crlf_changed = b"\r" in raw_bytes
    if crlf_changed:
        violations.append(
            Violation(path, 1, "line-endings", "CRLF/CR line endings normalized to LF")
        )

    # Comment-style and AI-narration checks apply to first-party C/C++/QML
    # only — vendored sources keep their upstream comment style.
    if first_party and path.suffix in _BRACE_FREE_SUFFIXES:
        violations.extend(find_comment_style_violations(raw_lines, path, fence_mask))
        violations.extend(find_ai_narration_violations(raw_lines, path, fence_mask))
        violations.extend(find_non_ascii_violations(raw_lines, path, fence_mask))
        violations.extend(
            find_qml_inline_comment_violations(raw_lines, path, fence_mask)
        )
        violations.extend(
            find_qml_underscore_property_violations(raw_lines, path, fence_mask)
        )
        violations.extend(find_interrupt_guard_violations(raw_lines, path, fence_mask))
        violations.extend(find_todouble_violations(raw_lines, path, fence_mask))
        violations.extend(find_qassert_violations(raw_lines, path, fence_mask))
        violations.extend(
            find_hotpath_assert_scope_violations(raw_lines, path, fence_mask)
        )
        violations.extend(find_undo_scope_violations(raw_lines, path, fence_mask))
        violations.extend(
            find_driver_setter_guard_violations(raw_lines, path, fence_mask)
        )
        violations.extend(find_trial_parity_violations(raw_lines, path, fence_mask))

        # Static-analysis rules (Qt/C++ semantic checks + QML conventions).
        # The rules module degrades gracefully when tree-sitter is missing.
        if _SEMANTIC_RULES is not None:
            for f in _SEMANTIC_RULES.analyze(path, raw_text, fence_mask):
                violations.append(Violation(path, f.line, f.kind, f.message))

    # Trial-parity wording check for the Markdown a customer reads.
    if path.suffix == ".md" and _is_user_facing_markdown(path):
        violations.extend(find_trial_parity_doc_violations(raw_lines, path))

    # Translation-unit size. The style contract caps functions at 100 lines
    # but said nothing about the file holding them, so god TUs accreted one
    # method per feature (FrameBuilder.cpp reached 4574 lines on the
    # hotpath). Flagged per file here, ratcheted in aggregate by --tu-census.
    if first_party and path.suffix in _TU_CENSUS_SUFFIXES:
        tu_lines = len(raw_lines)
        if tu_lines > _TU_CENSUS_THRESHOLD:
            violations.append(
                Violation(
                    path,
                    1,
                    "cxx-tu-too-long",
                    f"{tu_lines} lines (limit {_TU_CENSUS_THRESHOLD}); split with "
                    "a real sub-object in a sibling directory (one class = one .h/.cpp "
                    "pair) -- the aggregate ratchet is "
                    "code-verify.py --tu-census --check",
                )
            )

    # Christmas-tree property sort + id-placement check are QML-specific.
    if is_qml:
        lines = tokenize(raw_lines)
        id_violations, id_blanks = check_id_placement(lines, path)
        violations.extend(id_violations)

        runs = find_property_runs(lines)
        sortable_runs: list[tuple[int, int]] = []
        for start, end in runs:
            run_lines = lines[start:end]
            if not is_run_safe(run_lines):
                continue
            if is_sorted_ascending(run_lines):
                continue
            sortable_runs.append((start, end))
            violations.append(
                Violation(
                    path,
                    run_lines[0].start_idx + 1,
                    "christmas-tree",
                    f"{end - start} properties not sorted shortest->longest",
                )
            )

        if sortable_runs:
            new_lines = list(lines)
            for start, end in reversed(sortable_runs):
                sorted_block = sort_run(new_lines[start:end])
                new_lines[start:end] = sorted_block
            new_raws = []
            for line in new_lines:
                new_raws.extend(line.raws)
            qml_changed = True

    # Apply id-blank-line fixes against the (post-sort) raw line buffer.
    id_changed = bool(id_blanks)
    if id_changed:
        new_raws = apply_brace_free_fixes(new_raws, id_blanks)

    # Brace-free body blank-line rule applies to QML and C++ alike. Run
    # after id-blank fixes so the indices we collect line up with the
    # buffer we'll mutate. Other suffixes (Python, JS, Lua, …) get only
    # the CRLF normalization pass.
    bf_changed = False
    if first_party and path.suffix in _BRACE_FREE_SUFFIXES:
        bf_violations, insert_after = find_brace_free_violations(new_raws, path)
        # Re-derive the fence mask against the post-fix buffer (sort/id-blank
        # fixes can shift line numbers). Drop violations and inserts that
        # land inside a fenced range.
        post_mask = _compute_fence_mask(new_raws)
        bf_violations = [
            v
            for v in bf_violations
            if v.line - 1 < len(post_mask) and not post_mask[v.line - 1]
        ]
        insert_after = [
            idx for idx in insert_after if idx < len(post_mask) and not post_mask[idx]
        ]
        violations.extend(bf_violations)
        bf_changed = bool(insert_after)
        if bf_changed:
            new_raws = apply_brace_free_fixes(new_raws, insert_after)

    if not fix or (
        not qml_changed and not id_changed and not bf_changed and not crlf_changed
    ):
        return violations, None

    new_text = "\n".join(new_raws)
    if raw_text.endswith("\n") and not new_text.endswith("\n"):
        new_text += "\n"

    if new_text == raw_text:
        return violations, None

    return violations, new_text


# Suffixes that get full structural processing (christmas-tree / id /
# brace-free) plus CRLF normalization.
_BRACE_FREE_SUFFIXES = (".qml", ".cpp", ".h", ".c", ".mm")

# Suffixes that get CRLF normalization only — extend the structural-rule
# set with text formats that frequently arrive from Windows editors.
_TRACKED_SUFFIXES = _BRACE_FREE_SUFFIXES + (
    ".txt",
    ".md",
    ".py",
    ".svg",
    ".ts",
    ".js",
    ".lua",
    ".html",
    ".rcc",
    ".yml",
)


_SKIP_DIRS = {"build", "_deps", "ThirdParty", ".git", "node_modules", "qm"}

# The linter's own test corpus: every `bad.*` sample there is deliberately malformed, so
# scanning it would report the fixtures as findings and --fix would repair them, deleting the
# coverage. Only an explicitly-named path reaches them (which is how the tests run).
_SKIP_PATH_SUFFIXES = ("scripts/tests/fixtures", "app/tests/fuzz/corpus")


def _is_skipped_tree(path: Path) -> bool:
    posix = path.as_posix()
    return any(
        f"/{tail}/" in posix or posix.startswith(f"{tail}/")
        for tail in _SKIP_PATH_SUFFIXES
    )


def iter_source_files(targets: list[Path]) -> Iterable[Path]:
    """Yield tracked-suffix files under each target (recursive for dirs).
    Skips build / _deps / .git / vendored directories so generated and
    third-party artifacts aren't touched."""
    for target in targets:
        if target.is_file():
            if target.suffix in _TRACKED_SUFFIXES:
                yield target
            continue
        for root, dirs, files in os.walk(target):
            dirs[:] = [d for d in dirs if d not in _SKIP_DIRS]
            if _is_skipped_tree(Path(root)):
                dirs[:] = []
                continue
            for name in files:
                if name.endswith(_TRACKED_SUFFIXES):
                    yield Path(root) / name


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def default_targets() -> list[Path]:
    """Return the trees the formatter walks when no paths are given.

    `app/qml` and `app/src` get the full structural rule set; the rest are
    here so CRLF normalization reaches the wider tree of text/source files
    that formerly went through `sanitize-commit.sh`'s shell pass. Repo-root
    text files (CMakeLists.txt, *.md, *.yml) are picked up via individual
    file paths — `os.walk` from `repo` would also descend `build/`, which
    `_SKIP_DIRS` filters but at the cost of stat-ing every entry."""
    # scripts/code-verify.py  ->  <repo>
    repo = Path(__file__).resolve().parent.parent
    targets: list[Path] = [
        repo / "app",
        repo / "lib",
        repo / "examples",
        repo / "scripts",
        repo / "doc",
    ]
    for entry in repo.iterdir():
        if entry.is_file() and entry.suffix in _TRACKED_SUFFIXES:
            targets.append(entry)
    return targets


# Kinds the script CAN auto-fix in place. Anything else (multi-line-comment,
# ai-*, christmas-tree-once-merged) is flag-only and lives in `.code-report`
# for human / LLM follow-up.
_AUTO_FIXABLE_KINDS = frozenset(
    {
        "line-endings",
        "id-blank-line",
        "brace-free-blank",
        "christmas-tree",
    }
)

# Advisory rules: reported, written to `.code-report`, but DO NOT fail
# `--check`. These are CLAUDE.md guidance (length / nesting limits, doc
# coverage, opportunistic Keys::* migration) where the existing codebase
# has accumulated baseline debt. New code should still aim to clear them
# -- a reviewer reading `.code-report` is the enforcement mechanism, not
# CI. Errors (everything not in this set) are the contract CI enforces.
_ADVISORY_KINDS = frozenset(
    {
        # CLAUDE.md guidance with broad existing-code debt -- promote to
        # error once the report-driven cleanup catches up.
        "cxx-function-too-long",
        "cxx-nesting-too-deep",
        "cxx-anonymous-namespace",
        # Translation-unit size. Advisory per file because the 34 TUs
        # already over the line cannot be split in one pass; the gate
        # that actually blocks accretion is the --tu-census ratchet,
        # which fails when the excess-over-threshold pool grows.
        "cxx-tu-too-long",
        # Raw-ownership / raw-memory rules. Each looks for the absence of
        # every sanctioned owner (Qt parent, delete, deleteLater, smart
        # pointer) rather than for `new` itself -- the app has ~440 `new`
        # sites and almost all of them are parented. Advisory: the analysis
        # is intraprocedural, so an ownership transfer it cannot see reads
        # as a leak, and a wrong CI failure on a legitimate handoff costs
        # more than the report entry does.
        "mem-unreleased-owning-member",
        "mem-leaked-local-new",
        "mem-memfn-nontrivial",
        # `while (cond)` has no syntactically-visible upper bound. NASA P10
        # rule 2 requires every loop to articulate one. We shipped a hard
        # GUI-thread freeze in WindowManager::tileGrid (May 2026) when
        # `while (cols * rows < n)` wrapped around on int overflow -- the
        # bound was implicit, the overflow was missed in review, the loop
        # never terminated. Audit-only until the existing ~300 occurrences
        # are migrated to `for (int i = 0; i < kMax && cond; ++i)` or
        # explicitly fenced with `// code-verify off` once their bound is
        # documented in a comment.
        "cxx-while-loop",
        "qt-missing-nodiscard",
        "doc-missing-brief-cpp",
        "doc-missing-brief-h",
        "keys-hardcoded-literal",
        # AI-narration / multi-line-comment / qml-inline-comment are
        # heuristic-only and intentionally never fail CI. They populate the
        # report so a human / LLM cleanup pass has a checklist.
        "ai-first-person",
        "ai-throat-clearing",
        "ai-tutorial-voice",
        "ai-this-is-narration",
        "ai-rot-reference",
        "ai-hedging",
        "ai-restate-obvious",
        "ai-todo-no-context",
        # `--` as a sentence dash in a comment: the mechanical em-dash swap
        # an AI makes under the ASCII-only rule instead of rewriting the
        # sentence. Advisory, since hundreds of existing occurrences are the
        # cleanup checklist; new comments should recast the sentence instead.
        "comment-dash-substitute",
        "multi-line-comment",
        "qml-inline-comment",
        # `id:` not the first content line in a QML object body. The rule's walk stopped at the
        # first content line, so this half never fired at all (spec 0075, coordinator item 8);
        # fixing the walk surfaced ~150 existing sites, which are the cleanup checklist.
        # `id-blank-line`, the auto-fixable half, stays blocking.
        "id-placement",
        # Clone families. Reported by --dup-census (a per-file pass cannot see a pair) and
        # ratcheted against scripts/dup-census.json; advisory because the existing families
        # are the cleanup checklist.
        "cxx-duplicate-window",
        # SDK staleness is best-effort: it cannot see C++-registered commands
        # until api-schema.json is re-dumped, so a clean run does not prove the
        # SDK is current -- only that prelude/generator edits were regenerated.
        # Hand-edit detection is heuristic (banner check). Both ship advisory;
        # the hard error is sdk-byte-param-by-name (a real correctness bug).
        "sdk-out-of-date",
        "sdk-generated-edited",
        # Any comment inside a function body. Functions here are capped at
        # 100 lines, so the contract is a one-line `/** @brief ... */` above
        # the function plus self-explanatory code. Ships advisory: the
        # existing codebase has thousands of in-body comments, so the report
        # is the cleanup checklist. New code should carry none.
        "cxx-inbody-comment",
        # Functions whose whole body is dead weight: empty, only `Q_UNUSED`, or
        # a single constant return (`bool f() { return true; }`). Dead-code
        # removal candidates -- advisory and report-only because deleting one
        # ripples into its declaration, Q_PROPERTY blocks, QML bindings, and
        # call sites, which is a human-approved step, not an auto-fix.
        "cxx-trivial-function",
        # File-scope static/constexpr/const constants declared after the first
        # function instead of in a top `// Constants` section. Organizational;
        # advisory because hoisting is a manual move (group + dedupe + keep any
        # `#ifdef` guards) the report drives, not an auto-fix.
        "cxx-scattered-constant",
        # comment-narration is the AST-style scan from code_verify_rules.py
        # (separate driver). Banned tone / phrasing in any comment line.
        "comment-narration",
        # Function doxygen blocks above member-function declarations in headers
        # -- forbidden by CLAUDE.md, but the existing codebase has hundreds
        # so it ships as advisory.
        "doc-header-function-block",
        # Trailing doxygen `/**< ... */` member comments -- same ban, separate
        # signal because the fix is "delete the trailing block" rather than
        # "delete the leading block".
        "doc-trailing-member",
        # Verbose doxygen blocks (carry `@param`/`@return`/`@note`/`@see`,
        # blank-`*` paragraph splits, or wrap past 6 lines). A one-line
        # `/** @brief ... */` is the baseline, but a brief that absorbs a
        # load-bearing in-body why (the `cxx-inbody-comment` policy) may run to
        # 4-6 lines; past 6 it belongs in the commit message. Ships advisory.
        "doc-verbose-brief",
        # Raw stdio in Qt code -- `std::cout`, `<iostream>`, `printf` should
        # route through `qDebug()` / `qWarning()` so the message handler and
        # the Console widget see the output. Two known exceptions
        # (the message handler itself, Windows console attachment) wrap their
        # call in `// code-verify off` to declare intent.
        "qt-prefer-qdebug",
        # `std::endl` flushes the stream every line -- `'\n'` is faster and
        # more idiomatic for Qt streams. `Qt::endl` is the explicit-flush form.
        "qt-prefer-newline",
        # CPU-microarchitecture / perf rules. These reason about the compiled
        # assembly (idiv/udiv cost, lock-prefix RMW, false sharing, indirect
        # branches, vtable loads) and how the cost lands on Intel x86-64 and
        # ARM AArch64 hardware. Every one ships as advisory -- the existing
        # codebase has baseline debt and the report is the cleanup checklist.
        "perf-divide-runtime-divisor",
        "perf-modulo-runtime-divisor",
        "perf-divide-by-float-literal",
        "perf-pow-call",
        "perf-dynamic-cast",
        "perf-malloc-family",
        "perf-regex-construct",
        "perf-arg-chain",
        "perf-lock-acquire",
        "perf-string-alloc-hotpath",
        "perf-log-on-hotpath",
        "perf-throw-on-hotpath",
        "perf-large-by-value-param",
        "perf-shared-ptr-by-value",
        "perf-large-stack-buffer",
        "perf-recursive-hotpath",
        "perf-false-sharing-risk",
        "perf-virtual-hotpath",
        # New `X::instance()` call site outside the composition root. Spec 0001
        # captures each dependency as a member instead of reaching through the
        # Meyers singleton at every use. Advisory: ~1,850 existing sites make
        # the report a migration ratchet, not a gate; promotion to blocking is
        # per-class at the ratchet stage.
        "arch-singleton-instance",
        # Singleton reach left inside a class that already takes a
        # SessionContext& (spec 0039). Advisory for the same reason as the rule
        # above: it is the regression guard on the converted pilots, and the
        # census -- not CI -- is what makes the surface ratchet down.
        "arch-session-context-bypass",
        # Spacing deliberately has no rule: the repo has no spacing token
        # source, and negative/odd values are legitimate layout tools
        # (overlap offsets, optical alignment). qml-hardcoded-color and
        # qt-qassert-direct graduated to errors when the 2026-07 sweep
        # cleared their last report entries.
    }
)


_REPORT_HEADER = """\
# Code Quality Report

Generated by `scripts/code-verify.py`. Each entry below was flagged by a
heuristic that often signals AI-generated narration or a CLAUDE.md style
violation. The script will not auto-fix these — judgement calls belong
to a human or an LLM that has read the surrounding context.

## Why these rules exist (read this first)

Trust between a human reviewer and an LLM contributor depends on
predictability. The reviewer must be able to tell, at a glance, that
(a) the agent stayed inside the scope of the ask, (b) every comment
encodes a *why* that wouldn't be obvious from the code, and (c) no
narration was slipped in to pad the diff.

These rules do NOT ban comments. They ban *opaque* and *narrating*
comments, the ones that re-state what the next line already says or
hedge with tutorial voice. The cleanup decision is per-line, not
per-rule:

- A `// Drop unset rows so the median weighs only present samples`
  encodes a *why* the reader needs and the code can't show. **Keep.**
- A `// Loop over the rows` re-states what `for (auto& r : rows)`
  already says. **Delete.**
- A `// We loop over the rows so that we can compute the median` is
  tutorial voice with no extra information over either of the above.
  **Delete.**

When in doubt, ask: "if I removed this comment, would the next reader
miss something they couldn't recover from the code, the commit message,
or CLAUDE.md?" If no — delete. If yes — keep it, and shorten until only
the *why* survives.

## Rules to apply

- **One-line `//` section headers only.** Multi-line `//` blocks should
  collapse to one line or be deleted. Comments label sections; they do
  not narrate. (CLAUDE.md → "Comments & Doxygen".)
- **QML files use a 3-line sandwich.** Above a top-level `Item { }` /
  `Rectangle { }` / `function ... { }` / `property ...` declaration the
  convention is decorator + label + decorator (a blank `//` line, then
  `// Label`, then another blank `//` line). DO NOT collapse the
  sandwich to a bare one-liner. When a flagged 5+ line QML block is
  wrapped between `//` decorators, keep the decorators and shorten only
  the inner prose to a single line. C++ files use the opposite
  convention: a bare `// Label` above the block, no decorators
  (decorators in C++ are `//---`/`//===` and separate concern groups).
- **Header (.h) files: delete, don't shorten.** Member-variable, signal,
  function-declaration `//` blocks are forbidden in headers entirely.
  Drop the whole block. The only block-doc allowed in a header is
  `/** @brief ... */` directly above a TYPE-LEVEL definition (class,
  struct, enum, typedef, using-alias).
- **Hoist comments out of `{` bodies.** A `// ...` as the first line
  inside a `{` block should move above the block. Often the block itself
  can drop its braces.
- **No tutorial voice.** "We", "let's", "now we", "first we", "this is
  a function that..." — none of these belong in source comments.
- **No throat-clearing.** "Note that...", "Important:", "Keep in mind",
  "FYI" — drop the prefix or drop the comment.
- **No rot-references.** "this PR", "the recent fix", "as mentioned
  above", "see below" — these decay the moment they are committed; if
  they are load-bearing, the reason goes in the commit message.
- **No restating the code.** "Loops over X", "checks if Y", "sets X
  to Y" — the code already says that.
- **No emoji.** Banned outright by CLAUDE.md.
- **No caller-references.** "Used by X", "Called from Y", "Added for
  the Z flow" — those rot. If something is non-obvious, encode it in
  the *invariant*, not in trivia about who calls it.
- **No hedging.** "For now", "for clarity", "ideally", "perhaps",
  "maybe should..." — be definite or delete.
- **No empty TODOs.** "TODO: implement this" / "FIXME: handle edge
  case" without a ticket reference is noise — write the issue or the
  ticket, not the placeholder.
- **ASCII only.** Non-ASCII characters (em dashes, smart quotes, arrows
  like the right arrow, non-breaking spaces, micro signs, super/subscripts)
  break older toolchains and read as escape goo in legacy editors. Type
  `->` not the arrow, `<=` not the less-or-equal glyph, `1/2` not the
  fraction glyph, `'` and `"` not smart quotes. Words always work:
  `degrees`, `micro`, `approx`, `infinity`. An em dash (U+2014) is the
  exception: don't trade it for `--`, rewrite the sentence with a comma,
  colon, period, or parentheses (see `comment-dash-substitute`). If you
  can't type it on a US keyboard, don't put it in source. Doesn't matter
  how confidently the model emitted it — humans and LLMs both read words
  fine.
- **No QML inline comments inside object bodies.** A `// note` on its
  own line inside an `Item { }` / `Rectangle { }` / etc. is the
  AI-narration smell CLAUDE.md bans. Either use the QML sandwich
  (`//\n// Label\n//`) above the declaration, or delete the comment.
  This rule does not apply to JS function bodies (`function f() { }`,
  `onClicked: { }`, `() => { }`) — those follow C++ comment rules.
- **No bare `Q_ASSERT`.** It compiles out under `QT_NO_DEBUG`, so the
  precondition is unchecked in the shipped binary while the code reads
  as if it were guarded. Use `SS_ASSERT(cond, <recovery>)` from
  `app/src/SSAssert.h` (debug abort kept, release reports once per site
  then runs the recovery), `SS_ASSERT_LOG(cond)` when no recovery is
  meaningful, or `SS_ASSUME(cond)` for a guard that provably already ran
  in a zero-branch hot kernel. `continue` / `break` are not valid
  recovery actions — the macro's do/while(0) would swallow them.

## Static-analysis rules (semantic)

These come from `scripts/code_verify_rules.py` and use a tree-sitter
C++ AST plus targeted line scans. CLAUDE.md is the source of truth;
the kinds below are short labels.

**Errors (block CI):**
- `qt-bare-emit` — bare `emit signalName()` outside strings/comments;
  use `Q_EMIT`.
- `qt-uppercase-signal-slot` — `Q_SIGNALS:` / `Q_SLOTS:` section labels;
  use lowercase `signals:` / `slots:`.
- `qt-invokable-void` — `Q_INVOKABLE void f();` is wrong; move to
  `public slots:`. `Q_INVOKABLE` is for non-void returns.
- `qt-disconnect-nullptr` — `disconnect(<conn>, nullptr)` (the 2-arg
  form) drops every slot; capture and disconnect the
  `QMetaObject::Connection`.
- `qt-direct-jsengine-call` — `parseFunction.call(...)` bypasses the
  runtime watchdog; route through `IScriptEngine::guardedCall()`.
- `qt-header-member-init` — `int m_foo = 0;` inside a `Q_OBJECT` /
  `Q_GADGET` class body; move to the constructor member-init list.
- `cxx-goto-or-jmp` — `goto` / `setjmp` / `longjmp` (NASA P10 rule 1).
- `cxx-member-pointer-incomplete` — a `Class::*` declarator whose class
  is only forward-declared in the translation unit (the file's quoted
  `#include` closure carries a declaration but no definition). Under the
  MSVC ABI a member pointer's representation follows the inheritance
  model, so an incomplete class hands one TU a 20-byte generalized
  pointer and another an 8-byte single-inheritance one — and unity-build
  include order decides which. That ODR mismatch shipped on 2026-08-25
  as `PropertyHooks::LiveProviderOptions` storing dangling stack
  addresses. Fix: include the header that defines the class. This is
  clang's `-fcomplete-member-pointers` re-implemented as a lint; the flag
  itself cannot be enabled because it also rejects the consistent
  incomplete bases in protobuf's `message_lite.h`. Scoped to `app/`;
  classes the closure can't resolve (Qt, system, vendored) are skipped
  rather than guessed at. **Blocking**: the app tree is at zero, and the
  failure mode is a per-TU layout disagreement that is invisible at the
  declaration under review.
- `hotpath-allocation` — `new` / `make_shared` / `.append(` /
  `.push_back(` / bare `emit` inside a known hotpath method
  (`hotpathRxFrame`, `processData`, `applyTransform`, …).
- `qml-font-pixel` — `font.pixelSize` / `font.family` outside the
  dashboard widget allow-list; use `font: Cpp_Misc_CommonFonts.uiFont`
  (or another helper).
- `qml-bus-type-int` — `busType: 0` literal int; use
  `SerialStudio.BusType.<NAME>`.
- `qt-qassert-direct` — bare `Q_ASSERT` / `Q_ASSERT_X` outside a
  `// code-verify off` fence. It compiles out under `QT_NO_DEBUG`, so the
  shipped binary runs the guarded code unchecked. Use `SS_ASSERT(cond,
  <recovery>)` / `SS_ASSERT_LOG(cond)` from `app/src/SSAssert.h`; an
  expensive predicate (walks a container, allocates) stays a fenced
  `Q_ASSERT`. Error since the 2026-07 sweep converted the last bare site.
- `qml-hardcoded-color` — a hex (`"#2ecc71"`) or named (`"white"`) color
  literal as the value of a `color:`-family binding (`color:`,
  `border.color:`, `*Color:`). The theme layer already exposes 113 tokens via
  `Cpp_ThemeManager.colors["<key>"]`, including `error`, `alarm`, `alarm_ok`,
  `alarm_warning`, `alarm_critical`, `accent`, and `highlight`; a literal
  bypasses them and inverts under fluent-dark (`border.color: "white"` reads
  correctly in the light theme and wrong in the dark one). Never flagged:
  `"transparent"` (the absence of a color, and the null branch of ~96 theme
  ternaries), any right-hand side mentioning `Cpp_ThemeManager` (so
  `Qt.darker(Cpp_ThemeManager.colors["text"], 1.5)` and theme ternaries pass),
  literal-channel `Qt.rgba(0, 0, 0, 0.15)` shadow overlays (theme-blind but
  visually intentional on the skeuomorphic gauges), a `color:` key inside an
  inline JS object literal (comma-separated data rows -- filter descriptors,
  categorical palettes; a QML one-liner separates with `;` and still matches),
  and `shadowColor:` bound to pure black/white (MultiEffect cast-shadow ink
  whose opacity lives in `shadowOpacity`; the light theme's `shadow` token at
  0.15 alpha would erase the shadow). Intentional content colors that survive
  those filters (an artificial horizon's sky/ground, crosshairs over arbitrary
  image pixels) are fenced per region with `// code-verify off`. Error since the
  2026-07 sweep cleared the backlog.

**Advisories (don't block CI):**
- `cxx-function-too-long` — function body > 100 lines (NASA P10 rule 4).
- `cxx-tu-too-long` — a first-party `.cpp`/`.h`/`.qml` over 1500 lines.
  The function cap says nothing about the file holding the functions, so
  every feature appends a method and nothing ever forces extraction:
  `FrameBuilder.cpp` reached 4574 lines on the hotpath, and
  `ProjectModelCrud.cpp` — itself a spec-0002 split product — regrew to
  2481. Per-file it is advisory, because the 34 TUs already over the line
  cannot be split in one pass. The gate that blocks accretion is the
  aggregate ratchet: `--tu-census --check` sums each file's excess over
  the threshold and fails when that pool grows, so a split into pieces
  that are individually smaller always passes even when it raises the
  file count. Split by extracting a concern into an owned sub-object.
- `cxx-nesting-too-deep` — control-flow nesting > 3 levels (CLAUDE.md).
- `cxx-anonymous-namespace` — helpers/types/variables defined inside
  `namespace { ... }`. See "Anonymous-namespace helpers" below for why
  this is a problem and what the alternatives are.
- `cxx-while-loop` — `while (cond)` opener. NASA P10 rule 2 requires
  every loop to declare a fixed upper bound; `while` makes the bound
  implicit. Real bug shipped May 2026 in `WindowManager::tileGrid`:
  `while (cols * rows < n)` wrapped on signed-int overflow when the
  canvas was 0-sized mid-resize, the product turned permanently
  negative, and the GUI thread froze — no probe timer fired, no
  debugger available, hard kill required. Rewrite as
  `for (int i = 0; i < kMaxIterations && cond; ++i)` so the bound is
  named and reviewable. Suppress with `// code-verify off` when the
  bound is provably finite by construction (`while (queue.try_dequeue(x))`
  drains a finite SPSC queue; `while (it.hasNext())` walks a fixed
  collection). Do-while (`} while (cond);`) is exempt — the trailing
  `while` is the loop closer, not a fresh opener.
- `mem-unreleased-owning-member` — `m_x = new T(...)` where the `new`
  gets no Qt parent argument and no `delete m_x` / `m_x->deleteLater()` /
  `m_x->setParent(...)` / `WA_DeleteOnClose` exists anywhere under
  `app/src`. The release index is repo-wide on purpose: the god-file
  splits put a member's `new` and its `delete` in different TUs. Fix by
  passing a parent, holding the member in a smart pointer, or releasing
  it in the destructor.
- `mem-leaked-local-new` — a local `T* p = new T(...)` with no parent
  argument that never escapes its function: not passed to a call, not
  returned, not deleted, not captured, not handed to `setParent` /
  `reset` / `release`. `static` locals are exempt (the deliberate
  immortal-allocation idiom, e.g. `Redactor`'s pattern table). Fix with
  `std::unique_ptr`.
- `mem-memfn-nontrivial` — `memcpy` / `memmove` / `memset` / `memcmp`
  whose first or second argument is the object itself (`x` or `&x`, not
  `x.data()`) and whose declared type is not trivially copyable
  (`QString`, `QByteArray`, `QList`, `std::vector`, `std::shared_ptr`,
  …). A byte-wise copy of an implicitly-shared Qt container duplicates
  the d-pointer without the atomic refcount bump — two objects then free
  one buffer. Types resolve innermost-scope-first (function locals and
  parameters over class members), so the buffer idioms in `CircularBuffer`
  and `StreamBlockCodec` don't fire.
- `qt-missing-nodiscard` — non-void const member function in a header
  without `[[nodiscard]]`.
- `doc-missing-brief-cpp` — `.cpp` function definition without a
  preceding `/** @brief ... */`.
- `doc-missing-brief-h` — header type-level definition (class /
  struct / enum / typedef / using-alias) without `/** @brief ... */`.
- `keys-hardcoded-literal` — raw `"busType"` / `"frameStart"` / etc.
  literal in a writer or reader; use `Keys::*` from `Frame.h`.
- `comment-narration` — banned tone or phrasing in a comment line,
  per CLAUDE.md "Comments & Doxygen" (tutorial voice "we"/"let's",
  "This is a function/class that...", throat-clearing "Note that"/"FYI",
  caller references "Used by"/"Called from", rot references "this PR"/
  "see below", restating "Iterates over"/"Loops over", hedging "for now"/
  "ideally", filler "simply"/"basically"). Vendored / upstream-prose
  files (`ThirdParty/`, `SimpleCrypt`, `lemonsqueezy/`) are exempt.
  `@brief` lines are also exempt to keep false-positives low.
- `comment-dash-substitute` — a spaced double-hyphen ` -- ` used as a
  sentence dash in a `//` comment. ASCII-only source bans the em dash
  (U+2014), and the reflex is to swap in `--`; that is a mechanical glyph
  trade, not the rewrite the rule asks for. Recast the sentence with a
  comma, colon, period, or parentheses. The point is human, considered
  prose, not one dash glyph for another. (`i--`, `--i`, and `//---`
  banners don't match: the rule requires a space on both sides.)
- `cxx-inbody-comment` — a `//` or `/* */` comment inside a function body
  (tree-sitter-located; the `/** @brief */` above the function is outside
  the body node and never flagged). Every function in this codebase is
  capped at 100 lines (`cxx-function-too-long`), so the contract is one
  `/** @brief ... */` above the function plus code that reads on its own.
  The cleanup decision is per-line, same as the other comment rules: a
  comment that restates the next line or narrates ("increment the counter",
  "now loop over rows") is **deleted**; a comment that encodes a load-bearing
  *why* the code can't show is **folded up into the `@brief`** (lengthen the
  brief if needed — `doc-verbose-brief` is advisory and the brief is the
  right home for the why). A genuinely-needed in-body note (a literal table,
  a derivation) stays behind a reviewed `// code-verify off` / `on` fence.
  Tooling pragmas (`// clang-format`, `// NOLINT`, `// cppcheck-suppress`,
  `// fallthrough`) are directives, not prose, and are never flagged.
- `cxx-trivial-function` — a function whose entire body is dead weight:
  empty (`void onX() {}`), only `Q_UNUSED(...)` no-ops, or a single constant
  return (`bool foo() { return true; }`, `QVariantList groupModel() { return
  {}; }`). These are dead-code removal candidates. The fix is **not** to keep
  the function — it is to confirm it is genuinely unused and then **delete it
  along with its declaration and every call site**. Before deleting, verify:
  (a) it is not bound in QML (grep the `.qml` tree for the name and any
  `Q_PROPERTY` that READs it), (b) it is not `Q_INVOKABLE` / metacall-reached,
  (c) no C++ call site relies on it. Several legitimate-trivial patterns are
  already excluded so the report stays high-precision: interface overrides
  (`virtual` / `override` / `final`), `Q_PROPERTY` READ/WRITE/RESET/MEMBER
  getters (the constant IS the property value — a capability flag or fixed
  default exposed to QML), `default*()` providers (returning a fixed default
  is their job), functions with more than one definition in the file (`#ifdef`
  build variants whose other branch does the real work), and every function in
  a platform-variant file (`*_macOS.mm`, `*_CSD.cpp`, `*_windows.cpp`, …, where
  an empty body is the expected no-op stub). Constructors / destructors /
  conversion operators with an empty body are never flagged. Because removal
  ripples across headers, property blocks, QML, and call sites, this ships
  **report-only** — never auto-deleted.
- `cxx-scattered-constant` — a file-scope `static` / `constexpr` / `const`
  declaration (including anonymous-namespace ones) positioned **after** the
  first function definition in the file. Constants and file-locals belong in a
  single `// Constants` banner at the top of the file, not sprinkled between
  functions across thousands of lines where the next reader can't find them or
  notices a duplicate too late. The fix is to **hoist** the declaration into
  the top section (preserving any `#ifdef` guard around it) and delete the
  scattered copy. Declarations before the first function (the top section
  itself), class members, function-local `static`s, and `extern` declarations
  are not flagged. Advisory: the hoist is a manual, order-sensitive move.
- `doc-header-function-block` — function doxygen block above a non-inline
  member-function declaration in a header. Per CLAUDE.md "Headers (.h) —
  strict rule": only `/** @brief */` above type-level definitions belongs
  in a header. Delete the block.
- `doc-trailing-member` — trailing-style `/**< description */` doxygen on
  a header member variable. Same rule, separate signal because the fix is
  to drop the trailing block rather than the leading one.
- `doc-verbose-brief` — doxygen block above a function or type-level
  definition that carries `@param` / `@return` / `@returns` / `@retval` /
  `@throws` / `@throw` / `@exception` / `@see` / `@sa` / `@note` /
  `@warning` / `@todo` / `@since` / `@deprecated` / `@pre` / `@post` /
  `@invariant` / `@tparam` / `@details`, OR a blank ` *` continuation line
  (extended-description paragraph break), OR wraps past 6 lines. A one-line
  `/** @brief ... */` is the baseline; a brief that folds in a load-bearing
  in-body why (per `cxx-inbody-comment`) may legitimately run 4-6 lines, so
  only 7+ lines trips this. Past that, the prose belongs in the commit message.
- `qt-prefer-qdebug` — `std::cout`, `std::cerr`, `<iostream>`, or `printf`
  in Qt code. Routes through the Qt message handler / Console widget when
  using `qDebug` / `qWarning` instead. Two known exceptions (the message
  handler implementation itself, Windows console attachment before Qt is
  up) wrap the call in `// code-verify off`.
- `qt-prefer-newline` — `std::endl` flushes the underlying buffer on every
  emission. Use `'\n'` (or `Qt::endl` when explicit flushing is the point).
- `arch-singleton-instance` — a new `X::instance()` call site under `app/src`.
  Every one reaches through a Meyers singleton whose construction order is
  unpinned; spec 0001 captures each dependency as a member instead (ctor init
  list for leaves, `setupExternalConnections()` for core modules) or wires it
  in the `ModuleManager` composition root. Sanctioned and never flagged: the
  root files (`main.cpp`, `Misc/ModuleManager.cpp`), the accessor's own
  `X& X::instance()` body, `setupExternalConnections()` wiring bodies, the
  interim `static auto& x = X::instance();` hotpath cache, constructor
  member-initializer captures (`m_x(X::instance())` — the prescribed fix;
  which classes may ctor-capture what is governed by the spec's capture-safety
  table, not the linter), single-line `Q_ASSERT(...)` expressions
  (debug-only, no release-build construction edge), and Qt's own static
  accessors (`QCoreApplication::instance()` and friends -- not Serial Studio
  singletons, nothing to capture). Advisory: the report is
  the migration ratchet; the 2026-07 sweep converted the ~1,540-site backlog.
- `arch-session-context-bypass` — an `::instance()` reach inside a file that
  takes a `SessionContext&`, i.e. a class already converted to constructor
  injection (spec 0039). Unlike the rule above, the `static auto& x =
  X::instance();` cache and the ctor init-list capture are flagged too: they
  are the two forms a converted class relapses into, and the whole point of
  the conversion is that the class asks its context. The only sanctioned site
  is the class's own `instance()` accessor, where `SessionContext::current()`
  is passed to the constructor. Fix: `m_ctx.projectModel()` (or the matching
  accessor) instead of the singleton. Advisory; the census mode
  (`--singleton-census`) is the gate that blocks growth.
- `arch-context-ctor-nonempty` — a statement in the body of
  `SessionContext::SessionContext` or `SessionContext::~SessionContext`
  (spec 0039 M2). `SessionContext::current()` is a function-local static, so a
  context that constructed its modules would re-enter that Meyers guard from
  every module constructor that reaches a singleton (`FrameBuilder` →
  `LemonSqueezy`, `FrameParser` → `FrameBuilder`, `AppState` → `ProjectModel`)
  and abort at startup with `__cxa_guard_acquire detected recursive
  initialization` — the crash that shipped from `ProjectModel`'s ctor closure on
  2026-07-07. Ownership is installed by `adopt*()` from the composition root,
  after `current()` has fully returned, and released by `shutdown()`. **Blocking**:
  the failure is a startup abort on every machine, not a style nit.
- `arch-session-adopt-site` — a `SessionContext` `adopt*()` call outside
  `app/src/Misc/ModuleManager.cpp` (spec 0039 M2). The pinned construction order
  is only pinned while it lives in one readable sequence
  (`instantiateCoreModules()`); an adoption anywhere else either builds a session
  subsystem outside that sequence or hands a second object to a slot that INV-5
  says is assigned exactly once. **Blocking**, same reasoning: both failure modes
  are startup-time and neither is visible when reviewing the call site alone.

## Performance / CPU-microarchitecture rules

These rules reason about how the compiled C++ behaves at the assembly /
register / branch-predictor / cache level. The cycle counts below are
representative numbers for current Intel x86-64 (Skylake-derived) and
ARM AArch64 (Cortex-A7x/A78); the exact figures vary with the target.
All `perf-*` rules ship as advisory — they populate `.code-report` for
a follow-up cleanup pass, they do not fail `--check`.

**File-wide (any function body):**

- `perf-divide-runtime-divisor` — `/` with a non-literal divisor. Division
  is the slowest ALU op: `divsd` is 11-22 cyc on Skylake (not pipelined,
  blocks the whole divide unit), `fdiv` is 10-40 cyc on Cortex-A78,
  `idiv` is 20-40 cyc on x86. Cache the reciprocal once (`r = 1.0/d`)
  and multiply in the loop, or use a bit-shift when the divisor is a
  power-of-two integer.
- `perf-modulo-runtime-divisor` — `%` with a non-literal divisor. Same
  `idiv`/`udiv` cost as integer divide; for power-of-two N substitute
  `& (N - 1)` (single-cycle `and`). For runtime divisors hoist out of
  the loop or use a libdivide-style precomputed magic-number multiply.
- `perf-divide-by-float-literal` — `/` with a floating-point literal like
  `/ 2.5` or `/ 1024.0`. The compiler **does not** fold these into a
  reciprocal multiply without `-ffast-math` (the result would lose 1 ULP
  for non-exact reciprocals). Precompute `constexpr double kInvX = 1.0/X;`
  and multiply (~3 cyc `mulsd` vs ~12-22 cyc `divsd`).
- `perf-pow-call` — `pow(x, y)` / `std::pow(x, y)`. Goes through libm as
  `exp(log(x) * y)` (40+ cyc on Intel/ARM) and clobbers caller-saved
  FPU/SIMD state. For small integer exponents write the multiply
  (`x*x`, `x*x*x`). For `pow(x, 0.5)` use `std::sqrt`. For `pow(2.0, n)`
  use `std::ldexp(1.0, n)` (single mantissa-shift instruction).
- `perf-dynamic-cast` — `dynamic_cast<T>(...)`. Walks the inheritance
  graph via RTTI typeinfo string compares; 50-200+ cyc worst case and
  a runtime call. Use a discriminating enum + `static_cast`, or
  pre-resolve the cast once and store the typed pointer.
- `perf-malloc-family` — `malloc` / `calloc` / `realloc` / `free` /
  `aligned_alloc` / `posix_memalign`. Same arena-mutex cost as
  `new`/`delete` (glibc `malloc` and Windows `RtlAllocateHeap` both
  serialize on a per-arena mutex), not pipelineable. Use a pre-reserved
  buffer or a small-object pool.
- `perf-regex-construct` — `QRegularExpression(...)` constructor. Compiles
  the regex to a DFA/NFA state machine and heap-allocates capture
  tables; if invoked in a loop the regex gets recompiled every
  iteration. Build once as a file-scope `static const` or class member,
  reuse `.match(...)` per call.
- `perf-arg-chain` — `.arg(...).arg(...)` chains. Each `.arg()` returns
  a fresh QString (heap allocation + memcpy). Combine into one call
  (`.arg(a, b, c)`) or include `<QStringBuilder>` and use the `%`
  operator (single allocation sized exactly for the final string).
- `perf-lock-acquire` — `QMutexLocker` / `QReadLocker` / `QWriteLocker` /
  `std::lock_guard` / `std::unique_lock` / `std::scoped_lock` /
  `std::shared_lock` constructors, and bare `.lock()` / `.try_lock()` /
  `.lockForRead()` calls. Even uncontended, the `lock`-prefixed RMW on
  x86 takes ~20 cyc and serializes the store buffer; on ARM the
  `ldaxr/stxr` + DMB barrier pair costs similar. Contended bouncing
  thrashes the L1 cache line across cores (50-200x slowdown vs
  uncontended). Prefer thread-local / SPSC / per-core state, or a
  relaxed `std::atomic` when the invariant fits a single word.

**Hotpath-only (only flagged inside `hotpathRxFrame` / `processData` / …):**

- `perf-string-alloc-hotpath` — `QString("...")`, `QByteArray("...")`,
  `.toUtf8()`, `.toStdString()`, `.toLatin1()`, `.toLocal8Bit()`,
  `QString::fromUtf8`, `QString::fromLatin1`. Heap allocation + copy on
  every call, contended on the per-arena malloc mutex. Cache at init
  or use a fixed-size stack buffer for transient formatting.
- `perf-log-on-hotpath` — `qDebug` / `qInfo` / `qWarning` / `qCritical` /
  `qFatal` calls. Builds a `QDebug` stream, takes the global message
  handler mutex, formats and writes. `<<` is eager: even filtered-out
  categories pay the format cost. Gate behind a compile-time flag or
  move to a sampled counter.
- `perf-throw-on-hotpath` — `throw expr` statement. Stack unwinding via
  DWARF (Itanium ABI on Linux/macOS) or SEH (Windows) costs 1000s of
  cycles per frame, mispredicts every catch on the way out, and trashes
  the return-address stack predictor. Return an error code or an
  `std::expected`-style variant instead.

**Function-signature (any function):**

- `perf-large-by-value-param` — heavy types (`QString`, `QByteArray`,
  `QStringList`, `QList`, `QVector`, `QMap`, `QHash`, `QJsonObject`,
  `std::string`, `std::vector`, `std::map`, …) passed by value. Even
  Qt's COW containers pay an atomic refcount bump (`lock`-prefix on
  x86, `ldxr/stxr` on ARM without v8.1 LSE atomics) on every call.
  std:: containers do a full deep copy. Pass `const T&`.
- `perf-shared-ptr-by-value` — `std::shared_ptr<T>` / `QSharedPointer<T>` /
  `QSharedDataPointer<T>` parameters passed by value. Two atomic
  refcount ops per call (`lock add` on entry, `lock sub` on exit on
  x86, ~20 cyc each; `ldxr/stxr` loop on ARM without LSE). Pass
  `const std::shared_ptr<T>&` and copy only when you genuinely store
  the pointer.

**Function-body (any function):**

- `perf-large-stack-buffer` — local fixed-size array > 1024 elements.
  Stack-frame setup cost, pollutes L1 (32-48 KB) when called in a hot
  loop with other state already on the stack, and on deep call paths
  risks overflow (Windows default 1 MB, Linux default 8 MB). Promote
  to a class member, `thread_local`, or a pre-reserved buffer.

**Class-body (any class/struct):**

- `perf-false-sharing-risk` — two or more `std::atomic<>` / `QAtomicInt` /
  `QAtomicPointer<>` / `QAtomicInteger<>` members within four source
  lines of each other, neither carrying `alignas`. They will share a
  cache line (64 B on Intel/AArch64; up to 128 B on Apple Silicon
  M-series via the speculative line) and writes from different threads
  will thrash MESI/MOESI invalidations across cores — a 50-200x
  slowdown vs the uncontended case. Add `alignas(64)` / `alignas(`
  `std::hardware_destructive_interference_size)` on each, or insert a
  `char _pad[64 - sizeof(prev)];` buffer between them.

**Header-only line scan:**

- `perf-virtual-hotpath` — a hotpath method (`hotpathRxFrame`,
  `processData`, `applyTransform`, `onFrameReady`, …) declared
  `virtual`. Every call site emits a vtable load + indirect branch
  (5-10 cyc best case, 15-20 cyc misprediction penalty on polymorphic
  sites). The compiler can't inline through the indirect call. If
  there's only one implementation drop `virtual`, or mark `final` so
  the compiler can devirtualize when the dynamic type is statically
  known.

**Hotpath-body (only flagged inside hotpath methods):**

- `perf-recursive-hotpath` — a hotpath function calls itself by name.
  Recursion on a kHz frame loop blows the i-cache (200+ cyc per L2
  miss), mispredicts the return-address stack on every return, and
  prevents inlining. Rewrite iteratively (explicit work-list or
  `std::stack`-driven).

## Anonymous-namespace helpers (`cxx-anonymous-namespace`)

An anonymous namespace (`namespace { ... }` in a `.cpp`) gives every
symbol it contains internal linkage. That part is fine. The problem is
everything else it does on top:

- **Hard to trace.** `grep -rn "myHelper"` finds the call sites but not
  the definition unless you already know which `.cpp` to open. There is
  no qualified name, no `Foo::myHelper`, nothing the IDE's "Go to
  Definition" can latch onto without a working compile_commands.json.
- **Invisible in stack traces.** Mangled names from anonymous namespaces
  look like `(anonymous namespace)::myHelper(int)` — readable, but the
  symbol carries no clue which translation unit it lives in. Two helpers
  named `parse()` in two different `.cpp` files both surface the same
  way.
- **Hostile to doxygen, code search, and call-hierarchy tooling.**
  Doxygen filters anonymous-namespace symbols out of most graphs by
  default. IDE call-hierarchy views often skip them. Code-search
  indexers like Sourcegraph and OpenGrok give them weak cross-references.
- **Encourages drift.** "It's only used in this file" is true today;
  six months later somebody copies the helper into a second `.cpp`
  because they couldn't find the original, and now there are two
  divergent definitions of the same helper.
- **Confuses LLMs (and humans).** When you ask an assistant to "find
  the helper that does X", it has to read every `.cpp` to find the
  unnamed-namespace block. Named statics or detail namespaces are
  searchable by name.

**Alternatives, ordered by preference:**

The default in this codebase is **class-private static**. The reader's
mental model of a class comes from glancing at its header — every
helper that exists should show up there, even if its body lives in the
`.cpp`. A helper hidden in an anonymous namespace is a helper the
class's API does not advertise, which means the next person reading
the header gets an incomplete picture of how the class actually works.

1. **Class-private static method (default).** Almost every "helper in
   an anonymous namespace" should be a `private: static` on the class
   it serves. The header shows the full set of moving parts, the IDE
   outline lists the helper next to the public API, and the symbol
   carries a qualified name (`Dashboard::tickRepeatTimer`) that grep,
   stack traces, and doxygen all surface cleanly.

   ```cpp
   class Dashboard {
   private:
     static int tickRepeatTimer(int index,
                                QMap<int, QTimer*>& timers,
                                QMap<int, int>& counters);
   };
   ```

2. **`static` at file scope.** Use this when the helper genuinely has
   no class to belong to — e.g. a small pure function used by free
   functions in the same `.cpp`. Same linkage guarantee as the
   anonymous namespace, but the symbol has a real name and lives at
   the top of the file where readers expect it. If you are reaching
   for this and the helper takes a class instance as its first
   argument, stop and prefer option 1 instead.

   ```cpp
   // Good: file-scope static, easy to grep, clear ownership.
   static int clampToRange(int value, int lo, int hi);
   ```

3. **Named `detail` namespace.** Reserve this for translation-unit-
   private *types* (a small POD payload, a tag struct, an `enum class`)
   that are an implementation detail of free functions in the same
   `.cpp`. Anonymous namespaces are sometimes used to give a type
   internal linkage without macro tricks; a named `detail` namespace
   has the same effect as long as the type is only declared in the
   `.cpp`. If the type is owned by a class, prefer a private nested
   type in the header instead.

   ```cpp
   // Good: type has a real qualified name (`detail::TickState`) and
   // every grep, doxygen pass, and stack trace can find it.
   namespace detail {
   struct TickState {
     int counter = 0;
     QTimer* timer = nullptr;
   };
   }  // namespace detail
   ```

**When an anonymous namespace IS the right answer:** essentially never
in this codebase. The classic justification ("template specializations
need internal linkage") almost never applies; if you think it does,
leave a one-line comment above the namespace explaining why and wrap
the block in `// code-verify off` / `// code-verify on` so the rule
stays quiet without being deleted.

**Fix recipe for an LLM cleanup pass:**

1. Open the flagged `.cpp` at the listed line.
2. For each entity inside the anonymous namespace, decide:
   - Helper that operates on a class (touches its members, takes one
     of its instances, or only that class calls it) → **class-private
     static** in the matching header. This is the default. Goal: the
     header lists every helper a reader needs to understand the class.
   - Free helper with no class affinity → `static` at file scope.
   - Translation-unit-private type → `namespace detail { ... }` in
     the `.cpp`, OR a private nested type in the header when the type
     is owned by a class.
3. Move the entity OUT of the anonymous-namespace block, applying the
   chosen treatment. When promoting to a class-private static, declare
   it in the header alongside the other private statics and keep the
   definition in the `.cpp` (no need to inline).
4. Delete the now-empty `namespace { }` block.
5. The fix is mechanical and does not change behavior; verify with a
   clean build.

## Opt-out

Wrap a region with `// code-verify off` / `// code-verify on` (or the
`/* ... */` equivalent) to disable every rule between the fences. Use
sparingly and explain why in a one-line comment above the fence.
`code-format off/on` is accepted as a legacy synonym.

## Findings
"""


def _write_lf(path: Path, text: str) -> None:
    """Write text to path as UTF-8 with LF line endings on every platform.
    Strips any stray \\r before encoding and writes via write_bytes so
    Python's text-mode translation on Windows can't sneak CRLF in."""
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    path.write_bytes(normalized.encode("utf-8"))


def _write_report(report_path: Path, flag_only: list[Violation]) -> None:
    """Write `.code-report` at the repo root grouping flag-only violations
    by severity, then by kind, then by file. Errors are listed first
    (these block CI; an LLM cleanup pass should fix them). Advisories
    follow (CLAUDE.md guidance with existing-codebase debt -- worth
    addressing but they don't fail the build). Skips writing when there's
    nothing to report and removes any stale report from a previous run."""
    if not flag_only:
        if report_path.exists():
            report_path.unlink()
        return

    errors_by_kind: dict[str, list[Violation]] = {}
    advisory_by_kind: dict[str, list[Violation]] = {}
    for v in flag_only:
        bucket = advisory_by_kind if v.kind in _ADVISORY_KINDS else errors_by_kind
        bucket.setdefault(v.kind, []).append(v)

    out: list[str] = [_REPORT_HEADER]

    if errors_by_kind:
        total = sum(len(vs) for vs in errors_by_kind.values())
        out.append(
            f"\n## Errors ({total}) -- block CI\n\n"
            "These violations fail `code-verify.py --check`. An LLM cleanup\n"
            "pass should fix them before the next CI run -- the rules below\n"
            "encode CLAUDE.md's hard requirements (Qt conventions, hotpath\n"
            "safety, fence-protected non-ASCII, line endings, …).\n"
        )
        for kind in sorted(errors_by_kind):
            entries = errors_by_kind[kind]
            out.append(f"\n### `{kind}` ({len(entries)})\n")
            for v in entries:
                out.append(f"- `{v.path}:{v.line}` — {v.message}\n")

    if advisory_by_kind:
        total = sum(len(vs) for vs in advisory_by_kind.values())
        out.append(
            f"\n## Advisories ({total}) -- CI passes, fix when convenient\n\n"
            "CLAUDE.md guidance with broad existing-code debt. New code\n"
            "should aim to clear these. They populate the report so an\n"
            "incremental cleanup pass has a checklist; CI does not block.\n"
        )
        for kind in sorted(advisory_by_kind):
            entries = advisory_by_kind[kind]
            out.append(f"\n### `{kind}` ({len(entries)})\n")
            for v in entries:
                out.append(f"- `{v.path}:{v.line}` — {v.message}\n")

    out.append(f"\n---\n\n_Total flagged: {len(flag_only)}_\n")
    _write_lf(report_path, "".join(out))


# ---------------------------------------------------------------------------
# Script SDK consistency (whole-repo, runs once per invocation)
# ---------------------------------------------------------------------------
#
# The JS/Lua scripting SDK is GENERATED by scripts/generate-sdk.py from
# api-schema.json + prelude.js. The OBD-II example shipped broken (June 2026)
# because new commands landed in source but the four generated artifacts were
# never regenerated; a separate bug base64-wrapped console.send's text payload
# because byte detection keyed on the parameter NAME. These checks guard both.
# They are repo-global (not per-file), so they run once from main().

_SDK_GENERATED = (
    "app/rcc/api/SerialStudio.js",
    "app/rcc/api/SerialStudio.lua",
    "app/rcc/api/sdk-symbols.json",
)


def _should_check_sdk(paths: list[Path], repo_root: Path) -> bool:
    """True when this run covers the SDK: a whole-tree default run, or any path
    under app/ or scripts/ where the generator and its inputs live."""
    sdk_roots = (repo_root / "app", repo_root / "scripts")
    for p in paths:
        rp = p.resolve()
        if any(rp == r or r in rp.parents or rp in r.parents for r in sdk_roots):
            return True
    return False


def _sdk_consistency_violations(repo_root: Path) -> list[Violation]:
    """Three SDK guards: generated files hand-edited, name-based byte detection
    reintroduced in the generator, and the committed SDK out of date vs the
    committed api-schema.json. The staleness check is best-effort: it cannot see
    commands added in C++ until `--dump-api-schema` refreshes api-schema.json, so
    it only catches a generator/prelude edit that was never regenerated."""
    out: list[Violation] = []

    gen = repo_root / "scripts" / "generate-sdk.py"
    gen_text = gen.read_text(encoding="utf-8") if gen.exists() else ""

    # (1) Name-based byte detection must never come back -- encoding is an
    #     explicit "binary": true schema flag (SchemaBuilder byteProp).
    if re.search(r"^\s*BYTE_PARAMS\s*=", gen_text, re.MULTILINE):
        out.append(
            Violation(
                gen,
                1,
                "sdk-byte-param-by-name",
                "generate-sdk.py reintroduced a name-based BYTE_PARAMS set; byte "
                'encoding must key on the explicit "binary" schema flag '
                "(SchemaBuilder byteProp), not the parameter name",
            )
        )

    # (2) The generated SDK artifacts are emitted, never hand-edited.
    for rel in _SDK_GENERATED:
        f = repo_root / rel
        if not f.exists():
            continue
        head = f.read_text(encoding="utf-8")[:200]
        if "AUTO-GENERATED" not in head and rel.endswith((".js", ".lua")):
            out.append(
                Violation(
                    f,
                    1,
                    "sdk-generated-edited",
                    "generated SDK file lost its AUTO-GENERATED banner; regenerate "
                    "with scripts/sanitize-commit.py instead of editing by hand",
                )
            )

    # (3) Best-effort staleness: re-emit from the committed schema and diff.
    out.extend(_sdk_staleness_violations(repo_root))
    return out


def _sdk_staleness_violations(repo_root: Path) -> list[Violation]:
    """Re-run the generator in-memory against the committed api-schema.json and
    compare to the committed SerialStudio.js/.lua/sdk-symbols.json. A mismatch
    means prelude.js or generate-sdk.py changed without a regenerate."""
    out: list[Violation] = []
    schema = repo_root / "app" / "rcc" / "api" / "api-schema.json"
    gen = repo_root / "scripts" / "generate-sdk.py"
    if not schema.exists() or not gen.exists():
        return out

    try:
        spec = importlib.util.spec_from_file_location("generate_sdk", gen)
        mod = importlib.util.module_from_spec(spec)
        sys.modules["generate_sdk"] = mod
        spec.loader.exec_module(mod)
        commands = json.loads(schema.read_text(encoding="utf-8"))
        commands = mod.drop_namespace_collisions(commands)
        expected = {
            "app/rcc/api/SerialStudio.js": mod.emit_js(commands),
            "app/rcc/api/SerialStudio.lua": mod.emit_lua(commands),
            "app/rcc/api/sdk-symbols.json": json.dumps(
                mod.collect_symbols(commands), indent=2
            )
            + "\n",
        }
    except Exception as exc:
        # Never crash a commit if the generator's internals moved; surface why
        # the staleness check was skipped so a silent miss stays debuggable.
        print(f"[code-verify] sdk-out-of-date check skipped: {exc}", file=sys.stderr)
        return out

    for rel, want in expected.items():
        f = repo_root / rel
        if not f.exists():
            continue
        if f.read_text(encoding="utf-8") != want:
            out.append(
                Violation(
                    f,
                    1,
                    "sdk-out-of-date",
                    "committed SDK differs from generate-sdk.py output for the "
                    "current api-schema.json; run scripts/sanitize-commit.py "
                    "(after SerialStudio --dump-api-schema if commands changed)",
                )
            )
    return out


# ---------------------------------------------------------------------------
# Generated API surfaces (spec 0037, whole-repo)
# ---------------------------------------------------------------------------
#
# The gRPC field-number ledger and the typed proto are generated by
# scripts/generate-property-registry.py. The ledger is RELEASED WIRE STATE: a
# number that moves does not break a build, it makes a shipped client read one
# field out of another field's bytes. Two guards: the artifacts keep their
# do-not-edit marker, and no number assigned at HEAD ever changes meaning.

_API_GENERATED = {
    "app/rcc/api/proto-fields.json": "never edit by hand",
    "doc/grpc/serialstudio-typed.proto": "Auto-generated by Serial Studio ProtoGenerator",
    "app/src/DataModel/Generated/DatasetRegistry.h": "never edit by hand",
    "app/src/DataModel/Generated/DatasetSerialization.cpp": "never edit by hand",
    "app/src/DataModel/Generated/DatasetForm.cpp": "never edit by hand",
    "app/src/API/Generated/DatasetApiFields.cpp": "never edit by hand",
}

_LEDGER_REL = "app/rcc/api/proto-fields.json"


def _api_surface_violations(repo_root: Path) -> list[Violation]:
    """Marker + append-only guards over the spec-0037 generated API artifacts."""
    out: list[Violation] = []

    for rel, marker in _API_GENERATED.items():
        f = repo_root / rel
        if not f.exists():
            continue
        # The C++ artifacts carry the dual-license header first, so the marker sits
        # ~800 characters in; the window covers both shapes.
        if marker not in f.read_text(encoding="utf-8")[:1200]:
            out.append(
                Violation(
                    f,
                    1,
                    "api-generated-edited",
                    "generated API artifact lost its do-not-edit marker; regenerate "
                    "with scripts/generate-property-registry.py instead of editing "
                    "it by hand",
                )
            )

    out.extend(_proto_renumber_violations(repo_root))
    out.extend(_registry_field_map_violations(repo_root))
    return out


# ---------------------------------------------------------------------------
# Dataset property registry (spec 0036, whole-repo)
# ---------------------------------------------------------------------------
#
# Dataset property plumbing is DERIVED from app/rcc/properties/dataset.json:
# the serializer, the editor rows, the commit dispatcher and the API appliers
# are generated. The failure mode the registry exists to end is a second,
# hand-written field map growing beside the generated one -- a reader, an
# applier or a form builder that spells out a cluster of dataset property keys
# in a file the generator does not own. Identity keys are excluded: groupId /
# datasetId / uniqueId / title address an entity in every payload and say
# nothing about dataset property plumbing.

_REGISTRY_MANIFEST = "app/rcc/properties/dataset.json"
_REGISTRY_IDENTITY_KEYS = frozenset({"GroupId", "DatasetId", "UniqueId", "Title"})
_REGISTRY_KEY_CLUSTER = 4
_REGISTRY_GENERATED_DIRS = (
    "app/src/DataModel/Generated/",
    "app/src/API/Generated/",
)

# Non-generated files that legitimately name several dataset keys, each with the
# reason it is not a parallel field map.
_REGISTRY_KEY_ALLOWED = {
    # Home of the group/action/source serializers (Keys:: constants moved to FrameKeys.h,
    # spec 0070).
    "app/src/DataModel/Frame.h": "sibling-entity serializers",
    "app/src/DataModel/FrameKeys.h": "Keys:: declarations",
    # Alarm bands and FFT markers are nested entities the manifest routes to
    # hand-written readers through declared subEntity hooks.
    "app/src/DataModel/Frame.cpp": "hand-written sub-entity readers declared as manifest hooks",
    # Builds synthetic projects for the throughput gate; not a document surface.
    "app/src/Benchmark/HotpathBenchmark.cpp": "synthetic benchmark project fixtures",
    # The manifest's named escape hatches, split across two TUs so the unit
    # tier can link the ProjectModel-free validators alone.
    "app/src/DataModel/Project/PropertyHooks.cpp": "the manifest's named escape hatches",
    "app/src/DataModel/Project/PropertyValidators.cpp": "the ProjectModel-free validators",
}


def _dataset_registry_keys(repo_root: Path) -> set[str]:
    """Return the dataset property Keys:: constants declared by the manifest."""
    manifest = repo_root / _REGISTRY_MANIFEST
    if not manifest.exists():
        return set()

    try:
        data = json.loads(manifest.read_text(encoding="utf-8"))
    except Exception as exc:
        print(
            f"[code-verify] registry-parallel-field-map check skipped: {exc}",
            file=sys.stderr,
        )
        return set()

    keys: set[str] = set()
    for prop in data.get("properties", []):
        if prop.get("jsonKey"):
            keys.add(prop["jsonKey"])
        keys.update(prop.get("legacyKeys", []))
    for sub in data.get("subEntities", []):
        if sub.get("jsonKey"):
            keys.add(sub["jsonKey"])

    return keys - _REGISTRY_IDENTITY_KEYS


def _registry_field_map_violations(repo_root: Path) -> list[Violation]:
    """Flag a hand-written file that spells out a cluster of dataset property keys."""
    keys = _dataset_registry_keys(repo_root)
    if not keys:
        return []

    pattern = re.compile(r"\bKeys::(" + "|".join(sorted(keys)) + r")\b")
    out: list[Violation] = []
    for path in sorted((repo_root / "app" / "src").rglob("*")):
        if path.suffix not in (".cpp", ".h"):
            continue

        rel = path.resolve().relative_to(repo_root).as_posix()
        if rel in _REGISTRY_KEY_ALLOWED or rel.startswith(_REGISTRY_GENERATED_DIRS):
            continue

        text = path.read_text(encoding="utf-8", errors="replace")
        found: dict[str, int] = {}
        for match in pattern.finditer(text):
            found.setdefault(match.group(1), text.count("\n", 0, match.start()) + 1)

        if len(found) < _REGISTRY_KEY_CLUSTER:
            continue

        out.append(
            Violation(
                path,
                min(found.values()),
                "registry-parallel-field-map",
                f"names {len(found)} dataset property keys ({', '.join(sorted(found))}) "
                "outside the generated surfaces; dataset property plumbing is derived "
                f"from {_REGISTRY_MANIFEST} -- declare the property there and regenerate, "
                "or add this file to _REGISTRY_KEY_ALLOWED with the reason it is not a "
                "parallel field map",
            )
        )

    return out


def _proto_renumber_violations(repo_root: Path) -> list[Violation]:
    """Compare the committed ledger against HEAD's: a number may be retired, never moved."""
    out: list[Violation] = []
    ledger = repo_root / _LEDGER_REL
    if not ledger.exists():
        return out

    try:
        current = json.loads(ledger.read_text(encoding="utf-8")).get("commands", {})
        blob = subprocess.run(
            ["git", "show", f"HEAD:{_LEDGER_REL}"],
            cwd=repo_root,
            capture_output=True,
            text=True,
        )
        if blob.returncode != 0:
            return out
        previous = json.loads(blob.stdout).get("commands", {})
    except Exception as exc:
        print(
            f"[code-verify] proto-field-renumbered check skipped: {exc}",
            file=sys.stderr,
        )
        return out

    for name, was in previous.items():
        now = current.get(name)
        if not isinstance(now, dict):
            out.append(
                Violation(
                    ledger,
                    1,
                    "proto-field-renumbered",
                    f"command '{name}' was dropped from the ledger; entries are "
                    "retained forever so a GPL dump cannot delete commercial numbers",
                )
            )
            continue

        retired = set(now.get("reserved", []))
        for param, number in was.get("fields", {}).items():
            assigned = now.get("fields", {}).get(param)
            if assigned is None and number not in retired:
                out.append(
                    Violation(
                        ledger,
                        1,
                        "proto-field-renumbered",
                        f"{name}.{param} lost number {number} without retiring it; "
                        "a removed parameter's number must move to 'reserved'",
                    )
                )
            elif assigned is not None and assigned != number:
                out.append(
                    Violation(
                        ledger,
                        1,
                        "proto-field-renumbered",
                        f"{name}.{param} moved from field number {number} to "
                        f"{assigned}; gRPC numbers are append-only released state",
                    )
                )

        if now.get("next", 0) < was.get("next", 0):
            out.append(
                Violation(
                    ledger,
                    1,
                    "proto-field-renumbered",
                    f"{name}: 'next' went backwards, which would reissue a number",
                )
            )

    return out


_CENSUS_BASELINE = Path(__file__).with_name("singleton-census.json")
_CENSUS_SUFFIXES = (".cpp", ".cc", ".cxx", ".mm", ".h", ".hpp", ".hxx")


def _collect_singleton_census(repo_root: Path) -> dict:
    """Walk app/src and aggregate the per-file singleton census."""
    tree = repo_root / "app" / "src"
    buckets = {name: 0 for name in _SEMANTIC_RULES.SINGLETON_CENSUS_BUCKETS}
    per_file: dict[str, dict] = {}
    per_class: dict[str, int] = {}
    total = 0

    for path in sorted(iter_source_files([tree])):
        if path.suffix not in _CENSUS_SUFFIXES:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue

        stats = _SEMANTIC_RULES.singleton_census(path, text)
        if stats["total"] == 0:
            continue

        rel = path.resolve().relative_to(repo_root).as_posix()
        per_file[rel] = {"total": stats["total"], "buckets": stats["buckets"]}
        total += stats["total"]
        for name, count in stats["buckets"].items():
            buckets[name] += count
        for name, count in stats["classes"].items():
            per_class[name] = per_class.get(name, 0) + count

    return {
        "spec": "0039-session-context",
        "regenerate": "python scripts/code-verify.py --singleton-census --accept",
        "total": total,
        "files": len(per_file),
        "buckets": buckets,
        "per_class": dict(sorted(per_class.items(), key=lambda kv: -kv[1])),
        "per_file": per_file,
    }


def _print_singleton_census(census: dict) -> None:
    print(f"singleton census: {census['total']} occurrences in {census['files']} files")
    for name, count in census["buckets"].items():
        print(f"  {name:<13} {count}")

    print("\ntop reached singletons:")
    for name, count in list(census["per_class"].items())[:15]:
        print(f"  {count:>5}  {name}")


def _run_singleton_census(repo_root: Path, check: bool, accept: bool) -> int:
    """Report the census, gate it against the checked-in baseline, or
    re-baseline it. The gate blocks growth of the total and of the
    static-cache bucket; a decrease is accepted and asks for a re-baseline."""
    if _SEMANTIC_RULES is None or not getattr(
        _SEMANTIC_RULES, "HAS_TREE_SITTER", False
    ):
        print(
            "singleton census needs tree-sitter (pip install tree_sitter "
            "tree_sitter_cpp); refusing to report numbers it cannot classify",
            file=sys.stderr,
        )
        return 2

    census = _collect_singleton_census(repo_root)

    if accept:
        _CENSUS_BASELINE.write_text(
            json.dumps(census, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        _print_singleton_census(census)
        print(f"\nbaseline written to {_CENSUS_BASELINE}")
        return 0

    if not check:
        _print_singleton_census(census)
        return 0

    if not _CENSUS_BASELINE.is_file():
        print(
            f"no baseline at {_CENSUS_BASELINE}; seed it with "
            "--singleton-census --accept",
            file=sys.stderr,
        )
        return 2

    base = json.loads(_CENSUS_BASELINE.read_text(encoding="utf-8"))
    grown = [
        (rel, stats["total"], base.get("per_file", {}).get(rel, {}).get("total", 0))
        for rel, stats in census["per_file"].items()
        if stats["total"] > base.get("per_file", {}).get(rel, {}).get("total", 0)
    ]

    total_delta = census["total"] - base.get("total", 0)
    cache_delta = census["buckets"]["static-cache"] - base.get("buckets", {}).get(
        "static-cache", 0
    )

    if total_delta > 0 or cache_delta > 0:
        print(
            f"singleton census grew: total {base.get('total', 0)} -> "
            f"{census['total']}, static-cache "
            f"{base.get('buckets', {}).get('static-cache', 0)} -> "
            f"{census['buckets']['static-cache']}",
            file=sys.stderr,
        )
        for rel, now, before in grown:
            print(f"  {rel}: {before} -> {now}", file=sys.stderr)

        print(
            "\nTake the dependency through SessionContext (constructor "
            "injection, spec 0039) instead of reaching for the singleton. If "
            "the growth is deliberate, re-baseline with "
            "python scripts/code-verify.py --singleton-census --accept",
            file=sys.stderr,
        )
        return 1

    print(
        f"singleton census: {census['total']} occurrences "
        f"(baseline {base.get('total', 0)}), static-cache "
        f"{census['buckets']['static-cache']} "
        f"(baseline {base.get('buckets', {}).get('static-cache', 0)})"
    )
    if total_delta < 0 or cache_delta < 0:
        print(
            "the surface shrank; re-baseline with "
            "python scripts/code-verify.py --singleton-census --accept"
        )
    return 0


_TU_CENSUS_BASELINE = Path(__file__).with_name("tu-census.json")
_TU_CENSUS_SUFFIXES = (".cpp", ".h", ".qml")
_TU_CENSUS_TREES = ("app/src", "app/qml")
_TU_CENSUS_THRESHOLD = 1500
_TU_CENSUS_TIERS = (
    (4000, "critical"),
    (2500, "god"),
    (_TU_CENSUS_THRESHOLD, "oversized"),
)


def _tu_tier(lines: int) -> str:
    """Name the size band a translation unit falls into."""
    for floor, name in _TU_CENSUS_TIERS:
        if lines >= floor:
            return name
    return "ok"


def _collect_tu_census(repo_root: Path) -> dict:
    """Measure every first-party TU over the size threshold under app/."""
    buckets = {name: 0 for _, name in _TU_CENSUS_TIERS}
    per_file: dict[str, int] = {}
    excess = 0
    worst = 0

    trees = [repo_root / tree for tree in _TU_CENSUS_TREES]
    for path in sorted(iter_source_files([t for t in trees if t.exists()])):
        if path.suffix not in _TU_CENSUS_SUFFIXES:
            continue
        if not _is_first_party(path):
            continue
        try:
            lines = len(path.read_text(encoding="utf-8", errors="replace").splitlines())
        except OSError:
            continue
        if lines <= _TU_CENSUS_THRESHOLD:
            continue

        rel = path.resolve().relative_to(repo_root).as_posix()
        per_file[rel] = lines
        buckets[_tu_tier(lines)] += 1
        excess += lines - _TU_CENSUS_THRESHOLD
        worst = max(worst, lines)

    return {
        "rule": "cxx-tu-too-long",
        "regenerate": "python scripts/code-verify.py --tu-census --accept",
        "threshold": _TU_CENSUS_THRESHOLD,
        "excess": excess,
        "files": len(per_file),
        "worst": worst,
        "buckets": buckets,
        "per_file": dict(sorted(per_file.items(), key=lambda kv: -kv[1])),
    }


def _print_tu_census(census: dict) -> None:
    print(
        f"TU census: {census['files']} files over {census['threshold']} lines, "
        f"{census['excess']} excess lines, worst {census['worst']}"
    )
    for name, count in census["buckets"].items():
        print(f"  {name:<10} {count}")

    print("\nlargest translation units:")
    for rel, lines in list(census["per_file"].items())[:15]:
        print(f"  {lines:>5}  {rel}")


def _run_tu_census(repo_root: Path, check: bool, accept: bool) -> int:
    """Report the TU census, gate it against the checked-in baseline, or
    re-baseline it.

    The gated number is the excess-over-threshold pool, not the file count: a
    split that turns one 4574-line TU into three 1525-line ones raises the file
    count but drops the excess by 3000, and must pass. `worst` is gated too so
    the largest offender cannot grow while others shrink to pay for it."""
    census = _collect_tu_census(repo_root)

    if accept:
        _TU_CENSUS_BASELINE.write_text(
            json.dumps(census, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        _print_tu_census(census)
        print(f"\nbaseline written to {_TU_CENSUS_BASELINE}")
        return 0

    if not check:
        _print_tu_census(census)
        return 0

    if not _TU_CENSUS_BASELINE.is_file():
        print(
            f"no baseline at {_TU_CENSUS_BASELINE}; seed it with "
            "--tu-census --accept",
            file=sys.stderr,
        )
        return 2

    base = json.loads(_TU_CENSUS_BASELINE.read_text(encoding="utf-8"))
    base_files = base.get("per_file", {})
    grown = [
        (rel, lines, base_files.get(rel, 0))
        for rel, lines in census["per_file"].items()
        if lines > base_files.get(rel, 0)
    ]

    excess_delta = census["excess"] - base.get("excess", 0)
    worst_delta = census["worst"] - base.get("worst", 0)

    if excess_delta > 0 or worst_delta > 0:
        print(
            f"TU census grew: excess {base.get('excess', 0)} -> "
            f"{census['excess']}, worst {base.get('worst', 0)} -> "
            f"{census['worst']}",
            file=sys.stderr,
        )
        for rel, now, before in sorted(grown, key=lambda row: -(row[1] - row[2])):
            print(f"  {rel}: {before} -> {now}", file=sys.stderr)

        print(
            "\nExtract instead of appending: move the new concern into its own "
            "TU, as a real member sub-object (one class = one .h/.cpp pair in a "
            "sibling directory named after the facade). If the growth is deliberate, "
            "re-baseline with python scripts/code-verify.py --tu-census --accept",
            file=sys.stderr,
        )
        return 1

    print(
        f"TU census: {census['excess']} excess lines "
        f"(baseline {base.get('excess', 0)}), worst {census['worst']} "
        f"(baseline {base.get('worst', 0)})"
    )
    if excess_delta < 0 or worst_delta < 0:
        print(
            "the surface shrank; re-baseline with "
            "python scripts/code-verify.py --tu-census --accept"
        )
    return 0


# ---------------------------------------------------------------------------
# Duplicate-window census (spec 0075, finding L8 / the clone families in R12)
# ---------------------------------------------------------------------------
#
# The review found ~70% clones between whole driver pairs (S7/EtherNet-IP), the Gauge/Meter
# widget pair, and a 190-line delta between two 1700-line CI jobs. None of it is visible to a
# per-file linter: every file passes on its own. This census works on file PAIRS.
#
# Method: normalize each line (drop comments, collapse whitespace), slide a 10-line window over
# the result, and hash each window. Two files share a window when the same hash appears in both.
# A pair with more than 40 shared windows is a clone family, not an accident. Normalizing
# whitespace and comments is what lets a copy that was reformatted or re-commented still match;
# identifiers are deliberately NOT normalized, so two files that merely have the same shape
# (every Qt class has a ctor and a dtor) do not register.
#
# The gate is the SUM of shared windows over all reported pairs, ratcheted against
# scripts/dup-census.json: extracting a clone lowers it, copy-pasting raises it.

_DUP_CENSUS_BASELINE = Path(__file__).with_name("dup-census.json")
_DUP_CENSUS_SUFFIXES = (".cpp", ".h", ".qml")
_DUP_CENSUS_TREES = ("app/src", "app/qml")
_DUP_WINDOW_LINES = 10
_DUP_PAIR_THRESHOLD = 40

_DUP_COMMENT_RE = re.compile(r"//.*$")
_DUP_WS_RE = re.compile(r"\s+")


def _dup_normalized_lines(text: str) -> list[str]:
    """Comment-free, whitespace-collapsed, non-blank lines: the unit a window is built from."""
    out: list[str] = []
    for raw in text.split("\n"):
        line = _DUP_WS_RE.sub(" ", _DUP_COMMENT_RE.sub("", raw)).strip()
        if line and line not in ("{", "}", "};"):
            out.append(line)
    return out


def _dup_window_hashes(text: str) -> set[str]:
    """Every distinct 10-line window in a file, as digests."""
    lines = _dup_normalized_lines(text)
    if len(lines) < _DUP_WINDOW_LINES:
        return set()
    return {
        hashlib.sha1(
            "\n".join(lines[i : i + _DUP_WINDOW_LINES]).encode("utf-8")
        ).hexdigest()[:16]
        for i in range(len(lines) - _DUP_WINDOW_LINES + 1)
    }


def _collect_dup_census(repo_root: Path) -> dict:
    """Count shared 10-line windows for every first-party file pair over the threshold."""
    windows: dict[str, set[str]] = {}
    trees = [repo_root / tree for tree in _DUP_CENSUS_TREES]
    for path in sorted(iter_source_files([t for t in trees if t.exists()])):
        if path.suffix not in _DUP_CENSUS_SUFFIXES or not _is_first_party(path):
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        hashes = _dup_window_hashes(text)
        if hashes:
            windows[path.resolve().relative_to(repo_root).as_posix()] = hashes

    # Invert to an index so the pair walk touches only files that actually share a window,
    # instead of the full O(n^2) product over ~1200 files.
    owners: dict[str, list[str]] = {}
    for rel, hashes in windows.items():
        for digest in hashes:
            owners.setdefault(digest, []).append(rel)

    counts: dict[tuple[str, str], int] = {}
    for sharers in owners.values():
        if len(sharers) < 2 or len(sharers) > 16:
            continue
        for i, left in enumerate(sharers):
            for right in sharers[i + 1 :]:
                key = (left, right) if left < right else (right, left)
                counts[key] = counts.get(key, 0) + 1

    pairs = {
        f"{left} | {right}": count
        for (left, right), count in counts.items()
        if count > _DUP_PAIR_THRESHOLD
    }
    return {
        "rule": "cxx-duplicate-window",
        "regenerate": "python scripts/code-verify.py --dup-census --accept",
        "window_lines": _DUP_WINDOW_LINES,
        "pair_threshold": _DUP_PAIR_THRESHOLD,
        "shared": sum(pairs.values()),
        "pairs": dict(sorted(pairs.items(), key=lambda row: (-row[1], row[0]))),
    }


def _print_dup_census(census: dict) -> None:
    """Print the clone families, worst first."""
    print(
        f"duplicate-window census: {census['shared']} shared windows over "
        f"{len(census['pairs'])} file pairs "
        f"({census['window_lines']} normalized lines per window, "
        f"pair threshold {census['pair_threshold']})"
    )
    for pair, count in census["pairs"].items():
        print(f"  advisory: cxx-duplicate-window: {count} shared windows: {pair}")


def _run_dup_census(repo_root: Path, check: bool, accept: bool) -> int:
    """Report the duplicate-window census, gate it, or re-baseline it."""
    census = _collect_dup_census(repo_root)

    if accept:
        _DUP_CENSUS_BASELINE.write_text(
            json.dumps(census, indent=2, ensure_ascii=False) + "\n",
            encoding="utf-8",
            newline="\n",
        )
        _print_dup_census(census)
        print(f"\nbaseline written to {_DUP_CENSUS_BASELINE}")
        return 0

    if not check:
        _print_dup_census(census)
        return 0

    if not _DUP_CENSUS_BASELINE.is_file():
        print(
            f"no baseline at {_DUP_CENSUS_BASELINE}; seed it with "
            "--dup-census --accept",
            file=sys.stderr,
        )
        return 2

    base = json.loads(_DUP_CENSUS_BASELINE.read_text(encoding="utf-8"))
    base_pairs = base.get("pairs", {})
    delta = census["shared"] - base.get("shared", 0)

    if delta > 0:
        print(
            f"duplicate-window census grew: {base.get('shared', 0)} -> "
            f"{census['shared']} shared windows",
            file=sys.stderr,
        )
        for pair, count in census["pairs"].items():
            before = base_pairs.get(pair, 0)
            if count > before:
                print(f"  {pair}: {before} -> {count}", file=sys.stderr)
        print(
            "\nExtract the shared block into one owner instead of copying it. If the "
            "growth is deliberate, re-baseline with "
            "python scripts/code-verify.py --dup-census --accept",
            file=sys.stderr,
        )
        return 1

    print(
        f"duplicate-window census: {census['shared']} shared windows "
        f"(baseline {base.get('shared', 0)}) over {len(census['pairs'])} pairs"
    )
    if delta < 0:
        print(
            "the clone surface shrank; re-baseline with "
            "python scripts/code-verify.py --dup-census --accept"
        )
    return 0


def main(argv: list[str]) -> int:
    # Windows defaults stdout/stderr to cp1252; violation messages can carry
    # non-ASCII (em-dashes, smart quotes, U+2713) lifted from user source and
    # crash with UnicodeEncodeError before any report is written.
    for stream in (sys.stdout, sys.stderr):
        reconfigure = getattr(stream, "reconfigure", None)
        if reconfigure is not None:
            reconfigure(encoding="utf-8", errors="replace")

    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        epilog="With no arguments, runs --check on the repo's default trees.",
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="report only, no writes")
    group.add_argument(
        "--fix", action="store_true", help="rewrite files in place (explicit only)"
    )
    parser.add_argument(
        "--diff", action="store_true", help="show unified diff of proposed changes"
    )
    parser.add_argument(
        "--no-report",
        action="store_true",
        help="skip writing .code-report at the repo root",
    )
    parser.add_argument(
        "--singleton-census",
        action="store_true",
        help="classify every X::instance() under app/src (spec 0039)",
    )
    parser.add_argument(
        "--tu-census",
        action="store_true",
        help="measure first-party translation units over the size threshold",
    )
    parser.add_argument(
        "--dup-census",
        action="store_true",
        help="measure duplicated 10-line windows across first-party file pairs",
    )
    parser.add_argument(
        "--accept",
        action="store_true",
        help="re-baseline the census JSON (with --singleton-census / --tu-census / "
        "--dup-census)",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        type=Path,
        help="files or directories to process (default: repo trees)",
    )

    args = parser.parse_args(argv)

    if args.singleton_census:
        root = Path(__file__).resolve().parent.parent
        return _run_singleton_census(root, check=args.check, accept=args.accept)

    if args.tu_census:
        root = Path(__file__).resolve().parent.parent
        return _run_tu_census(root, check=args.check, accept=args.accept)

    if args.dup_census:
        root = Path(__file__).resolve().parent.parent
        return _run_dup_census(root, check=args.check, accept=args.accept)

    # Default to --check when neither mode was requested. This tool rewrites
    # sources in place; a bare invocation that silently reformats hundreds of
    # files leaves `git diff` as the only guard against a rule regression, so
    # writing is opt-in. sanitize-commit.py passes --fix explicitly.
    if not args.check and not args.fix:
        args.check = True

    # Default to the configured repo trees when no paths were supplied
    if not args.paths:
        targets = [t for t in default_targets() if t.exists()]
        if not targets:
            print("no default targets exist; pass paths explicitly", file=sys.stderr)
            return 2
        args.paths = targets

    files = sorted(set(iter_source_files(args.paths)))
    if not files:
        print("no source files found", file=sys.stderr)
        return 2

    total_violations = 0
    total_changed = 0
    error_count = 0
    advisory_count = 0
    flag_only: list[Violation] = []

    for path in files:
        violations, new_text = process_file(path, fix=args.fix)
        total_violations += len(violations)

        for v in violations:
            severity = "advisory" if v.kind in _ADVISORY_KINDS else "error"
            print(f"{v.path}:{v.line}: {severity}: {v.kind}: {v.message}")
            if v.kind in _ADVISORY_KINDS:
                advisory_count += 1
            else:
                error_count += 1
            # Include in report if not auto-fixable, OR if auto-fixable but
            # we're in --check mode (not actually fixing). Auto-fixable
            # violations in --fix mode are written away, so reporting them
            # would be noise.
            if v.kind not in _AUTO_FIXABLE_KINDS or args.check:
                flag_only.append(v)

        if new_text is not None:
            if args.diff:
                before = path.read_text(encoding="utf-8").splitlines(keepends=True)
                after = new_text.splitlines(keepends=True)
                sys.stdout.writelines(
                    difflib.unified_diff(before, after, str(path), str(path))
                )
            if args.fix:
                _write_lf(path, new_text)
                total_changed += 1
                print(f"{path}: rewrote", file=sys.stderr)

    # Whole-repo SDK consistency runs once, only when scanning default trees or
    # an SDK-relevant path (skip it for a single unrelated file lint).
    repo_root = Path(__file__).resolve().parent.parent
    if _should_check_sdk(args.paths, repo_root):
        for v in _sdk_consistency_violations(repo_root) + _api_surface_violations(
            repo_root
        ):
            severity = "advisory" if v.kind in _ADVISORY_KINDS else "error"
            print(f"{v.path}:{v.line}: {severity}: {v.kind}: {v.message}")
            if v.kind in _ADVISORY_KINDS:
                advisory_count += 1
            else:
                error_count += 1
            flag_only.append(v)

    if not args.no_report:
        _write_report(repo_root / ".code-report", flag_only)

    if args.check:
        print(
            f"\n{len(files)} files scanned, {error_count} errors, "
            f"{advisory_count} advisory ({len(flag_only)} flag-only)",
            file=sys.stderr,
        )
        # CI gate: fail on errors only. Advisory findings populate
        # `.code-report` for follow-up but don't block the build.
        return 1 if error_count else 0

    print(
        f"\n{len(files)} files scanned, {total_changed} rewritten, "
        f"{error_count} errors, {advisory_count} advisory "
        f"({len(flag_only)} flag-only)",
        file=sys.stderr,
    )
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
