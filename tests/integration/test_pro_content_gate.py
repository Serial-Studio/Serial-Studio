# Serial Studio - https://serial-studio.com/
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
#
# Spec 0094 (Pro-content connect gate), the automatable halves of AC2, AC3, AC10 and AC15: a
# project that carries dataset transforms or user tables must refuse to connect without an
# entitlement, a live session must end when such content arrives, the API commands that author
# that content must refuse too, and removing it must stay free.
# Like test_licensing_degradation.py, the tests read the app's real licensing state over the
# API and branch their asserts, so the suite passes on licensed, trial, expired and GPL
# instances while pinning each state's contract.

import copy
import time

import pytest

from utils.api_client import APIError

_TRANSFORM_LUA = "function transform(value)\n  return value * 2\nend\n"

# Core::License::requiresProMessage() opens every licensing refusal with this, in both builds.
_PRO_REQUIRED = "Serial Studio Pro is required"


def _entitled(api_client) -> bool:
    """True when the running app holds a license or an active trial; a GPL build never does."""
    if not api_client.command_exists("licensing.getStatus"):
        return False

    if api_client.command("licensing.getStatus").get("isActivated", False):
        return True

    trial = api_client.command("licensing.getTrialStatus")
    return bool(trial.get("trialEnabled", False))


def _new_project_with_dataset(api_client, title: str) -> dict:
    """Creates a one-group, one-dataset project and returns its exported configuration."""
    api_client.create_new_project(title=title)
    time.sleep(0.3)
    api_client.command("project.group.add", {"title": "Sensors", "widgetType": 0})
    time.sleep(0.2)
    api_client.command("project.dataset.add", {"options": 0})
    time.sleep(0.2)
    return api_client.command("project.exportJson").get("config", {})


def _transform_codes(config: dict) -> list:
    return [
        dataset.get("transformCode", "")
        for group in config.get("groups", [])
        for dataset in group.get("datasets", [])
    ]


def _expect_license_refusal(call, what: str) -> None:
    with pytest.raises(APIError) as refusal:
        call()

    assert refusal.value.code == "OPERATION_FAILED", (
        f"{what} must fail with OPERATION_FAILED without an entitlement, "
        f"got {refusal.value.code}: {refusal.value.message}"
    )
    assert (
        _PRO_REQUIRED in refusal.value.message
    ), f"{what} must name Serial Studio Pro as the reason: {refusal.value.message}"


@pytest.mark.integration
@pytest.mark.project
def test_transform_authoring_follows_entitlement(api_client, clean_state):
    """project.dataset.setTransformCode writes a transform only with an entitlement."""
    _new_project_with_dataset(api_client, "Spec 0094 Transform Authoring")
    params = {"groupId": 0, "datasetId": 0, "code": _TRANSFORM_LUA, "language": 1}

    if _entitled(api_client):
        api_client.command("project.dataset.setTransformCode", params)
        time.sleep(0.2)
        codes = _transform_codes(
            api_client.command("project.exportJson").get("config", {})
        )
        assert any(
            code.strip() for code in codes
        ), "an entitled app must store the transform"
        return

    _expect_license_refusal(
        lambda: api_client.command("project.dataset.setTransformCode", params),
        "setting transform code",
    )
    codes = _transform_codes(api_client.command("project.exportJson").get("config", {}))
    assert not any(
        code.strip() for code in codes
    ), "a refused setTransformCode must leave the project unchanged (spec 0094 R9)"


@pytest.mark.integration
@pytest.mark.project
def test_table_authoring_follows_entitlement(api_client, clean_state):
    """project.dataTable.add creates a user table only with an entitlement."""
    api_client.create_new_project(title="Spec 0094 Table Authoring")
    time.sleep(0.3)

    if _entitled(api_client):
        api_client.command("project.dataTable.add", {"name": "Calibration"})
        return

    _expect_license_refusal(
        lambda: api_client.command("project.dataTable.add", {"name": "Calibration"}),
        "adding a user table",
    )
    tables = (
        api_client.command("project.exportJson").get("config", {}).get("tables", [])
    )
    assert not tables, "a refused dataTable.add must leave the project without tables"


