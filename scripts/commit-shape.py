#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Serial Studio commit-shape gate.

Two of the Claude era's user-facing failures had the same shape: a change rode
inside a package too large to read, mixed with unrelated files, on a surface no
reviewer or CI job exercised (the 2026-09-03 licensing inversion inside an
815-file commit; the 2026-06-01 macOS slice regression inside a CI rewrite).
This judges the shape of a commit, not its content, before it lands:

  size        more than `max_files` paths needs a `Mechanical:` trailer naming
              what generated the diff (a formatter, a vendored bump, a rename)
  isolation   a commit touching an isolated surface (licensing, API auth and
              consent, the CI workflows) carries only that surface, its tests
              and its docs, so the reviewer reads one thing; no trailer lifts it
  spec-born   a commit that creates a spec.md or plan.md carries nothing
              outside that spec directory: the plan is born before the code,
              and git shows the order
  ci-fix      a `fix(ci)` commit cites the failing run it answers
              (github.com/<owner>/<repo>/actions/runs/<id>)
  coauthor    with --require-coauthor, the message carries a
              `Co-Authored-By: Claude` trailer; the hook sets this, because a
              commit the hook sees is always one Claude issued

Usage:
  commit-shape.py --staged [--message TEXT | --message-file PATH] [--worktree]
                  [--require-coauthor]
  commit-shape.py --command 'git commit -m ...'   the index against the message
                  parsed from a shell command (what the hook does)
  commit-shape.py --range A..B                    every commit in a range

Exit 0 when every commit passes, 1 on a violation, 2 when the check could not
run. Rules live in scripts/commit-shape.json.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import re
import shlex
import subprocess
import sys
from pathlib import Path

CONFIG_NAME = "commit-shape.json"
HEREDOC_RE = re.compile(r"<<-?\s*(['\"]?)(\w+)\1\s*\n(.*?)\n\s*\2(?:\s|$)", re.S)
COAUTHOR_RE = re.compile(r"^Co-Authored-By:.*\bclaude\b", re.I | re.M)
ALL_FLAG_RE = re.compile(r"\s(-a|-am|--all)\b")
AMEND_RE = re.compile(r"\s--amend\b")


def repo_root(start: Path | None = None) -> Path:
    """The git toplevel for `start` (or the cwd); exits 2 outside a repository."""
    try:
        out = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=str(start) if start else None,
            check=True,
            capture_output=True,
            text=True,
        ).stdout.strip()
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("commit-shape: not inside a git repository", file=sys.stderr)
        sys.exit(2)
    return Path(out)


def load_config(root: Path) -> dict:
    """The rule table beside this script, or beside the root's scripts/ dir."""
    for candidate in (
        root / "scripts" / CONFIG_NAME,
        Path(__file__).with_name(CONFIG_NAME),
    ):
        if candidate.is_file():
            return json.loads(candidate.read_text(encoding="utf-8"))
    raise FileNotFoundError(CONFIG_NAME)


def git_lines(root: Path, *args: str) -> list[str]:
    """stdout lines of a read-only git command; empty on failure."""
    done = subprocess.run(
        ["git", *args], cwd=str(root), capture_output=True, text=True, timeout=30
    )
    if done.returncode != 0:
        return []
    return [line for line in done.stdout.splitlines() if line.strip()]


def parse_name_status(lines: list[str]) -> list[tuple[str, str]]:
    """`git diff --name-status` lines into (status letter, path) pairs."""
    pairs = []
    for line in lines:
        status, _, path = line.partition("\t")
        if path:
            pairs.append((status.strip()[:1], path.strip().strip('"')))
    return pairs


def staged_payload(root: Path, include_worktree: bool = False) -> list[tuple[str, str]]:
    """What a commit would carry: the index, plus the worktree for -a/--all."""
    pairs = parse_name_status(
        git_lines(root, "diff", "--cached", "--name-status", "--no-renames")
    )
    if include_worktree:
        pairs += parse_name_status(
            git_lines(root, "diff", "--name-status", "--no-renames")
        )
    seen: dict[str, str] = {}
    for status, path in pairs:
        seen.setdefault(path, status)
    return sorted(((status, path) for path, status in seen.items()), key=lambda t: t[1])


