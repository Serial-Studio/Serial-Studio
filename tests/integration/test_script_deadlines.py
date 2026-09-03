"""
Script Execution Deadline Tests (spec 0075, WP-B)

Every API-reachable surface that compiles or runs user script code must return
an error when the code never finishes, and the app must still answer commands
afterwards. Before spec 0075 the painter / output-widget dry runs and the
editor validate paths ran user code in a bare QJSEngine with no watchdog, so
`{"code": "while(true){}"}` froze the whole application permanently.

Covered here:
 * controlScript.dryRun (JS)
 * project.painter.dryRun, project.outputWidget.dryRun (compile + sample run)
 * project.dataset.transform.dryRun (JS and Lua)
 * project.frameParser.dryRun / dryCompile (JS and Lua)
 * the frame-lane dataset transform, end to end through a loopback device

The GUI editors (Validate / Test buttons) share the same ScriptDryRun helper
as the dry-run commands, so the commands are the reachable proxy for them; the
stream-lane transform case lives with WP-A's tst_stream_worker.

Each test asserts a live command *after* the timeout: the deadline must free
the thread, not merely fail one call.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import APIError

# ---------------------------------------------------------------------------
# Fixtures / helpers
# ---------------------------------------------------------------------------

JS_INFINITE_LOOP = "var i = 0; while (true) { i = (i + 1) % 1000003; }"

LUA_INFINITE_LOOP = "local i = 0 while true do i = (i + 1) % 1000003 end"

JS_LOOPING_TRANSFORM = "function transform(v) { while (true) {} }"

LUA_LOOPING_TRANSFORM = "function transform(v) while true do end end"

JS_PASSTHROUGH = "function parse(frame) { return frame.split(','); }"

# The dry-run budget is 2 s (DataModel::kScriptDryRunBudgetMs); allow generous
# slack for a loaded CI runner while still failing fast on a real hang. The
# 30 s pytest-timeout in pytest.ini is the backstop.
MAX_DEADLINE_SECONDS = 12.0


def _assert_alive(api_client) -> None:
    """The app answers a cheap command after the deadline fired."""
    assert "running" in api_client.command("controlScript.getStatus")


def _timed(fn, *args, **kwargs):
    """Runs fn and returns (result_or_exception, elapsed_seconds)."""
    started = time.time()
    try:
        return fn(*args, **kwargs), time.time() - started
    except APIError as error:
        return error, time.time() - started


# ---------------------------------------------------------------------------
# Control script
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_control_script_dry_run_times_out(api_client, clean_state):
    """A control script looping at the top level reports a timeout, not a hang."""
    result, elapsed = _timed(
        api_client.command, "controlScript.dryRun", {"code": JS_INFINITE_LOOP}
    )

    assert not isinstance(result, APIError)
    assert elapsed < MAX_DEADLINE_SECONDS
    assert result.get("valid") is False
    assert "did not finish" in result.get("error", "")
    _assert_alive(api_client)


# ---------------------------------------------------------------------------
# Painter and output widget dry runs
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_painter_dry_run_times_out(api_client, clean_state):
    """project.painter.dryRun returns SCRIPT_TIMEOUT for a runaway program."""
    result, elapsed = _timed(
        api_client.command, "project.painter.dryRun", {"code": JS_INFINITE_LOOP}
    )

    assert isinstance(result, APIError)
    assert result.code == "SCRIPT_TIMEOUT"
    assert elapsed < MAX_DEADLINE_SECONDS
    _assert_alive(api_client)


@pytest.mark.project
def test_output_widget_dry_run_compile_times_out(api_client, clean_state):
    """A transmit script looping at the top level is cut off at compile time."""
    result, elapsed = _timed(
        api_client.command, "project.outputWidget.dryRun", {"code": JS_INFINITE_LOOP}
    )

    assert isinstance(result, APIError)
    assert result.code == "SCRIPT_TIMEOUT"
    assert elapsed < MAX_DEADLINE_SECONDS
    _assert_alive(api_client)


@pytest.mark.project
def test_output_widget_sample_run_times_out(api_client, clean_state):
    """transmit() itself looping is reported per sample run, dialog-style."""
    result, elapsed = _timed(
        api_client.command,
        "project.outputWidget.dryRun",
        {"code": "function transmit(v) { while (true) {} }", "inputValue": "1"},
    )

    assert not isinstance(result, APIError)
    assert elapsed < MAX_DEADLINE_SECONDS
    sample = result.get("sampleRun", {})
    assert sample.get("ok") is False
    assert sample.get("timedOut") is True
    _assert_alive(api_client)


# ---------------------------------------------------------------------------
# Transform and frame-parser dry runs (engine-level watchdogs)
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.parametrize(
    "language,code",
    [(0, JS_LOOPING_TRANSFORM), (1, LUA_LOOPING_TRANSFORM)],
    ids=["javascript", "lua"],
)
def test_transform_dry_run_times_out(api_client, clean_state, language, code):
    """project.dataset.transform.dryRun survives a looping transform()."""
    result, elapsed = _timed(
        api_client.command,
        "project.dataset.transform.dryRun",
        {"code": code, "language": language, "values": [1, 2]},
    )

    assert elapsed < MAX_DEADLINE_SECONDS
    if not isinstance(result, APIError):
        assert all(value is None for value in result.get("outputs", []))

    _assert_alive(api_client)


@pytest.mark.project
@pytest.mark.parametrize(
    "language,code",
    [
        (0, "function parse(frame) { while (true) {} }"),
        (1, "function parse(frame) while true do end end"),
    ],
    ids=["javascript", "lua"],
)
def test_frame_parser_dry_run_times_out(api_client, clean_state, language, code):
    """project.frameParser.dryRun survives a parser that never returns.

    frameEnd is explicit: with the default (no end delimiter) nothing is
    extracted and parse() would never run, so the test would pass vacuously.
    """
    result, elapsed = _timed(
        api_client.command,
        "project.frameParser.dryRun",
        {
            "code": code,
            "language": language,
            "inputBytes": "1,2,3\n",
            "frameEnd": "\n",
        },
    )

    assert elapsed < MAX_DEADLINE_SECONDS
    assert result is not None
    _assert_alive(api_client)


# ---------------------------------------------------------------------------
# Frame lane, end to end
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_frame_lane_transform_timeout_keeps_streaming(
    api_client, device_simulator, clean_state
):
    """A looping dataset transform falls back to the raw value, frame after frame."""
    api_client.create_new_project()
    time.sleep(0.2)

    group_id = api_client.add_group("G", widget_type=0)
    api_client.add_dataset(group_id, options=1)
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )
    api_client.set_frame_parser_code(JS_PASSTHROUGH, language=0, source_id=0)
    api_client.command(
        "project.dataset.setTransformCode",
        {"groupId": 0, "datasetId": 0, "code": JS_LOOPING_TRANSFORM},
    )
    time.sleep(0.2)

    assert api_client.command("project.activate").get("loaded")
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    device_simulator.send_frames([b"/*42*/"], interval_seconds=0.1)
    time.sleep(1.5)

    data = api_client.get_dashboard_data()
    api_client.disconnect_device()
    time.sleep(0.2)

    datasets = data.get("frame", {}).get("groups", [{}])[0].get("datasets", [])
    assert datasets, "no datasets in the published frame"
    assert datasets[0].get("numericValue") == pytest.approx(42.0)
    _assert_alive(api_client)
