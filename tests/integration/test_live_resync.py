"""
Live Project Re-sync Integration Test (spec 0119, AC8)

A project edit made while a multi-source session is connected re-syncs the frame builder
mid-session. On 2026-11-09 that dropped every per-source layout without announcing it again, and
the next synthetic refresh published the combined frame as source 0's layout. This drives the real
trigger end to end: the edit, the re-announcement, and a refresh while neither source is sending.

This pins the trigger and the re-announcement only. Once the re-announcement has rebuilt the
per-source frames a refresh cannot reach the combined-frame decision, and a rebuild loop that
restores the same widget count is invisible through the API. The tst_resync_decisions ctest suite
pins both of those.

The suite gates CI, so everything here polls to a deadline and nothing asserts a timing floor.

The app must be running with the API server enabled (localhost:7777).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import copy
import threading
import time

import pytest

from utils import DeviceSimulator
from utils.api_client import APIError

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

ALPHA_PORT = 9040
BETA_PORT = 9041

ALPHA_GROUP = "Alpha Data"
BETA_GROUP = "Beta Data"
BETA_GROUP_ID = 1
RENAMED_DATASET = "Beta Counter Renamed"

STREAM_INTERVAL_S = 0.05
POLL_S = 0.1
DEADLINE_S = 10.0
QUIET_S = 1.0
HOLD_S = 2.0

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


def _group(title: str, source_id: int, dataset_title: str) -> dict:
    return {
        "title": title,
        "widget": "",
        "sourceId": source_id,
        "datasets": [{"title": dataset_title, "value": "", "index": 1, "graph": True}],
    }


RESYNC_PROJECT = {
    "title": "Live re-sync test",
    "frameStart": "/*",
    "frameEnd": "*/",
    "frameDetection": 1,
    "decoder": 0,
    "sources": [
        _source(0, "Alpha", "/*", "*/"),
        _source(1, "Beta", "<<", ">>"),
    ],
    "groups": [
        _group(ALPHA_GROUP, 0, "Alpha Counter"),
        _group(BETA_GROUP, 1, "Beta Counter"),
    ],
    "actions": [],
}


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


class _CounterStream:
    """Streams monotonically-counting framed values until stopped; restartable."""

    def __init__(self, simulator: DeviceSimulator, start: bytes, end: bytes):
        self._simulator = simulator
        self._start = start
        self._end = end
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

    def _loop(self) -> None:
        while self._running.is_set():
            self._counter += 1
            payload = self._start + str(self._counter).encode() + b",0" + self._end
            try:
                self._simulator.send_frame(payload)
            except Exception:
                return

            time.sleep(STREAM_INTERVAL_S)


def _is_commercial(api_client) -> bool:
    """Multi-source needs a Commercial build; a refused source_add says this is not one."""
    try:
        new_id = api_client.source_add()
        api_client.source_delete(new_id)
        return True
    except APIError as e:
        if "commercial" in e.message.lower() or "license" in e.message.lower():
            return False
        raise


def _groups(api_client) -> list:
    return api_client.get_dashboard_data().get("frame", {}).get("groups", [])


def _values(api_client) -> dict:
    """Map each group title to its first dataset's numeric value."""
    return {
        group.get("title"): group["datasets"][0].get("numericValue", 0.0)
        for group in _groups(api_client)
        if group.get("datasets")
    }


def _dataset_titles(api_client) -> set:
    return {
        dataset.get("title")
        for group in _groups(api_client)
        for dataset in group.get("datasets", [])
    }


def _wait_until(predicate, message: str) -> None:
    deadline = time.monotonic() + DEADLINE_S
    while time.monotonic() < deadline:
        if predicate():
            return

        time.sleep(POLL_S)

    pytest.fail(message)


def _wait_both_advance(api_client) -> None:
    """Wait until both sources' counters have moved past where they are now."""
    _wait_until(
        lambda: {ALPHA_GROUP, BETA_GROUP} <= set(_values(api_client)),
        "the dashboard never showed both sources' groups",
    )
    before = _values(api_client)

    def advanced() -> bool:
        now = _values(api_client)
        return all(
            now.get(title, before[title]) > before[title]
            for title in (ALPHA_GROUP, BETA_GROUP)
        )

    _wait_until(advanced, f"a source stopped updating; started from {before}")


