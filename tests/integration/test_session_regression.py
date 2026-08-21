"""
Golden-Session Parser Regression Integration Tests (spec 0047, Pro).

Covers the --regress-session dual-replay child process through the API surface:
 * AC1 -- an untouched project regresses as `identical` with all-zero drift
 * AC2 -- a transform edit yields `value-drift` with exact counts and deltas
 * AC3 -- a frame-rejecting parser yields `coverage-drift` with ZERO value
   changes (the ordinal-pairing failure mode must not reproduce)
 * AC4 -- adding/removing datasets yields `structural-drift` naming both
 * AC5 -- an explicit candidate file is used and fingerprinted in the report
 * AC6 -- classification honored (table-fed skipped; legacy qualified)
 * AC7 -- a golden-tag sweep aggregates per-session verdicts
 * AC8 -- regression during live capture leaves the capture intact; verify
   and regress never run concurrently (destructive-marked)
 * Addendum -- structured errors carry errorCode + hint

Guard run: after this file, run test_session_verification.py to prove the
spec-0044 verify mode is untouched.

All tests are skipped on non-commercial builds via the `pro_only` fixture.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
"""

import json
import sqlite3
import time
from pathlib import Path

import pytest

from utils import APIError, session_diagnostics

# ---------------------------------------------------------------------------
# Fixtures & helpers (mirrors test_session_verification.py conventions)
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
        pytest.skip("session recording not licensed on this instance")


@pytest.fixture
def regress_available(api_client, pro_only):
    if not api_client.command_exists("sessions.regress"):
        pytest.skip("sessions.regress not available")


JS_PARSER = "function parse(frame) { return frame.split(','); }"

JS_PARSER_REJECTING = (
    "function parse(frame) {"
    "  if (parseInt(frame) % 30 === 0) return null;"
    "  return frame.split(',');"
    "}"
)


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


def _record_session(
    api_client, device_simulator, title: str, fresh_db: bool = True, with_table=False
) -> Path:
    """Record a small JS-parser session and return the canonical .db path."""
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

    if with_table:
        api_client.command("project.dataTable.add", {"name": "CalTable"})
        time.sleep(0.1)
        api_client.command(
            "project.dataset.setTransformCode",
            {
                "groupId": 0,
                "datasetId": 0,
                "code": "function transform(v) { return v; }",
                "language": 0,
            },
        )
        time.sleep(0.1)

    db_path = _db_path_for(api_client, title)
    if fresh_db:
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

    assert db_path.exists(), (
        f"session DB was never created at {db_path}; "
        f"state={session_diagnostics(api_client)}"
    )
    return db_path


def _open_archive(api_client, path: Path) -> int:
    api_client.command("sessions.openDatabase", {"filePath": str(path)})
    time.sleep(0.5)

    sessions = api_client.command("sessions.list").get("sessions", [])
    assert sessions, f"no sessions listed in {path}"
    return max(s["session_id"] for s in sessions)


def _run_regression(api_client, params: dict, timeout: float = 120.0) -> dict:
    """Start sessions.regress and poll sessions.getRegression to completion."""
    api_client.command("sessions.regress", params)

    deadline = time.time() + timeout
    while time.time() < deadline:
        state = api_client.command("sessions.getRegression")
        if not state.get("running", False):
            return state
        time.sleep(0.5)

    pytest.fail(f"regression {params} did not finish in {timeout}s")


def _datasets(report: dict) -> list:
    return report.get("datasets", [])


def _compared_entries(report: dict) -> list:
    return [d for d in _datasets(report) if "compared" in d]


# ---------------------------------------------------------------------------
# AC1 -- untouched project is identical
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_identity_regresses_identical(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress Identity")
    session_id = _open_archive(api_client, db_path)

    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] == "identical", report
    assert report["mode"] == "regression"
    for entry in _compared_entries(report):
        assert entry["changed"] == 0, entry
        assert entry["onlyBaseline"] == 0, entry
        assert entry["onlyCandidate"] == 0, entry
        assert entry["compared"] > 0, entry


