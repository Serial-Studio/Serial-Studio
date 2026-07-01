"""
Group / Dataset Enable-Disable Integration Tests

Covers the per-group and per-dataset `enabled` flag (Group::enabled / Dataset::enabled):
 * A disabled group is excluded from frame building (absent from the runtime frame).
 * A disabled dataset is excluded while its siblings keep their frame indices.
 * The `disabled` JSON flag round-trips through export -> reload.
 * A project whose only group is disabled ingests frames without crashing.

The Project Editor toggles the flag from the tree context menu; there is no API mutator yet,
so these tests drive the flag through the project-file path: build a project, export it, mark
items `disabled` in the JSON, reload, then feed frames and read dashboard.getData back.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

JS_PASSTHROUGH = "function parse(frame) { return frame.split(','); }"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _build_two_group_project(api_client) -> dict:
    """
    Build a ProjectFile-mode project with two groups and return its exported config:
      * "Active" group, datasets A1/A2/A3 at frame indices 1/2/3
      * "Off" group, dataset O1 at frame index 4
    The JS passthrough parser splits comma-separated frames into positional fields.
    """
    api_client.create_new_project()
    time.sleep(0.2)

    active = api_client.add_group("Active", widget_type=0)
    for _ in range(3):
        api_client.add_dataset(active, options=1)

    off = api_client.add_group("Off", widget_type=0)
    api_client.add_dataset(off, options=1)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )
    api_client.set_frame_parser_code(JS_PASSTHROUGH, language=0, source_id=0)
    time.sleep(0.15)

    return api_client.command("project.exportJson")["config"]


def _group_by_title(config: dict, title: str) -> dict:
    for group in config.get("groups", []):
        if group.get("title") == title:
            return group
    raise AssertionError(f"group {title!r} not found in config")


def _normalize_indices(config: dict) -> None:
    """Reassign contiguous frame indices 1..N across all datasets, in declaration order."""
    index = 1
    for group in config.get("groups", []):
        for dataset in group.get("datasets", []):
            dataset["index"] = index
            index += 1


def _send_csv_and_read(
    api_client, device_simulator, payloads, settle_seconds=1.2
) -> dict:
    """Connect the loopback, stream payloads, return dashboard.getData result."""
    assert api_client.command("project.activate").get("loaded")

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [b"/*" + p + b"*/" for p in payloads]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(settle_seconds)

    data = api_client.get_dashboard_data()
    api_client.disconnect_device()
    time.sleep(0.2)
    return data


def _runtime_groups(data: dict) -> dict:
    """Map runtime frame group title -> {dataset title: numericValue}."""
    out = {}
    for group in data.get("frame", {}).get("groups", []):
        out[group.get("title")] = {
            d.get("title"): d.get("numericValue", 0.0)
            for d in group.get("datasets", [])
        }
    return out


# ---------------------------------------------------------------------------
# Persistence round-trip (AC1)
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
def test_fresh_items_load_enabled(api_client, clean_state):
    """A freshly built project has every group and dataset enabled (no `disabled` key)."""
    config = _build_two_group_project(api_client)

    for group in config["groups"]:
        assert group.get("disabled", False) is False
        for dataset in group.get("datasets", []):
            assert dataset.get("disabled", False) is False


@pytest.mark.integration
@pytest.mark.project
def test_disabled_flag_round_trips(api_client, clean_state):
    """`disabled` set on a group and a dataset survives a loadJson -> exportJson cycle."""
    config = _build_two_group_project(api_client)

    _group_by_title(config, "Off")["disabled"] = True
    _group_by_title(config, "Active")["datasets"][1]["disabled"] = True

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    reexported = api_client.command("project.exportJson")["config"]
    assert _group_by_title(reexported, "Off").get("disabled") is True

    active_datasets = _group_by_title(reexported, "Active")["datasets"]
    disabled = [d for d in active_datasets if d.get("disabled")]
    assert len(disabled) == 1, "exactly one Active dataset should remain disabled"


@pytest.mark.integration
@pytest.mark.project
def test_legacy_project_without_flag_loads_enabled(api_client, clean_state):
    """A project JSON with no `disabled` key on any item loads fully enabled."""
    config = _build_two_group_project(api_client)
    for group in config["groups"]:
        group.pop("disabled", None)
        for dataset in group.get("datasets", []):
            dataset.pop("disabled", None)

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    reexported = api_client.command("project.exportJson")["config"]
    for group in reexported["groups"]:
        assert group.get("disabled", False) is False
        for dataset in group.get("datasets", []):
            assert dataset.get("disabled", False) is False


# ---------------------------------------------------------------------------
# Frame-building exclusion + index integrity (AC2 / R4 / R5)
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
def test_disabled_items_excluded_siblings_keep_indices(
    api_client, device_simulator, clean_state
):
    """
    Disabling the "Off" group and the middle "Active" dataset removes them from the runtime
    frame, while the surviving datasets still read their original frame indices (no shift).
    """
    config = _build_two_group_project(api_client)
    _normalize_indices(config)

    active = _group_by_title(config, "Active")
    a1, a2, a3 = active["datasets"]  # indices 1, 2, 3
    a2["disabled"] = True
    _group_by_title(config, "Off")["disabled"] = True  # dataset O1 is index 4

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    data = _send_csv_and_read(api_client, device_simulator, [b"10,20,30,40"])
    groups = _runtime_groups(data)

    assert "Off" not in groups, "disabled group must not reach the runtime frame"
    assert "Active" in groups, "enabled group must be present"

    active_values = groups["Active"]
    assert a2["title"] not in active_values, "disabled dataset must be excluded"
    assert active_values.get(a1["title"]) == pytest.approx(10.0), "index 1 unchanged"
    assert active_values.get(a3["title"]) == pytest.approx(
        30.0
    ), "index 3 unchanged (no shift)"


@pytest.mark.integration
@pytest.mark.project
def test_batch_disabled_groups_all_excluded(api_client, device_simulator, clean_state):
    """
    Batch-disabling several groups at once (the tree "Hide Selected" action, and a group-folder
    cascade, set each group's `disabled` flag) removes every one of them from the runtime frame
    while a surviving group keeps its datasets. There is no API mutator for the flag, so this
    drives the same JSON path the editor produces; the batch slot + folder cascade themselves are
    exercised in-app.
    """
    api_client.create_new_project()
    time.sleep(0.2)

    keep = api_client.add_group("Keep", widget_type=0)
    api_client.add_dataset(keep, options=1)
    for title in ("HideA", "HideB", "HideC"):
        group = api_client.add_group(title, widget_type=0)
        api_client.add_dataset(group, options=1)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )
    api_client.set_frame_parser_code(JS_PASSTHROUGH, language=0, source_id=0)
    time.sleep(0.15)

    config = api_client.command("project.exportJson")["config"]
    _normalize_indices(config)
    for title in ("HideA", "HideB", "HideC"):
        _group_by_title(config, title)["disabled"] = True

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    data = _send_csv_and_read(api_client, device_simulator, [b"10,20,30,40"])
    groups = _runtime_groups(data)

    assert "Keep" in groups, "enabled group must survive a batch disable"
    for title in ("HideA", "HideB", "HideC"):
        assert title not in groups, f"batch-disabled group {title!r} must be excluded"
    assert groups["Keep"] != {}, "surviving group keeps its dataset"


@pytest.mark.integration
@pytest.mark.project
def test_all_disabled_project_does_not_crash(api_client, device_simulator, clean_state):
    """A project whose only groups are all disabled ingests frames without crashing."""
    config = _build_two_group_project(api_client)
    for group in config["groups"]:
        group["disabled"] = True

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    data = _send_csv_and_read(api_client, device_simulator, [b"10,20,30,40"])
    assert _runtime_groups(data) == {}, "no enabled groups -> empty runtime frame"

    assert (
        "frame" in api_client.get_dashboard_data()
    )  # API server still responsive post-ingest
