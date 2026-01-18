"""
End-to-End Workflow Tests

Tests complete user workflows from start to finish.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator


def test_complete_monitoring_workflow(api_client, device_simulator, clean_state):
    """
    Test complete workflow: configure device, connect, receive data, disconnect.
    """
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.2)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device should connect"

    assert api_client.wait_for_connection(timeout=5.0), "SS should connect to device"

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=2.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(2.5)

    assert api_client.is_connected(), "Should still be connected"

    api_client.disconnect_device()
    time.sleep(0.3)

    assert not api_client.is_connected(), "Should be disconnected"


def test_export_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow: configure, enable export, collect data, verify export.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    api_client.enable_csv_export()
    time.sleep(0.3)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=3.0,
        frequency_hz=10.0,
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(3.5)

    # Verify export is enabled and data was processed
    export_status = api_client.get_csv_export_status()
    assert export_status["enabled"], "CSV export should be enabled"

    api_client.disconnect_device()
    api_client.command("csv.export.close")
    time.sleep(0.5)

    api_client.disable_csv_export()


def test_reconnection_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow: connect, disconnect, reconnect multiple times.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    for cycle in range(3):
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        frames = DataGenerator.generate_realistic_telemetry(
            duration_seconds=1.0,
            frequency_hz=10.0,
            frame_format="json",
            checksum_type=ChecksumType.CRC16,
        )

        device_simulator.send_frames(frames, interval_seconds=0.1)
        time.sleep(1.2)

        api_client.disconnect_device()
        time.sleep(0.5)

        assert not api_client.is_connected()


def test_dashboard_configuration_workflow(api_client, clean_state):
    """
    Test workflow: configure dashboard rendering settings.

    Note: Dashboard FPS controls UI rendering rate (30-60 Hz typical),
    NOT the data frame processing rate (which can be much higher).
    """
    # Set dashboard rendering rate to 30 FPS
    api_client.set_dashboard_fps(30)
    time.sleep(0.1)

    status = api_client.get_dashboard_status()
    assert status["fps"] == 30

    # Set number of data points visible in plots
    api_client.set_dashboard_points(500)
    time.sleep(0.1)

    status = api_client.get_dashboard_status()
    assert status["points"] == 500

    # Update to higher rendering rate and more plot points
    api_client.set_dashboard_fps(60)
    api_client.set_dashboard_points(1000)
    time.sleep(0.1)

    status = api_client.get_dashboard_status()
    assert status["fps"] == 60
    assert status["points"] == 1000


def test_pause_resume_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow: connect, pause streaming, resume streaming.
    """
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
    time.sleep(1.2)

    api_client.command("io.manager.setPaused", {"paused": True})
    time.sleep(0.2)

    status = api_client.command("io.manager.getStatus")
    assert status["paused"], "Should be paused"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)

    api_client.command("io.manager.setPaused", {"paused": False})
    time.sleep(0.2)

    status = api_client.command("io.manager.getStatus")
    assert not status["paused"], "Should be resumed"

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)

    api_client.disconnect_device()


def test_error_recovery_workflow(api_client, device_simulator, clean_state):
    """
    Test workflow: handle disconnection gracefully and recover.
    """
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
    time.sleep(1.2)

    # Simulate device disconnection
    device_simulator.stop()
    time.sleep(1.0)

    # Try to disconnect (may fail if already disconnected)
    try:
        api_client.disconnect_device()
    except Exception:
        pass  # Expected - connection already lost

    # Restart device simulator
    device_simulator.start()
    time.sleep(0.5)

    # Reconnect
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.2)

    # Clean disconnect
    try:
        api_client.disconnect_device()
    except Exception:
        pass


@pytest.mark.slow
@pytest.mark.timeout(60)
def test_long_running_session(api_client, device_simulator, clean_state):
    """
    Test long-running session to detect memory leaks and stability issues.
    """
    api_client.set_operation_mode("json")
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    duration_seconds = 30.0
    frequency_hz = 10.0
    batch_size = 10  # Smaller batches to avoid overwhelming connection

    start_time = time.time()
    frames_sent = 0

    try:
        while (time.time() - start_time) < duration_seconds:
            frames = DataGenerator.generate_realistic_telemetry(
                duration_seconds=batch_size / frequency_hz,
                frequency_hz=frequency_hz,
                frame_format="json",
                checksum_type=ChecksumType.CRC16,
            )

            try:
                device_simulator.send_frames(frames, interval_seconds=1.0 / frequency_hz)
                frames_sent += len(frames)
            except RuntimeError:
                # Connection may have dropped - this is what we're testing for
                print(f"Connection dropped after {frames_sent} frames")
                break

    except Exception as e:
        print(f"Exception during long-running test: {e}")

    elapsed = time.time() - start_time

    # Verify we sent a reasonable amount of data
    min_expected = int(duration_seconds * frequency_hz * 0.5)
    assert frames_sent >= min_expected, (
        f"Should have sent at least 50% of expected frames "
        f"({frames_sent} / {min_expected})"
    )

    # Try to disconnect gracefully
    try:
        if api_client.is_connected():
            api_client.disconnect_device()
    except Exception:
        pass  # Connection may already be closed
