"""
ProjectModel / ProjectEditor Integration Tests

Covers the v3.3 stateless API: group/dataset/action CRUD, bounds guards,
duplicate operations, export round-trips, and modified-state tracking.
All mutators take explicit ids -- there's no notion of "currently selected".

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import DataGenerator

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _status(api_client):
    return api_client.get_project_status()


def _last_group_id(api_client) -> int:
    groups = api_client.list_groups()
    assert groups, "Expected at least one group"
    return len(groups) - 1


def _last_dataset(api_client) -> tuple[int, int]:
    datasets = api_client.list_datasets()
    assert datasets, "Expected at least one dataset"
    last = datasets[-1]
    return last["groupId"], last["datasetId"]


def _last_action_id(api_client) -> int:
    actions = api_client.list_actions()
    assert actions, "Expected at least one action"
    return actions[-1]["actionId"]


def _exported_dataset(exported: dict, group_id: int, dataset_id: int) -> dict:
    for group in exported.get("groups", []):
        for ds in group.get("datasets", []):
            if ds.get("groupId") == group_id and ds.get("datasetId") == dataset_id:
                return ds

    raise AssertionError(
        f"Dataset ({group_id}, {dataset_id}) not found in exported config"
    )


# ---------------------------------------------------------------------------
# Group CRUD
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_add_single_group(api_client, clean_state):
    """Adding a group increases groupCount by 1."""
    before = _status(api_client)["groupCount"]
    api_client.add_group("Sensors")
    assert _status(api_client)["groupCount"] == before + 1


@pytest.mark.project
def test_add_multiple_groups(api_client, clean_state):
    """Adding N groups yields exactly N groups."""
    for i in range(3):
        api_client.add_group(f"Group {i}")

    assert _status(api_client)["groupCount"] == 3


@pytest.mark.project
def test_group_titles_preserved(api_client, clean_state):
    """Group titles returned by groups.list match what was set."""
    titles = ["Alpha", "Beta", "Gamma"]
    for t in titles:
        api_client.add_group(t)

    returned = [g["title"] for g in api_client.list_groups()]
    for t in titles:
        assert t in returned


@pytest.mark.project
def test_delete_group_reduces_count(api_client, clean_state):
    """Deleting a group by id decreases groupCount."""
    gid = api_client.add_group("ToDelete")
    api_client.add_dataset(gid)
    before = _status(api_client)["groupCount"]

    api_client.delete_group(gid)

    assert _status(api_client)["groupCount"] == before - 1


@pytest.mark.project
def test_delete_unknown_group_id_is_safe(api_client, clean_state):
    """Deleting a non-existent group must not crash."""
    from utils.api_client import APIError

    try:
        api_client.delete_group(99999)
    except APIError:
        # Server reporting a clean error is also acceptable.
        pass

    assert _status(api_client)["groupCount"] == 0


@pytest.mark.project
def test_duplicate_group(api_client, clean_state):
    """Duplicating a group by id adds a second group."""
    gid = api_client.add_group("Original")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    api_client.duplicate_group(gid)

    assert _status(api_client)["groupCount"] == 2


@pytest.mark.project
def test_duplicate_group_inherits_datasets(api_client, clean_state):
    """Duplicated group has at least as many datasets as the original."""
    gid = api_client.add_group("Base")
    for _ in range(3):
        api_client.add_dataset(gid)

    api_client.duplicate_group(gid)

    assert _status(api_client)["datasetCount"] >= 6
    assert _status(api_client)["groupCount"] == 2


@pytest.mark.project
def test_group_widget_types(api_client, clean_state):
    """Groups can be created with each widget type 0-6."""
    for wt in range(7):
        api_client.add_group(f"Widget{wt}", widget_type=wt)

    assert _status(api_client)["groupCount"] == 7


@pytest.mark.project
def test_group_update_renames(api_client, clean_state):
    """project.group.update can rename a group by id."""
    gid = api_client.add_group("OldName")
    api_client.update_group(gid, title="NewName")

    titles = [g["title"] for g in api_client.list_groups()]
    assert "NewName" in titles
    assert "OldName" not in titles


# ---------------------------------------------------------------------------
# Dataset CRUD
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_add_datasets_to_group(api_client, clean_state):
    """Adding datasets increments datasetCount."""
    gid = api_client.add_group("Sensors")
    for _ in range(5):
        api_client.add_dataset(gid)

    assert _status(api_client)["datasetCount"] >= 5


@pytest.mark.project
def test_delete_dataset_reduces_count(api_client, clean_state):
    """Deleting a dataset by id decreases datasetCount."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.delete_dataset(g, d)

    assert _status(api_client)["datasetCount"] == before - 1


