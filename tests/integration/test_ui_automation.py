"""
GUI-driving integration tests for the spec-0101 automation surface (AC1 to AC4, AC16).

Marker `gui`: every test here drives the live window through the `ui.automation.*` commands,
so the GUI app must be up on a desk with the API server enabled. A headless or offscreen
instance lists no window and the module skips (AC16); CI deselects the marker.

Clicking and typing are behind the interface-driving consent: start the app with
SERIAL_STUDIO_API_AUTO_CONSENT=1, or answer the one prompt the first click shows. The module
switches reduced motion on for its duration so no reply waits on an entrance animation, and
restores the previous value at the end.

The tests that need a live link (the console's send controls are enabled only while one is
open) switch the app to Console mode, point the network source at the TCP device simulator and
connect; the previous mode and bus type come back afterwards. Waits poll through the module
client so its socket keeps draining: an API client that stops reading while frames stream is
dropped once its write backlog passes the server's cap.
"""

import struct
import base64
import time

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError

pytestmark = [pytest.mark.integration, pytest.mark.gui]

MAIN = "main"
SETTINGS = "settings"
CONNECT = "io.toggleConnection"
PREFERENCES = "app.preferences"
LANGUAGE = "settings.general.language"
SETTINGS_CLOSE = "settings.close"
CONSOLE_FIELD = "console.send"
CONSOLE_MODE = 1
NETWORK_BUS = 1
SIMULATOR_PORT = 9000
SETTLE_SECONDS = 0.5
WINDOW_TIMEOUT = 5.0
LINK_TIMEOUT = 15.0
CONSENT_WAIT = 25.0


def _png_size(png_base64: str) -> tuple[int, int]:
    """Width and height from the IHDR chunk of a base64 PNG."""
    data = base64.b64decode(png_base64)
    assert data[:8] == b"\x89PNG\r\n\x1a\n", "reply is not a PNG"
    width, height = struct.unpack(">II", data[16:24])
    return width, height


def _windows(client: SerialStudioClient) -> dict:
    return {w["id"]: w for w in client.command("ui.automation.listWindows")["windows"]}


def _elements(client: SerialStudioClient, window: str = MAIN, **filters) -> list[dict]:
    params = {"window": window}
    params.update(filters)
    return client.command("ui.automation.listElements", params)["elements"]


def _element(client: SerialStudioClient, element_id: str, window: str = MAIN) -> dict:
    matches = [
        e for e in _elements(client, window, id=element_id) if e["id"] == element_id
    ]
    assert matches, f"no visible element {element_id!r} in window {window!r}"
    return matches[0]


