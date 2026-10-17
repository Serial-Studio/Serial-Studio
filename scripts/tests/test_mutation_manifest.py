# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Static checks on the claim-check mutation manifest (spec 0101, T10).

Fast on purpose: it never runs a mutation, it only proves each entry can still apply and
points at a real test. Run the mutations with scripts/tests/mutate_claim_check.py.

Run: pytest scripts/tests/test_mutation_manifest.py
"""

import importlib.util
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
HOOK = (REPO / ".claude" / "hooks" / "claim-check.py").read_text(encoding="utf-8")
TESTS = (HERE / "test_hooks.py").read_text(encoding="utf-8")


def _load_manifest():
    """Import mutate_claim_check.py by path and return its MANIFEST."""
    spec = importlib.util.spec_from_file_location(
        "mutate_claim_check", HERE / "mutate_claim_check.py"
    )
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module.MANIFEST


MANIFEST = _load_manifest()


def test_manifest_not_empty():
    """The manifest carries entries of the documented four-field shape."""
    assert MANIFEST
    assert all(len(entry) == 4 for entry in MANIFEST)


def test_search_occurs_exactly_once():
    """Every search string appears exactly once in claim-check.py."""
    for name, search, _, _ in MANIFEST:
        assert HOOK.count(search) == 1, name


def test_replace_differs():
    """Every replacement changes the source."""
    for name, search, replace, _ in MANIFEST:
        assert replace != search, name


def test_test_names_exist():
    """Every named test is a function defined in test_hooks.py."""
    defined = set(re.findall(r"^def (test_\w+)\(", TESTS, re.MULTILINE))
    for name, _, _, test_name in MANIFEST:
        assert test_name in defined, f"{name}: {test_name}"


def test_guard_names_unique():
    """Guard names do not repeat."""
    names = [entry[0] for entry in MANIFEST]
    assert len(names) == len(set(names))
