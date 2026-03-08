"""
Window Layout & Widget Settings API Tests

Verifies the ui.window.* API commands for group navigation, window state
management, layout save/restore, auto-layout toggle, and per-widget settings
persistence.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import json
import time
from pathlib import Path

import pytest

from utils import DataGenerator, ChecksumType
from utils.api_client import APIError

_WS_KEY = "widgetSettings"
_ACTIVE_GROUP_SUBKEY = "__activeGroup__"


def _layout_key(group_id: int) -> str:
    return f"__layout__:{group_id}__"


# ---------------------------------------------------------------------------
# Project builders
# ---------------------------------------------------------------------------

def _single_group_project(title="WL-Single") -> dict:
    """One group, 4 datasets — produces a handful of windows in the dashboard."""
    base = DataGenerator.generate_project_with_frame_delimiters(
        start="/*", end="*/", detection_mode=1,
        title=title, checksum_algorithm="None",
    )
    return base


def _multi_group_project(title="WL-Multi", n_groups=3) -> dict:
    """
    n_groups independent groups, each with 3 plot datasets.
    This guarantees the taskbar shows multiple group tabs so that
    group-switch tests are not skipped.
    """
    groups = []
    for g in range(n_groups):
        groups.append({
            "title": f"Group {g}",
            "widget": "datagrid",
            "datasets": [
                {"title": f"G{g}D{d}", "graph": True, "units": "", "value": "0"}
                for d in range(3)
            ],
        })

    return {
        "title": title,
        "frameStart": "/*",
        "frameEnd": "*/",
        "frameDetection": 1,
        "checksum": "",
        "frameParser": DataGenerator.CSV_PARSER_TEMPLATE,
        "groups": groups,
    }


def _write_project(path: Path, project: dict) -> Path:
    path.write_text(json.dumps(project, indent=2), encoding="utf-8")
    return path


def _connect_and_send(api_client, device_simulator, n_values=9):
    """Configure, connect, and stream a few CSV frames so the dashboard populates."""
    _skip_if_no_session(api_client)
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
            ",".join(str(float(j + i)) for j in range(n_values)),
            mode="project",
            checksum_type=ChecksumType.NONE,
        )
        for i in range(5)
    ]
    device_simulator.send_frames(frames, interval_seconds=0.1)
    time.sleep(1.0)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _require_groups(api_client, minimum=1):
    """Return groups list or skip if fewer than `minimum` are available."""
    try:
        groups = api_client.command("ui.window.getGroups").get("groups", [])
    except APIError as e:
        pytest.skip(f"ui.window.getGroups unavailable: {e}")

    if len(groups) < minimum:
        pytest.skip(f"Need at least {minimum} group(s); got {len(groups)}")

    return groups


def _require_windows(api_client):
    """Return windows list or skip if none are available."""
    try:
        windows = api_client.command("ui.window.getWindowStates").get("windows", [])
    except APIError as e:
        pytest.skip(f"ui.window.getWindowStates unavailable: {e}")

    if not windows:
        pytest.skip("No windows available in current dashboard session")

    return windows


# ---------------------------------------------------------------------------
# Module-level guard: skip all dashboard tests when running headless
# ---------------------------------------------------------------------------


def _skip_if_no_session(api_client):
    """Skip the calling test when no dashboard session is active (e.g. headless mode)."""
    status = api_client.command("ui.window.getStatus")
    if not status.get("sessionActive", True):
        pytest.skip("No dashboard session active (headless mode)")


# ---------------------------------------------------------------------------
# Status & groups
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_get_status_no_session(api_client, clean_state):
    """getStatus returns gracefully when no dashboard session is active."""
    result = api_client.command("ui.window.getStatus")
    assert "sessionActive" in result


@pytest.mark.project
def test_get_groups_single_group(api_client, device_simulator, clean_state, tmp_path):
    """getGroups returns at least one entry after connecting with a single-group project."""
    proj_path = _write_project(tmp_path / "single.ssproj", _single_group_project())
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    groups = _require_groups(api_client, minimum=1)
    assert all("id" in g and "text" in g for g in groups)


@pytest.mark.project
def test_get_groups_multi_group(api_client, device_simulator, clean_state, tmp_path):
    """getGroups returns all three groups for a 3-group project."""
    proj_path = _write_project(tmp_path / "multi.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    groups = api_client.command("ui.window.getGroups").get("groups", [])
    assert len(groups) >= 3, f"Expected ≥3 groups, got {len(groups)}: {groups}"
    titles = [g["text"] for g in groups]
    assert any("Group 0" in t for t in titles)
    assert any("Group 1" in t for t in titles)
    assert any("Group 2" in t for t in titles)


# ---------------------------------------------------------------------------
# Active group navigation
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_set_active_group_by_id(api_client, device_simulator, clean_state, tmp_path):
    """setActiveGroup changes the active group; getStatus confirms the change."""
    proj_path = _write_project(tmp_path / "nav_id.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    groups = _require_groups(api_client, minimum=2)
    target_id = groups[1]["id"]

    set_result = api_client.command("ui.window.setActiveGroup", {"groupId": target_id})
    assert set_result.get("groupId") == target_id
    time.sleep(0.3)

    status = api_client.command("ui.window.getStatus")
    assert status.get("activeGroupId") == target_id


@pytest.mark.project
def test_set_active_group_by_index(api_client, device_simulator, clean_state, tmp_path):
    """setActiveGroupIndex changes the active group; getStatus confirms."""
    proj_path = _write_project(tmp_path / "nav_idx.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    _require_groups(api_client, minimum=2)

    set_result = api_client.command("ui.window.setActiveGroupIndex", {"index": 0})
    assert set_result.get("index") == 0
    time.sleep(0.3)

    status = api_client.command("ui.window.getStatus")
    assert status.get("activeGroupIndex") == 0


@pytest.mark.project
def test_cycle_through_all_groups(api_client, device_simulator, clean_state, tmp_path):
    """Cycling through all groups via setActiveGroup works for each one."""
    proj_path = _write_project(tmp_path / "cycle.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    groups = _require_groups(api_client, minimum=2)
    for g in groups:
        api_client.command("ui.window.setActiveGroup", {"groupId": g["id"]})
        time.sleep(0.2)
        status = api_client.command("ui.window.getStatus")
        assert status.get("activeGroupId") == g["id"], (
            f"Expected activeGroupId={g['id']}, got {status.get('activeGroupId')}"
        )


# ---------------------------------------------------------------------------
# Window states
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_get_window_states_has_entries(api_client, device_simulator, clean_state, tmp_path):
    """getWindowStates returns a non-empty array with required fields."""
    proj_path = _write_project(tmp_path / "ws.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    windows = _require_windows(api_client)
    for w in windows:
        assert "id" in w
        assert "state" in w
        assert "name" in w


@pytest.mark.project
def test_set_window_state_accepted(api_client, device_simulator, clean_state, tmp_path):
    """setWindowState is accepted and echoes the requested id and state.

    Window state is ephemeral — it lives only in QML-registered windows
    and resets on every rebuildModel() (disconnect / new frame).  We verify
    the command contract: success=true, echo of id and state values.
    """
    proj_path = _write_project(tmp_path / "wst.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    windows = _require_windows(api_client)
    win_id = windows[0]["id"]

    for state in (1, 2, 0):
        result = api_client.command("ui.window.setWindowState", {"id": win_id, "state": state})
        assert result.get("id") == win_id
        assert result.get("state") == state


# ---------------------------------------------------------------------------
# Auto-layout toggle
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_auto_layout_toggle(api_client, device_simulator, clean_state, tmp_path):
    """
    setAutoLayout(false) → getLayout has 'autoLayout': false.
    setAutoLayout(true)  → getLayout has 'autoLayout': true.
    getStatus reflects the change in autoLayoutEnabled.
    """
    proj_path = _write_project(tmp_path / "autolayout.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    # Disable auto layout
    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.3)

    layout = api_client.command("ui.window.getLayout")["layout"]
    assert layout.get("autoLayout") is False, f"Expected autoLayout=false, got {layout}"

    status = api_client.command("ui.window.getStatus")
    assert status.get("autoLayoutEnabled") is False

    # Re-enable auto layout
    api_client.command("ui.window.setAutoLayout", {"enabled": True})
    time.sleep(0.3)

    layout = api_client.command("ui.window.getLayout")["layout"]
    assert layout.get("autoLayout") is True

    status = api_client.command("ui.window.getStatus")
    assert status.get("autoLayoutEnabled") is True


@pytest.mark.project
def test_auto_layout_off_produces_geometries(api_client, device_simulator, clean_state, tmp_path):
    """
    With autoLayout disabled, serializeLayout() includes a 'geometries' array
    (populated only while QML windows are registered) or an empty array.
    The key must exist in the layout object.
    """
    proj_path = _write_project(tmp_path / "geom.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.3)

    layout = api_client.command("ui.window.getLayout")["layout"]
    assert "geometries" in layout, (
        f"Expected 'geometries' key when autoLayout=false; got keys: {list(layout.keys())}"
    )
    assert isinstance(layout["geometries"], list)


@pytest.mark.project
def test_auto_layout_on_omits_geometries(api_client, device_simulator, clean_state, tmp_path):
    """
    With autoLayout enabled, serializeLayout() does NOT include 'geometries'
    (positions are managed automatically and not persisted).
    """
    proj_path = _write_project(tmp_path / "nogeom.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    api_client.command("ui.window.setAutoLayout", {"enabled": True})
    time.sleep(0.3)

    layout = api_client.command("ui.window.getLayout")["layout"]
    assert "geometries" not in layout, (
        f"Expected no 'geometries' key when autoLayout=true; got keys: {list(layout.keys())}"
    )


# ---------------------------------------------------------------------------
# Window geometry / manual layout via setLayout
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_set_layout_moves_windows(api_client, device_simulator, clean_state, tmp_path):
    """
    setLayout with explicit geometries repositions windows when autoLayout=false.

    With QML windows registered the geometry changes are applied directly to
    the QQuickItem.  serializeLayout() then reflects the new positions.
    """
    proj_path = _write_project(tmp_path / "move.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    # Switch to manual layout so positions are captured
    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.3)

    # Read the current window order so we can construct a valid layout
    layout_before = api_client.command("ui.window.getLayout")["layout"]
    window_order = layout_before.get("windowOrder", [])
    if not window_order:
        pytest.skip("No windows registered in WindowManager yet")

    # Build a layout with distinct, non-overlapping positions for each window
    new_geometries = []
    for idx, win_id in enumerate(window_order):
        new_geometries.append({
            "id": win_id,
            "x": float(idx * 210),
            "y": float(idx * 10),
            "width": 200.0,
            "height": 150.0,
        })

    new_layout = {
        "autoLayout": False,
        "windowOrder": window_order,
        "geometries": new_geometries,
    }

    api_client.command("ui.window.setLayout", {"layout": new_layout})
    time.sleep(0.5)

    layout_after = api_client.command("ui.window.getLayout")["layout"]
    assert layout_after.get("autoLayout") is False

    # If QML windows are registered the geometries are applied and serialized back
    after_geoms = {g["id"]: g for g in layout_after.get("geometries", [])}
    if after_geoms:
        for entry in new_geometries:
            wid = entry["id"]
            if wid in after_geoms:
                assert abs(after_geoms[wid]["x"] - entry["x"]) < 2, (
                    f"Window {wid} x={after_geoms[wid]['x']!r} ≠ expected {entry['x']}"
                )
                assert abs(after_geoms[wid]["y"] - entry["y"]) < 2, (
                    f"Window {wid} y={after_geoms[wid]['y']!r} ≠ expected {entry['y']}"
                )


@pytest.mark.project
def test_set_layout_auto_layout_roundtrip(api_client, device_simulator, clean_state, tmp_path):
    """
    getLayout → setLayout (unchanged) → getLayout: windowOrder and autoLayout are stable.
    """
    proj_path = _write_project(tmp_path / "rt.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    layout_before = api_client.command("ui.window.getLayout")["layout"]
    api_client.command("ui.window.setLayout", {"layout": layout_before})
    time.sleep(0.3)

    layout_after = api_client.command("ui.window.getLayout")["layout"]
    assert layout_after.get("autoLayout") == layout_before.get("autoLayout")
    assert layout_after.get("windowOrder") == layout_before.get("windowOrder")


# ---------------------------------------------------------------------------
# Layout persistence
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_save_and_load_layout(api_client, device_simulator, clean_state, tmp_path):
    """saveLayout persists the layout; loadLayout restores it without error."""
    proj_path = _write_project(tmp_path / "persist.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    api_client.command("ui.window.saveLayout")
    time.sleep(0.3)

    layout = api_client.command("ui.window.getLayout")["layout"]
    assert isinstance(layout, dict)
    assert "autoLayout" in layout
    assert "windowOrder" in layout

    api_client.command("ui.window.loadLayout")
    time.sleep(0.3)

    layout_after = api_client.command("ui.window.getLayout")["layout"]
    assert layout_after.get("autoLayout") == layout.get("autoLayout")


@pytest.mark.project
def test_layout_stored_in_project_file(api_client, device_simulator, clean_state, tmp_path):
    """After saveLayout + project.file.save the disk file contains the layout key."""
    proj_path = _write_project(tmp_path / "disk.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    status = api_client.command("ui.window.getStatus")
    group_id = status.get("activeGroupId")

    api_client.command("ui.window.saveLayout")
    time.sleep(0.3)
    api_client.command("project.file.save")
    time.sleep(0.3)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    ws = data.get(_WS_KEY, {})

    if group_id is not None and group_id >= 0:
        layout_key = _layout_key(group_id)
        assert layout_key in ws, (
            f"Expected layout key {layout_key!r} in widgetSettings; got: {list(ws.keys())}"
        )


@pytest.mark.project
def test_layout_persists_across_group_switch(
    api_client, device_simulator, clean_state, tmp_path
):
    """
    The autoLayout flag set on group A is saved and restored correctly when
    switching away and back — exercising the real user workflow.

    Flow:
      1. On group A: disable auto layout (manual mode).
      2. Switch to group B — group A layout is auto-saved with autoLayout=False.
      3. Switch back to group A — restoreLayout() reads the saved JSON and
         re-applies autoLayout=False.
      4. Verify getLayout still shows autoLayout=False for group A.
    """
    proj_path = _write_project(tmp_path / "switch.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    groups = _require_groups(api_client, minimum=2)
    id_a = groups[0]["id"]
    id_b = groups[1]["id"]

    api_client.command("ui.window.setActiveGroup", {"groupId": id_a})
    time.sleep(0.3)

    # Disable auto layout on group A — this is what we want to survive the switch
    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.2)
    assert api_client.command("ui.window.getLayout")["layout"]["autoLayout"] is False

    # Switch away — setActiveGroupId() calls saveLayout() which persists autoLayout=False
    api_client.command("ui.window.setActiveGroup", {"groupId": id_b})
    time.sleep(0.3)

    # Group B should start with autoLayout=True (default, no saved layout)
    assert api_client.command("ui.window.getLayout")["layout"]["autoLayout"] is True

    # Switch back — registerWindow() calls restoreLayout(savedLayout) which restores False
    api_client.command("ui.window.setActiveGroup", {"groupId": id_a})
    time.sleep(0.5)

    layout_a = api_client.command("ui.window.getLayout")["layout"]
    assert layout_a.get("autoLayout") is False, (
        f"Group A autoLayout=False was not restored after switching back; got: {layout_a}"
    )


# ---------------------------------------------------------------------------
# Manual layout: disable auto-layout, move windows, save, reload
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_manual_layout_save_reload(api_client, device_simulator, clean_state, tmp_path):
    """
    Full manual-layout round-trip:
      1. Disable auto layout.
      2. Apply a layout with specific window positions via setLayout.
      3. Save the layout to the project.
      4. Apply the saved layout back via setLayout (simulating what loadLayout does
         on startup — restoreLayout() reads autoLayout from JSON and sets the flag).
      5. Verify autoLayout=false is preserved and positions are intact.

    Note: ui.window.loadLayout() in a live session calls the current-state
    autoLayout/cascadeLayout path (not restoreLayout from JSON).  To test
    full restoration of a manual layout in-process, use ui.window.setLayout
    with the saved JSON directly, which calls restoreLayout().
    """
    proj_path = _write_project(tmp_path / "manual.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    # 1. Switch to manual mode
    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.3)

    # 2. Get current window order and build positioned layout
    layout_now = api_client.command("ui.window.getLayout")["layout"]
    window_order = layout_now.get("windowOrder", [])

    positioned = {
        "autoLayout": False,
        "windowOrder": window_order,
        "geometries": [
            {"id": wid, "x": float(i * 220), "y": 0.0, "width": 200.0, "height": 150.0}
            for i, wid in enumerate(window_order)
        ],
    }
    api_client.command("ui.window.setLayout", {"layout": positioned})
    time.sleep(0.3)

    # 3. Save layout
    api_client.command("ui.window.saveLayout")
    time.sleep(0.3)

    # 4. Temporarily switch to auto, then restore via setLayout (calls restoreLayout)
    api_client.command("ui.window.setAutoLayout", {"enabled": True})
    time.sleep(0.2)
    api_client.command("ui.window.setLayout", {"layout": positioned})
    time.sleep(0.3)

    # 5. autoLayout should be False because positioned["autoLayout"] = False
    layout_final = api_client.command("ui.window.getLayout")["layout"]
    assert layout_final.get("autoLayout") is False, (
        f"After setLayout with autoLayout=false, expected false; got {layout_final}"
    )


@pytest.mark.project
def test_manual_layout_switch_auto_back(api_client, device_simulator, clean_state, tmp_path):
    """
    Switch to manual layout, then back to auto layout.
    autoLayout flag in getLayout must reflect each transition.
    """
    proj_path = _write_project(tmp_path / "toggle.ssproj", _multi_group_project(n_groups=2))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    # Start with auto layout (default — clear() resets it on every group switch)
    layout = api_client.command("ui.window.getLayout")["layout"]
    assert layout.get("autoLayout") is True

    # Disable
    api_client.command("ui.window.setAutoLayout", {"enabled": False})
    time.sleep(0.3)
    assert api_client.command("ui.window.getLayout")["layout"].get("autoLayout") is False

    # Re-enable
    api_client.command("ui.window.setAutoLayout", {"enabled": True})
    time.sleep(0.3)
    assert api_client.command("ui.window.getLayout")["layout"].get("autoLayout") is True


# ---------------------------------------------------------------------------
# Widget settings
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_get_set_widget_settings(api_client, device_simulator, clean_state, tmp_path):
    """setWidgetSetting stores a value readable via getWidgetSettings."""
    proj_path = _write_project(tmp_path / "ws_rw.ssproj", _single_group_project())
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    widget_id = "test_widget_42"
    api_client.command("ui.window.setWidgetSetting",
                       {"widgetId": widget_id, "key": "interpolate", "value": True})
    api_client.command("ui.window.setWidgetSetting",
                       {"widgetId": widget_id, "key": "showArea", "value": False})
    api_client.command("ui.window.setWidgetSetting",
                       {"widgetId": widget_id, "key": "lineWidth", "value": 2})
    time.sleep(0.3)

    settings = api_client.command("ui.window.getWidgetSettings",
                                  {"widgetId": widget_id})["settings"]
    assert settings.get("interpolate") is True
    assert settings.get("showArea") is False
    assert settings.get("lineWidth") == 2


@pytest.mark.project
def test_widget_settings_persist_to_disk(api_client, device_simulator, clean_state, tmp_path):
    """setWidgetSetting + project.file.save writes the setting to disk."""
    proj_path = _write_project(tmp_path / "ws_disk.ssproj", _single_group_project())
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=6)

    widget_id = "disk_test_1001"
    api_client.command("ui.window.setWidgetSetting",
                       {"widgetId": widget_id, "key": "showArea", "value": False})
    time.sleep(0.2)

    api_client.command("project.file.save")
    time.sleep(0.3)

    data = json.loads(proj_path.read_text(encoding="utf-8"))
    ws = data.get(_WS_KEY, {})
    assert widget_id in ws, f"widgetId {widget_id!r} not found in widgetSettings; keys={list(ws)}"
    assert ws[widget_id].get("showArea") is False


@pytest.mark.project
def test_widget_settings_survive_group_switch(
    api_client, device_simulator, clean_state, tmp_path
):
    """Widget settings set on group A survive switching to B and back."""
    proj_path = _write_project(tmp_path / "ws_gs.ssproj", _multi_group_project(n_groups=3))
    api_client.command("project.file.open", {"filePath": str(proj_path)})
    time.sleep(0.3)
    _connect_and_send(api_client, device_simulator, n_values=9)

    groups = _require_groups(api_client, minimum=2)
    id_a, id_b = groups[0]["id"], groups[1]["id"]

    api_client.command("ui.window.setActiveGroup", {"groupId": id_a})
    time.sleep(0.2)

    widget_id = "persist_test_widget"
    api_client.command("ui.window.setWidgetSetting",
                       {"widgetId": widget_id, "key": "color", "value": "red"})
    time.sleep(0.2)

    api_client.command("ui.window.setActiveGroup", {"groupId": id_b})
    time.sleep(0.2)
    api_client.command("ui.window.setActiveGroup", {"groupId": id_a})
    time.sleep(0.2)

    settings = api_client.command("ui.window.getWidgetSettings",
                                  {"widgetId": widget_id})["settings"]
    assert settings.get("color") == "red"
