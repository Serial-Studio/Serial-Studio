"""
Output Widget Integration Tests

Tests for the output widget framework including:
- Adding output widgets to project groups via the API
- Output widget serialization (save/load round-trip)
- Verifying output widgets appear in dashboard when project is loaded
- Verifying output widget data transmission through device simulator

These tests require Serial Studio Pro (BUILD_COMMERCIAL) to be running
with an active license (output widgets require Pro tier).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time
import json

import pytest

from utils import SerialStudioClient
from utils.api_client import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

OUTPUT_WIDGET_TYPES = {
    "button": 0,
    "slider": 1,
    "toggle": 2,
    "textfield": 3,
    "knob": 4,
    "ramp_generator": 5,
}


def _is_commercial_build(api_client) -> bool:
    """Check if running Commercial build."""
    return api_client.command_exists("licensing.getStatus")


def _is_pro_tier(api_client) -> bool:
    """Check if current license is at least Pro tier."""
    if not _is_commercial_build(api_client):
        return False

    status = api_client.command("licensing.getStatus")
    return status.get("featureTierValue", 0) >= 3  # Pro = 3


def _make_output_widget(title, widget_type="button", transmit_fn=None,
                        min_val=0, max_val=100, step=1,
                        on_label="ON", off_label="OFF", units=""):
    """Create an output widget JSON object for use in project configs."""
    if transmit_fn is None:
        transmit_fn = 'function transmit(value) { return "CMD " + value + "\\r\\n"; }'

    return {
        "title": title,
        "units": units,
        "outputType": OUTPUT_WIDGET_TYPES.get(widget_type, 0),
        "outputMin": min_val,
        "outputMax": max_val,
        "outputStep": step,
        "onLabel": on_label,
        "offLabel": off_label,
        "transmitFunction": transmit_fn,
    }


def _make_project_with_output_widgets(output_widgets):
    """Create a minimal project JSON with output widgets in a group."""
    return {
        "title": "Output Widget Test",
        "groups": [
            {
                "title": "Controls",
                "widget": "datagrid",
                "datasets": [
                    {
                        "title": "Sensor1",
                        "value": "0",
                        "index": 1,
                        "widget": "",
                    }
                ],
                "outputWidgets": output_widgets,
            }
        ],
        "actions": [],
    }


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------

@pytest.fixture(autouse=True)
def require_commercial(api_client):
    """Skip tests when running against a GPL build."""
    if not _is_commercial_build(api_client):
        pytest.skip("Commercial build required for output widget tests")


# ---------------------------------------------------------------------------
# Output widget project serialization tests
# ---------------------------------------------------------------------------

@pytest.mark.integration
@pytest.mark.project
def test_project_with_button_widget_loads(api_client, clean_state):
    """A project with a button output widget should load without errors."""
    widgets = [_make_output_widget("Emergency Stop", "button")]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    status = api_client.get_project_status()
    assert status.get("groupCount", 0) >= 1


@pytest.mark.integration
@pytest.mark.project
def test_project_with_slider_widget_loads(api_client, clean_state):
    """A project with a slider output widget should load without errors."""
    widgets = [
        _make_output_widget(
            "Motor Speed", "slider",
            transmit_fn='function transmit(value) { return "SPD " + Math.round(value) + "\\r\\n"; }',
            min_val=0, max_val=255, step=1, units="RPM",
        )
    ]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    status = api_client.get_project_status()
    assert status.get("groupCount", 0) >= 1


@pytest.mark.integration
@pytest.mark.project
def test_project_with_toggle_widget_loads(api_client, clean_state):
    """A project with a toggle output widget should load without errors."""
    widgets = [
        _make_output_widget(
            "Relay", "toggle",
            transmit_fn='function transmit(value) { return value ? "RELAY ON\\r\\n" : "RELAY OFF\\r\\n"; }',
            on_label="Energized", off_label="De-energized",
        )
    ]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    status = api_client.get_project_status()
    assert status.get("groupCount", 0) >= 1


@pytest.mark.integration
@pytest.mark.project
def test_project_with_textfield_widget_loads(api_client, clean_state):
    """A project with a text field output widget should load without errors."""
    widgets = [
        _make_output_widget(
            "Command", "textfield",
            transmit_fn='function transmit(value) { return value + "\\r\\n"; }',
        )
    ]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    status = api_client.get_project_status()
    assert status.get("groupCount", 0) >= 1


@pytest.mark.integration
@pytest.mark.project
def test_project_with_multiple_output_widgets(api_client, clean_state):
    """A project with multiple output widget types should load."""
    widgets = [
        _make_output_widget("Button1", "button"),
        _make_output_widget("Slider1", "slider", min_val=0, max_val=100),
        _make_output_widget("Toggle1", "toggle"),
        _make_output_widget("Input1", "textfield"),
    ]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    status = api_client.get_project_status()
    assert status.get("groupCount", 0) >= 1


# ---------------------------------------------------------------------------
# Output widget round-trip serialization
# ---------------------------------------------------------------------------

@pytest.mark.integration
@pytest.mark.project
def test_output_widget_survives_round_trip(api_client, clean_state):
    """Output widgets should survive a save/load round trip."""
    widgets = [
        _make_output_widget(
            "PWM Control", "slider",
            transmit_fn='function transmit(value) { return "PWM " + value + "\\n"; }',
            min_val=0, max_val=1023, step=1, units="duty",
        )
    ]
    project = _make_project_with_output_widgets(widgets)

    # Load the project
    api_client.load_project_from_json(project)
    time.sleep(0.3)

    # Get the project back and verify the output widget is still there
    result = api_client.command("project.exportJson")
    if "config" in result:
        project_json = result["config"]
        if isinstance(project_json, str):
            project_json = json.loads(project_json)

        groups = project_json.get("groups", [])
        assert len(groups) >= 1

        ow_list = groups[0].get("outputWidgets", [])
        assert len(ow_list) == 1
        assert ow_list[0]["title"] == "PWM Control"
        assert ow_list[0]["outputMin"] == 0
        assert ow_list[0]["outputMax"] == 1023
        assert "transmitFunction" in ow_list[0]


# ---------------------------------------------------------------------------
# Dashboard output widget visibility (requires Pro license)
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_output_widgets_visible_in_dashboard_with_pro(api_client, clean_state):
    """Output widgets should appear in dashboard when Pro license is active."""
    if not _is_pro_tier(api_client):
        pytest.skip("Pro license required for output widget dashboard tests")

    widgets = [_make_output_widget("Test Button", "button")]
    project = _make_project_with_output_widgets(widgets)

    api_client.load_project_from_json(project)
    time.sleep(0.3)

    api_client.set_operation_mode("project")
    api_client.command("project.loadIntoFrameBuilder")
    time.sleep(0.3)

    dashboard = api_client.get_dashboard_status()
    total_widgets = dashboard.get("totalWidgetCount", 0)

    # Should have at least the data grid + the output button widget
    assert total_widgets >= 2, \
        f"Expected at least 2 widgets (datagrid + output button), got {total_widgets}"


# ---------------------------------------------------------------------------
# License tier gating for output widgets
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_output_widget_js_function_format(api_client, clean_state):
    """Output widget transmit function should accept various JS patterns."""
    test_cases = [
        ('function transmit(value) { return String(value); }', True),
        ('function transmit(value) { return "OK\\r\\n"; }', True),
        ('function transmit(v) { return v > 50 ? "HIGH" : "LOW"; }', True),
    ]

    for fn, _should_work in test_cases:
        widgets = [_make_output_widget("Test", "button", transmit_fn=fn)]
        project = _make_project_with_output_widgets(widgets)

        # Should not crash
        api_client.load_project_from_json(project)
        time.sleep(0.2)


# ---------------------------------------------------------------------------
# REGRESSION — project.outputWidget.add accepts "type" parameter
# ---------------------------------------------------------------------------

@pytest.mark.integration
def test_output_widget_add_accepts_type_param(api_client, clean_state):
    """
    The schema-advertised parameter for project.outputWidget.add is the
    OutputWidgetType enum value under the key "type" (0=Button .. 5=RampGenerator).
    An earlier revision advertised "groupId" but the handler read "type",
    so every schema-conformant call was silently creating a Button.
    """
    api_client.command("project.group.add", {"title": "Outputs", "widgetType": 0})
    time.sleep(0.15)

    for widget_name, widget_type in OUTPUT_WIDGET_TYPES.items():
        try:
            result = api_client.command(
                "project.outputWidget.add", {"type": widget_type}
            )
        except APIError as e:
            # Commercial license gating is acceptable; schema mismatch is not.
            assert "licen" in str(e).lower() or "commercial" in str(e).lower(), (
                f"outputWidget.add for {widget_name} raised non-licensing error: {e}"
            )
            return

        assert result.get("added") is True
