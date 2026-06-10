"""
io.getLatestFrame -- Latest Raw Frame API Integration Tests

Verifies the FrameBuilder latest-frame capture exposed through the
io.getLatestFrame command: the raw payload of the most recent frame, the
parser's channel tokens (pre dataset mapping), a monotonic sequence number,
and the text/base64 encoding switch. This is the data source behind the
control script's newFrame() / ensureDashboard() helpers.

Capture is gated on an active consumer; the network API server used by these
tests is one, so the gate is open for the whole session.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import base64
import time

import pytest

from utils.api_client import APIError

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------

_JS_SPLIT_PARSER = """
function parse(frame) {
    return frame.split(',');
}
"""


def _make_project(api_client):
    """Create a project with one group and two plotted datasets."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.group.add", {"title": "Probe", "widgetType": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.1)


def _connect_device(api_client, device_simulator):
    """Wire up project + JS split parser + network driver, then connect."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="", end_sequence="\n", operation_mode=0, frame_detection=0
    )
    api_client.set_frame_parser_code(_JS_SPLIT_PARSER, language=0)
    api_client.command("project.activate")
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(
        timeout=5.0
    ), "Serial Studio did not connect to the device simulator within 5 seconds"


def _wait_for_frame(api_client, min_sequence: int = 1, timeout: float = 5.0) -> dict:
    """Poll io.getLatestFrame until hasData and sequence >= min_sequence."""
    end = time.time() + timeout
    result = {}
    while time.time() < end:
        result = api_client.command("io.getLatestFrame")
        if result.get("hasData") and result.get("sequence", 0) >= min_sequence:
            return result

        time.sleep(0.05)

    return result


# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------


class TestLatestFrameApi:
    """io.getLatestFrame over the network API."""

    def test_no_data_returns_hasdata_false(self, api_client, clean_state):
        """Before any frame arrives, the command reports a state, not an error."""
        _make_project(api_client)

        result = api_client.command("io.getLatestFrame")
        assert result.get("hasData") is False
        assert result.get("sequence") == 0

    def test_latest_frame_text_values_and_sequence(
        self, api_client, device_simulator, clean_state
    ):
        """The latest frame exposes raw text, parser tokens, and a sequence."""
        _make_project(api_client)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"1.5,2.5\n")
        result = _wait_for_frame(api_client)

        assert result.get("hasData") is True, f"no frame captured: {result}"
        assert result.get("sequence", 0) >= 1
        assert result.get("sourceId") == 0
        assert result.get("timestampMs", 0) > 0
        assert result.get("text", "").strip() == "1.5,2.5"
        assert result.get("values") == ["1.5", "2.5"]
        assert result.get("valueCount") == 2

        # Unmapped channels (only 2 datasets exist) must still be visible.
        first_sequence = result["sequence"]
        device_simulator.send_frame(b"3.5,4.5,5.5\n")
        result = _wait_for_frame(api_client, min_sequence=first_sequence + 1)

        assert result.get("sequence", 0) > first_sequence
        assert result.get("values") == ["3.5", "4.5", "5.5"]
        assert result.get("valueCount") == 3

    def test_encoding_switch(self, api_client, device_simulator, clean_state):
        """encoding=text|base64|both controls which payload fields appear."""
        _make_project(api_client)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"7.5,8.5\n")
        assert _wait_for_frame(api_client).get("hasData") is True

        text_only = api_client.command("io.getLatestFrame", {"encoding": "text"})
        assert "text" in text_only and "base64" not in text_only

        b64_only = api_client.command("io.getLatestFrame", {"encoding": "base64"})
        assert "base64" in b64_only and "text" not in b64_only
        decoded = base64.b64decode(b64_only["base64"]).decode("utf-8")
        assert decoded.strip() == "7.5,8.5"

        both = api_client.command("io.getLatestFrame", {"encoding": "both"})
        assert "text" in both and "base64" in both
        assert both["text"] == text_only["text"]

    def test_invalid_encoding_rejected(self, api_client, clean_state):
        """An unknown encoding value is a structured error, not a crash."""
        with pytest.raises(APIError):
            api_client.command("io.getLatestFrame", {"encoding": "hex"})

    def test_disconnect_clears_capture(self, api_client, device_simulator, clean_state):
        """Disconnecting drops the captured frame so stale data never leaks."""
        _make_project(api_client)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"1.0,2.0\n")
        assert _wait_for_frame(api_client).get("hasData") is True

        api_client.disconnect_device()
        time.sleep(0.3)

        result = api_client.command("io.getLatestFrame")
        assert result.get("hasData") is False
