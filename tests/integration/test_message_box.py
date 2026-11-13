"""
Message box API (ui.window.showMessageBox).

The runtime cases exercise only the refusals, so the suite never leaves a modal dialog on a desk:
an empty headline, an unknown icon, button options without wait, and wait from a caller that is
not a control script or macro. The static cases pin the wiring (spec 0120 AC4): only the
script marshaller grants wait, and the control script and macros share it, so a frame parser or
transform on the data path can never block on a dialog; the box is interface-driving for remote
clients and stays allowed in sealed sessions; the script command refusals live in one place.

A stop throws into the apiCall it lands in (R5), so one runtime case cycles the link under a script
that calls apiCall as fast as it can and checks that no stop is reported as a script error.

The `gui` case (AC5) needs the app on a desk and is deselected in CI. It shows a box from a
control script and checks the provenance line, that automation lists it as modal, that an
automation Escape answers cancel, and that disconnecting closes it and stops the script. Two
checks stay with the maintainer's eyes: the box never shows the app icon, and quitting while one
is open exits promptly with no "did not stop" warning.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import re
import time
from pathlib import Path

import pytest

from utils.api_client import APIError

COMMAND = "ui.window.showMessageBox"
REPO = Path(__file__).resolve().parents[2]


def _refused(api_client, params):
    with pytest.raises(APIError) as exc:
        api_client.command(COMMAND, params)

    return exc.value


@pytest.mark.integration
class TestMessageBoxRefusals:
    """Every refusal answers before a dialog is built."""

    def test_command_is_registered(self, api_client):
        assert api_client.command_exists(COMMAND)

    def test_empty_text_is_refused(self, api_client):
        error = _refused(api_client, {"text": "   "})
        assert error.code == "MISSING_PARAM"

    def test_unknown_icon_is_refused(self, api_client):
        error = _refused(api_client, {"text": "Hello", "icon": "sparkles"})
        assert error.code == "INVALID_PARAM"
        assert "sparkles" in error.message

    def test_buttons_without_wait_are_refused(self, api_client):
        error = _refused(api_client, {"text": "Hello", "buttons": ["yes", "no"]})
        assert error.code == "INVALID_PARAM"
        assert "wait" in error.message

    def test_wait_from_an_external_client_is_refused(self, api_client):
        error = _refused(api_client, {"text": "Hello", "wait": True})
        assert error.code == "EXECUTION_ERROR"
        assert "control script" in error.message


def _source(relative: str) -> str:
    return (REPO / relative).read_text(encoding="utf-8", errors="replace")


def _array_body(text: str, name: str) -> str:
    """The text between `name[] = {` and its closing `};`."""
    start = text.index(f"{name}[] = {{")
    return text[start : text.index("};", start)]


def test_static_only_the_script_marshaller_grants_wait():
    """A wait permission opened anywhere else could park the pipeline behind a dialog. The control
    script and JS macros share the one marshaller that grants it (spec 0120 R7)."""
    grants = []
    pattern = re.compile(r"UserWaitScope\s+\w+\s*\(\s*true\b")
    for path in (REPO / "core").rglob("*.cpp"):
        if pattern.search(path.read_text(encoding="utf-8", errors="replace")):
            grants.append(path.relative_to(REPO).as_posix())

    assert grants == ["core/Pipeline/DataModel/Scripting/ControlScriptWorker.cpp"]
    worker = _source("core/Pipeline/DataModel/Scripting/ControlScriptWorker.cpp")
    assert re.search(
        r"UserWaitScope\s+\w+\s*\(\s*true\s*,\s*this\s*\)", worker
    ), "the grant must name its marshaller, or a stop cannot tell its own box from another's"
    macros = _source("core/Pipeline/DataModel/Scripting/MacroWorker.cpp")
    assert "ControlApiMarshaller* marshaller" in macros


def test_static_the_box_is_interface_driving_and_sealed_allowed():
    """A remote client asks the interface-actions consent once; a sealed deployment's own scripts
    can still show boxes (spec 0120 R3)."""
    capabilities = _source("core/Api/API/CommandCapabilities.cpp")
    capable = _array_body(capabilities, "kCapable")
    plain = _array_body(capabilities, "kPlain")
    assert re.search(r'"ui\.window\.showMessageBox"\),\s*DrivesUi,', capable)
    assert f'"{COMMAND}"' not in plain

    sealed = _source("core/Api/API/SealedPolicy.cpp")
    assert f'"{COMMAND}"' in _array_body(sealed, "kAllowed")
    assert f'"{COMMAND}"' not in _array_body(sealed, "kDenied")


def test_static_script_refusals_live_in_one_place():
    """The refusal list is ScriptCommandPolicy's alone, consulted only for script origin."""
    users = []
    for root in ("core", "app/src"):
        for path in (REPO / root).rglob("*"):
            if path.suffix not in (".cpp", ".h"):
                continue
            if "ScriptCommandPolicy::" in path.read_text(
                encoding="utf-8", errors="replace"
            ):
                users.append(path.relative_to(REPO).as_posix())

    assert users == ["core/Api/API/CommandHandler.cpp"]
    handler = _source("core/Api/API/CommandHandler.cpp")
    execute = handler[handler.index("API::CommandHandler::execute(") :]
    assert "CommandOrigin::Script" in execute[: execute.index("\n}")]


# ---------------------------------------------------------------------------
# AC5 -- a box on a desk (maintainer-run, deselected in CI)
# ---------------------------------------------------------------------------

