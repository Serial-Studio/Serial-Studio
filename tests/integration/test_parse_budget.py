"""
Parse-Budget Fair-Share Governor Integration Tests (spec 0051, M1)

Reproduces the bug-report.md scenario with two network sources: a "heavy"
source whose per-sample Lua transform saturates the parse budget, and a
"light" 10 Hz source that must keep updating at its natural rate.

Asserts (spec ACs 1-3):
 * AC1 -- the light source's dashboard values keep advancing while the heavy
   source is thinned smoothly (no 1 Hz lockstep steps).
 * AC2 -- after the overload stops, the heavy source returns to full rate in
   under one second.
 * AC3 -- while thinning is active, problems.list carries a parse-thinning
   finding under link.statistics, and it clears after recovery.

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import threading
import time

import pytest

from utils import DeviceSimulator

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

HEAVY_PORT = 9000
LIGHT_PORT = 9001
HEAVY_BURST = 8

# Bounded busy-loop transform: ~2.5 ms/frame on the dev machine, ~10 ms on a loaded
# CI runner. Oversubscription comes from the OFFERED rate (HEAVY_BURST frames per
# 4 ms send, ~2 kHz) rather than per-frame cost: the recovery AC needs 10 Hz of this
# transform to fit under the 0.45 fair share on the slowest runner (4M iterations
# measured ~100 ms/frame there, so the heavy source could never un-thin), while
# engagement needs offered load decisively past the 0.90 threshold (5x on the dev
# machine, far more on CI).
HEAVY_TRANSFORM = (
    "function transform(value)\n"
    "  local s = 0\n"
    "  for i = 1, 400000 do\n"
    "    s = s + i\n"
    "  end\n"
    "  return value\n"
    "end\n"
)

LUA_CSV_PARSER = (
    "function parse(frame)\n"
    "  local result = {}\n"
    "  for field in frame:gmatch('([^,]+)') do\n"
    "    result[#result + 1] = field\n"
    "  end\n"
    "  return result\n"
    "end\n"
)


def _source(source_id: int, title: str, start: str, end: str) -> dict:
    return {
        "sourceId": source_id,
        "title": title,
        "busType": 1,
        "frameStart": start,
        "frameEnd": end,
        "frameDetection": 1,
        "checksumAlgorithm": "",
        "decoderMethod": 0,
        "frameParserCode": LUA_CSV_PARSER,
        "frameParserLanguage": 1,
        "connectionSettings": {},
    }


BUDGET_PROJECT = {
    "title": "Parse budget test",
    "frameStart": "/*",
    "frameEnd": "*/",
    "frameDetection": 1,
    "decoder": 0,
    "sources": [
        _source(0, "Heavy", "/*", "*/"),
        _source(1, "Light", "<<", ">>"),
    ],
    "groups": [
        {
            "title": "Heavy Data",
            "widget": "",
            "sourceId": 0,
            "datasets": [
                {
                    "title": "Heavy Counter",
                    "value": "",
                    "index": 1,
                    "graph": True,
                    "transformCode": HEAVY_TRANSFORM,
                    "transformLanguage": 1,
                }
            ],
        },
        {
            "title": "Light Data",
            "widget": "",
            "sourceId": 1,
            "datasets": [
                {
                    "title": "Light Counter",
                    "value": "",
                    "index": 1,
                    "graph": True,
                }
            ],
        },
    ],
    "actions": [],
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


class _CounterStream:
    """Streams monotonically-counting framed values until stopped."""

    def __init__(
        self,
        simulator: DeviceSimulator,
        start: bytes,
        end: bytes,
        interval: float,
        burst: int = 1,
    ):
        self._simulator = simulator
        self._start = start
        self._end = end
        self._interval = interval
        self._burst = burst
        self._counter = 0
        self._running = threading.Event()
        self._thread: threading.Thread | None = None

    def start(self) -> None:
        self._running.set()
        self._thread = threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._running.clear()
        if self._thread:
            self._thread.join(timeout=2.0)
            self._thread = None

    def set_interval(self, interval: float, burst: int = 1) -> None:
        self._interval = interval
        self._burst = burst

    def _loop(self) -> None:
        while self._running.is_set():
            payload = b""
            for _ in range(self._burst):
                self._counter += 1
                payload += self._start + str(self._counter).encode() + b",0" + self._end
            try:
                self._simulator.send_frame(payload)
            except Exception:
                return
            time.sleep(self._interval)


def _group_value(data: dict, group_title: str) -> float:
    """Return the first dataset's numericValue for the named group."""
    for group in data.get("frame", {}).get("groups", []):
        if group.get("title") == group_title:
            datasets = group.get("datasets", [])
            assert datasets, f"group {group_title} has no datasets"
            return datasets[0].get("numericValue", 0.0)

    pytest.fail(f"group {group_title} not present in dashboard data")


def _sample_values(api_client, seconds: float, period: float) -> tuple[list, list]:
    """Poll dashboard.getData for `seconds`, returning (heavy, light) series."""
    heavy, light = [], []
    deadline = time.monotonic() + seconds
    while time.monotonic() < deadline:
        data = api_client.get_dashboard_data()
        heavy.append(_group_value(data, "Heavy Data"))
        light.append(_group_value(data, "Light Data"))
        time.sleep(period)

    return heavy, light


