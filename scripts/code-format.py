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

Comment-style and AI-narration findings (multi-line `//` runs, `//` as
the first line of a `{` body, tutorial voice, hedging, rot-references,
restate-the-code openers, emoji, …) are flag-only — collapsing or
hoisting comments is a judgement call. Findings are grouped by rule
into `.code-report` at the repo root for a follow-up human or LLM pass.

Wrap a region with `// code-format off` / `// code-format on` to disable
every rule between the fences (the `/* ... */` equivalent works too).

Usage:
    python3 scripts/code-format.py                          # fix everything under app/qml + app/src
    python3 scripts/code-format.py --check                  # report-only, whole tree
    python3 scripts/code-format.py --check app/qml          # report-only, explicit path
    python3 scripts/code-format.py --fix app/src            # rewrite C++ files
    python3 scripts/code-format.py --check app/qml/Foo.qml  # single file
    python3 scripts/code-format.py --fix --diff app/qml     # show changes

Called with no arguments the script defaults to --fix on the entire
<repo>/app/qml and <repo>/app/src trees.

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


def find_brace_free_violations(lines: list[str], path: Path) -> tuple[list[Violation],
                                                                     list[int]]:
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


def check_id_placement(lines: list[LogicalLine],
                       path: Path) -> tuple[list[Violation], list[int]]:
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

    # If the first content line wasn't the id, flag it (no auto-fix — moving
    # arbitrary content above the id would risk reordering side effects).
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

# `// code-format off` and `// code-format on` fence raw lines out of every
# rule the script enforces. The fence text is matched on a stripped line —
# both `//` and `/*…*/` forms are accepted so QML and C++ can both opt out.
_FENCE_OFF_RE = re.compile(r"^\s*(?://|/\*)\s*code-format\s+off\b")
_FENCE_ON_RE  = re.compile(r"^\s*(?://|/\*)\s*code-format\s+on\b")


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


