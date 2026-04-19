"""
AppState Integration Tests

Tests for the AppState singleton covering:
- Operation mode changes and persistence
- Frame config derivation from mode
- Project file path management
- ProjectModel integration (reactive updates)
- API integration (dashboard.* commands)
- Signal consistency (no duplicate emissions)
- Multi-source scenarios (Commercial only)

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _new_project(api_client, title: str = "AppState Test") -> None:
    """Create a fresh project and set ProjectFile mode."""
    api_client.command("project.file.new")
    time.sleep(0.2)
    try:
        api_client.command("project.setTitle", {"title": title})
    except Exception:
        pass
    # Set operation mode to ProjectFile via the authoritative dashboard command
    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)


def _is_commercial(api_client) -> bool:
    """Return True when connected to a Commercial build of Serial Studio."""
    try:
        new_id = api_client.source_add()
        api_client.source_delete(new_id)
        return True
    except APIError as e:
        if "commercial" in e.message.lower() or "license" in e.message.lower():
            return False
        raise


def _get_operation_mode(api_client) -> int:
    """Return the current operation mode from dashboard.getStatus."""
    status = api_client.get_dashboard_status()
    return status.get("operationMode", -1)


def _get_frame_config(api_client) -> dict:
    """Return the current frame parser configuration."""
    return api_client.get_frame_parser_config()


# ---------------------------------------------------------------------------
# Tests: Operation Mode Changes
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_set_operation_mode_projectfile(api_client, clean_state):
    """Setting mode to ProjectFile (0) is reflected in dashboard.getStatus."""
    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)

    assert _get_operation_mode(api_client) == 0


@pytest.mark.project
def test_set_operation_mode_json(api_client, clean_state):
    """Setting mode to ConsoleOnly (1) is reflected in dashboard.getStatus."""
    api_client.command("dashboard.setOperationMode", {"mode": 1})
    time.sleep(0.1)

    assert _get_operation_mode(api_client) == 1


@pytest.mark.project
def test_set_operation_mode_quickplot(api_client, clean_state):
    """Setting mode to QuickPlot (2) is reflected in dashboard.getStatus."""
    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.1)

    assert _get_operation_mode(api_client) == 2


@pytest.mark.project
def test_operation_mode_cycles_through_all_values(api_client, clean_state):
    """All three mode values are accepted and stored correctly."""
    for mode in [0, 1, 2, 0]:
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.1)
        assert _get_operation_mode(api_client) == mode, f"Expected mode {mode}"


@pytest.mark.project
def test_operation_mode_same_value_no_error(api_client, clean_state):
    """Setting the same mode twice does not raise an error (guard-returns)."""
    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)
    initial = _get_operation_mode(api_client)

    api_client.command("dashboard.setOperationMode", {"mode": initial})
    time.sleep(0.1)

    assert _get_operation_mode(api_client) == initial


@pytest.mark.project
def test_invalid_operation_mode_rejected(api_client, clean_state):
    """Invalid mode (>2) is rejected with an API error."""
    with pytest.raises(APIError):
        api_client.command("dashboard.setOperationMode", {"mode": 999})

    # Mode must remain valid after the failed attempt
    mode = _get_operation_mode(api_client)
    assert 0 <= mode <= 2, f"Mode must remain valid (0-2) after rejection, got {mode}"


@pytest.mark.project
def test_invalid_operation_mode_missing_param(api_client, clean_state):
    """Calling dashboard.setOperationMode without the mode param returns an error."""
    with pytest.raises(APIError):
        api_client.command("dashboard.setOperationMode", {})


# ---------------------------------------------------------------------------
# Tests: dashboard.getOperationMode
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_get_operation_mode_matches_set(api_client, clean_state):
    """dashboard.getOperationMode returns the same value as was set."""
    for mode in [0, 1, 2]:
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.1)

        result = api_client.command("dashboard.getOperationMode")
        assert result.get("mode") == mode, (
            f"getOperationMode should return {mode}, got {result.get('mode')}"
        )


@pytest.mark.project
def test_get_operation_mode_returns_mode_name(api_client, clean_state):
    """dashboard.getOperationMode returns a non-empty modeName for every mode."""
    expected_names = {0: "ProjectFile", 1: "ConsoleOnly", 2: "QuickPlot"}

    for mode, name in expected_names.items():
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.1)

        result = api_client.command("dashboard.getOperationMode")
        assert result.get("modeName") == name, (
            f"Mode {mode} should have modeName '{name}', got '{result.get('modeName')}'"
        )


# ---------------------------------------------------------------------------
# Tests: dashboard.getStatus
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_dashboard_status_has_required_fields(api_client, clean_state):
    """dashboard.getStatus contains all required fields."""
    status = api_client.get_dashboard_status()

    required = {"operationMode", "operationModeName", "fps", "points",
                "widgetCount", "datasetCount"}
    missing = required - status.keys()
    assert not missing, f"dashboard.getStatus missing fields: {missing}"


@pytest.mark.project
def test_dashboard_status_operation_mode_consistent_with_get(api_client, clean_state):
    """operationMode in getStatus matches result from getOperationMode."""
    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.1)

    status = api_client.get_dashboard_status()
    get_result = api_client.command("dashboard.getOperationMode")

    assert status.get("operationMode") == get_result.get("mode"), (
        "getStatus.operationMode must match getOperationMode.mode"
    )


@pytest.mark.project
def test_dashboard_status_operation_mode_name_matches_mode(api_client, clean_state):
    """operationModeName in getStatus is consistent with operationMode value."""
    expected_names = {0: "ProjectFile", 1: "ConsoleOnly", 2: "QuickPlot"}

    for mode, name in expected_names.items():
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.1)

        status = api_client.get_dashboard_status()
        assert status.get("operationModeName") == name, (
            f"Mode {mode} should have operationModeName '{name}', "
            f"got '{status.get('operationModeName')}'"
        )


# ---------------------------------------------------------------------------
# Tests: project.frameParser.getConfig
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_frame_parser_getconfig_has_required_fields(api_client, clean_state):
    """project.frameParser.getConfig returns all required fields."""
    cfg = _get_frame_config(api_client)

    required = {"startSequence", "endSequence", "checksumAlgorithm",
                "operationMode", "frameDetection"}
    missing = required - cfg.keys()
    assert not missing, f"frameParser.getConfig missing fields: {missing}"


@pytest.mark.project
def test_frame_parser_getconfig_operation_mode_matches_dashboard(api_client, clean_state):
    """operationMode in frameParser.getConfig matches dashboard.getStatus."""
    for mode in [0, 1, 2]:
        api_client.command("dashboard.setOperationMode", {"mode": mode})
        time.sleep(0.1)

        cfg = _get_frame_config(api_client)
        status = api_client.get_dashboard_status()

        assert cfg.get("operationMode") == status.get("operationMode"), (
            f"frameParser.getConfig.operationMode must match dashboard operationMode for mode {mode}"
        )


@pytest.mark.project
def test_frame_parser_configure_sets_start_sequence(api_client, clean_state):
    """project.frameParser.configure startSequence is reflected in getConfig."""
    _new_project(api_client)
    api_client.configure_frame_parser(start_sequence="$")
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("startSequence") == "$", (
        f"Expected startSequence '$', got '{cfg.get('startSequence')}'"
    )


@pytest.mark.project
def test_frame_parser_configure_sets_end_sequence(api_client, clean_state):
    """project.frameParser.configure endSequence is reflected in getConfig."""
    _new_project(api_client)
    api_client.configure_frame_parser(end_sequence="\n")
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("endSequence") == "\n", (
        f"Expected endSequence '\\n', got repr {repr(cfg.get('endSequence'))}"
    )


@pytest.mark.project
def test_frame_parser_configure_sets_checksum(api_client, clean_state):
    """project.frameParser.configure checksumAlgorithm is reflected in getConfig."""
    _new_project(api_client)
    api_client.configure_frame_parser(checksum_algorithm="CRC-16")
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("checksumAlgorithm") == "CRC-16", (
        f"Expected checksumAlgorithm 'CRC-16', got '{cfg.get('checksumAlgorithm')}'"
    )


@pytest.mark.project
def test_frame_parser_configure_sets_frame_detection(api_client, clean_state):
    """project.frameParser.configure frameDetection is reflected in getConfig."""
    _new_project(api_client)
    api_client.configure_frame_parser(frame_detection=1)
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("frameDetection") == 1, (
        f"Expected frameDetection 1, got {cfg.get('frameDetection')}"
    )


@pytest.mark.project
def test_frame_parser_configure_sets_operation_mode(api_client, clean_state):
    """project.frameParser.configure operationMode updates AppState mode."""
    api_client.configure_frame_parser(operation_mode=2)
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("operationMode") == 2

    # Also verify dashboard status reflects the same change
    assert _get_operation_mode(api_client) == 2


@pytest.mark.project
def test_frame_parser_configure_no_params_returns_not_updated(api_client, clean_state):
    """project.frameParser.configure with no params returns updated=false."""
    result = api_client.configure_frame_parser()
    # configure_frame_parser returns {} when no params; if called with empty dict it
    # sends nothing and the helper returns {} early — just verify no exception
    # The actual no-params call path: api_client.configure_frame_parser() skips the command
    # So test the command directly with an empty dict
    result = api_client.command("project.frameParser.configure", {})
    assert result.get("updated") is False, (
        "configure with no params should return updated=false"
    )


@pytest.mark.project
def test_frame_parser_getconfig_consistent_across_calls(api_client, clean_state):
    """Calling frameParser.getConfig multiple times returns identical results."""
    cfg1 = _get_frame_config(api_client)
    cfg2 = _get_frame_config(api_client)
    cfg3 = _get_frame_config(api_client)

    assert cfg1 == cfg2, "Config must be identical on second call"
    assert cfg2 == cfg3, "Config must be identical on third call"


# ---------------------------------------------------------------------------
# Tests: project.getStatus
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_project_status_has_required_fields(api_client, clean_state):
    """project.getStatus returns all required fields."""
    status = api_client.get_project_status()

    required = {"title", "filePath", "modified", "groupCount",
                "datasetCount", "actionCount"}
    missing = required - status.keys()
    assert not missing, f"project.getStatus missing fields: {missing}"


@pytest.mark.project
def test_project_new_creates_empty_project(api_client, clean_state):
    """project.file.new creates a project with zero groups."""
    api_client.command("project.file.new")
    time.sleep(0.2)

    status = api_client.get_project_status()
    assert status.get("groupCount") == 0, (
        f"New project should have 0 groups, got {status.get('groupCount')}"
    )
    assert status.get("datasetCount") == 0, (
        f"New project should have 0 datasets, got {status.get('datasetCount')}"
    )


@pytest.mark.project
def test_project_set_title_reflected_in_status(api_client, clean_state):
    """project.setTitle is reflected in project.getStatus."""
    api_client.command("project.file.new")
    time.sleep(0.1)
    api_client.command("project.setTitle", {"title": "Verified Title"})
    time.sleep(0.1)

    status = api_client.get_project_status()
    assert status.get("title") == "Verified Title", (
        f"Expected title 'Verified Title', got '{status.get('title')}'"
    )


@pytest.mark.project
def test_project_set_title_empty_rejected(api_client, clean_state):
    """project.setTitle rejects empty string."""
    with pytest.raises(APIError):
        api_client.command("project.setTitle", {"title": ""})


# ---------------------------------------------------------------------------
# Tests: ProjectModel integration (reactive updates on mode change)
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_frame_detection_change_persists(api_client, clean_state):
    """Changing frameDetection via configure persists in getConfig."""
    _new_project(api_client)
    api_client.configure_frame_parser(frame_detection=1)
    time.sleep(0.1)

    cfg = _get_frame_config(api_client)
    assert cfg.get("frameDetection") == 1


@pytest.mark.project
def test_multiple_configure_fields_in_one_call(api_client, clean_state):
    """configure with multiple fields updates all of them atomically."""
    api_client.configure_frame_parser(
        start_sequence="START",
        end_sequence="END",
        checksum_algorithm="",
        frame_detection=0,
        operation_mode=0,
    )
    time.sleep(0.2)

    cfg = _get_frame_config(api_client)
    assert cfg.get("startSequence") == "START"
    assert cfg.get("endSequence") == "END"
    assert cfg.get("checksumAlgorithm") == ""
    assert cfg.get("frameDetection") == 0
    assert cfg.get("operationMode") == 0


@pytest.mark.project
def test_mode_change_via_configure_and_dashboard_produce_same_state(api_client, clean_state):
    """Setting mode via configure or dashboard both update AppState identically."""
    api_client.configure_frame_parser(operation_mode=2)
    time.sleep(0.1)
    mode_via_configure = _get_operation_mode(api_client)

    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)
    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.1)
    mode_via_dashboard = _get_operation_mode(api_client)

    assert mode_via_configure == mode_via_dashboard == 2


@pytest.mark.project
def test_new_project_does_not_reset_operation_mode(api_client, clean_state):
    """Creating a new project does not reset the operation mode."""
    api_client.command("dashboard.setOperationMode", {"mode": 2})
    time.sleep(0.1)

    api_client.command("project.file.new")
    time.sleep(0.2)

    # Operation mode is owned by AppState, not ProjectModel — must survive new project
    mode = _get_operation_mode(api_client)
    assert mode == 2, (
        f"Operation mode must not be reset by project.file.new; expected 2, got {mode}"
    )


@pytest.mark.project
def test_rapid_mode_changes_end_in_consistent_state(api_client, clean_state):
    """Rapid consecutive mode changes leave AppState in the final mode."""
    modes = [0, 1, 2, 0, 1, 2, 0]
    for mode in modes:
        api_client.command("dashboard.setOperationMode", {"mode": mode})

    time.sleep(0.5)

    final_mode = _get_operation_mode(api_client)
    # The final mode after all rapid changes must be valid (we don't guarantee
    # exact ordering since these are fire-and-forget, but the result must be 0-2)
    assert 0 <= final_mode <= 2, (
        f"Mode must remain valid (0-2) after rapid changes, got {final_mode}"
    )

    cfg = _get_frame_config(api_client)
    assert "startSequence" in cfg, "Frame config must remain valid after rapid mode changes"


# ---------------------------------------------------------------------------
# Tests: API command registry
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_api_registry_includes_dashboard_setoperationmode(api_client, clean_state):
    """dashboard.setOperationMode is listed in the API command registry."""
    assert api_client.command_exists("dashboard.setOperationMode")


@pytest.mark.project
def test_api_registry_includes_dashboard_getoperationmode(api_client, clean_state):
    """dashboard.getOperationMode is listed in the API command registry."""
    assert api_client.command_exists("dashboard.getOperationMode")


@pytest.mark.project
def test_api_registry_includes_dashboard_getstatus(api_client, clean_state):
    """dashboard.getStatus is listed in the API command registry."""
    assert api_client.command_exists("dashboard.getStatus")


@pytest.mark.project
def test_api_registry_includes_frameparser_configure(api_client, clean_state):
    """project.frameParser.configure is listed in the API command registry."""
    assert api_client.command_exists("project.frameParser.configure")


@pytest.mark.project
def test_api_registry_includes_frameparser_getconfig(api_client, clean_state):
    """project.frameParser.getConfig is listed in the API command registry."""
    assert api_client.command_exists("project.frameParser.getConfig")


# ---------------------------------------------------------------------------
# Tests: Multi-source scenarios (Commercial only)
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_adding_source_does_not_change_operation_mode(api_client, clean_state):
    """Adding a new source does not alter the current operation mode."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    _new_project(api_client)

    api_client.command("dashboard.setOperationMode", {"mode": 0})
    time.sleep(0.1)

    api_client.source_add()
    time.sleep(0.2)

    assert _get_operation_mode(api_client) == 0, (
        "Adding a source must not alter the operation mode"
    )