# ---------------------------------------------------------------------------
# AC2 -- transform edit is pure value drift
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_transform_edit_is_value_drift(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress Value")
    session_id = _open_archive(api_client, db_path)

    # The transform parameter MUST be named `value`: a transform whose code lacks
    # the free identifier `value` is auto-classified as a computed/virtual dataset
    # (ProjectModelLoading resolveDatasetVirtualFlags), which the regression diff
    # skips -- yielding a false `identical` verdict instead of `value-drift`.
    api_client.command(
        "project.dataset.setTransformCode",
        {
            "groupId": 0,
            "datasetId": 0,
            "code": "function transform(value) { return value * 10; }",
            "language": 0,
        },
    )
    time.sleep(0.2)

    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] == "value-drift", report
    entries = _compared_entries(report)
    assert entries, report
    drifting = [d for d in entries if d["changed"] > 0]
    assert drifting, report
    for entry in drifting:
        assert entry["changed"] == entry["compared"] - 1, entry
        assert entry["onlyBaseline"] == 0, entry
        assert entry["onlyCandidate"] == 0, entry
        assert entry.get("maxDelta", 0) > 0, entry
        first = entry.get("firstDivergence", {})
        assert first.get("stage") == "transform", entry


# ---------------------------------------------------------------------------
# AC3 -- frame rejection is coverage drift, never false value diffs
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_frame_rejection_is_coverage_drift_only(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress Coverage")
    session_id = _open_archive(api_client, db_path)

    api_client.command(
        "project.frameParser.setCode",
        {"code": JS_PARSER_REJECTING, "language": 0, "sourceId": 0},
    )
    time.sleep(0.2)

    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] == "coverage-drift", report
    entries = _compared_entries(report)
    assert entries, report
    total_missing = sum(d["onlyBaseline"] for d in entries)
    total_changed = sum(d["changed"] for d in entries)
    assert total_missing > 0, report
    assert total_changed == 0, (
        "ordinal-pairing failure mode: frame loss reported as value changes",
        report,
    )


# ---------------------------------------------------------------------------
# AC4 -- dataset add/remove is structural drift
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_dataset_add_remove_is_structural_drift(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress Structural")
    session_id = _open_archive(api_client, db_path)

    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.2)

    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] == "structural-drift", report
    added = [d for d in _datasets(report) if d.get("structural") == "added"]
    assert added, report
    for entry in _compared_entries(report):
        assert entry["changed"] == 0, entry


# ---------------------------------------------------------------------------
# AC5 -- explicit candidate file, fingerprinted
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_explicit_candidate_file(
    api_client, device_simulator, clean_state, regress_available, tmp_path
):
    db_path = _record_session(api_client, device_simulator, "Regress Candidate")
    session_id = _open_archive(api_client, db_path)

    config = api_client.command("project.exportJson")["config"]
    candidate = tmp_path / "candidate.ssproj"
    candidate.write_text(json.dumps(config), encoding="utf-8")

    state = _run_regression(
        api_client, {"sessionId": session_id, "projectPath": str(candidate)}
    )
    report = state["report"]

    assert report["verdict"] == "identical", report
    assert report["candidate"]["path"] == str(candidate), report
    assert len(report["candidate"]["sha256"]) == 64, report


# ---------------------------------------------------------------------------
# AC6 -- classification honored: table-fed skipped, legacy qualified
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_table_fed_datasets_classified(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(
        api_client, device_simulator, "Regress Classified", with_table=True
    )
    session_id = _open_archive(api_client, db_path)

    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] != "error", report
    flagged = [
        d
        for d in _datasets(report)
        if "skipped" in d or d.get("finalsCompared") is False
    ]
    assert flagged, report


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_legacy_archive_regresses_with_qualifier(
    api_client, device_simulator, clean_state, regress_available, tmp_path
):
    db_path = _record_session(api_client, device_simulator, "Regress Legacy")
    _close_archive_quietly(api_client)

    legacy = tmp_path / "legacy.db"
    legacy.write_bytes(db_path.read_bytes())
    with sqlite3.connect(legacy) as conn:
        conn.execute("UPDATE sessions SET capture_format = NULL, repro_class = NULL")
        conn.commit()

    session_id = _open_archive(api_client, legacy)
    state = _run_regression(api_client, {"sessionId": session_id})
    report = state["report"]

    assert report["verdict"] in (
        "identical",
        "value-drift",
        "coverage-drift",
        "structural-drift",
    ), report
    assert report["legacyCapture"] is True, report
    assert report.get("notes"), report