def commit_payload(root: Path, sha: str) -> list[tuple[str, str]]:
    """The paths one existing commit carries."""
    lines = git_lines(root, "show", "--format=", "--name-status", "--no-renames", sha)
    return [(status, path) for status, path in parse_name_status(lines)]


def commit_message(root: Path, sha: str) -> str:
    return "\n".join(git_lines(root, "show", "-s", "--format=%B", sha))


def message_from_command(command: str, root: Path | None = None) -> str | None:
    """The message a `git commit` shell command would record, or None when the command
    leaves it to an editor, names a message file this parser cannot read (a shell variable
    in the path), or reuses a commit it cannot see."""
    heredoc = HEREDOC_RE.search(command)
    if heredoc:
        return heredoc.group(3)
    try:
        tokens = shlex.split(command)
    except ValueError:
        return None

    parts: list[str] = []
    i = 0
    while i < len(tokens):
        token = tokens[i]
        if token in ("-m", "--message") and i + 1 < len(tokens):
            parts.append(tokens[i + 1])
            i += 2
            continue
        if token.startswith("--message="):
            parts.append(token.split("=", 1)[1])
        elif token.startswith("-m") and not token.startswith("--") and len(token) > 2:
            parts.append(token[2:])
        elif token in ("-F", "--file") and i + 1 < len(tokens):
            parts.append(_read_message_file(tokens[i + 1], root))
            i += 2
            continue
        elif token.startswith("--file="):
            parts.append(_read_message_file(token.split("=", 1)[1], root))
        i += 1
    if any(part is None for part in parts):
        return None
    if parts:
        return "\n\n".join(parts)
    if root is not None and AMEND_RE.search(command):
        return commit_message(root, "HEAD")
    return None


def _read_message_file(path: str, root: Path | None) -> str | None:
    candidate = Path(path)
    if not candidate.is_absolute() and root is not None:
        candidate = root / candidate
    try:
        return candidate.read_text(encoding="utf-8")
    except OSError:
        return None


def matches(path: str, pattern: str) -> bool:
    """A pattern ending in '/' is a directory prefix; anything else is an exact path."""
    if pattern.endswith("/"):
        return path.startswith(pattern)
    return path == pattern


def in_any(path: str, patterns: list[str]) -> bool:
    return any(matches(path, pattern) for pattern in patterns)


def subject_of(message: str) -> str:
    for line in message.splitlines():
        if line.strip():
            return line.strip()
    return ""


def has_trailer(message: str, name: str) -> bool:
    return re.search(rf"^{re.escape(name)}:\s*\S", message, re.M) is not None


def check_size(paths: list[str], message: str | None, cfg: dict) -> str | None:
    limit = int(cfg.get("max_files", 60))
    if len(paths) <= limit:
        return None
    trailer = cfg.get("mechanical_trailer", "Mechanical")
    if message is not None and has_trailer(message, trailer):
        return None
    return (
        f"size: {len(paths)} paths staged, more than {limit}. Split the commit so a reviewer "
        f"can read it, or add a '{trailer}: <what generated the diff>' trailer when one tool "
        "produced every change"
    )


def check_isolation(paths: list[str], cfg: dict) -> str | None:
    hit = [
        group
        for group in cfg.get("isolated", [])
        if any(in_any(p, group["paths"]) for p in paths)
    ]
    if not hit:
        return None
    protected = [pattern for group in cfg["isolated"] for pattern in group["paths"]]
    companions = [pattern for group in hit for pattern in group.get("companions", [])]
    offenders = [
        p for p in paths if not in_any(p, protected) and not in_any(p, companions)
    ]
    if not offenders:
        return None
    names = ", ".join(group["name"] for group in hit)
    shown = ", ".join(offenders[:5]) + (" ..." if len(offenders) > 5 else "")
    return (
        f"isolation: {names} changes land alone with their tests and docs, but "
        f"{len(offenders)} other path(s) are staged ({shown}). Commit the {names} change "
        "first, then the rest"
    )


