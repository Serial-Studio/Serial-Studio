"""
Session Export (SQLite Recording) Integration Tests — Pro feature.

Covers the Sessions::Export worker:
 * Enable/disable the recording feature
 * End-to-end: connect loopback, stream frames, verify the .db file
   gets populated (sessions + readings rows)
 * ConsoleOnly refusal: setExportEnabled is a no-op while the app is in
   ConsoleOnly mode (dumb-terminal mode, no parsing, no DB recording)

All tests are skipped on non-commercial builds via the `pro_only` fixture.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import sqlite3
import time
from pathlib import Path

import pytest

from utils import APIError

# ---------------------------------------------------------------------------
# Pro gating fixture
# ---------------------------------------------------------------------------


@pytest.fixture
def pro_only(api_client):
    """Skip the test when the binary isn't a Pro build."""
    if not api_client.command_exists("sessions.getStatus"):
        pytest.skip("sessions.* API not available — GPL build")

    # Some builds register the command but the Sessions singleton may still be
    # non-functional. A simple getStatus call smoke-tests that path.
    try:
        api_client.command("sessions.getStatus")
    except APIError:
        pytest.skip("sessions.* API registered but returning errors")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

CONSOLE_ONLY_MODE = 1


def _enable_export(api_client, enabled: bool) -> None:
    api_client.command("sessions.setExportEnabled", {"enabled": enabled})
    time.sleep(0.1)


def _status(api_client) -> dict:
    return api_client.command("sessions.getStatus")


def _close_session(api_client) -> None:
    api_client.command("sessions.close")
    time.sleep(0.3)


def _db_path_for(api_client, title: str) -> Path:
    result = api_client.command("sessions.getDbPath", {"projectTitle": title})
    return Path(result["path"])


def _safe_unlink(path: Path) -> None:
    """
    Delete a session DB, tolerating a briefly-held handle.

    On Windows an open SQLite file cannot be removed (WinError 32), and the
    handle can linger past sessions.close (OS release lag, or a reader
    connection). This unlink is housekeeping, not an assertion -- the schema
    and row-count checks have already run -- so retry a few times and then give
    up quietly rather than failing the test on a cleanup artifact.
    """
    for attempt in range(10):
        try:
            path.unlink()
            return
        except FileNotFoundError:
            return
        except PermissionError:
            if attempt == 9:
                return
            time.sleep(0.3)


def _sqlite_tables(path: Path) -> set[str]:
    if not path.exists():
        return set()

    with sqlite3.connect(str(path)) as conn:
        return {
            r[0]
            for r in conn.execute("SELECT name FROM sqlite_master WHERE type='table'")
        }


def _row_count(path: Path, table: str) -> int:
    if not path.exists():
        return 0

    with sqlite3.connect(str(path)) as conn:
        try:
            return conn.execute(f"SELECT COUNT(*) FROM {table}").fetchone()[0]
        except sqlite3.OperationalError:
            return 0


# ---------------------------------------------------------------------------
# Basic flag toggling
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_export_flag_toggles(api_client, clean_state, pro_only):
    """setExportEnabled flips the flag and getStatus reflects it."""
    _enable_export(api_client, False)
    assert _status(api_client)["exportEnabled"] is False

    _enable_export(api_client, True)
    assert _status(api_client)["exportEnabled"] is True

    _enable_export(api_client, False)
    assert _status(api_client)["exportEnabled"] is False


@pytest.mark.project
def test_status_shape(api_client, clean_state, pro_only):
    """getStatus returns both exportEnabled and isOpen booleans."""
    status = _status(api_client)
    assert isinstance(status["exportEnabled"], bool)
    assert isinstance(status["isOpen"], bool)


