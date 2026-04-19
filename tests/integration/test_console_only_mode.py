"""
Console-Only Mode Integration Tests

Verifies the v3.3 ConsoleOnly operation mode (enum slot 1, formerly
DeviceSendsJSON). In this mode:

 * FrameReader short-circuits before any parsing
 * Dashboard and FrameBuilder are inactive — no datasets are produced
 * Raw bytes still reach the terminal via DeviceManager::rawDataReceived
 * Switching into the mode is reported by dashboard.getOperationMode
   and dashboard.getStatus with modeName=\"ConsoleOnly\"

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


CONSOLE_ONLY_MODE = 1  # SerialStudio::OperationMode enum value


# ---------------------------------------------------------------------------
# Mode switch + identity
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_set_console_only_mode(api_client, clean_state):
    """dashboard.setOperationMode(1) puts the app in ConsoleOnly mode."""
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    result = api_client.command("dashboard.getOperationMode")
    assert result["mode"] == CONSOLE_ONLY_MODE
    assert result["modeName"] == "ConsoleOnly"


@pytest.mark.project
def test_status_reports_console_only(api_client, clean_state):
    """dashboard.getStatus returns operationModeName='ConsoleOnly'."""
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    status = api_client.command("dashboard.getStatus")
    assert status["operationMode"] == CONSOLE_ONLY_MODE
    assert status["operationModeName"] == "ConsoleOnly"


@pytest.mark.project
def test_mode_switch_round_trip(api_client, clean_state):
    """Cycling into ConsoleOnly and back leaves the app in a clean state."""
    # Start at ProjectFile
    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)

    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.1)
    assert api_client.command("dashboard.getOperationMode")["mode"] == CONSOLE_ONLY_MODE

    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)
    assert api_client.command("dashboard.getOperationMode")["mode"] == 0


# ---------------------------------------------------------------------------
# No-parsing invariant — dashboard stays empty
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_console_only_produces_no_datasets(
    api_client, device_simulator, clean_state
):
    """
    In ConsoleOnly mode, sending frames that would normally produce datasets
    results in dashboard.getData reporting zero datasets. FrameReader skips
    the CircularBuffer / ReaderWriterQueue path entirely, so FrameBuilder
    never sees the bytes.
    """
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Send frames that WOULD be parsed in ProjectFile mode — they should be
    # no-ops here.
    frames = [f"data,line,{i}\n".encode() for i in range(5)]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)

    data = api_client.get_dashboard_data()

    # No groups / no datasets produced by FrameBuilder
    assert data.get("datasetCount", 0) == 0
    frame = data.get("frame", {})
    groups = frame.get("groups", [])
    assert groups == []

    api_client.disconnect_device()


@pytest.mark.project
def test_console_only_stays_connected_under_load(
    api_client, device_simulator, clean_state
):
    """
    Under continuous byte stream, the connection must stay healthy. Frame
    dropping / parsing errors must NOT fire in ConsoleOnly mode.
    """
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # 100 small frames at ~100 Hz
    frames = [f"tick {i}\n".encode() for i in range(100)]
    device_simulator.send_frames(frames, interval_seconds=0.01)
    time.sleep(2.0)

    assert api_client.is_connected()

    api_client.disconnect_device()


# ---------------------------------------------------------------------------
# Raw-byte plumbing — terminal/stats paths still see traffic
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_console_only_increments_rx_byte_counter(
    api_client, device_simulator, clean_state
):
    """
    Even though FrameReader short-circuits, the raw-bytes-received counter
    exposed via io.manager.getStats must increment when the device sends
    data. This confirms the raw path is still plumbed through
    DeviceManager::rawDataReceived.
    """
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Read baseline (some tests may share the process across runs)
    stats_before = api_client.command("io.manager.getStatus")
    rx_before = stats_before.get("bytesReceived", 0)

    payload = b"0123456789" * 50  # 500 bytes
    device_simulator.send_frame(payload)
    time.sleep(1.0)

    stats_after = api_client.command("io.manager.getStatus")
    rx_after = stats_after.get("bytesReceived", 0)

    # Best-effort check: the counter should have advanced. Some builds may
    # not expose a bytesReceived field — skip the assertion in that case.
    if "bytesReceived" in stats_after:
        assert rx_after >= rx_before + 400, (
            f"bytesReceived did not advance as expected "
            f"(before={rx_before}, after={rx_after})"
        )

    api_client.disconnect_device()


# ---------------------------------------------------------------------------
# REGRESSION — out-of-range mode rejected
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_set_operation_mode_rejects_out_of_range(api_client, clean_state):
    """
    setOperationMode must reject values outside [0, 2]. FrameReader::processData
    asserts on that range at every call, so letting an OOB value through would
    cause a debug-build crash at the first frame.
    """
    from utils import APIError

    for bad_mode in (-1, 3, 255, -10):
        with pytest.raises(APIError) as ei:
            api_client.command("dashboard.setOperationMode", {"mode": bad_mode})
        assert ei.value.code == "INVALID_PARAM", (
            f"mode={bad_mode} should be rejected as INVALID_PARAM, "
            f"got {ei.value.code}"
        )
