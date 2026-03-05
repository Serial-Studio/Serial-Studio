"""
ProjectModel / ProjectEditor Integration Tests

Covers the MVC refactor: group/dataset/action CRUD, bounds guards,
duplicate operations, export round-trips, and modified-state tracking.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time
from pathlib import Path

import pytest

from utils import DataGenerator


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _add_group(api_client, title="Group", widget_type=0):
    api_client.command("project.group.add", {"title": title, "widgetType": widget_type})
    time.sleep(0.15)


def _add_dataset(api_client, options=0):
    api_client.command("project.dataset.add", {"options": options})
    time.sleep(0.1)


def _add_action(api_client):
    api_client.command("project.action.add")
    time.sleep(0.1)


def _status(api_client):
    return api_client.get_project_status()


def _groups(api_client):
    return api_client.command("project.groups.list").get("groups", [])


def _datasets(api_client):
    return api_client.command("project.datasets.list").get("datasets", [])


def _actions(api_client):
    return api_client.command("project.actions.list").get("actions", [])


# ---------------------------------------------------------------------------
# Group CRUD
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_add_single_group(api_client, clean_state):
    """Adding a group increases groupCount by 1."""
    before = _status(api_client)["groupCount"]
    _add_group(api_client, "Sensors")
    assert _status(api_client)["groupCount"] == before + 1


@pytest.mark.project
def test_add_multiple_groups(api_client, clean_state):
    """Adding N groups yields exactly N groups."""
    api_client.create_new_project()
    time.sleep(0.2)

    for i in range(3):
        _add_group(api_client, f"Group {i}")

    assert _status(api_client)["groupCount"] == 3


@pytest.mark.project
def test_group_titles_preserved(api_client, clean_state):
    """Group titles returned by groups.list match what was set."""
    api_client.create_new_project()
    time.sleep(0.2)

    titles = ["Alpha", "Beta", "Gamma"]
    for t in titles:
        _add_group(api_client, t)

    returned = [g["title"] for g in _groups(api_client)]
    for t in titles:
        assert t in returned


@pytest.mark.project
def test_delete_group_reduces_count(api_client, clean_state):
    """Deleting the selected group decreases groupCount."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "ToDelete")
    _add_dataset(api_client)
    before = _status(api_client)["groupCount"]

    api_client.command("project.group.delete")
    time.sleep(0.2)

    assert _status(api_client)["groupCount"] == before - 1


@pytest.mark.project
def test_delete_group_when_empty_is_noop(api_client, clean_state):
    """Deleting when there are no groups must not crash."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.group.delete")
    time.sleep(0.2)

    assert _status(api_client)["groupCount"] == 0


@pytest.mark.project
def test_duplicate_group(api_client, clean_state):
    """Duplicating the selected group adds a second group."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Original")
    _add_dataset(api_client)
    _add_dataset(api_client)

    api_client.command("project.group.duplicate")
    time.sleep(0.2)

    assert _status(api_client)["groupCount"] == 2


@pytest.mark.project
def test_duplicate_group_inherits_datasets(api_client, clean_state):
    """Duplicated group has at least as many datasets as the original."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Base")
    for _ in range(3):
        _add_dataset(api_client)

    time.sleep(0.3)

    api_client.command("project.group.duplicate")
    time.sleep(0.3)

    assert _status(api_client)["datasetCount"] >= 3
    assert _status(api_client)["groupCount"] == 2


@pytest.mark.project
def test_group_widget_types(api_client, clean_state):
    """Groups can be created with each widget type 0–6."""
    api_client.create_new_project()
    time.sleep(0.2)

    for wt in range(7):
        _add_group(api_client, f"Widget{wt}", widget_type=wt)

    assert _status(api_client)["groupCount"] == 7


# ---------------------------------------------------------------------------
# Dataset CRUD
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_add_datasets_to_group(api_client, clean_state):
    """Adding datasets increments datasetCount."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Sensors")
    for _ in range(5):
        _add_dataset(api_client)

    assert _status(api_client)["datasetCount"] >= 5


@pytest.mark.project
def test_delete_dataset_reduces_count(api_client, clean_state):
    """Deleting the selected dataset decreases datasetCount."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G")
    _add_dataset(api_client)
    _add_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.command("project.dataset.delete")
    time.sleep(0.2)

    assert _status(api_client)["datasetCount"] == before - 1


@pytest.mark.project
def test_delete_last_dataset_removes_empty_group(api_client, clean_state):
    """Deleting the only dataset in a group also removes that group."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Solo")
    _add_dataset(api_client)

    group_count_before = _status(api_client)["groupCount"]
    api_client.command("project.dataset.delete")
    time.sleep(0.2)

    assert _status(api_client)["groupCount"] < group_count_before


