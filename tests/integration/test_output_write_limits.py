"""
Output Control Write Limit Integration Tests (spec 0109)

  * AC1 - a Number text field refuses an out-of-range value (nothing reaches the device, the
    out-of-range message is listed) and sends an in-range one once, quantised to Step. Marker
    `gui`: it types into the live dashboard through the spec-0101 driving surface, so it needs
    the GUI app on a desk, and SERIAL_STUDIO_API_AUTO_CONSENT=1 (or one answered prompt).
  * AC2 - project.outputWidget.get reports kind, Min, Max, Step and units.
  * AC3 - a Text field stays Text and a Number field stays Number through a round trip, and a
    widget saved before the spec carries none of the new keys.
  * AC5 and R3, structurally (no app required) - no clamp in the manual's examples or the two
    bundled templates, and the transmit script is reached only through the admission in
    Base::sendValue.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import re
import time
from pathlib import Path

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError
from utils.device_simulator import DeviceSimulator

# ---------------------------------------------------------------------------
# Structural guarantees (no running app required)
# ---------------------------------------------------------------------------

_REPO_ROOT = Path(__file__).resolve().parents[2]
_OUTPUT_DIR = _REPO_ROOT / "core" / "Ui" / "UI" / "Widgets" / "Output"
_MANUAL = _REPO_ROOT / "doc" / "help" / "Output-Controls.md"
_TEMPLATES = _REPO_ROOT / "app" / "rcc" / "scripts" / "output"
_CLAMP = re.compile(r"Math\.max\([^;]*Math\.min\(|Math\.min\([^;]*Math\.max\(", re.S)


def _function_body(text: str, name: str) -> str:
    """The brace-delimited body of the member function `name` in a class .cpp."""
    definition = text.index(f"::{name}(")
    opening = text.index("{", definition)
    closing = text.index("\n}\n", opening)
    return text[opening:closing]


@pytest.mark.parametrize("template", ["pwm_control.js", "pid_setpoint.js"])
def test_static_bundled_templates_do_not_clamp(template):
    """AC5: the control holds the range, so the bundled scripts carry no clamp."""
    code = (_TEMPLATES / template).read_text(encoding="utf-8")
    assert not _CLAMP.search(code), f"{template} still clamps its value"


def test_static_manual_examples_do_not_clamp():
    """AC5: no javascript example in the output controls page clamps the value."""
    text = _MANUAL.read_text(encoding="utf-8")
    fences = re.findall(r"```javascript\n(.*?)```", text, re.S)
    assert fences, "the manual lost its transmit examples"
    for fence in fences:
        assert not _CLAMP.search(fence), f"a manual example clamps:\n{fence}"


def test_static_script_is_reached_only_through_admission():
    """
    R3: deliverValue runs from sendValue and flushPendingValue only, and sendValue admits the
    value before it paces. A control kind added later inherits the check by calling sendValue.
    """
    text = (_OUTPUT_DIR / "Base.cpp").read_text(encoding="utf-8")
    send = _function_body(text, "sendValue")
    flush = _function_body(text, "flushPendingValue")

    outside = text.replace(send, "").replace(flush, "")
    calls = [m for m in re.finditer(r"\bdeliverValue\(", outside)]
    definition = [
        m for m in calls if outside[max(0, m.start() - 2) : m.start()] == "::"
    ]
    assert len(calls) == len(
        definition
    ), "deliverValue is called outside sendValue/flush"

    admit = send.find("admit")
    pacing = min(
        (i for i in (send.find("m_rateLimiter"), send.find("m_hasPending")) if i >= 0),
        default=-1,
    )
    assert admit >= 0, "sendValue no longer admits the value"
    assert pacing < 0 or admit < pacing, "admission must precede pacing in sendValue"


# ---------------------------------------------------------------------------
# Project builders
# ---------------------------------------------------------------------------

TEXT_FIELD = 3
SLIDER = 1
SETPOINT_FN = 'function transmit(value) { return "SP " + value + "\\n"; }'


def _number_field(title="Setpoint", **overrides):
    widget = {
        "title": title,
        "outputType": TEXT_FIELD,
        "inputKind": 1,
        "outputMin": 0,
        "outputMax": 100,
        "outputStep": 5,
        "outputUnits": "%",
        "quantizeToStep": True,
        "transmitFunction": SETPOINT_FN,
    }
    widget.update(overrides)
    return widget


def _project(output_widgets):
    """A CSV project with a dataset group and a dedicated Output Panel group (index 1)."""
    return {
        "title": "Output Write Limits Test",
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
                    {"title": "Sensor1", "value": "%1", "index": 1, "widget": ""}
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


def _exported_widgets(api_client):
    result = api_client.command("project.exportJson")
    config = result["config"]
    if isinstance(config, str):
        config = json.loads(config)
    return config["groups"][1].get("outputWidgets", [])


@pytest.fixture(autouse=True)
def _require_commercial_for_runtime(request):
    """Static tests need nothing; the rest need a Pro build, checked on first use."""
    if "api_client" in request.fixturenames:
        client = request.getfixturevalue("api_client")
        if not client.command_exists("licensing.getStatus"):
            pytest.skip("Commercial build required for output widget tests")


# ---------------------------------------------------------------------------
# AC2, AC3
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
def test_describe_reports_kind_range_step_and_units(api_client, clean_state):
    """AC2: the control's description carries everything a client needs before driving it."""
    api_client.load_project_from_json(_project([_number_field()]))
    time.sleep(0.3)

    described = api_client.command(
        "project.outputWidget.get", {"groupId": 1, "widgetId": 0}
    )
    assert described["type"] == TEXT_FIELD
    assert described["inputKind"] == 1
    assert described["minValue"] == 0
    assert described["maxValue"] == 100
    assert described["stepSize"] == 5
    assert described["units"] == "%"
    assert described["quantize"] is True


