"""
Layout Pattern Integration Tests (spec 0053)

Covers the auto-layout pattern choice, which lives beside the window geometry of
whatever workspace or group tab is on screen:
 * ui.window.setLayoutPattern / getLayout round-trip for every shipped pattern
 * defaults (Grid at a 1/2 split) when nothing was ever chosen
 * unknown pattern ids and out-of-range ratios are tolerated, not errors
 * choosing a pattern does not flip the project into customized-workspace mode
 * the choice does not disturb the saved manual geometry it sits next to

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import pytest

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _get_layout(api_client):
    return api_client.command("ui.window.getLayout")


def _set_pattern(api_client, pattern, ratio=None):
    params = {"pattern": pattern}
    if ratio is not None:
        params["ratio"] = ratio

    return api_client.command("ui.window.setLayoutPattern", params)


def _customize_enabled(api_client):
    return api_client.command("project.workspace.getCustomizeMode")["enabled"]


# ---------------------------------------------------------------------------
# Defaults
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_default_is_grid_at_one_half(api_client, clean_state):
    """With nothing stored, the dashboard reports Grid at 8/16."""
    layout = _get_layout(api_client)

    assert layout["pattern"] == ""
    assert layout["ratio"] == 8


# ---------------------------------------------------------------------------
# Round-trip
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.parametrize(
    "pattern", ["master-stack", "master-grid", "row", "column", "spiral", ""]
)
def test_every_pattern_round_trips(api_client, clean_state, pattern):
    """Each shipped pattern id is accepted and read back unchanged."""
    _set_pattern(api_client, pattern)

    assert _get_layout(api_client)["pattern"] == pattern


@pytest.mark.project
def test_ratio_round_trips(api_client, clean_state):
    """The split ratio is stored in sixteenths and read back unchanged."""
    _set_pattern(api_client, "master-stack", ratio=12)

    layout = _get_layout(api_client)
    assert layout["pattern"] == "master-stack"
    assert layout["ratio"] == 12


@pytest.mark.project
def test_ratio_persists_across_a_pattern_change(api_client, clean_state):
    """Switching pattern without naming a ratio keeps the ratio already in force."""
    _set_pattern(api_client, "master-grid", ratio=4)
    _set_pattern(api_client, "spiral")

    layout = _get_layout(api_client)
    assert layout["pattern"] == "spiral"
    assert layout["ratio"] == 4


# ---------------------------------------------------------------------------
# Tolerance
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_unknown_pattern_is_kept_not_rejected(api_client, clean_state):
    """
    An id this build does not know is stored verbatim rather than erroring: the
    tiler degrades it to Grid, and a project written by a newer build survives a
    round-trip through this one.
    """
    _set_pattern(api_client, "mosaic-from-the-future")

    assert _get_layout(api_client)["pattern"] == "mosaic-from-the-future"


@pytest.mark.project
@pytest.mark.parametrize("sent,stored", [(-4, 1), (0, 1), (16, 15), (99, 15)])
def test_out_of_range_ratio_clamps(api_client, clean_state, sent, stored):
    """Ratios outside 1..15 clamp into range instead of failing the call."""
    _set_pattern(api_client, "master-grid", ratio=sent)

    assert _get_layout(api_client)["ratio"] == stored


@pytest.mark.project
def test_missing_pattern_param_is_an_error(api_client, clean_state):
    """The pattern parameter is required; the ratio is not."""
    from utils import APIError

    with pytest.raises(APIError):
        api_client.command("ui.window.setLayoutPattern", {"ratio": 8})


# ---------------------------------------------------------------------------
# Isolation from the workspace list
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_choosing_a_pattern_does_not_customize_workspaces(api_client, clean_state):
    """
    The choice lives beside the window geometry, not on the workspace entry, so
    picking a layout must not materialize the auto workspace list the way renaming
    a workspace does.
    """
    assert _customize_enabled(api_client) is False

    _set_pattern(api_client, "row")

    assert _customize_enabled(api_client) is False


@pytest.mark.project
def test_pattern_does_not_disturb_saved_geometry(api_client, clean_state):
    """
    Pattern and ratio are sibling keys of the stored manual geometry; writing them
    must leave that geometry byte-identical.
    """
    before = _get_layout(api_client)["layout"]

    _set_pattern(api_client, "column", ratio=10)

    assert _get_layout(api_client)["layout"] == before
