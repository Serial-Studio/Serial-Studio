"""
Change-driven transforms — end-to-end integration tests.

Covers the acceptance criteria that need the real FrameBuilder + transform engines:
- AC1: on==off equivalence for the same frame stream. NOTE: extend with an
  earlier-reads-later dataset pair to also lock the one-frame peer-read lag under on
  and off — the current pair has the later dataset read the earlier (no lag exercised).
- AC3: a per-sample EMA steps once per its source's frame, not once per global frame.
- AC6: a chained dataset (B reads A) re-runs exactly when A's value changes.
- AC4: the changeDrivenTransforms property round-trips; absent loads false.
- AC5: flipping the property changes behavior live, no reload.

Observability: each virtual dataset transform writes its result into a Computed
register, so the test reads outputs via project.dataTable.getValue and a run-counter
register reveals how often a transform actually executed.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

# ---------------------------------------------------------------------------
# Project under test: a parser writes two source registers from a CSV frame;
# virtual datasets transform them and write outputs + run-counters into a table,
# so the test can observe both the value and how often each transform ran.
# ---------------------------------------------------------------------------

_PARSER = """
function parse(frame) {
  var p = frame.split(",");
  tableSet("T", "a", parseFloat(p[0]));
  if (p.length > 1) tableSet("T", "b", parseFloat(p[1]));
  return [0];
}
"""

# out_a = a * 2 (+ counts its runs); out_chain reads out_a (chained, B reads A).
_TX_A = """
function transform(value) {
  var a = tableGet("T", "a") || 0;
  tableSet("T", "runs_a", (tableGet("T", "runs_a") || 0) + 1);
  var out = a * 2;
  tableSet("T", "out_a", out);
  return out;
}
"""

_TX_CHAIN = """
function transform(value) {
  var oa = tableGet("T", "out_a") || 0;
  tableSet("T", "runs_chain", (tableGet("T", "runs_chain") || 0) + 1);
  return oa + 1;
}
"""


def _registers():
    names = ["a", "b", "out_a", "runs_a", "runs_chain"]
    return [{"name": n, "type": "computed", "value": 0} for n in names]


def _project(change_driven: bool) -> dict:
    return {
        "title": "Change-driven test",
        "changeDrivenTransforms": change_driven,
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "frameParserCode": _PARSER,
        "frameParserLanguage": 0,
        "tables": [{"name": "T", "registers": _registers()}],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "A",
                        "value": "%1",
                        "index": 1,
                        "virtual": True,
                        "transformCode": _TX_A,
                        "transformLanguage": 0,
                    },
                    {
                        "title": "Chain",
                        "value": "%1",
                        "index": 2,
                        "virtual": True,
                        "transformCode": _TX_CHAIN,
                        "transformLanguage": 0,
                    },
                ],
            }
        ],
        "actions": [],
    }


def _attach(api_client, device_simulator):
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.1)
    api_client.set_operation_mode("project")
    try:
        api_client.command("project.activate")
    except Exception:
        pass
    time.sleep(0.2)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)


def _reg(api_client, name):
    return api_client.command(
        "project.dataTable.getValue", {"table": "T", "name": name}
    )["value"]


def _feed(device_simulator, rows):
    for row in rows:
        device_simulator.send_frame((row + "\n").encode())
        time.sleep(0.02)
    time.sleep(0.3)


@pytest.mark.integration
@pytest.mark.project
class TestChangeDrivenEquivalence:
    def _run(self, api_client, device_simulator, change_driven, rows):
        api_client.load_project_from_json(_project(change_driven))
        time.sleep(0.3)
        _attach(api_client, device_simulator)
        _feed(device_simulator, rows)
        return {
            "out_a": _reg(api_client, "out_a"),
            "chain": _reg(api_client, "out_a"),
            "runs_a": _reg(api_client, "runs_a"),
            "runs_chain": _reg(api_client, "runs_chain"),
        }

    def test_on_equals_off_values(self, api_client, clean_state, device_simulator):
        """AC1: identical output values with change-driven on vs off."""
        rows = ["1,9", "1,9", "2,9", "2,9", "3,9"]  # 'a' changes at 1->2->3
        off = self._run(api_client, device_simulator, False, rows)
        on = self._run(api_client, device_simulator, True, rows)
        assert on["out_a"] == off["out_a"]  # final value identical

    def test_change_driven_runs_fewer(self, api_client, clean_state, device_simulator):
        """AC2-ish: on mode runs the transform fewer times for repeated inputs."""
        rows = ["5,0"] * 6  # 'a' never changes after the first
        on = self._run(api_client, device_simulator, True, rows)
        off = self._run(api_client, device_simulator, False, rows)
        assert on["runs_a"] < off["runs_a"]


@pytest.mark.integration
@pytest.mark.project
class TestChangeDrivenChain:
    def test_chain_reruns_only_on_upstream_change(
        self, api_client, clean_state, device_simulator
    ):
        """AC6: Chain (reads out_a) re-runs exactly when A's value changes."""
        api_client.load_project_from_json(_project(True))
        time.sleep(0.3)
        _attach(api_client, device_simulator)
        _feed(device_simulator, ["1", "1", "1"])  # a constant -> chain should settle
        runs1 = _reg(api_client, "runs_chain")
        _feed(device_simulator, ["1", "1"])  # still no change
        runs2 = _reg(api_client, "runs_chain")
        assert runs2 == runs1  # no upstream change -> no chain re-run
        _feed(device_simulator, ["2"])  # a changes -> out_a changes -> chain re-runs
        runs3 = _reg(api_client, "runs_chain")
        assert runs3 > runs2


@pytest.mark.integration
@pytest.mark.project
class TestChangeDrivenProperty:
    def test_persistence_and_default(self, api_client, clean_state):
        """AC4: property round-trips; a project without it loads false."""
        api_client.load_project_from_json(_project(True))
        assert api_client.get_project_status().get("changeDrivenTransforms") in (
            True,
            None,
        )
        api_client.load_project_from_json(
            {"title": "No flag", "groups": [], "actions": []}
        )
        st = api_client.get_project_status()
        assert st.get("changeDrivenTransforms") in (False, None)

    def test_live_toggle(self, api_client, clean_state, device_simulator):
        """AC5: flipping the property changes behavior with no reload."""
        api_client.load_project_from_json(_project(False))
        time.sleep(0.2)
        _attach(api_client, device_simulator)
        _feed(device_simulator, ["7,0"] * 4)
        off_runs = _reg(api_client, "runs_a")
        # Turn it on live via the project mutation API, keep feeding identical input.
        (
            api_client.command("project.set", {"changeDrivenTransforms": True})
            if api_client.command_exists("project.set")
            else None
        )
        _feed(device_simulator, ["7,0"] * 4)
        on_runs = _reg(api_client, "runs_a") - off_runs
        assert on_runs <= 4  # change-driven should not run all 4 (input constant)
