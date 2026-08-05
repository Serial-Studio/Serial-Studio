"""
Session Reproducibility Verification Integration Tests (spec 0044, Pro).

Covers the --verify-session child-process verifier through the API surface:
 * AC1 -- a freshly recorded session (Native/QuickPlot and JS parser variants)
   verifies as `reproduced` with zero divergences
 * AC2 -- tampering with one recorded reading yields `diverged`, names the
   dataset, and the integrity stage attributes it to archive modification
 * AC3 -- tampering with the stored project configuration yields `diverged`
   attributed to interpretation (integrity hashes still verify)
 * AC4 -- transforms + data tables classify the session as not mechanically
   verifiable for finals (`partial`), never a false `reproduced`
 * AC5 -- a legacy (pre-0044) archive verifies best-effort with the legacy
   qualifier and no crash or refusal
 * AC6 -- verification runs concurrently with a live capture without
   disturbing it (destructive-marked)
 * AC7 -- the verdict is stored in the archive (verifications table), so it
   survives app restart; sessions.getVerification reads it back

All tests are skipped on non-commercial builds via the `pro_only` fixture.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import json
import shutil
import sqlite3
import time
from pathlib import Path

import pytest

from utils import APIError

# ---------------------------------------------------------------------------
# Pro gating fixtures
# ---------------------------------------------------------------------------


@pytest.fixture
def pro_only(api_client):
    """Skip the test when session recording isn't usable on this instance."""
    if not api_client.command_exists("sessions.getStatus"):
        pytest.skip("sessions.* API not available -- GPL build")

    try:
        api_client.command("sessions.getStatus")
    except APIError:
        pytest.skip("sessions.* API registered but returning errors")

    try:
        api_client.command("sessions.setExportEnabled", {"enabled": True})
        time.sleep(0.1)
        enabled = api_client.command("sessions.getStatus").get("exportEnabled", False)
        api_client.command("sessions.setExportEnabled", {"enabled": False})
        time.sleep(0.1)
    except APIError:
        pytest.skip("session recording refused enable (license-gated)")

    if not enabled:
        pytest.skip("session recording is license-gated on this instance")


@pytest.fixture
def verify_available(api_client, pro_only):
    """Skip when the spec-0044 verification verbs are absent (older build)."""
    if not api_client.command_exists("sessions.verify"):
        pytest.skip("sessions.verify not available on this build")


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

JS_PARSER = "function parse(frame) { return frame.split(','); }"


def _enable_export(api_client, enabled: bool) -> None:
    api_client.command("sessions.setExportEnabled", {"enabled": enabled})
    time.sleep(0.1)


def _close_session(api_client) -> None:
    api_client.command("sessions.close")
    time.sleep(0.3)


def _db_path_for(api_client, title: str) -> Path:
    result = api_client.command("sessions.getDbPath", {"projectTitle": title})
    return Path(result["path"])


def _safe_unlink(path: Path) -> None:
    for suffix in ("", "-wal", "-shm"):
        target = path.with_name(path.name + suffix)
        for attempt in range(10):
            try:
                target.unlink()
                break
            except FileNotFoundError:
                break
            except PermissionError:
                if attempt == 9:
                    break
                time.sleep(0.3)


def _record_js_session(
    api_client, device_simulator, title: str, transform=None, with_table=False
) -> Path:
    """
    Record a small ProjectFile-mode session with the JS comma-split parser and
    return the canonical .db path. Optionally attaches a per-dataset transform
    and/or a user data table (AC4 classification inputs).
    """
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

    api_client.command(
        "project.frameParser.setCode",
        {"code": JS_PARSER, "language": 0, "sourceId": 0},
    )
    time.sleep(0.15)

    if transform is not None:
        api_client.command(
            "project.dataset.setTransformCode",
            {"groupId": 0, "datasetId": 0, "code": transform, "language": 0},
        )
        time.sleep(0.1)

    if with_table:
        api_client.command("project.dataTable.add", {"name": "CalTable"})
        time.sleep(0.1)

    db_path = _db_path_for(api_client, title)
    _safe_unlink(db_path)

    _enable_export(api_client, True)
    api_client.command("project.activate")
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [f"{i * 10}".encode() for i in range(20)]
    device_simulator.send_frames(
        [b"/*" + f + b"*/" for f in frames],
        interval_seconds=0.05,
    )
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)
    _enable_export(api_client, False)

    assert db_path.exists(), f"session DB was never created at {db_path}"
    return db_path