@pytest.mark.project
def test_delete_last_dataset_removes_empty_group(api_client, clean_state):
    """Deleting the only dataset in a group also removes that group."""
    gid = api_client.add_group("Solo")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    group_count_before = _status(api_client)["groupCount"]
    api_client.delete_dataset(g, d)

    assert _status(api_client)["groupCount"] < group_count_before


@pytest.mark.project
def test_delete_unknown_dataset_is_safe(api_client, clean_state):
    """Deleting a non-existent dataset must not crash."""
    from utils.api_client import APIError

    try:
        api_client.delete_dataset(99999, 99999)
    except APIError:
        pass

    assert _status(api_client)["datasetCount"] == 0


@pytest.mark.project
def test_duplicate_dataset(api_client, clean_state):
    """Duplicating a dataset adds one more dataset to the group."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.duplicate_dataset(g, d)

    assert _status(api_client)["datasetCount"] == before + 1


@pytest.mark.project
def test_dataset_option_toggle(api_client, clean_state):
    """project.dataset.setOption does not crash and count is unchanged."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)
    before = _status(api_client)["datasetCount"]

    api_client.set_dataset_option(g, d, option=0, enabled=True)
    api_client.set_dataset_option(g, d, option=0, enabled=False)

    assert _status(api_client)["datasetCount"] == before


@pytest.mark.project
def test_dataset_set_options_bitfield(api_client, clean_state):
    """project.dataset.setOptions applies several bits in one call and surfaces them in dataset.list."""
    gid = api_client.add_group("Audio")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    plot_fft_waterfall = 1 | 2 | 64
    api_client.set_dataset_options(g, d, options=plot_fft_waterfall)

    datasets = {(x["groupId"], x["datasetId"]): x for x in api_client.list_datasets()}
    assert datasets[(g, d)]["enabledOptions"] == plot_fft_waterfall


@pytest.mark.project
def test_dataset_update_field_patch(api_client, clean_state):
    """project.dataset.update can patch title and units in one call."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    api_client.update_dataset(g, d, title="Renamed", units="rpm")

    datasets = {(x["groupId"], x["datasetId"]): x for x in api_client.list_datasets()}
    assert datasets[(g, d)]["title"] == "Renamed"
    assert datasets[(g, d)]["units"] == "rpm"


@pytest.mark.project
def test_dataset_color_round_trip(api_client, clean_state):
    """A dataset color override survives exportJson -> loadJson."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    api_client.update_dataset(g, d, color="#ff0000")

    exported = api_client.command("project.exportJson")["config"]
    assert _exported_dataset(exported, g, d)["color"] == "#ff0000"

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadJson", {"config": exported})
    time.sleep(0.3)

    reloaded = api_client.command("project.exportJson")["config"]
    assert _exported_dataset(reloaded, g, d)["color"] == "#ff0000"


