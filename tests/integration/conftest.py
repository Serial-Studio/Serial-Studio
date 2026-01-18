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

    Automatically connects and disconnects.
    """
    client = SerialStudioClient()
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

    Disconnects device, disables exports, creates new project.
    Sets operation mode to "Device Sends JSON" for tests.
    """
    try:
        api_client.disconnect_device()
    except Exception:
        pass

    try:
        api_client.disable_csv_export()
    except Exception:
        pass

    try:
        api_client.create_new_project()
    except Exception:
        pass

    try:
        api_client.set_operation_mode("json")
    except Exception:
        pass

    yield

    try:
        api_client.disconnect_device()
    except Exception:
        pass

    try:
        api_client.disable_csv_export()
    except Exception:
        pass


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
        "markers", "integration: integration tests requiring running Serial Studio instance"
    )
    config.addinivalue_line("markers", "performance: performance benchmarks")
    config.addinivalue_line("markers", "requires_device: tests requiring physical hardware")
    config.addinivalue_line("markers", "network: tests using network driver")
    config.addinivalue_line("markers", "uart: tests using UART driver")
    config.addinivalue_line("markers", "ble: tests using Bluetooth LE driver")
    config.addinivalue_line("markers", "csv: tests involving CSV export")
    config.addinivalue_line("markers", "mdf4: tests involving MDF4 export (Pro feature)")
    config.addinivalue_line("markers", "project: tests involving project files")


def pytest_collection_modifyitems(config, items):
    """Automatically mark all tests as integration tests."""
    for item in items:
        if "integration" in str(item.fspath):
            item.add_marker(pytest.mark.integration)
        if "performance" in str(item.fspath):
            item.add_marker(pytest.mark.performance)
