"""
2D Array and Mixed Scalar/Vector Frame Parsing Tests

Tests multi-frame parsing when JavaScript returns:
- Pure 2D arrays: [[1,2,3], [4,5,6]]
- Mixed scalar/vector arrays: [scalar, [vec1, vec2, vec3]]
- Multiple vectors with different lengths

This feature enables efficient BLE packet parsing where batched samples
are automatically expanded into individual frames without JavaScript
code duplication.

Example BLE use case:
  [humidity, temp, [accel1...accel120]] → 120 frames of [humidity, temp, accelN]

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import DataGenerator


def _wait_for_frames(api_client, minimum: int, timeout: float = 2.0) -> int:
    """Wait for dashboard to receive at least minimum frames."""
    end_time = time.time() + timeout
    while time.time() < end_time:
        data = api_client.get_dashboard_data()
        count = data.get("rxFrames", 0)
        if count >= minimum:
            return count
        time.sleep(0.05)
    return api_client.get_dashboard_data().get("rxFrames", 0)


def _create_project_with_datasets(api_client, dataset_count: int):
    """Create a project with specified number of datasets."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.group.add", {"title": "Test Group", "widgetType": 0})
    time.sleep(0.2)

    for i in range(dataset_count):
        api_client.command("project.dataset.add", {"options": 1})
        time.sleep(0.1)


def _configure_js_parser(api_client, js_code: str):
    """Configure JavaScript parser code."""
    api_client.command("project.parser.setCode", {"code": js_code})
    time.sleep(0.2)


def _send_frame(api_client, device_simulator, payload: dict):
    """Send a JSON frame to Serial Studio."""
    frame = f"/*{json.dumps(payload)}*/"
    device_simulator.send(frame.encode())
    time.sleep(0.3)


class TestBasic2DArrayParsing:
    """Test pure 2D array parsing: [[1,2,3], [4,5,6]]"""

    def test_2d_array_generates_multiple_frames(
        self, api_client, device_simulator, clean_state
    ):
        """Test [[1,2,3], [4,5,6]] generates 2 frames."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"values": [[1.1, 2.2, 3.3], [4.4, 5.5, 6.6]]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 2)
        assert (
            final_frames >= initial_frames + 2
        ), f"Expected at least 2 new frames, got {final_frames - initial_frames}"

        data = api_client.get_dashboard_data()
        groups = data.get("groups", [])
        assert len(groups) > 0, "Should have at least one group"

        datasets = groups[0].get("datasets", [])
        assert len(datasets) == 3, "Should have 3 datasets"

        assert datasets[0]["value"] == "4.4", "Dataset 0 should have last frame value"
        assert datasets[1]["value"] == "5.5", "Dataset 1 should have last frame value"
        assert datasets[2]["value"] == "6.6", "Dataset 2 should have last frame value"

    def test_2d_array_with_different_row_lengths(
        self, api_client, device_simulator, clean_state
    ):
        """Test 2D array with rows of different lengths."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"values": [[1.1, 2.2], [3.3, 4.4, 5.5], [6.6]]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 3)
        assert final_frames >= initial_frames + 3, "Expected at least 3 new frames"


