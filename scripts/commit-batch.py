#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Serial Studio batch committer: a planned series of commits in one run.

A change that the commit-shape rules split into several commits (spec and plan first, an isolated
surface alone, at most 60 paths) used to cost a git add, a shape check and a git commit per commit,
each a separate step. This tool takes the whole series as one plan and, for every entry in order:

  1. builds the commit in a private index (HEAD plus exactly the entry's paths), so the real index
     is never staged and a refused entry leaves nothing behind to unstage;
  2. judges it with the same engines the commit hooks load: scripts/commit-shape.py (size,
     isolation, spec-born, fix(ci), and the Co-Authored-By trailer when Claude issues it) and the
     sanitize-freshness check of .claude/hooks/commit-sanitize-gate.py;
  3. commits it, then marks the committed paths as staged-clean in the real index.

It stops at the first refusal with the engine's own sentence; earlier commits stay made and the
rest of the work stays in the working tree. The rules are not relaxed: this changes how many steps
a valid series takes, never which series are valid.

Plan (JSON):
  {"commits": [{"message": "subject\\n\\nbody\\n\\nCo-Authored-By: ...", "paths": ["a.cpp", "dir/"]},
               {"message": "...", "rest": true}]}
  "paths" are pathspecs relative to the repository root; "rest": true takes every change left and
  is only valid as the last entry. A path claimed twice, a path with no change, or a change no entry
  claims (without a "rest" entry) is refused before anything is committed.

Usage:
  commit-batch.py PLAN.json [--require-coauthor] [--check]

  --check             validate the whole plan (paths, sanitize freshness, coauthor trailers) and
                      exit without committing; shape is judged per commit as it is built.
  --require-coauthor  every message must carry the `Co-Authored-By: Claude` trailer. Implied when
                      run under Claude Code (CLAUDECODE=1), as the hook implies it.

Exit 0 when every commit was made (or the plan checks clean), 1 on a refusal, 2 on a usage error.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import os
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def load_module(path: Path, name: str):
    """Loads a script or hook file as a module, so the batch and the hooks share one engine."""
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        sys.exit(f"commit-batch: cannot load {path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def git(args: list[str], env: dict | None = None, check: bool = True) -> str:
    """Runs git at the repository root and returns its stdout."""
    result = subprocess.run(
        ["git", *args], cwd=REPO_ROOT, env=env, capture_output=True, text=True
    )
    if check and result.returncode != 0:
        sys.exit(f"commit-batch: git {' '.join(args)} failed:\n{result.stderr.strip()}")
    return result.stdout


def changed_paths() -> set[str]:
    """Every path the working tree changes against HEAD, untracked files included."""
    out = git(["status", "--porcelain", "-z", "--untracked-files=all"])
    paths, entries, i = set(), out.split("\0"), 0
    while i < len(entries):
        entry = entries[i]
        if len(entry) > 3:
            paths.add(entry[3:])
            if entry[0] in "RC":
                i += 1
        i += 1
    return paths


def claimed(paths: set[str], specs: list[str]) -> set[str]:
    """The changed paths a list of pathspecs covers (a file, or everything under a directory)."""
    hits = set()
    for spec in specs:
        clean = spec.rstrip("/")
        hits |= {p for p in paths if p == clean or p.startswith(clean + "/")}
    return hits


def plan_assignments(plan: list[dict], changes: set[str]) -> list[set[str]]:
    """Resolves each entry to its changed paths, refusing overlaps, empty entries and leftovers."""
    taken, out = set(), []
    for index, entry in enumerate(plan):
        if entry.get("rest"):
            if index != len(plan) - 1:
                sys.exit('commit-batch: a "rest" entry must be the last one')
            out.append(changes - taken)
            taken |= out[-1]
            continue

        hits = claimed(changes, entry.get("paths", []))
        for spec in entry.get("paths", []):
            if not claimed(changes, [spec]):
                sys.exit(f"commit-batch: entry {index + 1}: {spec} has no change")

        overlap = hits & taken
        if overlap:
            sys.exit(
                f"commit-batch: entry {index + 1} claims paths an earlier entry took: "
                + ", ".join(sorted(overlap)[:5])
            )
        out.append(hits)
        taken |= hits

    left = changes - taken
    if left:
        sys.exit(
            'commit-batch: changes no entry claims (add them or a final "rest" entry): '
            + ", ".join(sorted(left)[:8])
        )
    return out


def check_messages(plan: list[dict], shape, require_coauthor: bool) -> None:
    """Refuses an empty message, and a missing Claude trailer when one is required."""
    for index, entry in enumerate(plan):
        message = entry.get("message", "")
        if not message.strip():
            sys.exit(f"commit-batch: entry {index + 1} has no message")
        if require_coauthor:
            finding = shape.check_coauthor(message)
            if finding:
                sys.exit(f"commit-batch: entry {index + 1}: {finding}")


def check_sanitized(paths: set[str], sanitize_gate) -> None:
    """The sanitize hook's freshness rule over the whole plan: no payload newer than its report."""
    files = sorted(p for p in paths if (REPO_ROOT / p).is_file())
    sources = [p for p in files if Path(p).suffix in sanitize_gate.SOURCE_SUFFIXES]
    docs = [p for p in files if sanitize_gate.is_doc_target(p)]
    stale = sanitize_gate.stale_files(REPO_ROOT, sources, ".code-report")
    stale += sanitize_gate.stale_files(REPO_ROOT, docs, ".doc-report")
    if stale:
        sys.exit(
            "commit-batch: run python3 scripts/sanitize-commit.py first; newer than its last "
            "run: " + ", ".join(stale[:8])
        )


def commit_entry(
    index: int, message: str, paths: set[str], require_coauthor: bool
) -> None:
    """Builds, judges and makes one commit in a private index, then syncs the real index."""
    with tempfile.TemporaryDirectory() as scratch:
        env = dict(os.environ, GIT_INDEX_FILE=str(Path(scratch) / "index"))
        git(["read-tree", "HEAD"], env=env)
        git(["add", "-A", "--", *sorted(paths)], env=env)

        message_file = Path(scratch) / "message"
        message_file.write_text(message if message.endswith("\n") else message + "\n")
        args = [
            sys.executable,
            str(REPO_ROOT / "scripts" / "commit-shape.py"),
            "--staged",
            "--message-file",
            str(message_file),
        ]
        if require_coauthor:
            args.append("--require-coauthor")
        verdict = subprocess.run(
            args, cwd=REPO_ROOT, env=env, capture_output=True, text=True
        )
        if verdict.returncode != 0:
            sys.exit(
                f"commit-batch: entry {index + 1} refused, nothing more committed:\n"
                + (verdict.stdout + verdict.stderr).strip()
            )

        git(["commit", "-q", "-F", str(message_file)], env=env)

    git(["add", "-A", "--", *sorted(paths)])
    print(git(["log", "--oneline", "-1"]).strip())


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("plan", help="JSON plan file")
    parser.add_argument(
        "--check", action="store_true", help="validate without committing"
    )
    parser.add_argument("--require-coauthor", action="store_true")
    args = parser.parse_args(argv)

    plan = json.loads(Path(args.plan).read_text()).get("commits", [])
    if not plan:
        print("commit-batch: the plan has no commits", file=sys.stderr)
        return 2

    if git(["diff", "--cached", "--name-only"]).strip():
        sys.exit(
            "commit-batch: the index already has staged changes; commit-batch builds every "
            "commit itself, so start from an index that matches HEAD"
        )

    require = args.require_coauthor or os.environ.get("CLAUDECODE") == "1"
    shape = load_module(REPO_ROOT / "scripts" / "commit-shape.py", "commit_shape")
    gate = load_module(
        REPO_ROOT / ".claude" / "hooks" / "commit-sanitize-gate.py", "sanitize_gate"
    )

    changes = changed_paths()
    plan_file = Path(args.plan).resolve()
    if plan_file.is_relative_to(REPO_ROOT):
        changes.discard(plan_file.relative_to(REPO_ROOT).as_posix())

    assignments = plan_assignments(plan, changes)
    check_messages(plan, shape, require)
    check_sanitized(set().union(*assignments), gate)
    if args.check:
        for entry, paths in zip(plan, assignments):
            print(f"{len(paths):3d} paths  {entry['message'].splitlines()[0]}")
        return 0

    for index, (entry, paths) in enumerate(zip(plan, assignments)):
        commit_entry(index, entry["message"], paths, require)

    return 0


if __name__ == "__main__":
    sys.exit(main())
