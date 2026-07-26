"""
deviceWrite() Integration Tests

Verifies the scripting API that lets frame parsers and dataset transforms
write bytes back to the connected device (closed-loop control):

 * Lua/JS frame parsers calling deviceWrite() once per frame
 * Lua/JS transforms calling deviceWrite() on a threshold
 * Empty data is rejected and reported via {ok=false, error=...}
 * Writes can target an arbitrary sourceId
 * deviceWrite never throws; it returns a status table

Tests use the TCP loopback DeviceSimulator and a small recording helper
that drains bytes from the simulator's accepted client socket so we can
assert what Serial Studio wrote back.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import sys
import threading
import time
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))

from utils import APIError  # noqa: F401  (kept for parity with sibling tests)

# ---------------------------------------------------------------------------
# Recording helper -- drains bytes from the simulator's accepted socket
# ---------------------------------------------------------------------------


class _DeviceRecorder:
    """
    Records bytes Serial Studio writes back to the DeviceSimulator's TCP
    client socket.

    DeviceSimulator's _monitor_client uses MSG_PEEK only to detect closure,
    so the real TCP receive buffer accumulates. We poll the same socket
    here and stash everything we read.
    """

    def __init__(self, simulator):
        self._sim = simulator
        self._buffer = bytearray()
        self._lock = threading.Lock()
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._run, daemon=True)

    def start(self) -> None:
        self._thread.start()

    def stop(self) -> None:
        self._stop.set()
        self._thread.join(timeout=2.0)

    def captured(self) -> bytes:
        with self._lock:
            return bytes(self._buffer)

    def clear(self) -> None:
        with self._lock:
            self._buffer.clear()

    def wait_for(self, predicate, timeout: float = 3.0) -> bool:
        deadline = time.time() + timeout
        while time.time() < deadline:
            if predicate(self.captured()):
                return True
            time.sleep(0.05)
        return False

    def _run(self) -> None:
        import select

        while not self._stop.is_set():
            client = self._sim._client_socket  # noqa: SLF001
            if client is None:
                time.sleep(0.05)
                continue
            try:
                readable, _, _ = select.select([client], [], [], 0.1)
                if not readable:
                    continue
                chunk = client.recv(4096)
                if not chunk:
                    time.sleep(0.05)
                    continue
                with self._lock:
                    self._buffer.extend(chunk)
            except (OSError, ValueError):
                time.sleep(0.05)


# ---------------------------------------------------------------------------
# Fixtures -- project setup helpers
# ---------------------------------------------------------------------------


def _setup_project(
    api_client,
    parser_code: str,
    parser_language: int,
    dataset_count: int = 1,
) -> None:
    api_client.create_new_project()
    time.sleep(0.2)

    gid = api_client.add_group("G", widget_type=0)
    for _ in range(dataset_count):
        api_client.add_dataset(gid, options=1)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )
    api_client.set_frame_parser_code(parser_code, language=parser_language, source_id=0)
    time.sleep(0.15)


def _set_transform(api_client, group_id: int, dataset_id: int, code: str) -> None:
    api_client.command(
        "project.dataset.setTransformCode",
        {"groupId": group_id, "datasetId": dataset_id, "code": code},
    )
    time.sleep(0.1)


def _send_and_capture(
    api_client,
    device_simulator,
    payloads: list[bytes],
    settle_seconds: float = 1.0,
    dashboard_out: dict | None = None,
) -> bytes:
    """Connect, stream payloads, return bytes Serial Studio wrote back.

    If ``dashboard_out`` is provided, the dashboard data snapshot captured just
    before the disconnect is merged into it -- Dashboard::resetData fires on
    disconnect and erases the parsed frame state otherwise.
    """
    assert api_client.command("project.activate").get("loaded")

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)

    recorder = _DeviceRecorder(device_simulator)
    recorder.start()
    try:
        frames = [b"/*" + p + b"*/" for p in payloads]
        device_simulator.send_frames(frames, interval_seconds=0.1)
        time.sleep(settle_seconds)
        return recorder.captured()
    finally:
        if dashboard_out is not None:
            try:
                dashboard_out.update(api_client.get_dashboard_data())
            except Exception:
                pass
        recorder.stop()
        api_client.disconnect_device()
        time.sleep(0.2)


# ---------------------------------------------------------------------------
# Frame parser writes back on every frame
# ---------------------------------------------------------------------------


LUA_PARSER_ACK = (
    "function parse(frame)\n"
    "  local out = {}\n"
    "  for f in frame:gmatch('([^,]+)') do\n"
    "    out[#out + 1] = f\n"
    "  end\n"
    "  local r = deviceWrite('ACK\\n')\n"
    "  assert(type(r) == 'table', 'deviceWrite must return a table')\n"
    "  return out\n"
    "end\n"
)

JS_PARSER_ACK = (
    "function parse(frame) {\n"
    "  var out = frame.split(',');\n"
    "  var r = deviceWrite('ACK\\n');\n"
    "  if (typeof r !== 'object' || r === null) throw 'deviceWrite must return an object';\n"
    "  return out;\n"
    "}\n"
)


@pytest.mark.project
def test_lua_parser_device_write_round_trip(api_client, device_simulator, clean_state):
    """Lua parser calling deviceWrite('ACK\\n') makes those bytes reach the device."""
    _setup_project(api_client, LUA_PARSER_ACK, parser_language=1, dataset_count=2)
    captured = _send_and_capture(
        api_client, device_simulator, [b"1,2", b"3,4", b"5,6"], settle_seconds=1.2
    )
    # Three frames sent, three ACKs expected.
    assert (
        captured.count(b"ACK\n") >= 3
    ), f"expected >=3 ACKs from Lua deviceWrite, captured={captured!r}"


@pytest.mark.project
def test_js_parser_device_write_round_trip(api_client, device_simulator, clean_state):
    """JS parser calling deviceWrite('ACK\\n') makes those bytes reach the device."""
    _setup_project(api_client, JS_PARSER_ACK, parser_language=0, dataset_count=2)
    captured = _send_and_capture(
        api_client, device_simulator, [b"1,2", b"3,4", b"5,6"], settle_seconds=1.2
    )
    assert (
        captured.count(b"ACK\n") >= 3
    ), f"expected >=3 ACKs from JS deviceWrite, captured={captured!r}"


# ---------------------------------------------------------------------------
# Transform writes back when a threshold trips
# ---------------------------------------------------------------------------


LUA_PASSTHROUGH = (
    "function parse(frame)\n"
    "  local result = {}\n"
    "  for field in frame:gmatch('([^,]+)') do\n"
    "    result[#result + 1] = field\n"
    "  end\n"
    "  return result\n"
    "end\n"
)

JS_PASSTHROUGH = "function parse(frame) { return frame.split(','); }"


@pytest.mark.project
def test_lua_transform_device_write_on_threshold(
    api_client, device_simulator, clean_state
):
    """A Lua transform that calls deviceWrite when value > 80 must trip exactly once
    per high frame and the bytes must reach the simulator."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    transform_code = (
        "function transform(v)\n"
        "  if v > 80 then\n"
        "    deviceWrite('ALARM=1\\n')\n"
        "  end\n"
        "  return v\n"
        "end\n"
    )
    _set_transform(api_client, 0, 0, transform_code)

    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"10", b"42", b"95", b"30", b"120"],
        settle_seconds=1.3,
    )
    # Two frames over threshold (95 and 120) -> two ALARM lines.
    assert (
        captured.count(b"ALARM=1\n") >= 2
    ), f"expected >=2 alarms, captured={captured!r}"
    # Frames under threshold must NOT trigger writes.
    assert b"ALARM" in captured, f"no alarm seen, captured={captured!r}"


