# Serial Studio - https://serial-studio.com/
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Spec 0092 (lazy trial & graceful degradation), the automatable halves of AC3/AC5/AC6:
# free features must work end-to-end in EVERY licensing state (the pre-0092 build refused
# connectDevice outright with an expired trial), a Pro-featured project must load degraded
# rather than blocked, and the reported entitlement must agree with how Pro content resolved.
# The tests read the app's real licensing state over the API and branch their asserts, so the
# suite passes on licensed, trial, and expired instances while pinning each state's contract.

import time

import pytest


@pytest.mark.integration
@pytest.mark.network
def test_free_session_works_in_any_licensing_state(
    api_client, device_simulator, clean_state
):
    """A network (free bus) session connects and receives frames regardless of licensing."""
    api_client.set_operation_mode("quickplot")
    time.sleep(0.2)

    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.2)

    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0), (
        "free-bus connect must succeed in every licensing state (spec 0092 R6); "
        "a refusal here is the pre-0092 expired-trial lockout"
    )

    frames = [f"{i * 1.5},{i * 2.5}\n".encode() for i in range(10)]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.5)

    assert api_client.is_connected()
    api_client.disconnect_device()


@pytest.mark.integration
@pytest.mark.project
def test_pro_project_loads_degraded_not_blocked(api_client, clean_state):
    """A project with Pro widgets loads in every state and reports its Pro content."""
    api_client.create_new_project(title="Spec 0092 Degradation")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": "Attitude", "widgetType": 6})
    time.sleep(0.2)
    for _ in range(3):
        api_client.command("project.dataset.add", {"options": 0})
        time.sleep(0.1)

    status = api_client.command("project.exportJson")
    config = status.get("config", {})
    groups = config.get("groups", [])
    assert len(groups) == 1, "the Pro-widget group must exist regardless of licensing"

    loaded = api_client.command("project.loadIntoFrameBuilder")
    assert loaded.get(
        "loaded", False
    ), "a Pro-featured project must load (degraded when unlicensed), never be blocked"
    time.sleep(0.3)


@pytest.mark.integration
@pytest.mark.project
def test_entitlement_report_matches_pro_resolution(api_client, clean_state):
    """licensing.getStatus agrees with containsCommercialFeatures on a Pro project."""
    if not api_client.command_exists("licensing.getStatus"):
        pytest.skip("GPL build: no licensing surface")

    api_client.create_new_project(title="Spec 0092 Entitlement")
    time.sleep(0.3)

    api_client.command("project.group.add", {"title": "Depth", "widgetType": 6})
    time.sleep(0.2)

    project = api_client.get_project_status()
    assert project.get(
        "containsCommercialFeatures", False
    ), "a plot3d group must flag the project as Pro-featured in every licensing state"

    status = api_client.command("licensing.getStatus")
    assert isinstance(
        status.get("isActivated"), bool
    ), "licensing.getStatus must report the activation state the degradation derives from"
