# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Behavior tests for scripts/ledger-verify.py: row parsing, the open/codified split, and the
ratchet against a baseline in a throwaway tree.

Run: pytest scripts/tests/test_ledger_verify.py
"""

import importlib.util
import json
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
SCRIPT = REPO / "scripts" / "ledger-verify.py"

HEADER = "# Ledger\n\n## Section\n\n| Mistake | Fix |\n|---------|-----|\n"
CODIFIED = "| Mutex in `FrameReader` | Recreate it. Codified: code-verify:spsc-lock, 2026-09-09. |\n"
NOT_YET = "| Auditing by enumerating sites | Enumerate from the source. Codified: not yet. |\n"
UNTAGGED = (
    "| `QMap::operator[]` on a lookup | `.find()` and validate \\| pipe kept. |\n"
)


def _load():
    spec = importlib.util.spec_from_file_location("ledger_verify", SCRIPT)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


lv = _load()


def test_rows_skip_header_and_separator():
    rows = lv.parse_rows(HEADER + CODIFIED + NOT_YET + UNTAGGED)
    assert [mistake[:5] for mistake, _ in rows] == ["Mutex", "Audit", "`QMap"]
    assert rows[2][1] == "`.find()` and validate | pipe kept."


def test_classification():
    rows = lv.parse_rows(HEADER + CODIFIED + NOT_YET + UNTAGGED)
    assert [lv.classify(fix) for _, fix in rows] == ["codified", "not-yet", "untagged"]
    assert lv.summarize(rows) == {
        "rows": 3,
        "open": 2,
        "codified": 1,
        "not-yet": 1,
        "untagged": 1,
    }


def test_real_ledger_parses_to_the_documented_shape():
    rows = lv.parse_rows(
        (REPO / "doc" / "claude" / "common-mistakes.md").read_text(encoding="utf-8")
    )
    summary = lv.summarize(rows)
    assert summary["rows"] >= 50
    assert summary["codified"] >= 5
    baseline = json.loads(
        (REPO / "scripts" / "ledger-baseline.json").read_text(encoding="utf-8")
    )
    assert summary["open"] <= baseline["open"], "the real ledger grew past its baseline"


def _tree(tmp_path, body):
    root = tmp_path / "tree"
    (root / "doc" / "claude").mkdir(parents=True)
    (root / "scripts").mkdir()
    (root / "doc" / "claude" / "common-mistakes.md").write_text(
        HEADER + body, encoding="utf-8"
    )
    return root


def _run(root, *args):
    return subprocess.run(
        [sys.executable, str(SCRIPT), "--root", str(root), *args],
        capture_output=True,
        text=True,
    )


def test_check_needs_a_baseline(tmp_path):
    root = _tree(tmp_path, CODIFIED + NOT_YET)
    assert _run(root, "--check").returncode == 2


def test_accept_then_check_then_growth(tmp_path):
    root = _tree(tmp_path, CODIFIED + NOT_YET + UNTAGGED)
    assert _run(root, "--accept").returncode == 0
    baseline = json.loads(
        (root / "scripts" / "ledger-baseline.json").read_text(encoding="utf-8")
    )
    assert baseline["open"] == 2
    assert _run(root, "--check").returncode == 0

    ledger = root / "doc" / "claude" / "common-mistakes.md"
    ledger.write_text(ledger.read_text(encoding="utf-8") + UNTAGGED, encoding="utf-8")
    grown = _run(root, "--check")
    assert grown.returncode == 1
    assert "grew from 2 to 3" in grown.stderr

    ledger.write_text(
        HEADER + CODIFIED + CODIFIED + UNTAGGED + UNTAGGED, encoding="utf-8"
    )
    assert _run(root, "--check").returncode == 0


def test_list_prints_open_rows(tmp_path):
    root = _tree(tmp_path, CODIFIED + NOT_YET + UNTAGGED)
    done = _run(root, "--list")
    assert "[not-yet] Auditing" in done.stdout
    assert "[untagged] `QMap" in done.stdout
    assert "Mutex" not in done.stdout


def test_missing_ledger_is_a_failure_to_run(tmp_path):
    assert _run(tmp_path, "--check").returncode == 2
