"""
Data-table handle API — end-to-end integration tests.

Exercises the real DataTableStore through both script engines and the marshalled
control-script commands, covering what the Node-only tests/scripts unit cannot:

- AC1 / R6: a JS parser writing by handle reaches the store; handle and name read
  back identically.
- AC6 / R8: the same holds for a Lua parser (Lua parity).
- AC7 / R9: a control script (here, the marshalled project.dataTable.* commands)
  resolves a handle and reads/writes through it, agreeing with name access.
- AC5 / R7: after a table-definition edit rebuilds the store, an old handle is a
  safe no-op (written=false), never touching the wrong register.

AC4 (the >80% per-frame-time drop) is a maintainer in-app observation on the BADAQ
project; there is no stable automated timing assertion here.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

CHANNELS = ["ch1", "ch2", "ch3", "ch4"]


def _project_with_table_and_parser(parser_code: str, parser_language: int) -> dict:
    """A project carrying a 'DAQ' table of computed registers plus a frame parser."""
    # Parser code/language live per-source: the loader ignores them at the top
    # level (only the legacy "frameParser" key or a "sources" array is read).
    return {
        "title": "Handle API test",
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 0,
                "frameStart": "/*",
                "frameEnd": "*/",
                "frameDetection": 1,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": parser_code,
                "frameParserLanguage": parser_language,
                "connectionSettings": {},
            }
        ],
        "tables": [
            {
                "name": "DAQ",
                "registers": [
                    {"name": ch, "type": "computed", "value": 0} for ch in CHANNELS
                ],
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "X",
                        "units": "",
                        "value": "%1",
                        "index": 1,
                        "min": 0,
                        "max": 100,
                    }
                ],
            }
        ],
        "actions": [],
    }


# Parsers resolve handles lazily on the first parse() call: that always runs after
# the table store is built, so it is robust regardless of script-vs-store load order.
_JS_PARSER = """
var H = null;
function parse(frame) {
  if (H === null) H = tableHandleMany("DAQ", ["ch1","ch2","ch3","ch4"]);
  var p = frame.split(",");
  for (var i = 0; i < H.length; i++) tableSetH(H[i], parseInt(p[i], 10));
  return p;
}
"""

_LUA_PARSER = """
H = nil
function parse(frame)
  if H == nil then H = tableHandleMany("DAQ", {"ch1","ch2","ch3","ch4"}) end
  local p = {}
  for s in string.gmatch(frame, "([^,]+)") do p[#p + 1] = s end
  for i = 1, #H do tableSetH(H[i], tonumber(p[i])) end
  return p
end
"""


def _attach_via_network(api_client, device_simulator) -> None:
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    api_client.configure_frame_parser(
        start_sequence="/*", end_sequence="*/", operation_mode=0, frame_detection=1
    )
    try:
        api_client.command("project.activate")
    except Exception:
        pass
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _read(api_client, table, name):
    return api_client.command(
        "project.dataTable.getValue", {"table": table, "name": name}
    )["value"]


@pytest.mark.integration
@pytest.mark.project
class TestParserHandleParity:
    def _run_parser(self, api_client, device_simulator, code, language):
        project = _project_with_table_and_parser(code, language)
        api_client.load_project_from_json(project)
        time.sleep(0.3)
        _attach_via_network(api_client, device_simulator)
        device_simulator.send_frame(b"/*1,2,3,4*/")
        time.sleep(0.5)
        return [_read(api_client, "DAQ", ch) for ch in CHANNELS]

    def test_js_parser_handle_writes_reach_store(
        self, api_client, clean_state, device_simulator
    ):
        """A JS parser writing by handle lands the values in the real store (AC1/R6)."""
        assert self._run_parser(api_client, device_simulator, _JS_PARSER, 0) == [
            1,
            2,
            3,
            4,
        ]

    def test_lua_parser_handle_writes_reach_store(
        self, api_client, clean_state, device_simulator
    ):
        """The Lua parser must behave identically (AC6/R8 — Lua parity)."""
        assert self._run_parser(api_client, device_simulator, _LUA_PARSER, 1) == [
            1,
            2,
            3,
            4,
        ]


@pytest.mark.integration
@pytest.mark.project
class TestControlScriptHandlePath:
    def _load_ctrl_table(self, api_client):
        project = {
            "title": "Handle control test",
            "tables": [
                {
                    "name": "CTRL",
                    "registers": [{"name": "reg", "type": "computed", "value": 0}],
                }
            ],
            "groups": [],
            "actions": [],
        }
        api_client.load_project_from_json(project)
        time.sleep(0.2)

    def test_control_script_handle_roundtrip(self, api_client, clean_state):
        """Resolve + write + read by handle through the marshalled commands (AC7/R9)."""
        self._load_ctrl_table(api_client)

        handle = api_client.command(
            "project.dataTable.handle", {"table": "CTRL", "name": "reg"}
        )["handle"]
        assert handle != -1

        written = api_client.command(
            "project.dataTable.setValueH", {"handle": handle, "value": 42}
        )["written"]
        assert written is True

        got = api_client.command("project.dataTable.getValueH", {"handle": handle})
        assert got["found"] is True
        assert got["value"] == 42

        # The handle write must be visible through name-based access too.
        assert _read(api_client, "CTRL", "reg") == 42

    def test_invalid_handle_is_safe_noop(self, api_client, clean_state):
        """A -1 handle reads found=false and writes written=false, never throwing (R5)."""
        self._load_ctrl_table(api_client)

        got = api_client.command("project.dataTable.getValueH", {"handle": -1})
        assert got["found"] is False

        written = api_client.command(
            "project.dataTable.setValueH", {"handle": -1, "value": 5}
        )["written"]
        assert written is False

    def test_stale_handle_after_table_edit_is_noop(self, api_client, clean_state):
        """Editing table defs rebuilds the store; the old handle must not write (AC5/R7).

        Assumes addRegister re-initializes the table store (generation bump). If this
        ever fails with written=True, the store-rebuild/handle coupling regressed.
        """
        self._load_ctrl_table(api_client)

        stale = api_client.command(
            "project.dataTable.handle", {"table": "CTRL", "name": "reg"}
        )["handle"]
        assert stale != -1

        # Force a store rebuild by adding a register.
        api_client.command(
            "project.dataTable.addRegister",
            {"table": "CTRL", "name": "reg2", "computed": True, "value": 0},
        )
        time.sleep(0.2)

        written = api_client.command(
            "project.dataTable.setValueH", {"handle": stale, "value": 999}
        )["written"]
        assert written is False
        assert _read(api_client, "CTRL", "reg") != 999

        # A freshly resolved handle still works after the rebuild.
        fresh = api_client.command(
            "project.dataTable.handle", {"table": "CTRL", "name": "reg"}
        )["handle"]
        assert (
            api_client.command(
                "project.dataTable.setValueH", {"handle": fresh, "value": 7}
            )["written"]
            is True
        )
        assert _read(api_client, "CTRL", "reg") == 7
