"""
Source Configuration API Tests

Tests for project.source.configure and project.source.setProperty commands
that allow API callers to configure device connection settings into project
sources — mirroring what the GUI Setup panel does.

Covers:
- project.source.configure (multi-property batch update)
- project.source.setProperty (single-property update)
- Both route through the UI-config driver for source 0 in single-source
  ProjectFile mode, so changes propagate to the Setup panel and persist
  in source[0].connectionSettings.
- project.source.getConfiguration returns the persisted settings.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils import DeviceSimulator


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _new_project(api_client, dataset_count: int = 3):
    """Create a new project in ProjectFile mode with one group and dataset_count datasets."""
    api_client.command("project.file.new")
    time.sleep(0.1)
    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)
    api_client.command("project.group.add", {"title": "G", "widgetType": 0})
    time.sleep(0.1)
    for _ in range(dataset_count):
        api_client.command("project.dataset.add", {"options": 1})
        time.sleep(0.1)


def _connect_network(api_client, device_simulator):
    """Configure network driver via project.source.configure, then connect."""
    # Set network bus type first so activeUiDriver() returns m_networkUi
    api_client.command("io.manager.setBusType", {"busType": 1})
    time.sleep(0.1)

    api_client.source_configure(0, {
        "address": "127.0.0.1",
        "tcpPort": 9000,
        "socketTypeIndex": 0,
    })
    time.sleep(0.1)

    api_client.configure_frame_parser(
        start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
    )
    api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.2)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), (
        "Serial Studio did not connect to the device simulator within 5 seconds"
    )


def _send_json_frame(api_client, device_simulator, values: list):
    """Send a JSON frame with the given dataset values."""
    payload = json.dumps({"values": values})
    frame = f"/*{payload}*/".encode()
    device_simulator.send_frame(frame)
    time.sleep(0.3)


def _get_dataset_values(api_client) -> list:
    """Return current dataset values from the first group."""
    data = api_client.get_dashboard_data()
    groups = data.get("frame", {}).get("groups", [])
    if not groups:
        return []
    return [d.get("value", "") for d in groups[0].get("datasets", [])]


def _wait_for_values(api_client, expected: list, timeout: float = 3.0) -> list:
    """Poll until dashboard shows the expected values."""
    end = time.time() + timeout
    while time.time() < end:
        values = _get_dataset_values(api_client)
        if values == expected:
            return values
        time.sleep(0.05)
    return _get_dataset_values(api_client)


# ---------------------------------------------------------------------------
# Tests: project.source.configure
# ---------------------------------------------------------------------------

class TestSourceConfigure:
    """Tests for the project.source.configure command."""

    def test_configure_command_exists(self, api_client, clean_state):
        """project.source.configure must be registered in the API."""
        assert api_client.command_exists("project.source.configure"), (
            "project.source.configure command is not registered"
        )

    def test_configure_accepted_without_error(self, api_client, clean_state):
        """project.source.configure must succeed without raising an error."""
        _new_project(api_client, dataset_count=1)

        # Set bus type to network first so the UI driver is the right type
        api_client.command("io.manager.setBusType", {"busType": 1})
        time.sleep(0.1)

        # This must not raise; result is not checked here (persistence only applies
        # when a project file is saved to disk)
        api_client.source_configure(0, {
            "address": "127.0.0.1",
            "tcpPort": 9000,
            "socketTypeIndex": 0,
        })

    def test_configure_with_missing_params_returns_error(self, api_client, clean_state):
        """configure without required params must return an API error."""
        from utils.api_client import APIError

        with pytest.raises(APIError) as exc_info:
            api_client.command("project.source.configure", {"sourceId": 0})

        assert exc_info.value.code == "MISSING_PARAM"

    def test_configure_with_invalid_source_returns_error(self, api_client, clean_state):
        """configure with a non-existent sourceId must return an API error."""
        from utils.api_client import APIError

        with pytest.raises(APIError) as exc_info:
            api_client.command(
                "project.source.configure",
                {"sourceId": 99, "settings": {"address": "127.0.0.1"}},
            )

        assert exc_info.value.code in ("INVALID_PARAM", "OPERATION_FAILED")

    def test_configure_end_to_end_with_device(
        self, api_client, device_simulator, clean_state
    ):
        """Configure network via source.configure then connect and receive data."""
        _new_project(api_client)

        js_code = """
