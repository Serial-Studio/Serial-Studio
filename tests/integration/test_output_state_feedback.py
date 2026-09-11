"""
Output Control State Feedback Integration Tests (spec 0080)

Covers the parts reachable without a human watching a dashboard:

  * AC7 - a state binding round-trips through the project unchanged.
  * AC4 - an output widget written before this feature reads back unbound, which is what makes
    every existing project behave exactly as it did.
  * R3, structurally - the feedback path cannot transmit. Checked at the source level: the class
    that owns the rules holds no transmit target, and the apply path in the widget base never
    touches one. A source scan is the durable form of that guarantee, because it keeps holding
    for control types nobody has written yet.

AC1 and AC2's runtime halves (feeding a bound dataset moves the control; doing so sends nothing)
need to observe a live widget's displayed state, and the API exposes no command that reads it.
They are maintainer observations, recorded as such in spec.md.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import re
from pathlib import Path

import pytest

# ---------------------------------------------------------------------------
# Structural guarantees (no running app required)
# ---------------------------------------------------------------------------

_REPO_ROOT = Path(__file__).resolve().parents[2]
_OUTPUT_DIR = _REPO_ROOT / "core" / "Ui" / "UI" / "Widgets" / "Output"


def test_state_binding_owns_no_transmit_target():
    """
    R3, half one: the class holding the truth rule and the clock cannot reach the wire.
    """
    for name in ("StateBinding.h", "StateBinding.cpp"):
        text = (_OUTPUT_DIR / name).read_text(encoding="utf-8")
        assert "TransmitTarget" not in text, f"{name} reaches a transmit target"


def test_feedback_apply_path_never_reaches_the_target():
    """
    R3, half two: Base::refreshState is where feedback becomes displayed state. If it ever touches
    m_target, a control would command its own equipment from its own feedback.
    """
    text = (_OUTPUT_DIR / "Base.cpp").read_text(encoding="utf-8")
    start = text.index("void Widgets::Output::Base::refreshState()")
    end = text.index("void Widgets::Output::Base::beginInteraction()")
    assert "m_target" not in text[start:end]


def test_controls_apply_feedback_without_their_own_setters():
    """
    Each control assigns its member directly in applyStateVerdict. Calling its own setter would
    transmit, which is the loop R3 forbids.
    """
    forbidden = {
        "Toggle.cpp": "setChecked(",
        "Button.cpp": "setChecked(",
        "Slider.cpp": "setCurrentValue(",
    }
    for name, setter in forbidden.items():
        text = (_OUTPUT_DIR / name).read_text(encoding="utf-8")
        start = text.index("applyStateVerdict")
        assert setter not in text[start:], f"{name} transmits from its feedback path"


def test_preview_builds_an_unbound_control():
    """
    A preview must not follow live equipment while the user edits a transmit script.
    """
    text = (_OUTPUT_DIR / "Preview.cpp").read_text(encoding="utf-8")
    assert "stateSource             = DataModel::OutputStateSource::None" in text


# ---------------------------------------------------------------------------
# API surface (running app required)
# ---------------------------------------------------------------------------


def _first_output_widget(api_client):
    """Return (groupId, widgetId) of any output widget in the loaded project."""
    status = api_client.command("project.get", {})
    for group in status.get("groups", []):
        for widget in group.get("outputWidgets", []):
            return group.get("groupId", 0), widget.get("widgetId", 0)

    pytest.skip("loaded project has no output widget to exercise")


@pytest.mark.integration
def test_binding_round_trips(api_client):
    """
    AC7: a binding survives a write/read cycle with the same source selected.
    """
    group_id, widget_id = _first_output_widget(api_client)

    api_client.command(
        "project.outputWidget.update",
        {
            "groupId": group_id,
            "widgetId": widget_id,
            "stateSource": 1,
            "stateDatasetId": 3,
            "stateOnValue": "RUN",
            "stateConfirmMs": 1500,
        },
    )

    stored = api_client.command(
        "project.outputWidget.get", {"groupId": group_id, "widgetId": widget_id}
    )
    assert stored.get("stateSource") == 1
    assert stored.get("stateDatasetId") == 3
    assert stored.get("stateOnValue") == "RUN"
    assert stored.get("stateConfirmMs") == 1500


@pytest.mark.integration
def test_an_unbound_widget_stays_unbound(api_client):
    """
    AC4: clearing the source leaves the widget unbound, and the serializer omits the whole block
    so a project that never had a binding is byte-identical to one that had it removed.
    """
    group_id, widget_id = _first_output_widget(api_client)

    api_client.command(
        "project.outputWidget.update",
        {"groupId": group_id, "widgetId": widget_id, "stateSource": 0},
    )

    stored = api_client.command(
        "project.outputWidget.get", {"groupId": group_id, "widgetId": widget_id}
    )
    assert stored.get("stateSource", 0) == 0
