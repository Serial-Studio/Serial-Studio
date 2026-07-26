"""
Dashboard Freeze Flag Integration Tests

Tests for the project-persisted dashboard freeze flag (spec 0007):
  - project.exportJson exposes "frozen" (absent-means-false default)
  - a config loaded with "frozen": true survives a load/export round-trip on
    any license tier (the loader and serializer must never strip the flag,
    only the user-facing toggle is license-gated)
  - creating a new project resets the flag to false

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest


def _build_simple_project(api_client, title):
    """Helper: create a minimal project with one group and one dataset."""
    api_client.create_new_project(title=title)
    time.sleep(0.3)

    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)


@pytest.mark.project
def test_project_export_includes_frozen_flag(api_client, clean_state):
    """Verify a fresh project exports "frozen": false."""
    _build_simple_project(api_client, "Freeze Default")

    result = api_client.command("project.exportJson")
    config = result["config"]

    assert "frozen" in config, "Exported project must expose the 'frozen' flag"
    assert config["frozen"] is False, "A fresh project must export frozen=false"


@pytest.mark.project
def test_frozen_flag_survives_load_export_roundtrip(api_client, clean_state):
    """Load a frozen config and re-export it; the flag must be preserved
    verbatim regardless of the running instance's license tier."""
    _build_simple_project(api_client, "Freeze Roundtrip")

    export = api_client.command("project.exportJson")
    config = export["config"]
    config["frozen"] = True

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    result = api_client.command("project.exportJson")
    assert result["config"].get("frozen") is True, (
        "A project loaded with frozen=true must re-export frozen=true; "
        "stripping the flag on load/save is data loss"
    )


@pytest.mark.project
def test_frozen_flag_resets_on_new_project(api_client, clean_state):
    """Verify that creating a new project clears a previously loaded flag."""
    _build_simple_project(api_client, "Freeze Then New")

    export = api_client.command("project.exportJson")
    config = export["config"]
    config["frozen"] = True

    api_client.command("project.loadJson", {"config": config})
    time.sleep(0.3)

    api_client.create_new_project(title="Fresh After Frozen")
    time.sleep(0.3)

    result = api_client.command("project.exportJson")
    assert (
        result["config"].get("frozen") is False
    ), "A new project must start unfrozen even after a frozen one was loaded"
