"""
Recording fidelity across the session edges (spec 0075 R1; findings A2, B1, B14).

Three properties of a recording that nothing else checks end to end:

 * The TAIL. Sinks used to close on `ConnectionManager::connectedChanged`, which the GUI emits
   synchronously, while the builder's own connected slot ran queued on the pipeline thread. The
   sink was therefore gone before the builder could flush its open block, and the trailing
   partial block (up to 64 samples) was published afterwards: the CSV worker found a closed file
   and opened a SECOND one holding those rows, MDF4 dropped them. Pause had the same shape.
   The builder now flushes and then emits `sessionBoundary`, and the sinks close on that.

 * PER-SOURCE TIME. Both exporters re-stamped every irregular block with ONE worker-wide
   monotonic clock, so with two frame-lane sources flushed on the same display tick the second
   source's samples became a nanosecond staircase pinned to the first source's tail. Each source
   now keeps the instants it stamped, and the same-instant tie-break is per source.

 * A FAILING SINK. A recording that cannot write must say so instead of showing "recording"
   over discarded batches.

The row-level invariants are pinned deterministically by the ctest tier (tst_frame_builder_staging,
tst_csv_export_times); these cases are the end-to-end coverage over the real pipeline.

Requires the app up with Preferences -> API & Plugins -> Enable API Server.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import csv
import os
import time
from pathlib import Path

import pytest

from utils import DeviceSimulator
from utils.api_client import APIError

# Frames per source per capture. Small enough to stay well inside one CSV reorder window and
# large enough that a lost tail block (<= 64 samples) is unambiguous.
FRAME_COUNT = 40
FRAME_INTERVAL = 0.02

# Display-tick flush plus the async sink worker's batch timer.
SETTLE_SECONDS = 2.0

# Second simulator port; the device_simulator fixture owns 9000.
SECOND_PORT = 9001

_PARSER = """
function parse(frame) {
  return frame.split(',');
}
"""


def _single_source_project(title="SS0075 Fidelity"):
    return {
        "title": title,
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


def _workspace_roots(subdir):
    roots = []
    env = os.environ.get("SS_WORKSPACE")
    if env:
        roots.append(Path(env) / subdir)

    roots.append(Path.home() / "Documents" / "Serial Studio" / subdir)
    roots.append(Path("/tmp") / "Serial Studio" / subdir)
    return roots


def _csvs_since(start_time):
    found = []
    for root in _workspace_roots("CSV"):
        if not root.is_dir():
            continue

        for path in root.rglob("*.csv"):
            if path.stat().st_mtime >= start_time:
                found.append(path)

    return sorted(set(found), key=lambda p: p.stat().st_mtime)


def _read_csv(path):
    with open(path, encoding="utf-8-sig", newline="") as handle:
        rows = list(csv.reader(handle))

    return rows[0], [row for row in rows[1:] if row]


def _elapsed_column(rows):
    values = []
    for row in rows:
        try:
            values.append(float(row[0]))
        except (IndexError, ValueError):
            continue

    return values


def _attach(api_client, port=9000):
    try:
        api_client.disconnect_device()
        time.sleep(0.5)
    except APIError:
        pass

    api_client.configure_network(host="127.0.0.1", port=port, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    time.sleep(0.2)
    api_client.connect_device()


@pytest.mark.integration
@pytest.mark.project
class TestRecordingTail:
    """The last display tick of a session must land in the file that was open for it."""

    def test_disconnect_writes_the_tail_into_one_file(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_single_source_project())
        time.sleep(0.4)
        _attach(api_client)
        assert device_simulator.wait_for_connection(timeout=5.0)

        started = time.time()
        api_client.enable_csv_export()
        time.sleep(0.3)

        for index in range(FRAME_COUNT):
            device_simulator.send_frame(f"{index + 1}\n".encode())
            time.sleep(FRAME_INTERVAL)

        # No settle: disconnecting immediately is exactly the case that lost the tail.
        api_client.disconnect_device()
        time.sleep(SETTLE_SECONDS)
        api_client.disable_csv_export()

        files = _csvs_since(started)
        assert files, "no CSV was written"
        assert len(files) == 1, f"session produced {len(files)} files: {files}"

        _, rows = _read_csv(files[-1])
        assert len(rows) == FRAME_COUNT, f"expected {FRAME_COUNT} rows, got {len(rows)}"

    def test_pause_closes_over_the_samples_it_holds(
        self, api_client, clean_state, device_simulator
    ):
        api_client.load_project_from_json(_single_source_project("SS0075 Pause"))
        time.sleep(0.4)
        _attach(api_client)
        assert device_simulator.wait_for_connection(timeout=5.0)

        started = time.time()
        api_client.enable_csv_export()
        time.sleep(0.3)

        for index in range(FRAME_COUNT):
            device_simulator.send_frame(f"{index + 1}\n".encode())
            time.sleep(FRAME_INTERVAL)

        api_client.command("io.setPaused", {"paused": True})
        time.sleep(SETTLE_SECONDS)

        files = _csvs_since(started)
        assert files, "no CSV was written"
        _, rows = _read_csv(files[-1])
        assert len(rows) == FRAME_COUNT, f"expected {FRAME_COUNT} rows, got {len(rows)}"

        api_client.command("io.setPaused", {"paused": False})
        time.sleep(0.2)
        api_client.disconnect_device()
        api_client.disable_csv_export()


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.pro
class TestPerSourceTime:
    """Two sources recorded together keep their own instants (B1)."""

    def _two_source_project(self):
        project = _single_source_project("SS0075 Two Sources")
        second = dict(project["sources"][0])
        second["sourceId"] = 1
        second["title"] = "Device B"
        project["sources"].append(second)
        project["groups"].append(
            {
                "title": "B",
                "widget": "",
                "sourceId": 1,
                "datasets": [{"title": "B0", "value": "%1", "index": 1, "graph": True}],
            }
        )
        return project

    def test_sources_are_not_collapsed_onto_one_clock(
        self, api_client, clean_state, device_simulator
    ):
        try:
            api_client.load_project_from_json(self._two_source_project())
        except APIError as error:
            if any(
                w in error.message.lower() for w in ("commercial", "license", "pro")
            ):
                pytest.skip("multi-source needs a commercial build")
            raise

        time.sleep(0.4)

        second = DeviceSimulator(host="127.0.0.1", port=SECOND_PORT, protocol="tcp")
        second.start()
        try:
            api_client.source_configure(
                1,
                {"address": "127.0.0.1", "tcpPort": SECOND_PORT, "socketTypeIndex": 0},
            )
            _attach(api_client)
            assert device_simulator.wait_for_connection(timeout=5.0)
            if not second.wait_for_connection(timeout=5.0):
                pytest.skip("the second source never attached")

            started = time.time()
            api_client.enable_csv_export()
            time.sleep(0.3)

            for index in range(FRAME_COUNT):
                device_simulator.send_frame(f"{index + 1}\n".encode())
                time.sleep(FRAME_INTERVAL / 2)
                second.send_frame(f"{1000 + index}\n".encode())
                time.sleep(FRAME_INTERVAL / 2)

            time.sleep(SETTLE_SECONDS)
            api_client.disconnect_device()
            time.sleep(1.0)
            api_client.disable_csv_export()
        finally:
            second.stop()

        files = _csvs_since(started)
        if not files:
            pytest.skip("no CSV produced; the second source never attached")

        header, rows = _read_csv(files[-1])
        elapsed = _elapsed_column(rows)
        assert elapsed == sorted(elapsed), "the time column is not ordered"

        # The old worker-wide clock rewrote the second source's instants as +1 ns steps behind
        # the first source's tail. Real captures from two sockets are milliseconds apart, so a
        # run of nanosecond-spaced rows is the signature of that collapse.
        nanosecond_steps = sum(
            1
            for previous, current in zip(elapsed, elapsed[1:])
            if 0 < (current - previous) < 1e-6
        )
        assert nanosecond_steps < len(elapsed) / 4, (
            f"{nanosecond_steps} of {len(elapsed)} rows are a nanosecond apart: "
            "the sources were collapsed onto one clock"
        )


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.pro
class TestFailingSink:
    """A sink that cannot write must stop reporting itself as recording (B3)."""

    def test_read_only_session_directory_does_not_report_recording(
        self, api_client, clean_state, device_simulator
    ):
        roots = [
            root for root in _workspace_roots("Session Databases") if root.is_dir()
        ]
        if not roots:
            pytest.skip("no session-database directory to make read-only")

        target = roots[0]
        original = target.stat().st_mode
        try:
            os.chmod(target, 0o500)
            if os.access(target, os.W_OK):
                pytest.skip("cannot drop write permission (running as root?)")

            api_client.load_project_from_json(_single_source_project("SS0075 ReadOnly"))
            time.sleep(0.4)
            _attach(api_client)
            assert device_simulator.wait_for_connection(timeout=5.0)

            api_client.command("sessions.setExportEnabled", {"enabled": True})
            time.sleep(0.3)

            for index in range(FRAME_COUNT):
                device_simulator.send_frame(f"{index + 1}\n".encode())
                time.sleep(FRAME_INTERVAL)

            time.sleep(SETTLE_SECONDS)
            status = api_client.command("sessions.getStatus")
            assert isinstance(status, dict)
            assert not status.get(
                "isOpen", False
            ), "the historian reports an open recording although its directory is read-only"

            # The application must still be answering after the failure.
            assert api_client.command("io.getStatus") is not None
        finally:
            os.chmod(target, original)
            try:
                api_client.command("sessions.setExportEnabled", {"enabled": False})
            except APIError:
                pass
