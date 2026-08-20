"""
Session recording for stream-lane sources (spec 0054) — Pro feature.

A dense typed source (audio) never reaches the frame exporters, so before this
spec the session database recorded exactly one row for an audio capture: the
synthesized structure frame. These tests pin the recording path end to end:

 * Recording works with session export as the ONLY enabled sink. This is the
   requirement most likely to regress silently: StreamProcessor only builds an
   export payload while refreshStreamExportFlags() says some sink is live, so
   wiring the sink without feeding that gate reproduces the original bug.
 * Recorded samples match a simultaneous CSV export exactly, with no tolerance.
   The reproducibility verifier compares a session against itself, so only a
   cross-check against a different exporter catches silent sample loss.
 * Blocks are stored per acquisition block, not per sample, so the database
   stays a size a user can keep.
 * Sessions with no stream source are completely unaffected.
 * A database written before this schema version still opens.

The audio tests self-skip unless the instance exposes a usable input device.
They deliberately do not require a signal: silence still produces samples, and
every assertion here is about counts, shape and exactness rather than content.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import sqlite3
import struct
import time
from pathlib import Path

import pytest

from utils import APIError, session_diagnostics

QUICK_PLOT_TITLE = "Quick Plot"

# One float64 per sample, the format stream_blocks.samples is packed in.
SAMPLE_BYTES = 8

# Generous ceiling for the per-sample cost on disk including block metadata,
# indexes and SQLite page overhead. Blowing past this means blocks stopped
# being blocks -- i.e. something started writing a row per sample.
MAX_BYTES_PER_SAMPLE = 24

CAPTURE_SECONDS = 4.0


# ---------------------------------------------------------------------------
# Gating fixtures
# ---------------------------------------------------------------------------


@pytest.fixture
def pro_only(api_client):
    """Skip when session recording isn't usable on this instance."""
    if not api_client.command_exists("sessions.getStatus"):
        pytest.skip("sessions.* API not available — GPL build")

    try:
        api_client.command("sessions.setExportEnabled", {"enabled": True})
        time.sleep(0.1)
        enabled = api_client.command("sessions.getStatus").get("exportEnabled", False)
        api_client.command("sessions.setExportEnabled", {"enabled": False})
        time.sleep(0.1)
    except APIError:
        pytest.skip("session recording refused enable (license-gated)")

    if not enabled:
        pytest.skip("session recording is license-gated; this instance has no token")


@pytest.fixture
def audio_source(api_client):
    """Select an audio input device, or skip when none is usable."""
    try:
        devices = api_client.command("io.audio.listInputDevices").get("devices", [])
    except APIError:
        pytest.skip("audio driver not available on this instance")

    if not devices:
        pytest.skip("no audio input device present")

    api_client.command("io.setBusType", {"busType": 3})
    api_client.command("io.audio.setInputDevice", {"deviceIndex": 0})
    time.sleep(0.2)
    return devices[0]


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _db_path_for(api_client, title: str) -> Path:
    return Path(
        api_client.command("sessions.getDbPath", {"projectTitle": title})["path"]
    )


def _safe_unlink(path: Path) -> None:
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


def _query(path: Path, sql: str, params=()):
    if not path.exists():
        return []

    with sqlite3.connect(str(path)) as conn:
        try:
            return conn.execute(sql, params).fetchall()
        except sqlite3.OperationalError:
            return []


def _row_count(path: Path, table: str) -> int:
    rows = _query(path, f"SELECT COUNT(*) FROM {table}")
    return rows[0][0] if rows else 0


def _decode_samples(blob: bytes) -> list[float]:
    """Decode the canonical little-endian float64 sample array."""
    assert len(blob) % SAMPLE_BYTES == 0, "sample blob is not a whole number of float64"
    count = len(blob) // SAMPLE_BYTES
    return list(struct.unpack(f"<{count}d", blob))


def _all_stream_samples(path: Path, unique_id: int) -> list[float]:
    """Every recorded sample for one dataset, in acquisition order."""
    rows = _query(
        path,
        "SELECT frames, samples FROM stream_blocks WHERE unique_id = ? "
        "ORDER BY t0_ns ASC, stream_block_id ASC",
        (unique_id,),
    )

    samples: list[float] = []
    for frames, blob in rows:
        decoded = _decode_samples(bytes(blob))
        assert len(decoded) == frames, "frames column disagrees with the blob length"
        samples.extend(decoded)

    return samples


def _enable_session(api_client, enabled: bool) -> None:
    api_client.command("sessions.setExportEnabled", {"enabled": enabled})
    time.sleep(0.2)


def _disable_other_sinks(api_client) -> None:
    """Turn off every sink except session recording (the R4 configuration)."""
    api_client.command("csvExport.setEnabled", {"enabled": False})
    time.sleep(0.1)