def _distinct(series: list) -> int:
    return len(set(series))


def _thinning_findings(api_client) -> list:
    result = api_client.command("problems.list", {"checkerId": "link.statistics"})
    findings = result.get("findings", [])
    return [f for f in findings if f.get("code") == "parse-thinning"]


# ---------------------------------------------------------------------------
# Fixture
# ---------------------------------------------------------------------------


@pytest.fixture
def budget_env(api_client, clean_state):
    """Two TCP simulators + the dual-source budget project, connected."""
    heavy_sim = DeviceSimulator(port=HEAVY_PORT)
    light_sim = DeviceSimulator(port=LIGHT_PORT)
    heavy_sim.start()
    light_sim.start()

    api_client.load_project_from_json(copy.deepcopy(BUDGET_PROJECT))
    time.sleep(0.3)

    api_client.command("io.setBusType", {"busType": 1})
    time.sleep(0.1)
    api_client.source_configure(
        0, {"address": "127.0.0.1", "tcpPort": HEAVY_PORT, "socketTypeIndex": 0}
    )
    api_client.source_configure(
        1, {"address": "127.0.0.1", "tcpPort": LIGHT_PORT, "socketTypeIndex": 0}
    )
    time.sleep(0.2)

    api_client.connect_device()
    if not heavy_sim.wait_for_connection(timeout=5.0):
        api_client.disconnect_device()
        time.sleep(1.0)
        api_client.connect_device()

    assert heavy_sim.wait_for_connection(timeout=5.0), "heavy simulator never connected"
    assert light_sim.wait_for_connection(timeout=5.0), "light simulator never connected"

    heavy = _CounterStream(heavy_sim, b"/*", b"*/", interval=0.004, burst=HEAVY_BURST)
    light = _CounterStream(light_sim, b"<<", b">>", interval=0.1)

    yield api_client, heavy, light

    heavy.stop()
    light.stop()
    try:
        api_client.disconnect_device()
    except Exception:
        pass
    heavy_sim.stop()
    light_sim.stop()
    time.sleep(1.0)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


@pytest.mark.network
class TestParseBudgetFairShare:
    def test_light_source_stays_live_and_heavy_thins_smoothly(self, budget_env):
        """AC1: overload thins only the offender, and smoothly (no 1 Hz beat)."""
        api, heavy, light = budget_env

        light.start()
        heavy.start()
        time.sleep(3.0)

        heavy_series, light_series = _sample_values(api, seconds=2.0, period=0.1)

        # Counter DELTA, not distinct polls: a loaded runner answers getData slowly enough
        # that 2 s yields well under 20 polls, which caps distinct values below any
        # threshold however live the source is.
        light_advanced = light_series[-1] - light_series[0]
        assert light_advanced >= 12, (
            f"light source starved: counter advanced {light_advanced} in 2 s "
            f"(expected ~20 at 10 Hz): {light_series}"
        )
        assert _distinct(heavy_series) >= 4, (
            f"heavy source frozen in lockstep steps: {_distinct(heavy_series)} "
            f"distinct values in 2 s: {heavy_series}"
        )

    def test_problem_center_reports_and_clears_thinning(self, budget_env):
        """AC3: parse-thinning finding present under load, gone after recovery."""
        api, heavy, light = budget_env

        light.start()
        heavy.start()
        time.sleep(4.0)

        findings = _thinning_findings(api)
        assert findings, "no parse-thinning finding while heavy source is over budget"
        assert "Heavy" in findings[0].get("title", ""), findings[0]

        heavy.set_interval(0.25)
        time.sleep(3.0)

        assert not _thinning_findings(api), "parse-thinning finding did not clear"

    def test_recovery_within_one_second(self, budget_env):
        """AC2: heavy source returns to full rate < 1 s after the overload stops."""
        api, heavy, light = budget_env

        light.start()
        heavy.start()
        time.sleep(3.0)

        heavy.set_interval(0.1)
        time.sleep(1.0)

        # The counter DELTA is the recovery signal, not the number of distinct polls: the
        # producer and the poller both run near 10 Hz, so a fraction of polls alias onto the
        # same value however healthy the source is. A thinned source advances a handful of
        # counts per second; a recovered one tracks the ~10 Hz feed.
        heavy_series, _ = _sample_values(api, seconds=1.5, period=0.1)
        advanced = heavy_series[-1] - heavy_series[0]
        assert advanced >= 10, (
            f"heavy source did not recover to full rate: counter advanced {advanced} "
            f"in 1.5 s at 10 Hz: {heavy_series}"
        )
        # The delta above is the recovery signal; this only rules out a source that jumps in
        # one lockstep burst. Kept loose because poll aliasing costs distinct samples on a
        # loaded runner (CI has produced 5 distinct values on a series that advanced 14).
        assert _distinct(heavy_series) >= 4, (
            f"heavy source still updating in lockstep steps: "
            f"{_distinct(heavy_series)} distinct values in 1.5 s"
        )
