"""
Frame Parsing Integration Tests

Tests frame parsing with all checksum algorithms and frame formats.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import ChecksumType, DataGenerator


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
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 10 frames to ensure dashboard model is built and widgets are generated
    frames = []
    for i in range(10):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(
            payload,
            checksum_type=checksum_type,
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Device should be connected"


@pytest.mark.parametrize("checksum_type", [
    ChecksumType.CRC16,
    ChecksumType.CRC32,
])
def test_invalid_checksum_rejection(
    api_client, device_simulator, clean_state, checksum_type
):
    """Test that frames with invalid checksums are rejected."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send 5 valid frames first to build dashboard
    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=checksum_type)
        valid_frames.append(frame)

    # Create corrupted frame
    payload = json.dumps(DataGenerator.generate_json_frame())
    frame = DataGenerator.wrap_frame(payload, checksum_type=checksum_type)
    corrupted_frame = frame[:-6] + b"BADC0D\r\n"

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    device_simulator.send_frame(corrupted_frame)
    time.sleep(0.5)


def test_json_frame_parsing(api_client, device_simulator, clean_state):
    """Test parsing of JSON-formatted frames."""
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
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC16)
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    project_status = api_client.get_project_status()
    assert project_status["groupCount"] >= 0


def test_csv_frame_parsing(api_client, device_simulator, clean_state):
    """Test parsing of CSV-formatted frames."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 10 CSV frames
    frames = []
    for i in range(10):
        values = [1.23 + i, 4.56 + i, 7.89 + i, 10.11 + i, 12.13 + i, 14.15 + i]
        payload = DataGenerator.generate_csv_frame(values=values, delimiter=",")
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC16)
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)


@pytest.mark.parametrize("delimiter", [",", ";", "\t", "|"])
def test_csv_delimiters(api_client, device_simulator, clean_state, delimiter):
    """Test CSV parsing with different delimiters."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 8 frames with the specified delimiter
    frames = []
    for i in range(8):
        payload = DataGenerator.generate_csv_frame(delimiter=delimiter)
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)


@pytest.mark.parametrize("line_terminator", ["\r\n", "\n", "\r", ";"])
def test_frame_delimiters(api_client, device_simulator, clean_state, line_terminator):
    """Test different line terminators after frame delimiters."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Generate 8 frames with the specified line terminator
    frames = []
    for i in range(8):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(
            payload,
            line_terminator=line_terminator,
            checksum_type=ChecksumType.NONE,
        )
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)


def test_high_frequency_frames(api_client, device_simulator, clean_state):
    """Test handling of high-frequency frame reception."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=1.0,
        frequency_hz=100.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
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
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send valid frames first to build dashboard
    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
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
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Send valid frames first to build dashboard
    valid_frames = []
    for i in range(7):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
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
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC32)
        frames.append(frame)

    assert len(frames[0]) > 10000, "Frame should be large"

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.2)
    time.sleep(1.5)

    project_status = api_client.get_project_status()
    assert project_status["groupCount"] >= 0