MAIN = "main"
DIALOG = "dialog"
TABLE = "Box"
PROVENANCE = "Sent by the project's scripts or an API client, not by Serial Studio."
CONSENT_WAIT = 25.0

_BOX_SCRIPT = """
var answers = 0;

function setup() {}

function loop() {
  var r = apiCall("ui.window.showMessageBox", {
    text: "Spec 0120 box",
    informativeText: "Press nothing.",
    icon: "question",
    wait: true,
    buttons: ["ok"]
  });
  answers += 1;
  tableSet("%(table)s", "answers", answers);
  tableSet("%(table)s", "cancelled", r.ok && r.result.button === "cancel" ? 1 : 0);
  delay(100);
}
"""


def _box_project() -> dict:
    return {
        "title": "Message box",
        "frameEnd": "\\n",
        "frameDetection": 0,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device",
                "busType": 0,
                "frameStart": "",
                "frameEnd": "\\n",
                "frameDetection": 0,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": "function parse(frame) { return frame.split(','); }",
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "tables": [
            {
                "name": TABLE,
                "registers": [
                    {"name": "answers", "type": "computed", "value": 0},
                    {"name": "cancelled", "type": "computed", "value": -1},
                    {"name": "ticks", "type": "computed", "value": 0},
                ],
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [{"title": "A", "value": "%1", "index": 1}],
            }
        ],
        "actions": [],
    }


def _wait_until(predicate, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if predicate():
            return True
        time.sleep(0.1)
    return False


def _windows(api_client) -> list:
    return api_client.command("ui.automation.listWindows")["windows"]


def _dialog(api_client):
    return next((w for w in _windows(api_client) if w["id"] == DIALOG), None)


def _register(api_client, name: str):
    result = api_client.command(
        "project.dataTable.getValue", {"table": TABLE, "name": name}
    )
    return result.get("value")


def _connect_box_project(api_client, device_simulator, script: str) -> None:
    api_client.load_project_from_json(_box_project())
    time.sleep(0.3)
    api_client.command("controlScript.set", {"code": script})
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except APIError:
        pass

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _finding_codes(api_client) -> list:
    problems = api_client.command("problems.run")
    return [f.get("code") for f in problems.get("findings", [])]


_TICKING_SCRIPT = """
var ticks = 0;

function setup() {}

function loop() {
  ticks += 1;
  tableSet("%(table)s", "ticks", ticks);
}
"""


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_a_stop_mid_api_call_is_no_script_error(
    api_client, clean_state, device_simulator
):
    """A stop now throws into the apiCall it lands in (R5). That throw must end the run quietly:
    reported as an error, every disconnect of a script that calls apiCall in its loop would raise
    control-script-stopped, and a restart would mark the new run stopped."""
    _connect_box_project(
        api_client, device_simulator, _TICKING_SCRIPT % {"table": TABLE}
    )

    for cycle in range(3):
        assert _wait_until(
            lambda: (_register(api_client, "ticks") or 0) > 0, 5.0
        ), f"the script never ticked (cycle {cycle})"
        api_client.disconnect_device()
        time.sleep(0.5)
        assert "control-script-stopped" not in _finding_codes(api_client)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)
        assert _wait_until(
            lambda: api_client.command("controlScript.getStatus")["running"], 5.0
        ), f"the script did not run again after reconnecting (cycle {cycle})"

    assert "control-script-stopped" not in _finding_codes(api_client)


def _prime_ui_consent(api_client) -> None:
    """Asks the interface-actions consent before any box is up: a click on an id that does not
    exist drives nothing once granted."""
    deadline = time.monotonic() + CONSENT_WAIT
    while True:
        try:
            api_client.command(
                "ui.automation.click", {"window": MAIN, "id": "spec0120.absent"}
            )
            return
        except APIError as error:
            if error.code != "CONSENT_REQUIRED":
                return
            if time.monotonic() >= deadline:
                raise
            time.sleep(0.5)


@pytest.mark.integration
@pytest.mark.gui
def test_a_script_box_is_marked_listed_cancelled_and_closed_on_stop(
    api_client, clean_state, device_simulator
):
    if not _windows(api_client):
        pytest.skip("no visible window: the app runs headless or offscreen")

    _prime_ui_consent(api_client)
    _connect_box_project(api_client, device_simulator, _BOX_SCRIPT % {"table": TABLE})
    assert _wait_until(lambda: _dialog(api_client) is not None, 5.0), "no box appeared"

    dialog = _dialog(api_client)
    main = next(w for w in _windows(api_client) if w["id"] == MAIN)
    assert dialog["modal"] and main["modal"]
    assert "Spec 0120 box" in dialog["text"]
    assert PROVENANCE in dialog["informativeText"]

    api_client.command("ui.automation.key", {"window": MAIN, "keys": "Escape"})
    assert _wait_until(
        lambda: (_register(api_client, "answers") or 0) >= 1, 5.0
    ), "Escape did not answer the waiting call"
    assert _register(api_client, "cancelled") == 1

    assert _wait_until(
        lambda: _dialog(api_client) is not None, 5.0
    ), "the loop stopped asking"
    api_client.disconnect_device()
    assert _wait_until(
        lambda: _dialog(api_client) is None, 5.0
    ), "stopping left the box open"
    time.sleep(1.0)
    assert _dialog(api_client) is None, "the stopped script asked again"
