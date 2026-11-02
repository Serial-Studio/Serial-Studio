#!/usr/bin/env python3
"""Claude Code PreToolUse hook: refuse whole-file reads above a line budget.

Mechanizes CLAUDE.md's "searches fan out, conclusions come back": a file over
MAX_LINES read in one piece lands whole in the main context, where a read-only
subagent could have returned the conclusion instead. Two tool shapes are gated:

  Read  - a file_path with no offset/limit and more than MAX_LINES lines is
          denied, with a reason that points at the Explore subagent and at a
          sliced re-read for the part that will be edited.
  Bash  - `cat`/`less`/`more` on such a file is denied the same way. Piped or
          redirected commands pass (a targeted read, or not a read into
          context), and so do `head`, `tail`, `sed -n` and `grep`, which are
          targeted by construction.

Design choices specific to this repo:
  - MAX_LINES defaults to 800 (SS_READ_GATE_LINES overrides). Spotify's shunt
    plugin, where the pattern comes from, gates at 350; here that would refuse
    a quarter of the tree, and the 1500-line TU census already caps growth.
  - The hotpath files CLAUDE.md orders read in full (FrameBuilder,
    CircularBuffer, FrameReader, Dashboard) and the canonical driver
    (BluetoothLE) pass the gate by basename, so the two rules never collide.
  - Fail-open everywhere: any internal error exits 0 silently. A denied read
    is a redirect, never a loss, so the hook only ever costs one retry.
  - The repo root honors CLAUDE_PROJECT_DIR for tests, falling back to the file location.
"""

import json
import os
import re
import shlex
import sys
from pathlib import Path

DEFAULT_MAX_LINES = 800
READ_IN_FULL = {
    "FrameBuilder",
    "CircularBuffer",
    "FrameReader",
    "Dashboard",
    "BluetoothLE",
}
WHOLE_FILE_COMMANDS = {"cat", "less", "more"}
SEGMENT_RE = re.compile(r"&&|\|\||;|\n")


def repo_root() -> Path:
    """Resolve the repository root: CLAUDE_PROJECT_DIR when set, else this file's location."""
    override = os.environ.get("CLAUDE_PROJECT_DIR", "")
    if override:
        return Path(override).resolve()
    return Path(__file__).resolve().parents[2]


def max_lines() -> int:
    """The line budget, SS_READ_GATE_LINES when it is a positive integer."""
    raw = os.environ.get("SS_READ_GATE_LINES", "")
    if raw.isdigit() and int(raw) > 0:
        return int(raw)
    return DEFAULT_MAX_LINES


def count_lines(path: Path) -> int:
    """Line count of a regular file, 0 when it cannot be read."""
    try:
        with path.open("rb") as handle:
            return sum(1 for _ in handle)
    except OSError:
        return 0


def oversized(path: Path, budget: int) -> int:
    """The file's line count when it exceeds the budget and is not read-in-full, else 0."""
    if not path.is_file() or path.stem in READ_IN_FULL:
        return 0

    lines = count_lines(path)
    return lines if lines > budget else 0


def read_target(event: dict) -> Path | None:
    """The whole-file target of a Read call, None for a sliced read."""
    tool_input = event.get("tool_input") or {}
    raw = tool_input.get("file_path")
    if not raw or tool_input.get("offset") or tool_input.get("limit"):
        return None
    return Path(str(raw))


def bash_targets(event: dict, root: Path) -> list[Path]:
    """Files a Bash command reads whole into context via cat/less/more."""
    command = str((event.get("tool_input") or {}).get("command") or "")
    if not command or "|" in command or ">" in command:
        return []

    base = Path(str(event.get("cwd") or root))
    targets: list[Path] = []
    for segment in SEGMENT_RE.split(command):
        try:
            words = shlex.split(segment)
        except ValueError:
            continue
        if not words or words[0] not in WHOLE_FILE_COMMANDS:
            continue
        targets += [base / w for w in words[1:] if not w.startswith("-")]
    return targets


def deny(path: Path, lines: int, budget: int, root: Path) -> None:
    """Emit the deny decision with the redirect the model should take instead."""
    try:
        shown = path.resolve().relative_to(root).as_posix()
    except ValueError:
        shown = str(path)

    reason = (
        f"{shown} is {lines} lines (gate: {budget}). A whole-file read this size goes to "
        "a read-only Explore subagent with a cheaper model (Agent, subagent_type "
        '"Explore", model "sonnet"): ask it the question and take the conclusion back, '
        "then re-read only the slice you will edit, with offset/limit or `sed -n`. A "
        "line number a subagent reports is a hint; re-read that slice before editing. "
        "Hotpath files CLAUDE.md names as read-in-full pass this gate; "
        "SS_READ_GATE_LINES retunes it."
    )
    payload = {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "deny",
            "permissionDecisionReason": reason,
        }
    }
    print(json.dumps(payload))


def main() -> None:
    try:
        event = json.load(sys.stdin)
        root = repo_root()
        budget = max_lines()

        tool = str(event.get("tool_name") or "")
        if tool == "Read":
            target = read_target(event)
            targets = [target] if target is not None else []
        else:
            targets = bash_targets(event, root)

        for path in targets:
            lines = oversized(path, budget)
            if lines:
                deny(path, lines, budget, root)
                break
    except Exception:
        pass
    sys.exit(0)


if __name__ == "__main__":
    main()
