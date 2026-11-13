# SPDX-FileCopyrightText: 2020-2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
The test and translation tooling fixes of spec 0120.

- A test whose service is missing skips on a developer's machine but fails on a CI leg that
  started the service, where a skip used to turn "the broker never came up" into a green run.
- The translation update removes obsolete strings, as its own success message always claimed;
  without the flag each .ts file kept about 2,050 dead entries.

Run: pytest scripts/tests/test_test_tooling.py
"""

import ast
import importlib.util
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]


def _required_services():
    path = REPO / "tests" / "utils" / "required_services.py"
    spec = importlib.util.spec_from_file_location("required_services", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@pytest.mark.parametrize("variable", ["REQUIRE_BROKER", "REQUIRE_OPCUA_SIM"])
def test_a_missing_service_skips_when_this_run_did_not_promise_it(
    monkeypatch, variable
):
    services = _required_services()
    monkeypatch.delenv(getattr(services, variable), raising=False)
    with pytest.raises(pytest.skip.Exception):
        services.missing_service("service down", getattr(services, variable))


@pytest.mark.parametrize("variable", ["REQUIRE_BROKER", "REQUIRE_OPCUA_SIM"])
def test_a_missing_service_fails_on_a_leg_that_started_it(monkeypatch, variable):
    services = _required_services()
    monkeypatch.setenv(getattr(services, variable), "1")
    with pytest.raises(pytest.fail.Exception):
        services.missing_service("service down", getattr(services, variable))


def test_the_translation_update_drops_obsolete_strings():
    source = (REPO / "app" / "translations" / "translation_manager.py").read_text()
    tree = ast.parse(source)
    run_lupdate = next(
        node
        for node in ast.walk(tree)
        if isinstance(node, ast.FunctionDef) and node.name == "run_lupdate"
    )
    lists = [
        [el.value for el in node.elts if isinstance(el, ast.Constant)]
        for node in ast.walk(run_lupdate)
        if isinstance(node, ast.List)
    ]
    commands = [items for items in lists if "lupdate" in items]
    assert commands, "run_lupdate no longer builds an lupdate command"
    assert all("-no-obsolete" in items for items in commands)
