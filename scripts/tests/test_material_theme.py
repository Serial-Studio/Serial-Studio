# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Data checks for the material theme format (spec 0113), run without the application: the
bundled material themes carry the complete colour table and only names they define, their
lettering meets the contrast bar, the two Fluent themes carry none of the new sections, the
Default names only what it defines (its 2026-11-07 shading amended spec 0113's "Default
unchanged" non-goal at the maintainer's request), and the grain tile matches its generator.

Run: pytest scripts/tests/test_material_theme.py
"""

import json
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
THEMES = REPO / "app" / "rcc" / "themes"
QRC = REPO / "app" / "rcc" / "rcc.qrc"
GENERATOR = REPO / "scripts" / "generate-material-tiles.py"

NEW_SECTIONS = ("fonts", "materials", "surfaces", "styles", "widgets")
RECIPES = {"flat", "gradient", "brushed-metal", "glass"}
LOOKS = {"led": {"lamp"}, "output": {"hardware", "plain"}}
OLDER_THEMES = ("fluent-light", "fluent-dark")
BUNDLED_THEMES = ("millennium", "millennium-dark", "starliner")
MATERIAL_THEMES = ("default",) + BUNDLED_THEMES

# Text colour over the surface it is drawn on (spec 0113 R13: at least 4.5 to 1)
TEXT_PAIRS = (
    ("text", "window"),
    ("text", "base"),
    ("window_text", "window"),
    ("button_text", "button"),
    ("widget_text", "widget_window"),
    ("widget_text", "widget_base"),
    ("pane_section_label", "pane_background"),
    ("pane_caption_foreground", "pane_caption_bg_top"),
    ("pane_caption_foreground", "pane_caption_bg_bottom"),
    ("toolbar_text", "toolbar_top"),
    ("toolbar_text", "toolbar_bottom"),
    ("taskbar_text", "taskbar_top"),
    ("taskbar_text", "taskbar_bottom"),
    ("table_text", "table_cell_bg"),
    ("table_fg_header", "table_bg_header_top"),
    ("window_caption_active_text", "window_caption_active_top"),
    ("window_caption_active_text", "window_caption_active_bottom"),
    ("window_caption_inactive_text", "window_caption_inactive_top"),
    ("tooltip_text", "tooltip_base"),
    ("console_text", "console_base"),
    ("highlighted_text", "highlight"),
    ("placeholder_text", "base"),
)


def load(name: str) -> dict:
    return json.loads((THEMES / f"{name}.json").read_text(encoding="utf-8"))


def luminance(color: str) -> float:
    digits = color.lstrip("#")[-6:]
    channels = [int(digits[i : i + 2], 16) / 255 for i in (0, 2, 4)]
    linear = [
        c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4 for c in channels
    ]
    return 0.2126 * linear[0] + 0.7152 * linear[1] + 0.0722 * linear[2]


def contrast(first: str, second: str) -> float:
    high, low = sorted((luminance(first), luminance(second)), reverse=True)
    return (high + 0.05) / (low + 0.05)


@pytest.mark.parametrize("name", OLDER_THEMES)
def test_older_themes_carry_no_new_section(name):
    theme = load(name)
    assert not [section for section in NEW_SECTIONS if section in theme]


@pytest.mark.parametrize("name", BUNDLED_THEMES)
def test_bundled_theme_is_in_the_resource_tree(name):
    qrc = QRC.read_text(encoding="utf-8")
    assert f"<file>themes/{name}.json</file>" in qrc
    assert f"<file>themes/code-editor/{name}.xml</file>" in qrc
    assert load(name)["title"].lower().replace(" ", "-") == name
    assert load(name)["parameters"]["code-editor-theme"] == name


@pytest.mark.parametrize("name", BUNDLED_THEMES)
def test_bundled_theme_carries_the_complete_colour_table(name):
    assert set(load(name)["colors"]) == set(load("default")["colors"])


@pytest.mark.parametrize("name", MATERIAL_THEMES)
def test_material_theme_names_only_what_it_defines(name):
    theme = load(name)
    materials = theme["materials"]
    assert {entry["recipe"] for entry in materials.values()} <= RECIPES
    assert set(theme["surfaces"].values()) <= set(materials)
    for key, look in theme.get("widgets", {}).items():
        assert look in LOOKS[key]

    for rule in theme.get("styles", {}).values():
        if "font" in rule:
            assert rule["font"] in theme.get("fonts", {})

        if "material" in rule:
            assert rule["material"] in materials


@pytest.mark.parametrize("name", BUNDLED_THEMES)
@pytest.mark.parametrize("text,surface", TEXT_PAIRS)
def test_bundled_theme_lettering_contrast(name, text, surface):
    colors = load(name)["colors"]
    assert contrast(colors[text], colors[surface]) >= 4.5


@pytest.mark.parametrize("name", MATERIAL_THEMES)
def test_bundled_themes_keep_the_interface_fonts(name):
    assert "fonts" not in load(name)
    assert "Michroma" not in QRC.read_text(encoding="utf-8")


def test_grain_tile_matches_its_generator():
    run = subprocess.run(
        [sys.executable, str(GENERATOR), "--check"], capture_output=True, text=True
    )
    assert run.returncode == 0, run.stdout + run.stderr
