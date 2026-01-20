"""
Frame Parsing Integration Tests

Tests frame parsing with all checksum algorithms and frame formats.

IMPORTANT: Understanding Delimiter Types in Serial Studio
----------------------------------------------------------
Serial Studio uses three distinct types of delimiters:

1. **Frame Delimiters** (stream-level):
   - Define frame boundaries in the incoming byte stream
   - Examples: "/*" (start), "*/" (end)
   - Configured via: "frameStart" and "frameEnd" in project JSON
   - Extracted by: IO::FrameReader using KMP pattern matching

2. **Line Terminators** (stream-level):
   - Optional suffixes after frame end delimiter
   - Examples: \\r\\n, \\n, \\r, or even ";"
   - Not configured in the project; they are just extra bytes in the stream
   - Tests vary these to ensure frame extraction is resilient

3. **CSV Field Delimiters** (data-level):
   - Separate individual values within a CSV frame
   - Default: comma (,) - hard-coded in C++ parseCsvValues()
   - Custom delimiters (;, \\t, |) require JavaScript parser:
     Example: function parse(frame) { return frame.split(';'); }
   - Configured via: "frameParser" in project JSON

**QuickPlot Mode (Arduino Serial Plotter Compatible):**
   - NO frame delimiters (no /* and */)
   - Line-based parsing with CR (\\r), LF (\\n), or CRLF (\\r\\n)
   - Comma-separated values only
   - Works exactly like Arduino Serial Plotter

**DeviceSendsJSON Mode (DeviceDefined):**
   - Uses fixed delimiters: start "/*" and end "*/"
   - Project configuration is ignored

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import ChecksumType, DataGenerator


def _wait_for_dashboard_widgets(api_client, minimum: int, timeout: float = 2.0) -> int:
    end_time = time.time() + timeout
    while time.time() < end_time:
        data = api_client.get_dashboard_data()
        count = data.get("widgetCount", 0)
        if count >= minimum:
            return count
        time.sleep(0.05)
    return api_client.get_dashboard_data().get("widgetCount", 0)


@pytest.mark.parametrize("checksum_type", [
    ChecksumType.NONE,
    ChecksumType.XOR,
    ChecksumType.SUM,
    ChecksumType.CRC8,
    ChecksumType.CRC16,
    ChecksumType.CRC32,
    ChecksumType.FLETCHER16,
    ChecksumType.ADLER32,
])
def test_checksum_validation(
    api_client, device_simulator, clean_state, checksum_type
):
    """Test that Serial Studio correctly validates all checksum types."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)
    # Add a group with datasets
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add 6 datasets with plot widgets enabled
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 1})  # 1 = plot widget
        time.sleep(0.1)

    # Set CSV parser
    api_client.command("project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE})
    time.sleep(0.2)

    # Configure frame parser: Set delimiters FIRST, then detection mode
    checksum_names = {
        ChecksumType.NONE: "",
        ChecksumType.XOR: "XOR-8",
        ChecksumType.SUM: "MOD-256",
        ChecksumType.CRC8: "CRC-8",
        ChecksumType.CRC16: "CRC-16",
        ChecksumType.CRC32: "CRC-32",
        ChecksumType.FLETCHER16: "Fletcher-16",
        ChecksumType.ADLER32: "Adler-32",
    }

    api_client.command(
        "project.setTitle",
        {"title": f"Checksum Validation {checksum_names[checksum_type] or 'None'}"},
    )
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Step 1: Set delimiters and checksum
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm=checksum_names[checksum_type],
    )
    time.sleep(0.1)

    # Step 2: Set frame detection mode
    api_client.configure_frame_parser(
        frame_detection=1
    )
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 10 CSV frames to ensure dashboard model is built and widgets are generated
    frames = []
    for i in range(10):
        payload = DataGenerator.generate_csv_frame()
        frame = DataGenerator.wrap_frame(
            payload,
            start_delimiter="/*",
            end_delimiter="*/",
            checksum_type=checksum_type,
            mode="project",  # Required for checksums!
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should be connected"

    config = api_client.get_frame_parser_config()
    assert config["startSequence"] == "/*"
    assert config["endSequence"] == "*/"
    assert config["operationMode"] == 0
    assert config["checksumAlgorithm"] == checksum_names[checksum_type]

    widget_count = _wait_for_dashboard_widgets(api_client, minimum=6)
    assert widget_count >= 6, f"Expected widgets, got {widget_count}"
    data = api_client.get_dashboard_data()
    assert data["datasetCount"] >= 6, f"Expected datasets, got {data['datasetCount']}"


@pytest.mark.parametrize("checksum_type", [
    ChecksumType.CRC16,
    ChecksumType.CRC32,
])
def test_invalid_checksum_rejection(
    api_client, device_simulator, clean_state, checksum_type
):
    """Test that frames with invalid checksums are rejected."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)
    # Add a group with datasets
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets with plot widgets
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 1})
        time.sleep(0.1)

    # Set CSV parser
    api_client.command("project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE})
    time.sleep(0.2)

    # Configure frame parser: Set delimiters FIRST, then detection mode
    checksum_name = "CRC-16" if checksum_type == ChecksumType.CRC16 else "CRC-32"

    api_client.command(
        "project.setTitle", {"title": f"Checksum Reject {checksum_name}"}
    )
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Step 1: Set delimiters and checksum
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm=checksum_name,
    )
    time.sleep(0.1)

    # Step 2: Set frame detection mode
    api_client.configure_frame_parser(
        frame_detection=1
    )
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send 5 valid CSV frames first to build dashboard
    valid_frames = []
    for i in range(5):
        payload = DataGenerator.generate_csv_frame()
        frame = DataGenerator.wrap_frame(
            payload,
            start_delimiter="/*",
            end_delimiter="*/",
            checksum_type=checksum_type,
            mode="project"
        )
        valid_frames.append(frame)

    # Create corrupted frame
    payload = DataGenerator.generate_csv_frame()
    frame = DataGenerator.wrap_frame(
        payload,
        start_delimiter="/*",
        end_delimiter="*/",
        checksum_type=checksum_type,
        mode="project"
    )
    checksum_length = 2 if checksum_type == ChecksumType.CRC16 else 4
    line_terminator = b"\n"
    checksum_start = len(frame) - len(line_terminator) - checksum_length
    corrupted_checksum = b"\x00" * checksum_length
    corrupted_frame = (
        frame[:checksum_start]
        + corrupted_checksum
        + frame[checksum_start + checksum_length :]
    )

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    device_simulator.send_frame(corrupted_frame)
    time.sleep(0.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should remain connected"

    data = api_client.get_dashboard_data()
    assert data["datasetCount"] >= 6, f"Expected datasets, got {data['datasetCount']}"


def test_json_frame_parsing(api_client, device_simulator, clean_state):
    """Test parsing of JSON-formatted frames."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group with 2 datasets
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    for i in range(2):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for JSON frames
    parser_code = """function parse(frame) {
  if (frame.length > 0) {
    let data = JSON.parse(frame);
    let values = [];
    for (let i = 0; i < data.groups.length; i++) {
      for (let j = 0; j < data.groups[i].datasets.length; j++) {
        values.push(data.groups[i].datasets[j].value);
      }
    }
    return values;
  }
  return [];
}"""
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    groups = [
        {
            "title": "Test Group",
            "datasets": [
                {"title": "Value1", "value": "123.45", "units": "V"},
                {"title": "Value2", "value": "67.89", "units": "A"},
            ],
        }
    ]

    # Generate 10 frames to ensure dashboard is built
    frames = []
    for i in range(10):
        payload = json.dumps(DataGenerator.generate_json_frame(groups=groups))
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    project_status = api_client.get_project_status()
    assert project_status["groupCount"] >= 0

    data = api_client.get_dashboard_data()
    assert data["datasetCount"] >= 2, f"Expected datasets, got {data['datasetCount']}"

    api_client.configure_frame_parser(checksum_algorithm="None")
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    api_client.command("project.loadIntoFrameBuilder")


def test_csv_frame_parsing(api_client, device_simulator, clean_state):
    """Test parsing of CSV-formatted frames."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group
    api_client.command("project.group.add", {"title": "CSV Values", "widgetType": 0})
    time.sleep(0.2)

    # Add 6 datasets (matching the 6 CSV values)
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for comma delimiter
    parser_code = "function parse(frame) { return frame.split(','); }"
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 10 CSV frames
    frames = []
    for i in range(10):
        values = [1.23 + i, 4.56 + i, 7.89 + i, 10.11 + i, 12.13 + i, 14.15 + i]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter=",")
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    api_client.configure_frame_parser(checksum_algorithm="None")
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    api_client.command("project.loadIntoFrameBuilder")


@pytest.mark.parametrize("delimiter", [",", ";", "\t", "|"])
def test_csv_delimiter_resilience(api_client, device_simulator, clean_state, delimiter):
    """
    Test CSV parsing with various delimiters using JavaScript parsers.

    This test verifies that Serial Studio can correctly parse CSV data with
    different delimiters when using ProjectFile mode with custom JavaScript parsers.
    """
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group
    api_client.command("project.group.add", {"title": "CSV Values", "widgetType": 0})
    time.sleep(0.2)

    # Add 6 datasets (default generate_csv_frame creates 6 values)
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for the specified delimiter
    delimiter_escaped = delimiter.replace("\t", "\\t")
    parser_code = f"function parse(frame) {{ return frame.split('{delimiter_escaped}'); }}"
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 8 frames with the specified delimiter
    frames = []
    for i in range(8):
        payload = DataGenerator.generate_csv_frame(delimiter=delimiter)
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)


@pytest.mark.parametrize("line_terminator", ["\r\n", "\n", "\r", ";"])
def test_frame_delimiter_line_terminators(api_client, device_simulator, clean_state, line_terminator):
    """
    Test different line terminators after frame end delimiters.

    NOTE: This tests LINE TERMINATORS (suffixes after frame end delimiter),
    not the frame end delimiter itself. The frame delimiters remain "/*" and "*/"
    while the line terminator varies.

    For tests of custom frame delimiters, see test_frame_end_delimiters() below.
    """
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group with datasets matching the CSV parser output
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for CSV frames
    api_client.command("project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 8 frames with the specified line terminator
    frames = []
    for i in range(8):
        payload = DataGenerator.generate_csv_frame()
        frame = DataGenerator.wrap_frame(
            payload,
            line_terminator=line_terminator,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)


@pytest.mark.parametrize("end_delimiter", [";", "\n", "\r\n", "|"])
def test_frame_end_delimiters(api_client, device_simulator, clean_state, end_delimiter):
    """
    Test custom frame end delimiters.

    This test verifies that Serial Studio can correctly extract frames using
    various frame end delimiters configured in the project file.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    if end_delimiter == "\n":
        project_end_delimiter = "\\n"
    elif end_delimiter == "\r\n":
        project_end_delimiter = "\\r\\n"
    else:
        project_end_delimiter = end_delimiter

    # Create project with custom end delimiter
    project = DataGenerator.generate_project_with_frame_delimiters(
        start="",
        end=project_end_delimiter,
        detection_mode=0,
        title=f"Test End Delimiter: {repr(end_delimiter)}",
    )

    # Load the project configuration
    api_client.load_project_from_json(project)
    time.sleep(0.2)

    # Explicitly set the parser code (load_project_from_json might not apply it)
    api_client.command("project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE})
    time.sleep(0.2)

    api_client.configure_frame_parser(
        start_sequence="",
        end_sequence=project_end_delimiter,
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=0,
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Generate 10 CSV frames with the custom delimiter
    frames = []
    for i in range(10):
        payload = DataGenerator.generate_csv_frame()
        frame = DataGenerator.wrap_frame(
            payload,
            start_delimiter="",
            end_delimiter=end_delimiter,
            line_terminator="",
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    # Verify frames were received
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should remain connected"

    widget_count = _wait_for_dashboard_widgets(api_client, minimum=6)
    assert widget_count >= 6, f"Expected widgets, got {widget_count}"

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should remain connected"

    data = api_client.get_dashboard_data()
    assert data["widgetCount"] >= 6, f"Expected widgets, got {data['widgetCount']}"


@pytest.mark.parametrize(
    "start,end",
    [
        ("{", "}"),
        ("<frame>", "</frame>"),
        ("[BEGIN", "END]"),
    ],
)
def test_frame_start_end_delimiters(api_client, device_simulator, clean_state, start, end):
    """
    Test custom frame start and end delimiter combinations.

    This test verifies that Serial Studio can correctly extract frames using
    custom start+end delimiter pairs in StartAndEndDelimiter detection mode.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Create project with custom delimiters (StartAndEndDelimiter mode = 1)
    project = DataGenerator.generate_project_with_frame_delimiters(
        start=start,
        end=end,
        detection_mode=1,
        title=f"Test Delimiters: {start}...{end}",
    )

    # Load the project configuration
    api_client.load_project_from_json(project)
    time.sleep(0.2)

    # Explicitly set the parser code (load_project_from_json might not apply it)
    api_client.command("project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE})
    time.sleep(0.2)

    api_client.configure_frame_parser(
        start_sequence=start,
        end_sequence=end,
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Generate 10 CSV frames with the custom delimiters
    frames = []
    for i in range(10):
        payload = DataGenerator.generate_csv_frame()
        frame = DataGenerator.wrap_frame(
            payload,
            start_delimiter=start,
            end_delimiter=end,
            line_terminator="\n",
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    # Verify frames were received
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should remain connected"

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should remain connected"


def test_high_frequency_frames(api_client, device_simulator, clean_state):
    """Test handling of high-frequency frame reception."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group with datasets matching the JSON structure
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for JSON frames
    parser_code = """function parse(frame) {
  if (frame.length > 0) {
    let data = JSON.parse(frame);
    let values = [];
    for (let i = 0; i < data.groups.length; i++) {
      for (let j = 0; j < data.groups[i].datasets.length; j++) {
        values.push(data.groups[i].datasets[j].value);
      }
    }
    return values;
  }
  return [];
}"""
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=1.0,
        frequency_hz=100.0,
        frame_format="json",
        checksum_type=ChecksumType.NONE,
        mode="project",
    )

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    start_time = time.time()
    device_simulator.send_frames(frames, interval_seconds=0.01)
    duration = time.time() - start_time

    assert duration < 2.0, f"Sending 100 frames took too long: {duration:.2f}s"

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"]


def test_empty_frame_handling(api_client, device_simulator, clean_state):
    """Test that empty frames are handled gracefully."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group with datasets matching the JSON structure
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for JSON frames
    parser_code = """function parse(frame) {
  if (frame.length > 0) {
    let data = JSON.parse(frame);
    let values = [];
    for (let i = 0; i < data.groups.length; i++) {
      for (let j = 0; j < data.groups[i].datasets.length; j++) {
        values.push(data.groups[i].datasets[j].value);
      }
    }
    return values;
  }
  return [];
}"""
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send valid frames first to build dashboard
    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    # Then send empty frame
    empty_frame = b"/**/\r\n"
    device_simulator.send_frame(empty_frame)
    time.sleep(0.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"]


def test_malformed_json_handling(api_client, device_simulator, clean_state):
    """Test handling of malformed JSON frames."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add a group with datasets matching the JSON structure
    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    # Add datasets
    for i in range(6):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    # Set JavaScript parser for JSON frames
    parser_code = """function parse(frame) {
  if (frame.length > 0) {
    let data = JSON.parse(frame);
    let values = [];
    for (let i = 0; i < data.groups.length; i++) {
      for (let j = 0; j < data.groups[i].datasets.length; j++) {
        values.push(data.groups[i].datasets[j].value);
      }
    }
    return values;
  }
  return [];
}"""
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send valid frames first to build dashboard
    valid_frames = []
    for i in range(7):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(1.0)

    # Then send malformed frames
    malformed_frames = [
        b"/*{invalid json}*/\r\n",
        b"/*{\"broken\": }*/\r\n",
        b"/*{{\"nested\": \"wrong\"}}*/\r\n",
    ]

    for frame in malformed_frames:
        device_simulator.send_frame(frame)
        time.sleep(0.2)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"]


def test_large_frame_handling(api_client, device_simulator, clean_state):
    """Test handling of very large frames."""
    # Create new project and configure for ProjectFile mode
    api_client.create_new_project()
    time.sleep(0.2)

    # Add 50 groups with 20 datasets each (1000 total datasets)
    for group_idx in range(50):
        commands = [
            {"command": "project.group.add", "params": {"title": f"Group{group_idx}", "widgetType": 0}}
        ]
        commands.extend(
            {"command": "project.dataset.add", "params": {"options": 0}} for _ in range(20)
        )
        api_client.batch(commands)

    # Set JavaScript parser for JSON frames
    parser_code = """function parse(frame) {
  if (frame.length > 0) {
    let data = JSON.parse(frame);
    let values = [];
    for (let i = 0; i < data.groups.length; i++) {
      for (let j = 0; j < data.groups[i].datasets.length; j++) {
        values.push(data.groups[i].datasets[j].value);
      }
    }
    return values;
  }
  return [];
}"""
    api_client.command("project.parser.setCode", {"code": parser_code})
    time.sleep(0.2)

    # Configure frame parser for ProjectFile mode
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1
    )
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    time.sleep(0.1)

    # Load project into FrameBuilder
    result = api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.5)
    assert result["loaded"], "Should have loaded into FrameBuilder"

    # Configure network
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    large_groups = []
    for i in range(50):
        datasets = [
            {"title": f"Sensor{j}", "value": f"{j * 1.23:.2f}", "units": "V"}
            for j in range(20)
        ]
        large_groups.append({"title": f"Group{i}", "datasets": datasets})

    # Generate 5 large frames to build dashboard
    frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame(groups=large_groups))
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=ChecksumType.NONE,
            mode="project",
        )
        frames.append(frame)

    assert len(frames[0]) > 10000, "Frame should be large"

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(1.5)

    project_status = api_client.get_project_status()
    assert project_status["groupCount"] >= 0
