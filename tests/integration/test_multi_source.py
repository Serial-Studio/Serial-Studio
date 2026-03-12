"""
Multi-Source Integration Tests

Tests for the multi-device / multi-source API: project.source.* commands.
Covers GPL vs. Commercial behavior, source CRUD, per-source configuration,
frame parser round-trips, and export/import fidelity.

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

def _is_commercial(api_client) -> bool:
    """Return True when connected to a Commercial build of Serial Studio."""
    try:
        api_client.source_add()
        return True
    except APIError as e:
        if "commercial" in e.message.lower() or "license" in e.message.lower():
            return False
        raise


def _new_project(api_client, title: str = "Multi-Source Test") -> None:
    """Create a fresh project in ProjectFile mode."""
    api_client.command("project.file.new")
    time.sleep(0.2)
    try:
        api_client.command("project.setTitle", {"title": title})
    except Exception:
        pass
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="",
        operation_mode=0,   # ProjectFile
    )
    time.sleep(0.1)


# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_source_list_default(api_client, clean_state):
    """New project always has exactly one source (sourceId 0)."""
    _new_project(api_client)

    sources = api_client.source_list()

    assert len(sources) >= 1, "Every project must have at least one source"
    ids = [s["sourceId"] for s in sources]
    assert 0 in ids, "Source 0 must always exist"


@pytest.mark.project
def test_source_add_commercial_only(api_client, clean_state):
    """source.add returns a new sourceId on Commercial, error on GPL."""
    _new_project(api_client)

    try:
        new_id = api_client.source_add()
        # Commercial build: must get a positive source ID
        assert new_id > 0, "New source ID must be > 0"
    except APIError as e:
        # GPL build: expect a clear 'commercial' / 'license' rejection
        assert any(
            word in e.message.lower() for word in ("commercial", "license", "pro")
        ), f"GPL rejection message should mention licensing, got: {e.message}"


@pytest.mark.project
def test_source_count_matches_list(api_client, clean_state):
    """After adding two sources, source.list returns 3 entries."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add is Commercial-only")

    _new_project(api_client)  # reset after probe add

    id1 = api_client.source_add()
    id2 = api_client.source_add()

    sources = api_client.source_list()
    assert len(sources) == 3, f"Expected 3 sources, got {len(sources)}"

    ids = {s["sourceId"] for s in sources}
    assert 0 in ids
    assert id1 in ids
    assert id2 in ids


@pytest.mark.project
def test_source_delete_disallows_primary(api_client, clean_state):
    """Deleting source 0 (primary) must always fail."""
    _new_project(api_client)

    with pytest.raises(APIError) as exc_info:
        api_client.source_delete(0)

    assert exc_info.value.code != "OK", "Deleting source 0 should raise an error"


@pytest.mark.project
def test_source_update_title(api_client, clean_state):
    """source.update with a new title is reflected in source.list."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add/update is Commercial-only")

    _new_project(api_client)

    new_id = api_client.source_add()
    api_client.source_update(new_id, title="Renamed Device")
    time.sleep(0.1)

    sources = api_client.source_list()
    match = next((s for s in sources if s["sourceId"] == new_id), None)
    assert match is not None, f"Source {new_id} not found after update"
    assert match["title"] == "Renamed Device", (
        f"Expected title 'Renamed Device', got '{match['title']}'"
    )


@pytest.mark.project
def test_source_update_bus_type(api_client, clean_state):
    """source.update with a new busType is persisted in getConfiguration."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add/update is Commercial-only")

    _new_project(api_client)

    new_id = api_client.source_add()
    api_client.source_update(new_id, busType=1)  # 1 = Network
    time.sleep(0.1)

    cfg = api_client.source_get_configuration(new_id)
    assert cfg.get("busType") == 1, (
        f"Expected busType 1 (Network), got {cfg.get('busType')}"
    )


