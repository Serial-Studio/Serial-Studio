"""
Silent-failure hardening integration tests (spec 0088)

Covers the seven hardening requirements end to end: transform result hygiene
(R1), locale-independent alarm participation (R2), acknowledgements surviving
an unrequested link drop (R3), the console log honoring the session boundary
and pause (R4), starved-dataset reporting for short frames (R5), the
control-script death finding (R6) and the disabled-annunciator finding (R7).

The operator-requested-disconnect variant of R3 is already pinned by
test_aural_alerts.py::test_band_entry_raises_and_disconnect_clears. R7 has no
API verb for the master enable, so its test asserts consistency against the
current preference instead of toggling it.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _state(api_client) -> dict:
    return api_client.command("alarms.state")


def _point(state: dict, title: str) -> dict:
    for point in state.get("points", []):
        if point.get("title") == title:
            return point

    return {}


def _fresh(api_client) -> None:
    api_client.command("alarms.clear")
    api_client.command("alarms.setProjectSounds", {"sounds": {"sequence": "A-4"}})
    api_client.command("notifications.clearAll")
    time.sleep(0.15)


def _finding_codes(api_client) -> list:
    problems = api_client.command("problems.run")
    return [f.get("code") for f in problems.get("findings", [])]


def _findings(api_client) -> list:
    return api_client.command("problems.run").get("findings", [])


def _load_band_project(api_client, datasets=None, transforms=None) -> None:
    """Builds a one-group network project; datasets is a list of titles (frame
    index 1..N), transforms an optional {index: (code, language)} map."""
    datasets = datasets or ["EGT"]
    transforms = transforms or {}

    api_client.create_new_project(title="Hardening")
    time.sleep(0.3)
    api_client.command("project.group.add", {"title": "Engine", "widgetType": 0})
    time.sleep(0.2)
    for i, title in enumerate(datasets):
        api_client.command("project.dataset.add", {"groupId": 0, "options": 0})
        time.sleep(0.1)
        fields = dict(
            title=title,
            widgetMin=0,
            widgetMax=1000,
            alarmBands=[
                {"min": 0, "max": 800, "severity": 1, "label": "Normal"},
                {"min": 800, "max": 1000, "severity": 3, "label": "Redline"},
            ],
        )
        if i + 1 in transforms:
            code, language = transforms[i + 1]
            fields["transformCode"] = code
            fields["transformLanguage"] = language

        api_client.update_dataset(0, i, **fields)
        time.sleep(0.1)

    api_client.command(
        "project.frameParser.setCode",
        {"code": "function parse(frame) { return frame.split(','); }"},
    )
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.2)
    api_client.set_operation_mode("project")
    assert api_client.command("project.activate")["loaded"]


def _connect(api_client, device_simulator) -> None:
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _send(device_simulator, rows, interval=0.1) -> None:
    frames = [
        DataGenerator.wrap_frame(row, mode="project", checksum_type=ChecksumType.NONE)
        for row in rows
    ]
    device_simulator.send_frames(frames, interval_seconds=interval)


# ---------------------------------------------------------------------------
# R2 / AC2 — locale-independent alarm participation
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
def test_alarms_channel_post_raises_a_point(api_client, clean_state):
    _fresh(api_client)
    api_client.command(
        "notifications.post",
        {"level": 2, "channel": "Alarms", "title": "User alarm", "subtitle": "boom"},
    )
    time.sleep(0.2)

    point = _point(_state(api_client), "User alarm")
    assert point.get("kind") == "notification"
    assert point.get("state") == "alert"


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_band_alarm_yields_exactly_one_point(api_client, device_simulator, clean_state):
    _fresh(api_client)
    _load_band_project(api_client)
    _connect(api_client, device_simulator)

    _send(device_simulator, ["500", "900", "950"])
    time.sleep(1.5)

    state = _state(api_client)
    matching = [p for p in state["points"] if p.get("title") == "EGT"]
    assert len(matching) == 1
    assert matching[0]["kind"] == "band"


# ---------------------------------------------------------------------------
# R3 / AC3 — acknowledgements survive an unrequested drop
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
@pytest.mark.slow
def test_acknowledged_point_survives_link_drop(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client)
    _connect(api_client, device_simulator)

    _send(device_simulator, ["900", "950", "960"])
    time.sleep(1.5)
    assert _point(_state(api_client), "EGT").get("state") == "alert"

    api_client.command("alarms.acknowledge")
    time.sleep(0.2)
    assert _point(_state(api_client), "EGT").get("state") == "acknowledged"

    device_simulator.stop()
    time.sleep(1.5)

    held = _state(api_client)
    assert _point(held, "EGT").get("state") == "acknowledged"
    assert held["sounding"] is None

    device_simulator.start()
    _connect(api_client, device_simulator)
    _send(device_simulator, ["955", "958", "960"])
    time.sleep(1.5)

    state = _state(api_client)
    assert _point(state, "EGT").get("state") == "acknowledged"
    assert state["sounding"] is None


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
@pytest.mark.slow
def test_unacknowledged_point_resumes_alerting_after_drop(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client)
    _connect(api_client, device_simulator)

    _send(device_simulator, ["900", "950"])
    time.sleep(1.5)
    assert _point(_state(api_client), "EGT").get("state") == "alert"

    device_simulator.stop()
    time.sleep(1.5)

    held = _state(api_client)
    assert _point(held, "EGT").get("state") == "alert"
    assert held["sounding"] is None

    device_simulator.start()
    _connect(api_client, device_simulator)
    _send(device_simulator, ["955", "958", "960"])
    time.sleep(1.5)

    state = _state(api_client)
    assert _point(state, "EGT").get("state") == "alert"
    assert state["sounding"] is not None


# ---------------------------------------------------------------------------
# R1 / AC1 — transform result hygiene
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
@pytest.mark.parametrize(
    "code,language,reason",
    [
        ("function transform(v) { return 0/0; }", 0, "NaN"),
        ("function transform(v) { }", 0, "no value"),
        ("function transform(v) return 0/0 end", 1, "NaN"),
    ],
)
def test_bad_transform_results_report_a_problem(
    api_client, device_simulator, clean_state, code, language, reason
):
    _fresh(api_client)
    _load_band_project(api_client, transforms={1: (code, language)})
    _connect(api_client, device_simulator)

    _send(device_simulator, ["500", "510", "520"])
    time.sleep(1.5)

    findings = _findings(api_client)
    transform_findings = [f for f in findings if f.get("code") == "transform-errors"]
    assert transform_findings, f"no transform finding for {reason}"
    assert any(reason in f.get("explanation", "") for f in transform_findings)


# ---------------------------------------------------------------------------
# R5 / AC5 — starved datasets are reported
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
@pytest.mark.slow
def test_short_frames_name_starved_datasets_and_clear(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client, datasets=["Alpha", "Beta", "Gamma"])
    _connect(api_client, device_simulator)

    _send(device_simulator, ["1,2", "3,4", "5,6"], interval=0.2)
    time.sleep(4.0)

    for _ in range(4):
        findings = _findings(api_client)
        time.sleep(0.3)

    short = [f for f in findings if f.get("code") == "short-frames"]
    assert short, "short-frames finding did not appear"
    assert "Gamma" in short[0].get("explanation", "")
    assert "Alpha" not in short[0].get("explanation", "")

    _send(device_simulator, ["1,2,3", "4,5,6"], interval=0.2)
    time.sleep(1.5)
    assert "short-frames" not in _finding_codes(api_client)


# ---------------------------------------------------------------------------
# R6 / AC6 — a dead control script is a visible problem
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
@pytest.mark.project
@pytest.mark.network
@pytest.mark.slow
def test_control_script_error_reports_and_clears(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client)
    api_client.command(
        "controlScript.set",
        {
            "code": "function setup() { }\n"
            "function loop() { throw new Error('boom 0088'); }"
        },
    )
    time.sleep(0.2)
    _connect(api_client, device_simulator)
    time.sleep(2.0)

    findings = _findings(api_client)
    stopped = [f for f in findings if f.get("code") == "control-script-stopped"]
    assert stopped, "control-script-stopped finding did not appear"
    assert "boom 0088" in stopped[0].get("explanation", "")

    api_client.command(
        "controlScript.set",
        {"code": "function setup() { }\nfunction loop() { delay(100); }"},
    )
    time.sleep(2.0)
    assert "control-script-stopped" not in _finding_codes(api_client)


# ---------------------------------------------------------------------------
# R7 / AC7 — a silent annunciator is a visible problem
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.project
def test_disabled_annunciator_finding_matches_state(api_client, clean_state):
    _fresh(api_client)
    _load_band_project(api_client)
    time.sleep(0.5)

    enabled = _state(api_client)["enabled"]
    codes = _finding_codes(api_client)
    if enabled:
        assert "alarms.disabled" not in codes
    else:
        assert "alarms.disabled" in codes


@pytest.mark.integration
def test_no_disabled_finding_without_alarm_configuration(api_client, clean_state):
    _fresh(api_client)
    api_client.create_new_project(title="No alarms")
    time.sleep(0.5)
    assert "alarms.disabled" not in _finding_codes(api_client)


# ---------------------------------------------------------------------------
# R4 / AC4 — the console log honors pause
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
@pytest.mark.project
@pytest.mark.network
@pytest.mark.slow
def test_console_log_closes_on_pause_and_reopens_on_resume(
    api_client, device_simulator, clean_state
):
    if not api_client.command_exists("consoleExport.setEnabled"):
        pytest.skip("console export API unavailable in this build")

    _fresh(api_client)
    _load_band_project(api_client)
    api_client.command("consoleExport.setEnabled", {"enabled": True})
    _connect(api_client, device_simulator)

    _send(device_simulator, ["500", "510"], interval=0.2)
    time.sleep(1.0)
    status = api_client.command("consoleExport.getStatus")
    assert status.get("isOpen") or status.get("open"), status

    api_client.command("io.setPaused", {"paused": True})
    time.sleep(1.0)
    status = api_client.command("consoleExport.getStatus")
    assert not (status.get("isOpen") or status.get("open")), status

    api_client.command("io.setPaused", {"paused": False})
    _send(device_simulator, ["520", "530"], interval=0.2)
    time.sleep(1.0)
    status = api_client.command("consoleExport.getStatus")
    assert status.get("isOpen") or status.get("open"), status

    api_client.command("consoleExport.setEnabled", {"enabled": False})
