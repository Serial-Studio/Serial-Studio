"""
LED Panel migration (spec 0114)

A project saved before 0114 flagged datasets with `led: true` and the dashboard
synthesized one LED panel per group out of them. The loader now turns every such
group into a real `ledpanel` group right after its source, copies only the
flagged datasets that were not hidden, retargets the workspace refs that pointed
at the phantom panel, and marks the project modified. These tests load the three
legacy fixtures through the API and assert that shape, then prove the migration
is one-shot: a migrated document reloaded through project.loadJson is unchanged.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import time
from pathlib import Path

import pytest

FIXTURES = Path(__file__).resolve().parents[1] / "fixtures" / "projects" / "legacy"
LED_PANEL_WIDGET_TYPE = 8


def _raw(name: str) -> dict:
    return json.loads((FIXTURES / f"{name}.ssproj").read_text(encoding="utf-8"))


def _open(api_client, name: str) -> dict:
    api_client.command("project.open", {"filePath": str(FIXTURES / f"{name}.ssproj")})
    time.sleep(0.3)
    return api_client.command("project.exportJson")["config"]


def _lit_titles(group: dict) -> list:
    return [
        d["title"]
        for d in group["datasets"]
        if d.get("led") and not d.get("hideOnDashboard")
    ]


def _shape(config: dict) -> list:
    return [
        (g["title"], g.get("widget", ""), len(g["datasets"])) for g in config["groups"]
    ]


@pytest.mark.project
@pytest.mark.parametrize("fixture", ["led-flag-all", "led-flag-mixed"])
def test_flagged_groups_gain_a_panel_right_after_them(api_client, clean_state, fixture):
    """Each group with lit datasets is followed by a `<title> LEDs` ledpanel group holding
    copies of exactly those datasets, in order, with their frame indices."""
    raw = _raw(fixture)
    config = _open(api_client, fixture)

    affected = [g for g in raw["groups"] if _lit_titles(g)]
    assert affected, "fixture lost its flagged datasets"
    assert len(config["groups"]) == len(raw["groups"]) + len(affected)

    for source in affected:
        position = [g["title"] for g in config["groups"]].index(source["title"])
        panel = config["groups"][position + 1]
        assert panel["widget"] == "ledpanel"
        assert panel["title"] == f"{source['title']} LEDs"
        assert [d["title"] for d in panel["datasets"]] == _lit_titles(source)
        lit = {d["title"]: d["index"] for d in source["datasets"]}
        assert [d["index"] for d in panel["datasets"]] == [
            lit[t] for t in _lit_titles(source)
        ]
        kept = config["groups"][position]
        assert len(kept["datasets"]) == len(source["datasets"])

    assert sum(1 for g in config["groups"] if g.get("widget") == "ledpanel") == len(
        affected
    )
    assert not any("led" in d for g in config["groups"] for d in g["datasets"])


@pytest.mark.project
def test_hidden_flagged_dataset_gets_no_light_and_refs_follow(api_client, clean_state):
    """A flagged dataset that was hidden showed no light, so it is not copied; the workspace
    ref that addressed the phantom panel now names the new group's uniqueId."""
    config = _open(api_client, "led-flag-hidden")

    assert _shape(config) == [("Doors", "datagrid", 3), ("Doors LEDs", "ledpanel", 1)]
    panel = config["groups"][1]
    assert panel["datasets"][0]["title"] == "Front"
    assert panel["datasets"][0]["ledHigh"] == 1

    workspaces = [w for w in config.get("workspaces", []) if w["title"] == "Doors"]
    assert len(workspaces) == 1
    refs = {r["widgetType"]: r["groupId"] for r in workspaces[0]["widgetRefs"]}
    assert refs[1] == config["groups"][0]["uniqueId"]
    assert refs[LED_PANEL_WIDGET_TYPE] == panel["uniqueId"]


@pytest.mark.project
def test_migration_is_one_shot(api_client, clean_state):
    """A migrated document carries nothing that triggers the migration again: reloading it
    through loadJson keeps every group and workspace as it was."""
    first = _open(api_client, "led-flag-mixed")
    before = [
        (w["title"], len(w.get("widgetRefs", []))) for w in first.get("workspaces", [])
    ]

    api_client.command("project.loadJson", {"config": first})
    time.sleep(0.3)
    second = api_client.command("project.exportJson")["config"]

    assert _shape(second) == _shape(first)
    assert [
        (w["title"], len(w.get("widgetRefs", []))) for w in second.get("workspaces", [])
    ] == before
    assert sum(1 for g in second["groups"] if g.get("widget") == "ledpanel") == 1
