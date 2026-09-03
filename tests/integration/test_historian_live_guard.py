"""
The session being recorded cannot be deleted from the explorer (spec 0075 R1.5, finding B4).

`deleteSession` had no guard against `Sessions::Export::currentSessionId()`, and the explorer
opens the same WAL database the historian is recording into. Deleting the live session dropped
the rows already written while the worker kept inserting orphan blocks into a session row that no
longer existed, and `finalizeSession` then updated nothing at all.

Both the explorer facade and the database worker now refuse that id, so this case is about the
observable outcome: after asking to delete the live session, the session is still there.

Requires the app up with Preferences -> API & Plugins -> Enable API Server, and a commercial
build (the historian is Pro).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

FRAME_COUNT = 20
FRAME_INTERVAL = 0.02
SETTLE_SECONDS = 2.0

_PARSER = """
function parse(frame) {
  return frame.split(',');
}
"""


def _project():
    return {
        "title": "SS0075 Live Guard",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 1,
                "frameStart": "",
                "frameEnd": "\n",
                "frameDetection": 1,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": _PARSER,
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "groups": [
            {
                "title": "A",
                "widget": "",
                "sourceId": 0,
                "datasets": [{"title": "A0", "value": "%1", "index": 1, "graph": True}],
            }
        ],
        "actions": [],
    }


def _session_ids(api_client):
    try:
        listing = api_client.command("sessions.list")
    except APIError:
        return None

    if isinstance(listing, dict):
        listing = listing.get("sessions", [])

    if not isinstance(listing, list):
        return None

    return [entry.get("session_id") for entry in listing if isinstance(entry, dict)]


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.pro
class TestHistorianLiveGuard:
    def test_deleting_the_live_session_is_refused(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_project())
        time.sleep(0.4)

        try:
            api_client.disconnect_device()
            time.sleep(0.4)
        except APIError:
            pass

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.set_operation_mode("project")
        time.sleep(0.2)
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        try:
            api_client.command("sessions.setExportEnabled", {"enabled": True})
        except APIError as error:
            if any(
                w in error.message.lower() for w in ("commercial", "license", "pro")
            ):
                pytest.skip("the historian needs a commercial build")
            raise

        time.sleep(0.3)
        for index in range(FRAME_COUNT):
            device_simulator.send_frame(f"{index + 1}\n".encode())
            time.sleep(FRAME_INTERVAL)

        time.sleep(SETTLE_SECONDS)

        status = api_client.command("sessions.getStatus")
        assert isinstance(status, dict)
        if not status.get("isOpen", False):
            pytest.skip("no live session was opened; nothing to guard")

        live_id = status.get("currentSessionId", -1)
        if live_id is None or live_id < 0:
            pytest.skip("the live session id is not exposed by this build")

        before = _session_ids(api_client)
        if before is None:
            pytest.skip(
                "the explorer has no database open; sessions.list is unavailable"
            )

        assert live_id in before, "the live session is not listed"

        try:
            api_client.command("sessions.delete", {"sessionId": live_id})
        except APIError:
            pass

        time.sleep(1.0)

        after = _session_ids(api_client)
        assert after is not None
        assert live_id in after, "the live session was deleted while it was recording"

        assert api_client.command("io.getStatus") is not None

        try:
            api_client.command("sessions.setExportEnabled", {"enabled": False})
        except APIError:
            pass

        api_client.disconnect_device()
