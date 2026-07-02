#!/usr/bin/env python3
"""Claude Code PreToolUse hook: gate `git commit` behind a fresh sanitize run.

CLAUDE.md's contract is "run scripts/sanitize-commit.py before every commit".
This hook enforces the observable half mechanically: sanitize's last run is
marked by the .code-report / .doc-report files it regenerates, so a commit
whose staged files are newer than the matching report is blocked (exit 2)
with instructions to run sanitize and retry.

Design choices specific to this repo:
  - Staged files (`git diff --cached --name-only`) are the commit's payload;
    `-a`/`--all` commits also fold in modified tracked files.
  - Source staleness is judged against .code-report; doc staleness (the
    documentation-verify targets: README.md, AGENTS.md, doc/help/**,
    examples/**/README.md) against .doc-report. A missing report counts as
    fresh: the linters delete their report when there is nothing to flag, so
    absence means a clean tree, not a tree sanitize never saw.
  - Fail-open on any internal error, silent for non-commit commands: only a
    genuinely stale commit attempt is ever blocked.
"""

import json
import re
import subprocess
import sys
from pathlib import Path

GIT_TIMEOUT_S = 10
COMMIT_RE = re.compile(r"\bgit(\s+-\S+)*\s+commit\b")
ALL_FLAG_RE = re.compile(r"\s(-a|-am|--all)\b")
SOURCE_SUFFIXES = {".c", ".cpp", ".cc", ".cxx", ".c++", ".h", ".hpp", ".hh", ".hxx", ".qml"}


def repo_root() -> Path:
    """Resolve the repository root from this file's location (.claude/hooks/)."""
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


def commit_payload(root: Path, command: str) -> list[str]:
    """Files this commit would carry: the index, plus the worktree for -a/--all."""
    files = git_lines(root, ["diff", "--cached", "--name-only"])
    if ALL_FLAG_RE.search(command):
        files += git_lines(root, ["diff", "--name-only"])
    return sorted(set(files))


def is_doc_target(rel: str) -> bool:
    """Match the documentation-verify.py target set."""
    if rel in ("README.md", "AGENTS.md"):
        return True
    if rel.startswith("doc/help/") and rel.endswith(".md"):
        return True
    return rel.startswith("examples/") and rel.endswith("README.md")


def stale_files(root: Path, files: list[str], report: str) -> list[str]:
    """Files newer than the report (none when it is absent: the tree is clean)."""
    marker = root / report
    if not files or not marker.is_file():
        return []

    threshold = marker.stat().st_mtime
    stale: list[str] = []
    for rel in files:
        candidate = root / rel
        if candidate.is_file() and candidate.stat().st_mtime > threshold:
            stale.append(rel)
    return stale


def main() -> None:
    try:
        event = json.load(sys.stdin)
        command = str((event.get("tool_input") or {}).get("command") or "")
        if not COMMIT_RE.search(command):
            sys.exit(0)

        root = repo_root()
        payload = commit_payload(root, command)
        sources = [f for f in payload if Path(f).suffix.lower() in SOURCE_SUFFIXES]
        docs = [f for f in payload if is_doc_target(f)]

        stale_src = stale_files(root, sources, ".code-report")
        stale_doc = stale_files(root, docs, ".doc-report")
        if not stale_src and not stale_doc:
            sys.exit(0)

        parts = []
        if stale_src:
            parts.append(f"{len(stale_src)} source file(s) newer than .code-report")
        if stale_doc:
            parts.append(f"{len(stale_doc)} doc file(s) newer than .doc-report")
        sys.stderr.write(
            "Commit blocked: sanitize is stale for this commit ("
            + "; ".join(parts)
            + "). Run `python3 scripts/sanitize-commit.py`, then retry the commit "
            "(CLAUDE.md: run sanitize-commit.py before every commit).\n"
        )
        sys.exit(2)
    except SystemExit:
        raise
    except Exception:
        sys.exit(0)


if __name__ == "__main__":
    main()
