"""
Acknowledged Commands Integration Tests (spec 0110)

  * AC1 - a button sends "BLINK <token>"; a device frame echoing the token with code 0 reads
    `acknowledged`, code 3 with a message reads `refused`, and no echo reads `unanswered`, both
    with frames still flowing and with the device silent. Marker `gui`: the outcome is read from
    the live dashboard through the spec-0101 driving surface (`state` on listElements), so it
    needs the GUI app on a desk and SERIAL_STUDIO_API_AUTO_CONSENT=1 (or one answered prompt).
  * AC3 - a toggle with both a state source and a reply holds the request after an
    acknowledgement until the state confirms it, and returns to the reported state on a refusal.
  * AC4 - the reply ids round-trip through update, get, export and load, and a widget written
    before the spec reads back unbound.
  * AC7 - a state-only toggle stops "waiting" within its window even when the device goes silent.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import re
import time

import pytest

from utils.api_client import APIError
from utils.device_simulator import DeviceSimulator

BUTTON = 0
TOGGLE = 2
MAIN = "main"
GROUP = 1
CELL = f"dashboard.output.{GROUP}.0"
CONFIRM_MS = 800
LINK_TIMEOUT = 15.0
CONSENT_WAIT = 25.0
REPLY_KEYS = ("replyTokenDatasetId", "replyCodeDatasetId", "replyMessageDatasetId")

BLINK_FN = 'function transmit(value, token) { return "BLINK " + token + "\\n"; }'
TOGGLE_FN = (
    "function transmit(value, token) {"
    ' return (value ? "ON " : "OFF ") + token + "\\n"; }'
)


# ---------------------------------------------------------------------------
# Project builders and helpers
# ---------------------------------------------------------------------------


def _widget(kind, title, fn, **extra):
    widget = {
        "title": title,
        "outputType": kind,
        "outputMin": 0,
        "outputMax": 1,
        "outputStep": 1,
        "transmitFunction": fn,
    }
    widget.update(extra)
    return widget


def _project(output_widgets):
    """A CSV project: sensor, token, code and message datasets, then an Output Panel (group 1)."""
    names = ("Sensor", "ReplyToken", "ReplyCode", "ReplyMessage")
    return {
        "title": "Acknowledged Commands Test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "frameParserCode": "function parse(frame) {\n  return frame.split(',');\n}\n",
        "frameParserLanguage": 0,
        "groups": [
            {
                "title": "Data",
                "widget": "datagrid",
                "datasets": [
                    {"title": name, "value": f"%{i + 1}", "index": i + 1, "widget": ""}
                    for i, name in enumerate(names)
                ],
            },
            {
                "title": "Output Panel",
                "widget": "",
                "groupType": 1,
                "datasets": [],
                "outputWidgets": output_widgets,
            },
        ],
        "actions": [],
    }


def _exported(api_client):
    config = api_client.command("project.exportJson")["config"]
    return json.loads(config) if isinstance(config, str) else config


def _dataset_ids(api_client):
    """The uniqueIds of the four data-group datasets, in column order."""
    datasets = _exported(api_client)["groups"][0]["datasets"]
    return [d.get("uniqueId", i) for i, d in enumerate(datasets)]


def _bind_reply(api_client, with_state=False):
    ids = _dataset_ids(api_client)
    fields = {
        "groupId": GROUP,
        "widgetId": 0,
        "replyTokenDatasetId": ids[1],
        "replyCodeDatasetId": ids[2],
        "replyMessageDatasetId": ids[3],
        "stateConfirmMs": CONFIRM_MS,
    }
    if with_state:
        fields.update({"stateSource": 1, "stateDatasetId": ids[0]})
    api_client.command("project.outputWidget.update", fields)


@pytest.fixture(autouse=True)
def _require_commercial_for_runtime(request):
    if "api_client" in request.fixturenames:
        client = request.getfixturevalue("api_client")
        if not client.command_exists("licensing.getStatus"):
            pytest.skip("Commercial build required for output widget tests")


# ---------------------------------------------------------------------------
# AC4
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
def test_reply_ids_round_trip(api_client, clean_state):
    """AC4: the ids survive update, get, export and a load of that export."""
    api_client.load_project_from_json(_project([_widget(BUTTON, "Blink", BLINK_FN)]))
    time.sleep(0.3)
    _bind_reply(api_client)

    ids = _dataset_ids(api_client)
    got = api_client.command("project.outputWidget.get", {"groupId": GROUP, "widgetId": 0})
    assert got["replyTokenDatasetId"] == ids[1]
    assert got["replyCodeDatasetId"] == ids[2]
    assert got["replyMessageDatasetId"] == ids[3]
    assert got["stateConfirmMs"] == CONFIRM_MS

    exported = _exported(api_client)
    api_client.load_project_from_json(exported)
    time.sleep(0.3)
    again = api_client.command("project.outputWidget.get", {"groupId": GROUP, "widgetId": 0})
    assert again["replyTokenDatasetId"] == ids[1]
    assert again["replyCodeDatasetId"] == ids[2]


@pytest.mark.integration
@pytest.mark.project
def test_widget_saved_before_the_spec_is_unbound(api_client, clean_state):
    """AC4: a pre-spec widget reads back unbound and exports none of the three keys."""
    api_client.load_project_from_json(_project([_widget(BUTTON, "Blink", BLINK_FN)]))
    time.sleep(0.3)

    got = api_client.command("project.outputWidget.get", {"groupId": GROUP, "widgetId": 0})
    for key in REPLY_KEYS:
        assert key not in got, f"unbound widget reports {key}"

    for widget in _exported(api_client)["groups"][GROUP].get("outputWidgets", []):
        for key in REPLY_KEYS:
            assert key not in widget, f"unbound widget exported {key}"


# ---------------------------------------------------------------------------
# Live dashboard (gui)
# ---------------------------------------------------------------------------


def _elements(client, **filters):
    params = {"window": MAIN}
    params.update(filters)
    return client.command("ui.automation.listElements", params)["elements"]


def _element(client, element_id):
    for element in _elements(client, id=element_id):
        if element["id"] == element_id:
            return element
    return None


def _state(client):
    element = _element(client, CELL)
    return (element or {}).get("state", {})


def _drive(client, command, params):
    deadline = time.time() + CONSENT_WAIT
    while True:
        try:
            return client.command(command, params)
        except APIError as error:
            if error.code != "CONSENT_REQUIRED" or time.time() >= deadline:
                raise
            time.sleep(0.5)


def _wait_until(predicate, timeout):
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return True
        time.sleep(0.1)
    return False


def _frame(sensor, token, code, message):
    return f"/*{sensor},{token},{code},{message}*/\n".encode()


@pytest.fixture
def capturing_device(api_client):
    sim = DeviceSimulator(host="127.0.0.1", port=9000, protocol="tcp", capture=True)
    sim.start()
    yield sim
    try:
        if api_client.is_connected():
            api_client.disconnect_device()
            time.sleep(0.1)
    except Exception:
        pass
    sim.stop()


def _open_dashboard(api_client, device, widgets, control_suffix, bind):
    if not api_client.command("ui.automation.listWindows")["windows"]:
        pytest.skip("no visible window: the app runs headless or offscreen")

    api_client.load_project_from_json(_project(widgets))
    time.sleep(0.3)
    bind(api_client)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except APIError:
        pass

    api_client.connect_device()
    assert device.wait_for_connection(LINK_TIMEOUT), "the device saw no client"
    device.send_frame(_frame(0, 0, 0, "none"))
    assert _wait_until(
        lambda: _element(api_client, f"{CELL}.{control_suffix}") is not None, LINK_TIMEOUT
    ), "the control never appeared on the dashboard"
    device.clear_received()


def _press(client, device, suffix):
    """Click the control and return the token the command carried, if any."""
    before = _state(client).get("token", 0)
    _drive(client, "ui.automation.click", {"window": MAIN, "id": f"{CELL}.{suffix}"})
    assert _wait_until(lambda: device.received() != b"", 3.0), "nothing reached the device"
    match = re.search(rb"(\d+)\n$", device.received())
    assert match, f"no token in {device.received()!r}"
    token = int(match.group(1))
    assert token != before
    device.clear_received()
    return token


def _stream(device, frame, seconds=0.6):
    end = time.time() + seconds
    while time.time() < end:
        device.send_frame(frame)
        time.sleep(0.1)


@pytest.fixture
def button_dashboard(api_client, clean_state, capturing_device):
    _open_dashboard(
        api_client,
        capturing_device,
        [_widget(BUTTON, "Blink", BLINK_FN)],
        "button",
        _bind_reply,
    )
    return capturing_device


@pytest.mark.integration
@pytest.mark.gui
def test_acknowledged(api_client, button_dashboard):
    """AC1: the echoed token with code 0 reads acknowledged and names the token."""
    device = button_dashboard
    token = _press(api_client, device, "button")
    assert _state(api_client).get("outcome") == "waiting"

    _stream(device, _frame(0, token, 0, "OK"))
    state = _state(api_client)
    assert state.get("outcome") == "acknowledged"
    assert state.get("token") == token


@pytest.mark.integration
@pytest.mark.gui
def test_refused_with_message(api_client, button_dashboard):
    """AC1: code 3 with a message reads refused and carries the message."""
    device = button_dashboard
    token = _press(api_client, device, "button")

    _stream(device, _frame(0, token, 3, "too hot"))
    state = _state(api_client)
    assert state.get("outcome") == "refused"
    assert state.get("code") == 3
    assert state.get("message") == "too hot"


@pytest.mark.integration
@pytest.mark.gui
def test_unanswered_while_frames_flow(api_client, button_dashboard):
    """AC1: frames that never echo the token leave the command unanswered after the window."""
    device = button_dashboard
    token = _press(api_client, device, "button")

    _stream(device, _frame(0, token - 1 if token > 1 else 32767, 0, "stale"), seconds=1.6)
    assert _state(api_client).get("outcome") == "unanswered"


@pytest.mark.integration
@pytest.mark.gui
def test_unanswered_when_the_device_goes_silent(api_client, button_dashboard):
    """AC1 and R10: no frame arrives at all, and the window still ends the wait."""
    device = button_dashboard
    _press(api_client, device, "button")

    assert _wait_until(lambda: _state(api_client).get("outcome") == "unanswered", 1.8)


# ---------------------------------------------------------------------------
# AC3, AC7
# ---------------------------------------------------------------------------


@pytest.fixture
def toggle_with_both(api_client, clean_state, capturing_device):
    widgets = [_widget(TOGGLE, "Fan", TOGGLE_FN)]
    _open_dashboard(
        api_client,
        capturing_device,
        widgets,
        "toggle",
        lambda client: _bind_reply(client, with_state=True),
    )
    return capturing_device


@pytest.mark.integration
@pytest.mark.gui
def test_acknowledged_toggle_holds_until_state_confirms(api_client, toggle_with_both):
    """AC3: after an acknowledgement the request is held until the state reports it."""
    device = toggle_with_both
    token = _press(api_client, device, "toggle")

    _stream(device, _frame(0, token, 0, "OK"), seconds=0.4)
    state = _state(api_client)
    assert state.get("outcome") == "acknowledged"
    assert state.get("holding") is True

    _stream(device, _frame(1, token, 0, "OK"), seconds=0.4)
    assert _state(api_client).get("holding") is False
    assert _element(api_client, f"{CELL}.toggle").get("checked") is True


@pytest.mark.integration
@pytest.mark.gui
def test_refused_toggle_returns_to_the_reported_state(api_client, toggle_with_both):
    """AC3: a refusal releases the hold at once and the toggle follows the source again."""
    device = toggle_with_both
    token = _press(api_client, device, "toggle")

    _stream(device, _frame(0, token, 3, "interlock"), seconds=0.4)
    state = _state(api_client)
    assert state.get("outcome") == "refused"
    assert state.get("holding") is False
    assert _element(api_client, f"{CELL}.toggle").get("checked") is False


@pytest.mark.integration
@pytest.mark.gui
def test_state_only_wait_ends_when_the_device_goes_silent(api_client, clean_state, capturing_device):
    """AC7: a state-only toggle stops waiting within its window with no frame to prompt it."""
    widget = _widget(TOGGLE, "Fan", TOGGLE_FN)

    def bind_state_only(client):
        client.command(
            "project.outputWidget.update",
            {
                "groupId": GROUP,
                "widgetId": 0,
                "stateSource": 1,
                "stateDatasetId": _dataset_ids(client)[0],
                "stateConfirmMs": CONFIRM_MS,
            },
        )

    _open_dashboard(api_client, capturing_device, [widget], "toggle", bind_state_only)
    _stream(capturing_device, _frame(0, 0, 0, "none"), seconds=0.5)

    _press(api_client, capturing_device, "toggle")
    assert _wait_until(lambda: _state(api_client).get("waiting") is False, 1.8)
