"""
Message box API (ui.window.showMessageBox).

The runtime cases exercise only the refusals, so the suite never leaves a modal dialog on a desk:
an empty headline, an unknown icon, button options without wait, and wait from a caller that is
not the control script. The static case pins the invariant behind wait: only the control-script
marshaller grants it, so a frame parser or transform on the data path can never block on a
dialog.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import re
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


def test_static_only_the_control_script_grants_wait():
    """A wait permission opened anywhere else could park the pipeline behind a dialog."""
    grants = []
    pattern = re.compile(r"UserWaitScope\s+\w+\s*\(\s*true\s*\)")
    for path in (REPO / "core").rglob("*.cpp"):
        if pattern.search(path.read_text(encoding="utf-8", errors="replace")):
            grants.append(path.relative_to(REPO).as_posix())

    assert grants == ["core/Pipeline/DataModel/Scripting/ControlScriptWorker.cpp"]
