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

from utils import DataGenerator


def _get_dataset_values(api_client, group_idx: int = 0) -> list:
    """Get current dataset values from the dashboard."""
    data = api_client.get_dashboard_data()
    groups = data.get("frame", {}).get("groups", [])
    if not groups or group_idx >= len(groups):
        return []
    return [d.get("value", "") for d in groups[group_idx].get("datasets", [])]


def _wait_for_dataset_values(
    api_client, expected: list, group_idx: int = 0, timeout: float = 3.0
) -> list:
    """Wait until dashboard datasets show the expected values."""
    end_time = time.time() + timeout
    while time.time() < end_time:
        values = _get_dataset_values(api_client, group_idx)
        if values == expected:
            return values
        time.sleep(0.05)
    return _get_dataset_values(api_client, group_idx)


def _wait_for_any_frame(api_client, group_idx: int = 0, timeout: float = 3.0) -> bool:
    """Wait until any non-empty dataset value appears."""
    end_time = time.time() + timeout
    while time.time() < end_time:
        values = _get_dataset_values(api_client, group_idx)
        if any(v for v in values):
            return True
        time.sleep(0.05)
    return False


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
    device_simulator.send_frame(frame.encode())
    time.sleep(0.3)


def _connect_device(api_client):
    """Configure project mode, load into frame builder, and connect device."""
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
    )
    api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    time.sleep(0.5)


