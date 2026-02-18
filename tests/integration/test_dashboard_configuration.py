"""
Dashboard Configuration Integration Tests

Tests for dashboard FPS, data-point count, operation mode switching, and
data/status queries. Covers commands:
  - dashboard.setFPS / dashboard.getFPS
  - dashboard.setPoints / dashboard.getPoints
  - dashboard.setOperationMode / dashboard.getOperationMode
  - dashboard.getStatus
  - dashboard.getData

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator


@pytest.mark.integration
def test_dashboard_fps_set_and_get(api_client, clean_state):
    """Verify dashboard FPS can be set and read back via getFPS."""
    for fps in [15, 30, 60]:
        api_client.command("dashboard.setFPS", {"fps": fps})
        time.sleep(0.1)

        result = api_client.command("dashboard.getFPS")
        assert result.get("fps") == fps


@pytest.mark.integration
def test_dashboard_fps_boundary_values(api_client, clean_state):
    """Verify dashboard FPS accepts boundary values 1 and 240."""
    api_client.command("dashboard.setFPS", {"fps": 1})
    time.sleep(0.1)
    result = api_client.command("dashboard.getFPS")
    assert result.get("fps") == 1

    api_client.command("dashboard.setFPS", {"fps": 240})
    time.sleep(0.1)
    result = api_client.command("dashboard.getFPS")
    assert result.get("fps") == 240


@pytest.mark.integration
def test_dashboard_fps_invalid(api_client, clean_state):
    """Verify out-of-range FPS values are rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("dashboard.setFPS", {"fps": 0})

    with pytest.raises(APIError):
        api_client.command("dashboard.setFPS", {"fps": 241})


@pytest.mark.integration
def test_dashboard_points_set_and_get(api_client, clean_state):
    """Verify data-point count can be set and read back via getPoints."""
    for points in [100, 500, 1000, 5000]:
        api_client.command("dashboard.setPoints", {"points": points})
        time.sleep(0.1)

        result = api_client.command("dashboard.getPoints")
        assert result.get("points") == points


@pytest.mark.integration
def test_dashboard_points_boundary_values(api_client, clean_state):
    """Verify data-point count accepts boundary values 1 and 100000."""
    api_client.command("dashboard.setPoints", {"points": 1})
    time.sleep(0.1)
    result = api_client.command("dashboard.getPoints")
    assert result.get("points") == 1

    api_client.command("dashboard.setPoints", {"points": 100000})
    time.sleep(0.1)
    result = api_client.command("dashboard.getPoints")
    assert result.get("points") == 100000


@pytest.mark.integration
def test_dashboard_points_invalid(api_client, clean_state):
    """Verify out-of-range data-point values are rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("dashboard.setPoints", {"points": 0})

    with pytest.raises(APIError):
        api_client.command("dashboard.setPoints", {"points": 100001})


@pytest.mark.integration
def test_dashboard_operation_mode_cycle(api_client, clean_state):
    """Verify operation mode can be set to all three valid values."""
    mode_names = {0: "ProjectFile", 1: "DeviceSendsJSON", 2: "QuickPlot"}

    for mode, expected_name in mode_names.items():
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.2)

        result = api_client.command("dashboard.getOperationMode")
        assert result.get("mode") == mode
        assert result.get("modeName") == expected_name


@pytest.mark.integration
def test_dashboard_operation_mode_invalid(api_client, clean_state):
    """Verify invalid operation mode index is rejected."""
    from utils.api_client import APIError

    with pytest.raises(APIError):
        api_client.command("dashboard.setOperationMode", {"mode": 3})

    with pytest.raises(APIError):
        api_client.command("dashboard.setOperationMode", {"mode": -1})


@pytest.mark.integration
def test_dashboard_get_status_fields(api_client, clean_state):
    """Verify dashboard.getStatus returns all required fields."""
    status = api_client.command("dashboard.getStatus")

    expected_fields = [
        "operationMode",
        "operationModeName",
        "fps",
        "points",
        "widgetCount",
        "datasetCount",
    ]

    for field in expected_fields:
        assert field in status, f"Missing field in dashboard.getStatus: {field}"


@pytest.mark.integration
def test_dashboard_get_status_reflects_changes(api_client, clean_state):
    """Verify dashboard.getStatus reflects FPS and points updates."""
    api_client.command("dashboard.setFPS", {"fps": 25})
    api_client.command("dashboard.setPoints", {"points": 750})
    time.sleep(0.2)

    status = api_client.command("dashboard.getStatus")
    assert status["fps"] == 25
    assert status["points"] == 750


@pytest.mark.integration
def test_dashboard_get_data_fields(api_client, clean_state):
    """Verify dashboard.getData returns widgetCount, datasetCount, and frame."""
    data = api_client.command("dashboard.getData")

    assert "widgetCount" in data, "Missing widgetCount in dashboard.getData"
    assert "datasetCount" in data, "Missing datasetCount in dashboard.getData"
    assert "frame" in data, "Missing frame in dashboard.getData"
    assert isinstance(data["widgetCount"], int)
    assert isinstance(data["datasetCount"], int)


@pytest.mark.integration
def test_dashboard_data_updates_after_frame_received(
    api_client, device_simulator, clean_state
):
    """Verify dashboard.getData reflects live data after frames are processed."""
    api_client.set_operation_mode("json")
    api_client.set_dashboard_fps(30)
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=1.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    data = api_client.command("dashboard.getData")
    assert isinstance(data["widgetCount"], int)
    assert isinstance(data["datasetCount"], int)

    api_client.disconnect_device()


@pytest.mark.integration
def test_dashboard_fps_independent_of_operation_mode(api_client, clean_state):
    """Verify FPS setting persists when switching operation modes."""
    api_client.command("dashboard.setFPS", {"fps": 20})
    time.sleep(0.1)

    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)
    assert api_client.command("dashboard.getFPS")["fps"] == 20

    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.1)
    assert api_client.command("dashboard.getFPS")["fps"] == 20