def _wait_until(predicate, timeout: float) -> bool:
    """Polls through the client, which keeps its socket drained while it waits."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        if predicate():
            return True
        time.sleep(0.1)
    return False


def _wait_for_window(client: SerialStudioClient, window: str, present: bool) -> bool:
    return _wait_until(lambda: (window in _windows(client)) == present, WINDOW_TIMEOUT)


def _wait_for_element(
    client: SerialStudioClient,
    element_id: str,
    window: str = MAIN,
    timeout: float = WINDOW_TIMEOUT,
) -> dict | None:
    """The element once it is catalogued, or None on timeout.

    A freshly opened window's page content enters the item tree a tick after the window
    itself appears, so a query issued the instant `listWindows` shows the window can miss a
    control that is about to be there. Polling through the client also keeps its socket drained.
    """
    found: dict = {}

    def seen() -> bool:
        matches = [
            e for e in _elements(client, window, id=element_id) if e["id"] == element_id
        ]
        if matches:
            found["element"] = matches[0]
        return bool(matches)

    return found.get("element") if _wait_until(seen, timeout) else None


def _drive(client: SerialStudioClient, command: str, params: dict) -> dict:
    """Issue a driving command, waiting out the one interface-driving consent prompt.

    A driving command returns CONSENT_REQUIRED the instant the prompt is posted, before the
    maintainer can answer it; re-issuing it never re-posts the prompt and never executes while
    refused, so it is safe to poll until the standing answer lands. A "no" surfaces as a
    different message and is raised at once; SERIAL_STUDIO_API_AUTO_CONSENT=1 answers on the
    first try.
    """
    deadline = time.time() + CONSENT_WAIT
    while True:
        try:
            return client.command(command, params)
        except APIError as error:
            if error.code != "CONSENT_REQUIRED" or time.time() >= deadline:
                raise
            time.sleep(0.5)


@pytest.fixture(scope="module")
def ui(serial_studio_running):
    """A module-scoped client; skips the module when the app shows no window (AC16)."""
    client = SerialStudioClient()
    client.connect()
    if not client.command("ui.automation.listWindows")["windows"]:
        client.disconnect()
        pytest.skip("no visible window: the app runs headless or offscreen (AC16)")

    previous = client.command("ui.automation.setReduceMotion", {"enabled": True})[
        "previous"
    ]
    yield client
    client.command("ui.automation.setReduceMotion", {"enabled": previous})
    client.disconnect()


@pytest.fixture
def console_mode(ui):
    """Console mode with the network source aimed at the simulator; mode and bus type return."""
    if ui.is_connected():
        pytest.skip("needs an idle instance: a link is open")

    previous_mode = ui.command("dashboard.getOperationMode")["mode"]
    previous_bus = ui.command("io.getStatus")["busType"]
    ui.command("dashboard.setOperationMode", {"mode": CONSOLE_MODE})
    ui.configure_network(host="127.0.0.1", port=SIMULATOR_PORT, socket_type="tcp")

    # A freshly launched instance that started on another bus (CAN hardware still enumerating
    # at launch) can report the switch applied while the previous driver is still settling, so
    # confirm the Network driver is actually selected before a test dials, re-issuing once if it
    # has not landed. Without this the first console-mode test dials whatever bus was active and
    # connects to the wrong peer.
    def _on_network() -> bool:
        return ui.command("io.getStatus").get("busType") == NETWORK_BUS

    if not _wait_until(_on_network, LINK_TIMEOUT):
        ui.command("io.setBusType", {"busType": NETWORK_BUS})
        assert _wait_until(
            _on_network, LINK_TIMEOUT
        ), "the Network bus did not take effect"

    yield
    if ui.is_connected():
        ui.disconnect_device()
        _wait_until(lambda: not ui.is_connected(), LINK_TIMEOUT)

    ui.command("io.setBusType", {"busType": previous_bus})
    ui.command("dashboard.setOperationMode", {"mode": previous_mode})


@pytest.fixture
def link(ui, console_mode, device_simulator):
    """A live TCP link to the simulator, which is what enables the console's send controls."""
    ui.connect_device()
    assert _wait_until(ui.is_connected, LINK_TIMEOUT), "the link did not come up"
    assert device_simulator.wait_for_connection(
        LINK_TIMEOUT
    ), "the simulator saw no client"
    yield


# --- AC1: window inventory --------------------------------------------------------------------


def test_main_window_is_listed(ui):
    windows = _windows(ui)
    assert MAIN in windows, f"main window missing from {sorted(windows)}"
    main = windows[MAIN]
    for key in (
        "title",
        "x",
        "y",
        "width",
        "height",
        "scale",
        "active",
        "modal",
        "focusedId",
    ):
        assert key in main, f"window entry lacks {key!r}"
    assert main["width"] > 0 and main["height"] > 0
    assert main["scale"] >= 1.0
    assert main["modal"] is False


def test_focus_follows_the_last_click(ui, link):
    field = _element(ui, CONSOLE_FIELD)
    _drive(ui, "ui.automation.click", {"window": MAIN, "id": CONSOLE_FIELD})
    time.sleep(SETTLE_SECONDS)
    assert _windows(ui)[MAIN]["focusedId"] == CONSOLE_FIELD
    assert field["role"] == "editabletext"


# --- AC2: identifiers by command id, stable across a language switch --------------------------


def test_toolbar_controls_carry_command_ids(ui):
    elements = _elements(ui)
    ids = {e["id"] for e in elements if e["id"]}
    assert CONNECT in ids, f"connect button not catalogued; ids: {sorted(ids)[:40]}"
    assert (
        PREFERENCES in ids
    ), "the Preferences toolbar button must carry its command id"
    assert any(i.startswith("console.") for i in ids)
    assert all("." in i for i in ids if i), "every identifier is a dotted literal"
    roles = {e["role"] for e in elements}
    assert "button" in roles and "editabletext" in roles


