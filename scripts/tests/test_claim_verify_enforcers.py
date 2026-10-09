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
