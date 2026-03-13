"""
Project Save / Widget-Settings Persistence Tests

Verifies that widgetSettings (including the merged dashboardLayout and
activeGroupId sub-keys) survive save round-trips and connect/disconnect cycles.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time
from pathlib import Path

import pytest

from utils import DataGenerator, ChecksumType

_WS_KEY = "widgetSettings"
_ACTIVE_GROUP_SUBKEY = "activeGroup"


def _layout_key(group_id: int) -> str:
    return f"layout:{group_id}"


_SAMPLE_WIDGET_SETTINGS = {
    "1001": {"interpolate": True, "showArea": False},
    "1002": {"visibleCurves": [True, True]},
}

_SAMPLE_LAYOUT = {
    "windows": [{"id": 0, "x": 0, "y": 0, "w": 400, "h": 300}]
}


def _base_project(extra: dict | None = None) -> dict:
    """Return a valid project dict using the standard CSV parser template."""
    project = DataGenerator.generate_project_with_frame_delimiters(
        start="/*",
        end="*/",
        detection_mode=1,
        title="Save-Test Project",
        checksum_algorithm="None",
    )
    if extra:
        project.update(extra)
    return project


def _write_project(path: Path, extra: dict | None = None) -> Path:
    path.write_text(json.dumps(_base_project(extra), indent=2), encoding="utf-8")
    return path


def _connect_and_send(api_client, device_simulator):
    api_client.configure_frame_parser(
        start_sequence="/*",
        end_sequence="*/",
        checksum_algorithm="None",
        operation_mode=0,
        frame_detection=1,
    )
    time.sleep(0.1)
    api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
    api_client.connect_device()
    assert device_simulator.wait_for_connection(timeout=5.0)
    time.sleep(0.3)

    frames = [
        DataGenerator.wrap_frame(
            ",".join(str(float(j + i)) for j in range(6)),
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(5)
    ]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.0)


@pytest.mark.project
def test_project_file_save_and_reload(api_client, clean_state, tmp_path):
    """project.file.save writes a valid JSON file that can be reopened."""
    proj_path = tmp_path / "save_test.ssproj"
    _write_project(proj_path)

    open_result = api_client.command("project.file.open", {"filePath": str(proj_path)})
    assert open_result.get("filePath")
    time.sleep(0.3)

    save_result = api_client.command("project.file.save")
    assert save_result.get("saved") is True
    assert save_result.get("filePath")
    time.sleep(0.2)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    assert "title" in data
    assert "groups" in data


@pytest.mark.project
def test_widget_settings_persisted_in_saved_file(api_client, clean_state, tmp_path):
    """widgetSettings survive a save round-trip unchanged."""
    proj_path = tmp_path / "ws_persist.ssproj"
    _write_project(proj_path, extra={_WS_KEY: _SAMPLE_WIDGET_SETTINGS})

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    api_client.command("project.file.save")
    time.sleep(0.2)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    assert _WS_KEY in data
    assert data[_WS_KEY] == _SAMPLE_WIDGET_SETTINGS


@pytest.mark.project
def test_widget_settings_survive_connect_disconnect(
    api_client, device_simulator, clean_state, tmp_path
):
    """
    Regression: widgetSettings must not be erased when the device disconnects.

    Before the fix, Taskbar::saveLayout() on connectedChanged(false) called
    finalizeProjectSave() with stale in-memory state, dropping widgetSettings.
    """
    proj_path = tmp_path / "ws_survive_disconnect.ssproj"
    _write_project(proj_path, extra={_WS_KEY: _SAMPLE_WIDGET_SETTINGS})

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)

    _connect_and_send(api_client, device_simulator)
    api_client.disconnect_device()
    time.sleep(0.5)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    assert _WS_KEY in data, "widgetSettings must survive connect/disconnect"
    assert data[_WS_KEY] == _SAMPLE_WIDGET_SETTINGS, (
        f"widgetSettings values must be unchanged after disconnect; got {data[_WS_KEY]}"
    )


@pytest.mark.project
def test_layout_stored_inside_widget_settings(api_client, clean_state, tmp_path):
    """
    dashboardLayout and activeGroupId are migrated into widgetSettings on open.

    Legacy top-level keys are folded into per-group layout keys of the form
    "layout:<groupId>" and the activeGroup sub-key.
    """
    group_id = 42
    legacy_project = _base_project()
    legacy_project["dashboardLayout"] = _SAMPLE_LAYOUT
    legacy_project["activeGroupId"] = group_id

    proj_path = tmp_path / "legacy_layout.ssproj"
    proj_path.write_text(json.dumps(legacy_project, indent=2), encoding="utf-8")

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    api_client.command("project.file.save")
    time.sleep(0.2)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    assert _WS_KEY in data, "widgetSettings must be present after save"
    ws = data[_WS_KEY]
    layout_key = _layout_key(group_id)
    assert layout_key in ws, f"{layout_key} must be inside widgetSettings"
    assert _ACTIVE_GROUP_SUBKEY in ws, "activeGroup sub-key must be inside widgetSettings"
    assert ws[layout_key] == _SAMPLE_LAYOUT
    assert ws[_ACTIVE_GROUP_SUBKEY] == group_id


@pytest.mark.project
def test_layout_and_widget_settings_coexist(api_client, clean_state, tmp_path):
    """Per-group layout keys and numeric widget settings coexist in widgetSettings."""
    group_id = 1
    ws = dict(_SAMPLE_WIDGET_SETTINGS)
    ws[_layout_key(group_id)] = _SAMPLE_LAYOUT
    ws[_ACTIVE_GROUP_SUBKEY] = group_id

    proj_path = tmp_path / "coexist.ssproj"
    _write_project(proj_path, extra={_WS_KEY: ws})

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    api_client.command("project.file.save")
    time.sleep(0.2)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    saved_ws = data.get(_WS_KEY, {})
    assert saved_ws.get(_layout_key(group_id)) == _SAMPLE_LAYOUT
    assert saved_ws.get(_ACTIVE_GROUP_SUBKEY) == group_id
    assert saved_ws.get("1001") == _SAMPLE_WIDGET_SETTINGS["1001"]
    assert saved_ws.get("1002") == _SAMPLE_WIDGET_SETTINGS["1002"]


@pytest.mark.project
def test_layout_survives_connect_disconnect(
    api_client, device_simulator, clean_state, tmp_path
):
    """Per-group layout keys inside widgetSettings must survive a disconnect cycle."""
    group_id = 5
    ws = {_layout_key(group_id): _SAMPLE_LAYOUT, _ACTIVE_GROUP_SUBKEY: group_id}
    proj_path = tmp_path / "layout_survive.ssproj"
    _write_project(proj_path, extra={_WS_KEY: ws})

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)

    _connect_and_send(api_client, device_simulator)
    api_client.disconnect_device()
    time.sleep(0.5)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    saved_ws = data.get(_WS_KEY, {})
    assert saved_ws.get(_layout_key(group_id)) == _SAMPLE_LAYOUT, (
        "per-group layout key must survive disconnect"
    )
    assert saved_ws.get(_ACTIVE_GROUP_SUBKEY) == group_id, (
        "activeGroup sub-key must survive disconnect"
    )


@pytest.mark.project
def test_exported_json_matches_file_on_disk(api_client, clean_state, tmp_path):
    """project.exportJson and project.file.save must agree on widgetSettings."""
    proj_path = tmp_path / "export_vs_disk.ssproj"
    _write_project(proj_path, extra={_WS_KEY: _SAMPLE_WIDGET_SETTINGS})

    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)

    exported = api_client.command("project.exportJson")["config"]
    api_client.command("project.file.save")
    time.sleep(0.2)

    disk = json.loads(proj_path.read_text(encoding="utf-8"))
    assert exported.get(_WS_KEY) == disk.get(_WS_KEY), (
        "exportJson and file.save must agree on widgetSettings"
    )
    assert len(exported.get("groups", [])) == len(disk.get("groups", []))
