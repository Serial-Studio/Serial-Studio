"""
Zero-Hardware Demo Integration Tests (spec 0002-zero-hardware-demo)

Drives the bundled rocket-launch demo over the TCP API and asserts:
- system.startDemo stages/opens the demo project and connects the dummy source
- the control-loop simulation streams (the mission clock advances)
- the expected core dashboard groups exist

Requires a running Serial Studio instance with the API server enabled.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

DEMO_TITLE = "Rocket Launch Demo"
CORE_GROUPS = {"Flight Instruments", "Attitude", "Mission Status"}


def _mission_clock(api_client) -> float:
    """Read the live mission-elapsed-time register written by the control loop."""
    result = api_client.command(
        "project.dataTable.getValue", {"table": "Flight", "name": "met"}
    )
    return float(result["value"])


def _start_demo_and_wait(api_client, timeout: float = 15.0) -> float:
    """Start the demo and poll until the simulation is ticking the mission clock."""
    api_client.command("system.startDemo")

    # The register reads back its 0.0 default as soon as the table store is
    # built, before the control loop's first write; returning that default
    # breaks callers that expect a live countdown value. Wait until the clock
    # actually moves between two polls.
    last = None
    deadline = time.time() + timeout
    while time.time() < deadline:
        try:
            value = _mission_clock(api_client)
        except (APIError, KeyError, ValueError):
            time.sleep(0.25)
            continue

        if last is not None and value != last:
            return value

        last = value
        time.sleep(0.25)

    pytest.fail("mission clock never started ticking after system.startDemo")


@pytest.mark.integration
@pytest.mark.project
def test_demo_command_exists(api_client):
    """system.startDemo is registered with the command registry."""
    assert api_client.command_exists("system.startDemo")


@pytest.mark.integration
@pytest.mark.project
def test_demo_starts_and_streams(api_client, clean_state):
    """The demo opens, connects, and the simulation advances in real time."""
    first = _start_demo_and_wait(api_client)

    time.sleep(1.5)
    second = _mission_clock(api_client)
    assert second > first, f"mission clock did not advance ({first} -> {second})"

    assert api_client.is_connected(), "demo source is not connected"

    status = api_client.get_project_status()
    assert (
        status.get("title") == DEMO_TITLE
    ), f"unexpected project: {status.get('title')}"


@pytest.mark.integration
@pytest.mark.project
def test_demo_dashboard_groups(api_client, clean_state):
    """The demo project exposes its core (free-tier) dashboard groups."""
    _start_demo_and_wait(api_client)

    result = api_client.command("project.group.list")
    titles = {g.get("title") for g in result.get("groups", [])}
    missing = CORE_GROUPS - titles
    assert not missing, f"missing demo groups: {missing} (got: {titles})"


@pytest.mark.integration
@pytest.mark.project
def test_demo_restart_begins_new_flight(api_client, clean_state):
    """Starting the demo again restarts the flight from the countdown."""
    _start_demo_and_wait(api_client)
    time.sleep(1.0)

    api_client.command("system.startDemo")
    deadline = time.time() + 15.0
    while time.time() < deadline:
        try:
            if _mission_clock(api_client) < -3.0:
                return
        except (APIError, KeyError, ValueError):
            pass
        time.sleep(0.25)

    pytest.fail("demo did not restart from the countdown")
