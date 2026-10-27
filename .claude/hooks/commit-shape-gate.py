#!/usr/bin/env python3
"""Claude Code PreToolUse hook: judge the shape of a `git commit` before it runs.

Companion to commit-sanitize-gate.py, which checks that sanitize ran; this
checks the commit's shape with scripts/commit-shape.py (size, isolated
surfaces, spec-born, fix(ci) run citation) and additionally requires the
`Co-Authored-By: Claude` trailer, because every commit this hook sees is one
Claude issued. A finding blocks the commit (exit 2) with the engine's own
sentences, which name the fix.

Design choices specific to this repo:
  - The engine is loaded from the project's scripts/ dir (CLAUDE_PROJECT_DIR
    honored for tests), so the hook and the CLI can never disagree.
  - A command that stages and commits in one step is refused: a PreToolUse
    hook sees only the index that exists before the command runs.
  - Fail-open on any internal error (missing engine, git failure) and silent
    for non-commit commands: only a genuinely mis-shaped commit is blocked.
"""

import importlib.util
import json
import os
import re
import sys
from pathlib import Path

COMMIT_RE = re.compile(r"\bgit(\s+-\S+)*\s+commit\b")
STAGE_RE = re.compile(r"\bgit(\s+-\S+)*\s+(add|rm|mv|stage)\b")
STAGING_FINDING = (
    "staging: this command stages and commits in one step, so the gate would judge an "
    "index that does not exist yet. Run the `git add` in one command and the `git commit` "
    "in the next"
)


def repo_root() -> Path:
    """Resolve the repository root: CLAUDE_PROJECT_DIR when set, else this file's location."""
    override = os.environ.get("CLAUDE_PROJECT_DIR", "")
    if override:
        return Path(override).resolve()
    return Path(__file__).resolve().parents[2]


def load_engine(root: Path):
    """scripts/commit-shape.py as a module, or None when the project has none."""
    source = root / "scripts" / "commit-shape.py"
    if not source.is_file():
        return None
    spec = importlib.util.spec_from_file_location("commit_shape", source)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> None:
    try:
        event = json.load(sys.stdin)
        command = str((event.get("tool_input") or {}).get("command") or "")
        if not COMMIT_RE.search(command):
            sys.exit(0)

        root = repo_root()
        engine = load_engine(root)
        if engine is None:
            sys.exit(0)

        findings = [STAGING_FINDING] if STAGE_RE.search(command) else []
        findings += engine.judge_command(root, command, require_coauthor=True)
        if not findings:
            sys.exit(0)

        sys.stderr.write(
            "Commit blocked by the commit-shape gate:\n"
            + "".join(f"  - {finding}\n" for finding in findings)
            + "(CLAUDE.md 'Trust Contract': commit shape. `python3 scripts/commit-shape.py "
            "--command '<the command>' --require-coauthor` reproduces this.)\n"
        )
        sys.exit(2)
    except SystemExit:
        raise
    except Exception:
        sys.exit(0)


if __name__ == "__main__":
    main()
