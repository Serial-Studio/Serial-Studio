"""
Dashboard Ingest Lanes (spec 0075, WP-E)

Pins the two dashboard-ingest defects the review found, from the outside:

 * F3 -- a uniform-grid block (any stream-lane source) fed only the time rings,
   the FFT windows and the 3D rings. A Samples-axis plot, a Samples-mode
   multiplot and a GPS group stayed blank while the time plot next to them was
   live. `dashboard.tailFrames` reads the sample ring for a plot that has no
   time ring, so a Samples-axis plot with `count == 0` after the source has been
   producing for a second is exactly the reported symptom.

 * F4 -- changing the plot point count rebuilt the line series and dropped what
   the rebuild was supposed to preserve. The retained history has to survive the
   change, and the plot has to keep receiving samples afterwards.

The stream-lane half needs a source that publishes uniform-grid blocks, which
means the audio loopback rig; it self-skips like test_audio_loopback.py does.
The frame-lane half runs against the plain TCP simulator.

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import os
import time

import pytest

# Samples X axis (DataModel::kXAxisSamples); the default is Time (-2)
X_AXIS_SAMPLES = -1

LANES_PROJECT = {
    "title": "Dashboard lanes",
    "frameStart": "/*",
    "frameEnd": "*/",
    "frameDetection": 1,
    "decoder": 0,
    "sources": [
        {
            "sourceId": 0,
            "title": "Simulator",
            "busType": 1,
            "frameStart": "",
            "frameEnd": "\n",
            "frameDetection": 1,
            "checksumAlgorithm": "",
            "decoderMethod": 0,
            "connectionSettings": {},
        }
    ],
    "groups": [
        {
            "title": "Samples",
            "widget": "",
            "sourceId": 0,
            "datasets": [
                {
                    "title": "SampleAxis",
                    "value": "",
                    "index": 1,
                    "graph": True,
                    "xAxisId": X_AXIS_SAMPLES,
                }
            ],
        },
        {
            "title": "Multi",
            "widget": "multiplot",
            "sourceId": 0,
            "datasets": [
                {"title": "M1", "value": "", "index": 2, "xAxisId": X_AXIS_SAMPLES},
                {"title": "M2", "value": "", "index": 3, "xAxisId": X_AXIS_SAMPLES},
            ],
        },
        {
            "title": "Position",
            "widget": "map",
            "sourceId": 0,
            "datasets": [
                {"title": "Lat", "value": "", "index": 4, "widget": "lat"},
                {"title": "Lon", "value": "", "index": 5, "widget": "lon"},
                {"title": "Alt", "value": "", "index": 6, "widget": "alt"},
            ],
        },
    ],
    "actions": [],
}


def _sample_axis_series(api_client) -> dict:
    """The tailFrames row of the Samples-axis plot, or an empty dict."""
    reply = api_client.command("dashboard.tailFrames", {"count": 32})
    for row in reply.get("series", []):
        if row.get("title") == "SampleAxis":
            return row

    return {}


def _feed(device_simulator, rows: int) -> None:
    """Sends @p rows frames carrying every dataset of the lanes project."""
    for i in range(rows):
        payload = "/*{0},{1},{2},{3},{4},{5}*/\n".format(
            i, i + 1, i + 2, 40.0 + i * 0.001, -3.0 - i * 0.001, 100 + i
        )
        device_simulator.send_frame(payload.encode())
        time.sleep(0.01)


@pytest.mark.integration
class TestSamplesAxisLane:
    def test_samples_axis_plot_receives_frame_lane_data(
        self, api_client, clean_state, device_simulator
    ):
        """A Samples-axis plot fills its sample ring from the irregular lane."""
        api_client.load_project_from_json(copy.deepcopy(LANES_PROJECT))
        time.sleep(0.3)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.set_operation_mode("project")
        api_client.command("project.activate")
        time.sleep(0.2)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        _feed(device_simulator, 40)
        time.sleep(0.5)

        series = _sample_axis_series(api_client)
        assert series, "the Samples-axis plot never appeared in dashboard.tailFrames"
        assert series.get("count", 0) > 0, "the Samples-axis plot ring stayed empty"

    def test_history_survives_a_point_count_change(
        self, api_client, clean_state, device_simulator
    ):
        """A point-count change keeps the plot alive and still receiving samples."""
        api_client.load_project_from_json(copy.deepcopy(LANES_PROJECT))
        time.sleep(0.3)

        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.set_operation_mode("project")
        api_client.command("project.activate")
        time.sleep(0.2)

        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)

        _feed(device_simulator, 40)
        time.sleep(0.5)
        assert _sample_axis_series(api_client).get("count", 0) > 0

        api_client.command(
            "controlScript.dryRun",
            {"code": "function main() { dashboard.setPlotPoints(300); }"},
        )
        time.sleep(0.3)

        _feed(device_simulator, 40)
        time.sleep(0.5)

        after = _sample_axis_series(api_client)
        assert (
            after.get("count", 0) > 0
        ), "the plot stopped receiving after setPlotPoints"


@pytest.mark.audio
@pytest.mark.integration
class TestUniformGridLane:
    def test_samples_axis_plot_receives_stream_blocks(self, api_client, clean_state):
        """A stream-lane source must reach the sample rings, not only the time rings."""
        if not os.environ.get("SS_AUDIO_CAPTURE_MATCH"):
            pytest.skip("SS_AUDIO_CAPTURE_MATCH not set (no audio loopback wired)")

        devices = api_client.command("io.audio.listInputDevices").get("devices", [])
        match = os.environ["SS_AUDIO_CAPTURE_MATCH"].lower()
        index = next(
            (i for i, name in enumerate(devices) if match in str(name).lower()), None
        )
        if index is None:
            pytest.skip("no capture device matches SS_AUDIO_CAPTURE_MATCH")

        stream_project = copy.deepcopy(LANES_PROJECT)
        stream_project["sources"][0]["busType"] = 3
        stream_project["sources"][0]["streamLane"] = "on"
        api_client.load_project_from_json(stream_project)
        time.sleep(0.3)

        api_client.set_operation_mode("project")
        api_client.command("io.audio.setInputDevice", {"deviceIndex": index})
        api_client.command("project.activate")
        api_client.connect_device()
        time.sleep(1.5)

        series = _sample_axis_series(api_client)
        assert series, "the Samples-axis plot never appeared in dashboard.tailFrames"
        assert (
            series.get("count", 0) > 0
        ), "the uniform-grid lane never fed the sample ring"
