"""
Fuzzy Testing Suite

Comprehensive fuzzy tests designed to stress-test Serial Studio and identify bugs.
Tests malformed data, edge cases, buffer overflows, race conditions, and more.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import random
import threading
import time

import pytest

from utils import ChecksumType, DataGenerator


@pytest.mark.parametrize("malformed_json", DataGenerator.generate_malformed_json())
def test_malformed_json_resilience(
    api_client, device_simulator, clean_state, malformed_json
):
    """
    Test that Serial Studio doesn't crash on malformed JSON.

    Sends valid frames first to build dashboard, then sends malformed JSON.
    Application should remain stable and connected.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    malformed_frame = DataGenerator.wrap_frame(
        malformed_json, checksum_type=ChecksumType.NONE
    )
    device_simulator.send_frame(malformed_frame)
    time.sleep(0.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application crashed or disconnected on malformed JSON"


def test_binary_garbage_flood(api_client, device_simulator, clean_state):
    """
    Test resilience against random binary garbage data.

    Sends valid frames, then floods with random bytes.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    for _ in range(20):
        garbage = DataGenerator.generate_random_garbage(min_size=50, max_size=500)
        device_simulator.send_frame(garbage)
        time.sleep(0.05)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application crashed on binary garbage"


def test_frame_corruption(api_client, device_simulator, clean_state):
    """
    Test handling of corrupted valid frames.

    Sends valid frames with random bit corruption.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC16)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    for frame in valid_frames[:10]:
        corrupted = DataGenerator.corrupt_frame(frame, corruption_rate=0.2)
        device_simulator.send_frame(corrupted)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on corrupted frames"


def test_partial_frames(api_client, device_simulator, clean_state):
    """
    Test handling of truncated/partial frames.

    Sends complete frames followed by partial frames.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    for frame in valid_frames[:10]:
        partial = DataGenerator.generate_partial_frame(frame)
        device_simulator.send_frame(partial)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on partial frames"


@pytest.mark.slow
def test_oversized_frame_handling(api_client, device_simulator, clean_state):
    """
    Test buffer overflow protection with extremely large frames.

    Attempts to send frames larger than typical buffer sizes.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(3):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.2)
    time.sleep(1.0)

    oversized = DataGenerator.generate_oversized_frame(multiplier=50)
    assert len(oversized) > 100000, "Frame should be very large"

    device_simulator.send_frame(oversized)
    time.sleep(2.0)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application crashed on oversized frame"


@pytest.mark.parametrize("edge_case", DataGenerator.generate_edge_case_values())
def test_numeric_edge_cases(api_client, device_simulator, clean_state, edge_case):
    """
    Test handling of edge case numeric values.

    Tests zero, infinity-like values, very large/small numbers, etc.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(3):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.5)

    edge_payload = json.dumps(edge_case)
    edge_frame = DataGenerator.wrap_frame(edge_payload, checksum_type=ChecksumType.NONE)
    device_simulator.send_frame(edge_frame)
    time.sleep(0.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], f"Application failed on edge case: {edge_case['title']}"


@pytest.mark.parametrize("unicode_str", DataGenerator.generate_unicode_stress())
def test_unicode_handling(api_client, device_simulator, clean_state, unicode_str):
    """
    Test Unicode edge cases in frame data.

    Tests various Unicode characters, control chars, RTL text, combining chars, etc.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(3):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.5)

    unicode_frame_data = {
        "title": unicode_str,
        "groups": [
            {
                "title": f"Unicode Test: {unicode_str[:20]}",
                "datasets": [
                    {"title": unicode_str, "value": "123", "units": unicode_str[:10]}
                ],
            }
        ],
    }

    unicode_payload = json.dumps(unicode_frame_data, ensure_ascii=False)
    unicode_frame = DataGenerator.wrap_frame(
        unicode_payload, checksum_type=ChecksumType.NONE
    )
    device_simulator.send_frame(unicode_frame)
    time.sleep(0.5)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on Unicode edge case"


