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
    result = api_client.command(
        "sessions.getCanonicalDbPath", {"projectTitle": title}
    )
    return Path(result["path"])


def _sqlite_tables(path: Path) -> set[str]:
    if not path.exists():
        return set()

    with sqlite3.connect(str(path)) as conn:
        return {
            r[0]
            for r in conn.execute(
                "SELECT name FROM sqlite_master WHERE type='table'"
            )
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
    api_client.command("project.dataset.add", {"options": 1})
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
        "project.parser.setCode",
        {"code": lua_code, "language": 1, "sourceId": 0},
    )
    time.sleep(0.15)

    # Pre-compute the expected DB path and ensure we start clean
    db_path = _db_path_for(api_client, title)
    if db_path.exists():
        db_path.unlink()

    # Enable session recording, then sync project + connect
    _enable_export(api_client, True)
    api_client.command("project.loadIntoFrameBuilder")
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

    # And that at least one session + reading rows made it in
    assert _row_count(db_path, "sessions") >= 1
    assert _row_count(db_path, "readings") >= 5, (
        "fewer readings than expected — timer may not have drained the queue"
    )

    # Cleanup so we don't leak test artifacts
    if db_path.exists():
        db_path.unlink()
    _enable_export(api_client, False)


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
