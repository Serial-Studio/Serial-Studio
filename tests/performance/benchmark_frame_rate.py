"""
Frame Rate Performance Benchmarks

Measure Serial Studio's frame processing performance.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils import ChecksumType, DataGenerator, DeviceSimulator, SerialStudioClient


@pytest.fixture
def api_client():
    """Provide API client for benchmarks."""
    client = SerialStudioClient()
    client.connect()
    yield client
    try:
        client.disconnect_device()
    except Exception:
        pass
    client.disconnect()


@pytest.fixture
def device_simulator():
    """Provide device simulator for benchmarks."""
    sim = DeviceSimulator(host="127.0.0.1", port=9000, protocol="tcp")
    sim.start()
    yield sim
    sim.stop()


@pytest.mark.performance
@pytest.mark.parametrize("frequency_hz", [10, 50, 100, 200, 500, 1000])
def test_frame_rate_throughput(
    benchmark, api_client, device_simulator, frequency_hz
):
    """
    Benchmark: Measure frames-per-second throughput at different rates.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    def send_frames():
        frames = DataGenerator.generate_realistic_telemetry(
            duration_seconds=1.0,
            frequency_hz=frequency_hz,
            frame_format="json",
            checksum_type=ChecksumType.CRC16,
        )

        start_time = time.time()
        device_simulator.send_frames(frames, interval_seconds=1.0 / frequency_hz)
        elapsed = time.time() - start_time

        time.sleep(0.5)

        return elapsed, len(frames)

    result = benchmark(send_frames)
    elapsed, frame_count = result

    actual_rate = frame_count / elapsed if elapsed > 0 else 0

    print(
        f"\nFrame rate benchmark @ {frequency_hz} Hz: "
        f"{actual_rate:.1f} FPS ({frame_count} frames in {elapsed:.3f}s)"
    )

    api_client.disconnect_device()


@pytest.mark.performance
def test_sustained_high_frequency(benchmark, api_client, device_simulator):
    """
    Benchmark: Sustained high-frequency data streaming (1 kHz for 10 seconds).
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    def sustained_stream():
        frequency_hz = 1000.0
        duration_seconds = 10.0
        batch_size = 100

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
        return elapsed, frames_sent

    result = benchmark.pedantic(sustained_stream, iterations=1, rounds=1)
    elapsed, frames_sent = result

    average_rate = frames_sent / elapsed if elapsed > 0 else 0

    print(
        f"\nSustained streaming: "
        f"{average_rate:.1f} FPS average "
        f"({frames_sent} frames in {elapsed:.2f}s)"
    )

    api_client.disconnect_device()


@pytest.mark.performance
@pytest.mark.parametrize("checksum_type", [
    ChecksumType.NONE,
    ChecksumType.CRC16,
    ChecksumType.CRC32,
])
def test_checksum_overhead(
    benchmark, api_client, device_simulator, checksum_type
):
    """
    Benchmark: Measure checksum validation overhead.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    def send_with_checksum():
        frames = DataGenerator.generate_realistic_telemetry(
            duration_seconds=1.0,
            frequency_hz=100.0,
            frame_format="json",
            checksum_type=checksum_type,
        )

        start_time = time.time()
        device_simulator.send_frames(frames, interval_seconds=0.01)
        elapsed = time.time() - start_time

        time.sleep(0.5)

        return elapsed, len(frames)

    result = benchmark(send_with_checksum)
    elapsed, frame_count = result

    print(
        f"\nChecksum {checksum_type.value}: "
        f"{frame_count / elapsed:.1f} FPS "
        f"({elapsed:.3f}s for {frame_count} frames)"
    )

    api_client.disconnect_device()


@pytest.mark.performance
@pytest.mark.parametrize("frame_size", ["small", "medium", "large"])
def test_frame_size_impact(benchmark, api_client, device_simulator, frame_size):
    """
    Benchmark: Measure impact of frame size on processing speed.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    sizes = {
        "small": 5,
        "medium": 50,
        "large": 200,
    }

    num_datasets = sizes[frame_size]

    def send_sized_frames():
        groups = []
        for i in range(num_datasets // 10):
            datasets = [
                {"title": f"S{j}", "value": f"{j * 1.23:.2f}", "units": "V"}
                for j in range(10)
            ]
            groups.append({"title": f"G{i}", "datasets": datasets})

        frames = []
        for _ in range(100):
            payload = DataGenerator.generate_json_frame(groups=groups)
            frame = DataGenerator.wrap_frame(
                DataGenerator.generate_json_frame(groups=groups),
                checksum_type=ChecksumType.CRC16,
            )
            frames.append(frame)

        start_time = time.time()
        device_simulator.send_frames(frames, interval_seconds=0.01)
        elapsed = time.time() - start_time

        time.sleep(0.5)

        return elapsed, len(frames)

    result = benchmark(send_sized_frames)
    elapsed, frame_count = result

    print(
        f"\nFrame size {frame_size} ({num_datasets} datasets): "
        f"{frame_count / elapsed:.1f} FPS ({elapsed:.3f}s)"
    )

    api_client.disconnect_device()
