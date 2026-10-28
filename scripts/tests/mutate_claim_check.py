#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Mutation proof for .claude/hooks/claim-check.py (spec 0101, T10).

Each MANIFEST entry breaks exactly one guard in a throwaway copy of the hooks and names the
test in scripts/tests/test_hooks.py that must then fail. A guard whose mutation survives is a
guard no test pins. Exit 0 only when every mutation is killed.

Not covered, by design: the six CLAIM_PATTERNS entries cannot be killed one at a time. Every
claim fixture says "Done. All tests pass.", which patterns 1 and 5 both match, and no fixture
isolates patterns 2, 3, 4 or 6 on a warning path. The whole tuple is one guard below.
FENCE_RE is also unpinned: the fence's own backticks trip the backtick exclusion, so the
fenced-claim fixture stays silent with fence stripping removed. The semicolon mutation also
strips `$?`, because that fixture's command is `pytest tests; echo $?` and the glob
refusal would otherwise catch the `?`.

Run: python3 scripts/tests/mutate_claim_check.py [--list]
"""

import os
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
HOOKS = REPO / ".claude" / "hooks"
TARGET = "claim-check.py"
TEST_FILE = "scripts/tests/test_hooks.py"

_FORBIDDEN = 'r"[;|&#`$!]|\\bexit\\b|\\btrap\\b"'


def _forbidden(chars: str) -> str:
    """The FORBIDDEN_RE literal with its character class replaced by `chars`."""
    return f'r"[{chars}]|\\bexit\\b|\\btrap\\b"'


MANIFEST = (
    (
        "claim-patterns",
        "for pattern in CLAIM_PATTERNS:",
        "for pattern in ():",
        "test_claim_edit_no_verify",
    ),
    (
        "edit-trigger",
        "if verified or not (edited or shell_writes):",
        "if True:",
        "test_claim_edit_no_verify",
    ),
    (
        "verify-re",
        r"|\bpytest\b|\bctest\b",
        r"|\bpytestx\b|\bctest\b",
        "test_claim_plain_pytest",
    ),
    (
        "result-index",
        "_result_problem(block, extra)",
        "None",
        "test_claim_masked_interrupted",
    ),
    (
        "missing-result",
        'if use_id not in results:\n        return "no result was recorded"\n'
        "    return results[use_id] or creditable_command(command)",
        "return results.get(use_id) or creditable_command(command)",
        "test_claim_missing_result",
    ),
    ("is-error", 'if block.get("is_error"):', "if False:", "test_claim_errored_run"),
    (
        "marker-interrupted",
        'if source.get("interrupted"):',
        "if False:",
        "test_claim_masked_interrupted",
    ),
    (
        "marker-background",
        'if source.get("backgroundTaskId"):',
        "if False:",
        "test_claim_masked_background",
    ),
    (
        "marker-timeout",
        'if source.get("timedOutAfterMs"):',
        "if False:",
        "test_claim_masked_timeout",
    ),
    (
        "marker-reinterpreted",
        'if source.get("returnCodeInterpretation"):',
        "if False:",
        "test_claim_masked_reinterpreted",
    ),
    (
        "session-filter",
        "if not owner or owner == session_id:",
        "if True:",
        "test_claim_other_session",
    ),
    (
        "sidechain-filter",
        'if entry.get("type") != "assistant" or entry.get("isSidechain"):\n'
        "            continue\n        for block",
        'if entry.get("type") != "assistant":\n            continue\n        for block',
        "test_claim_subagent_run",
    ),
    ("refuse-pipe", _FORBIDDEN, _forbidden("&;#`$!"), "test_claim_piped_pytest"),
    (
        "refuse-semicolon",
        'replace("&&", "\\x00")',
        'replace("&&", "\\x00").replace(";", "\\x00").replace("$?", "")',
        "test_claim_semicolon",
    ),
    ("refuse-comment", _FORBIDDEN, _forbidden(";|&`$!"), "test_claim_comment"),
    (
        "refuse-or",
        _FORBIDDEN,
        'r"[;&#`$!]|(?<!\\|)\\|(?!\\|)|\\bexit\\b|\\btrap\\b"',
        "test_claim_or_true",
    ),
    ("refuse-glob", 'elif char in "*?[":', 'elif char in "":', "test_claim_glob"),
    (
        "refuse-no-run-flag",
        '"pytest": ("--collect-only", "--co", "--setup-only", "--setup-plan"),',
        '"pytest": (),',
        "test_claim_no_run_flag",
    ),
    (
        "exclude-backtick",
        "if _in_backticks(text, match.start(), match.end()):",
        "if False:",
        "test_claim_backtick_claim",
    ),
    (
        "exclude-blockquote",
        "if _in_blockquote(text, match.start()):",
        "if False:",
        "test_claim_blockquote_claim",
    ),
    (
        "exclude-question",
        "if key in seen or _ends_in_question(text, match.end()):",
        "if key in seen:",
        "test_claim_question_claim",
    ),
    (
        "stop-hook-active",
        'if event.get("stop_hook_active"):',
        "if False:",
        "test_claim_stop_hook_active",
    ),
    (
        "env-switch",
        'if os.environ.get("SS_CLAIM_CHECK", "on").lower() == "off":',
        "if False:",
        "test_claim_env_off",
    ),
    (
        "hidden-readings",
        "return [text, removed, spaced]",
        "return [text]",
        "test_claim_hidden_claim",
    ),
    (
        "final-message",
        "return final if isinstance(final, str) and final.strip() else None",
        "return None",
        "test_claim_final_message",
    ),
)


def judge(output: str, returncode: int) -> bool:
    """True when pytest output shows exactly one collected test that failed cleanly."""
    lines = [line for line in output.splitlines() if line.strip()]
    if returncode == 0 or not lines:
        return False
    summary = lines[-1]
    return bool(re.search(r"\b1 failed\b", summary)) and "error" not in summary.lower()


def run_mutation(hooks_copy: Path, original: str, entry: tuple) -> str:
    """Apply one manifest entry to the copy, run its test, return KILLED/SURVIVED/INVALID."""
    _, search, replace, test_name = entry
    if original.count(search) != 1 or search == replace:
        return "INVALID"
    (hooks_copy / TARGET).write_text(
        original.replace(search, replace), encoding="utf-8"
    )
    env = {**os.environ, "SS_HOOKS_UNDER_TEST": str(hooks_copy)}
    done = subprocess.run(
        [
            sys.executable,
            "-m",
            "pytest",
            TEST_FILE,
            "-q",
            "-k",
            test_name,
            "-p",
            "no:cacheprovider",
        ],
        cwd=str(REPO),
        env=env,
        capture_output=True,
        text=True,
        timeout=300,
    )
    return "KILLED" if judge(done.stdout, done.returncode) else "SURVIVED"


def main() -> int:
    """Run every mutation; exit 0 only when all are killed."""
    if "--list" in sys.argv[1:]:
        for name, _, _, test_name in MANIFEST:
            print(f"{name:24} {test_name}")
        return 0
    original = (HOOKS / TARGET).read_text(encoding="utf-8")
    killed = 0
    with tempfile.TemporaryDirectory() as tmp:
        copy = Path(tmp) / "hooks"
        shutil.copytree(HOOKS, copy, ignore=shutil.ignore_patterns("__pycache__"))
        for entry in MANIFEST:
            verdict = run_mutation(copy, original, entry)
            killed += verdict == "KILLED"
            print(f"{verdict:8} {entry[0]:24} {entry[3]}", flush=True)
    print(f"{killed} of {len(MANIFEST)} killed")
    return 0 if killed == len(MANIFEST) else 1


if __name__ == "__main__":
    sys.exit(main())
