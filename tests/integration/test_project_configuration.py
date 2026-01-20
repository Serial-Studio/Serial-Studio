"""
Project Configuration Integration Tests

Tests configuring Serial Studio projects via the API.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import ChecksumType, DataGenerator


@pytest.mark.project
def test_operation_mode_configuration(api_client, clean_state):
    """
    Test setting different operation modes via API.

    Operation modes:
    0 = ProjectFile (manual JSON project definition)
    1 = DeviceSendsJSON (device sends JSON frames)
    2 = QuickPlot (CSV-based quick plotting)
    """
    # Get current operation mode
    status = api_client.get_dashboard_status()
    initial_mode = status.get("operationMode", 0)

    # Try setting to DeviceSendsJSON mode
    api_client.command("dashboard.setOperationMode", {"mode": 1})
    time.sleep(0.2)

    status = api_client.get_dashboard_status()
    assert status["operationMode"] == 1, "Should be in DeviceSendsJSON mode"

    # Try setting to QuickPlot mode
    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.2)

    status = api_client.get_dashboard_status()
    assert status["operationMode"] == 2, "Should be in QuickPlot mode"

    # Restore original mode
    api_client.command("dashboard.setOperationMode", {"mode": initial_mode})


@pytest.mark.project
def test_create_project_with_groups(api_client, device_simulator, clean_state):
    """
    Test creating a project structure via API (groups and datasets).

    Uses the new workflow:
    1. Create project via API
    2. Add groups/datasets via API
    3. Set parser code
    4. Load into FrameBuilder via API
    5. Send frames to validate dashboard works
    """
    # Create new project
    api_client.create_new_project()
    time.sleep(0.3)

    # Add a group with title and widget type
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    status = api_client.get_project_status()
    assert status["groupCount"] >= 1, "Should have added a group"

    # Add datasets to the group
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.1)

    status = api_client.get_project_status()
    assert status["datasetCount"] >= 3, "Should have at least 3 datasets"

    # Set JavaScript parser (returns array for ProjectFile mode)
    parser_code = """
function parse(frame) {
    return frame.split(',');
}
"""
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

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load the project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)

    assert result["loaded"], "Should have loaded into FrameBuilder"
    assert result["source"] == "API", "Source should be API"

    # Configure network and send test frames
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Send 10 frames to ensure dashboard model is built and widgets are generated
    frames = []
    for i in range(10):
        payload = f"{20.0 + i},{30.0 + i},{40.0 + i}"
        frame = DataGenerator.wrap_frame(
            payload,
            mode="project",
            checksum_type=ChecksumType.NONE
        )
        frames.append(frame)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # Verify connection is still active
    assert api_client.is_connected()

    # Cleanup
    api_client.disconnect_device()


@pytest.mark.project
def test_device_sends_json_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow with DeviceSendsJSON operation mode.

    In this mode, the device sends complete JSON frames with proper Frame keys:
    {"title": "...", "groups": [{"title": "...", "datasets": [...]}]}

    Frames are newline-delimited, no checksums or special delimiters needed.
    """
    # Configure for DeviceSendsJSON mode
    api_client.set_operation_mode("json")
    time.sleep(0.2)

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Create frames using DataGenerator - it handles delimiters automatically
    frames = []
    for i in range(10):
        frame_data = DataGenerator.generate_json_frame()
        payload = json.dumps(frame_data)
        # wrap_frame adds /* */ delimiters automatically for JSON mode
        frames.append(DataGenerator.wrap_frame(payload))

    # Connect and send data
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # Verify data was received
    assert api_client.is_connected()

    api_client.disconnect_device()