def _open_archive(api_client, path: Path) -> int:
    """Open an archive for browsing and return its latest session id."""
    api_client.command("sessions.openDatabase", {"filePath": str(path)})
    time.sleep(0.5)

    sessions = api_client.command("sessions.list").get("sessions", [])
    assert sessions, f"no sessions listed in {path}"
    return max(s["session_id"] for s in sessions)


def _run_verification(api_client, session_id: int, timeout: float = 90.0) -> dict:
    """Start sessions.verify and poll sessions.getVerification to completion."""
    api_client.command("sessions.verify", {"sessionId": session_id})

    deadline = time.time() + timeout
    while time.time() < deadline:
        state = api_client.command(
            "sessions.getVerification", {"sessionId": session_id}
        )
        if not state.get("verifying", False) and state.get("verification"):
            return state["verification"]
        time.sleep(0.5)

    pytest.fail(f"verification of session {session_id} did not finish in {timeout}s")


def _detail(verification: dict) -> dict:
    detail = verification.get("detail", {})
    assert detail, "verification record has no detail JSON"
    return detail


def _dataset_entries(verification: dict) -> list:
    return _detail(verification).get("datasets", [])


# ---------------------------------------------------------------------------
# AC1 -- round trip reproduces
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_js_session_reproduced(
    api_client, device_simulator, clean_state, verify_available
):
    """A freshly recorded JS-parser session verifies as reproduced (AC1)."""
    title = f"VerifyJs_{time.time_ns()}"
    db_path = _record_js_session(api_client, device_simulator, title)

    session_id = _open_archive(api_client, db_path)
    verification = _run_verification(api_client, session_id)

    assert verification["verdict"] == "reproduced", _detail(verification)
    for entry in _dataset_entries(verification):
        assert entry.get("mismatches", 0) == 0
        assert not entry.get("countMismatch", False)

    _safe_unlink(db_path)


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_quickplot_native_session_reproduced(
    api_client, device_simulator, clean_state, verify_available
):
    """A QuickPlot session (synthesized Native parser project) reproduces (AC1)."""
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    db_path = _db_path_for(api_client, "Quick Plot")
    _safe_unlink(db_path)

    _enable_export(api_client, True)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    frames = [f"{i},{i * 2},{i * 3}\n".encode() for i in range(20)]
    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(2.5)

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)
    _enable_export(api_client, False)

    session_id = _open_archive(api_client, db_path)
    verification = _run_verification(api_client, session_id)

    assert verification["verdict"] == "reproduced", _detail(verification)

    _safe_unlink(db_path)


