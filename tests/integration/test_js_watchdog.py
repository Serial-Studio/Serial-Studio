"""
JavaScript Watchdog Regression Tests

Guards the bug where the JS frame-parser / transform watchdog was driven by a
QTimer living on the same thread as the blocking QJSValue::call(). Because the
event loop is blocked while the script runs, that timer could never fire, so an
infinite loop in user JavaScript hung the whole application instead of being
interrupted. The interrupt now comes from a dedicated DataModel::JsWatchdogThread
(QJSEngine::setInterrupted is thread-safe), mirroring the Lua engine's behaviour.

These tests would hang the app before the fix; pytest-timeout (30s, see
pytest.ini) turns that hang into a fast failure if the watchdog ever regresses.

What this suite covers:

  1. Frame parser: an infinite loop inside parse() is interrupted at probe time,
     surfacing as an APIError from project.frameParser.setCode within ~1s.
  2. Frame parser: an infinite loop at JavaScript file scope is also interrupted.
  3. Baseline: a well-formed JS parser still loads, proving (1)/(2) detect a real
     interruption rather than a parser that simply always errors.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _install_js_parser(api_client, js_code: str) -> None:
    """Create a ProjectFile project and install js_code as the source-0 parser.

    Passing language=0 (JavaScript) to project.frameParser.setCode triggers
    up-front engine validation, which probes the parse() function. A runaway
    probe is interrupted by the watchdog and raises APIError to the caller.
    """
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.group.add", {"title": "JS Group", "widgetType": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.05)

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
        {"code": js_code, "language": 0, "sourceId": 0},
    )
    time.sleep(0.2)


# ---------------------------------------------------------------------------
# Watchdog — infinite loops must be interrupted, not hang the app
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_js_infinite_loop_in_parse_interrupted(api_client, clean_state):
    """An infinite loop inside parse() must be interrupted within ~1s.

    The watchdog arms before the probe call; when the deadline expires the
    JsWatchdogThread sets the engine interrupted flag from another thread, the
    call returns an error, and setCode reports a failure to the API caller.
    """
    js_code = "function parse(frame) { while (true) {} return [frame]; }"

    start = time.time()
    with pytest.raises(APIError):
        _install_js_parser(api_client, js_code)
    elapsed = time.time() - start

    assert elapsed < 5.0, (
        f"Watchdog did not interrupt the JS parse() loop within 5s "
        f"(took {elapsed:.2f}s)"
    )


@pytest.mark.project
def test_js_infinite_loop_at_file_scope_interrupted(api_client, clean_state):
    """An infinite loop at JS file scope is interrupted during evaluation."""
    js_code = "while (true) {}\nfunction parse(frame) { return [frame]; }"

    start = time.time()
    with pytest.raises(APIError):
        _install_js_parser(api_client, js_code)
    elapsed = time.time() - start

    assert elapsed < 5.0, (
        f"Watchdog did not interrupt the top-level JS loop within 5s "
        f"(took {elapsed:.2f}s)"
    )


@pytest.mark.project
def test_js_valid_parser_still_loads(api_client, clean_state):
    """A well-formed JS parser loads cleanly -- the interruption tests above
    must be catching a real timeout, not a parser that always errors."""
    js_code = "function parse(frame) { return frame.split(','); }"

    # Must NOT raise.
    _install_js_parser(api_client, js_code)

    stored = api_client.command("project.frameParser.getCode", {"sourceId": 0})
    assert "function parse(" in stored.get("code", "")
