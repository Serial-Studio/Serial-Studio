# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""Tests for registry-verify's device-declaration gate (spec 0111 AC9): a property with no wire
status, a golden line with a key outside the registry, an envelope name that is not a Keys::
constant and a key that carries code all fail; the checked-in manifests pass."""

from __future__ import annotations

import copy
import importlib.util
import json
import re
import sys
from pathlib import Path

SCRIPTS = Path(__file__).resolve().parents[1]
FIXTURES = Path(__file__).resolve().parent / "wire-fixtures"


def _load():
    sys.path.insert(0, str(SCRIPTS))
    spec = importlib.util.spec_from_file_location(
        "registry_verify", SCRIPTS / "registry-verify.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules["registry_verify"] = module
    spec.loader.exec_module(module)
    return module


RV = _load()


def _inputs():
    manifest = RV.dataset_manifest()
    declaration = json.loads(RV.DECLARATION_MANIFEST.read_text(encoding="utf-8"))
    text = RV.FRAME_H.read_text(encoding="utf-8") + RV.FRAME_KEYS_H.read_text(
        encoding="utf-8"
    )
    keys = set(re.findall(r"inline constexpr KeyView (\w+)\(", text))
    return manifest, declaration, keys, RV.group_widget_ids()


def _check(manifest, declaration, keys, widgets, golden=FIXTURES / "clean"):
    errors: list[str] = []
    RV.check_wire_vocabulary(errors, manifest, declaration, keys, widgets, golden)
    return errors


def test_checked_in_manifests_and_a_clean_golden_line_pass():
    assert _check(*_inputs()) == []


def test_the_real_golden_lines_pass():
    assert _check(*_inputs(), golden=RV.GOLDEN_DECLARATIONS) == []


def test_a_property_without_wire_fails():
    manifest, declaration, keys, widgets = _inputs()
    manifest = copy.deepcopy(manifest)
    del manifest["properties"][0]["wire"]
    errors = _check(manifest, declaration, keys, widgets)
    assert any("needs exactly one of include or exclude" in e for e in errors)


def test_a_golden_line_with_a_key_outside_the_registry_fails():
    errors = _check(*_inputs(), golden=FIXTURES / "bad-key")
    assert errors == [
        "bad-key.decl:1: 'groups[0].datasets[0].transformCode' is not in the wire "
        "vocabulary"
    ]


def test_an_envelope_name_that_is_not_a_keys_constant_fails():
    manifest, declaration, keys, widgets = _inputs()
    declaration = copy.deepcopy(declaration)
    declaration["envelope"]["group"].append("NoSuchKey")
    errors = _check(manifest, declaration, keys, widgets)
    assert any("envelope.group names unknown Keys::NoSuchKey" in e for e in errors)


def test_code_paths_and_connections_never_ride_the_wire():
    manifest, declaration, keys, widgets = _inputs()
    declaration = copy.deepcopy(declaration)
    declaration["envelope"]["source"].append("SourceConn")
    declaration["groupWidgets"].append("painter")
    manifest = copy.deepcopy(manifest)
    bands = next(s for s in manifest["subEntities"] if s["field"] == "alarmBands")
    bands["wire"]["keys"].append("Sound")
    errors = _check(manifest, declaration, keys, widgets)
    assert any("puts Keys::SourceConn on the wire" in e for e in errors)
    assert any("group widget 'painter'" in e for e in errors)
    assert any("puts Keys::Sound on the wire" in e for e in errors)