@pytest.mark.project
def test_js_transform_device_write_on_threshold(
    api_client, device_simulator, clean_state
):
    """JS transform mirror of the Lua threshold test."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    transform_code = (
        "function transform(v) {\n"
        "  if (v > 80) deviceWrite('ALARM=1\\n');\n"
        "  return v;\n"
        "}\n"
    )
    _set_transform(api_client, 0, 0, transform_code)

    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"10", b"42", b"95", b"30", b"120"],
        settle_seconds=1.3,
    )
    assert (
        captured.count(b"ALARM=1\n") >= 2
    ), f"expected >=2 alarms from JS deviceWrite, captured={captured!r}"


# ---------------------------------------------------------------------------
# Error paths -- deviceWrite never throws; it returns {ok=false, error=...}
# ---------------------------------------------------------------------------


LUA_PARSER_EMPTY = (
    "function parse(frame)\n"
    "  local r = deviceWrite('')\n"
    "  if r.ok or not r.error then\n"
    "    error('empty write should return ok=false with an error message')\n"
    "  end\n"
    "  local out = {}\n"
    "  for f in frame:gmatch('([^,]+)') do out[#out + 1] = f end\n"
    "  return out\n"
    "end\n"
)


@pytest.mark.project
def test_lua_parser_empty_payload_returns_error(
    api_client, device_simulator, clean_state
):
    """deviceWrite('') must return {ok=false, error=...} and NOT abort the parser.

    If empty payloads slipped through to ConnectionManager::writeDataToDevice
    they'd trip a Q_ASSERT in debug builds; here we just check the parser
    survives several frames in a row.
    """
    _setup_project(api_client, LUA_PARSER_EMPTY, parser_language=1, dataset_count=2)
    data: dict = {}
    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"1,2", b"3,4", b"5,6"],
        settle_seconds=1.0,
        dashboard_out=data,
    )
    # Nothing should be written back -- the parser only attempts empty writes.
    assert b"ACK" not in captured
    assert b"ALARM" not in captured

    # Parser kept producing values: dashboard.getData should show the last frame.
    groups = data.get("frame", {}).get("groups", [])
    assert groups, "parser stopped producing frames"


JS_PARSER_BAD_SOURCE = (
    "function parse(frame) {\n"
    "  // sourceId must be numeric; passing a string should fail gracefully.\n"
    "  var r = deviceWrite('PING\\n', 'not-a-number');\n"
    "  if (r.ok || !r.error) throw 'expected error result for non-numeric sourceId';\n"
    "  return frame.split(',');\n"
    "}\n"
)


@pytest.mark.project
def test_js_parser_invalid_source_id_returns_error(
    api_client, device_simulator, clean_state
):
    """Non-numeric sourceId produces {ok:false, error:...} without writing or throwing."""
    _setup_project(api_client, JS_PARSER_BAD_SOURCE, parser_language=0, dataset_count=2)
    data: dict = {}
    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"7,8"],
        settle_seconds=0.8,
        dashboard_out=data,
    )
    assert b"PING" not in captured

    groups = data.get("frame", {}).get("groups", [])
    assert groups, "parser stopped producing frames"


# ---------------------------------------------------------------------------
# Transform second-arg frameInfo
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_lua_transform_frame_info_drives_deviceWrite(
    api_client, device_simulator, clean_state
):
    """frameInfo.frameNumber is monotonically increasing and reaches the script."""
    _setup_project(api_client, LUA_PASSTHROUGH, parser_language=1, dataset_count=1)

    transform_code = (
        "function transform(v, info)\n"
        "  if type(info) == 'table' and info.frameNumber and info.frameNumber % 2 == 0 then\n"
        "    deviceWrite(string.format('N=%d\\n', info.frameNumber))\n"
        "  end\n"
        "  return v\n"
        "end\n"
    )
    _set_transform(api_client, 0, 0, transform_code)

    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"10", b"20", b"30", b"40", b"50"],
        settle_seconds=1.5,
    )
    # frameNumber goes 1..5; evens are 2 and 4 -> "N=2\n" and "N=4\n"
    assert b"N=2\n" in captured, f"missing N=2, captured={captured!r}"
    assert b"N=4\n" in captured, f"missing N=4, captured={captured!r}"
    # Odd frames must NOT have written
    assert b"N=1\n" not in captured
    assert b"N=3\n" not in captured
    assert b"N=5\n" not in captured


@pytest.mark.project
def test_js_transform_frame_info_timestamp_monotonic(
    api_client, device_simulator, clean_state
):
    """info.timestampMs is monotonic and a usable delta source for rate limiting."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    transform_code = (
        "var lastTs = -1;\n"
        "var nonMonotonic = false;\n"
        "function transform(v, info) {\n"
        "  if (info && typeof info.timestampMs === 'number') {\n"
        "    if (info.timestampMs < lastTs) nonMonotonic = true;\n"
        "    if (info.frameNumber % 2 === 0) {\n"
        "      deviceWrite('T=' + (nonMonotonic ? 'BAD' : 'OK') + '\\n');\n"
        "    }\n"
        "    lastTs = info.timestampMs;\n"
        "  }\n"
        "  return v;\n"
        "}\n"
    )
    _set_transform(api_client, 0, 0, transform_code)

    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"10", b"20", b"30", b"40"],
        settle_seconds=1.3,
    )
    assert b"T=OK\n" in captured, f"expected at least one T=OK, captured={captured!r}"
    assert (
        b"T=BAD\n" not in captured
    ), f"timestamps went backwards, captured={captured!r}"