@pytest.mark.project
def test_delete_dataset_when_none_is_noop(api_client, clean_state):
    """Deleting a dataset with no datasets present must not crash."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.dataset.delete")
    time.sleep(0.2)

    assert _status(api_client)["datasetCount"] == 0


@pytest.mark.project
def test_duplicate_dataset(api_client, clean_state):
    """Duplicating a dataset adds one more dataset to the group."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G")
    _add_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.command("project.dataset.duplicate")
    time.sleep(0.2)

    assert _status(api_client)["datasetCount"] == before + 1


@pytest.mark.project
def test_dataset_option_toggle(api_client, clean_state):
    """project.dataset.setOption does not crash and count is unchanged."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G")
    _add_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.command("project.dataset.setOption", {"option": 0, "enabled": True})
    time.sleep(0.1)
    api_client.command("project.dataset.setOption", {"option": 0, "enabled": False})
    time.sleep(0.1)

    assert _status(api_client)["datasetCount"] == before


@pytest.mark.project
def test_datasets_list_contains_group_info(api_client, clean_state):
    """datasets.list entries report which group each dataset belongs to."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Sensors")
    _add_dataset(api_client)
    _add_dataset(api_client)

    datasets = _datasets(api_client)
    assert len(datasets) >= 2
    for d in datasets:
        assert "groupTitle" in d or "group" in d or "groupId" in d


# ---------------------------------------------------------------------------
# Action CRUD
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_add_action(api_client, clean_state):
    """Adding an action increases actionCount."""
    api_client.create_new_project()
    time.sleep(0.2)

    before = _status(api_client).get("actionCount", 0)
    _add_action(api_client)

    assert _status(api_client).get("actionCount", 0) == before + 1


@pytest.mark.project
def test_add_multiple_actions(api_client, clean_state):
    """Adding N actions yields N actions in the list."""
    api_client.create_new_project()
    time.sleep(0.2)

    for _ in range(4):
        _add_action(api_client)

    assert len(_actions(api_client)) == 4


@pytest.mark.project
def test_action_titles_are_unique(api_client, clean_state):
    """Auto-numbered action titles must all be distinct."""
    api_client.create_new_project()
    time.sleep(0.2)

    for _ in range(3):
        _add_action(api_client)

    titles = [a["title"] for a in _actions(api_client)]
    assert len(titles) == len(set(titles))


@pytest.mark.project
def test_delete_action(api_client, clean_state):
    """Deleting the selected action decreases actionCount."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_action(api_client)
    _add_action(api_client)
    before = _status(api_client).get("actionCount", 0)

    api_client.command("project.action.delete")
    time.sleep(0.2)

    assert _status(api_client).get("actionCount", 0) == before - 1


@pytest.mark.project
def test_delete_action_when_none_is_noop(api_client, clean_state):
    """Deleting an action when none exist must not crash."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.action.delete")
    time.sleep(0.2)

    assert _status(api_client).get("actionCount", 0) == 0


@pytest.mark.project
def test_duplicate_action(api_client, clean_state):
    """Duplicating an action adds one more action."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_action(api_client)
    before = _status(api_client).get("actionCount", 0)

    api_client.command("project.action.duplicate")
    time.sleep(0.2)

    assert _status(api_client).get("actionCount", 0) == before + 1


# ---------------------------------------------------------------------------
# Modified state
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_modified_flag_set_after_add_group(api_client, clean_state, tmp_path):
    """modified flag is true after adding a group to a saved project."""
    proj_path = tmp_path / "mod_test.ssproj"
    proj = DataGenerator.generate_project_with_frame_delimiters(
        start="/*", end="*/", detection_mode=1, title="Mod", checksum_algorithm="None"
    )
    proj_path.write_text(json.dumps(proj, indent=2), encoding="utf-8")

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    api_client.command("project.file.save")
    time.sleep(0.2)

    _add_group(api_client, "NewGroup")

    st = _status(api_client)
    assert st.get("modified") is True


@pytest.mark.project
def test_modified_flag_cleared_after_save(api_client, clean_state, tmp_path):
    """modified flag is false immediately after saving."""
    proj_path = tmp_path / "mod_save.ssproj"
    proj = DataGenerator.generate_project_with_frame_delimiters(
        start="/*", end="*/", detection_mode=1, title="MS", checksum_algorithm="None"
    )
    proj_path.write_text(json.dumps(proj, indent=2), encoding="utf-8")

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _add_group(api_client, "Extra")
    _add_dataset(api_client)

    api_client.command("project.file.save")
    time.sleep(0.3)

    st = _status(api_client)
    assert st.get("modified") is False


# ---------------------------------------------------------------------------
# Export / import round-trip
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_export_preserves_group_count(api_client, clean_state):
    """Exported JSON has the same number of groups as the live project."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "A")
    _add_dataset(api_client)
    _add_group(api_client, "B")
    _add_dataset(api_client)

    exported = api_client.command("project.exportJson")["config"]
    assert len(exported.get("groups", [])) == _status(api_client)["groupCount"]