# ---------------------------------------------------------------------------
# End-to-end: ProjectFile mode with real frames
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
def test_session_records_project_file_frames(
    api_client, device_simulator, clean_state, pro_only, tmp_path
):
    """
    In ProjectFile mode, enabling session export and streaming frames produces
    a populated `sessions` + `readings` table in the project's canonical .db.
    """
    # Build a minimal ProjectFile-mode project with one plot dataset
    title = f"SessionTest_{time.time_ns()}"
    api_client.create_new_project(title)
    time.sleep(0.2)

    api_client.command("project.group.add", {"title": "G", "widgetType": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.1)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        operation_mode=0,
    )
    api_client.configure_frame_parser(frame_detection=1, operation_mode=0)
    time.sleep(0.1)

    lua_code = (
        "function parse(frame)\n"
        "  local out = {}\n"
        "  for field in frame:gmatch('([^,]+)') do\n"
        "    out[#out + 1] = field\n"
        "  end\n"
        "  return out\n"
        "end\n"
    )
    api_client.command(
        "project.frameParser.setCode",
        {"code": lua_code, "language": 1, "sourceId": 0},
    )
    time.sleep(0.15)

    # Pre-compute the expected DB path and ensure we start clean
    db_path = _db_path_for(api_client, title)
    _safe_unlink(db_path)

    # Enable session recording, then sync project + connect
    _enable_export(api_client, True)
    api_client.command("project.activate")
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Stream a handful of frames
    frames = [f"{i * 10}\n".encode() for i in range(20)]
    device_simulator.send_frames(
        [b"/*" + f.rstrip(b"\n") + b"*/" for f in frames],
        interval_seconds=0.05,
    )
    # FrameConsumer's timer interval is 1000ms — give it room to drain
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)

    # Close the session so the final batch is flushed to disk
    _close_session(api_client)

    # Verify the DB exists with the expected schema
    assert db_path.exists(), f"Session DB was never created at {db_path}"

    tables = _sqlite_tables(db_path)
    assert "sessions" in tables
    assert "readings" in tables
    assert "columns" in tables

    # And that at least one session + reading + column row made it in
    assert _row_count(db_path, "sessions") >= 1
    assert (
        _row_count(db_path, "readings") >= 5
    ), "fewer readings than expected — timer may not have drained the queue"
    assert (
        _row_count(db_path, "columns") >= 1
    ), "columns table is empty — replay would fail with 'missing column definitions'"

    # Cleanup so we don't leak test artifacts
    _safe_unlink(db_path)
    _enable_export(api_client, False)


# ---------------------------------------------------------------------------
# End-to-end: QuickPlot mode (regression — Issue: empty `columns` rows)
# ---------------------------------------------------------------------------
#
# QuickPlot frames don't carry a project source list, so `buildExportSchema`
# used to look up `sourceTitles[0]` and get a null QString back. Qt binds a
# null QString as SQL NULL, which violates the NOT NULL constraint on
# `columns.source_title`, every INSERT silently failed (logged via qWarning
# only), and replay then aborted with "missing column definitions".
#
# The fixes:
#   * `FrameBuilder::buildQuickPlotFrame` / `buildQuickPlotAudioFrame` push a
#     synthetic Source row so every downstream exporter sees `frame.sources`.
#   * `buildExportSchema` defaults missing source titles to "" (empty,
#     non-null) instead of QString().
#
# These tests guard both fixes so the bug can't sneak back in.

QUICK_PLOT_TITLE = "Quick Plot"


@pytest.mark.project
@pytest.mark.slow
def test_quickplot_session_writes_column_rows(
    api_client, device_simulator, clean_state, pro_only
):
    """
    Regression: in QuickPlot mode the recorder must populate `columns`. If it
    doesn't, replay later fails with "missing column definitions" and the
    session is effectively unreadable.
    """
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    _safe_unlink(db_path)

    _enable_export(api_client, True)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    # Three CSV channels, line-terminated — the QuickPlot wire format
    frames = [f"{i},{i * 2},{i * 3}\n".encode() for i in range(20)]
    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)

    assert db_path.exists(), f"QuickPlot session DB was not created at {db_path}"

    # The bug we're guarding against: zero rows in `columns`.
    column_count = _row_count(db_path, "columns")
    assert column_count >= 3, (
        f"QuickPlot session recorded 0 (or too few) column definitions "
        f"({column_count}). The replay path will fail with "
        f"'missing column definitions'."
    )

    # And there should be actual data, otherwise the test is meaningless
    assert _row_count(db_path, "readings") >= 5

    # Source title is the NOT NULL column that originally broke the INSERT —
    # assert it's an empty string, not NULL, for every recorded column.
    with sqlite3.connect(str(db_path)) as conn:
        nulls = conn.execute(
            "SELECT COUNT(*) FROM columns WHERE source_title IS NULL"
        ).fetchone()[0]
    assert nulls == 0, "columns.source_title must never be NULL"

    _safe_unlink(db_path)
    _enable_export(api_client, False)