def _close_archive_quietly(api_client) -> None:
    try:
        api_client.command("sessions.close")
    except APIError:
        pass
    time.sleep(0.3)


# ---------------------------------------------------------------------------
# AC7 -- golden-tag sweep aggregates
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(360)
def test_golden_tag_sweep(api_client, device_simulator, clean_state, regress_available):
    title = "Regress Sweep"
    db_path = _record_session(api_client, device_simulator, title)
    _record_session(api_client, device_simulator, title, fresh_db=False)
    _record_session(api_client, device_simulator, title, fresh_db=False)

    api_client.command("sessions.openDatabase", {"filePath": str(db_path)})
    time.sleep(0.5)
    sessions = api_client.command("sessions.list").get("sessions", [])
    assert len(sessions) >= 3, sessions

    api_client.command("sessions.addTag", {"label": "golden"})
    time.sleep(0.2)
    tags = api_client.command("sessions.listTags").get("tags", [])
    tag_id = next(t["tag_id"] for t in tags if t["label"] == "golden")
    for s in sessions[:3]:
        api_client.command(
            "sessions.assignTag", {"sessionId": s["session_id"], "tagId": tag_id}
        )
        time.sleep(0.1)

    api_client.command(
        "project.dataset.setTransformCode",
        {
            "groupId": 0,
            "datasetId": 0,
            "code": "function transform(value) { return value + 1; }",
            "language": 0,
        },
    )
    time.sleep(0.2)

    state = _run_regression(api_client, {"tag": "golden"}, timeout=300.0)
    sweep = state["sweep"]

    reports = sweep.get("reports", [])
    assert len(reports) == 3, sweep
    summary = sweep["summary"]
    verdicts = [r["report"].get("verdict") for r in reports]
    assert summary["drifted"] == sum(1 for v in verdicts if v.endswith("-drift")), sweep
    assert summary["passed"] == sum(1 for v in verdicts if v == "identical"), sweep
    assert summary["drifted"] >= 1, sweep


# ---------------------------------------------------------------------------
# AC8 -- non-interference and mutual exclusion (destructive)
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.destructive
@pytest.mark.timeout(360)
def test_regression_during_live_capture(
    api_client, device_simulator, clean_state, regress_available
):
    archived = _record_session(api_client, device_simulator, "Regress Concurrent A")
    session_id = _open_archive(api_client, archived)

    _enable_export(api_client, True)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)
    device_simulator.send_frames(
        [b"/*" + f"{i}".encode() + b"*/" for i in range(50)],
        interval_seconds=0.1,
    )

    api_client.command("sessions.regress", {"sessionId": session_id})

    with pytest.raises(APIError):
        api_client.command("sessions.verify", {"sessionId": session_id})

    deadline = time.time() + 180
    while time.time() < deadline:
        if not api_client.command("sessions.getRegression").get("running"):
            break
        time.sleep(0.5)

    report = api_client.command("sessions.getRegression")["report"]
    assert report.get("verdict"), report

    status = api_client.command("sessions.getStatus")
    assert status.get("exportEnabled", False) is True

    api_client.disconnect_device()
    time.sleep(0.2)
    _close_session(api_client)
    _enable_export(api_client, False)


# ---------------------------------------------------------------------------
# Addendum -- structured errors carry errorCode + hint
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_structured_error_for_bad_session_id(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress Errors")
    _open_archive(api_client, db_path)

    state = _run_regression(api_client, {"sessionId": 99999})
    report = state["report"]

    assert report["verdict"] == "error", report
    assert report["errorCode"] == "session-not-found", report
    assert report.get("hint"), report


@pytest.mark.project
@pytest.mark.slow
@pytest.mark.timeout(240)
def test_structured_error_for_invalid_candidate(
    api_client, device_simulator, clean_state, regress_available
):
    db_path = _record_session(api_client, device_simulator, "Regress BadCand")
    session_id = _open_archive(api_client, db_path)

    state = _run_regression(
        api_client,
        {"sessionId": session_id, "projectJson": "this is not json"},
    )
    report = state["report"]

    assert report["verdict"] == "error", report
    assert report["errorCode"] == "regress-candidate-invalid", report
    assert report.get("hint"), report