def check_spec_born(payload: list[tuple[str, str]], cfg: dict) -> str | None:
    spec_dir = cfg.get("spec_dir", "doc/claude/specs/")
    born = set()
    for status, path in payload:
        if status != "A" or not path.startswith(spec_dir):
            continue
        parts = path[len(spec_dir) :].split("/")
        if len(parts) == 2 and parts[1] in cfg.get(
            "spec_born_files", ["spec.md", "plan.md"]
        ):
            born.add(spec_dir + parts[0] + "/")
    if not born:
        return None
    others = [p for _, p in payload if not any(p.startswith(d) for d in born)]
    if not others:
        return None
    shown = ", ".join(others[:5]) + (" ..." if len(others) > 5 else "")
    return (
        f"spec-born: {', '.join(sorted(born))} is created in this commit, so it carries only "
        f"that spec directory, but {len(others)} other path(s) are staged ({shown}). Land the "
        "spec and plan first, then the implementation"
    )


def check_ci_fix(message: str, cfg: dict) -> str | None:
    if not re.search(cfg.get("ci_fix_subject", r"^fix\(ci\)"), subject_of(message)):
        return None
    if re.search(cfg.get("run_url", r"github\.com/\S+/actions/runs/\d+"), message):
        return None
    return (
        "ci-fix: a fix(ci) commit cites the failing run it answers; put the run URL "
        "(github.com/<owner>/<repo>/actions/runs/<id>) in the body"
    )


def check_coauthor(message: str | None) -> str | None:
    if message is None:
        return (
            "coauthor: the commit message could not be read from the command (no -m, -F or "
            "heredoc), so the Co-Authored-By: Claude trailer cannot be checked; pass it with -m"
        )
    if COAUTHOR_RE.search(message):
        return None
    return "coauthor: add the 'Co-Authored-By: Claude ...' trailer; every commit Claude issues carries it"


def evaluate(
    payload: list[tuple[str, str]],
    message: str | None,
    cfg: dict,
    require_coauthor: bool = False,
) -> list[str]:
    """Every rule the commit breaks, as one sentence each; empty when it passes."""
    paths = [path for _, path in payload]
    findings = [
        check_size(paths, message, cfg),
        check_isolation(paths, cfg),
        check_spec_born(payload, cfg),
        check_ci_fix(message, cfg) if message is not None else None,
        check_coauthor(message) if require_coauthor else None,
    ]
    return [finding for finding in findings if finding]


def judge_command(
    root: Path, command: str, require_coauthor: bool = False
) -> list[str]:
    """What the hook asks: the index (plus the worktree for -a) against the command's message."""
    cfg = load_config(root)
    payload = staged_payload(
        root, include_worktree=ALL_FLAG_RE.search(command) is not None
    )
    return evaluate(payload, message_from_command(command, root), cfg, require_coauthor)


def judge_range(root: Path, span: str) -> int:
    cfg = load_config(root)
    failed = 0
    for sha in git_lines(root, "rev-list", "--reverse", "--no-merges", span):
        findings = evaluate(commit_payload(root, sha), commit_message(root, sha), cfg)
        if not findings:
            continue
        failed += 1
        print(f"{sha[:9]} {subject_of(commit_message(root, sha))}")
        for finding in findings:
            print(f"  - {finding}")
    print(f"commit-shape: {failed} commit(s) with findings in {span}")
    return 1 if failed else 0


def parse_args(argv: list[str] | None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Judge the shape of a commit before it lands"
    )
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--staged", action="store_true", help="judge the index")
    mode.add_argument(
        "--command", help="judge the index against a `git commit` shell command"
    )
    mode.add_argument("--range", help="judge every commit in A..B")
    parser.add_argument("--message", help="the commit message (with --staged)")
    parser.add_argument(
        "--message-file", help="file holding the commit message (with --staged)"
    )
    parser.add_argument(
        "--worktree", action="store_true", help="include unstaged changes (-a)"
    )
    parser.add_argument("--require-coauthor", action="store_true")
    parser.add_argument(
        "--root", help="repository root (default: the cwd's repository)"
    )
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    root = repo_root(Path(args.root) if args.root else None)
    if args.range:
        return judge_range(root, args.range)

    if args.command:
        findings = judge_command(root, args.command, args.require_coauthor)
    else:
        message = args.message
        if args.message_file:
            message = Path(args.message_file).read_text(encoding="utf-8")
        payload = staged_payload(root, include_worktree=args.worktree)
        findings = evaluate(payload, message, load_config(root), args.require_coauthor)

    for finding in findings:
        print(f"commit-shape: {finding}", file=sys.stderr)
    if findings:
        return 1
    print("commit-shape: no findings")
    return 0


if __name__ == "__main__":
    sys.exit(main())
