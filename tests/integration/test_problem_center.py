"""
Problem Center Integration Tests (spec 0033)

Covers the project, link, and script diagnostic checkers behind the
problems.* API: standing findings that replace themselves per run, clear
by themselves once the cause is fixed, carry a jump target, and report
link/script conditions from the 1 Hz counter sample.

The link tests need at least three 1 Hz samples before a sustained
condition is reported, so they stream in one-second rounds and re-run the
checkers between rounds instead of sleeping blind.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import ChecksumType, DataGenerator

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

PROJECT_CHECKERS = (
    "project.frame-index",
    "project.empty-group",
    "project.reference",
    "project.numeric-range",
    "project.alias",
)


def _run(api_client) -> dict:
    """Force a re-run of every checker and return the refreshed payload."""
    return api_client.command("problems.run")


def _list(api_client, **params) -> dict:
    return api_client.command("problems.list", params or None)


def _findings(result: dict, code: str = None, checker_id: str = None) -> list:
    rows = result.get("findings", [])
    if code is not None:
        rows = [f for f in rows if f.get("code") == code]
    if checker_id is not None:
        rows = [f for f in rows if f.get("checkerId") == checker_id]
    return rows


def _codes(result: dict) -> list:
    return [f.get("code") for f in result.get("findings", [])]


def _project_findings(result: dict) -> list:
    """Only the project-schema slice, so link/script noise cannot flap a compare."""
    return [
        f for f in result.get("findings", []) if f.get("checkerId") in PROJECT_CHECKERS
    ]


def _one(result: dict, code: str) -> dict:
    rows = _findings(result, code=code)
    assert (
        len(rows) == 1
    ), f"Expected exactly one '{code}' finding, got {len(rows)}: {_codes(result)}"
    return rows[0]


def _configure_csv_project(
    api_client,
    dataset_count: int = 3,
    checksum: str = "",
    frame_detection: int = 1,
) -> int:
    """Build a minimal ProjectFile-mode CSV project and push it into FrameBuilder."""
    gid = api_client.add_group("Telemetry")
    for i in range(dataset_count):
        api_client.add_dataset(gid)

    for i, ds in enumerate(api_client.list_datasets()):
        api_client.update_dataset(
            ds["groupId"], ds["datasetId"], title=f"Channel {i}", index=i + 1
        )

    api_client.set_frame_parser_code(DataGenerator.CSV_PARSER_TEMPLATE, language=0)
    time.sleep(0.2)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm=checksum,
        operation_mode=0,
    )
    time.sleep(0.1)
    api_client.configure_frame_parser(frame_detection=frame_detection, operation_mode=0)
    time.sleep(0.2)

    result = api_client.command("project.activate")
    assert result["loaded"], "Project should have loaded into FrameBuilder"
    time.sleep(0.2)
    return gid


def _valid_frames(count: int, checksum_type: ChecksumType = ChecksumType.NONE) -> list:
    frames = []
    for i in range(count):
        payload = DataGenerator.generate_csv_frame(values=[1.0 + i, 2.0 + i, 3.0 + i])
        frames.append(
            DataGenerator.wrap_frame(
                payload,
                start_delimiter="/*",
                end_delimiter="*/",
                checksum_type=checksum_type,
                mode="project",
            )
        )
    return frames


def _corrupt_frames(
    count: int, checksum_type: ChecksumType, checksum_length: int
) -> list:
    """Well-formed frames whose trailing checksum bytes are zeroed out."""
    frames = []
    for frame in _valid_frames(count, checksum_type):
        start = len(frame) - len(b"\n") - checksum_length
        frames.append(frame[:start] + b"\x00" * checksum_length + b"\n")
    return frames


def _stream_rounds(
    api_client,
    device_simulator,
    frames: list,
    rounds: int,
    code: str = None,
    expect: bool = True,
) -> dict:
    """
    Send `frames` once per ~1 s round, re-running the checkers between rounds.

    Returns the last payload. Stops early once `code` matches `expect`, so a
    passing test does not pay for every round.
    """
    result = {}
    for _ in range(rounds):
        device_simulator.send_frames(frames, interval_seconds=1.0 / max(len(frames), 1))
        time.sleep(0.2)
        result = _run(api_client)
        if code is not None and (code in _codes(result)) == expect:
            return result

    return result


# ---------------------------------------------------------------------------
# AC2 -- standing project findings
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_duplicate_frame_index_reported_once(api_client, clean_state):
    """Two datasets on one frame index yield one finding that does not accumulate."""
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    datasets = api_client.list_datasets()
    assert len(datasets) == 2
    for i, ds in enumerate(datasets):
        api_client.update_dataset(
            ds["groupId"], ds["datasetId"], title=f"Channel {i}", index=1
        )

    time.sleep(0.3)
    result = _run(api_client)
    finding = _one(result, "duplicate-frame-index")

    # plan.md picks Warning over spec.md's Error: two datasets legitimately
    # share an index when one value drives two widgets.
    assert finding["severity"] == "warning"
    assert finding["checkerId"] == "project.frame-index"
    assert "Channel 0" in finding["explanation"]
    assert "Channel 1" in finding["explanation"]
    assert finding["remedy"]
    assert finding["jump"] == "dataset"
    assert finding["entityUniqueId"] == datasets[1]["uniqueId"]

    # R4: re-running with the condition unchanged must not duplicate it.
    again = _run(api_client)
    assert _project_findings(again) == _project_findings(result)


@pytest.mark.project
def test_finding_clears_when_condition_fixed(api_client, clean_state):
    """A fixed project problem disappears without any user action."""
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    for i, ds in enumerate(api_client.list_datasets()):
        api_client.update_dataset(
            ds["groupId"], ds["datasetId"], title=f"Channel {i}", index=1
        )

    time.sleep(0.3)
    assert "duplicate-frame-index" in _codes(_run(api_client))

    second = api_client.list_datasets()[1]
    api_client.update_dataset(second["groupId"], second["datasetId"], index=2)
    time.sleep(0.3)

    assert "duplicate-frame-index" not in _codes(_run(api_client))


# ---------------------------------------------------------------------------
# AC3 -- references, empty groups, inverted ranges
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_dangling_xaxis_reference_has_jump_target(api_client, clean_state):
    """An X-axis pointing at a deleted dataset is an error that jumps to the plot."""
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)
    api_client.add_dataset(gid)

    datasets = api_client.list_datasets()
    x_source, plotted = datasets[0], datasets[1]
    api_client.update_dataset(
        x_source["groupId"], x_source["datasetId"], title="X Source", index=1
    )
    api_client.update_dataset(
        plotted["groupId"],
        plotted["datasetId"],
        title="Plotted",
        index=2,
        graph=True,
        xAxisId=x_source["uniqueId"],
    )
    time.sleep(0.3)
    assert "dangling-x-axis" not in _codes(_run(api_client))

    api_client.delete_dataset(x_source["groupId"], x_source["datasetId"])
    time.sleep(0.3)

    finding = _one(_run(api_client), "dangling-x-axis")
    assert finding["severity"] == "error"
    assert finding["checkerId"] == "project.reference"
    assert finding["jump"] == "dataset"
    assert finding["entityUniqueId"] == plotted["uniqueId"]


@pytest.mark.project
def test_empty_group_and_inverted_range_are_warnings(api_client, clean_state):
    """An empty group and a min-above-max plot range both report as warnings."""
    api_client.add_group("Empty Panel")
    gid = api_client.add_group("Telemetry")
    api_client.add_dataset(gid)

    dataset = api_client.list_datasets()[-1]
    api_client.update_dataset(
        dataset["groupId"],
        dataset["datasetId"],
        title="Altitude",
        index=1,
        graph=True,
        pltMin=100.0,
        pltMax=1.0,
    )
    time.sleep(0.3)

    result = _run(api_client)

    empty = _one(result, "empty-group")
    assert empty["severity"] == "warning"
    assert empty["checkerId"] == "project.empty-group"
    assert empty["jump"] == "group"
    assert "Empty Panel" in empty["explanation"]

    inverted = _one(result, "inverted-plot-range")
    assert inverted["severity"] == "warning"
    assert inverted["checkerId"] == "project.numeric-range"
    assert inverted["entityUniqueId"] == dataset["uniqueId"]

    assert result["counts"]["warning"] >= 2


# ---------------------------------------------------------------------------
# AC4 -- link: bytes in, no frames out
# ---------------------------------------------------------------------------


@pytest.mark.network
@pytest.mark.slow
def test_delimiter_mismatch_reports_no_frames_extracted(
    api_client, device_simulator, clean_state
):
    """Bytes that never match the delimiters raise a standing link finding."""
    _configure_csv_project(api_client)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    junk = [b"1.0,2.0,3.0;" for _ in range(10)]
    result = _stream_rounds(
        api_client, device_simulator, junk, rounds=8, code="bytes-without-frames"
    )

    finding = _one(result, "bytes-without-frames")
    assert finding["severity"] == "error"
    assert finding["checkerId"] == "link.statistics"
    assert finding["remedy"]

    # Correct frames clear the sustained window on the next sample.
    cleared = _stream_rounds(
        api_client,
        device_simulator,
        _valid_frames(10),
        rounds=4,
        code="bytes-without-frames",
        expect=False,
    )
    assert "bytes-without-frames" not in _codes(cleared)

    api_client.disconnect_device()


# ---------------------------------------------------------------------------
# AC5 -- link: checksum failure rate
# ---------------------------------------------------------------------------


@pytest.mark.network
@pytest.mark.slow
def test_checksum_failure_rate_reported_and_cleared(
    api_client, device_simulator, clean_state
):
    """A corrupt stream reports a checksum-failure rate; a fresh link clears it."""
    _configure_csv_project(api_client, checksum="CRC-16")

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    corrupt = _corrupt_frames(15, ChecksumType.CRC16, checksum_length=2)
    result = _stream_rounds(
        api_client, device_simulator, corrupt, rounds=6, code="checksum-failures"
    )

    finding = _one(result, "checksum-failures")
    assert finding["severity"] == "warning"
    assert finding["checkerId"] == "link.statistics"
    assert finding["explanation"], "The finding must report the rate that triggered it"

    # The rate is accumulated for the life of the reader, so it clears when the
    # link is reopened and correct frames flow -- not by diluting the old total.
    api_client.disconnect_device()
    time.sleep(1.0)
    assert "checksum-failures" not in _codes(_run(api_client))

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not reconnect"
    cleared = _stream_rounds(
        api_client,
        device_simulator,
        _valid_frames(15, ChecksumType.CRC16),
        rounds=4,
        code="checksum-failures",
        expect=False,
    )
    assert "checksum-failures" not in _codes(cleared)

    api_client.disconnect_device()


# ---------------------------------------------------------------------------
# AC6 -- script: failing per-dataset transform
# ---------------------------------------------------------------------------


@pytest.mark.network
@pytest.mark.slow
def test_failing_transform_reports_error_text_and_count(
    api_client, device_simulator, clean_state
):
    """A transform that always throws reports its message and a repeat count."""
    _configure_csv_project(api_client)

    dataset = api_client.list_datasets()[0]
    api_client.update_dataset(
        dataset["groupId"],
        dataset["datasetId"],
        transformCode="function transform(value) { throw new Error('transform blew up'); }",
        transformLanguage=0,
    )
    time.sleep(0.2)
    assert api_client.command("project.activate")["loaded"]
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), "Device did not connect"

    result = _stream_rounds(
        api_client,
        device_simulator,
        _valid_frames(10),
        rounds=4,
        code="transform-errors",
    )

    finding = _one(result, "transform-errors")
    assert finding["severity"] == "warning"
    assert finding["checkerId"] == "script.transform"
    assert "transform blew up" in finding["explanation"]
    assert (
        "once" not in finding["explanation"]
    ), "Repeated failures must not read as a single one"
    assert finding["jump"] == "dataset"
    assert finding["entityUniqueId"] == dataset["uniqueId"]

    api_client.disconnect_device()


# ---------------------------------------------------------------------------
# AC7 -- the API surface itself
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_problems_run_refreshes_list(api_client, clean_state):
    """problems.run refreshes what problems.list returns, with a stable shape."""
    checkers = api_client.command("problems.listCheckers")
    ids = [c["id"] for c in checkers["checkers"]]
    for expected in PROJECT_CHECKERS + (
        "link.statistics",
        "script.parser",
        "script.transform",
    ):
        assert expected in ids, f"Checker {expected} is not registered"

    assert checkers["total"] == len(checkers["checkers"])
    for checker in checkers["checkers"]:
        assert checker["triggers"], "Every checker must declare at least one trigger"

    baseline = _run(api_client)
    assert "empty-group" not in _codes(baseline)

    api_client.add_group("Empty Panel")
    time.sleep(0.3)

    refreshed = _run(api_client)
    assert "empty-group" in _codes(refreshed)

    listed = _list(api_client)
    assert _codes(listed) == _codes(refreshed)
    for key in ("findings", "counts", "total", "matchCount", "lastRun", "hint"):
        assert key in listed, f"problems.list result is missing '{key}'"

    counts = listed["counts"]
    assert set(counts) == {"info", "warning", "error"}
    assert listed["total"] == counts["info"] + counts["warning"] + counts["error"]

    warnings = _list(api_client, severity="warning")
    assert warnings["findings"], "Expected at least the empty-group warning"
    assert all(f["severity"] == "warning" for f in warnings["findings"])

    scoped = _list(api_client, checkerId="project.empty-group")
    assert scoped["findings"]
    assert all(f["checkerId"] == "project.empty-group" for f in scoped["findings"])

    windowed = _list(api_client, limit=1)
    assert len(windowed["findings"]) <= 1
    assert windowed["matchCount"] >= len(windowed["findings"])
