"""
Workspace tile identity (spec 0083).

A workspace tile is resolved by the group and dataset it was placed for, not by its
position among widgets of the same type. Inserting or moving a group must leave every
tile attached to its original widget, across a save/reload as well.

Needs the app up with the API server enabled (see tests/README.md).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

pytestmark = [pytest.mark.integration, pytest.mark.project]

WIDGET_PLOT = 9


def _project_with_plot_groups(api_client, titles: list[str]) -> list[dict]:
    """One datagrid group per title, each holding one plotted dataset; returns group.list."""
    api_client.create_new_project()
    time.sleep(0.2)

    for title in titles:
        gid = api_client.add_group(title, widget_type=0)
        api_client.add_dataset(gid, options=1)

    api_client.set_operation_mode("project")
    time.sleep(0.2)
    return api_client.list_groups()


def _dataset_unique_id(api_client, group_unique_id: int) -> int:
    for dataset in api_client.list_datasets():
        if dataset.get("groupUniqueId", dataset.get("groupId")) == group_unique_id:
            return dataset["uniqueId"]

    raise AssertionError(f"no dataset in group {group_unique_id}")


def _tile(api_client, workspace_id: int) -> dict:
    widgets = api_client.command("project.workspace.get", {"id": workspace_id})[
        "widgets"
    ]
    assert len(widgets) == 1
    return widgets[0]


def test_tile_keeps_its_dataset_when_a_group_moves_before_it(api_client, clean_state):
    """The plot tile of group B keeps B's dataset after C is moved to position 0."""
    groups = _project_with_plot_groups(api_client, ["A", "B"])
    uid_b = groups[1]["uniqueId"]

    api_client.command("project.workspace.setCustomizeMode", {"enabled": True})
    ws = api_client.command("project.workspace.add", {"title": "Bench"})["id"]

    added = api_client.command(
        "project.workspace.addWidget",
        {
            "workspaceId": ws,
            "widgetType": WIDGET_PLOT,
            "groupId": uid_b,
            "datasetId": 0,
        },
    )
    assert added["relativeIndex"] == 1
    dataset_uid = added["datasetUniqueId"]
    assert dataset_uid >= 0

    gid_c = api_client.add_group("C", widget_type=0)
    api_client.add_dataset(gid_c, options=1)
    api_client.command("project.group.move", {"groupId": gid_c, "newPosition": 0})
    time.sleep(0.3)

    tile = _tile(api_client, ws)
    assert tile["groupId"] == uid_b
    assert tile["datasetUniqueId"] == dataset_uid
    assert tile["relativeIndex"] == 2

    project_json = api_client.command("project.exportJson")["config"]
    api_client.create_new_project()
    time.sleep(0.2)
    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)

    tile = _tile(api_client, ws)
    assert tile["datasetUniqueId"] == dataset_uid
    assert tile["relativeIndex"] == 2
    assert api_client.command("project.workspace.validate")["ok"] is True


def test_legacy_ref_without_identity_is_back_filled_on_load(api_client, clean_state):
    """A file whose ref carries only the ordinal loads with datasetUniqueId filled in."""
    groups = _project_with_plot_groups(api_client, ["A", "B"])
    uid_b = groups[1]["uniqueId"]
    dataset_uid = _dataset_unique_id(api_client, uid_b)

    api_client.command("project.workspace.setCustomizeMode", {"enabled": True})
    ws = api_client.command("project.workspace.add", {"title": "Legacy"})["id"]
    project_json = api_client.command("project.exportJson")["config"]

    legacy_ref = {"widgetType": WIDGET_PLOT, "groupId": uid_b, "relativeIndex": 1}
    for workspace in project_json["workspaces"]:
        if workspace["workspaceId"] == ws:
            workspace["widgetRefs"] = [legacy_ref]

    api_client.create_new_project()
    time.sleep(0.2)
    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)

    tile = _tile(api_client, ws)
    assert tile["datasetUniqueId"] == dataset_uid
    assert tile["relativeIndex"] == 1


def test_externally_shifted_ordinal_is_rebound_on_load(api_client, clean_state):
    """A generator that inserted a group but left a stale ordinal still opens with the tile."""
    groups = _project_with_plot_groups(api_client, ["A", "B"])
    uid_b = groups[1]["uniqueId"]
    dataset_uid = _dataset_unique_id(api_client, uid_b)

    api_client.command("project.workspace.setCustomizeMode", {"enabled": True})
    ws = api_client.command("project.workspace.add", {"title": "Shifted"})["id"]
    api_client.command(
        "project.workspace.addWidget",
        {
            "workspaceId": ws,
            "widgetType": WIDGET_PLOT,
            "groupId": uid_b,
            "datasetId": 0,
        },
    )
    project_json = api_client.command("project.exportJson")["config"]

    inserted = dict(project_json["groups"][0])
    inserted["title"] = "Inserted"
    inserted["uniqueId"] = project_json["nextUniqueId"]
    for dataset in inserted["datasets"]:
        dataset["uniqueId"] = dataset["uniqueId"] + 100000
    project_json["groups"].insert(0, inserted)
    project_json["nextUniqueId"] = project_json["nextUniqueId"] + 1

    api_client.create_new_project()
    time.sleep(0.2)
    api_client.command("project.loadJson", {"config": project_json})
    time.sleep(0.3)

    tile = _tile(api_client, ws)
    assert tile["datasetUniqueId"] == dataset_uid
    assert tile["relativeIndex"] == 2