@pytest.mark.integration
@pytest.mark.project
def test_clearing_a_transform_is_free_in_any_state(api_client, clean_state):
    """Removing Pro content never needs an entitlement (spec 0094 R7)."""
    config = _new_project_with_dataset(api_client, "Spec 0094 Removal")
    seeded = copy.deepcopy(config)
    seeded["groups"][0]["datasets"][0]["transformCode"] = _TRANSFORM_LUA
    seeded["groups"][0]["datasets"][0]["transformLanguage"] = 1

    api_client.load_project_from_json(seeded)
    time.sleep(0.3)
    loaded = _transform_codes(
        api_client.command("project.exportJson").get("config", {})
    )
    assert any(code.strip() for code in loaded), (
        "loading a project that carries a transform must stay allowed in every state; "
        "the connect gate is what stops it from running"
    )

    api_client.command(
        "project.dataset.setTransformCode", {"groupId": 0, "datasetId": 0, "code": ""}
    )
    time.sleep(0.2)
    cleared = _transform_codes(
        api_client.command("project.exportJson").get("config", {})
    )
    assert not any(
        code.strip() for code in cleared
    ), "clearing a transform must work without an entitlement"


@pytest.mark.integration
@pytest.mark.project
def test_pro_content_project_refuses_connect_without_entitlement(
    api_client, clean_state
):
    """io.connect names the license when the project carries a transform (spec 0094 R2)."""
    config = _new_project_with_dataset(api_client, "Spec 0094 Connect Gate")
    seeded = copy.deepcopy(config)
    seeded["groups"][0]["datasets"][0]["transformCode"] = _TRANSFORM_LUA
    seeded["groups"][0]["datasets"][0]["transformLanguage"] = 1

    api_client.load_project_from_json(seeded)
    time.sleep(0.3)
    api_client.set_operation_mode("project")
    time.sleep(0.3)

    if _entitled(api_client):
        try:
            api_client.command("io.connect")
        except APIError as error:
            assert _PRO_REQUIRED not in error.message, (
                "an entitled app must never refuse a connect for licensing: "
                + error.message
            )
        finally:
            if api_client.is_connected():
                api_client.disconnect_device()
        return

    _expect_license_refusal(lambda: api_client.command("io.connect"), "connecting")
    assert not api_client.is_connected(), "a refused connect must not open a device"


@pytest.mark.integration
@pytest.mark.project
def test_free_project_is_never_refused_for_licensing(api_client, clean_state):
    """A project with no Pro content is not refused for licensing in any state (R1)."""
    _new_project_with_dataset(api_client, "Spec 0094 Free Project")
    api_client.set_operation_mode("project")
    time.sleep(0.3)

    try:
        api_client.command("io.connect")
    except APIError as error:
        assert _PRO_REQUIRED not in error.message, (
            "a project without transforms or tables must never be refused for licensing "
            "(spec 0092 R6): " + error.message
        )
    finally:
        if api_client.is_connected():
            api_client.disconnect_device()


@pytest.mark.integration
@pytest.mark.network
@pytest.mark.project
def test_pro_content_loaded_into_a_live_session(
    api_client, device_simulator, clean_state
):
    """A live session without an entitlement ends when its project gains a transform."""
    _new_project_with_dataset(api_client, "Spec 0094 Live Session")
    api_client.set_operation_mode("project")
    time.sleep(0.3)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    time.sleep(0.3)
    api_client.connect_device()
    assert device_simulator.wait_for_connection(
        timeout=5.0
    ), "a project without Pro content must connect in every licensing state"

    # Without saved connection settings the reload keeps device 0 open, so only the
    # live-session check (not the reconnect after a rebuild) can end the session.
    live = copy.deepcopy(api_client.command("project.exportJson").get("config", {}))
    for source in live.get("sources", []):
        source.pop("connection", None)

    live["groups"][0]["datasets"][0]["transformCode"] = _TRANSFORM_LUA
    live["groups"][0]["datasets"][0]["transformLanguage"] = 1
    api_client.load_project_from_json(live)
    time.sleep(1.0)

    try:
        if _entitled(api_client):
            if not api_client.is_connected():
                try:
                    api_client.command("io.connect")
                except APIError as error:
                    assert _PRO_REQUIRED not in error.message, error.message
            return

        assert not api_client.is_connected(), (
            "loading Pro content into a live session without an entitlement must end "
            "the session (spec 0094 R2)"
        )
    finally:
        if api_client.is_connected():
            api_client.disconnect_device()
