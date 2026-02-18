"""
Project Import/Export Integration Tests

Tests for JSON-based project serialization: exporting the current project as
a JSON object and re-loading it via project.loadFromJSON. Exercises:
  - project.exportJson
  - project.loadFromJSON
  - project.getStatus (for structure verification)
  - project.groups.list / project.datasets.list

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator


def _build_simple_project(api_client, group_title="Sensors", dataset_count=3):
    """Helper: create a minimal project with one group and N datasets."""
    api_client.create_new_project(title="Export Test")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": group_title, "widgetType": 0})
    time.sleep(0.2)

    for _ in range(dataset_count):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.05)


@pytest.mark.project
def test_project_export_json_returns_object(api_client, clean_state):
    """Verify project.exportJson returns a non-empty JSON object."""
    _build_simple_project(api_client)

    result = api_client.command("project.exportJson")
    assert "config" in result, "exportJson should return a 'config' key"

    config = result["config"]
    assert isinstance(config, dict), "Exported config must be a dict"
    assert len(config) > 0, "Exported config must not be empty"


@pytest.mark.project
def test_project_export_json_has_required_fields(api_client, clean_state):
    """Verify exported JSON includes frameParser and groups fields."""
    _build_simple_project(api_client)

    result = api_client.command("project.exportJson")
    config = result["config"]

    assert "frameParser" in config, "Exported project must have 'frameParser'"
    assert "groups" in config, "Exported project must have 'groups'"
    assert isinstance(config["groups"], list), "'groups' must be a list"


@pytest.mark.project
def test_project_export_preserves_group_count(api_client, clean_state):
    """Verify exported project contains the same number of groups created via API."""
    api_client.create_new_project(title="Multi-Group Export")
    time.sleep(0.3)

    for i in range(3):
        api_client.command("project.group.add", {"title": f"Group {i}", "widgetType": 0})
        time.sleep(0.1)

    result = api_client.command("project.exportJson")
    config = result["config"]

    assert len(config["groups"]) == 3, "Exported JSON should contain 3 groups"


@pytest.mark.project
def test_project_load_from_json(api_client, clean_state):
    """Verify project.loadFromJSON loads a project from a JSON config object."""
    _build_simple_project(api_client, dataset_count=4)

    # Export current project
    result = api_client.command("project.exportJson")
    config = result["config"]

    # Reset project
    api_client.create_new_project(title="Blank")
    time.sleep(0.3)

    initial_status = api_client.get_project_status()
    initial_groups = initial_status.get("groupCount", 0)

    # Re-load from exported JSON
    load_result = api_client.command("project.loadFromJSON", {"config": config})
    time.sleep(0.3)

    assert load_result is not None, "loadFromJSON should return a result"

    loaded_status = api_client.get_project_status()
    assert loaded_status.get("groupCount", 0) >= 1, "Loaded project should have at least one group"


@pytest.mark.project
def test_project_roundtrip_preserves_structure(api_client, clean_state):
    """Export a project and re-import it; group count must match."""
    api_client.create_new_project(title="Roundtrip Test")
    time.sleep(0.3)

    for i in range(2):
        api_client.command("project.group.add", {"title": f"Channel {i}", "widgetType": 0})
        time.sleep(0.1)
        for _ in range(2):
            api_client.command("project.dataset.add", {"options": 0})
            time.sleep(0.05)

    # Capture original structure
    original_status = api_client.get_project_status()
    original_groups = original_status.get("groupCount", 0)

    # Export
    export_result = api_client.command("project.exportJson")
    config = export_result["config"]
    time.sleep(0.1)

    # Create blank project then re-import
    api_client.create_new_project(title="After Roundtrip")
    time.sleep(0.3)

    api_client.command("project.loadFromJSON", {"config": config})
    time.sleep(0.3)

    restored_status = api_client.get_project_status()
    restored_groups = restored_status.get("groupCount", 0)

    assert restored_groups == original_groups, (
        f"Group count should be preserved after roundtrip: "
        f"expected {original_groups}, got {restored_groups}"
    )


@pytest.mark.project
def test_project_load_from_json_and_stream_data(
    api_client, device_simulator, clean_state
):
    """Verify that a project loaded from JSON can parse and display live frames."""
    _build_simple_project(api_client, dataset_count=3)

    parser_code = "function parse(frame) { return frame.split(','); }"
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    # Export current project then reload it
    export_result = api_client.command("project.exportJson")
    config = export_result["config"]
    time.sleep(0.1)

    api_client.command("project.loadFromJSON", {"config": config})
    time.sleep(0.3)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result.get("loaded"), "Should load into FrameBuilder after JSON import"

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [
        DataGenerator.wrap_frame(
            f"{10.0 + i},{20.0 + i},{30.0 + i}",
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(10)
    ]

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    assert api_client.is_connected()
    api_client.disconnect_device()


@pytest.mark.project
def test_project_groups_list(api_client, clean_state):
    """Verify project.groups.list returns all created groups with titles."""
    api_client.create_new_project(title="List Test")
    time.sleep(0.3)

    group_titles = ["Alpha", "Beta", "Gamma"]
    for title in group_titles:
        api_client.command("project.group.add", {"title": title, "widgetType": 0})
        time.sleep(0.1)

    groups_result = api_client.command("project.groups.list")
    assert "groups" in groups_result, "groups.list should return 'groups' key"

    returned_titles = [g.get("title", "") for g in groups_result["groups"]]
    for title in group_titles:
        assert title in returned_titles, f"Group '{title}' should appear in groups.list"


@pytest.mark.project
def test_project_datasets_list(api_client, clean_state):
    """Verify project.datasets.list returns all datasets across all groups."""
    api_client.create_new_project(title="Datasets List Test")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": "Group A", "widgetType": 0})
    time.sleep(0.1)

    for _ in range(5):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.05)

    datasets_result = api_client.command("project.datasets.list")
    assert "datasets" in datasets_result, "datasets.list should return 'datasets' key"
    assert len(datasets_result["datasets"]) >= 5, "Should have at least 5 datasets"


@pytest.mark.project
def test_project_group_duplication(api_client, clean_state):
    """Verify project.group.duplicate increases the group count by one."""
    api_client.create_new_project(title="Duplicate Group Test")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": "Original Group", "widgetType": 0})
    time.sleep(0.2)

    status_before = api_client.get_project_status()
    count_before = status_before.get("groupCount", 0)

    api_client.command("project.group.duplicate")
    time.sleep(0.3)

    status_after = api_client.get_project_status()
    count_after = status_after.get("groupCount", 0)

    assert count_after == count_before + 1, (
        f"Group count should increase by 1 after duplicate: "
        f"before={count_before}, after={count_after}"
    )


@pytest.mark.project
def test_project_dataset_duplication(api_client, clean_state):
    """Verify project.dataset.duplicate increases the dataset count by one."""
    api_client.create_new_project(title="Duplicate Dataset Test")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)

    status_before = api_client.get_project_status()
    count_before = status_before.get("datasetCount", 0)

    api_client.command("project.dataset.duplicate")
    time.sleep(0.3)

    status_after = api_client.get_project_status()
    count_after = status_after.get("datasetCount", 0)

    assert count_after == count_before + 1, (
        f"Dataset count should increase by 1 after duplicate: "
        f"before={count_before}, after={count_after}"
    )


@pytest.mark.project
def test_project_frame_parser_config_roundtrip(api_client, clean_state):
    """Verify frame parser settings can be set and retrieved with matching values."""
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="CRC-16",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    config = api_client.get_frame_parser_config()
    assert config.get("startSequence") == "/*"
    assert config.get("endSequence") == "*/"
    assert config.get("frameDetection") == 1
