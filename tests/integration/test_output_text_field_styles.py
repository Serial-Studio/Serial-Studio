"""
Output Text Field Styles and Sizes Tests (spec 0118)

A text field output widget reports its style and size over the API, and patching
either through project.outputWidget.update changes the dashboard without a reload.

Requires a running Serial Studio instance with the API server enabled.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import socket
import threading
import time

import pytest

from utils.api_client import APIError

PORT = 9031
MAIN = "main"
CONSENT_WAIT = 60.0
GROUP = 1
WIDGET = 0
PREFIX = f"dashboard.output.{GROUP}.{WIDGET}"
STYLE_READOUT = 4
INPUT_TEXT = 0
INPUT_NUMBER = 1


class LoopbackDevice:
    """A TCP server that streams one CSV line at a time and swallows what the app sends."""

    def __init__(self):
        self._running = True
        self._server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._server.bind(("127.0.0.1", PORT))
        self._server.listen(1)
        self._server.settimeout(0.2)
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def _loop(self):
        while self._running:
            try:
                conn, _ = self._server.accept()
            except (socket.timeout, OSError):
                continue
            conn.settimeout(0.2)
            while self._running:
                try:
                    conn.sendall(b"1\n")
                    conn.recv(4096)
                except socket.timeout:
                    pass
                except OSError:
                    break
                time.sleep(0.05)
            conn.close()

    def stop(self):
        self._running = False
        self._thread.join(timeout=2.0)
        self._server.close()


def project():
    """One source, a datagrid group and an output group holding one text field."""
    return {
        "title": "Text Field Styles",
        "frameEnd": "\\n",
        "frameDetection": 0,
        "decoder": 0,
        "frameParserCode": "function parse(frame) {\n  return frame.split(',');\n}\n",
        "frameParserLanguage": 0,
        "groups": [
            {
                "title": "Raw",
                "widget": "datagrid",
                "datasets": [
                    {"title": "First", "value": "%1", "index": 1, "widget": ""}
                ],
            },
            {
                "title": "Controls",
                "widget": "",
                "groupType": 1,
                "outputWidgets": [
                    {
                        "title": "Command",
                        "outputType": 3,
                        "sourceId": 0,
                        "icon": "",
                        "initialValue": 0,
                        "outputMin": 0,
                        "outputMax": 100,
                        "outputStep": 1,
                        "outputTxEncoding": 0,
                        "transmitFunction": (
                            "function transmit(value) {\n"
                            '  return "CMD " + value + "\\n";\n}'
                        ),
                    }
                ],
            },
        ],
        "actions": [],
    }


def wait_until(predicate, timeout=5.0):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return True
        time.sleep(0.1)
    return False


def drive(api_client, command, params):
    """Retries while the user has not yet answered the driving consent."""
    deadline = time.time() + CONSENT_WAIT
    while True:
        try:
            return api_client.command(command, params)
        except APIError as error:
            if error.code != "CONSENT_REQUIRED" or time.time() >= deadline:
                raise
            time.sleep(0.5)


def element_ids(api_client):
    params = {"window": MAIN, "id": PREFIX}
    elements = drive(api_client, "ui.automation.listElements", params)["elements"]
    return [e["id"] for e in elements if e.get("visible", True)]


def has_suffix(api_client, suffix):
    return any(i.endswith(suffix) for i in element_ids(api_client))


def get_widget(api_client):
    return api_client.command(
        "project.outputWidget.get", {"groupId": GROUP, "widgetId": WIDGET}
    )


def update_widget(api_client, **fields):
    params = {"groupId": GROUP, "widgetId": WIDGET}
    params.update(fields)
    return api_client.command("project.outputWidget.update", params)


@pytest.fixture
def connected_field(api_client, clean_state):
    """The project loaded and connected to the loopback device, output controls on screen."""
    if not api_client.command_exists("licensing.getStatus"):
        pytest.skip("Commercial build required for output controls")
    if not api_client.command("ui.automation.listWindows")["windows"]:
        pytest.skip("no visible window: the app runs headless or offscreen")

    device = LoopbackDevice()
    try:
        drive(api_client, "project.loadJson", {"config": project()})
        time.sleep(0.3)
        api_client.configure_network(host="127.0.0.1", port=PORT, socket_type="tcp")
        api_client.set_operation_mode("project")
        try:
            api_client.command("project.activate")
        except APIError:
            pass
        api_client.connect_device()
        assert wait_until(api_client.is_connected, 10.0), "the app never connected"
        assert wait_until(
            lambda: has_suffix(api_client, ".input"), 10.0
        ), "the text field never appeared on the dashboard"
        yield api_client
    finally:
        try:
            if api_client.is_connected():
                api_client.disconnect_device()
        except APIError:
            pass
        device.stop()


@pytest.mark.integration
@pytest.mark.gui
class TestOutputTextFieldStyles:
    """AC6: style and size read back and apply live."""

    def test_defaults(self, connected_field):
        """A fresh text field is Standard style at Normal size."""
        widget = get_widget(connected_field)
        assert widget.get("type") == 3, widget
        assert widget.get("style") == 0, widget
        assert widget.get("size") == 1, widget

    def test_readout_style_applies_live(self, connected_field):
        """Style 4 on a number field reads back and adds the readout without a reload."""
        assert not has_suffix(connected_field, ".readout")

        update_widget(connected_field, style=STYLE_READOUT, inputKind=INPUT_NUMBER)
        widget = get_widget(connected_field)
        assert widget.get("style") == STYLE_READOUT, widget
        assert widget.get("inputKind") == INPUT_NUMBER, widget
        assert wait_until(
            lambda: has_suffix(connected_field, ".readout")
        ), f"no readout element: {element_ids(connected_field)}"

    def test_size_applies(self, connected_field):
        """Size 3 (extra large) reads back after the update."""
        update_widget(connected_field, size=3)
        assert get_widget(connected_field).get("size") == 3

        update_widget(connected_field, size=1)
        assert get_widget(connected_field).get("size") == 1

    def test_readout_on_text_kind_draws_standard(self, connected_field):
        """R6: the readout style is stored on a text field but draws as Standard."""
        update_widget(connected_field, style=STYLE_READOUT, inputKind=INPUT_NUMBER)
        assert wait_until(lambda: has_suffix(connected_field, ".readout"))

        update_widget(connected_field, style=STYLE_READOUT, inputKind=INPUT_TEXT)
        widget = get_widget(connected_field)
        assert widget.get("style") == STYLE_READOUT, widget
        assert widget.get("inputKind") == INPUT_TEXT, widget
        assert wait_until(
            lambda: not has_suffix(connected_field, ".readout")
        ), f"readout still visible: {element_ids(connected_field)}"
        assert has_suffix(connected_field, ".input"), element_ids(connected_field)