def _wait_quiet(api_client) -> None:
    """Wait until the dashboard's values have held still for QUIET_S."""
    state = {"values": _values(api_client), "since": time.monotonic()}

    def quiet() -> bool:
        now = _values(api_client)
        if now != state["values"]:
            state["values"] = now
            state["since"] = time.monotonic()

        return time.monotonic() - state["since"] >= QUIET_S

    _wait_until(quiet, "the dashboard's values never settled after the streams stopped")


def _hold_widget_count(api_client, expected: int) -> None:
    """Assert the widget count holds for HOLD_S: a lost or duplicated layout moves it."""
    deadline = time.monotonic() + HOLD_S
    while time.monotonic() < deadline:
        count = api_client.get_widget_count()
        assert count == expected, f"widget count moved to {count}, expected {expected}"
        time.sleep(POLL_S)


# ---------------------------------------------------------------------------
# Fixture
# ---------------------------------------------------------------------------


@pytest.fixture
def resync_env(api_client, clean_state):
    """Two TCP simulators and the two-source project, connected and idle."""
    if not _is_commercial(api_client):
        pytest.skip("multi-source requires a Commercial build")

    alpha_sim = DeviceSimulator(port=ALPHA_PORT)
    beta_sim = DeviceSimulator(port=BETA_PORT)
    alpha_sim.start()
    beta_sim.start()

    api_client.load_project_from_json(copy.deepcopy(RESYNC_PROJECT))
    time.sleep(0.3)

    api_client.command("io.setBusType", {"busType": 1})
    time.sleep(0.1)
    api_client.source_configure(
        0, {"address": "127.0.0.1", "tcpPort": ALPHA_PORT, "socketTypeIndex": 0}
    )
    api_client.source_configure(
        1, {"address": "127.0.0.1", "tcpPort": BETA_PORT, "socketTypeIndex": 0}
    )
    time.sleep(0.2)

    api_client.connect_device()
    if not alpha_sim.wait_for_connection(timeout=5.0):
        api_client.disconnect_device()
        time.sleep(1.0)
        api_client.connect_device()

    assert alpha_sim.wait_for_connection(timeout=5.0), "alpha simulator never connected"
    assert beta_sim.wait_for_connection(timeout=5.0), "beta simulator never connected"

    alpha = _CounterStream(alpha_sim, b"/*", b"*/")
    beta = _CounterStream(beta_sim, b"<<", b">>")

    yield api_client, alpha, beta

    alpha.stop()
    beta.stop()
    try:
        api_client.disconnect_device()
    except Exception:
        pass
    alpha_sim.stop()
    beta_sim.stop()
    time.sleep(1.0)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------


@pytest.mark.network
@pytest.mark.pro
@pytest.mark.timeout(120)
def test_live_edit_keeps_every_source_on_the_dashboard(resync_env):
    """R6: an edit and a synthetic refresh before any next frame lose no source's widgets."""
    api, alpha, beta = resync_env

    alpha.start()
    beta.start()
    _wait_both_advance(api)

    # Stopping both is the only deterministic way to act before either source's next frame.
    alpha.stop()
    beta.stop()
    _wait_quiet(api)
    widget_count = api.get_widget_count()
    assert widget_count > 0, "the dashboard built no widgets"

    # A real value change guarantees the mutation epoch moves and the re-sync is scheduled.
    api.update_dataset(BETA_GROUP_ID, 0, title=RENAMED_DATASET)

    # From the GUI this queues and answers published=false, so the response proves nothing.
    api.command("dashboard.reprocess")

    # The new title arriving is what proves the re-sync ran with both sources silent;
    # without it this would pass vacuously on the pre-edit layout.
    _wait_until(
        lambda: api.get_widget_count() == widget_count
        and RENAMED_DATASET in _dataset_titles(api),
        f"the widget count never settled back to {widget_count} with the edit applied",
    )

    beta.start()
    _hold_widget_count(api, widget_count)

    alpha.start()
    _wait_both_advance(api)
    _hold_widget_count(api, widget_count)