@pytest.mark.integration
@pytest.mark.project
def test_text_stays_text_and_number_stays_number(api_client, clean_state):
    """AC3, half one: both kinds survive a load and an export."""
    text = {
        "title": "Console",
        "outputType": TEXT_FIELD,
        "transmitFunction": SETPOINT_FN,
    }
    api_client.load_project_from_json(_project([text, _number_field()]))
    time.sleep(0.3)

    widgets = _exported_widgets(api_client)
    assert len(widgets) == 2
    assert "inputKind" not in widgets[0]
    assert widgets[1]["inputKind"] == 1
    assert widgets[1]["outputUnits"] == "%"
    assert widgets[1]["quantizeToStep"] is True


@pytest.mark.integration
@pytest.mark.project
def test_widget_saved_before_the_spec_gains_no_keys(api_client, clean_state):
    """AC3, half two: a pre-spec widget is exported with none of the four new keys."""
    legacy = [
        {
            "title": "Fan",
            "outputType": SLIDER,
            "outputMin": 0,
            "outputMax": 255,
            "outputStep": 1,
            "transmitFunction": SETPOINT_FN,
        },
        {"title": "Console", "outputType": TEXT_FIELD, "transmitFunction": SETPOINT_FN},
    ]
    api_client.load_project_from_json(_project(legacy))
    time.sleep(0.3)

    for widget in _exported_widgets(api_client):
        for key in ("inputKind", "outputDecimals", "outputUnits", "quantizeToStep"):
            assert key not in widget, f"{widget['title']} gained {key!r}"


# ---------------------------------------------------------------------------
# AC1 (gui)
# ---------------------------------------------------------------------------

MAIN = "main"
CELL = "dashboard.output.1.0"
SETTLE_SECONDS = 0.5
CAPTURE_WINDOW = 0.5
LINK_TIMEOUT = 15.0
CONSENT_WAIT = 25.0


def _elements(client, **filters):
    params = {"window": MAIN}
    params.update(filters)
    return client.command("ui.automation.listElements", params)["elements"]


def _listed(client, element_id):
    return any(e["id"] == element_id for e in _elements(client, id=element_id))


def _drive(client, command, params):
    """Issue a driving command, waiting out the one interface-driving consent prompt."""
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


@pytest.fixture
def capturing_device(api_client):
    """A TCP device that keeps the bytes the app writes to it."""
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


@pytest.fixture
def number_field_dashboard(api_client, clean_state, capturing_device):
    """The Number field on a live dashboard, linked to the capturing device."""
    if not api_client.command("ui.automation.listWindows")["windows"]:
        pytest.skip("no visible window: the app runs headless or offscreen")

    api_client.load_project_from_json(_project([_number_field()]))
    time.sleep(0.3)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except APIError:
        pass

    api_client.connect_device()
    assert capturing_device.wait_for_connection(
        LINK_TIMEOUT
    ), "the device saw no client"
    capturing_device.send_frame(b"/*42*/\n")
    assert _wait_until(
        lambda: _listed(api_client, f"{CELL}.input"), LINK_TIMEOUT
    ), "the Number field never appeared on the dashboard"
    capturing_device.clear_received()
    yield capturing_device


def _type_and_send(client, text):
    _drive(client, "ui.automation.click", {"window": MAIN, "id": f"{CELL}.input"})
    _drive(
        client,
        "ui.automation.key",
        {"window": MAIN, "target": f"{CELL}.input", "text": text},
    )
    _drive(
        client,
        "ui.automation.key",
        {"window": MAIN, "target": f"{CELL}.input", "keys": "Return"},
    )


@pytest.mark.integration
@pytest.mark.gui
def test_out_of_range_is_refused_and_in_range_is_quantised(
    api_client, number_field_dashboard
):
    """AC1: 250 is refused with the range shown and no bytes; 43 arrives once as 45."""
    device = number_field_dashboard

    _type_and_send(api_client, "250")
    time.sleep(CAPTURE_WINDOW)
    assert device.received() == b"", "an out-of-range value reached the device"
    assert _listed(api_client, f"{CELL}.outOfRange"), "no out-of-range message is shown"

    _drive(
        api_client,
        "ui.automation.key",
        {"window": MAIN, "target": f"{CELL}.input", "keys": "Ctrl+A"},
    )
    _type_and_send(api_client, "43")
    assert _wait_until(
        lambda: device.received() != b"", 2.0
    ), "the in-range value was lost"
    time.sleep(CAPTURE_WINDOW)
    assert device.received() == b"SP 45\n"
    assert not _listed(api_client, f"{CELL}.outOfRange"), "the message did not clear"