class TestBasic2DArrayParsing:
    """Test pure 2D array parsing: [[1,2,3], [4,5,6]]"""

    def test_2d_array_generates_multiple_frames(
        self, api_client, device_simulator, clean_state
    ):
        """Test [[1,2,3], [4,5,6]] generates 2 frames; last frame values are verified."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)
        _connect_device(api_client)

        payload = {"values": [[1.1, 2.2, 3.3], [4.4, 5.5, 6.6]]}
        _send_frame(api_client, device_simulator, payload)

        # Last row is [4.4, 5.5, 6.6]; JS number-to-string keeps decimals
        expected = ["4.4", "5.5", "6.6"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, f"Expected last-frame values {expected}, got {values}"

    def test_2d_array_with_different_row_lengths(
        self, api_client, device_simulator, clean_state
    ):
        """Test 2D array with rows of different lengths; last row drives final values."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)
        _connect_device(api_client)

        payload = {"values": [[1.1, 2.2], [3.3, 4.4, 5.5], [6.6]]}
        _send_frame(api_client, device_simulator, payload)

        # At least one frame must be received
        received = _wait_for_any_frame(api_client)
        assert received, "Should have received at least one frame from the 2D array"

        # Last row is [6.6], so dataset[0] must end up as "6.6"
        values = _get_dataset_values(api_client)
        assert len(values) >= 1 and values[0] == "6.6", (
            f"First dataset should reflect last-row value 6.6, got {values}"
        )


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
        _connect_device(api_client)

        payload = {"humidity": 25.5, "temp": 60.0, "accel": [1.1, 2.2, 3.3]}
        _send_frame(api_client, device_simulator, payload)

        # Last frame: [25.5, 60, 3.3] — JS renders 60.0 as "60" (integer string)
        expected = ["25.5", "60", "3.3"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, (
            f"Expected last-frame values {expected}, got {values}"
        )

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
        _connect_device(api_client)

        payload = {"scalar": 100.0, "vec1": [1.1, 2.2], "vec2": [3.3, 4.4]}
        _send_frame(api_client, device_simulator, payload)

        # Last frame: [100, 2.2, 4.4] — JS renders 100.0 as "100"
        expected = ["100", "2.2", "4.4"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, (
            f"Expected last-frame values {expected}, got {values}"
        )

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
        _connect_device(api_client)

        payload = {
            "scalar": 99.9,
            "long_vec": [1.1, 2.2, 3.3],
            "short_vec": [8.8, 9.9],
        }
        _send_frame(api_client, device_simulator, payload)

        # short_vec is extended by repeating last value: [8.8, 9.9, 9.9]
        # Last frame (index 2): [99.9, 3.3, 9.9]
        expected = ["99.9", "3.3", "9.9"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, (
            f"Expected last-frame values {expected}, got {values}"
        )


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
        _connect_device(api_client)

        accel_samples = [float(i) * 0.1 for i in range(120)]
        payload = {"humidity": 45.2, "temp": 22.5, "accel_samples": accel_samples}

        start_time = time.time()
        _send_frame(api_client, device_simulator, payload)
        processing_time = time.time() - start_time

        # Last frame (index 119): [45.2, 22.5, 11.9]
        expected = ["45.2", "22.5", "11.9"]
        values = _wait_for_dataset_values(api_client, expected, timeout=3.0)
        assert values == expected, (
            f"Expected BLE last-frame values {expected}, got {values}"
        )
        assert processing_time < 1.0, (
            f"Processing should complete within 1 second, took {processing_time:.3f}s"
        )


class TestEdgeCases:
    """Test edge cases and error handling."""

    def test_empty_array_returns_no_frames(
        self, api_client, device_simulator, clean_state
    ):
        """Test empty array [] does not update dashboard dataset values."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    return [];
}
"""
        _configure_js_parser(api_client, js_code)
        _connect_device(api_client)

        values_before = _get_dataset_values(api_client)

        payload = {"test": "data"}
        _send_frame(api_client, device_simulator, payload)

        time.sleep(0.5)
        values_after = _get_dataset_values(api_client)
        assert values_after == values_before, (
            f"Empty array should not change dataset values; "
            f"before={values_before}, after={values_after}"
        )

    def test_mixed_with_empty_vectors(
        self, api_client, device_simulator, clean_state
    ):
        """Test [s, [], [v1,v2]] skips empty vector; 2 frames generated."""
        _create_project_with_datasets(api_client, 2)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return [data.scalar, [], data.vec];
}
"""
        _configure_js_parser(api_client, js_code)
        _connect_device(api_client)

        payload = {"scalar": 50.0, "vec": [1.1, 2.2]}
        _send_frame(api_client, device_simulator, payload)

        # Empty vector is skipped; scalars=[50], vectors=[[1.1,2.2]]
        # Last frame (index 1): [50, 2.2] — JS renders 50.0 as "50"
        expected = ["50", "2.2"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, (
            f"Expected last-frame values {expected}, got {values}"
        )


class TestBackwardCompatibility:
    """Test backward compatibility with existing 1D array behavior."""

    def test_1d_array_generates_single_frame(
        self, api_client, device_simulator, clean_state
    ):
        """Test [1, 2, 3] still generates a single frame (no regression)."""
        _create_project_with_datasets(api_client, 3)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        _configure_js_parser(api_client, js_code)
        _connect_device(api_client)

        payload = {"values": [1.1, 2.2, 3.3]}
        _send_frame(api_client, device_simulator, payload)

        expected = ["1.1", "2.2", "3.3"]
        values = _wait_for_dataset_values(api_client, expected)
        assert values == expected, (
            f"1D array should produce a single frame with values {expected}, got {values}"
        )

    def test_csv_mode_unchanged(self, api_client, device_simulator, clean_state):
        """Test CSV mode still works as before (no parseMultiFrame)."""
        _create_project_with_datasets(api_client, 3)

        api_client.command(
            "project.parser.setCode", {"code": DataGenerator.CSV_PARSER_TEMPLATE}
        )
        time.sleep(0.2)

        _connect_device(api_client)

        payload = DataGenerator.generate_csv_frame()
        frame = f"/*{payload}*/"
        device_simulator.send_frame(frame.encode())
        time.sleep(0.3)

        received = _wait_for_any_frame(api_client)
        assert received, "CSV frame should populate at least one dataset value"
