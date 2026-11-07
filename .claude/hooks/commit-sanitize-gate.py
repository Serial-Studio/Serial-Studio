#!/usr/bin/env python3
"""Claude Code PreToolUse hook: lint the files a `git commit` carries, and only those.

Until 2026-11-07 this hook demanded a fresh whole-tree `sanitize-commit.py` run (minutes
of clang-format, code-verify --fix and every generator over the entire tree) before any
commit, judged by the mtime of .code-report. That made a one-line commit cost a full
sanitize, and the sanitize rewrote files the developer had in progress. CI runs every one
of those checks on the pushed tree anyway, so the pre-commit gate now does the cheap,
scoped half: the staged C++/QML files go through `code-verify.py --check --no-report` and,
when the pinned clang-format is on PATH, `clang-format --dry-run --Werror`. Blocking
errors refuse the commit (exit 2) with the linter's own lines, which name the fix.

Design choices specific to this repo:
  - Staged files (`git diff --cached --name-only`) are the commit's payload; `-a`/`--all`
    and a pathspec commit (`git commit -- a.cpp`) also fold in modified tracked files.
  - Only blocking errors refuse; advisories are baseline debt (CLAUDE.md).
  - clang-format runs only at the exact version tests/requirements.txt pins, so a stray
    system clang-format never judges the tree against a different style.
  - Fail-open on any internal error, silent for non-commit commands.
  - The repo root honors CLAUDE_PROJECT_DIR for tests, falling back to the file location.
"""

import json
import os
import re
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

GIT_TIMEOUT_S = 10
LINT_TIMEOUT_S = 120
MAX_OUTPUT_CHARS = 6000
COMMIT_RE = re.compile(r"\bgit(\s+-\S+)*\s+commit\b")
ALL_FLAG_RE = re.compile(r"\s(-a|-am|--all)\b")
SHELL_OPERATORS = {"&&", "||", ";", "|", ">", ">>", "<"}
SOURCE_SUFFIXES = {
    ".c",
    ".cpp",
    ".cc",
    ".cxx",
    ".c++",
    ".h",
    ".hpp",
    ".hh",
    ".hxx",
    ".qml",
}
CXX_SUFFIXES = {".c", ".cpp", ".cc", ".cxx", ".c++", ".h", ".hpp", ".hh", ".hxx"}
FORMAT_SKIP = {"miniaudio.h", "fast_float.h"}


def repo_root() -> Path:
    """Resolve the repository root: CLAUDE_PROJECT_DIR when set, else this file's location."""
    override = os.environ.get("CLAUDE_PROJECT_DIR", "")
    if override:
        return Path(override).resolve()
    return Path(__file__).resolve().parents[2]


def git_lines(root: Path, args: list[str]) -> list[str]:
    """Run a read-only git command and return its stdout lines."""
    result = subprocess.run(
        ["git"] + args,
        capture_output=True,
        text=True,
        timeout=GIT_TIMEOUT_S,
        cwd=str(root),
    )
    if result.returncode != 0:
        return []
    return [line.strip('"') for line in result.stdout.splitlines() if line]


def pathspecs(command: str) -> list[str]:
    """The pathspecs after `--`, up to the next shell operator; empty when there are none."""
    try:
        tokens = shlex.split(command)
    except ValueError:
        return []
    if "--" not in tokens:
        return []
    specs = []
    for token in tokens[tokens.index("--") + 1 :]:
        if token in SHELL_OPERATORS:
            break
        specs.append(token.rstrip("/"))
    return specs


def commit_payload(root: Path, command: str) -> list[str]:
    """Files this commit would carry: the index, plus the worktree for -a or the pathspecs."""
    files = git_lines(root, ["diff", "--cached", "--name-only"])
    worktree = git_lines(root, ["diff", "--name-only"])
    if ALL_FLAG_RE.search(command):
        files += worktree
    else:
        specs = pathspecs(command)
        files += [
            f for f in worktree if any(f == s or f.startswith(s + "/") for s in specs)
        ]
    return sorted(set(files))


def pinned_clang_format(root: Path) -> str | None:
    """The clang-format on PATH, only when it is the exact release the repo pins."""
    manifest = root / "tests" / "requirements.txt"
    if not manifest.is_file():
        return None
    pin = re.search(r"^clang-format==(\S+)", manifest.read_text(encoding="utf-8"), re.M)
    binary = shutil.which("clang-format")
    if pin is None or binary is None:
        return None
    probe = subprocess.run(
        [binary, "--version"], capture_output=True, text=True, timeout=GIT_TIMEOUT_S
    )
    return binary if f"version {pin.group(1)}" in probe.stdout else None


def lint_errors(root: Path, sources: list[str]) -> str:
    """code-verify's blocking errors over the staged sources; empty when clean or advisory-only."""
    verify = root / "scripts" / "code-verify.py"
    if not verify.is_file():
        return ""
    result = subprocess.run(
        [sys.executable, str(verify), "--check", "--no-report", *sources],
        capture_output=True,
        text=True,
        timeout=LINT_TIMEOUT_S,
        cwd=str(root),
    )
    if result.returncode == 0:
        return ""
    return ((result.stdout or "") + "\n" + (result.stderr or "")).strip()


def format_errors(root: Path, sources: list[str]) -> str:
    """clang-format drift over the staged C++; empty when clean or the pin is not on PATH."""
    files = [
        s
        for s in sources
        if Path(s).suffix in CXX_SUFFIXES and Path(s).name not in FORMAT_SKIP
    ]
    binary = pinned_clang_format(root) if files else None
    if binary is None:
        return ""
    result = subprocess.run(
        [binary, "--dry-run", "--Werror", *files],
        capture_output=True,
        text=True,
        timeout=LINT_TIMEOUT_S,
        cwd=str(root),
    )
    if result.returncode == 0:
        return ""
    fix = "clang-format -i " + " ".join(files)
    return f"clang-format drift (fix: {fix})\n" + result.stderr.strip()


def main() -> None:
    try:
        event = json.load(sys.stdin)
        command = str((event.get("tool_input") or {}).get("command") or "")
        if not COMMIT_RE.search(command):
            sys.exit(0)

        root = repo_root()
        payload = commit_payload(root, command)
        sources = [
            f
            for f in payload
            if Path(f).suffix.lower() in SOURCE_SUFFIXES and (root / f).is_file()
        ]
        if not sources:
            sys.exit(0)

        checks = (lint_errors(root, sources), format_errors(root, sources))
        problems = [p for p in checks if p]
        if not problems:
            sys.exit(0)

        body = "\n\n".join(problems)
        if len(body) > MAX_OUTPUT_CHARS:
            body = body[:MAX_OUTPUT_CHARS] + "\n... (truncated)"
        sys.stderr.write(
            "Commit blocked: the staged source files have blocking lint or format errors. "
            "Fix them and retry the commit.\n\n" + body + "\n"
        )
        sys.exit(2)
    except SystemExit:
        raise
    except Exception:
        sys.exit(0)


if __name__ == "__main__":
    main()