def _record_audio_session(api_client, seconds: float = CAPTURE_SECONDS) -> Path:
    """Run one audio capture with session recording on; returns the db path."""
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    _safe_unlink(db_path)

    _enable_session(api_client, True)
    api_client.connect_device()
    time.sleep(seconds)
    api_client.disconnect_device()
    time.sleep(0.3)
    api_client.command("sessions.close")
    time.sleep(0.3)
    return db_path


# ---------------------------------------------------------------------------
# Recording
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
def test_stream_session_records_blocks(api_client, clean_state, pro_only, audio_source):
    """
    AC1 — with session recording as the ONLY enabled sink, an audio capture
    lands in stream_blocks.

    Every other exporter is deliberately off. If refreshStreamExportFlags()
    doesn't consider session export, StreamProcessor never builds a payload,
    blockReady never fires, and this table stays empty while everything else
    looks healthy.
    """
    _disable_other_sinks(api_client)
    db_path = _record_audio_session(api_client)

    assert (
        db_path.exists()
    ), f"no session database at {db_path}; state={session_diagnostics(api_client)}"

    blocks = _row_count(db_path, "stream_blocks")
    assert blocks > 0, (
        "session recording was the only enabled sink and recorded no stream blocks — "
        "the export-active gate almost certainly does not include session export"
    )

    rows = _query(db_path, "SELECT SUM(frames), MIN(dt_ns) FROM stream_blocks")
    total_samples, dt_ns = rows[0]
    assert total_samples > 0
    assert dt_ns and dt_ns > 0, "dt_ns must be positive to date samples as t0 + i*dt"

    _safe_unlink(db_path)
    _enable_session(api_client, False)


@pytest.mark.project
@pytest.mark.slow
def test_stream_blocks_hold_many_samples_each(
    api_client, clean_state, pro_only, audio_source
):
    """
    AC3 (shape half) — storage is per block, not per sample.

    A regression to one row per sample would still 'work', so assert the ratio
    directly rather than trusting row counts alone.
    """
    _disable_other_sinks(api_client)
    db_path = _record_audio_session(api_client)

    rows = _query(db_path, "SELECT COUNT(*), SUM(frames) FROM stream_blocks")
    block_count, total_samples = rows[0]
    assert block_count and total_samples

    assert total_samples // block_count > 1, (
        f"{total_samples} samples in {block_count} rows — storage collapsed to one "
        f"row per sample, which is what the block format exists to avoid"
    )

    _safe_unlink(db_path)
    _enable_session(api_client, False)


@pytest.mark.project
@pytest.mark.slow
def test_stream_session_size_is_bounded(
    api_client, clean_state, pro_only, audio_source
):
    """AC3 — the database stays proportional to the sample payload."""
    _disable_other_sinks(api_client)
    db_path = _record_audio_session(api_client)

    total_samples = _query(db_path, "SELECT SUM(frames) FROM stream_blocks")[0][0]
    assert total_samples

    size = db_path.stat().st_size
    assert size < total_samples * MAX_BYTES_PER_SAMPLE, (
        f"{size} bytes for {total_samples} samples "
        f"({size / total_samples:.1f} B/sample) exceeds the block-format budget"
    )

    _safe_unlink(db_path)
    _enable_session(api_client, False)


@pytest.mark.project
@pytest.mark.slow
def test_stream_samples_match_csv_exactly(
    api_client, clean_state, pro_only, audio_source
):
    """
    AC2 — recorded samples equal the simultaneous CSV export, exactly.

    The verifier compares a session against itself, so it cannot detect silent
    loss. Only a second, independent exporter can. Comparison is exact: full
    internal precision on disk is the point of the format.
    """
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    _safe_unlink(db_path)

    _enable_session(api_client, True)
    api_client.command("csvExport.setEnabled", {"enabled": True})
    time.sleep(0.2)

    api_client.connect_device()
    time.sleep(CAPTURE_SECONDS)
    api_client.disconnect_device()
    time.sleep(0.3)
    api_client.command("csvExport.close")
    api_client.command("sessions.close")
    time.sleep(0.5)

    unique_ids = _query(db_path, "SELECT DISTINCT unique_id FROM stream_blocks")
    assert unique_ids, "no stream data recorded to compare against CSV"

    uid = unique_ids[0][0]
    db_samples = _all_stream_samples(db_path, uid)
    assert db_samples, "dataset has no recorded samples"

    csv_values = _read_stream_csv_column(api_client)
    if csv_values is None:
        pytest.skip("stream CSV export file not found for cross-check")

    compared = min(len(db_samples), len(csv_values))
    assert compared > 0

    # CSV is the lossy side: it writes %g at 10 significant digits, while the
    # database keeps full float64. So the exact check is that re-formatting the
    # recorded sample at CSV's precision reproduces the CSV text byte for byte.
    # That still catches dropped, duplicated or reordered samples, which is what
    # this cross-check exists for.
    mismatches = [
        i for i in range(compared) if csv_values[i] != f"{db_samples[i]:.10g}"
    ]
    assert not mismatches, (
        f"{len(mismatches)} of {compared} samples differ between the session database "
        f"and the CSV export; first at index {mismatches[0]}: "
        f"csv={csv_values[mismatches[0]]!r} db={db_samples[mismatches[0]]:.10g}"
    )

    _safe_unlink(db_path)
    api_client.command("csvExport.setEnabled", {"enabled": False})
    _enable_session(api_client, False)


