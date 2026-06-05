"""
Top-Level Pytest Configuration

Shared fixtures available to every test suite (integration, security, etc).
Suite-specific conftests may shadow these with stricter behaviour where
needed (e.g. integration/conftest.py adds per-test cleanup).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import sys
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent))

from utils import SerialStudioClient


@pytest.fixture(scope="session")
def serial_studio_running():
    """
    Verify Serial Studio is running with API enabled.

    Sessions-scoped so the check runs once per pytest invocation.
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
    """Connected API client with automatic disconnect on teardown."""
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
    Reset the app to a known-good ProjectFile state before each test.

    Same behaviour as the integration-suite fixture: disconnect any device,
    disable CSV export, create a fresh project, park in ProjectFile mode.
    Handles transient ConnectionErrors caused by rate limiting.
    """

    def safe_command(func, *args, max_retries=2, **kwargs):
        for attempt in range(max_retries):
            try:
                return func(*args, **kwargs)
            except ConnectionError:
                if attempt < max_retries - 1:
                    time.sleep(1.0)
                    try:
                        api_client.disconnect()
                        api_client.connect()
                    except Exception:
                        pass
            except Exception:
                pass

    safe_command(api_client.disconnect_device)
    safe_command(api_client.disable_csv_export)
    safe_command(api_client.create_new_project)
    safe_command(api_client.set_operation_mode, "project")

    time.sleep(0.5)

    yield

    safe_command(api_client.disconnect_device)
    safe_command(api_client.disable_csv_export)

    time.sleep(0.5)


# Tests that hammer many fresh connections with no pacing ("max stress" probes).
# On shared CI runners the odd connection drops or times out, which is a runner
# artifact, not a regression -- so they retry their own transient failures. The
# list is curated on purpose: a blanket rerun would also re-run a hard app crash
# against a dead instance and waste a full suite per attempt.
_FLAKY_TESTS = {
    "test_rapid_status_queries_no_crash",
    "test_command_authorization",
    # Wall-clock throughput / response-latency probes: the assertion is a timing
    # floor against a live local socket, so a loaded runner trips it without any
    # code regression. One rerun on a quieter scheduler clears the artifact.
    "test_high_frequency_frames",
    "test_sustained_processing_vs_rendering",
    "test_long_running_session",
    "test_concurrent_connections_status",
    "test_js_transform_action_fire_invalid_id_returns_error",
}


def pytest_collection_modifyitems(config, items):
    """Attach a bounded rerun marker to the curated flaky tests, if the plugin is present."""
    if not config.pluginmanager.hasplugin("rerunfailures"):
        return

    for item in items:
        if item.name in _FLAKY_TESTS:
            item.add_marker(pytest.mark.flaky(reruns=2, reruns_delay=2))