# ---------------------------------------------------------------------------
# AC2 / AC3 -- tamper detection and attribution
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_tampered_reading_attributed_to_archive(
    api_client, device_simulator, clean_state, verify_available, tmp_path
):
    """Flipping one recorded reading diverges and flags archive modification (AC2)."""
    title = f"VerifyTamper_{time.time_ns()}"
    db_path = _record_js_session(api_client, device_simulator, title)

    copy_path = tmp_path / "tampered.db"
    shutil.copy(db_path, copy_path)
    with sqlite3.connect(str(copy_path)) as conn:
        conn.execute(
            "UPDATE readings SET final_numeric_value = final_numeric_value + 1, "
            "final_string_value = 'tampered' WHERE reading_id = "
            "(SELECT MIN(reading_id) FROM readings)"
        )
        conn.commit()

    session_id = _open_archive(api_client, copy_path)
    verification = _run_verification(api_client, session_id)

    assert verification["verdict"] == "diverged", _detail(verification)

    detail = _detail(verification)
    assert (
        detail["integrity"]["readings"] == "mismatch"
    ), "readings fingerprint should attribute the tamper to archive modification"

    flagged = [e for e in _dataset_entries(verification) if e.get("mismatches", 0) > 0]
    assert flagged, "no dataset reported the tampered reading"
    assert flagged[0].get("firstMismatch"), "first mismatch pair missing"

    _safe_unlink(db_path)


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_tampered_parser_attributed_to_interpretation(
    api_client, device_simulator, clean_state, verify_available, tmp_path
):
    """Editing the stored parser diverges with intact integrity hashes (AC3)."""
    title = f"VerifyInterp_{time.time_ns()}"
    db_path = _record_js_session(api_client, device_simulator, title)

    copy_path = tmp_path / "reinterpreted.db"
    shutil.copy(db_path, copy_path)
    evil_parser = "function parse(frame) { return frame.split(',').map(x => x * 2); }"
    with sqlite3.connect(str(copy_path)) as conn:
        row = conn.execute(
            "SELECT session_id, project_json FROM sessions "
            "ORDER BY session_id DESC LIMIT 1"
        ).fetchone()
        project = json.loads(row[1])
        for source in project.get("sources", []):
            source["frameParserCode"] = evil_parser
        conn.execute(
            "UPDATE sessions SET project_json = ? WHERE session_id = ?",
            (json.dumps(project), row[0]),
        )
        conn.commit()

    session_id = _open_archive(api_client, copy_path)
    verification = _run_verification(api_client, session_id)

    assert verification["verdict"] == "diverged", _detail(verification)

    detail = _detail(verification)
    assert detail["integrity"]["rawBytes"] == "verified"
    assert detail["integrity"]["readings"] == "verified", (
        "readings hash must still verify -- the divergence is interpretation, "
        "not archive damage"
    )

    flagged = [e for e in _dataset_entries(verification) if e.get("mismatches", 0) > 0]
    assert flagged, "no dataset reported the interpretation change"
    assert flagged[0]["firstMismatch"]["stage"] == "parse"

    _safe_unlink(db_path)


# ---------------------------------------------------------------------------
# AC4 -- classification beats a false green
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_table_fed_transforms_classified(
    api_client, device_simulator, clean_state, verify_available
):
    """Transforms + data tables classify finals as not verifiable (AC4)."""
    title = f"VerifyClass_{time.time_ns()}"
    db_path = _record_js_session(
        api_client,
        device_simulator,
        title,
        transform="function transform(value) { return value; }",
        with_table=True,
    )

    session_id = _open_archive(api_client, db_path)
    verification = _run_verification(api_client, session_id)

    assert verification["verdict"] in ("partial", "not_verifiable"), (
        f"table-fed transforms must never verify as plain reproduced: "
        f"{_detail(verification)}"
    )

    classification = _detail(verification).get("classification", {})
    assert classification.get("transformsPresent") is True
    assert classification.get("tablesPresent") is True

    _safe_unlink(db_path)


# ---------------------------------------------------------------------------
# AC5 -- legacy archives
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_legacy_archive_verifies_with_qualifier(
    api_client, device_simulator, clean_state, verify_available, tmp_path
):
    """A pre-0044 archive verifies best-effort with the legacy qualifier (AC5)."""
    title = f"VerifyLegacy_{time.time_ns()}"
    db_path = _record_js_session(api_client, device_simulator, title)

    legacy_path = tmp_path / "legacy.db"
    shutil.copy(db_path, legacy_path)
    with sqlite3.connect(str(legacy_path)) as conn:
        if sqlite3.sqlite_version_info < (3, 35, 0):
            pytest.skip("sqlite3 too old to strip spec-0044 columns")

        for column in (
            "raw_sha256",
            "readings_sha256",
            "app_version",
            "capture_format",
            "repro_class",
            "frames_dropped",
            "overflow_bytes",
        ):
            conn.execute(f"ALTER TABLE sessions DROP COLUMN {column}")
        conn.execute("DROP TABLE IF EXISTS verifications")
        conn.execute("PRAGMA user_version = 0")
        conn.commit()

    session_id = _open_archive(api_client, legacy_path)
    verification = _run_verification(api_client, session_id)

    detail = _detail(verification)
    assert detail["legacyCapture"] is True
    assert verification["verdict"] in (
        "reproduced",
        "diverged",
        "partial",
        "not_verifiable",
    ), f"legacy archive must not error out: {detail}"
    assert detail["integrity"]["rawBytes"] == "missing"
    assert detail["integrity"]["readings"] == "missing"

    _safe_unlink(db_path)