function parse(frame) {
    var data = JSON.parse(frame);
    return data.values;
}
"""
        api_client.command("project.parser.setCode", {"code": js_code})
        time.sleep(0.1)

        _connect_network(api_client, device_simulator)

        _send_json_frame(api_client, device_simulator, [7.7, 8.8, 9.9])

        expected = ["7.7", "8.8", "9.9"]
        values = _wait_for_values(api_client, expected)
        assert values == expected, f"Expected {expected}, got {values}"


# ---------------------------------------------------------------------------
# Tests: project.source.setProperty (updated routing)
# ---------------------------------------------------------------------------

class TestSourceSetProperty:
    """Tests for the updated project.source.setProperty command."""

    def test_set_property_command_exists(self, api_client, clean_state):
        """project.source.setProperty must be registered in the API."""
        assert api_client.command_exists("project.source.setProperty")

    def test_set_property_missing_params_returns_error(self, api_client, clean_state):
        """setProperty without all required params must return MISSING_PARAM."""
        from utils.api_client import APIError

        with pytest.raises(APIError) as exc_info:
            api_client.command("project.source.setProperty", {"sourceId": 0})

        assert exc_info.value.code == "MISSING_PARAM"

    def test_set_property_invalid_source_returns_error(self, api_client, clean_state):
        """setProperty with a non-existent sourceId must fail."""
        from utils.api_client import APIError

        with pytest.raises(APIError):
            api_client.command(
                "project.source.setProperty",
                {"sourceId": 99, "key": "address", "value": "127.0.0.1"},
            )

    def test_set_property_single_key_end_to_end(
        self, api_client, device_simulator, clean_state
    ):
        """Set network properties one by one, connect, and receive data."""
        _new_project(api_client)

        js_code = """
function parse(frame) {
    var d = JSON.parse(frame);
    return d.values;
}
"""
        api_client.command("project.parser.setCode", {"code": js_code})
        time.sleep(0.1)

        api_client.command("io.manager.setBusType", {"busType": 1})
        time.sleep(0.1)

        # Set properties individually via setProperty (routes through UI driver)
        api_client.source_set_property(0, "address", "127.0.0.1")
        time.sleep(0.05)
        api_client.source_set_property(0, "tcpPort", 9000)
        time.sleep(0.05)
        api_client.source_set_property(0, "socketTypeIndex", 0)
        time.sleep(0.05)

        api_client.configure_frame_parser(
            start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
        )
        api_client.command("project.loadIntoFrameBuilder")
        time.sleep(0.2)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0), (
            "Serial Studio did not connect after setProperty configuration"
        )

        _send_json_frame(api_client, device_simulator, [1.1, 2.2, 3.3])

        expected = ["1.1", "2.2", "3.3"]
        values = _wait_for_values(api_client, expected)
        assert values == expected, f"Expected {expected}, got {values}"


# ---------------------------------------------------------------------------
# Tests: source.getConfiguration reflects applied settings
# ---------------------------------------------------------------------------

class TestSourceGetConfiguration:
    """Verify project.source.getConfiguration returns the full source struct."""

    def test_get_configuration_returns_source_fields(self, api_client, clean_state):
        """getConfiguration must return all standard source fields."""
        _new_project(api_client, dataset_count=1)

        cfg = api_client.source_get_configuration(0)

        assert "sourceId" in cfg
        assert "busType" in cfg
        assert cfg["sourceId"] == 0

    def test_get_configuration_invalid_source_returns_error(
        self, api_client, clean_state
    ):
        """getConfiguration with an invalid sourceId must return INVALID_PARAM."""
        from utils.api_client import APIError

        with pytest.raises(APIError) as exc_info:
            api_client.source_get_configuration(99)

        assert exc_info.value.code == "INVALID_PARAM"