def test_ids_survive_a_language_switch_driven_through_preferences(ui):
    before = {e["id"] for e in _elements(ui) if e["id"]}

    _drive(ui, "ui.automation.click", {"window": MAIN, "id": PREFERENCES})
    assert _wait_for_window(ui, SETTINGS, True), "Preferences did not open"
    try:
        language = _wait_for_element(ui, LANGUAGE, SETTINGS)
        assert (
            language is not None
        ), "the General page's language selector was not catalogued"
        assert language["role"] == "combobox"
        _drive(
            ui,
            "ui.automation.key",
            {"window": SETTINGS, "target": LANGUAGE, "keys": "Down"},
        )
        time.sleep(SETTLE_SECONDS)
        switched = _element(ui, LANGUAGE, SETTINGS)
        assert switched.get("text") != language.get(
            "text"
        ), "the language selector did not move"

        during = {e["id"] for e in _elements(ui) if e["id"]}
        assert (
            during == before
        ), f"identifiers changed with the language: {sorted(before ^ during)}"
    finally:
        # Restore the language first, but never let a failed restore skip the close: a Settings
        # window left open lands modally over the main window and breaks every later test that
        # types or clicks there.
        try:
            _drive(
                ui,
                "ui.automation.key",
                {"window": SETTINGS, "target": LANGUAGE, "keys": "Up"},
            )
            time.sleep(SETTLE_SECONDS)
        except APIError:
            pass
        if SETTINGS in _windows(ui):
            _drive(
                ui, "ui.automation.click", {"window": SETTINGS, "id": SETTINGS_CLOSE}
            )
        assert _wait_for_window(ui, SETTINGS, False), "Preferences did not close"

    after = {e["id"] for e in _elements(ui) if e["id"]}
    assert after == before


# --- AC3: driving ------------------------------------------------------------------------------


def test_connect_by_clicking_the_toolbar_button(ui, console_mode, device_simulator):
    assert not ui.is_connected()

    reply = _drive(ui, "ui.automation.click", {"window": MAIN, "id": CONNECT})
    assert reply["hit"] is True and reply["id"] == CONNECT
    assert _wait_until(ui.is_connected, LINK_TIMEOUT), "the click did not connect"
    assert device_simulator.wait_for_connection(
        LINK_TIMEOUT
    ), "the simulator saw no client"

    _drive(ui, "ui.automation.click", {"window": MAIN, "id": CONNECT})
    assert _wait_until(
        lambda: not ui.is_connected(), LINK_TIMEOUT
    ), "the second click did not disconnect"


def test_typed_text_lands_in_the_console_field(ui, link):
    _drive(
        ui,
        "ui.automation.key",
        {"window": MAIN, "target": CONSOLE_FIELD, "text": "hello 42"},
    )
    assert _element(ui, CONSOLE_FIELD)["text"] == "hello 42"

    ui.command(
        "ui.automation.key", {"window": MAIN, "target": CONSOLE_FIELD, "keys": "Ctrl+A"}
    )
    ui.command("ui.automation.key", {"window": MAIN, "keys": "Backspace"})
    # The catalog omits `text` when a field is empty, so an emptied field has no key to read.
    assert _element(ui, CONSOLE_FIELD).get("text", "") == ""


def test_click_on_a_disabled_control_is_refused(ui):
    """The send field is disabled whenever no link is open, so it is the disabled target."""
    if ui.is_connected():
        pytest.skip("needs an idle instance: a link is open")

    control = _element(ui, CONSOLE_FIELD)
    assert control["enabled"] is False
    with pytest.raises(APIError) as refused:
        ui.command("ui.automation.click", {"window": MAIN, "id": CONSOLE_FIELD})
    assert refused.value.code == "EXECUTION_ERROR"
    assert "disabled" in refused.value.message


def test_click_outside_the_window_is_refused(ui):
    with pytest.raises(APIError) as refused:
        ui.command("ui.automation.click", {"window": MAIN, "x": -10, "y": -10})
    assert refused.value.code == "INVALID_PARAM"


# --- AC4: screenshot ---------------------------------------------------------------------------


def test_screenshot_decodes_and_matches_the_element_crop(ui):
    full = ui.command("ui.automation.screenshot", {"window": MAIN})
    width, height = _png_size(full["png"])
    assert (width, height) == (full["width"], full["height"])
    assert width > 0 and height > 0
    assert full["scale"] == 1.0

    scale = _windows(ui)[MAIN]["scale"]
    button = _element(ui, CONNECT)
    crop = ui.command("ui.automation.screenshot", {"window": MAIN, "id": CONNECT})
    crop_width, crop_height = _png_size(crop["png"])
    assert abs(crop_width - round(button["width"] * scale)) <= 1
    assert abs(crop_height - round(button["height"] * scale)) <= 1

    half = ui.command(
        "ui.automation.screenshot", {"window": MAIN, "id": CONNECT, "scale": 0.5}
    )
    half_width, _ = _png_size(half["png"])
    assert half["scale"] == 0.5
    assert abs(half_width - round(crop_width * 0.5)) <= 1
