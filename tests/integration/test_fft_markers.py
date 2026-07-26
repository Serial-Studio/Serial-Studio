"""
FFT Frequency Marker Tests (spec 0019)

Exercises the fftMarkers dataset feature end-to-end against a running Serial
Studio (API server on localhost:7777):
  - project.dataset.setFFTMarkers / getFFTMarkers atomic round-trip
  - project.dataset.update with the fftMarkers key
  - persistence through project.open / project.save
  - validation: invalid entries dropped (droppedInvalid), reversed bands
    demoted to point markers, reversed warn/alarm thresholds swapped
  - projects without markers serialize without the fftMarkers key

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest


def _project():
    """A minimal project: one group with one FFT-enabled dataset (fs = 2000 Hz)."""
    return {
        "title": "FFT marker test",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "groups": [
            {
                "title": "Vibration",
                "widget": "",
                "datasets": [
                    {
                        "title": "Accelerometer",
                        "index": 1,
                        "fft": True,
                        "fftSamples": 1024,
                        "fftSamplingRate": 2000,
                    }
                ],
            }
        ],
        "actions": [],
    }


_MARKERS = [
    {
        "freq": 800,
        "label": "Gear mesh",
        "color": "#ff5722",
        "warningDb": -40,
        "alarmDb": -25,
    },
    {"freq": 120, "endFreq": 180, "label": "Bearing band", "color": "#03a9f4"},
    {"freq": 50, "label": "Mains"},
]


def _ids(api_client):
    """Return (groupId, datasetId) of the first dataset in the loaded project."""
    groups = api_client.command("project.group.list", {})
    group = groups["groups"][0]
    datasets = api_client.command("project.dataset.list", {"groupId": group["groupId"]})
    return group["groupId"], datasets["datasets"][0]["datasetId"]


def _set(api_client, group_id, dataset_id, markers):
    return api_client.command(
        "project.dataset.setFFTMarkers",
        {"groupId": group_id, "datasetId": dataset_id, "fftMarkers": markers},
    )


def _get(api_client, group_id, dataset_id):
    return api_client.command(
        "project.dataset.getFFTMarkers", {"groupId": group_id, "datasetId": dataset_id}
    )


@pytest.mark.integration
@pytest.mark.project
class TestFFTMarkersAPI:
    def test_set_get_round_trip(self, api_client, clean_state):
        """AC2: setFFTMarkers stores the list verbatim and getFFTMarkers echoes it."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        result = _set(api_client, gid, did, _MARKERS)
        assert result["count"] == 3
        assert "droppedInvalid" not in result

        got = _get(api_client, gid, did)
        assert got["count"] == 3
        assert got["nyquist"] == 1000.0

        by_freq = {m["freq"]: m for m in got["fftMarkers"]}
        assert by_freq[800]["label"] == "Gear mesh"
        assert by_freq[800]["color"] == "#ff5722"
        assert by_freq[800]["warningDb"] == -40
        assert by_freq[800]["alarmDb"] == -25
        assert "endFreq" not in by_freq[800]

        assert by_freq[120]["endFreq"] == 180
        assert "warningDb" not in by_freq[120]
        assert "alarmDb" not in by_freq[120]

        assert "warningDb" not in by_freq[50]

    def test_empty_array_clears(self, api_client, clean_state):
        """AC2: an empty array removes every marker."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        _set(api_client, gid, did, _MARKERS)
        result = _set(api_client, gid, did, [])
        assert result["count"] == 0
        assert _get(api_client, gid, did)["count"] == 0

    def test_invalid_entries_dropped(self, api_client, clean_state):
        """AC2: junk entries are dropped and counted; the stored list stays valid."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        result = _set(
            api_client,
            gid,
            did,
            [
                {"freq": -5, "label": "negative"},
                {"freq": 0},
                {"label": "no freq at all"},
                {"freq": 1e300, "label": "absurd"},
                {"freq": 440, "label": "valid"},
            ],
        )
        assert result["count"] == 1
        assert result["droppedInvalid"] == 4

        got = _get(api_client, gid, did)
        assert got["count"] == 1
        assert got["fftMarkers"][0]["freq"] == 440

    def test_reversed_band_becomes_point(self, api_client, clean_state):
        """R1: endFreq <= freq demotes the entry to a point marker."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        _set(api_client, gid, did, [{"freq": 500, "endFreq": 200, "label": "reversed"}])
        got = _get(api_client, gid, did)
        assert got["count"] == 1
        assert "endFreq" not in got["fftMarkers"][0]

    def test_reversed_thresholds_swapped(self, api_client, clean_state):
        """R6: warningDb > alarmDb is normalized by swapping the two."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        _set(api_client, gid, did, [{"freq": 300, "warningDb": -10, "alarmDb": -50}])
        got = _get(api_client, gid, did)["fftMarkers"][0]
        assert got["warningDb"] == -50
        assert got["alarmDb"] == -10

    def test_dataset_update_key(self, api_client, clean_state):
        """AC2: the generic project.dataset.update path accepts fftMarkers."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)
        gid, did = _ids(api_client)

        api_client.command(
            "project.dataset.update",
            {
                "groupId": gid,
                "datasetId": did,
                "fftMarkers": [{"freq": 800, "label": "X"}],
            },
        )
        got = _get(api_client, gid, did)
        assert got["count"] == 1
        assert got["fftMarkers"][0]["label"] == "X"


@pytest.mark.integration
@pytest.mark.project
class TestFFTMarkersPersistence:
    def test_markers_survive_save(self, api_client, clean_state, tmp_path):
        """AC1: markers written through the API persist in the saved project file."""
        proj_path = tmp_path / "fft_markers.ssproj"
        proj_path.write_text(json.dumps(_project(), indent=2), encoding="utf-8")

        api_client.command("project.open", {"filePath": str(proj_path)})
        time.sleep(0.3)
        gid, did = _ids(api_client)
        _set(api_client, gid, did, _MARKERS)

        api_client.command("project.save")
        time.sleep(0.2)

        data = json.loads(proj_path.read_text(encoding="utf-8"))
        stored = data["groups"][0]["datasets"][0]["fftMarkers"]
        assert len(stored) == 3
        by_freq = {m["freq"]: m for m in stored}
        assert by_freq[800]["alarmDb"] == -25
        assert by_freq[120]["endFreq"] == 180
        assert "warningDb" not in by_freq[50]

    def test_markers_survive_reload(self, api_client, clean_state, tmp_path):
        """AC1: a saved project reloads with an identical marker list."""
        proj_path = tmp_path / "fft_markers_reload.ssproj"
        proj_path.write_text(json.dumps(_project(), indent=2), encoding="utf-8")

        api_client.command("project.open", {"filePath": str(proj_path)})
        time.sleep(0.3)
        gid, did = _ids(api_client)
        _set(api_client, gid, did, _MARKERS)
        api_client.command("project.save")
        time.sleep(0.2)

        api_client.command("project.open", {"filePath": str(proj_path)})
        time.sleep(0.3)
        gid, did = _ids(api_client)
        got = _get(api_client, gid, did)
        assert got["count"] == 3

    def test_no_markers_means_no_key(self, api_client, clean_state, tmp_path):
        """R1: a dataset without markers serializes without the fftMarkers key."""
        proj_path = tmp_path / "fft_no_markers.ssproj"
        proj_path.write_text(json.dumps(_project(), indent=2), encoding="utf-8")

        api_client.command("project.open", {"filePath": str(proj_path)})
        time.sleep(0.3)
        api_client.command("project.save")
        time.sleep(0.2)

        data = json.loads(proj_path.read_text(encoding="utf-8"))
        assert "fftMarkers" not in data["groups"][0]["datasets"][0]
