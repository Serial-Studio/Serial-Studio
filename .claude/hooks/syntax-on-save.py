#!/usr/bin/env python3
"""Claude Code PostToolUse hook: compiler-truth check on a just-edited C++ file.

Fires after Edit/Write/MultiEdit. When the touched file is C++, it runs
scripts/syntax-check.py (clang -fsyntax-only via the newest compile database)
and feeds any diagnostics back to the agent, so type errors, missing includes
and broken constructs surface the moment they are introduced instead of in the
developer's next build. Sanctioned 2026-10-04: every AI batch-edit failure of
2026-10-03/04 was compiler-class and lint-invisible.

Mirrors verify-on-save.py's contract: never blocks, never raises; a clean
result (or a missing database) stays silent.
"""

import json
import subprocess
import sys
from pathlib import Path

CXX_SUFFIXES = {".cpp", ".cc", ".cxx", ".c++", ".h", ".hpp", ".hh", ".hxx"}
CHECK_TIMEOUT_S = 120
MAX_CONTEXT_CHARS = 6000


def emit_and_exit(context: str) -> None:
    """Print the PostToolUse advisory-context JSON, then exit 0 (non-blocking)."""
    payload = {
        "hookSpecificOutput": {
            "hookEventName": "PostToolUse",
            "additionalContext": context,
        }
    }
    print(json.dumps(payload))
    sys.exit(0)


def main() -> None:
    try:
        event = json.load(sys.stdin)
    except Exception:
        sys.exit(0)

    tool_input = event.get("tool_input") or {}
    raw_path = tool_input.get("file_path")
    if not raw_path:
        sys.exit(0)

    edited = Path(str(raw_path))
    if edited.suffix.lower() not in CXX_SUFFIXES:
        sys.exit(0)

    repo_root = Path(__file__).resolve().parents[2]
    checker = repo_root / "scripts" / "syntax-check.py"
    if not checker.is_file() or not edited.is_file():
        sys.exit(0)

    try:
        result = subprocess.run(
            [sys.executable, str(checker), str(edited)],
            capture_output=True,
            text=True,
            timeout=CHECK_TIMEOUT_S,
            cwd=str(repo_root),
        )
    except Exception:
        sys.exit(0)

    # Exit 0 is clean, exit 2 is no compile database: both stay silent so the
    # hook never nags on machines without a configured build.
    if result.returncode != 1:
        sys.exit(0)

    body = (result.stdout or "").strip()
    if not body:
        sys.exit(0)

    if len(body) > MAX_CONTEXT_CHARS:
        body = body[:MAX_CONTEXT_CHARS] + "\n... (truncated)"

    header = (
        f"syntax-check.py found COMPILER errors in {edited.name} (the file just "
        "edited). The developer's build will fail on these; fix them before "
        "continuing or claiming the edit is done.\n\n"
    )
    emit_and_exit(header + body)


if __name__ == "__main__":
    main()