@pytest.mark.project
def test_export_preserves_dataset_count(api_client, clean_state):
    """Exported JSON contains same total datasets as the live project."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G1")
    _add_dataset(api_client)
    _add_dataset(api_client)

    exported = api_client.command("project.exportJson")["config"]
    total_ds = sum(len(g.get("datasets", [])) for g in exported.get("groups", []))
    assert total_ds == _status(api_client)["datasetCount"]


@pytest.mark.project
def test_load_from_json_round_trip(api_client, clean_state):
    """loadFromJSON restores the same group/dataset structure."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "Round")
    _add_dataset(api_client)
    _add_dataset(api_client)
    _add_dataset(api_client)

    exported = api_client.command("project.exportJson")["config"]
    original_group_count = _status(api_client)["groupCount"]
    original_ds_count = _status(api_client)["datasetCount"]

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadFromJSON", {"config": exported})
    time.sleep(0.3)

    st = _status(api_client)
    assert st["groupCount"] == original_group_count
    assert st["datasetCount"] == original_ds_count


@pytest.mark.project
def test_export_contains_required_keys(api_client, clean_state):
    """Exported JSON must have title and groups keys."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "K")
    _add_dataset(api_client)

    exported = api_client.command("project.exportJson")["config"]
    assert "title" in exported
    assert "groups" in exported


@pytest.mark.project
def test_file_open_then_export_matches_disk(api_client, clean_state, tmp_path):
    """project.exportJson and file on disk agree on group structure."""
    proj = DataGenerator.generate_project_with_frame_delimiters(
        start="/*", end="*/", detection_mode=1, title="DiskMatch", checksum_algorithm="None"
    )
    proj_path = tmp_path / "disk_match.ssproj"
    proj_path.write_text(json.dumps(proj, indent=2), encoding="utf-8")

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)

    exported = api_client.command("project.exportJson")["config"]
    disk = json.loads(proj_path.read_text(encoding="utf-8"))

    assert len(exported.get("groups", [])) == len(disk.get("groups", []))


# ---------------------------------------------------------------------------
# Title management
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_set_project_title(api_client, clean_state):
    """project.setTitle updates the title in getStatus."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.setTitle", {"title": "My Custom Title"})
    time.sleep(0.1)

    st = _status(api_client)
    assert st.get("title") == "My Custom Title"


@pytest.mark.project
def test_new_project_resets_title(api_client, clean_state):
    """create_new_project resets to a non-empty default title."""
    api_client.command("project.setTitle", {"title": "Old Title"})
    time.sleep(0.1)

    api_client.create_new_project()
    time.sleep(0.2)

    st = _status(api_client)
    assert st.get("title") != "Old Title"
    assert len(st.get("title", "")) > 0


# ---------------------------------------------------------------------------
# Successive add/delete stress
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_rapid_add_delete_groups(api_client, clean_state):
    """Add then delete groups in quick succession without crashing."""
    api_client.create_new_project()
    time.sleep(0.2)

    for i in range(5):
        _add_group(api_client, f"G{i}")
        _add_dataset(api_client)

    before = _status(api_client)["groupCount"]

    for _ in range(3):
        api_client.command("project.group.delete")
        time.sleep(0.2)

    assert _status(api_client)["groupCount"] < before


@pytest.mark.project
def test_rapid_add_delete_datasets(api_client, clean_state):
    """Add many datasets then delete them down, count decreases each time."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G")
    for _ in range(6):
        _add_dataset(api_client)

    time.sleep(0.2)
    before = _status(api_client)["datasetCount"]

    for _ in range(4):
        api_client.command("project.dataset.delete")
        time.sleep(0.2)

    assert _status(api_client)["datasetCount"] < before


@pytest.mark.project
def test_add_group_dataset_action_together(api_client, clean_state):
    """Mixed add operations keep counts consistent."""
    api_client.create_new_project()
    time.sleep(0.2)

    _add_group(api_client, "G1")
    _add_dataset(api_client)
    _add_dataset(api_client)
    _add_action(api_client)
    _add_action(api_client)
    _add_action(api_client)

    st = _status(api_client)
    assert st["groupCount"] >= 1
    assert st["datasetCount"] >= 2
    assert st.get("actionCount", 0) >= 3
