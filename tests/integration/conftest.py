"""
Pytest Configuration and Fixtures

Shared fixtures for Serial Studio integration tests.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import os
import subprocess
import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils import DataGenerator, DeviceSimulator, SerialStudioClient


@pytest.fixture(scope="session")
def serial_studio_running():
    """
    Verify Serial Studio is running with API enabled.

    This fixture doesn't start Serial Studio - it expects it to be
    running already. This is the recommended approach for CI/CD and
    manual testing.
    """
    client = SerialStudioClient()

    try:
        client.connect()
        client.disconnect()
        return True
    except ConnectionError:
        pytest.fail(
            "Serial Studio is not running or API Server is not enabled.\n"
            "Please:\n"
            "  1. Start Serial Studio\n"
            "  2. Enable API Server (Settings > Miscellaneous > Enable API Server)\n"
            "  3. Ensure it's listening on port 7777"
        )


@pytest.fixture
def api_client(serial_studio_running):
    """
    Provide a connected API client.

    Automatically connects and disconnects. The command timeout is wider
    than the client default because functional tests should survive a
    transient CI-runner stall; the security suite keeps short timeouts on
    purpose (there a TimeoutError means a hang). Real hangs are still
    bounded by the 30 s per-test pytest timeout.
    """
    client = SerialStudioClient(timeout=15.0)
    client.connect()

    yield client

    try:
        if client.is_connected():
            client.disconnect_device()
    except Exception:
        pass

    client.disconnect()


@pytest.fixture
def clean_state(api_client):
    """
    Ensure Serial Studio is in a clean state.

    Disconnects device, disables exports, creates a new project, and parks
    the app in ProjectFile mode. ProjectFile is the right default because
    most integration tests exercise project-editor mutations, CSV export,
    workspaces, datasets, etc. — all of which are gated off in ConsoleOnly
    (the dumb-terminal mode that replaced the old "Device Sends JSON" slot
    in v3.3). Tests that need QuickPlot or ConsoleOnly explicitly set their
    own mode after this fixture runs.

    Handles server disconnections gracefully (e.g., from rate limiting).
    """

    # Helper to retry operations if server disconnected
    def safe_command(func, *args, max_retries=2, **kwargs):
        for attempt in range(max_retries):
            try:
                return func(*args, **kwargs)
            except ConnectionError:
                if attempt < max_retries - 1:
                    # Server might have disconnected us, try to reconnect
                    time.sleep(1.0)
                    try:
                        api_client.disconnect()
                        api_client.connect()
                    except:
                        pass
                else:
                    # Final attempt failed, ignore
                    pass
            except Exception:
                pass

    # Clean up before test
    safe_command(api_client.disconnect_device)
    safe_command(api_client.disable_csv_export)
    safe_command(api_client.create_new_project)
    safe_command(api_client.set_operation_mode, "project")

    # Add delay to let server recover from any rate limiting
    time.sleep(0.5)

    yield

    # Clean up after test
    safe_command(api_client.disconnect_device)
    safe_command(api_client.disable_csv_export)

    # Add delay before next test
    time.sleep(0.5)


@pytest.fixture
def device_simulator():
    """
    Provide a device simulator for testing.

    Automatically starts and stops the simulator.
    """
    sim = DeviceSimulator(host="127.0.0.1", port=9000, protocol="tcp")
    sim.start()

    yield sim

    sim.stop()


@pytest.fixture
def mosquitto_broker():
    """
    Skip the test unless an MQTT broker is reachable on 127.0.0.1:1883.

    CI launches `mosquitto -d` before pytest; local developers can run
    `mosquitto -p 1883` in another terminal. Returns a dict with the
    broker connection parameters so the test doesn't hardcode them.
    """
    import socket

    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(2.0)
    try:
        sock.connect(("127.0.0.1", 1883))
    except (ConnectionRefusedError, OSError, socket.timeout):
        pytest.skip("MQTT broker not reachable on 127.0.0.1:1883")
    finally:
        try:
            sock.close()
        except Exception:
            pass

    return {"host": "127.0.0.1", "port": 1883}


@pytest.fixture
def mqtt_subscriber(mosquitto_broker):
    """
    Provide a paho-mqtt subscriber connected to the local broker.

    Returns a helper object exposing `subscribe(topic)` and a `messages` list
    that is populated asynchronously as the broker delivers messages.
    """
    try:
        import paho.mqtt.client as mqtt
    except ImportError:
        pytest.skip("paho-mqtt is not installed")

    received: list = []

    class _Subscriber:
        def __init__(self):
            try:
                self.client = mqtt.Client(
                    client_id=f"ss-test-{time.time_ns()}",
                    callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
                )
            except (AttributeError, TypeError):
                # Older paho-mqtt without callback_api_version arg
                self.client = mqtt.Client(client_id=f"ss-test-{time.time_ns()}")

            def _on_message(_client, _userdata, msg):
                received.append({"topic": msg.topic, "payload": bytes(msg.payload)})

            self.client.on_message = _on_message
            self.client.connect(mosquitto_broker["host"], mosquitto_broker["port"], 30)
            self.client.loop_start()
            self.messages = received

        def subscribe(self, topic: str, qos: int = 0) -> None:
            self.client.subscribe(topic, qos)
            # Give the broker a moment to register the subscription so
            # subsequent publishes are delivered.
            time.sleep(0.2)

        def wait_for_messages(self, count: int = 1, timeout: float = 5.0) -> bool:
            end = time.time() + timeout
            while time.time() < end:
                if len(received) >= count:
                    return True
                time.sleep(0.05)
            return False

        def close(self) -> None:
            try:
                self.client.loop_stop()
                self.client.disconnect()
            except Exception:
                pass

    sub = _Subscriber()
    yield sub
    sub.close()


@pytest.fixture
def data_generator():
    """Provide a data generator instance."""
    return DataGenerator()


@pytest.fixture
def test_data_dir():
    """Return path to test data directory."""
    return Path(__file__).parent.parent / "fixtures"


@pytest.fixture
def temp_dir(tmp_path):
    """Provide a temporary directory for test outputs."""
    return tmp_path


@pytest.fixture
def sample_project(test_data_dir):
    """Return path to sample project file."""
    project_path = test_data_dir / "projects" / "sample.json"
    if not project_path.exists():
        pytest.skip(f"Sample project not found: {project_path}")
    return str(project_path)


@pytest.fixture(scope="session")
def checksum_types():
    """Return list of all checksum types to test."""
    from utils.data_generator import ChecksumType

    return [
        ChecksumType.NONE,
        ChecksumType.XOR,
        ChecksumType.SUM,
        ChecksumType.CRC8,
        ChecksumType.CRC16,
        ChecksumType.CRC32,
        ChecksumType.FLETCHER16,
        ChecksumType.ADLER32,
    ]


def pytest_configure(config):
    """Configure pytest with custom markers."""
    config.addinivalue_line(
        "markers", "slow: marks tests as slow (deselect with '-m \"not slow\"')"
    )
    config.addinivalue_line(
        "markers",
        "integration: integration tests requiring running Serial Studio instance",
    )
    config.addinivalue_line("markers", "performance: performance benchmarks")
    config.addinivalue_line(
        "markers", "requires_device: tests requiring physical hardware"
    )
    config.addinivalue_line("markers", "network: tests using network driver")
    config.addinivalue_line("markers", "uart: tests using UART driver")
    config.addinivalue_line("markers", "ble: tests using Bluetooth LE driver")
    config.addinivalue_line("markers", "csv: tests involving CSV export")
    config.addinivalue_line(
        "markers", "mdf4: tests involving MDF4 export (Pro feature)"
    )
    config.addinivalue_line("markers", "project: tests involving project files")


def pytest_collection_modifyitems(config, items):
    """Automatically mark all tests as integration tests."""
    for item in items:
        if "integration" in str(item.fspath):
            item.add_marker(pytest.mark.integration)
        if "performance" in str(item.fspath):
            item.add_marker(pytest.mark.performance)
