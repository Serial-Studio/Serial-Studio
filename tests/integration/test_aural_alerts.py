"""
Aural alerts integration tests (spec 0087)

Drives the ISA-18.1 annunciator behind the alarms.* API: notification and band
points, sequences A / M / R with and without option 4, acknowledge vs silence vs reflash, arbitration,
repeat timing, project sound overrides (rejection, missing file, channel map
round-trip) and the clean state after a project load or a disconnect.

The notification-driven cases post through notifications.post, which is a Pro
command; they are marked `pro`. The band case streams a value into a Critical
band through the network simulator.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator

CHANNEL = "Engine"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _state(api_client) -> dict:
    return api_client.command("alarms.state")


def _post(api_client, level: int, title: str, subtitle: str = "") -> None:
    api_client.command(
        "notifications.post",
        {"level": level, "channel": CHANNEL, "title": title, "subtitle": subtitle},
    )
    time.sleep(0.15)


def _resolve(api_client, title: str) -> None:
    api_client.command(
        "notifications.resolve", {"channel": CHANNEL, "title": title, "subtitle": ""}
    )
    time.sleep(0.15)


def _set_sequence(api_client, letter: str) -> None:
    api_client.command("alarms.setProjectSounds", {"sounds": {"sequence": letter}})
    time.sleep(0.15)


def _point(state: dict, title: str) -> dict:
    for point in state.get("points", []):
        if point.get("title") == title:
            return point

    return {}


def _fresh(api_client) -> None:
    api_client.command("alarms.clear")
    _set_sequence(api_client, "A-4")
    api_client.command("notifications.clearAll")
    time.sleep(0.15)


# ---------------------------------------------------------------------------
# AC1 — point lifecycle per sequence
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
def test_critical_notification_is_a_warning_point(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 2, "EGT high", "1050 C")

    state = _state(api_client)
    point = _point(state, "EGT high")
    assert point.get("kind") == "notification"
    assert point.get("priority") == 2
    assert point.get("state") == "alert"
    assert state["unacknowledgedCount"] == 1
    assert state["highestPriority"] == 2
    assert state["alerting"] is True


@pytest.mark.integration
@pytest.mark.pro
def test_sequence_a_acknowledge_then_resolve_drops_point(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 2, "EGT high")
    api_client.command("alarms.acknowledge")
    time.sleep(0.1)
    assert _point(_state(api_client), "EGT high").get("state") == "acknowledged"

    _resolve(api_client, "EGT high")
    state = _state(api_client)
    assert _point(state, "EGT high") == {}
    assert state["highestPriority"] == -1


@pytest.mark.integration
@pytest.mark.parametrize("letter,locks_in", [("A", True), ("A-4", False)])
def test_option_4_decides_lock_in_before_acknowledge(
    api_client, clean_state, letter, locks_in
):
    _fresh(api_client)
    _set_sequence(api_client, letter)
    assert _state(api_client)["sequence"] == letter

    _post(api_client, 3, "Oil pressure low")
    _resolve(api_client, "Oil pressure low")

    state = _state(api_client)
    point = _point(state, "Oil pressure low")
    if locks_in:
        assert point.get("state") == "alert"
        assert state["sounding"] is not None
        api_client.command("alarms.acknowledge")
        time.sleep(0.1)
    else:
        assert point == {}
        assert state["sounding"] is None

    assert _point(_state(api_client), "Oil pressure low") == {}


@pytest.mark.integration
@pytest.mark.pro
@pytest.mark.parametrize("letter", ["M", "M-4", "R", "R-4"])
def test_manual_sequences_park_in_return_to_normal_until_reset(
    api_client, clean_state, letter
):
    _fresh(api_client)
    _set_sequence(api_client, letter)
    assert _state(api_client)["sequence"] == letter

    _post(api_client, 2, "EGT high")
    api_client.command("alarms.acknowledge")
    time.sleep(0.1)
    _resolve(api_client, "EGT high")

    state = _state(api_client)
    assert _point(state, "EGT high").get("state") == "returnToNormal"
    assert state["ringbackPending"] is (letter in ("R", "R-4"))

    api_client.command("alarms.reset")
    time.sleep(0.1)
    assert _point(_state(api_client), "EGT high") == {}


@pytest.mark.integration
@pytest.mark.pro
def test_warning_notification_is_a_caution_point(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 1, "Battery low")
    point = _point(_state(api_client), "Battery low")
    assert point.get("priority") == 1
    assert point.get("state") == "alert"


# ---------------------------------------------------------------------------
# AC2 — arbitration and silence / reflash
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
def test_highest_unsilenced_priority_sounds(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 1, "Battery low")
    _post(api_client, 2, "EGT high")
    sounding = _state(api_client)["sounding"]
    assert sounding is not None
    assert sounding["priority"] == 2

    api_client.command("alarms.silence")
    time.sleep(0.1)
    assert _state(api_client)["sounding"] is None
    assert _state(api_client)["unacknowledgedCount"] == 2

    _post(api_client, 1, "Oil pressure")
    sounding = _state(api_client)["sounding"]
    assert sounding is not None
    assert sounding["priority"] == 1


# ---------------------------------------------------------------------------
# AC4 — repetition
# ---------------------------------------------------------------------------


@pytest.mark.integration
@pytest.mark.pro
@pytest.mark.slow
def test_warning_burst_repeats_at_interval(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 2, "EGT high")
    first = _state(api_client)["sounding"]
    assert first["burstCount"] == 1

    time.sleep(5.5)
    later = _state(api_client)["sounding"]
    assert later["burstCount"] >= 2
    assert later["burstStartedMs"] - first["burstStartedMs"] >= 4500


@pytest.mark.integration
@pytest.mark.pro
def test_info_notification_is_a_single_advisory_burst(api_client, clean_state):
    _fresh(api_client)
    _post(api_client, 0, "Transform loaded")
    state = _state(api_client)
    assert state["points"] == []
    sounding = state["sounding"]
    if sounding is not None:
        assert sounding["priority"] == 0
        assert sounding["burstCount"] == 1


# ---------------------------------------------------------------------------
# AC5 / AC11 — project overrides
# ---------------------------------------------------------------------------


@pytest.mark.integration
def test_non_wav_override_is_rejected_but_stored(api_client, clean_state, tmp_path):
    _fresh(api_client)
    bogus = tmp_path / "horn.txt"
    bogus.write_text("not audio")
    result = api_client.command(
        "alarms.setProjectSounds",
        {"sounds": {"channels": {CHANNEL: {"warning": str(bogus)}}}},
    )
    assert result["ok"] is True
    assert len(result["rejected"]) >= 1
    assert all(r["reason"] for r in result["rejected"])
    assert any(r["path"] == str(bogus) for r in result["rejected"])


@pytest.mark.integration
@pytest.mark.pro
def test_missing_override_reports_a_problem_and_still_alerts(
    api_client, clean_state, tmp_path
):
    _fresh(api_client)
    missing = str(tmp_path / "gone.wav")
    result = api_client.command(
        "alarms.setProjectSounds",
        {"sounds": {"channels": {CHANNEL: {"warning": missing}}}},
    )
    if any("allowed roots" in r["reason"] for r in result["rejected"]):
        pytest.skip(
            "temp dir is outside SERIAL_STUDIO_API_ALLOWED_PATHS; policy stripped the path"
        )

    _post(api_client, 2, "EGT high")
    time.sleep(0.5)

    assert _point(_state(api_client), "EGT high").get("state") == "alert"
    problems = api_client.command("problems.run")
    codes = [f.get("code") for f in problems.get("findings", [])]
    assert "alarms.sound-file" in codes


@pytest.mark.integration
def test_channel_map_round_trips_through_project_json(api_client, clean_state):
    _fresh(api_client)
    sounds = {
        "sequence": "M",
        "channels": {
            CHANNEL: {"warning": "sounds/horn.wav", "caution": "sounds/chime.wav"}
        },
    }
    api_client.command("alarms.setProjectSounds", {"sounds": sounds})
    time.sleep(0.2)

    exported = api_client.command("project.exportJson")["config"]
    assert exported.get("sounds") == sounds

    api_client.create_new_project(title="Round trip")
    time.sleep(0.3)
    assert api_client.command("alarms.getProjectSounds") == {}

    api_client.load_project_from_json(exported)
    time.sleep(0.5)
    assert api_client.command("alarms.getProjectSounds") == sounds
    assert _state(api_client)["sequence"] == "M"


# ---------------------------------------------------------------------------
# AC1 (band path) and AC9 — live data and clean state
# ---------------------------------------------------------------------------


GROUP_DATA_GRID = 0
GROUP_BAR_PANEL = 10

BANDS = [
    {"min": 0, "max": 800, "severity": 1, "label": "Normal"},
    {"min": 800, "max": 1000, "severity": 3, "label": "Redline"},
]


def _load_band_project(
    api_client, widget="gauge", hidden=False, bar_panel=False, suppressed=False
) -> None:
    api_client.create_new_project(title="Alarm bands")
    time.sleep(0.3)
    api_client.command(
        "project.group.add",
        {
            "title": "Engine",
            "widgetType": GROUP_BAR_PANEL if bar_panel else GROUP_DATA_GRID,
        },
    )
    time.sleep(0.2)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 0})
    time.sleep(0.1)
    fields = dict(
        title="EGT",
        widget=widget,
        hideOnDashboard=hidden,
        widgetMin=0,
        widgetMax=1000,
        alarmBands=BANDS,
    )
    if suppressed:
        fields["suppressAlarms"] = True

    api_client.update_dataset(0, 0, **fields)
    time.sleep(0.2)
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


def _send(device_simulator, values, interval=0.1) -> None:
    frames = [
        DataGenerator.wrap_frame(
            str(v), mode="project", checksum_type=ChecksumType.NONE
        )
        for v in values
    ]
    device_simulator.send_frames(frames, interval_seconds=interval)


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_project_load_leaves_no_points(api_client, clean_state):
    _fresh(api_client)
    _load_band_project(api_client)
    state = _state(api_client)
    assert state["points"] == []
    assert state["sounding"] is None


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_band_entry_raises_and_disconnect_clears(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _set_sequence(api_client, "R")
    _load_band_project(api_client)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    _send(device_simulator, [500.0, 520.0, 900.0, 950.0])
    time.sleep(1.5)

    state = _state(api_client)
    point = _point(state, "EGT")
    assert point.get("kind") == "band"
    assert point.get("priority") == 2
    assert point.get("state") == "alert"

    api_client.command("alarms.acknowledge")
    time.sleep(0.1)
    api_client.disconnect_device()
    time.sleep(0.5)

    state = _state(api_client)
    assert state["points"] == []
    assert state["ringbackPending"] is False
    assert state["sounding"] is None


# ---------------------------------------------------------------------------
# Spec 0093 — alarm-band applicability
# ---------------------------------------------------------------------------


def _band_run(api_client, device_simulator, values=(500.0, 520.0, 900.0, 950.0)):
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    _send(device_simulator, list(values))
    time.sleep(1.5)


def _band_events(api_client) -> list:
    if not api_client.command_exists("notifications.list"):
        return []

    events = api_client.command("notifications.list").get("events", [])
    return [e for e in events if e.get("title") == "EGT"]


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_band_without_widget_raises_no_point(api_client, device_simulator, clean_state):
    _fresh(api_client)
    _load_band_project(api_client, widget="")
    _band_run(api_client, device_simulator)

    assert _point(_state(api_client), "EGT") == {}
    assert _band_events(api_client) == []


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_bar_panel_member_without_widget_raises_point(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client, widget="", bar_panel=True)
    _band_run(api_client, device_simulator)

    point = _point(_state(api_client), "EGT")
    assert point.get("kind") == "band"
    assert point.get("priority") == 2


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_hidden_gauge_raises_no_point(api_client, device_simulator, clean_state):
    _fresh(api_client)
    _load_band_project(api_client, widget="gauge", hidden=True)
    _band_run(api_client, device_simulator)

    assert _point(_state(api_client), "EGT") == {}


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_removing_and_restoring_the_widget_toggles_the_point(
    api_client, device_simulator, clean_state
):
    _fresh(api_client)
    _load_band_project(api_client)
    _band_run(api_client, device_simulator)
    assert _point(_state(api_client), "EGT").get("kind") == "band"

    api_client.update_dataset(0, 0, widget="")
    time.sleep(0.5)
    _send(device_simulator, [900.0, 950.0, 960.0])
    time.sleep(1.5)
    assert _point(_state(api_client), "EGT") == {}

    api_client.update_dataset(0, 0, widget="gauge")
    time.sleep(0.5)
    _send(device_simulator, [900.0, 950.0, 960.0])
    time.sleep(1.5)
    assert _point(_state(api_client), "EGT").get("kind") == "band"


@pytest.mark.integration
@pytest.mark.project
def test_bands_on_widgetless_dataset_survive_export(api_client, clean_state):
    _fresh(api_client)
    _load_band_project(api_client, widget="")

    exported = api_client.command("project.exportJson")["config"]
    datasets = [d for g in exported.get("groups", []) for d in g.get("datasets", [])]
    egt = [d for d in datasets if d.get("title") == "EGT"]
    assert len(egt) == 1
    assert len(egt[0].get("alarmBands", [])) == len(BANDS)


# ---------------------------------------------------------------------------
# Spec 0093 amendment 1 — per-dataset alarm suppression
# ---------------------------------------------------------------------------


def _datasets_of(config: dict) -> list:
    return [d for g in config.get("groups", []) for d in g.get("datasets", [])]


def _egt_of(config: dict) -> dict:
    egt = [d for d in _datasets_of(config) if d.get("title") == "EGT"]
    assert len(egt) == 1
    return egt[0]


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_suppressed_gauge_raises_no_point(api_client, device_simulator, clean_state):
    _fresh(api_client)
    _load_band_project(api_client, suppressed=True)
    _band_run(api_client, device_simulator)

    assert _point(_state(api_client), "EGT") == {}
    assert _band_events(api_client) == []

    api_client.update_dataset(0, 0, suppressAlarms=False)
    time.sleep(0.5)
    _send(device_simulator, [900.0, 950.0, 960.0])
    time.sleep(1.5)

    point = _point(_state(api_client), "EGT")
    assert point.get("kind") == "band"
    assert point.get("priority") == 2


@pytest.mark.integration
@pytest.mark.project
def test_suppress_alarms_round_trips_through_export(api_client, clean_state):
    _fresh(api_client)
    _load_band_project(api_client, suppressed=True)

    exported = api_client.command("project.exportJson")["config"]
    assert _egt_of(exported).get("suppressAlarms") is True

    api_client.create_new_project(title="Round trip")
    time.sleep(0.3)
    api_client.load_project_from_json(exported)
    time.sleep(0.5)

    reloaded = api_client.command("project.exportJson")["config"]
    assert _egt_of(reloaded).get("suppressAlarms") is True

    _load_band_project(api_client)
    plain = api_client.command("project.exportJson")["config"]
    assert "alarmBands" in _egt_of(plain)
    assert all("suppressAlarms" not in d for d in _datasets_of(plain))
