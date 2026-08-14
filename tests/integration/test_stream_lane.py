"""
Typed Stream Lane Integration Tests (spec 0051, M4)

Covers the pieces of the stream lane that are testable without audio
hardware, plus an environment-gated live check that reuses the audio
loopback rig from test_audio_loopback.py:

 * streamLane persistence -- the per-source "streamLane" override survives a
   project load/serialize round trip through the API ("on"/"off" persist,
   "auto" stays absent).
 * QuickPlot audio liveness (audio-gated) -- with a loopback capture device,
   QuickPlot over the Audio bus renders live dashboard values through the
   typed stream path (no CSV text lane).

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import os
import time

import pytest

STREAM_PROJECT = {
    "title": "Stream lane persistence",
    "frameStart": "/*",
    "frameEnd": "*/",
    "frameDetection": 1,
    "decoder": 0,
    "sources": [
        {
            "sourceId": 0,
            "title": "Audio A",
            "busType": 3,
            "frameStart": "",
            "frameEnd": "\n",
            "frameDetection": 1,
            "checksumAlgorithm": "",
            "decoderMethod": 0,
            "streamLane": "off",
            "connectionSettings": {},
        },
        {
            "sourceId": 1,
            "title": "Audio B",
            "busType": 3,
            "frameStart": "",
            "frameEnd": "\n",
            "frameDetection": 1,
            "checksumAlgorithm": "",
            "decoderMethod": 0,
            "streamLane": "on",
            "connectionSettings": {},
        },
    ],
    "groups": [
        {
            "title": "Levels",
            "widget": "",
            "sourceId": 0,
            "datasets": [{"title": "CH1", "value": "", "index": 1, "graph": True}],
        }
    ],
    "actions": [],
}


def _sources_from_project(data: dict) -> list:
    project = data.get("config", data.get("project", data))
    if isinstance(project, str):
        import json as _json

        project = _json.loads(project)

    return project.get("sources", [])


@pytest.mark.project
class TestStreamLanePersistence:
    def test_stream_lane_round_trips(self, api_client, clean_state):
        """streamLane "on"/"off" survive load -> serialize; auto stays absent."""
        api_client.load_project_from_json(copy.deepcopy(STREAM_PROJECT))
        time.sleep(0.3)

        result = api_client.command("project.exportJson", {})
        sources = _sources_from_project(result)
        assert len(sources) >= 2, f"expected 2 sources, got {len(sources)}"

        by_id = {src.get("sourceId"): src for src in sources}
        assert by_id[0].get("streamLane") == "off"
        assert by_id[1].get("streamLane") == "on"

    def test_absent_lane_stays_absent(self, api_client, clean_state):
        """A source without the key loads as auto and never gains the key."""
        project = copy.deepcopy(STREAM_PROJECT)
        for src in project["sources"]:
            src.pop("streamLane", None)

        api_client.load_project_from_json(project)
        time.sleep(0.3)

        result = api_client.command("project.exportJson", {})
        for src in _sources_from_project(result):
            assert src.get("streamLane", "") in ("", None) or "streamLane" not in src


@pytest.mark.audio
class TestQuickPlotStreamLane:
    def test_quickplot_audio_stream_is_live(self, api_client, clean_state):
        """AC13: QuickPlot audio renders live values through the typed lane."""
        if not os.environ.get("SS_AUDIO_CAPTURE_MATCH"):
            pytest.skip("no loopback capture device configured")

        api_client.command("app.setOperationMode", {"mode": 2})
        api_client.command("io.setBusType", {"busType": 3})
        time.sleep(0.3)

        api_client.connect_device()
        time.sleep(2.0)

        values = []
        for _ in range(20):
            data = api_client.get_dashboard_data()
            groups = data.get("frame", {}).get("groups", [])
            if groups and groups[0].get("datasets"):
                values.append(groups[0]["datasets"][0].get("numericValue", 0.0))

            time.sleep(0.25)

        api_client.disconnect_device()

        assert (
            len(set(values)) >= 3
        ), "QuickPlot audio dashboard never advanced through the stream lane"
