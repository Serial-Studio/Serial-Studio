#!/usr/bin/env python3
"""Remove comments that live inside C++ function bodies (cxx-inbody-comment).

Deterministic, tree-sitter-backed companion to scripts/code-verify.py. It
is the safe bulk half of the in-body-comment cleanup: every function here is
capped at 100 lines, so the contract is one `/** @brief ... */` above the
function plus code that reads on its own, and a comment INSIDE the body is
removed.

What it removes: every `//` or `/* */` comment whose tree-sitter node sits
inside a function body (or a constructor's member-initializer list). What it
leaves untouched:

  - The `/** @brief ... */` ABOVE a function -- that node is a sibling of the
    definition, never inside the body, so it is never matched.
  - `//---` / `//===` concern-group banners BETWEEN functions (same reason).
  - Tooling pragmas inside a body (`// clang-format off/on`, `// NOLINT`,
    `// cppcheck-suppress`, `// fallthrough`) -- directives, not prose.
  - Anything inside a `// code-verify off` / `// code-verify on` fence.

Safety: only comment byte-spans are deleted. Code bytes are never moved or
rewritten, so a behavior change is impossible -- the worst case is a stray
blank line, and blank-line runs created by removal collapse to one. Trailing
whitespace is trimmed. Run `code-verify.py --fix` afterwards to normalize the
brace-free-body blank-line rule.

What it deliberately does NOT do (left to a human / the cleanup workflow):
writing or improving doxygen briefs, folding a load-bearing "why" up into the
brief, and deciding whether a function is dead. This tool only does the
mechanical, reviewable bulk.

Usage:
    python3 scripts/strip-inbody-comments.py app/src/UI/Taskbar.cpp
    python3 scripts/strip-inbody-comments.py app/src           # whole tree
    python3 scripts/strip-inbody-comments.py --check app/src   # count only
    python3 scripts/strip-inbody-comments.py --diff <path>     # show changes

Exit codes:
    0 - success (or, in --check mode, nothing to strip)
    1 - --check found strippable comments
    2 - argument / environment error (e.g. tree-sitter missing)
"""

from __future__ import annotations

import argparse
import difflib
import importlib.util
import os
import re
import sys
from pathlib import Path
from typing import Iterable

# Reuse the exact same tree-sitter parser and the in-body / pragma helpers the
# linter uses, so this tool and `cxx-inbody-comment` can never disagree about
# what counts as an in-body comment. Loaded by absolute path because the
# sibling module's importable name is fixed.
_rules_spec = importlib.util.spec_from_file_location(
    "code_verify_rules", Path(__file__).with_name("code_verify_rules.py")
)
if _rules_spec is None or _rules_spec.loader is None:
    print("cannot locate code_verify_rules.py next to this script", file=sys.stderr)
    sys.exit(2)
_rules = importlib.util.module_from_spec(_rules_spec)
sys.modules["code_verify_rules"] = _rules
_rules_spec.loader.exec_module(_rules)

if not _rules.HAS_TREE_SITTER:
    print(
        "tree-sitter / tree-sitter-cpp not installed; cannot strip safely",
        file=sys.stderr,
    )
    sys.exit(2)


# Fence detection mirrors code-verify.py's _compute_fence_mask. Kept inline
# (the regexes are tiny) so this script doesn't have to import the dash-named
# driver module.
_FENCE_OFF_RE = re.compile(r"^\s*(?://|/\*)\s*code-(?:verify|format)\s+off\b")
_FENCE_ON_RE = re.compile(r"^\s*(?://|/\*)\s*code-(?:verify|format)\s+on\b")

_CPP_SUFFIXES = (".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hxx", ".mm")
_SKIP_DIRS = {"build", "_deps", "ThirdParty", ".git", "node_modules", "qm"}

# Vendored / upstream files keep their original comment style: any path under
# a skipped directory (ThirdParty, _deps, ...) or a known vendored helper by
# name. Mirrors code_verify_rules._is_vendored_path so the strip and the linter
# agree on what is first-party.
_VENDORED_NAME_PREFIXES = ("SimpleCrypt",)


def _is_vendored(path: Path) -> bool:
    """True when `path` is vendored / upstream and must never be stripped,
    regardless of whether it was reached by a directory walk or passed
    explicitly on the command line."""
    parts = set(path.parts)
    if parts & _SKIP_DIRS:
        return True
    lowered = str(path).replace("\\", "/").lower()
    if "/lemonsqueezy/" in lowered:
        return True
    return any(path.name.startswith(p) for p in _VENDORED_NAME_PREFIXES)


def _fence_mask(lines: list[str]) -> list[bool]:
    """Per-line bitmap; True when the line sits inside a code-verify fence."""
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