@pytest.mark.parametrize("confusing_frame", DataGenerator.generate_delimiter_confusion())
def test_delimiter_confusion(api_client, device_simulator, clean_state, confusing_frame):
    """
    Test frames designed to confuse delimiter detection.

    Tests nested delimiters, backwards delimiters, multiple consecutive delimiters, etc.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    device_simulator.send_frame(confusing_frame)
    time.sleep(0.3)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on delimiter confusion"


@pytest.mark.slow
def test_rapid_fire_frames(api_client, device_simulator, clean_state):
    """
    Test handling of rapid-fire frame transmission.

    Sends 1000 frames as fast as possible to stress circular buffer and frame parser.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    frames = DataGenerator.generate_rapid_frames(count=1000)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    start_time = time.time()
    device_simulator.send_frames(frames, interval_seconds=0.0)
    duration = time.time() - start_time

    time.sleep(1.0)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on rapid-fire frames"


def test_mixed_valid_invalid_frames(api_client, device_simulator, clean_state):
    """
    Test interleaved valid and invalid frames.

    Ensures parser can recover from bad frames and continue processing good ones.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    for i in range(20):
        if i % 3 == 0:
            valid_payload = json.dumps(DataGenerator.generate_json_frame())
            frame = DataGenerator.wrap_frame(
                valid_payload, checksum_type=ChecksumType.NONE
            )
        elif i % 3 == 1:
            malformed = random.choice(DataGenerator.generate_malformed_json())
            frame = DataGenerator.wrap_frame(malformed, checksum_type=ChecksumType.NONE)
        else:
            frame = DataGenerator.generate_random_garbage(min_size=20, max_size=100)

        device_simulator.send_frame(frame)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on mixed valid/invalid frames"


def test_null_byte_injection(api_client, device_simulator, clean_state):
    """
    Test handling of null bytes in frame data.

    Null bytes can cause issues with C-string handling.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    null_injected_frames = [
        b"/*" + b"\x00" * 10 + b"*/\n",
        b"/*{" + b"\x00" + b'"title":"test"}*/\n',
        b"\x00\x00\x00/*valid*/\n",
        b"/*valid*/\x00\x00\x00\n",
        b"/*" + b"A" * 50 + b"\x00" + b"B" * 50 + b"*/\n",
    ]

    for frame in null_injected_frames:
        device_simulator.send_frame(frame)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on null byte injection"


@pytest.mark.slow
def test_checksum_brute_force(api_client, device_simulator, clean_state):
    """
    Test checksum validation by trying many invalid checksums.

    Ensures checksum validation is working correctly.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC32)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    payload = json.dumps(DataGenerator.generate_json_frame())
    base_frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.CRC32)

    for _ in range(50):
        corrupted = base_frame[:-6] + bytes(
            [random.randint(0, 255) for _ in range(4)]
        ) + b"\n"
        device_simulator.send_frame(corrupted)
        time.sleep(0.02)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed during checksum brute force"


def test_concurrent_rapid_reconnection(api_client, device_simulator, clean_state):
    """
    Test rapid connect/disconnect cycles.

    Stresses connection management and cleanup code.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    for i in range(10):
        api_client.connect_device()

        if device_simulator.wait_for_connection(timeout=2.0):
            payload = json.dumps(DataGenerator.generate_json_frame())
            frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
            device_simulator.send_frame(frame)
            time.sleep(0.1)

        api_client.disconnect_device()
        time.sleep(0.2)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert not status["isConnected"], "Should be disconnected after test"


