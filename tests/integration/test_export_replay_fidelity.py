"""
Export/replay fidelity — the sink-masked republish lane must not starve the exports (spec 0064).

A dataset whose value comes from a data table rather than from parsed channels has exactly one
publication path: the synthetic refresh. `dashboardTick()` runs it with the sinks fed;
`refreshStreamDrivenFrames()` and `dashboard.reprocess` run it with the sinks MASKED. Both lanes
shared one "already republished" mark, so a masked refresh consumed the change-driven transform
clock and the export pass then found nothing changed and skipped — the dashboard updated while
every recording stayed empty. A 635-dataset project recorded four channels that way, and the
four were the only ones not fed from tables.

The project here reproduces that shape without hardware: the parser writes a table register and
returns NO datasets, so the parse lane never stages a row and the republish lane is the only
publisher.

`dashboard.reprocess` stands in for the stream lane's masked refresh, but note what it cannot do:
it marshals asynchronously from the API thread, so it cannot force the interleaving the real
stream lane produces by running ON the pipeline thread between parse batches. These cases are
end-to-end COVERAGE, not the lock. The invariant itself is pinned deterministically by the
`tst_republish_lanes` ctest suite; do not treat a pass here as proof the lane rule holds.

Also locks the CSV timestamp contract: elapsed starts at or after zero and never decreases, and a
Serial Studio CSV is replayable by Serial Studio without a time-column prompt.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import csv
import os
import time
from pathlib import Path

import pytest

# The parser deliberately returns NO datasets: everything the dashboard shows comes from the
# table via the virtual dataset's transform, which is the BADAQ shape and the only shape in
# which the republish lane is the sole publisher.
_PARSER = """
function parse(frame) {
  tableSet("T", "raw", parseFloat(frame));
  return [];
}
"""

_TRANSFORM = """
function transform(value) {
  return (tableGet("T", "raw") || 0) * 2;
}
"""

SETTLE_SECONDS = 2.5


def _project() -> dict:
    return {
        "title": "SS0064 Fidelity",
        "changeDrivenTransforms": True,
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 0,
                "frameStart": "$",
                "frameEnd": "\n",
                "frameDetection": 0,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": _PARSER,
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "tables": [
            {
                "name": "T",
                "registers": [{"name": "raw", "type": "computed", "value": 0}],
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "Table Fed",
                        "value": "%1",
                        "index": 1,
                        "virtual": True,
                        "transformCode": _TRANSFORM,
                        "transformLanguage": 0,
                    }
                ],
            }
        ],
        "actions": [],
    }


def _workspace_roots(subdir):
    roots = []
    env = os.environ.get("SS_WORKSPACE")
    if env:
        roots.append(Path(env) / subdir)

    roots.append(Path.home() / "Documents" / "Serial Studio" / subdir)
    roots.append(Path("/tmp") / "Serial Studio" / subdir)
    return roots


def _newest_csv_since(start_time):
    newest, newest_mtime = None, start_time - 1.0
    for root in _workspace_roots("CSV"):
        if not root.is_dir():
            continue

        for path in root.rglob("*.csv"):
            mtime = path.stat().st_mtime
            if mtime >= start_time and mtime > newest_mtime:
                newest, newest_mtime = path, mtime

    return newest


def _read_csv(path):
    with open(path, encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.reader(handle))

    return rows[0], rows[1:]


def _attach(api_client, device_simulator):
    try:
        api_client.disconnect_device()
        time.sleep(0.6)
    except Exception:
        pass

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except Exception:
        pass

    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _record(api_client, device_simulator, masked_refreshes):
    """
    Records a short capture, interleaving masked refreshes with the export refresh.

    Returns (header, rows) of the CSV the app wrote. The masked refresh is what a live stream
    source performs on every UI tick; the export refresh is what a control script performs.
    """
    api_client.load_project_from_json(_project())
    time.sleep(0.4)
    _attach(api_client, device_simulator)

    started = time.time()
    api_client.enable_csv_export()
    time.sleep(0.3)

    for index in range(30):
        device_simulator.send_frame(f"{index + 1}\n".encode())
        for _ in range(masked_refreshes):
            api_client.command("dashboard.reprocess")

        api_client.command("dashboard.tick")
        time.sleep(0.05)

    time.sleep(SETTLE_SECONDS)
    api_client.disable_csv_export()
    time.sleep(1.0)
    api_client.disconnect_device()

    path = _newest_csv_since(started)
    if path is None:
        return None, None

    return _read_csv(path)


def _column_is_populated(header, rows, title):
    for index, name in enumerate(header):
        if name.endswith(title):
            return any(row[index].strip() for row in rows if len(row) > index)

    pytest.fail(f"column {title!r} missing from header {header!r}")


@pytest.mark.integration
@pytest.mark.project
class TestRepublishLaneFidelity:
    def test_table_fed_dataset_reaches_the_csv(
        self, api_client, clean_state, device_simulator
    ):
        """Baseline: with no masked refresh competing, the export lane records the dataset."""
        header, rows = _record(api_client, device_simulator, masked_refreshes=0)
        assert header is not None, "no CSV was written at all"
        assert rows, "export produced no rows"
        assert _column_is_populated(header, rows, "Table Fed")

    def test_export_lane_survives_interleaved_masked_refreshes(
        self, api_client, clean_state, device_simulator
    ):
        """
        Coverage, not a lock: masked refreshes interleaved with the export refresh must not stop
        the recording. Cannot force the pipeline-thread ordering the stream lane creates, so a
        pass here does not prove the lane rule -- tst_republish_lanes does that.
        """
        header, rows = _record(api_client, device_simulator, masked_refreshes=3)
        assert header is not None, (
            "masked refreshes starved the export lane: the export pass never published, so the "
            "recording was never even created -- see spec 0064"
        )
        assert rows, "export produced no rows"
        assert _column_is_populated(
            header, rows, "Table Fed"
        ), "masked refresh starved the export lane -- spec 0064 regression"


@pytest.mark.integration
@pytest.mark.project
class TestCsvTimestampContract:
    def test_elapsed_column_is_non_negative_and_monotonic(
        self, api_client, clean_state, device_simulator
    ):
        """
        The exporter measures elapsed from the earliest sample in the recording. Taking the
        first block of a batch instead put a multi-source recording's origin after its own
        earliest sample, and the player then refused to read its own file.
        """
        header, rows = _record(api_client, device_simulator, masked_refreshes=0)
        assert header is not None, "no CSV was written"
        assert header[0].strip().lower().startswith("elapsed")

        elapsed = [float(row[0]) for row in rows if row and row[0].strip()]
        assert elapsed, "no elapsed values written"
        assert elapsed[0] >= 0.0, f"first elapsed value is negative: {elapsed[0]}"
        assert elapsed == sorted(elapsed), "elapsed column is not monotonic"

    def test_serial_studio_csv_replays_without_a_prompt(
        self, api_client, clean_state, device_simulator
    ):
        """
        Opening a Serial Studio CSV must never ask the user to describe its own time column. The
        player blocks on a modal when detection fails, so a failure here hangs rather than
        asserts -- the timeout is the signal.
        """
        started = time.time()
        _record(api_client, device_simulator, masked_refreshes=0)
        path = _newest_csv_since(started)
        assert path is not None

        api_client.command("csvPlayer.open", {"filePath": str(path)})
        time.sleep(1.5)
        status = api_client.command("csvPlayer.getStatus")
        assert status.get("isOpen"), "player refused a CSV this app wrote"
        api_client.command("csvPlayer.close")


@pytest.mark.integration
@pytest.mark.project
class TestReplayNeverReRecords:
    def test_replaying_a_csv_creates_no_new_recording(
        self, api_client, clean_state, device_simulator
    ):
        """
        R8. Replay reaches the dashboard and the read-only observers, never a recording sink.
        The sink mask rides on the block rather than on a FrameBuilder member, because replay
        batches rows and the display tick may flush a pending block outside the call that staged
        it -- an unmarked block would be re-recorded into a brand new file.
        """
        header, _ = _record(api_client, device_simulator, masked_refreshes=0)
        assert header is not None

        started = time.time()
        source = _newest_csv_since(started - 3600)
        assert source is not None

        api_client.enable_csv_export()
        api_client.command("csvPlayer.open", {"filePath": str(source)})
        api_client.command("csvPlayer.setPaused", {"paused": False})
        time.sleep(4.0)
        api_client.command("csvPlayer.close")
        api_client.disable_csv_export()
        time.sleep(1.0)

        assert (
            _newest_csv_since(started) is None
        ), "replay re-recorded itself into a new CSV"
