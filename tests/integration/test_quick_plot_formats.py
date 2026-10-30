"""
Quick Plot Multi-Format Input Tests (spec 0106)

Quick Plot detects the line format from the first numeric line: named fields
(temp:23.5) title channels, a unit after a number becomes the dataset units, a
text line never changes the dashboard structure, and the frame parser dry run
previews the detected split.

Requires a running Serial Studio instance with the API server enabled.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

DEGREE_C = "°C"
JS_SPLIT_PARSER = "function parse(frame) { return frame.split(','); }"


def dashboard_datasets(api_client):
    """Title -> dataset dict from the live dashboard frame (dashboard.getData)."""
    frame = api_client.command("dashboard.getData").get("frame", {})
    out = {}
    for group in frame.get("groups", []):
        for dataset in group.get("datasets", []):
            out[dataset.get("title")] = dataset
    return out


def wait_for_datasets(api_client, count, timeout=8.0):
    """Poll until the dashboard frame holds at least count datasets."""
    deadline = time.time() + timeout
    datasets = {}
    while time.time() < deadline:
        datasets = dashboard_datasets(api_client)
        if len(datasets) >= count:
            return datasets
        time.sleep(0.1)
    return datasets


def wait_for_value(api_client, title, expected, timeout=8.0):
    """Poll until the dataset with the given title shows the expected number."""
    deadline = time.time() + timeout
    datasets = {}
    while time.time() < deadline:
        datasets = dashboard_datasets(api_client)
        try:
            if abs(float(datasets[title].get("value")) - expected) < 1e-6:
                return datasets
        except (KeyError, TypeError, ValueError):
            pass
        time.sleep(0.1)
    return datasets


def connect_quick_plot(api_client, device_simulator):
    """Enter Quick Plot mode and connect the TCP device simulator."""
    api_client.set_operation_mode("quickplot")
    time.sleep(0.5)  # Let queued rebuildDevices settle before configuring
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.3)  # Let queued setBusType restoration settle
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def send_lines(device_simulator, lines):
    """Send each byte line to the connected device with a short pause."""
    for line in lines:
        device_simulator.send_frame(line)
        time.sleep(0.05)


@pytest.mark.integration
class TestQuickPlotFormats:
    """Quick Plot format detection over a live TCP device."""

    def test_named_lines_label_without_header(
        self, api_client, device_simulator, clean_state
    ):
        """Named fields title the channels with no header line."""
        connect_quick_plot(api_client, device_simulator)
        send_lines(
            device_simulator, [b"temp: 23.5, hum: 60\n", b"temp: 23.6, hum: 61\n"]
        )

        wait_for_value(api_client, "temp", 23.6)
        datasets = wait_for_value(api_client, "hum", 61.0)
        api_client.disconnect_device()

        assert {"temp", "hum"} <= set(datasets), f"named fields not titled: {datasets}"
        assert float(datasets["temp"]["value"]) == pytest.approx(23.6)
        assert float(datasets["hum"]["value"]) == pytest.approx(61.0)

    def test_unit_suffix_reaches_dataset(
        self, api_client, device_simulator, clean_state
    ):
        """A unit after a number becomes the dataset units."""
        connect_quick_plot(api_client, device_simulator)
        line = b"temp:23.5\xc2\xb0C;pressure:1013hPa\n"
        send_lines(device_simulator, [line, line])

        wait_for_value(api_client, "temp", 23.5)
        datasets = wait_for_value(api_client, "pressure", 1013.0)
        api_client.disconnect_device()

        assert {"temp", "pressure"} <= set(datasets), f"fields not titled: {datasets}"
        assert datasets["temp"].get("units") == DEGREE_C, f"bad units: {datasets}"
        assert datasets["pressure"].get("units") == "hPa", f"bad units: {datasets}"
        assert float(datasets["temp"]["value"]) == pytest.approx(23.5)
        assert float(datasets["pressure"]["value"]) == pytest.approx(1013.0)

    def test_text_line_leaves_structure(
        self, api_client, device_simulator, clean_state
    ):
        """A non-numeric text line never changes the dashboard structure."""
        connect_quick_plot(api_client, device_simulator)
        send_lines(device_simulator, [b"23.5,60\n"])

        before = wait_for_datasets(api_client, 2)
        assert len(before) == 2, f"expected two datasets: {before}"
        count_before = api_client.command("dashboard.getStatus").get("datasetCount")
        ids_before = sorted(d.get("uniqueId") for d in before.values())

        send_lines(device_simulator, [b"WiFi reconnecting...\n", b"23.6,61\n"])
        wait_for_value(api_client, list(before)[0], 23.6)
        time.sleep(0.5)

        after = dashboard_datasets(api_client)
        count_after = api_client.command("dashboard.getStatus").get("datasetCount")
        api_client.disconnect_device()

        assert count_after == count_before, "text line changed datasetCount"
        assert sorted(d.get("uniqueId") for d in after.values()) == ids_before
        values = sorted(float(d["value"]) for d in after.values())
        assert values == pytest.approx([23.6, 61.0]), f"wrong values: {after}"


DRY_RUN_CASES = [
    ("1,2,3\n", [["1", "2", "3"]]),
    ("1;2;3\n", [["1", "2", "3"]]),
    ("temp:1 hum:2\n", [["temp:1", "hum:2"]]),
    ('{"t":1,"h":2}\n', [["t:1", "h:2"]]),
    ("/*1,2*/\n", [["1", "2"]]),
]


@pytest.mark.integration
@pytest.mark.parametrize("input_bytes,rows", DRY_RUN_CASES)
def test_dry_run_previews_detected_split(api_client, clean_state, input_bytes, rows):
    """The Quick Plot dry run returns the detected split cells of one line."""
    result = api_client.command(
        "project.frameParser.dryRun",
        {
            "code": JS_SPLIT_PARSER,
            "language": 0,
            "operationMode": 2,  # SerialStudio::QuickPlot
            "inputBytes": input_bytes,
        },
    )

    assert result["ok"], f"dry run failed for {input_bytes!r}: {result}"
    assert result["frameCount"] == 1, f"expected 1 frame for {input_bytes!r}"
    assert result["frames"][0]["rows"] == rows