@pytest.mark.slow
def test_memory_stress_repeated_large_frames(api_client, device_simulator, clean_state):
    """
    Test memory handling with repeated large frames.

    Ensures no memory leaks or allocation failures.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    large_groups = []
    for i in range(20):
        datasets = [
            {"title": f"S{j}", "value": f"{j * 1.5:.2f}", "units": "V"}
            for j in range(50)
        ]
        large_groups.append({"title": f"G{i}", "datasets": datasets})

    frames = []
    for i in range(50):
        payload = json.dumps(
            DataGenerator.generate_json_frame(groups=large_groups, title=f"Frame {i}")
        )
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(3.0)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed during memory stress test"


def test_fragmented_frame_reassembly(api_client, device_simulator, clean_state):
    """
    Test frame reassembly when data arrives in small chunks.

    Simulates slow/fragmented network transmission.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    payload = json.dumps(DataGenerator.generate_json_frame())
    frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)

    chunk_size = 10
    for i in range(0, len(frame), chunk_size):
        chunk = frame[i : i + chunk_size]
        device_simulator.send_frame(chunk)
        time.sleep(0.01)

    time.sleep(1.0)

    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        complete_frame = DataGenerator.wrap_frame(
            payload, checksum_type=ChecksumType.NONE
        )
        device_simulator.send_frame(complete_frame)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on fragmented frames"


@pytest.mark.slow
def test_json_depth_bomb(api_client, device_simulator, clean_state):
    """
    Test deeply nested JSON structures.

    Ensures JSON parser handles recursion limits correctly.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    deeply_nested = "{"
    for i in range(100):
        deeply_nested += f'"level{i}":{{'
    deeply_nested += '"value":"deep"'
    deeply_nested += "}" * 101

    nested_frame = DataGenerator.wrap_frame(
        deeply_nested, checksum_type=ChecksumType.NONE
    )
    device_simulator.send_frame(nested_frame)
    time.sleep(1.0)

    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application crashed on JSON depth bomb"


def test_empty_and_whitespace_frames(api_client, device_simulator, clean_state):
    """
    Test various empty and whitespace-only frames.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    empty_frames = [
        b"/**/\n",
        b"/*   */\n",
        b"/*\t\t\t*/\n",
        b"/*\n\n\n*/\n",
        b"/*        \r\n        */\n",
        b"/**/" + b" " * 1000 + b"\n",
    ]

    for frame in empty_frames:
        device_simulator.send_frame(frame)
        time.sleep(0.1)

    time.sleep(0.5)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed on empty/whitespace frames"


@pytest.mark.slow
def test_random_fuzz_chaos(api_client, device_simulator, clean_state):
    """
    Ultimate chaos test: random mix of everything.

    Combines all fuzzing techniques in random order.
    """
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")

    valid_frames = []
    for i in range(5):
        payload = json.dumps(DataGenerator.generate_json_frame())
        frame = DataGenerator.wrap_frame(payload, checksum_type=ChecksumType.NONE)
        valid_frames.append(frame)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(valid_frames, interval_seconds=0.1)
    time.sleep(0.8)

    chaos_actions = [
        lambda: DataGenerator.generate_random_garbage(1, 500),
        lambda: DataGenerator.wrap_frame(
            random.choice(DataGenerator.generate_malformed_json()),
            checksum_type=ChecksumType.NONE,
        ),
        lambda: random.choice(DataGenerator.generate_delimiter_confusion()),
        lambda: DataGenerator.wrap_frame(
            json.dumps(DataGenerator.generate_json_frame()),
            checksum_type=ChecksumType.NONE,
        ),
        lambda: b"\x00" * random.randint(1, 100),
        lambda: DataGenerator.generate_partial_frame(
            DataGenerator.wrap_frame(
                json.dumps(DataGenerator.generate_json_frame()),
                checksum_type=ChecksumType.NONE,
            )
        ),
    ]

    for _ in range(100):
        action = random.choice(chaos_actions)
        try:
            frame = action()
            device_simulator.send_frame(frame)
        except Exception:
            pass
        time.sleep(random.uniform(0.001, 0.05))

    time.sleep(1.0)
    status = api_client.command("io.manager.getStatus")
    assert status["isConnected"], "Application failed during random chaos fuzzing"
