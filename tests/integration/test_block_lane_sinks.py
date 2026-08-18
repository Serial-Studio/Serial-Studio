"""
Unified block lane -- sink coverage and stream visibility (spec 0055).

Covers the two things the lane unification is supposed to guarantee and that
nothing else checks:

 * T25 / AC12 -- every sink, enabled ALONE, still produces a non-empty
   artifact. FrameBuilder gates the whole fan-out on one cached flag
   (`m_anyAsyncSink`); a sink missing from that flag's inputs stays enabled in
   the UI while receiving nothing, so recording yields a valid-looking, empty
   file. That failure shipped once already for the stream export sinks, which
   is why it gets a test per sink rather than one test with everything on.

 * T35 / AC2 -- a project whose ONLY source is dense (stream lane) reaches the
   consumers that were blind to it before spec 0055: MQTT published nothing and
   gRPC exposed nothing, because both implemented only the frame path. Each
   assertion here fails on a pre-0055 build.

The audio-backed cases need a virtual capture device; they skip without one,
exactly like test_audio_loopback.py.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import os
import time
from pathlib import Path

import pytest

# Frames the simulator pushes per capture, and the gap between them. The device
# simulator is passive -- it only emits what a test hands it -- so a sink test
# that merely sleeps would assert against an empty artifact and pass for the
# wrong reason.
FRAME_COUNT = 60
FRAME_INTERVAL = 0.02

# Settle time for the display-tick flush to close a partial block (spec 0055 D1)
# and for the async sink workers to reach their files.
SETTLE_SECONDS = 2.0

# Project title QuickPlot records under; names the session database directory.
QUICKPLOT_TITLE = "Quick Plot"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _workspace_roots(subdir):
    """Candidate roots where Serial Studio writes exports of one kind."""
    roots = []
    env = os.environ.get("SS_WORKSPACE")
    if env:
        roots.append(Path(env) / subdir)

    roots.append(Path.home() / "Documents" / "Serial Studio" / subdir)
    roots.append(Path("/tmp") / "Serial Studio" / subdir)
    return roots


def _newest_since(subdir, suffix, start_time):
    """Newest export of `suffix` under `subdir` modified at/after start_time."""
    newest = None
    newest_mtime = start_time - 1.0
    for root in _workspace_roots(subdir):
        if not root.is_dir():
            continue

        for path in root.rglob(f"*{suffix}"):
            mtime = path.stat().st_mtime
            if mtime >= start_time and mtime > newest_mtime:
                newest = path
                newest_mtime = mtime

    return newest


def _dashboard_groups(payload):
    """
    Groups out of a dashboard.getData reply.

    The datasets live under `frame.groups`; `datasetCount` is a sibling summary
    field. Reading the wrong level yields an empty list on a working build.
    """
    frame = payload.get("frame") or {}
    return frame.get("groups") or []


def _capture(api_client, device_simulator, while_live=None, frames=FRAME_COUNT):
    """
    Connect, push frames, run `while_live`, disconnect. Returns the pre-connect time.

    QuickPlot is the operation mode used throughout: it derives datasets from a
    plain CSV stream, so no project fixture is needed for the sinks to see a
    populated frame.

    `while_live` runs before the disconnect because the live surfaces are
    transient -- the dashboard drops its groups and the file sinks close their
    files on disconnect, so an assertion made afterwards reads an empty state on
    a perfectly working build.
    """
    started = time.time()

    # Mode first: switching it rebuilds device 0 (ConnectionManager::rebuildDevices), which
    # retires the live driver and drops the link out from under an established connection.
    api_client.set_operation_mode("quickplot")
    time.sleep(0.3)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert api_client.wait_for_connection(timeout=10.0), "device never connected"
    assert device_simulator.wait_for_connection(
        timeout=10.0
    ), "app never opened the simulator socket"

    device_simulator.send_frames([b"1,2,3\n"] * frames, interval_seconds=FRAME_INTERVAL)
    time.sleep(SETTLE_SECONDS)

    if while_live is not None:
        while_live()

    api_client.disconnect_device()
    time.sleep(0.5)
    return started


@pytest.fixture
def pro_only(api_client):
    """Skip when the Pro sinks are absent (GPL build)."""
    if not api_client.command_exists("sessions.getStatus"):
        pytest.skip("Pro sinks unavailable -- GPL build")


@pytest.fixture
def audio_source(api_client):
    """Select a dense (stream-lane) audio input, or skip."""
    match = os.environ.get("SS_AUDIO_CAPTURE_MATCH", "loopback").lower()
    result = api_client.command("io.audio.listInputDevices")
    for index, name in enumerate(result.get("devices", [])):
        if match in name.lower():
            api_client.command("io.audio.selectInputDevice", {"index": index})
            return {"index": index, "name": name}

    pytest.skip("no virtual capture device in io.audio.listInputDevices")


# ---------------------------------------------------------------------------
# T25 / AC12 -- each sink alone must still produce data
# ---------------------------------------------------------------------------


def test_csv_sink_alone_writes_rows(api_client, clean_state, device_simulator):
    """CSV enabled on its own produces a file with at least one data row."""
    api_client.enable_csv_export()

    started = _capture(api_client, device_simulator)
    api_client.command("csvExport.close")
    api_client.disable_csv_export()
    time.sleep(0.5)

    path = _newest_since("CSV", ".csv", started)
    assert path is not None, "CSV export produced no file"

    lines = [
        line for line in path.read_text(errors="replace").splitlines() if line.strip()
    ]
    assert len(lines) >= 2, f"CSV export wrote only a header: {path}"


@pytest.mark.slow
def test_session_sink_alone_records_samples(
    api_client, clean_state, pro_only, device_simulator
):
    """Session recording enabled on its own stores samples, not an empty archive."""
    import sqlite3

    api_client.command("sessions.setExportEnabled", {"enabled": True})

    opened = {}

    def _check_live():
        opened.update(api_client.command("sessions.getStatus"))

    _capture(api_client, device_simulator, while_live=_check_live)
    api_client.command("sessions.setExportEnabled", {"enabled": False})
    time.sleep(1.0)

    assert opened.get(
        "isOpen"
    ), "session recording stayed closed while a stream was running"

    archive = api_client.command(
        "sessions.getDbPath", {"projectTitle": QUICKPLOT_TITLE}
    ).get("path")
    assert archive, "session recording reported no archive path"

    con = sqlite3.connect(f"file:{archive}?mode=ro", uri=True)
    try:
        tables = {
            row[0]
            for row in con.execute("SELECT name FROM sqlite_master WHERE type='table'")
        }
        assert (
            "blocks" in tables
        ), "a current build must record into the unified blocks table"

        samples = con.execute("SELECT COALESCE(SUM(frames), 0) FROM blocks").fetchone()[
            0
        ]
        assert samples > 0, "session archive contains no samples"
    finally:
        con.close()


def test_api_sink_alone_streams_frames(api_client, clean_state, device_simulator):
    """An API subscriber alone still receives frames; the sink is gated per client."""
    seen = {}

    def _read_live():
        seen["payload"] = api_client.command("dashboard.getData")

    _capture(api_client, device_simulator, while_live=_read_live)

    groups = _dashboard_groups(seen.get("payload", {}))
    assert groups, "no dashboard data reached the API sink"

    values = [
        dataset.get("value")
        for group in groups
        for dataset in group.get("datasets", [])
    ]
    assert any(v not in (None, "") for v in values), "API sink served empty datasets"


# ---------------------------------------------------------------------------
# T35 / AC2 -- a dense-only project reaches the previously blind consumers
# ---------------------------------------------------------------------------


@pytest.mark.slow
def test_dense_source_reaches_mqtt(
    api_client, clean_state, pro_only, audio_source, mqtt_subscriber
):
    """
    An audio-only project publishes over MQTT.

    Pre-0055 this asserted nothing arrived: MQTT::Publisher implemented only
    the frame path, so a stream-lane source produced no payloads at all.
    """
    api_client.command("mqtt.setMode", {"mode": 0})
    api_client.command(
        "mqtt.configure",
        {"hostname": "127.0.0.1", "port": 1883, "topic": "ss/blocklane"},
    )
    mqtt_subscriber.subscribe("ss/blocklane/#")
    api_client.command("mqtt.setEnabled", {"enabled": True})

    api_client.connect_device()
    time.sleep(3.0)
    api_client.disconnect_device()

    api_client.command("mqtt.setEnabled", {"enabled": False})
    assert mqtt_subscriber.messages, "a dense-only project published nothing over MQTT"


@pytest.mark.slow
def test_dense_source_reaches_session_recording(
    api_client, clean_state, pro_only, audio_source
):
    """An audio-only project records samples through the unified blocks table."""
    import sqlite3

    api_client.command("sessions.setExportEnabled", {"enabled": True})
    api_client.connect_device()
    time.sleep(3.0)
    api_client.disconnect_device()
    api_client.command("sessions.setExportEnabled", {"enabled": False})
    time.sleep(1.0)

    status = api_client.command("sessions.getStatus")
    archive = status.get("filePath") or status.get("path") or status.get("dbPath")
    if not archive:
        archive = api_client.command("sessions.getDbPath").get("path")

    assert archive, "session recording reported no archive path"

    con = sqlite3.connect(f"file:{archive}?mode=ro", uri=True)
    try:
        samples = con.execute("SELECT COALESCE(SUM(frames), 0) FROM blocks").fetchone()[
            0
        ]
        uniform = con.execute(
            "SELECT COUNT(*) FROM blocks WHERE dt_ns != 0"
        ).fetchone()[0]
    finally:
        con.close()

    assert samples > 0, "a dense-only project recorded no samples"
    assert uniform > 0, "a dense source must record on a uniform grid (dt_ns != 0)"


@pytest.mark.slow
def test_dense_source_writes_single_csv(api_client, clean_state, audio_source):
    """
    An audio-only recording produces ONE csv, not a per-source stream file.

    Pre-0055 the dense lane wrote `*_stream_source0.csv` beside the frame
    lane's file (spec 0051 M5); R6 folds them into one.
    """
    api_client.enable_csv_export()
    started = time.time()
    api_client.connect_device()
    time.sleep(3.0)
    api_client.disconnect_device()
    api_client.command("csvExport.close")
    api_client.disable_csv_export()

    written = []
    for root in _workspace_roots("CSV"):
        if not root.is_dir():
            continue

        written.extend(p for p in root.rglob("*.csv") if p.stat().st_mtime >= started)

    assert written, "a dense-only project exported no CSV"
    assert not any("_stream_source" in p.name for p in written), (
        "the per-source stream CSV must not be produced any more: "
        f"{[p.name for p in written]}"
    )
