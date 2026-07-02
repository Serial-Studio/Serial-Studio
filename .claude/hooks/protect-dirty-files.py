#!/usr/bin/env python3
"""Claude Code hook: guard files the user modified before the session began.

Mechanizes the trust-contract rule "never touch files outside your own edits"
(CLAUDE.md). One script serves three events, dispatched on hook_event_name:

  SessionStart - snapshot `git status --porcelain` into a per-session state
                 file; purge state files older than PURGE_AGE_DAYS.
  PreToolUse   - an Edit/Write/MultiEdit targeting a snapshot file that has
                 not yet been user-approved answers permissionDecision "ask",
                 so the user confirms before pre-existing work is touched.
                 Lazily snapshots when SessionStart never fired (resume with
                 a cleared temp dir): fail-safe, worst case one extra prompt.
  PostToolUse  - records the file as approved (the tool only runs after the
                 user allowed it), so later edits to it never re-prompt.

Design choices specific to this repo:
  - Fail-open everywhere: any internal error exits 0 silently. The hook is a
    mechanical backstop for CLAUDE.md's behavioral rule, and it also binds
    subagents, which do not reliably internalize the trust contract.
  - Untracked entries are guarded too (they are user work-in-progress that a
    Write would clobber); untracked directories match by path prefix.
  - State lives in the OS temp dir keyed by session_id, so the guard itself
    never dirties the repo tree or git status.
"""

import json
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

GIT_TIMEOUT_S = 10
PURGE_AGE_DAYS = 7
STATE_DIR = Path(tempfile.gettempdir()) / "claude-ss-dirty-guard"


def repo_root() -> Path:
    """Resolve the repository root from this file's location (.claude/hooks/)."""
    return Path(__file__).resolve().parents[2]


def state_path(session_id: str) -> Path:
    """Map a session id to its state file, sanitized for the filesystem."""
    safe = "".join(c if c.isalnum() or c in "-_" else "_" for c in session_id)
    return STATE_DIR / f"{safe}.json"


def snapshot_dirty(root: Path) -> list[str]:
    """List working-tree paths already modified/untracked, per git porcelain."""
    result = subprocess.run(
        ["git", "status", "--porcelain"],
        capture_output=True,
        text=True,
        timeout=GIT_TIMEOUT_S,
        cwd=str(root),
    )
    if result.returncode != 0:
        return []

    entries: list[str] = []
    for line in result.stdout.splitlines():
        if len(line) < 4:
            continue
        path = line[3:]
        if " -> " in path:
            path = path.split(" -> ", 1)[1]
        entries.append(path.strip('"'))
    return entries


def write_state(path: Path, state: dict) -> None:
    """Persist the snapshot/approval state for this session."""
    STATE_DIR.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(state), encoding="utf-8")


def ensure_state(event: dict, root: Path) -> dict:
    """Load this session's state, lazily snapshotting if SessionStart never ran."""
    path = state_path(str(event.get("session_id") or "default"))
    if path.is_file():
        state = json.loads(path.read_text(encoding="utf-8"))
        state.setdefault("dirty", [])
        state.setdefault("approved", [])
        return state

    state = {"dirty": snapshot_dirty(root), "approved": []}
    write_state(path, state)
    return state


def target_rel(event: dict, root: Path) -> str | None:
    """Return the tool's target path relative to the repo, or None if outside."""
    raw = (event.get("tool_input") or {}).get("file_path")
    if not raw:
        return None

    rel = os.path.relpath(os.path.abspath(str(raw)), str(root))
    rel = rel.replace(os.sep, "/")
    if rel.startswith(".."):
        return None
    return rel


def is_dirty(rel: str, dirty: list[str]) -> bool:
    """Check membership in the snapshot; directory entries match by prefix."""
    for entry in dirty:
        if entry.endswith("/"):
            if rel.startswith(entry):
                return True
        elif rel == entry:
            return True
    return False


def handle_session_start(event: dict, root: Path) -> None:
    """Take a fresh snapshot for the session and purge stale state files."""
    cutoff = time.time() - PURGE_AGE_DAYS * 86400
    if STATE_DIR.is_dir():
        for entry in STATE_DIR.glob("*.json"):
            if entry.stat().st_mtime < cutoff:
                entry.unlink()

    path = state_path(str(event.get("session_id") or "default"))
    write_state(path, {"dirty": snapshot_dirty(root), "approved": []})


def handle_pre_tool_use(event: dict, root: Path) -> None:
    """Ask the user before the first touch of a file that predates the session."""
    rel = target_rel(event, root)
    if rel is None:
        return

    state = ensure_state(event, root)
    if rel in state["approved"] or not is_dirty(rel, state["dirty"]):
        return

    payload = {
        "hookSpecificOutput": {
            "hookEventName": "PreToolUse",
            "permissionDecision": "ask",
            "permissionDecisionReason": (
                f"{rel} was already modified before this session started; it may be "
                "the user's in-progress work (trust contract: never touch files "
                "outside your own edits). Confirm before editing it."
            ),
        }
    }
    print(json.dumps(payload))


def handle_post_tool_use(event: dict, root: Path) -> None:
    """Record a user-approved edit so the same file does not re-prompt."""
    rel = target_rel(event, root)
    if rel is None:
        return

    state = ensure_state(event, root)
    if rel in state["approved"] or not is_dirty(rel, state["dirty"]):
        return

    state["approved"].append(rel)
    write_state(state_path(str(event.get("session_id") or "default")), state)


def main() -> None:
    try:
        event = json.load(sys.stdin)
        root = repo_root()
        handlers = {
            "SessionStart": handle_session_start,
            "PreToolUse": handle_pre_tool_use,
            "PostToolUse": handle_post_tool_use,
        }
        handler = handlers.get(str(event.get("hook_event_name")))
        if handler is not None:
            handler(event, root)
    except Exception:
        pass
    sys.exit(0)


if __name__ == "__main__":
    main()