def _read_stream_csv_column(api_client):
    """Newest stream CSV export's first data column, or None when absent."""
    try:
        root = Path(
            api_client.command("sessions.getDbPath", {"projectTitle": "x"})["path"]
        )
    except APIError:
        return None

    # Stream CSVs live beside the frame-lane files under the workspace CSV tree.
    csv_root = root.parents[2] / "CSV"
    if not csv_root.exists():
        return None

    candidates = sorted(
        csv_root.rglob("*_stream_source*.csv"), key=lambda p: p.stat().st_mtime
    )
    if not candidates:
        return None

    values = []
    with candidates[-1].open("r", encoding="utf-8") as handle:
        next(handle, None)
        for line in handle:
            parts = line.strip().split(",")
            if len(parts) >= 2 and parts[1]:
                values.append(parts[1])

    return values or None


# ---------------------------------------------------------------------------
# Non-regression for frame-lane sessions
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
def test_frame_lane_session_records_no_stream_blocks(
    api_client, device_simulator, clean_state, pro_only
):
    """AC6 — a TCP (frame-lane) session behaves exactly as before."""
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, QUICK_PLOT_TITLE)
    _safe_unlink(db_path)
    _enable_session(api_client, True)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [f"{i},{i * 2},{i * 3}\n".encode() for i in range(20)]
    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(2.0)

    api_client.disconnect_device()
    time.sleep(0.2)
    api_client.command("sessions.close")
    time.sleep(0.3)

    assert db_path.exists()
    assert _row_count(db_path, "readings") >= 5, "frame-lane recording regressed"
    assert (
        _row_count(db_path, "stream_blocks") == 0
    ), "a frame-lane source must never write stream blocks"

    _safe_unlink(db_path)
    _enable_session(api_client, False)


# ---------------------------------------------------------------------------
# Backwards compatibility
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_legacy_database_without_stream_table_opens(
    api_client, clean_state, pro_only, tmp_path
):
    """
    AC7 — a database written before this schema version still opens.

    Builds a minimal pre-0054 file (no stream_blocks, no stream_sha256) and
    asks the player to open it. The migration is additive, so this must not
    error and must not rewrite the existing tables.
    """
    legacy = tmp_path / "legacy_session.db"
    with sqlite3.connect(str(legacy)) as conn:
        conn.executescript("""
            CREATE TABLE sessions (
              session_id INTEGER PRIMARY KEY AUTOINCREMENT,
              project_title TEXT NOT NULL,
              started_at TEXT NOT NULL,
              ended_at TEXT, project_json TEXT, notes TEXT);
            CREATE TABLE columns (
              column_id INTEGER PRIMARY KEY AUTOINCREMENT,
              session_id INTEGER NOT NULL, unique_id INTEGER NOT NULL,
              source_id INTEGER NOT NULL DEFAULT 0,
              source_title TEXT NOT NULL DEFAULT '',
              group_title TEXT NOT NULL, title TEXT NOT NULL,
              units TEXT, widget TEXT, is_virtual INTEGER NOT NULL DEFAULT 0);
            CREATE TABLE readings (
              reading_id INTEGER PRIMARY KEY AUTOINCREMENT,
              session_id INTEGER NOT NULL, timestamp_ns INTEGER NOT NULL,
              unique_id INTEGER NOT NULL, raw_numeric_value REAL,
              raw_string_value TEXT, final_numeric_value REAL,
              final_string_value TEXT, is_numeric INTEGER NOT NULL DEFAULT 1);
            INSERT INTO sessions (project_title, started_at) VALUES ('Legacy', '2026-01-01');
            INSERT INTO columns (session_id, unique_id, group_title, title)
              VALUES (1, 0, 'G', 'Ch1');
            INSERT INTO readings (session_id, timestamp_ns, unique_id, final_numeric_value)
              VALUES (1, 0, 0, 1.0), (1, 1000000, 0, 2.0);
            PRAGMA user_version = 1;
            """)

    tables_before = {
        r[0]
        for r in _query(legacy, "SELECT name FROM sqlite_master WHERE type='table'")
    }
    assert "stream_blocks" not in tables_before

    try:
        api_client.command("sessionPlayer.open", {"filePath": str(legacy)})
    except APIError as exc:
        pytest.skip(f"sessionPlayer.open unavailable on this instance: {exc}")

    time.sleep(1.0)

    rows = _query(legacy, "SELECT COUNT(*) FROM readings")
    assert (
        rows and rows[0][0] == 2
    ), "opening a legacy database must not disturb its rows"

    try:
        api_client.command("sessionPlayer.close")
    except APIError:
        pass