def find_comment_style_violations(
    lines: list[str], path: Path, fence_mask: list[bool]
) -> list[Violation]:
    """Flag two AI-typical patterns:

    - `multi-line-comment`: ≥2 consecutive `//` comment lines. CLAUDE.md
      caps in-body `//` comments at one line; multi-line narration is
      flagged for human collapse / removal.
    - `comment-after-brace`: a `//` line as the first non-blank content
      inside a `{` block. Hoisting the comment above the block (and often
      dropping the braces) reads better — see CLAUDE.md "Comments &
      Doxygen" guidance.

    Neither is auto-fixable: collapsing comments loses information,
    hoisting requires judgement about the surrounding block."""
    violations: list[Violation] = []
    n = len(lines)

    i = 0
    while i < n:
        if fence_mask[i] or not _is_line_comment(lines[i]):
            i += 1
            continue

        run_start = i
        while i < n and not fence_mask[i] and _is_line_comment(lines[i]):
            i += 1
        run_len = i - run_start

        if run_len >= 2:
            violations.append(
                Violation(
                    path,
                    run_start + 1,
                    "multi-line-comment",
                    f"{run_len} consecutive `//` lines — collapse to one line "
                    "or drop (CLAUDE.md: in-body comments are one-line section headers)",
                )
            )

    for i, line in enumerate(lines):
        if fence_mask[i]:
            continue
        sanitized = _strip_eol_comment(line)
        if not sanitized.endswith("{"):
            continue
        # Find the next non-blank line
        j = i + 1
        while j < n and lines[j].strip() == "":
            j += 1
        if j >= n or fence_mask[j]:
            continue
        if _is_line_comment(lines[j]):
            violations.append(
                Violation(
                    path,
                    j + 1,
                    "comment-after-brace",
                    "`//` comment as first line of a `{` body — hoist the "
                    "comment above the block opener (CLAUDE.md: comments label "
                    "sections, they don't narrate from inside)",
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
        re.compile(r"\b(?:we'(?:ll|ve|re)|we\s+(?:need|want|have|will|should|now|first|then|can)|let'?s\b|let\s+us\b)",
                   re.IGNORECASE),
    ),
    # Throat-clearing prefixes that add no information
    (
        "throat-clearing",
        re.compile(r"^\s*(?:note(?:\s+that)?|important|keep\s+in\s+mind|remember(?:\s+that)?|please\s+note|fyi|n\.?b\.?)\s*[:,]",
                   re.IGNORECASE),
    ),
    # Tutorial step markers
    (
        "tutorial-voice",
        re.compile(r"^\s*(?:now|here|first(?:ly)?|second(?:ly)?|next|then|finally)\s*,?\s*(?:we\b|you\b|i\b)",
                   re.IGNORECASE),
    ),
    # "This is..." / "This does..." / "This function..." narration
    (
        "this-is-narration",
        re.compile(r"^\s*this\s+(?:is|does|function|method|class|file|module|code|block|line)\b",
                   re.IGNORECASE),
    ),
    # PR/fix/change self-references that rot the moment they're committed
    (
        "rot-reference",
        re.compile(r"\b(?:this\s+(?:pr|patch|fix|commit|change|cl)|the\s+(?:recent|previous|above|aforementioned)\s+(?:change|fix|pr|commit)|as\s+(?:mentioned|noted|described)\s+above|see\s+(?:above|below))\b",
                   re.IGNORECASE),
    ),
    # Hedging vocabulary — "for now", "for clarity", "for readability",
    # "in theory", "ideally", "perhaps", "maybe should"
    (
        "hedging",
        re.compile(r"\b(?:for\s+(?:now|clarity|readability|reference|completeness)|in\s+theory|ideally|perhaps|maybe\s+(?:should|we|this)|might\s+want\s+to)\b",
                   re.IGNORECASE),
    ),
    # Restating-the-code openers — strongest signal of AI prose
    (
        "restate-obvious",
        re.compile(r"^\s*(?:loop(?:s)?\s+(?:over|through)|iterate(?:s)?\s+(?:over|through)|check(?:s)?\s+(?:if|whether)|set(?:s)?\s+\w+\s+to|return(?:s)?\s+the|get(?:s)?\s+the|create(?:s)?\s+(?:a|an|the)|initialize(?:s)?\s+(?:a|an|the)|store(?:s)?\s+the|update(?:s)?\s+the)\b",
                   re.IGNORECASE),
    ),
    # AI-typical TODO/FIXME without a ticket reference — "TODO: implement this"
    (
        "todo-no-context",
        re.compile(r"^\s*(?:todo|fixme|xxx|hack)\s*[:!\-]?\s*(?:implement|handle|add|fix|consider|figure\s+out|deal\s+with|come\s+back\s+to)\b",
                   re.IGNORECASE),
    ),
    # Emoji in comments — CLAUDE.md bans them outright
    (
        "emoji",
        re.compile(r"[\U0001F300-\U0001FAFF\U00002600-\U000027BF\U0001F000-\U0001F2FF]"),
    ),
    # "Used by X" / "Called from Y" — caller references rot
    (
        "caller-reference",
        re.compile(r"\b(?:used\s+by|called\s+(?:by|from)|invoked\s+(?:by|from)|added\s+for|needed\s+for)\s+(?:the\s+)?[A-Z]?\w",
                   re.IGNORECASE),
    ),
    # Trailing ellipsis on `//` lines — typically AI hand-waving "...etc"
    (
        "trailing-ellipsis",
        re.compile(r"\.{3,}\s*$"),
    ),
)


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
        if re.match(r"^\s*(?:clang-format|code-format|NOLINT|cppcheck-suppress)", payload, re.IGNORECASE):
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
    return ("app", "qml") in zip(parts, parts[1:]) or ("app", "src") in zip(parts, parts[1:])


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
        bf_violations = [v for v in bf_violations
                         if v.line - 1 < len(post_mask) and not post_mask[v.line - 1]]
        insert_after = [idx for idx in insert_after
                        if idx < len(post_mask) and not post_mask[idx]]
        violations.extend(bf_violations)
        bf_changed = bool(insert_after)
        if bf_changed:
            new_raws = apply_brace_free_fixes(new_raws, insert_after)

    if not fix or (not qml_changed and not id_changed and not bf_changed and not crlf_changed):
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
    ".txt", ".md", ".py", ".svg", ".ts", ".js", ".lua", ".html", ".rcc", ".yml",
)


