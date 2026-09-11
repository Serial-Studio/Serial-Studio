"""
Transmit Script Editor Integration Tests (spec 0079)

Covers the parts of the transmit editor that are reachable without a human at
the keyboard:

  * AC7 - the editor's validity gate must NOT leak into the API. A syntax-error
    transmit script still writes successfully through project.outputWidget.update,
    because the gate is a UI guard and automation stays responsible for its own
    dry run.
  * AC3 (structural half) - a preview must be unable to transmit. This is checked
    at the source level rather than at runtime: the output widget classes must
    contain no connection manager at all, so a preview built from them has no
    device to reach. A source scan is the durable form of that guarantee - it
    keeps holding for widget types nobody has written yet.

The runtime halves of AC3/AC4/AC5/AC6 drive a modal editor window and are
maintainer observations; the API cannot open that window or move its controls.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import re
from pathlib import Path

import pytest

# ---------------------------------------------------------------------------
# Structural guarantee (no running app required)
# ---------------------------------------------------------------------------

_REPO_ROOT = Path(__file__).resolve().parents[2]
_OUTPUT_DIR = _REPO_ROOT / "core" / "Ui" / "UI" / "Widgets" / "Output"

# Panel is the one place allowed to resolve a destination; everything else must
# hold a TransmitTarget and nothing else (spec 0079, plan T7)
_DESTINATION_OWNER = "Panel.cpp"

_FORBIDDEN = re.compile(r"ConnectionManager|handlerContext")


def test_widget_classes_hold_no_connection():
    """
    AC3, structural half: no output widget class may reference a connection.

    If this fails, a preview built from those classes could reach a device, and
    previewing a relay or PWM script against live hardware would actuate it.
    """
    assert _OUTPUT_DIR.is_dir(), f"missing {_OUTPUT_DIR}"

    offenders = []
    for path in sorted(_OUTPUT_DIR.rglob("*.[ch]pp")) + sorted(
        _OUTPUT_DIR.rglob("*.h")
    ):
        if path.name == _DESTINATION_OWNER:
            continue

        text = path.read_text(encoding="utf-8", errors="replace")
        for number, line in enumerate(text.splitlines(), start=1):
            if _FORBIDDEN.search(line):
                offenders.append(f"{path.name}:{number}: {line.strip()}")

    assert not offenders, (
        "output widget code outside "
        f"{_DESTINATION_OWNER} reaches a connection:\n  " + "\n  ".join(offenders)
    )


def test_preview_uses_a_zero_interval_target():
    """
    R16: the preview reports every interaction, so its target must not pace.
    """
    preview = (_OUTPUT_DIR / "Preview.cpp").read_text(encoding="utf-8")
    assert "minIntervalMs = 0" in preview


# ---------------------------------------------------------------------------
# API surface (running app required)
# ---------------------------------------------------------------------------


def _first_output_widget(api_client):
    """Return (groupId, widgetId) of any output widget in the loaded project."""
    status = api_client.command("project.get", {})
    for group in status.get("groups", []):
        for widget in group.get("outputWidgets", []):
            return group.get("groupId", 0), widget.get("widgetId", 0)

    pytest.skip("loaded project has no output widget to exercise")


@pytest.mark.integration
def test_api_still_accepts_an_invalid_transmit_script(api_client):
    """
    AC7: the editor's gate must not leak into project.outputWidget.update.

    An automation that builds a script in two steps would start failing on the
    first if the gate had leaked, so this asserts the permissive behaviour is
    still intact - and that the script really was stored.
    """
    group_id, widget_id = _first_output_widget(api_client)
    broken = "function transmit(value) {\n  return [0x01,\n}"

    api_client.command(
        "project.outputWidget.update",
        {"groupId": group_id, "widgetId": widget_id, "transmitFunction": broken},
    )

    stored = api_client.command(
        "project.outputWidget.get", {"groupId": group_id, "widgetId": widget_id}
    )
    assert stored.get("transmitFunction") == broken


@pytest.mark.integration
def test_dry_run_reports_the_shared_verdict(api_client):
    """
    T3: the dry run's response shape is frozen, and its verdict now comes from
    the same checker the editor reads. A syntax error keeps ok/compileError/line;
    a wrong entry point stays a distinct outcome rather than a compile error.
    """
    syntax_error = api_client.command(
        "project.outputWidget.dryRun", {"code": "function transmit(v) { return [1,"}
    )
    assert syntax_error["ok"] is False
    assert syntax_error["compileError"]
    assert syntax_error["line"] > 0

    wrong_name = api_client.command(
        "project.outputWidget.dryRun", {"code": "function send(v) { return [v]; }"}
    )
    assert wrong_name["ok"] is False
    assert "transmit(value)" in wrong_name["compileError"]

    good = api_client.command(
        "project.outputWidget.dryRun",
        {"code": "function transmit(v) { return [1, v & 0xFF]; }", "inputValue": 7},
    )
    assert good["ok"] is True
    assert good["hasTransmit"] is True
    assert good["sampleRun"]["ok"] is True
    assert good["sampleRun"]["byteCount"] == 2
