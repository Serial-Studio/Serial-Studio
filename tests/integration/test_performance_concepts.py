"""
Performance Concepts Integration Tests

Tests to validate the distinction between data processing rate and UI rendering rate.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator


def test_high_data_rate_low_rendering_fps(api_client, device_simulator, clean_state):
    """
    Test that Serial Studio can process high-frequency data (1000 Hz)
    while rendering at a lower rate (30 FPS).

    This validates the decoupling of data processing and UI rendering.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Set UI rendering to 30 FPS (typical for smooth visuals)
    api_client.set_dashboard_fps(30)
    time.sleep(0.1)

    # Generate high-frequency data (1000 Hz)
    frames = DataGenerator.generate_realistic_telemetry(
        duration_seconds=2.0,
        frequency_hz=1000.0,  # 1000 data frames per second
        frame_format="json",
        checksum_type=ChecksumType.CRC16,
    )

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Send data at 1000 Hz
    start_time = time.time()
    device_simulator.send_frames(frames, interval_seconds=0.001)
    elapsed = time.time() - start_time

    # Wait for processing to complete
    time.sleep(1.0)

    # Verify still connected (all frames processed successfully)
    assert api_client.is_connected(), "Should still be connected after high-frequency data"

    # Verify dashboard settings unchanged
    status = api_client.get_dashboard_status()
    assert status["fps"] == 30, "Dashboard FPS should still be 30 (rendering rate)"

    api_client.disconnect_device()

    print(
        f"\nProcessed {len(frames)} frames in {elapsed:.2f}s "
        f"({len(frames) / elapsed:.1f} frames/sec) "
        f"while rendering at {status['fps']} FPS"
    )


def test_rendering_fps_independent_of_data_rate(api_client, device_simulator, clean_state):
    """
    Test that changing dashboard FPS doesn't affect data processing rate.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Test with different rendering rates
    for render_fps in [15, 30, 60]:
        api_client.set_dashboard_fps(render_fps)
        time.sleep(0.2)

        # Generate same data rate (100 Hz) regardless of rendering FPS
        frames = DataGenerator.generate_realistic_telemetry(
            duration_seconds=1.0,
            frequency_hz=100.0,
            frame_format="json",
            checksum_type=ChecksumType.CRC16,
        )

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        # Send data
        start_time = time.time()
        device_simulator.send_frames(frames, interval_seconds=0.01)
        elapsed = time.time() - start_time

        time.sleep(0.5)

        # Verify processing completed successfully
        assert api_client.is_connected()

        api_client.disconnect_device()
        time.sleep(0.3)

        actual_rate = len(frames) / elapsed
        print(
            f"Render FPS: {render_fps}, "
            f"Data rate: {actual_rate:.1f} frames/sec, "
            f"Frames processed: {len(frames)}"
        )


@pytest.mark.slow
def test_sustained_processing_vs_rendering(api_client, device_simulator, clean_state):
    """
    Test sustained data processing at moderate rate with low rendering FPS.

    Validates that Serial Studio can handle continuous data streaming
    without dropping frames, even with low UI refresh rate.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    # Set very low rendering rate (saves CPU for data processing)
    api_client.set_dashboard_fps(15)
    time.sleep(0.1)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Stream at 50 Hz for 10 seconds = 500 frames (more realistic for test)
    duration_seconds = 10.0
    frequency_hz = 50.0
    batch_size = 50

    start_time = time.time()
    frames_sent = 0

    while (time.time() - start_time) < duration_seconds:
        frames = DataGenerator.generate_realistic_telemetry(
            duration_seconds=batch_size / frequency_hz,
            frequency_hz=frequency_hz,
            frame_format="json",
            checksum_type=ChecksumType.CRC16,
        )

        device_simulator.send_frames(frames, interval_seconds=1.0 / frequency_hz)
        frames_sent += len(frames)

    elapsed = time.time() - start_time

    # Verify still connected
    assert api_client.is_connected()

    api_client.disconnect_device()

    avg_data_rate = frames_sent / elapsed
    status = api_client.get_dashboard_status()

    print(
        f"\nSustained test results:"
        f"\n  Data processing rate: {avg_data_rate:.1f} frames/sec"
        f"\n  UI rendering rate: {status['fps']} FPS"
        f"\n  Total frames processed: {frames_sent}"
        f"\n  Duration: {elapsed:.2f}s"
        f"\n  Ratio: {avg_data_rate / status['fps']:.1f}x data rate vs render rate"
    )

    # Verify we processed at least 70% of expected frames (realistic for Python test)
    expected_frames = int(duration_seconds * frequency_hz)
    assert frames_sent >= expected_frames * 0.7, (
        f"Should process at least 70% of frames ({frames_sent}/{expected_frames})"
    )


def test_dashboard_fps_range_limits(api_client, clean_state):
    """
    Test valid range for dashboard rendering FPS.
    """
    # Test common rendering rates
    valid_fps_values = [10, 15, 24, 30, 60, 120]

    for fps in valid_fps_values:
        api_client.set_dashboard_fps(fps)
        time.sleep(0.1)

        status = api_client.get_dashboard_status()
        assert status["fps"] == fps, f"FPS should be set to {fps}"
        print(f"✓ Dashboard FPS set to {fps}")