def _inbody_comment_spans(src: bytes) -> list[tuple[int, int]]:
    """Return sorted (start_byte, end_byte) spans of every comment that lives
    inside a function body, excluding fenced regions and tooling pragmas."""
    tree = _rules._CPP_PARSER.parse(src)
    text = src.decode("utf-8", errors="replace")
    mask = _fence_mask(text.split("\n"))
    spans: list[tuple[int, int]] = []
    for node in _rules._walk(tree.root_node):
        if node.type != "comment":
            continue
        if not _rules._comment_in_function_body(node):
            continue
        line_idx = node.start_point[0]
        if 0 <= line_idx < len(mask) and mask[line_idx]:
            continue
        if _rules._is_inbody_pragma(_rules._node_text(node, src)):
            continue
        spans.append((node.start_byte, node.end_byte))
    spans.sort()
    return spans


def _strip_text(src_text: str) -> str:
    """Return `src_text` with every in-body comment removed.

    A line-leading comment takes its whole line (including indentation and the
    trailing newline) with it; a trailing comment after code leaves the code
    and is trimmed of the resulting trailing whitespace. Runs of blank lines
    created by removal collapse to a single blank, and the original trailing
    newline is preserved."""
    src = src_text.encode("utf-8")
    spans = _inbody_comment_spans(src)
    if not spans:
        return src_text

    # Delete right-to-left so earlier byte offsets stay valid. For each comment
    # span, swallow the leading indentation when the comment starts the line,
    # and the trailing newline when nothing but whitespace follows it -- that
    # turns a whole-line comment into a clean line deletion instead of an empty
    # indented line.
    out = bytearray(src)
    for start, end in reversed(spans):
        line_start = out.rfind(b"\n", 0, start) + 1
        before = out[line_start:start]
        line_end = out.find(b"\n", end)
        if line_end == -1:
            line_end = len(out)
        after = out[end:line_end]
        leading_only = before.strip() == b""
        trailing_only = after.strip() == b""
        if leading_only and trailing_only:
            cut_end = line_end + 1 if line_end < len(out) else line_end
            del out[line_start:cut_end]
        else:
            del out[start:end]

    result = out.decode("utf-8", errors="replace")
    lines = [ln.rstrip() for ln in result.split("\n")]

    collapsed: list[str] = []
    for ln in lines:
        if ln == "" and collapsed and collapsed[-1] == "":
            continue
        collapsed.append(ln)

    new_text = "\n".join(collapsed)
    if src_text.endswith("\n") and not new_text.endswith("\n"):
        new_text += "\n"
    return new_text


def _iter_files(targets: list[Path]) -> Iterable[Path]:
    """Yield C++ source files under each target (recursive for directories),
    skipping build / vendored trees. Vendored files are skipped even when
    passed explicitly -- an explicit `app/src/ThirdParty/x.h` must not be
    stripped just because the directory walk was bypassed."""
    for target in targets:
        if target.is_file():
            if target.suffix in _CPP_SUFFIXES and not _is_vendored(target):
                yield target
            continue
        for root, dirs, files in os.walk(target):
            dirs[:] = [d for d in dirs if d not in _SKIP_DIRS]
            for name in files:
                cand = Path(root) / name
                if name.endswith(_CPP_SUFFIXES) and not _is_vendored(cand):
                    yield cand


def _write_lf(path: Path, text: str) -> None:
    """Write `text` as UTF-8 with LF endings on every platform."""
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    path.write_bytes(normalized.encode("utf-8"))


def main(argv: list[str]) -> int:
    for stream in (sys.stdout, sys.stderr):
        reconfigure = getattr(stream, "reconfigure", None)
        if reconfigure is not None:
            reconfigure(encoding="utf-8", errors="replace")

    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument(
        "--check", action="store_true", help="report counts only, write nothing"
    )
    parser.add_argument(
        "--diff", action="store_true", help="print a unified diff of each change"
    )
    parser.add_argument("paths", nargs="+", type=Path, help="files or directories")
    args = parser.parse_args(argv)

    files = sorted(set(_iter_files(args.paths)))
    if not files:
        print("no C++ source files found", file=sys.stderr)
        return 2

    total_spans = 0
    changed = 0
    for path in files:
        raw = path.read_bytes().decode("utf-8", errors="replace")
        spans = _inbody_comment_spans(raw.encode("utf-8"))
        if not spans:
            continue
        total_spans += len(spans)
        new_text = _strip_text(raw)
        if new_text == raw:
            continue
        changed += 1
        if args.diff:
            sys.stdout.writelines(
                difflib.unified_diff(
                    raw.splitlines(keepends=True),
                    new_text.splitlines(keepends=True),
                    str(path),
                    str(path),
                )
            )
        if args.check:
            print(f"{path}: {len(spans)} in-body comment(s)")
        else:
            _write_lf(path, new_text)
            print(f"{path}: stripped {len(spans)} in-body comment(s)", file=sys.stderr)

    mode = "would strip" if args.check else "stripped"
    print(
        f"\n{len(files)} files scanned, {mode} {total_spans} comment(s) "
        f"across {changed} file(s)",
        file=sys.stderr,
    )
    return 1 if (args.check and total_spans) else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
