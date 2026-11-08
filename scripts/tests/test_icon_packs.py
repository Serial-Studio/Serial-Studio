# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""Tests for icon-packs.py's gate (spec 0115 AC1, AC11): a pack file in a fixed category, an id
the house set lacks, a tier outside the ladder, a pack under the coverage gate and a mapping row
without a house icon all fail; a complete pack passes, and the composer badges a base.
"""

from __future__ import annotations

import importlib.util
import json
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

import pytest

SCRIPTS = Path(__file__).resolve().parents[1]

SVG = (
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48">'
    '<path fill="#123456" d="M0 0h48v48H0z"/></svg>'
)


def _load():
    spec = importlib.util.spec_from_file_location(
        "icon_packs", SCRIPTS / "icon-packs.py"
    )
    module = importlib.util.module_from_spec(spec)
    sys.modules["icon_packs"] = module
    spec.loader.exec_module(module)
    return module


IP = _load()


def _write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


@pytest.fixture
def tree(tmp_path, monkeypatch):
    """A two-icon house set, a matching vocabulary and mapping, and an empty pack root."""
    icons = tmp_path / "icons"
    packs = tmp_path / "iconpacks"
    _write(icons / "widgets" / "32" / "gauge.svg", SVG)
    _write(icons / "editor" / "32" / "add-gauge.svg", SVG)
    _write(icons / "system" / "16" / "missing.svg", SVG)
    _write(
        tmp_path / "icon-bases.txt",
        "# base|depicts|terms|ids\n"
        "gauge|round gauge dial|speedometer;gauge|editor/add-gauge+plus;widgets/gauge\n",
    )
    _write(
        tmp_path / "icons8-map.csv",
        "base,status,fluent_id,fluent_name,fluent_common,outline_id,outline_name,glass_id,"
        "glass_name,deco_id,deco_name,note,ids\n"
        "gauge,found,abc123,Speedometer,speedometer,?,?,NONE,NONE,NONE,NONE,,"
        "editor/add-gauge+plus;widgets/gauge\n",
    )
    packs.mkdir()
    monkeypatch.setattr(IP, "ICONS", icons)
    monkeypatch.setattr(IP, "PACKS", packs)
    monkeypatch.setattr(IP, "QRC", packs / "iconpacks.qrc")
    monkeypatch.setattr(IP, "BASES", tmp_path / "icon-bases.txt")
    monkeypatch.setattr(IP, "MAPPING", tmp_path / "icons8-map.csv")
    return tmp_path


def _pack(root: Path, name: str, mono: bool = False) -> Path:
    folder = root / "iconpacks" / name
    _write(
        folder / "pack.json",
        json.dumps(
            {
                "id": name,
                "title": name.title(),
                "style": "fluent",
                "mono": mono,
                "translations": {},
            }
        ),
    )
    return folder


def _errors(gate: float = 0.85) -> list[str]:
    errors: list[str] = []
    house = IP.house_ids()
    for folder in IP.pack_dirs():
        IP.check_pack(folder, house, gate, errors)
    IP.check_mapping(house, errors)
    return errors


def test_complete_pack_passes(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    _write(folder / "editor" / "48" / "add-gauge.svg", SVG)
    assert _errors() == []


def test_fixed_category_is_refused(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    _write(folder / "editor" / "48" / "add-gauge.svg", SVG)
    _write(folder / "window" / "32" / "close.svg", SVG)
    assert any("fixed category" in e for e in _errors())


def test_id_outside_house_set_is_refused(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    _write(folder / "editor" / "48" / "add-gauge.svg", SVG)
    _write(folder / "widgets" / "48" / "unicorn.svg", SVG)
    assert any("no house icon widgets/unicorn" in e for e in _errors())


def test_tier_outside_ladder_is_refused(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "20" / "gauge.svg", SVG)
    _write(folder / "editor" / "48" / "add-gauge.svg", SVG)
    assert any("tier not in" in e for e in _errors())


def test_pack_under_the_gate_is_refused(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    assert any("under the gate" in e for e in _errors())
    assert _errors(gate=0.5) == []


def test_mapping_row_without_a_house_icon_is_refused(tree):
    _write(
        tree / "icon-bases.txt",
        "gauge|round gauge dial|speedometer|editor/add-gauge+plus;widgets/gauge;"
        "widgets/ghost\n",
    )
    assert any("widgets/ghost, which the house tree lacks" in e for e in _errors())


def test_house_icon_without_a_row_is_refused(tree):
    _write(tree / "icons" / "widgets" / "32" / "compass.svg", SVG)
    assert any("widgets/compass has no metaphor row" in e for e in _errors())


def test_compose_badges_the_base(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    IP.cmd_compose(type("A", (), {"pack": "fluent"})())
    composed = (folder / "editor" / "48" / "add-gauge.svg").read_text(encoding="utf-8")
    assert 'data-ss-badge="plus"' in composed
    assert 'viewBox="0 0 48 48"' in composed
    assert _errors() == []


def test_qrc_lists_every_pack_file_and_skips_scratch(tree):
    folder = _pack(tree, "fluent")
    _write(folder / "widgets" / "48" / "gauge.svg", SVG)
    _write(folder / IP.BASES_DIR / "gauge.svg", SVG)
    IP.cmd_qrc(None)
    text = IP.QRC.read_text(encoding="utf-8")
    assert "<file>fluent/widgets/48/gauge.svg</file>" in text
    assert "<file>fluent/pack.json</file>" in text
    assert "_bases" not in text


PROVIDER = SCRIPTS.parent / "core" / "Ui" / "Misc" / "IconPackImageProvider.cpp"

COMPOSITE = (
    '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48"><defs><mask id="ss-knockout">'
    '<rect x="0" y="0" width="48" height="48" fill="white"/><circle cx="37" cy="37" r="12" '
    'fill="black"/></mask></defs><g mask="url(#ss-knockout)"><path d="M4 4h20v20H4z"/>'
    '<path fill="#1A1A1A" stroke="#000" style="fill:#222;stroke:none" d="M1 1h1"/></g>'
    '<g data-ss-badge="plus"><rect fill="#000000" x="30" y="36" width="14" height="2"/></g></svg>'
)


def _cpp_recolour():
    """The provider's two regexes and replacement templates, read from the C++ source so this
    test fails when the group numbers in the source drift (2026-11-07: a \\3 for a \\4 broke
    every tinted composite)."""
    text = PROVIDER.read_text(encoding="utf-8")
    patterns = re.findall(r'QStringLiteral\(\s*R"re\((.*?)\)re"\)', text, re.S)
    assert len(patterns) == 2, patterns
    attr_pat, style_pat = patterns
    replacements = re.findall(
        r"text\.replace\(k(Attribute|Style),\s*(.*?)\);", text, re.S
    )
    assert [name for name, _ in replacements] == ["Attribute", "Style"]

    def template(expr: str) -> str:
        groups = re.findall(r'QStringLiteral\("\\\\(\d)"\)', expr)
        return groups

    attr_groups = template(replacements[0][1])
    style_groups = template(replacements[1][1])

    def recolour(svg: str, tint: str) -> str:
        hexc = "#" + tint

        def attr_sub(m):
            return m.group(int(attr_groups[0])) + hexc + m.group(int(attr_groups[1]))

        def style_sub(m):
            return m.group(int(style_groups[0])) + hexc

        svg = re.sub(attr_pat, attr_sub, svg)
        svg = re.sub(style_pat, style_sub, svg)
        return svg

    return recolour


def test_provider_recolour_keeps_composites_well_formed():
    recolour = _cpp_recolour()
    out = recolour(COMPOSITE, "e6e8ea")
    ET.fromstring(out)
    assert 'fill="#e6e8ea"' in out and 'stroke="#e6e8ea"' in out
    assert "fill:#e6e8ea" in out
    assert 'fill="white"' in out and 'fill="black"' in out
    assert "#e6e8ea#" not in out