class TestMixedScalarVectorParsing:
    """Test mixed scalar/vector parsing: [scalar1, scalar2, [vec1, vec2, vec3]]"""

    def test_mixed_scalar_single_vector(
        self, api_client, device_simulator, clean_state
    ):
        """Test [humidity, temp, [accel1, accel2, accel3]] generates 3 frames."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.humidity, data.temp, data.accel];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"humidity": 25.5, "temp": 60.0, "accel": [1.1, 2.2, 3.3]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 3)
        assert final_frames >= initial_frames + 3, "Expected at least 3 new frames"

        data = api_client.get_dashboard_data()
        groups = data.get("groups", [])
        datasets = groups[0].get("datasets", [])

        assert (
            datasets[0]["value"] == "25.5"
        ), "Scalar should repeat across all frames"
        assert (
            datasets[1]["value"] == "60.0"
        ), "Scalar should repeat across all frames"
        assert datasets[2]["value"] == "3.3", "Last vector element in last frame"

    def test_mixed_scalar_multiple_vectors_same_length(
        self, api_client, device_simulator, clean_state
    ):
        """Test [s, [v1,v2], [v3,v4]] generates 2 frames."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.scalar, data.vec1, data.vec2];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"scalar": 100.0, "vec1": [1.1, 2.2], "vec2": [3.3, 4.4]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 2)
        assert final_frames >= initial_frames + 2, "Expected at least 2 new frames"

    def test_mixed_scalar_multiple_vectors_different_lengths(
        self, api_client, device_simulator, clean_state
    ):
        """Test vector extension: [s, [v1,v2,v3], [v4,v5]] extends shorter vector."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.scalar, data.long_vec, data.short_vec];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {
            "scalar": 99.9,
            "long_vec": [1.1, 2.2, 3.3],
            "short_vec": [8.8, 9.9],
        }
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 3)
        assert (
            final_frames >= initial_frames + 3
        ), "Expected 3 frames (max vector length)"


class TestBLEUseCaseSimulation:
    """Test realistic BLE packet parsing scenario."""

    def test_large_ble_packet_120_samples(
        self, api_client, device_simulator, clean_state
    ):
        """Test [humidity, temp, [accel1...accel120]] performance."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.humidity, data.temp, data.accel_samples];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        accel_samples = [float(i) * 0.1 for i in range(120)]
        payload = {"humidity": 45.2, "temp": 22.5, "accel_samples": accel_samples}

        start_time = time.time()
        _send_frame(api_client, device_simulator, payload)
        processing_time = time.time() - start_time

        final_frames = _wait_for_frames(api_client, initial_frames + 120, timeout=3.0)
        assert (
            final_frames >= initial_frames + 120
        ), f"Expected 120 frames, got {final_frames - initial_frames}"

        assert (
            processing_time < 1.0
        ), f"Processing should be fast, took {processing_time:.3f}s"


class TestEdgeCases:
    """Test edge cases and error handling."""

    def test_empty_array_returns_no_frames(
        self, api_client, device_simulator, clean_state
    ):
        """Test empty array [] returns no frames."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    return [];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"test": "data"}
        _send_frame(api_client, device_simulator, payload)

        time.sleep(0.3)
        final_frames = api_client.get_dashboard_data().get("rxFrames", 0)
        assert (
            final_frames == initial_frames
        ), "Empty array should not generate frames"

    def test_mixed_with_empty_vectors(
        self, api_client, device_simulator, clean_state
    ):
        """Test [s, [], [v1,v2]] skips empty vector."""
        _create_project_with_datasets(api_client, 2)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.scalar, [], data.vec];
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"scalar": 50.0, "vec": [1.1, 2.2]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 2)
        assert final_frames >= initial_frames + 2, "Empty vectors should be skipped"


class TestBackwardCompatibility:
    """Test backward compatibility with existing 1D array behavior."""

    def test_1d_array_generates_single_frame(
        self, api_client, device_simulator, clean_state
    ):
        """Test [1, 2, 3] still generates single frame (no regression)."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = {"values": [1.1, 2.2, 3.3]}
        _send_frame(api_client, device_simulator, payload)

        final_frames = _wait_for_frames(api_client, initial_frames + 1)
        assert (
            final_frames >= initial_frames + 1
        ), "1D array should generate exactly 1 frame"

        data = api_client.get_dashboard_data()
        groups = data.get("groups", [])
        datasets = groups[0].get("datasets", [])

        assert datasets[0]["value"] == "1.1", "Value should match first element"
        assert datasets[1]["value"] == "2.2", "Value should match second element"
        assert datasets[2]["value"] == "3.3", "Value should match third element"

    def test_csv_mode_unchanged(self, api_client, device_simulator, clean_state):
        """Test CSV mode still works as before (no parseMultiFrame)."""
        _create_project_with_datasets(api_client, 3)

        api_client.command(
            "project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE}
        )
        time.sleep(0.2)

        api_client.set_operation_mode("project")
        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        time.sleep(0.5)

        initial_frames = api_client.get_dashboard_data().get("rxFrames", 0)

        payload = DataGenerator.generate_csv_frame()
        frame = f"/*{payload}*/"
        device_simulator.send(frame.encode())
        time.sleep(0.3)

        final_frames = _wait_for_frames(api_client, initial_frames + 1)
        assert final_frames >= initial_frames + 1, "CSV should generate 1 frame"