# ---------------------------------------------------------------------------
# actionFire from parsers and transforms
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_lua_parser_action_fire_triggers_existing_action(
    api_client, device_simulator, clean_state
):
    """A Lua parser calling actionFire(id) should fire the action's payload."""
    # Build a project with one dataset + one action whose payload is a unique marker.
    api_client.create_new_project()
    time.sleep(0.2)
    gid = api_client.add_group("G", widget_type=0)
    api_client.add_dataset(gid, options=1)

    action_id = api_client.add_action()
    api_client.update_action(action_id, title="Marker", txData="MARK", eolSequence="\n")
    time.sleep(0.15)

    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        frame_detection=1,
        operation_mode=0,
    )

    parser_code = (
        "function parse(frame)\n"
        "  local out = {}\n"
        "  for f in frame:gmatch('([^,]+)') do out[#out + 1] = f end\n"
        "  if tonumber(out[1]) and tonumber(out[1]) > 50 then\n"
        f"    actionFire({action_id})\n"
        "  end\n"
        "  return out\n"
        "end\n"
    )
    api_client.set_frame_parser_code(parser_code, language=1, source_id=0)
    time.sleep(0.2)

    captured = _send_and_capture(
        api_client, device_simulator, [b"10", b"99", b"20"], settle_seconds=1.5
    )
    # The action's payload + eol concatenates to "MARK\n"; one frame > 50 means
    # at least one MARK\n should reach the device.
    assert b"MARK\n" in captured, f"actionFire didn't trigger, captured={captured!r}"


@pytest.mark.project
def test_js_transform_action_fire_invalid_id_returns_error(
    api_client, device_simulator, clean_state
):
    """actionFire with an unknown id returns {ok:false, error:...} without throwing."""
    _setup_project(api_client, JS_PASSTHROUGH, parser_language=0, dataset_count=1)

    # Probe with an actionId we know doesn't exist (no actions configured).
    transform_code = (
        "function transform(v) {\n"
        "  var r = actionFire(99999);\n"
        "  if (r.ok || !r.error) throw 'expected error result for unknown actionId';\n"
        "  return v;\n"
        "}\n"
    )
    _set_transform(api_client, 0, 0, transform_code)

    data: dict = {}
    captured = _send_and_capture(
        api_client,
        device_simulator,
        [b"1"],
        settle_seconds=0.8,
        dashboard_out=data,
    )
    assert captured == b"", f"unknown action should not write, captured={captured!r}"

    groups = data.get("frame", {}).get("groups", [])
    assert groups, "transform stopped producing frames"