@pytest.mark.project
@pytest.mark.slow
def test_quickplot_session_loads_for_replay(
    api_client, device_simulator, clean_state, pro_only
):
    """
    The player loader reads the `columns` table to align dataset uids. With
    the original bug, opening a QuickPlot DB surfaced
    `columnUniqueIds.empty()` and the loader emitted the
    "missing column definitions" error. Verify the column unique-id list is
    present and matches the readings we recorded.
    """
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    _safe_unlink(db_path)

    _enable_export(api_client, True)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [f"{i},{i * 2}\n".encode() for i in range(15)]
    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)

    assert db_path.exists()

    with sqlite3.connect(str(db_path)) as conn:
        column_uids = {r[0] for r in conn.execute("SELECT unique_id FROM columns")}
        reading_uids = {
            r[0] for r in conn.execute("SELECT DISTINCT unique_id FROM readings")
        }

    assert column_uids, "columns table is empty — replay would abort"
    assert reading_uids, "readings table is empty — test setup is wrong"
    assert reading_uids.issubset(column_uids), (
        f"readings reference uids not in columns: {reading_uids - column_uids}. "
        f"The replay path won't be able to map them back to dataset metadata."
    )

    _safe_unlink(db_path)
    _enable_export(api_client, False)


@pytest.mark.project
@pytest.mark.slow
def test_quickplot_session_project_json_embeds_source(
    api_client, device_simulator, clean_state, pro_only
):
    """
    The synthetic project JSON written to `sessions.project_json` must include
    a `sources` array. The FrameBuilder fix populates `frame.sources` so the
    serializer doesn't have to fall back to inventing one — and other
    exporters (CSV, MDF4) reading the frame see a real source.
    """
    # Ensure no stale session/db state survives from prior tests sharing the path.
    _enable_export(api_client, False)
    _close_session(api_client)

    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    for suffix in ("", "-wal", "-shm"):
        _safe_unlink(db_path.with_name(db_path.name + suffix))

    _enable_export(api_client, True)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames(
        [b"1,2\n", b"3,4\n", b"5,6\n", b"7,8\n", b"9,10\n"],
        interval_seconds=0.05,
    )
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)
    _enable_export(api_client, False)

    assert db_path.exists()
    # Schema check first so we get a clear failure if the worker never finished init.
    tables = _sqlite_tables(db_path)
    assert "sessions" in tables, f"sessions table missing -- found: {sorted(tables)}"

    import json

    with sqlite3.connect(str(db_path)) as conn:
        row = conn.execute(
            "SELECT project_json FROM sessions ORDER BY session_id DESC LIMIT 1"
        ).fetchone()
    assert row, "no session row was written to the DB"
    project_json_text = row[0]

    project = json.loads(project_json_text)
    sources = project.get("sources", [])
    assert sources, "stored project JSON has no sources array"

    # Title must be present and non-empty — the NOT NULL bug's root cause
    titles = [s.get("title", "") for s in sources]
    assert all(titles), f"a stored source has an empty title: {titles}"

    _safe_unlink(db_path)


# ---------------------------------------------------------------------------
# Lifecycle
# ---------------------------------------------------------------------------
#
# NOTE: ConsoleOnly mode is a "dumb terminal" — no parsing, no dashboard, no
# session DB recording. setExportEnabled(true) is a no-op in ConsoleOnly and
# the worker never opens a database. Tests that asserted otherwise (raw-bytes
# capture, project_json clearing, session close in console mode) were
# removed when the old DeviceSendsJSON slot was repurposed in v3.3.


@pytest.mark.project
def test_close_is_idempotent(api_client, clean_state, pro_only):
    """sessions.close is safe to call when nothing is open."""
    _enable_export(api_client, False)
    _close_session(api_client)
    _close_session(api_client)

    status = _status(api_client)
    assert status["isOpen"] is False


@pytest.mark.project
def test_console_only_refuses_export(api_client, clean_state, pro_only):
    """
    ConsoleOnly is a dumb-terminal mode — session DB recording must stay off
    and any setExportEnabled(true) must be a no-op while in this mode.
    """
    api_client.command("dashboard.setOperationMode", {"mode": CONSOLE_ONLY_MODE})
    time.sleep(0.2)

    _enable_export(api_client, True)
    status = _status(api_client)
    assert status["exportEnabled"] is False
    assert status["isOpen"] is False
