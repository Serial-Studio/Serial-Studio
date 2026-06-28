#!/usr/bin/env python3
"""Claude Code PostToolUse hook: lint a just-edited source file.

Fires after Edit/Write/MultiEdit. When the touched file is C++ or QML, it runs
the repo's own structural linter (scripts/code-verify.py --check --no-report on
that single file) and feeds any *blocking errors* back to the agent as advisory
context, so style regressions surface the moment they are introduced instead of
at commit time.

Design choices specific to this repo:
  - Only blocking errors are surfaced (code-verify exits non-zero). Advisory
    findings are repo baseline debt (see CLAUDE.md); echoing them on every save
    would be noise, so a clean-or-advisory-only result stays silent.
  - --no-report keeps the hook from clobbering a full-tree .code-report the
    developer may be reading.
  - The hook never blocks and never raises: any internal failure exits 0
    silently so routine editing is never disrupted.
"""

import json
import subprocess
import sys
from pathlib import Path

SOURCE_SUFFIXES = {".cpp", ".cc", ".cxx", ".c++", ".h", ".hpp", ".hh", ".hxx", ".qml"}
VERIFY_TIMEOUT_S = 45
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
    if edited.suffix.lower() not in SOURCE_SUFFIXES:
        sys.exit(0)

    repo_root = Path(__file__).resolve().parents[2]
    verify = repo_root / "scripts" / "code-verify.py"
    if not verify.is_file() or not edited.is_file():
        sys.exit(0)

    try:
        result = subprocess.run(
            [sys.executable, str(verify), "--check", "--no-report", str(edited)],
            capture_output=True,
            text=True,
            timeout=VERIFY_TIMEOUT_S,
            cwd=str(repo_root),
        )
    except Exception:
        sys.exit(0)

    # Exit 0 from code-verify means clean or advisory-only: stay silent so
    # routine edits to files carrying baseline advisory debt do not spam.
    if result.returncode == 0:
        sys.exit(0)

    violations = (result.stdout or "").strip()
    summary = (result.stderr or "").strip()
    if not violations and not summary:
        sys.exit(0)

    body = violations if not summary else f"{violations}\n\n{summary}"
    if len(body) > MAX_CONTEXT_CHARS:
        body = body[:MAX_CONTEXT_CHARS] + "\n... (truncated)"

    header = (
        f"code-verify.py flagged blocking errors in {edited.name} "
        "(the file just edited). These fail CI; resolve them before continuing. "
        "Advisory lines are baseline debt, but new code should clear them too.\n\n"
    )
    emit_and_exit(header + body)


if __name__ == "__main__":
    main()