@pytest.mark.project
def test_log_axis_round_trip(api_client, clean_state):
    """The log-axis flags survive exportJson -> loadJson; absent keys mean linear."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    fresh = api_client.command("project.exportJson")["config"]
    fresh_ds = _exported_dataset(fresh, g, d)
    assert "plotLogX" not in fresh_ds
    assert "plotLogY" not in fresh_ds
    assert "fftLogX" not in fresh_ds

    api_client.update_dataset(g, d, plotLogX=True, plotLogY=True, fftLogX=True)

    exported = api_client.command("project.exportJson")["config"]
    exported_ds = _exported_dataset(exported, g, d)
    assert exported_ds["plotLogX"] is True
    assert exported_ds["plotLogY"] is True
    assert exported_ds["fftLogX"] is True

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadJson", {"config": exported})
    time.sleep(0.3)

    reloaded = api_client.command("project.exportJson")["config"]
    reloaded_ds = _exported_dataset(reloaded, g, d)
    assert reloaded_ds["plotLogX"] is True
    assert reloaded_ds["plotLogY"] is True
    assert reloaded_ds["fftLogX"] is True


@pytest.mark.project
def test_fft_ballistics_round_trip(api_client, clean_state):
    """The FFT ballistics fields survive exportJson -> loadJson; absent keys mean off."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    fresh = api_client.command("project.exportJson")["config"]
    fresh_ds = _exported_dataset(fresh, g, d)
    assert "fftBallistics" not in fresh_ds
    assert "fftBallisticsRelease" not in fresh_ds

    api_client.update_dataset(g, d, fftBallistics=True, fftBallisticsRelease=500)

    exported = api_client.command("project.exportJson")["config"]
    exported_ds = _exported_dataset(exported, g, d)
    assert exported_ds["fftBallistics"] is True
    assert exported_ds["fftBallisticsRelease"] == 500

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadJson", {"config": exported})
    time.sleep(0.3)

    reloaded = api_client.command("project.exportJson")["config"]
    reloaded_ds = _exported_dataset(reloaded, g, d)
    assert reloaded_ds["fftBallistics"] is True
    assert reloaded_ds["fftBallisticsRelease"] == 500


