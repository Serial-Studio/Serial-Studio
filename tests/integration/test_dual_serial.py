"""
Dual Serial Port Integration Tests

Tests that exercise two independent UART data sources over virtual serial
ports (PTYs).  Each source uses different frame delimiters and feeds its
own dashboard group, validating end-to-end multi-source UART data flow.

Requires:
- Serial Studio running with API enabled (port 7777)
- Commercial build (multi-source is gated by BUILD_COMMERCIAL)
- PTY support (macOS or Linux)

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import copy
import os
import threading
import time

import pytest

from utils import DualSerialPorts, PTY_AVAILABLE
from utils.api_client import APIError
from utils.data_generator import DataGenerator

_IS_CI = os.environ.get("CI", "").lower() in ("true", "1", "yes")

# ---------------------------------------------------------------------------
# Skip conditions
# ---------------------------------------------------------------------------

pytestmark = [
    pytest.mark.uart,
    pytest.mark.integration,
    pytest.mark.project,
    pytest.mark.skipif(not PTY_AVAILABLE, reason="PTY support not available"),
    pytest.mark.skipif(_IS_CI, reason="QSerialPort cannot open PTY devices reliably in CI"),
]


def _is_commercial(api_client) -> bool:
    """Return True when connected to a Commercial build of Serial Studio."""
    try:
        api_client.source_add()
        return True
    except APIError as e:
        if any(w in e.message.lower() for w in ("commercial", "license", "pro")):
            return False
        raise


# ---------------------------------------------------------------------------
# Project definition
# ---------------------------------------------------------------------------

_CSV_PARSER = DataGenerator.CSV_PARSER_TEMPLATE

DUAL_SENSOR_PROJECT = {
    "title": "Dual Sensor Test",
    "sources": [
        {
            "sourceId": 0,
            "title": "Sensor Alpha",
            "busType": 0,
            "frameDetection": 1,
            "frameStart": "/*",
            "frameEnd": "*/",
            "checksumAlgorithm": "",
            "hexadecimalDelimiters": False,
            "decoder": 0,
            "decoderMethod": 0,
            "frameParserCode": _CSV_PARSER,
            "connection": {},
        },
        {
            "sourceId": 1,
            "title": "Sensor Bravo",
            "busType": 0,
            "frameDetection": 1,
            "frameStart": "<<",
            "frameEnd": ">>",
            "checksumAlgorithm": "",
            "hexadecimalDelimiters": False,
            "decoder": 0,
            "decoderMethod": 0,
            "frameParserCode": _CSV_PARSER,
            "connection": {},
        },
    ],
    "groups": [
        {
            "title": "Alpha Environment",
            "widget": "datagrid",
            "datasets": [
                {
                    "title": "Alpha Temp",
                    "index": 1,
                    "units": "C",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
                {
                    "title": "Alpha Humidity",
                    "index": 2,
                    "units": "%",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
                {
                    "title": "Alpha Pressure",
                    "index": 3,
                    "units": "hPa",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
            ],
        },
        {
            "title": "Bravo Environment",
            "widget": "datagrid",
            "sourceId": 1,
            "datasets": [
                {
                    "title": "Bravo Temp",
                    "index": 1,
                    "units": "C",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
                {
                    "title": "Bravo Humidity",
                    "index": 2,
                    "units": "%",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
                {
                    "title": "Bravo Pressure",
                    "index": 3,
                    "units": "hPa",
                    "value": "",
                    "graph": True,
                    "fft": False,
                    "log": False,
                    "led": False,
                    "widget": "",
                },
            ],
        },
    ],
    "actions": [],
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _get_group_values(api_client, group_idx: int) -> list:
    """Extract dataset values from a specific group in dashboard.getData."""
    data = api_client.get_dashboard_data()
    groups = data.get("frame", {}).get("groups", [])
    if group_idx >= len(groups):
        return []
    return [d.get("value", "") for d in groups[group_idx].get("datasets", [])]


def _wait_for_group_values(
    api_client,
    group_idx: int,
    expected: list,
    timeout: float = 5.0,
) -> list:
    """Poll dashboard until the specified group shows expected values."""
    end = time.time() + timeout
    while time.time() < end:
        values = _get_group_values(api_client, group_idx)
        if values == expected:
            return values
        time.sleep(0.05)
    return _get_group_values(api_client, group_idx)


def _send_alpha_frame(dual_ports, values: list) -> None:
    """Send a /*csv*/ frame to the alpha PTY."""
    dual_ports.alpha.write_csv_frame(values, "/*", "*/")


def _send_bravo_frame(dual_ports, values: list) -> None:
    """Send a <<csv>> frame to the bravo PTY."""
    dual_ports.bravo.write_csv_frame(values, "<<", ">>")


def _load_project_with_ports(api_client, dual_ports) -> None:
    """Load the dual-sensor project with PTY device paths filled in."""
    project = copy.deepcopy(DUAL_SENSOR_PROJECT)
    project["sources"][0]["connection"]["device"] = dual_ports.device_path_a
    project["sources"][1]["connection"]["device"] = dual_ports.device_path_b

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    api_client.source_configure(
        0, {"device": dual_ports.device_path_a, "baudRate": 9600}
    )
    api_client.source_configure(
        1, {"device": dual_ports.device_path_b, "baudRate": 9600}
    )
    time.sleep(0.2)


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture
def dual_ports():
    """Create two PTY pairs, yield DualSerialPorts, close on teardown."""
    ports = DualSerialPorts()
    ports.open()
    yield ports
    ports.close()


@pytest.fixture
def dual_serial_env(api_client, dual_ports, clean_state):
    """
    Load the dual-sensor project with PTY paths and skip if not Commercial.

    Yields (api_client, dual_ports) for convenience.
    """
    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    # Reset after the _is_commercial probe (which calls source_add)
    api_client.command("project.file.new")
    time.sleep(0.2)

    _load_project_with_ports(api_client, dual_ports)
    yield api_client, dual_ports


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

class TestDualSerialConnect:
    """Verify connection setup with two UART sources over PTYs."""

    def test_connect_succeeds(self, dual_serial_env):
        """Loading a dual-UART project and connecting should succeed."""
        api, ports = dual_serial_env

        api.connect_device()
        assert api.wait_for_connection(timeout=5.0), (
            "Serial Studio did not connect within 5 seconds"
        )
        assert api.is_connected()

    def test_two_sources_listed(self, dual_serial_env):
        """source.list must return two sources, both with busType 0 (UART)."""
        api, ports = dual_serial_env

        sources = api.source_list()
        assert len(sources) == 2, f"Expected 2 sources, got {len(sources)}"

        for src in sources:
            assert src["busType"] == 0, (
                f"Source {src['sourceId']} busType should be 0 (UART), "
                f"got {src['busType']}"
            )


class TestDualSerialDataFlow:
    """Verify that frames sent on each PTY reach the correct dashboard group."""

    def test_both_sources_receive_data(self, dual_serial_env):
        """
        Send known values on both PTYs and verify each dashboard group
        receives the correct data independently.
        """
        api, ports = dual_serial_env

        api.connect_device()
        assert api.wait_for_connection(timeout=5.0)
        time.sleep(0.3)

        alpha_values = ["25.0", "60.0", "1013.0"]
        bravo_values = ["18.5", "45.0", "995.0"]

        _send_alpha_frame(ports, [25.0, 60.0, 1013.0])
        _send_bravo_frame(ports, [18.5, 45.0, 995.0])

        got_alpha = _wait_for_group_values(api, 0, alpha_values)
        got_bravo = _wait_for_group_values(api, 1, bravo_values)

        assert got_alpha == alpha_values, (
            f"Alpha group: expected {alpha_values}, got {got_alpha}"
        )
        assert got_bravo == bravo_values, (
            f"Bravo group: expected {bravo_values}, got {got_bravo}"
        )

    def test_no_cross_contamination(self, dual_serial_env):
        """
        Send different values on each port and confirm no cross-contamination
        between groups.
        """
        api, ports = dual_serial_env

        api.connect_device()
        assert api.wait_for_connection(timeout=5.0)
        time.sleep(0.3)

        # Send only on alpha
        _send_alpha_frame(ports, [99.0, 88.0, 77.0])
        got_alpha = _wait_for_group_values(api, 0, ["99.0", "88.0", "77.0"])
        assert got_alpha == ["99.0", "88.0", "77.0"]

        # Bravo should still show default/empty values (no data sent yet)
        got_bravo = _get_group_values(api, 1)
        assert got_bravo != ["99.0", "88.0", "77.0"], (
            "Bravo group received alpha's data — cross-contamination detected"
        )

        # Now send on bravo and verify
        _send_bravo_frame(ports, [11.0, 22.0, 33.0])
        got_bravo = _wait_for_group_values(api, 1, ["11.0", "22.0", "33.0"])
        assert got_bravo == ["11.0", "22.0", "33.0"]

        # Alpha should still show its own values
        got_alpha = _get_group_values(api, 0)
        assert got_alpha == ["99.0", "88.0", "77.0"], (
            "Alpha group was overwritten by bravo's data"
        )


class TestDualSerialSequentialUpdates:
    """Verify that multiple rounds of frames update correctly."""

    def test_sequential_updates(self, dual_serial_env):
        """
        Send several rounds of frames and verify the dashboard reflects
        the latest values from each source.
        """
        api, ports = dual_serial_env

        api.connect_device()
        assert api.wait_for_connection(timeout=5.0)
        time.sleep(0.3)

        rounds = [
            ([10.0, 20.0, 30.0], [40.0, 50.0, 60.0]),
            ([11.0, 21.0, 31.0], [41.0, 51.0, 61.0]),
            ([12.0, 22.0, 32.0], [42.0, 52.0, 62.0]),
        ]

        for alpha_vals, bravo_vals in rounds:
            _send_alpha_frame(ports, alpha_vals)
            _send_bravo_frame(ports, bravo_vals)
            time.sleep(0.2)

        # Verify final round values
        expected_alpha = ["12.0", "22.0", "32.0"]
        expected_bravo = ["42.0", "52.0", "62.0"]

        got_alpha = _wait_for_group_values(api, 0, expected_alpha)
        got_bravo = _wait_for_group_values(api, 1, expected_bravo)

        assert got_alpha == expected_alpha, (
            f"Alpha final: expected {expected_alpha}, got {got_alpha}"
        )
        assert got_bravo == expected_bravo, (
            f"Bravo final: expected {expected_bravo}, got {got_bravo}"
        )


class TestDualSerialDifferentRates:
    """Verify that different data rates on each port work correctly."""

    def test_different_rates(self, dual_serial_env):
        """
        Send 10 frames on alpha at 10 Hz and 5 frames on bravo at 5 Hz
        concurrently, then verify both groups received their final values.
        """
        api, ports = dual_serial_env

        api.connect_device()
        assert api.wait_for_connection(timeout=5.0)
        time.sleep(0.3)

        def send_alpha_burst():
            for i in range(10):
                _send_alpha_frame(ports, [float(i), float(i * 2), float(i * 3)])
                time.sleep(0.1)

        def send_bravo_burst():
            for i in range(5):
                _send_bravo_frame(
                    ports, [float(100 + i), float(200 + i), float(300 + i)]
                )
                time.sleep(0.2)

        t_alpha = threading.Thread(target=send_alpha_burst)
        t_bravo = threading.Thread(target=send_bravo_burst)

        t_alpha.start()
        t_bravo.start()
        t_alpha.join()
        t_bravo.join()

        # Final alpha frame: i=9 → [9.0, 18.0, 27.0]
        expected_alpha = ["9.0", "18.0", "27.0"]
        # Final bravo frame: i=4 → [104.0, 204.0, 304.0]
        expected_bravo = ["104.0", "204.0", "304.0"]

        got_alpha = _wait_for_group_values(api, 0, expected_alpha)
        got_bravo = _wait_for_group_values(api, 1, expected_bravo)

        assert got_alpha == expected_alpha, (
            f"Alpha after burst: expected {expected_alpha}, got {got_alpha}"
        )
        assert got_bravo == expected_bravo, (
            f"Bravo after burst: expected {expected_bravo}, got {got_bravo}"
        )