_SKIP_DIRS = {"build", "_deps", "ThirdParty", ".git", "node_modules", "qm"}


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
    # scripts/code-format.py  ->  <repo>
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
# comment-after-brace, ai-*, christmas-tree-once-merged) is flag-only and
# lives in `.code-report` for human / LLM follow-up.
_AUTO_FIXABLE_KINDS = frozenset({
    "line-endings",
    "id-blank-line",
    "brace-free-blank",
    "christmas-tree",
})


_REPORT_HEADER = """\
# Code Quality Report

Generated by `scripts/code-format.py`. Each entry below was flagged by a
heuristic that often signals AI-generated narration or a CLAUDE.md style
violation. The script will not auto-fix these — judgement calls belong
to a human or an LLM that has read the surrounding context.

## Rules to apply

- **One-line `//` section headers only.** Multi-line `//` blocks should
  collapse to one line or be deleted. Comments label sections; they do
  not narrate. (CLAUDE.md → "Comments & Doxygen".)
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

## Opt-out

Wrap a region with `// code-format off` / `// code-format on` (or the
`/* ... */` equivalent) to disable every rule between the fences. Use
sparingly and explain why in a one-line comment above the fence.

## Findings
"""


def _write_report(report_path: Path, flag_only: list[Violation]) -> None:
    """Write `.code-report` at the repo root grouping flag-only violations
    by kind, then by file. Skips writing when there's nothing to report
    and removes any stale report from a previous run."""
    if not flag_only:
        if report_path.exists():
            report_path.unlink()
        return

    by_kind: dict[str, list[Violation]] = {}
    for v in flag_only:
        by_kind.setdefault(v.kind, []).append(v)

    out: list[str] = [_REPORT_HEADER]
    for kind in sorted(by_kind):
        entries = by_kind[kind]
        out.append(f"\n### `{kind}` ({len(entries)})\n")
        for v in entries:
            out.append(f"- `{v.path}:{v.line}` — {v.message}\n")

    out.append(f"\n---\n\n_Total flagged: {len(flag_only)}_\n")
    report_path.write_text("".join(out), encoding="utf-8", newline="\n")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(
        description=__doc__.splitlines()[0],
        epilog="With no arguments, runs --fix on the repo's default trees.",
    )
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--check", action="store_true", help="report only, no writes")
    group.add_argument("--fix", action="store_true", help="rewrite files in place (default)")
    parser.add_argument("--diff", action="store_true",
                        help="show unified diff of proposed changes")
    parser.add_argument("--no-report", action="store_true",
                        help="skip writing .code-report at the repo root")
    parser.add_argument("paths", nargs="*", type=Path,
                        help="files or directories to process (default: repo trees)")

    args = parser.parse_args(argv)

    # Default to --fix when neither mode was explicitly requested
    if not args.check and not args.fix:
        args.fix = True

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
    flag_only: list[Violation] = []

    for path in files:
        violations, new_text = process_file(path, fix=args.fix)
        total_violations += len(violations)

        for v in violations:
            print(f"{v.path}:{v.line}: {v.kind}: {v.message}")
            if v.kind not in _AUTO_FIXABLE_KINDS:
                flag_only.append(v)

        if new_text is not None:
            if args.diff:
                before = path.read_text(encoding="utf-8").splitlines(keepends=True)
                after = new_text.splitlines(keepends=True)
                sys.stdout.writelines(
                    difflib.unified_diff(before, after, str(path), str(path))
                )
            if args.fix:
                path.write_text(new_text, encoding="utf-8", newline="\n")
                total_changed += 1
                print(f"{path}: rewrote", file=sys.stderr)

    if not args.no_report:
        repo_root = Path(__file__).resolve().parent.parent
        _write_report(repo_root / ".code-report", flag_only)

    if args.check:
        print(f"\n{len(files)} files scanned, {total_violations} violations "
              f"({len(flag_only)} flag-only)", file=sys.stderr)
        return 1 if total_violations else 0

    print(f"\n{len(files)} files scanned, {total_changed} rewritten, "
          f"{len(flag_only)} flag-only violations", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
