# SPDX-FileCopyrightText: 2026 Alex Spataru <https://serial-studio.com/>
# SPDX-License-Identifier: GPL-3.0-or-later
"""Tests for claim-verify's enforcer markers (spec 0095 M5): a rule that claims a mechanism with
`Enforced:` or `Codified:` must name one that exists, and a marker naming nothing real fails.
"""

from __future__ import annotations

import importlib.util
import sys
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1]


def _load():
    sys.path.insert(0, str(SCRIPTS))
    spec = importlib.util.spec_from_file_location(
        "claim_verify", SCRIPTS / "claim-verify.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules["claim_verify"] = module
    spec.loader.exec_module(module)
    return module


CV = _load()
ENFORCERS = CV.EnforcerIndex(CV.build_source_index())
DOC = Path(__file__)


def _findings(text: str) -> list:
    lines = text.splitlines()
    return CV.check_enforcers(DOC, lines, [True] * len(lines), ENFORCERS)


def test_real_mechanisms_of_every_kind_resolve():
    text = (
        "Enforced: code-verify:bus-on-hotpath, ctest:tst_cached_flags, "
        "anchor:cached-hotpath-flags, script:scripts/code-verify.py#--singleton-census, "
        "hook:canary-check.py, compile:DSP::RingCapacity."
    )
    assert _findings(text) == []


def test_a_missing_mechanism_is_an_error():
    found = _findings("Enforced: ctest:tst_no_such_suite, anchor:no-such-anchor")
    assert [f.kind for f in found] == ["enforcer-missing", "enforcer-missing"]
    assert all(f.error for f in found)


def test_codified_ledger_tags_are_checked_too():
    assert len(_findings("Codified: code-verify:no-such-rule, 2026-10-07.")) == 1


def test_debt_and_not_yet_name_nothing_to_resolve():
    assert _findings("Codified: not yet.") == []
    assert _findings("Debt: ctest:tst_planned_later") == []


def test_wrapped_marker_items_are_checked():
    text = (
        "- **Rule.** Something binding. Enforced:\n"
        "  compile:DSP::RingCapacity, ctest:tst_no_such_suite. Detail: a doc.\n"
        "- **Next rule.** ctest:tst_also_missing is prose, not a marker."
    )
    found = _findings(text)
    assert [(f.line, f.kind) for f in found] == [(2, "enforcer-missing")]


def test_fenced_regions_are_skipped():
    lines = ["Enforced: ctest:tst_no_such_suite"]
    assert CV.check_enforcers(DOC, lines, [False], ENFORCERS) == []


def _sections(
    tmp_path: Path, heading: str, target_text: str = "# Doc\n\n## Real Heading.\n"
):
    (tmp_path / "target.md").write_text(target_text, encoding="utf-8")
    doc = tmp_path / "doc.md"
    lines = [f'See [target](target.md) "{heading}" for detail.']
    return CV.check_sections(doc, lines, [True])


def test_section_pointing_at_an_existing_heading_passes(tmp_path):
    assert _sections(tmp_path, "Real Heading") == []


def test_section_pointing_at_a_renamed_heading_is_reported(tmp_path):
    found = _sections(tmp_path, "Old Heading")
    assert [f.kind for f in found] == ["section-missing"]
    assert found[0].error


def test_section_matches_heading_text_before_a_separator(tmp_path):
    body = "# Doc\n\n## Real Heading — with a subtitle (spec 1)\n"
    assert _sections(tmp_path, "Real Heading", body) == []
    assert [f.kind for f in _sections(tmp_path, "real heading", body)] == [
        "section-missing"
    ]


def test_section_matches_heading_without_its_trailing_parenthetical(tmp_path):
    body = "# Doc\n\n## Real Heading (spec 0099)\n"
    assert _sections(tmp_path, "Real Heading", body) == []
    assert _sections(tmp_path, "Real Heading (spec 0099)", body) == []


def test_section_matches_a_bold_run_in_label(tmp_path):
    body = "# Doc\n\n**Real Heading (spec 0059).** The paragraph that is the section.\n"
    assert _sections(tmp_path, "Real Heading", body) == []
    assert [f.kind for f in _sections(tmp_path, "Other", body)] == ["section-missing"]


def test_section_matches_a_heading_wrapped_in_backticks(tmp_path):
    assert _sections(tmp_path, "core/", "# Doc\n\n## `core/`\n") == []


def test_section_ignores_headings_inside_fences(tmp_path):
    found = _sections(tmp_path, "Fenced", "# Doc\n```\n## Fenced\n```\n")
    assert [f.kind for f in found] == ["section-missing"]


def test_unknown_or_ambiguous_backticked_names_are_skipped(tmp_path):
    doc = tmp_path / "doc.md"
    lines = ['`no-such-file-anywhere.md` "X"', '`spec.md` "X"']
    assert CV.check_sections(doc, lines, [True, True]) == []


def _budget_tree(tmp_path: Path, claude_words: int, doc_words: int) -> Path:
    (tmp_path / "doc" / "claude" / "specs").mkdir(parents=True, exist_ok=True)
    (tmp_path / "doc" / "claude" / "a.md").write_text(
        "w " * doc_words, encoding="utf-8"
    )
    (tmp_path / "doc" / "claude" / "specs" / "s.md").write_text(
        "w " * 50, encoding="utf-8"
    )
    body = (
        "[a](doc/claude/a.md) [a again](doc/claude/a.md#x) [s](doc/claude/specs/s.md) "
    )
    (tmp_path / "CLAUDE.md").write_text(
        body + "w " * (claude_words - 4), encoding="utf-8"
    )
    return tmp_path


def test_context_budget_counts_distinct_linked_docs_without_specs(tmp_path):
    root = _budget_tree(tmp_path, 100, 30)
    assert CV._collect_context_budget(root) == {
        "claude_md_words": 100,
        "one_hop_words": 30,
    }


def test_context_budget_accept_then_grow_shrink(tmp_path, capsys):
    root = _budget_tree(tmp_path, 100, 30)
    baseline = tmp_path / "budget.json"
    assert CV._run_context_budget(root, False, True, baseline) == 0
    assert baseline.read_text(encoding="utf-8").endswith("}\n")
    assert CV._run_context_budget(root, True, False, baseline) == 0

    _budget_tree(tmp_path, 120, 30)
    assert CV._run_context_budget(root, True, False, baseline) == 1
    assert "extract to a doc/claude sub-doc" in capsys.readouterr().err

    _budget_tree(tmp_path, 80, 90)
    assert CV._run_context_budget(root, True, False, baseline) == 0
    out = capsys.readouterr().out
    assert "re-baseline" in out and "+60" in out


def test_context_budget_missing_baseline_exits_two(tmp_path):
    root = _budget_tree(tmp_path, 100, 30)
    assert CV._run_context_budget(root, True, False, tmp_path / "none.json") == 2
