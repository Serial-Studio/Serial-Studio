"""
Spec 0101 AC5: the interface-driving consent behind `ui.automation.click` and `ui.automation.key`.

Two launch modes, following test_project_load_consent.py:

  (default)
      A GUI instance started WITHOUT SERIAL_STUDIO_API_AUTO_CONSENT. The first click posts the
      "Allow API clients to drive the user interface?" prompt and is refused with
      CONSENT_REQUIRED; the maintainer sees that prompt once. Answering "no" turns later
      refusals into "denied by the user"; both shapes pass. Granting the device-write consent
      does not unlock these commands (tst_server_auth pins that without a prompt).

  SS_TEST_AUTO_CONSENT=1
      An instance started WITH SERIAL_STUDIO_API_AUTO_CONSENT=1: the gate is open and the
      handler runs.

Marker `gui`: a headless or offscreen instance lists no window and the module skips. The
target in both modes is the console's send field, disabled whenever no link is open, so a
click that reaches the handler is refused for being disabled and nothing in the interface
changes; the module skips while a link is open. The default mode runs before the interface
consent has been answered in that instance: once it is granted, the two refusal tests skip.
"""

import os

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError

AUTO_CONSENT = os.environ.get("SS_TEST_AUTO_CONSENT") == "1"
DISABLED_CONTROL = "console.send"
MAIN = "main"

pytestmark = [pytest.mark.security, pytest.mark.gui]

needs_no_consent = pytest.mark.skipif(
    AUTO_CONSENT, reason="runs against an instance started without AUTO_CONSENT"
)
needs_auto_consent = pytest.mark.skipif(
    not AUTO_CONSENT, reason="needs SS_TEST_AUTO_CONSENT=1 and an AUTO_CONSENT instance"
)


@pytest.fixture(scope="module")
def ui():
    client = SerialStudioClient()
    client.connect()
    if not client.command("ui.automation.listWindows")["windows"]:
        client.disconnect()
        pytest.skip("no visible window: the app runs headless or offscreen (AC16)")

    if client.is_connected():
        client.disconnect()
        pytest.skip("needs an idle instance: a link is open")

    yield client
    client.disconnect()


def test_read_only_commands_need_no_consent(ui):
    windows = ui.command("ui.automation.listWindows")["windows"]
    assert any(w["id"] == MAIN for w in windows)
    elements = ui.command(
        "ui.automation.listElements", {"window": MAIN, "id": DISABLED_CONTROL}
    )
    assert any(e["id"] == DISABLED_CONTROL for e in elements["elements"])


@needs_no_consent
def test_click_is_refused_until_the_interface_consent_is_granted(ui):
    with pytest.raises(APIError) as refused:
        ui.command("ui.automation.click", {"window": MAIN, "id": DISABLED_CONTROL})

    if refused.value.code == "CONSENT_REQUIRED":
        assert "consent" in refused.value.message
    elif "disabled" in refused.value.message:
        pytest.skip("the interface consent was already granted in this instance")
    else:
        assert refused.value.code == "EXECUTION_ERROR"
        assert "denied by the user" in refused.value.message


@needs_no_consent
def test_key_is_refused_until_the_interface_consent_is_granted(ui):
    try:
        ui.command("ui.automation.key", {"window": MAIN, "keys": "Escape"})
    except APIError as refused:
        assert refused.code in ("CONSENT_REQUIRED", "EXECUTION_ERROR")
        assert "disabled" not in refused.message
        return

    pytest.skip("the interface consent was already granted in this instance")


@needs_auto_consent
def test_click_reaches_the_handler_with_auto_consent(ui):
    with pytest.raises(APIError) as refused:
        ui.command("ui.automation.click", {"window": MAIN, "id": DISABLED_CONTROL})

    assert refused.value.code == "EXECUTION_ERROR"
    assert "disabled" in refused.value.message