@pytest.mark.project
def test_quick_plot_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow with QuickPlot operation mode.

    In this mode, CSV data is automatically parsed and plotted.
    """
    # Configure for QuickPlot mode
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Create CSV frames (comma-separated values)
    frames = []
    for i in range(10):
        values = [
            20 + i,  # Temperature
            50 + i,  # Humidity
            1013 + i,  # Pressure
            37.77 + i * 0.001,  # Latitude
            -122.41 + i * 0.001,  # Longitude
        ]
        payload = ",".join(f"{v:.3f}" for v in values)
        # QuickPlot mode: simple newline-delimited CSV
        frame = payload.encode("utf-8") + b"\n"
        frames.append(frame)

    # Connect and send data
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # Verify data was received
    assert api_client.is_connected()

    api_client.disconnect_device()


@pytest.mark.project
def test_create_project_via_api(api_client, device_simulator, clean_state):
    """
    Test creating a complete project structure via API and sending data.
    """
    # Create new project
    api_client.create_new_project()
    time.sleep(0.3)

    # Add a group
    api_client.command("project.group.add", {"title": "Sensors", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets to the group
    for _ in range(5):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.05)

    # Verify project structure
    status = api_client.get_project_status()
    assert status["groupCount"] > 0, "Project should have groups"
    assert status["datasetCount"] >= 5, "Project should have at least 5 datasets"

    # Get groups list
    groups = api_client.command("project.groups.list")
    assert "groups" in groups, "Should return groups list"
    assert len(groups["groups"]) > 0, "Should have at least one group"

    # Get datasets list
    datasets = api_client.command("project.datasets.list")
    assert "datasets" in datasets, "Should return datasets list"

    # Set JavaScript parser
    parser_code = """
function parse(frame) {
    return frame.split(',');
}
"""
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

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network and send test frames
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Send 10 frames to build dashboard and generate widgets
    frames = []
    for i in range(10):
        payload = f"{10.0 + i},{20.0 + i},{30.0 + i},{40.0 + i},{50.0 + i}"
        frame = DataGenerator.wrap_frame(
            payload,
            mode="project",
            checksum_type=ChecksumType.NONE
        )
        frames.append(frame)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(2.5)

    # Verify connection
    assert api_client.is_connected()

    # Cleanup
    api_client.disconnect_device()


@pytest.mark.project
def test_project_parser_configuration(api_client, clean_state):
    """
    Test configuring custom frame parser via API.
    """
    # Define a simple JavaScript parser
    parser_code = """
//------------------------------------------------------------------------------
// Frame Parser Function
//------------------------------------------------------------------------------

/**
 * Parses a CSV frame into an array of values.
 *
 * @param {string} frame - The CSV data from the data source
 * @returns {array} Array of string values
 */
function parse(frame) {
    return frame.split(',');
}
"""

    # Set the parser code
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Get the parser code back
    result = api_client.command("project.parser.getCode")
    assert "code" in result, "Should return parser code"
    assert len(result["code"]) > 0, "Parser code should not be empty"


@pytest.mark.project
def test_project_with_actions(api_client, clean_state):
    """
    Test adding actions to a project via API.

    Actions are commands that can be triggered from the dashboard.
    """
    # Create new project
    api_client.create_new_project()
    time.sleep(0.3)

    # Add a group first (required for valid project)
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Get initial action count
    status = api_client.get_project_status()
    initial_actions = status.get("actionCount", 0)

    # Add an action
    api_client.command("project.action.add")
    time.sleep(0.2)

    # Get actions list
    actions = api_client.command("project.actions.list")
    assert "actions" in actions, "Should return actions list"


@pytest.mark.project
def test_full_project_configuration_workflow(api_client, device_simulator, clean_state):
    """
    Complete workflow: configure project, set mode, stream data, verify.
    """
    # 1. Configure for DeviceSendsJSON mode
    api_client.set_operation_mode("json")
    time.sleep(0.2)

    # 2. Configure dashboard
    api_client.set_dashboard_fps(30)
    api_client.set_dashboard_points(500)
    time.sleep(0.2)

    # 3. Configure network connection
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # 4. Generate test data
    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=2.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )

    # 5. Connect and stream data
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.5)

    # 6. Verify everything is working
    assert api_client.is_connected()

    status = api_client.get_dashboard_status()
    assert status["fps"] == 30
    assert status["points"] == 500

    # 7. Cleanup
    api_client.disconnect_device()
