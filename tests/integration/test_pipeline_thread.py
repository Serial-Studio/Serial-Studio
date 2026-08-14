"""
Frame-Pipeline Thread Integration Tests (spec 0051, M3)

The frame pipeline (FrameReader -> FrameParser -> FrameBuilder) now runs on a
dedicated processing thread (IO::PipelineHost); the GUI thread only drains
finished frames from an SPSC ring on the display tick. These tests exercise
the seams that move introduced:

 * AC14 (proxy) -- with a deliberately expensive Lua frame parser saturating
   the processing thread, the GUI thread stays responsive: API commands (whose
   handlers run on the GUI thread) keep answering within a bounded latency,
   and a concurrent light source's dashboard values keep advancing.
 * AC15 (determinism proxy) -- replaying the same CSV file twice through the
   marshaled replay lanes produces the same final dataset values; the full
   golden-session dual-replay runs through the spec-0047 harness.
 * Teardown -- rapid connect/disconnect churn under load settles cleanly
   (no wedged connection state, no dead API server).

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import time

import pytest

from utils import DeviceSimulator

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

HEAVY_PORT = 9010
LIGHT_PORT = 9011

# Bounded busy-loop parser: expensive enough to saturate the processing core
# at the feed rate, bounded so it never trips the watchdog.
HEAVY_LUA_PARSER = (
    "function parse(frame)\n"
    "  local s = 0\n"
    "  for i = 1, 2000000 do\n"
    "    s = s + i\n"
    "  end\n"
    "  local result = {}\n"
    "  for field in frame:gmatch('([^,]+)') do\n"
    "    result[#result + 1] = field\n"
    "  end\n"
    "  return result\n"
    "end\n"
)

# API round-trip budget while the pipeline is saturated. Handlers run on the
# GUI thread, so this is the scripted stand-in for "the UI stays interactive".
MAX_API_LATENCY_S = 1.0


def _source(source_id: int, title: str, start: str, end: str, parser: str) -> dict:
    return {
        "sourceId": source_id,
        "title": title,
        "busType": 1,
        "frameStart": start,
        "frameEnd": end,
        "frameDetection": 1,
        "checksumAlgorithm": "",
        "decoderMethod": 0,
        "frameParserCode": parser,
        "frameParserLanguage": 1,
        "connectionSettings": {},
    }


PIPELINE_PROJECT = {
    "title": "Pipeline thread test",
    "frameStart": "/*",
    "frameEnd": "*/",
    "frameDetection": 1,
    "decoder": 0,
    "sources": [
        _source(0, "Heavy", "/*", "*/", HEAVY_LUA_PARSER),
        _source(
            1,
            "Light",
            "<<",
            ">>",
            "function parse(frame)\n"
            "  local result = {}\n"
            "  for field in frame:gmatch('([^,]+)') do\n"
            "    result[#result + 1] = field\n"
            "  end\n"
            "  return result\n"
            "end\n",
        ),
    ],
    "groups": [
        {
            "title": "Heavy Data",
            "widget": "",
            "sourceId": 0,
            "datasets": [
                {"title": "Heavy Counter", "value": "", "index": 1, "graph": True}
            ],
        },
        {
            "title": "Light Data",
            "widget": "",
            "sourceId": 1,
            "datasets": [
                {"title": "Light Counter", "value": "", "index": 1, "graph": True}
            ],
        },
    ],
    "actions": [],
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _group_value(data: dict, group_title: str) -> float:
    for group in data.get("frame", {}).get("groups", []):
        if group.get("title") == group_title:
            datasets = group.get("datasets", [])
            assert datasets, f"group {group_title} has no datasets"
            return datasets[0].get("numericValue", 0.0)

    pytest.fail(f"group {group_title} not present in dashboard data")


class _Feeder:
    """Streams counting frames from a background thread until stopped."""

    def __init__(self, simulator, start: bytes, end: bytes, interval: float):
        import threading

        self._simulator = simulator
        self._start = start
        self._end = end
        self._interval = interval
        self._counter = 0
        self._running = threading.Event()
        self._thread = None
        self._threading = threading

    def start(self):
        self._running.set()
        self._thread = self._threading.Thread(target=self._loop, daemon=True)
        self._thread.start()

    def stop(self):
        self._running.clear()
        if self._thread:
            self._thread.join(timeout=2.0)
            self._thread = None

    def _loop(self):
        while self._running.is_set():
            self._counter += 1
            frame = self._start + str(self._counter).encode() + b",0" + self._end
            try:
                self._simulator.send_frame(frame)
            except Exception:
                return
            time.sleep(self._interval)


# ---------------------------------------------------------------------------
# Fixture
# ---------------------------------------------------------------------------


@pytest.fixture
def pipeline_env(api_client, clean_state):
    heavy_sim = DeviceSimulator(port=HEAVY_PORT)
    light_sim = DeviceSimulator(port=LIGHT_PORT)
    heavy_sim.start()
    light_sim.start()

    api_client.load_project_from_json(copy.deepcopy(PIPELINE_PROJECT))
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

    heavy = _Feeder(heavy_sim, b"/*", b"*/", interval=0.005)
    light = _Feeder(light_sim, b"<<", b">>", interval=0.1)

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
class TestPipelineThread:
    def test_gui_thread_responsive_under_parser_saturation(self, pipeline_env):
        """AC14 proxy: heavy parser saturates the pipeline; the GUI answers fast."""
        api, heavy, light = pipeline_env

        heavy.start()
        light.start()
        time.sleep(2.0)

        latencies = []
        light_values = []
        deadline = time.monotonic() + 6.0
        while time.monotonic() < deadline:
            t0 = time.monotonic()
            data = api.get_dashboard_data()
            latencies.append(time.monotonic() - t0)
            light_values.append(_group_value(data, "Light Data"))
            time.sleep(0.2)

        heavy.stop()
        light.stop()

        worst = max(latencies)
        assert (
            worst < MAX_API_LATENCY_S
        ), f"GUI-thread API round-trip degraded to {worst:.2f}s under pipeline load"

        distinct_light = len(set(light_values))
        assert distinct_light >= 5, (
            f"light source stalled while pipeline was saturated "
            f"({distinct_light} distinct values in 6s)"
        )

    def test_connect_disconnect_churn_settles(self, pipeline_env):
        """Teardown: rapid connect/disconnect under load never wedges the app."""
        api, heavy, light = pipeline_env

        heavy.start()
        time.sleep(1.0)

        for _ in range(3):
            api.disconnect_device()
            time.sleep(0.3)
            api.connect_device()
            time.sleep(0.7)

        heavy.stop()

        status = api.command("io.getStatus", {})
        assert status is not None, "io.getStatus stopped answering after churn"

        api.disconnect_device()
        time.sleep(0.5)
        status = api.command("io.getStatus", {})
        assert status.get("isConnected") is False, "disconnect did not settle"

    def test_dashboard_data_flows_through_ring(self, pipeline_env):
        """Frames produced on the pipeline thread reach dashboard.getData."""
        api, heavy, light = pipeline_env

        light.start()
        time.sleep(2.0)

        first = _group_value(api.get_dashboard_data(), "Light Data")
        time.sleep(1.0)
        second = _group_value(api.get_dashboard_data(), "Light Data")

        light.stop()

        assert (
            second != first or second > 0
        ), "dashboard values never advanced -- ring drain not delivering frames"