# ---------------------------------------------------------------------------
# AC6 -- concurrent with a live capture
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.destructive
@pytest.mark.timeout(240)
def test_verification_during_live_capture(
    api_client, device_simulator, clean_state, verify_available, tmp_path
):
    """Verification must not disturb a live capture (AC6)."""
    archived_title = f"VerifyConcA_{time.time_ns()}"
    archived_db = _record_js_session(api_client, device_simulator, archived_title)
    archive_copy = tmp_path / "concurrent.db"
    shutil.copy(archived_db, archive_copy)

    live_title = f"VerifyConcB_{time.time_ns()}"
    api_client.create_new_project(live_title)
    time.sleep(0.2)
    api_client.command("project.group.add", {"title": "G", "widgetType": 0})
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*", end_sequence="*/", checksum_algorithm="", operation_mode=0
    )
    api_client.configure_frame_parser(frame_detection=1, operation_mode=0)
    api_client.command(
        "project.frameParser.setCode", {"code": JS_PARSER, "language": 0, "sourceId": 0}
    )
    time.sleep(0.15)

    live_db = _db_path_for(api_client, live_title)
    _safe_unlink(live_db)

    _enable_export(api_client, True)
    api_client.command("project.activate")
    time.sleep(0.2)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    session_id = _open_archive(api_client, archive_copy)
    api_client.command("sessions.verify", {"sessionId": session_id})

    frames = [b"/*" + f"{i}".encode() + b"*/" for i in range(60)]
    device_simulator.send_frames(frames, interval_seconds=0.05)
    time.sleep(3.0)

    deadline = time.time() + 120
    verification = None
    while time.time() < deadline:
        state = api_client.command(
            "sessions.getVerification", {"sessionId": session_id}
        )
        if not state.get("verifying", False) and state.get("verification"):
            verification = state["verification"]
            break
        time.sleep(0.5)
    assert verification, "verification never completed while capture was live"

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)
    _enable_export(api_client, False)

    assert live_db.exists(), "live capture DB vanished during verification"
    with sqlite3.connect(str(live_db)) as conn:
        live_rows = conn.execute("SELECT COUNT(*) FROM readings").fetchone()[0]
    assert live_rows >= 10, "live capture stopped recording during verification"

    assert verification["verdict"] == "reproduced", _detail(verification)

    _safe_unlink(archived_db)
    _safe_unlink(live_db)


# ---------------------------------------------------------------------------
# AC7 -- verdict durability
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(180)
def test_verdict_stored_in_archive(
    api_client, device_simulator, clean_state, verify_available
):
    """
    The verdict is appended to the archive's verifications table, so it
    survives app restart (AC7 -- restart itself is a maintainer observation;
    this asserts the durable row exists and reads back).
    """
    title = f"VerifyStore_{time.time_ns()}"
    db_path = _record_js_session(api_client, device_simulator, title)

    session_id = _open_archive(api_client, db_path)
    verification = _run_verification(api_client, session_id)
    assert verification["verdict"]

    with sqlite3.connect(str(db_path)) as conn:
        rows = conn.execute(
            "SELECT verdict, app_version, detail_json FROM verifications "
            "WHERE session_id = ?",
            (session_id,),
        ).fetchall()
    assert rows, "no verification row was appended to the archive"
    assert rows[-1][0] == verification["verdict"]
    assert json.loads(rows[-1][2])["verdict"] == verification["verdict"]

    fresh = api_client.command("sessions.getVerification", {"sessionId": session_id})
    assert fresh["verification"]["verdict"] == verification["verdict"]

    _safe_unlink(db_path)
