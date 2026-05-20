"""
Generic apiCall() Gateway -- Frame Parser Integration Tests

Verifies that Lua and JavaScript frame parsers can reach Serial Studio's
full API command surface through `apiCall(method, params?)` and
`apiCallList()`, the in-process gateways installed by ScriptApiCall.

The parser writes the gateway's result into a dataset value, then the
test reads it back via the network TCP API. That round-trip proves the
exact same `API::CommandRegistry::execute` is reachable from both
entry points.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

from utils import DataGenerator

# -----------------------------------------------------------------------------
# Helpers
# -----------------------------------------------------------------------------


def _get_dataset_values(api_client, group_idx: int = 0) -> list:
    data = api_client.get_dashboard_data()
    groups = data.get("frame", {}).get("groups", [])
    if not groups or group_idx >= len(groups):
        return []

    return [d.get("value", "") for d in groups[group_idx].get("datasets", [])]


def _wait_for_first_value(api_client, group_idx: int = 0, timeout: float = 5.0) -> list:
    """Block until at least one dataset value appears, then return all values."""
    end = time.time() + timeout
    while time.time() < end:
        values = _get_dataset_values(api_client, group_idx)
        if values and any(v not in ("", None) for v in values):
            return values

        time.sleep(0.05)

    return _get_dataset_values(api_client, group_idx)


def _make_project_with_two_datasets(api_client):
    """Create a project: one group, two datasets named A and B."""
    api_client.create_new_project()
    time.sleep(0.2)

    api_client.command("project.group.add", {"title": "Probe", "widgetType": 0})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.1)
    api_client.command("project.dataset.add", {"groupId": 0, "options": 1})
    time.sleep(0.1)


def _connect_device(api_client, device_simulator):
    """Wire up project + frame parser + network driver, then connect."""
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="", end_sequence="\n", operation_mode=0, frame_detection=0
    )
    api_client.command("project.activate")
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(
        timeout=5.0
    ), "Serial Studio did not connect to the device simulator within 5 seconds"


# -----------------------------------------------------------------------------
# Parser snippets -- each one stores apiCall's result into the frame so
# the test can assert against it via dashboard.getData.
# -----------------------------------------------------------------------------


_LUA_PROBE_COMMAND_COUNT = """
function parse(frame)
  local cmds = apiCallList()
  return { tostring(#cmds), "lua" }
end
"""

_LUA_PROBE_UNKNOWN_METHOD = """
function parse(frame)
  local r = apiCall("definitely.not.a.command", { x = 1 })
  return { tostring(r.ok), r.errorCode or "no-code" }
end
"""

_LUA_PROBE_DASHBOARD_GETSTATUS = """
function parse(frame)
  local r = apiCall("dashboard.getStatus")
  if not r.ok then
    return { "fail", tostring(r.error) }
  end

  -- result.running is the canonical "is the dashboard live?" flag.
  return { tostring(r.result.running), "ok" }
end
"""

_LUA_PROBE_BAD_ARGS = """
function parse(frame)
  -- Method must be a string; nil should be rejected without crashing.
  local r = apiCall(nil)
  return { tostring(r.ok), r.error or "no-error" }
end
"""

_JS_PROBE_COMMAND_COUNT = """
function parse(frame) {
    var cmds = apiCallList();
    return [String(cmds.length), "js"];
}
"""

_JS_PROBE_DASHBOARD_GETSTATUS = """
function parse(frame) {
    var r = apiCall("dashboard.getStatus");
    if (!r.ok) return ["fail", String(r.error)];
    return [String(r.result.running), "ok"];
}
"""


# -----------------------------------------------------------------------------
# Tests -- Lua
# -----------------------------------------------------------------------------


class TestApiCallLua:
    """apiCall() from the Lua frame parser."""

    def test_apicalllist_returns_command_array(
        self, api_client, device_simulator, clean_state
    ):
        """apiCallList() should return the full registry; same count as api.getCommands."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_LUA_PROBE_COMMAND_COUNT, language=1)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert len(values) == 2
        assert values[1] == "lua", f"language tag wrong: {values}"

        # The count must match what api.getCommands reports over TCP.
        api_count = len(api_client.command("api.getCommands").get("commands", []))
        parser_count = int(values[0])
        assert (
            parser_count == api_count
        ), f"parser saw {parser_count} commands, TCP saw {api_count}"
        assert parser_count > 50, "registry should expose well over 50 commands"

    def test_unknown_method_returns_structured_error(
        self, api_client, device_simulator, clean_state
    ):
        """Calling a missing command must NOT crash the parser."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_LUA_PROBE_UNKNOWN_METHOD, language=1)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert len(values) == 2
        assert values[0] == "false", f"ok must be false: {values}"
        assert values[1] == "UNKNOWN_COMMAND", f"unexpected errorCode: {values}"

    def test_dashboard_getstatus_round_trip(
        self, api_client, device_simulator, clean_state
    ):
        """The same dashboard.getStatus call from Lua and TCP returns matching data."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_LUA_PROBE_DASHBOARD_GETSTATUS, language=1)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert len(values) == 2
        assert values[1] == "ok", f"parser-side error: {values}"

        # Compare against the TCP path's view of the same command.
        tcp_status = api_client.command("dashboard.getStatus")
        expected = "true" if tcp_status.get("running") else "false"
        assert (
            values[0] == expected
        ), f"Lua saw running={values[0]} but TCP saw running={expected}"

    def test_bad_args_dont_crash_parser(
        self, api_client, device_simulator, clean_state
    ):
        """apiCall(nil) should return ok=false, error=string -- never throw."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_LUA_PROBE_BAD_ARGS, language=1)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert values[0] == "false"
        assert (
            "method" in values[1].lower() or "string" in values[1].lower()
        ), f"unhelpful error: {values}"


# -----------------------------------------------------------------------------
# Tests -- JavaScript
# -----------------------------------------------------------------------------


class TestApiCallJavaScript:
    """apiCall() from the JavaScript frame parser."""

    def test_apicalllist_returns_command_array(
        self, api_client, device_simulator, clean_state
    ):
        """JS path also surfaces the full command registry."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_JS_PROBE_COMMAND_COUNT, language=0)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert len(values) == 2
        assert values[1] == "js"

        api_count = len(api_client.command("api.getCommands").get("commands", []))
        parser_count = int(values[0])
        assert (
            parser_count == api_count
        ), f"JS parser saw {parser_count} commands, TCP saw {api_count}"

    def test_dashboard_getstatus_round_trip(
        self, api_client, device_simulator, clean_state
    ):
        """dashboard.getStatus from JS matches what TCP sees."""
        _make_project_with_two_datasets(api_client)
        api_client.set_frame_parser_code(_JS_PROBE_DASHBOARD_GETSTATUS, language=0)
        time.sleep(0.2)
        _connect_device(api_client, device_simulator)

        device_simulator.send_frame(b"trigger\n")
        values = _wait_for_first_value(api_client)

        assert len(values) == 2
        assert values[1] == "ok", f"JS-side error: {values}"

        tcp_status = api_client.command("dashboard.getStatus")
        expected = "true" if tcp_status.get("running") else "false"
        assert values[0] == expected
