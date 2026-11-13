"""
Spec 0120 AC3 -- a project's scripts are refused the few commands that reach past the project.

The project is first saved to its own file by this test, a remote API client, which also pins
that a remote save works as before (R11). Its control script then asks for two refused commands
while it runs: removing an extension repository with no index (a call that changes nothing even
when allowed) and saving the project to a second file. It writes each answer into a user table as
numbers, and the test reads the table back over the API. Both are refused with the script policy's
message and the second file never appears (R10). Last, the same repository call from this test
reaches its handler instead of the script policy (R11).

Requires the app running with the API server on (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils.api_client import APIError

TABLE = "Policy"
POLICY_TEXT = "not available to a project's scripts"
REGISTERS = ["repo_ok", "repo_policy", "save_ok", "save_policy"]

_PARSER = """
function parse(frame) {
  return frame.split(",");
}
"""

_CONTROL_SCRIPT = """
var results = null;

function attempt(key, method, params) {
  var r = apiCall(method, params);
  var refused = !r.ok && String(r.error).indexOf("%(policy)s") >= 0;
  results[key + "_ok"] = r.ok ? 1 : 0;
  results[key + "_policy"] = refused ? 1 : 0;
}

function setup() {
  results = {};
  attempt("repo", "extensions.removeRepository", {});
  attempt("save", "project.save", { filePath: %(other)s });
}

function loop() {
  for (var key in results)
    tableSet("%(table)s", key, results[key]);

  delay(100);
}
"""


def _project() -> dict:
    return {
        "title": "Script command policy",
        "frameEnd": "\\n",
        "frameDetection": 0,
        "decoder": 0,
        "sources": [
            {
                "sourceId": 0,
                "title": "Device",
                "busType": 0,
                "frameStart": "",
                "frameEnd": "\\n",
                "frameDetection": 0,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": _PARSER,
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "tables": [
            {
                "name": TABLE,
                "registers": [
                    {"name": n, "type": "computed", "value": -1} for n in REGISTERS
                ],
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [{"title": "A", "value": "%1", "index": 1}],
            }
        ],
        "actions": [],
    }


def _connect(api_client, device_simulator) -> None:
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except APIError:
        pass
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _register(api_client, name: str):
    result = api_client.command(
        "project.dataTable.getValue", {"table": TABLE, "name": name}
    )
    return result.get("value")


def _results(api_client, timeout: float = 5.0) -> dict:
    """Polls until the script has written every register, or returns what it has."""
    deadline = time.monotonic() + timeout
    values = {}
    while time.monotonic() < deadline:
        values = {name: _register(api_client, name) for name in REGISTERS}
        if all(v is not None and v >= 0 for v in values.values()):
            return values

        time.sleep(0.2)

    return values


@pytest.mark.integration
@pytest.mark.project
@pytest.mark.network
def test_control_script_is_refused_and_remote_client_is_not(
    api_client, clean_state, device_simulator, tmp_path
):
    own = tmp_path / "own.ssproj"
    other = tmp_path / "other.ssproj"

    api_client.load_project_from_json(_project())
    time.sleep(0.3)
    api_client.command("controlScript.set", {"code": ""})

    api_client.command("project.save", {"filePath": str(own)})
    time.sleep(0.3)
    assert own.exists(), "a remote client's project.save to a path no longer writes it"

    script = _CONTROL_SCRIPT % {
        "policy": POLICY_TEXT,
        "other": json.dumps(str(other)),
        "table": TABLE,
    }
    api_client.command("controlScript.set", {"code": script})
    time.sleep(0.3)
    _connect(api_client, device_simulator)

    values = _results(api_client)
    assert values == {
        "repo_ok": 0,
        "repo_policy": 1,
        "save_ok": 0,
        "save_policy": 1,
    }, f"the control script was not refused by the script policy: {values}"
    assert not other.exists(), "a project script saved the project over another file"

    with pytest.raises(APIError) as exc:
        api_client.command("extensions.removeRepository", {})

    assert POLICY_TEXT not in exc.value.message
    assert "index" in exc.value.message