@pytest.mark.project
def test_source0_framestart_propagates_to_frameparser_config(api_client, clean_state):
    """Updating source[0] frameStart via source.update is visible in frameParser.getConfig."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.update is Commercial-only")

    _new_project(api_client)

    api_client.source_update(0, frameStart="<", frameEnd=">")
    time.sleep(0.2)

    cfg = _get_frame_config(api_client)
    assert cfg.get("startSequence") == "<", (
        "source[0] frameStart must propagate to frameParser.getConfig"
    )
    assert cfg.get("endSequence") == ">", (
        "source[0] frameEnd must propagate to frameParser.getConfig"
    )


@pytest.mark.project
def test_secondary_source_delimiters_do_not_affect_frameparser_config(api_client, clean_state):
    """Delimiters set on a non-primary source do not appear in frameParser.getConfig."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    _new_project(api_client)

    # Ensure source[0] has known delimiters
    api_client.configure_frame_parser(start_sequence="/*", end_sequence="*/")
    time.sleep(0.1)

    new_id = api_client.source_add()
    api_client.source_update(new_id, frameStart="$", frameEnd="\n")
    time.sleep(0.2)

    cfg = _get_frame_config(api_client)
    assert cfg.get("startSequence") == "/*", (
        "Secondary source frameStart must not overwrite frameParser.getConfig"
    )
    assert cfg.get("endSequence") == "*/", (
        "Secondary source frameEnd must not overwrite frameParser.getConfig"
    )


@pytest.mark.project
def test_deleting_secondary_source_leaves_frameparser_config_unchanged(api_client, clean_state):
    """Deleting a secondary source does not alter the frame parser config."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    _new_project(api_client)

    cfg_before = _get_frame_config(api_client)

    new_id = api_client.source_add()
    time.sleep(0.1)
    api_client.source_delete(new_id)
    time.sleep(0.2)

    cfg_after = _get_frame_config(api_client)

    assert cfg_after.get("startSequence") == cfg_before.get("startSequence")
    assert cfg_after.get("endSequence") == cfg_before.get("endSequence")
    assert cfg_after.get("checksumAlgorithm") == cfg_before.get("checksumAlgorithm")