@pytest.mark.project
def test_dataset_color_clear_restores_automatic(api_client, clean_state):
    """Setting color to an empty string reverts the dataset to automatic."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    api_client.update_dataset(g, d, color="#00ff00")
    api_client.update_dataset(g, d, color="")

    exported = api_client.command("project.exportJson")["config"]
    assert "color" not in _exported_dataset(exported, g, d)


@pytest.mark.project
def test_dataset_without_color_is_automatic(api_client, clean_state):
    """Datasets never given a color export without a color key (automatic state)."""
    gid = api_client.add_group("G")
    api_client.add_dataset(gid)
    g, d = _last_dataset(api_client)

    exported = api_client.command("project.exportJson")["config"]
    assert "color" not in _exported_dataset(exported, g, d)


@pytest.mark.project
def test_datasets_list_contains_group_info(api_client, clean_state):
    """datasets.list entries report which group each dataset belongs to."""
    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    datasets = api_client.list_datasets()
    assert len(datasets) >= 2
    for d in datasets:
        assert "groupId" in d
        assert "datasetId" in d


# ---------------------------------------------------------------------------
# Action CRUD
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_add_action(api_client, clean_state):
    """Adding an action increases actionCount."""
    before = _status(api_client).get("actionCount", 0)
    api_client.add_action()

    assert _status(api_client).get("actionCount", 0) == before + 1


@pytest.mark.project
def test_add_multiple_actions(api_client, clean_state):
    """Adding N actions yields N actions in the list."""
    for _ in range(4):
        api_client.add_action()

    assert len(api_client.list_actions()) == 4


@pytest.mark.project
def test_action_titles_are_unique(api_client, clean_state):
    """Auto-numbered action titles must all be distinct."""
    for _ in range(3):
        api_client.add_action()

    titles = [a["title"] for a in api_client.list_actions()]
    assert len(titles) == len(set(titles))


@pytest.mark.project
def test_delete_action(api_client, clean_state):
    """Deleting an action by id decreases actionCount."""
    api_client.add_action()
    api_client.add_action()
    aid = _last_action_id(api_client)
    before = _status(api_client).get("actionCount", 0)

    api_client.delete_action(aid)

    assert _status(api_client).get("actionCount", 0) == before - 1


@pytest.mark.project
def test_delete_unknown_action_is_safe(api_client, clean_state):
    """Deleting an unknown action id must not crash."""
    from utils.api_client import APIError

    try:
        api_client.delete_action(99999)
    except APIError:
        pass

    assert _status(api_client).get("actionCount", 0) == 0


@pytest.mark.project
def test_duplicate_action(api_client, clean_state):
    """Duplicating an action adds one more action."""
    api_client.add_action()
    aid = _last_action_id(api_client)
    before = _status(api_client).get("actionCount", 0)

    api_client.duplicate_action(aid)

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

    api_client.command("project.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    api_client.command("project.save")
    time.sleep(0.2)

    api_client.add_group("NewGroup")

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

    api_client.command("project.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    gid = api_client.add_group("Extra")
    api_client.add_dataset(gid)

    api_client.command("project.save")
    time.sleep(0.3)

    st = _status(api_client)
    assert st.get("modified") is False


# ---------------------------------------------------------------------------
# Export / import round-trip
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_export_preserves_group_count(api_client, clean_state):
    """Exported JSON has the same number of groups as the live project."""
    g1 = api_client.add_group("A")
    api_client.add_dataset(g1)
    g2 = api_client.add_group("B")
    api_client.add_dataset(g2)

    exported = api_client.command("project.exportJson")["config"]
    assert len(exported.get("groups", [])) == _status(api_client)["groupCount"]


@pytest.mark.project
def test_export_preserves_dataset_count(api_client, clean_state):
    """Exported JSON contains same total datasets as the live project."""
    gid = api_client.add_group("G1")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    exported = api_client.command("project.exportJson")["config"]
    total_ds = sum(len(g.get("datasets", [])) for g in exported.get("groups", []))
    assert total_ds == _status(api_client)["datasetCount"]


@pytest.mark.project
def test_load_from_json_round_trip(api_client, clean_state):
    """loadJson restores the same group/dataset structure."""
    gid = api_client.add_group("Round")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    exported = api_client.command("project.exportJson")["config"]
    original_group_count = _status(api_client)["groupCount"]
    original_ds_count = _status(api_client)["datasetCount"]

    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.loadJson", {"config": exported})
    time.sleep(0.3)

    st = _status(api_client)
    assert st["groupCount"] == original_group_count
    assert st["datasetCount"] == original_ds_count


@pytest.mark.project
def test_export_contains_required_keys(api_client, clean_state):
    """Exported JSON must have title and groups keys."""
    gid = api_client.add_group("K")
    api_client.add_dataset(gid)

    exported = api_client.command("project.exportJson")["config"]
    assert "title" in exported
    assert "groups" in exported


@pytest.mark.project
def test_file_open_then_export_matches_disk(api_client, clean_state, tmp_path):
    """project.exportJson and file on disk agree on group structure."""
    proj = DataGenerator.generate_project_with_frame_delimiters(
        start="/*",
        end="*/",
        detection_mode=1,
        title="DiskMatch",
        checksum_algorithm="None",
    )
    proj_path = tmp_path / "disk_match.ssproj"
    proj_path.write_text(json.dumps(proj, indent=2), encoding="utf-8")

    api_client.command("project.open", {"filePath": str(proj_path)})
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
    api_client.command("project.setTitle", {"title": "My Custom Title"})

    st = _status(api_client)
    assert st.get("title") == "My Custom Title"


@pytest.mark.project
def test_new_project_resets_title(api_client, clean_state):
    """create_new_project resets to a non-empty default title."""
    api_client.command("project.setTitle", {"title": "Old Title"})

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
    gids = []
    for i in range(5):
        gid = api_client.add_group(f"G{i}")
        api_client.add_dataset(gid)
        gids.append(gid)

    before = _status(api_client)["groupCount"]

    for gid in gids[:3]:
        api_client.delete_group(gid)

    assert _status(api_client)["groupCount"] < before


@pytest.mark.project
def test_rapid_add_delete_datasets(api_client, clean_state):
    """Add many datasets then delete them down, count decreases each time."""
    gid = api_client.add_group("G")
    for _ in range(6):
        api_client.add_dataset(gid)

    before = _status(api_client)["datasetCount"]

    for _ in range(4):
        g, d = _last_dataset(api_client)
        api_client.delete_dataset(g, d)

    assert _status(api_client)["datasetCount"] < before


@pytest.mark.project
def test_add_group_dataset_action_together(api_client, clean_state):
    """Mixed add operations keep counts consistent."""
    gid = api_client.add_group("G1")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)
    for _ in range(3):
        api_client.add_action()

    st = _status(api_client)
    assert st["groupCount"] >= 1
    assert st["datasetCount"] >= 2
    assert st.get("actionCount", 0) >= 3
