"""
Bar Panel group widget + extreme-hold dataset option (spec 0052)

Covers the document-level surfaces reachable over the API: creating bar-panel
groups (widgetType 10 and the raw widget string), the barPanelStyle group key
round-trip through export/loadJson, and the extremeHold dataset flag with its
persist-when-true serialization rule. Visual severity/marker checks stay
maintainer acceptance criteria.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

BAR_PANEL_WIDGET_TYPE = 10


def _exported_groups(api_client) -> list:
    return api_client.command("project.exportJson")["config"]["groups"]


def _exported_group(api_client, title: str) -> dict:
    for group in _exported_groups(api_client):
        if group.get("title") == title:
            return group

    raise AssertionError(f"Group '{title}' not found in exported config")


# ---------------------------------------------------------------------------
# Group creation & widget string
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_add_bar_panel_group_by_enum(api_client, clean_state):
    """project.group.add accepts widgetType 10 and stores the barpanel widget string."""
    api_client.add_group("Pressures", widget_type=BAR_PANEL_WIDGET_TYPE)

    exported = _exported_group(api_client, "Pressures")
    assert exported.get("widget") == "barpanel"


@pytest.mark.project
def test_update_group_to_bar_panel_by_string(api_client, clean_state):
    """project.group.update can switch an existing group to the barpanel widget."""
    gid = api_client.add_group("Temperatures")
    api_client.add_dataset(gid)
    api_client.update_group(gid, widget="barpanel")

    exported = _exported_group(api_client, "Temperatures")
    assert exported.get("widget") == "barpanel"
    assert len(exported.get("datasets", [])) == 1


# ---------------------------------------------------------------------------
# barPanelStyle persistence
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_bar_panel_style_round_trips_through_load(api_client, clean_state):
    """A barPanelStyle value survives loadJson -> exportJson unchanged."""
    api_client.add_group("Rake", widget_type=BAR_PANEL_WIDGET_TYPE)

    config = api_client.command("project.exportJson")["config"]
    for group in config["groups"]:
        if group.get("title") == "Rake":
            group["barPanelStyle"] = "vertical"

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    exported = _exported_group(api_client, "Rake")
    assert exported.get("barPanelStyle") == "vertical"


@pytest.mark.project
def test_bar_panel_style_defaults_to_absent(api_client, clean_state):
    """Auto orientation is the empty default and is not written to the project JSON."""
    api_client.add_group("AutoPanel", widget_type=BAR_PANEL_WIDGET_TYPE)

    exported = _exported_group(api_client, "AutoPanel")
    assert "barPanelStyle" not in exported


# ---------------------------------------------------------------------------
# extremeHold dataset option
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_extreme_hold_persists_when_true(api_client, clean_state):
    """project.dataset.update accepts extremeHold and the flag survives export."""
    gid = api_client.add_group("Vibration", widget_type=BAR_PANEL_WIDGET_TYPE)
    record = api_client.add_dataset(gid)
    dataset_id = record.get("datasetId", 0)

    api_client.update_dataset(gid, dataset_id, extremeHold=True)

    exported = _exported_group(api_client, "Vibration")
    datasets = exported.get("datasets", [])
    assert datasets, "Expected the dataset to survive export"
    assert datasets[0].get("extremeHold") is True


@pytest.mark.project
def test_extreme_hold_false_is_not_serialized(api_client, clean_state):
    """extremeHold uses persist-when-true: a false flag never lands in the JSON."""
    gid = api_client.add_group("Calm", widget_type=BAR_PANEL_WIDGET_TYPE)
    record = api_client.add_dataset(gid)
    dataset_id = record.get("datasetId", 0)

    api_client.update_dataset(gid, dataset_id, extremeHold=False)

    exported = _exported_group(api_client, "Calm")
    datasets = exported.get("datasets", [])
    assert datasets, "Expected the dataset to survive export"
    assert "extremeHold" not in datasets[0]
