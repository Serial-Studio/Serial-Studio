"""
Replay timeline tests (spec 0020): tape-style scrubbing and lossless catch-up.

Exercises the CSV player against a running Serial Studio (API server on
localhost:7777): backward scrubbing matches a direct seek (AC1), playback
holds the recording's pace without dropping rows (AC3), and replay never
feeds the recording sinks (AC5). MDF4 shares the same code path but has no
file-authoring helper here; the CSV coverage exercises the shared lanes.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

ROW_COUNT = 200
ROW_HZ = 50.0
SETTLE_SECONDS = 0.6


def _fixture_project():
    """Three plotted datasets so tailFrames exposes the ring contents."""
    return {
        "title": "Replay timeline fixture",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "groups": [
            {
                "title": "Signals",
                "widget": "",
                "datasets": [
                    {"title": "A", "index": 1, "graph": True},
                    {"title": "B", "index": 2, "graph": True},
                    {"title": "C", "index": 3, "graph": True},
                ],
            },
        ],
    }


def _write_recording(path):
    """Constant-rate CSV: numeric timestamps at ROW_HZ, deterministic values."""
    lines = ["Time,A,B,C"]
    for row in range(ROW_COUNT):
        ts = row / ROW_HZ
        lines.append(f"{ts:.6f},{row},{2 * row},{3 * row}")
    path.write_text("\n".join(lines) + "\n")


def _open_recording(api_client, tmp_path):
    api_client.load_project_from_json(_fixture_project())
    time.sleep(0.5)

    recording = tmp_path / "recording.csv"
    _write_recording(recording)
    api_client.command("csvPlayer.open", {"filePath": str(recording)})
    time.sleep(0.5)

    status = api_client.command("csvPlayer.getStatus")
    assert status["isOpen"] is True
    assert status["frameCount"] == ROW_COUNT
    return recording


def _tail_values(api_client, title="A", count=256):
    result = api_client.command("dashboard.tailFrames", {"count": count})
    for entry in result.get("series", []):
        if entry.get("title") == title:
            return list(entry.get("y", []))
    return []


@pytest.mark.integration
@pytest.mark.csv
class TestReplayTimeline:
    def test_backward_scrub_matches_direct_seek(
        self, api_client, clean_state, tmp_path
    ):
        """AC1: forward-then-backward scrubbing ends with the same plot window
        as a fresh direct seek to the same position."""
        _open_recording(api_client, tmp_path)

        api_client.command("csvPlayer.setProgress", {"progress": 0.8})
        time.sleep(SETTLE_SECONDS)
        api_client.command("csvPlayer.setProgress", {"progress": 0.4})
        time.sleep(SETTLE_SECONDS)
        scrubbed = _tail_values(api_client)
        position = api_client.command("csvPlayer.getStatus")["framePosition"]

        api_client.command("csvPlayer.close")
        time.sleep(0.3)
        _open_recording(api_client, tmp_path)
        api_client.command("csvPlayer.setProgress", {"progress": 0.4})
        time.sleep(SETTLE_SECONDS)
        direct = _tail_values(api_client)
        direct_position = api_client.command("csvPlayer.getStatus")["framePosition"]

        assert position == direct_position
        assert len(scrubbed) > 0
        assert scrubbed == direct

        api_client.command("csvPlayer.close")

    def test_scrub_then_play_resumes(self, api_client, clean_state, tmp_path):
        """AC1 sanity: playback after a backward scrub advances cleanly."""
        _open_recording(api_client, tmp_path)

        api_client.command("csvPlayer.setProgress", {"progress": 0.9})
        time.sleep(SETTLE_SECONDS)
        api_client.command("csvPlayer.setProgress", {"progress": 0.25})
        time.sleep(SETTLE_SECONDS)
        start = api_client.command("csvPlayer.getStatus")["framePosition"]

        api_client.command("csvPlayer.setPaused", {"paused": False})
        time.sleep(1.0)
        status = api_client.command("csvPlayer.getStatus")
        assert status["framePosition"] > start

        api_client.command("csvPlayer.close")

    def test_playback_pace_and_losslessness(self, api_client, clean_state, tmp_path):
        """AC3: playback tracks the recording clock and delivers every row."""
        _open_recording(api_client, tmp_path)

        api_client.command("csvPlayer.setProgress", {"progress": 0.0})
        time.sleep(SETTLE_SECONDS)
        api_client.command("csvPlayer.setPaused", {"paused": False})

        time.sleep(2.0)
        mid = api_client.command("csvPlayer.getStatus")["framePosition"]
        expected_mid = int(2.0 * ROW_HZ)
        assert (
            abs(mid - expected_mid) <= ROW_HZ // 2
        ), f"pace drift: at 2 s playback sits at row {mid}, expected ~{expected_mid}"

        time.sleep(ROW_COUNT / ROW_HZ)
        status = api_client.command("csvPlayer.getStatus")
        assert status["isPlaying"] is False
        assert status["framePosition"] == ROW_COUNT - 1

        values = _tail_values(api_client)
        expected = [float(row) for row in range(ROW_COUNT)]
        assert (
            values[-len(expected) :] == expected[-len(values) :] or values == expected
        )

        api_client.command("csvPlayer.close")

    def test_replay_never_re_records(self, api_client, clean_state, tmp_path):
        """AC5: with CSV export enabled, neither playback nor scrubbing opens
        an export file -- replay must never feed a recording sink."""
        _open_recording(api_client, tmp_path)
        api_client.command("csvExport.setEnabled", {"enabled": True})

        api_client.command("csvPlayer.setPaused", {"paused": False})
        time.sleep(1.0)
        api_client.command("csvPlayer.setPaused", {"paused": True})
        api_client.command("csvPlayer.setProgress", {"progress": 0.2})
        time.sleep(SETTLE_SECONDS)

        status = api_client.command("csvExport.getStatus")
        assert status.get("enabled") is True
        assert status.get("isOpen") is False, "replay opened a CSV export file"

        api_client.command("csvExport.setEnabled", {"enabled": False})
        api_client.command("csvPlayer.close")