@pytest.mark.project
def test_source_update_frame_delimiters(api_client, clean_state):
    """source.update with frameStart/frameEnd is persisted in getConfiguration."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add/update is Commercial-only")

    _new_project(api_client)

    new_id = api_client.source_add()
    api_client.source_update(new_id, frameStart="$", frameEnd="\n")
    time.sleep(0.1)

    cfg = api_client.source_get_configuration(new_id)
    assert cfg.get("frameStart") == "$", (
        f"Expected frameStart '$', got '{cfg.get('frameStart')}'"
    )
    assert cfg.get("frameEnd") == "\n", (
        f"Expected frameEnd '\\n', got repr {repr(cfg.get('frameEnd'))}"
    )


@pytest.mark.project
def test_source_set_property(api_client, clean_state):
    """source.setProperty persists a driver property in connectionSettings."""
    _new_project(api_client)

    cfg_before = api_client.source_get_configuration(0)
    settings_before = cfg_before.get("connectionSettings", {})

    api_client.source_set_property(0, "baudRate", 115200)
    time.sleep(0.1)

    cfg_after = api_client.source_get_configuration(0)
    settings_after = cfg_after.get("connectionSettings", {})

    assert settings_after.get("baudRate") == 115200, (
        f"baudRate should be 115200 after setProperty, got {settings_after.get('baudRate')}"
    )


@pytest.mark.project
def test_source_frame_parser_roundtrip(api_client, clean_state):
    """setFrameParserCode / getFrameParserCode round-trip preserves code exactly."""
    _new_project(api_client)

    js_code = (
        "function parse(frame) {\n"
        "  return frame.split(',');\n"
        "}\n"
    )

    api_client.source_set_frame_parser_code(0, js_code)
    time.sleep(0.1)

    returned = api_client.source_get_frame_parser_code(0)
    assert returned == js_code, (
        f"Frame parser code round-trip mismatch.\n"
        f"Expected: {repr(js_code)}\n"
        f"Got:      {repr(returned)}"
    )


@pytest.mark.project
def test_source_get_configuration_has_expected_keys(api_client, clean_state):
    """source.getConfiguration returns all mandatory fields for source 0."""
    _new_project(api_client)

    cfg = api_client.source_get_configuration(0)

    required_keys = {
        "sourceId",
        "title",
        "busType",
        "frameStart",
        "frameEnd",
        "checksumAlgorithm",
        "frameDetection",
        "decoderMethod",
    }
    missing = required_keys - cfg.keys()
    assert not missing, f"source.getConfiguration missing keys: {missing}"
    assert cfg["sourceId"] == 0


@pytest.mark.project
def test_source_list_has_expected_fields(api_client, clean_state):
    """Every entry in source.list includes the required summary fields."""
    _new_project(api_client)

    sources = api_client.source_list()
    assert len(sources) >= 1

    required_keys = {"sourceId", "title", "busType"}
    for src in sources:
        missing = required_keys - src.keys()
        assert not missing, (
            f"Source entry missing keys {missing}: {src}"
        )


@pytest.mark.project
def test_source_delete_valid_source(api_client, clean_state):
    """Deleting a non-primary source succeeds (Commercial build)."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add/delete is Commercial-only")

    _new_project(api_client)

    new_id = api_client.source_add()
    sources_before = api_client.source_list()
    count_before = len(sources_before)

    api_client.source_delete(new_id)
    time.sleep(0.1)

    sources_after = api_client.source_list()
    assert len(sources_after) == count_before - 1, (
        f"Expected {count_before - 1} sources after delete, got {len(sources_after)}"
    )
    ids_after = {s["sourceId"] for s in sources_after}
    assert new_id not in ids_after, f"Deleted source {new_id} still present in list"


@pytest.mark.project
def test_source_update_checksum_preserved(api_client, clean_state):
    """Per-source checksumAlgorithm survives an update and is reflected in getConfiguration."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("source.add/update is Commercial-only")

    _new_project(api_client)

    new_id = api_client.source_add()
    api_client.source_update(new_id, checksumAlgorithm="CRC-16")
    time.sleep(0.1)

    cfg = api_client.source_get_configuration(new_id)
    assert cfg.get("checksumAlgorithm") == "CRC-16", (
        f"Expected checksumAlgorithm 'CRC-16', got '{cfg.get('checksumAlgorithm')}'"
    )


@pytest.mark.project
def test_export_reimport_multi_source(api_client, clean_state):
    """Export + re-import of a 3-source project preserves source count (Commercial)."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    _new_project(api_client)

    id1 = api_client.source_add()
    id2 = api_client.source_add()
    api_client.source_update(id1, title="Sensor A")
    api_client.source_update(id2, title="Sensor B")
    time.sleep(0.2)

    exported = api_client.command("project.exportJson")
    config = exported["config"]

    api_client.command("project.file.new")
    time.sleep(0.2)
    api_client.load_project_from_json(config)
    time.sleep(0.3)

    sources = api_client.source_list()
    assert len(sources) == 3, (
        f"Re-imported project should have 3 sources, got {len(sources)}"
    )


@pytest.mark.project
def test_gpl_multi_source_file_truncates_to_one(api_client, clean_state):
    """
    On a GPL build, loading a project with >1 source must silently truncate to 1.
    On a Commercial build this test is skipped.
    """
    _new_project(api_client)

    # Build a 3-source config manually
    config = {
        "title": "Multi-Source GPL Test",
        "frameParser": {
            "startSequence": "/*",
            "endSequence": "*/",
            "checksumAlgorithm": "",
            "operationMode": 0,
            "frameDetection": 0,
        },
        "groups": [],
        "sources": [
            {"sourceId": 0, "title": "Device 1", "busType": 0},
            {"sourceId": 1, "title": "Device 2", "busType": 0},
            {"sourceId": 2, "title": "Device 3", "busType": 0},
        ],
    }

    try:
        api_client.load_project_from_json(config)
    except Exception:
        pass

    time.sleep(0.3)
    sources = api_client.source_list()

    if len(sources) == 3:
        pytest.skip("Commercial build — truncation test not applicable")

    assert len(sources) == 1, (
        f"GPL build must truncate multi-source project to 1 source, got {len(sources)}"
    )
    assert sources[0]["sourceId"] == 0


@pytest.mark.project
def test_source_frame_parser_independent_per_source(api_client, clean_state):
    """Each source stores its own frame parser code independently."""
    _new_project(api_client)

    if not _is_commercial(api_client):
        pytest.skip("Multi-source requires Commercial build")

    _new_project(api_client)

    new_id = api_client.source_add()

    code_0 = "function parse(f) { return f.split(','); }"
    code_1 = "function parse(f) { return f.split(';'); }"

    api_client.source_set_frame_parser_code(0, code_0)
    api_client.source_set_frame_parser_code(new_id, code_1)
    time.sleep(0.1)

    returned_0 = api_client.source_get_frame_parser_code(0)
    returned_1 = api_client.source_get_frame_parser_code(new_id)

    assert returned_0 == code_0, (
        f"Source 0 frame parser mismatch: {repr(returned_0)}"
    )
    assert returned_1 == code_1, (
        f"Source {new_id} frame parser mismatch: {repr(returned_1)}"
    )
    assert returned_0 != returned_1, "Two sources must not share frame parser state"
