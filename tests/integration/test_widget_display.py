"""
Widget display-title override and freeze-title mode tests (spec 0013).

Exercises project.dashboard.setWidgetTitle / getWidgetTitles /
setWidgetFreezeTitle against a running Serial Studio (API server on
localhost:7777): override round-trip through save/reload, reorder survival
(uniqueId keying), deleted-target inertness, and the canonical-title
isolation contract (exports and project.* responses never see overrides).

Covers spec 0013 acceptance criteria AC1, AC3 and AC7.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError

RPM_UID = 10
TEMP_UID = 11
HUMIDITY_UID = 12
ENGINE_GROUP_UID = 600
CABIN_GROUP_UID = 601


def _fixture_project():
    """Two groups with explicit, stable uniqueIds so overrides can be
    asserted against known keys after reorders and reloads."""
    return {
        "title": "Widget display fixture",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "groups": [
            {
                "title": "Engine",
                "uniqueId": ENGINE_GROUP_UID,
                "widget": "datagrid",
                "datasets": [
                    {
                        "title": "RPM",
                        "uniqueId": RPM_UID,
                        "index": 1,
                        "widget": "gauge",
                    },
                    {
                        "title": "Temp",
                        "uniqueId": TEMP_UID,
                        "index": 2,
                        "widget": "bar",
                    },
                ],
            },
            {
                "title": "Cabin",
                "uniqueId": CABIN_GROUP_UID,
                "widget": "",
                "datasets": [
                    {
                        "title": "Humidity",
                        "uniqueId": HUMIDITY_UID,
                        "index": 3,
                        "widget": "gauge",
                    },
                ],
            },
        ],
    }


def _load_fixture(api_client):
    api_client.load_project_from_json(_fixture_project())
    time.sleep(0.5)


GAUGE_TYPE = 11
FFT_TYPE = 7


def _override_map(api_client) -> dict:
    """Entity-level rows keyed by uniqueId (widget-level rows excluded)."""
    result = api_client.command("project.dashboard.getWidgetTitles", {})
    return {
        row["uniqueId"]: row for row in result["titles"] if row["scope"] == "entity"
    }


def _widget_override_map(api_client) -> dict:
    """Widget-level rows keyed by (widgetType, uniqueId)."""
    result = api_client.command("project.dashboard.getWidgetTitles", {})
    return {
        (row["widgetType"], row["uniqueId"]): row
        for row in result["titles"]
        if row["scope"] == "widget"
    }


@pytest.mark.integration
@pytest.mark.project
class TestWidgetTitleOverrides:
    def test_set_get_and_clear(self, api_client, clean_state):
        """AC7: set echoes previous/canonical, get lists it, empty clears."""
        _load_fixture(api_client)

        result = api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "title": "Engine Speed"},
        )
        assert result["canonical"] == "RPM"
        assert result["previous"] == ""
        assert result["cleared"] is False

        rows = _override_map(api_client)
        assert rows[RPM_UID]["title"] == "Engine Speed"
        assert rows[RPM_UID]["canonical"] == "RPM"

        result = api_client.command(
            "project.dashboard.setWidgetTitle", {"uniqueId": RPM_UID, "title": ""}
        )
        assert result["cleared"] is True
        assert RPM_UID not in _override_map(api_client)

    def test_widget_level_scope_wins_and_clears_independently(
        self, api_client, clean_state
    ):
        """Amended R1: a dataset shown in several widgets can title each one;
        the widget-level entry wins over the entity-level entry."""
        _load_fixture(api_client)

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "title": "Engine Speed"},
        )
        result = api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "widgetType": FFT_TYPE, "title": "RPM Spectrum"},
        )
        assert result["scope"] == "widget"
        assert result["widgetType"] == FFT_TYPE

        entity = _override_map(api_client)
        widget = _widget_override_map(api_client)
        assert entity[RPM_UID]["title"] == "Engine Speed"
        assert widget[(FFT_TYPE, RPM_UID)]["title"] == "RPM Spectrum"
        assert widget[(FFT_TYPE, RPM_UID)]["canonical"] == "RPM"

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "widgetType": FFT_TYPE, "title": ""},
        )
        assert (FFT_TYPE, RPM_UID) not in _widget_override_map(api_client)
        assert _override_map(api_client)[RPM_UID]["title"] == "Engine Speed"

    def test_unknown_uid_and_invalid_mode_rejected(self, api_client, clean_state):
        """AC7: unresolvable uniqueId and bad mode strings are errors."""
        _load_fixture(api_client)

        with pytest.raises(APIError):
            api_client.command(
                "project.dashboard.setWidgetTitle",
                {"uniqueId": 99999, "title": "Ghost"},
            )

        with pytest.raises(APIError):
            api_client.command(
                "project.dashboard.setWidgetFreezeTitle",
                {"widgetType": GAUGE_TYPE, "uniqueId": RPM_UID, "mode": "sideways"},
            )

        with pytest.raises(APIError):
            api_client.command(
                "project.dashboard.setWidgetFreezeTitle",
                {"widgetType": FFT_TYPE, "uniqueId": RPM_UID, "mode": "painted"},
            )

    def test_freeze_mode_set_and_default_clears(self, api_client, clean_state):
        """AC7: non-default modes persist per (widgetType, uniqueId); writing the
        per-type default (painted for gauges) removes the stored entry."""
        _load_fixture(api_client)

        result = api_client.command(
            "project.dashboard.setWidgetFreezeTitle",
            {"widgetType": GAUGE_TYPE, "uniqueId": RPM_UID, "mode": "hidden"},
        )
        assert result["mode"] == "hidden"
        assert result["previous"] == "painted"

        exported = api_client.command("project.exportJson", {})["config"]
        freeze = exported.get("widgetDisplay", {}).get("freezeTitle", {})
        assert freeze.get(f"{GAUGE_TYPE}:{RPM_UID}") == "hidden"

        api_client.command(
            "project.dashboard.setWidgetFreezeTitle",
            {"widgetType": GAUGE_TYPE, "uniqueId": RPM_UID, "mode": "painted"},
        )
        exported = api_client.command("project.exportJson", {})["config"]
        freeze = exported.get("widgetDisplay", {}).get("freezeTitle", {})
        assert f"{GAUGE_TYPE}:{RPM_UID}" not in freeze

    def test_round_trip_through_save_and_reload(
        self, api_client, clean_state, tmp_path
    ):
        """AC1: overrides and freeze modes survive save -> new -> open."""
        _load_fixture(api_client)

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "title": "Engine Speed"},
        )
        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": ENGINE_GROUP_UID, "title": "Powertrain"},
        )
        api_client.command(
            "project.dashboard.setWidgetFreezeTitle",
            {"widgetType": GAUGE_TYPE, "uniqueId": HUMIDITY_UID, "mode": "bar"},
        )

        path = str(tmp_path / "widget_display.ssproj")
        api_client.command("project.save", {"filePath": path})
        time.sleep(0.5)

        api_client.command("project.new", {})
        time.sleep(0.5)
        assert not _override_map(api_client)

        api_client.command("project.open", {"filePath": path})
        time.sleep(0.5)

        rows = _override_map(api_client)
        assert rows[RPM_UID]["title"] == "Engine Speed"
        assert rows[RPM_UID]["canonical"] == "RPM"
        assert rows[ENGINE_GROUP_UID]["title"] == "Powertrain"
        assert rows[ENGINE_GROUP_UID]["canonical"] == "Engine"

        exported = api_client.command("project.exportJson", {})["config"]
        freeze = exported.get("widgetDisplay", {}).get("freezeTitle", {})
        assert freeze.get(f"{GAUGE_TYPE}:{HUMIDITY_UID}") == "bar"

    def test_override_survives_group_reorder(self, api_client, clean_state):
        """AC1/R9: uniqueId keying survives positional renumbering."""
        _load_fixture(api_client)

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": HUMIDITY_UID, "title": "Cabin RH"},
        )

        api_client.command("project.group.move", {"groupId": 1, "newPosition": 0})
        time.sleep(0.5)

        rows = _override_map(api_client)
        assert rows[HUMIDITY_UID]["title"] == "Cabin RH"
        assert rows[HUMIDITY_UID]["canonical"] == "Humidity"

    def test_orphaned_override_is_inert(self, api_client, clean_state, tmp_path):
        """AC1: deleting the target leaves the entry harmless and the
        project loadable."""
        _load_fixture(api_client)

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": TEMP_UID, "title": "Coolant"},
        )
        api_client.command("project.dataset.delete", {"groupId": 0, "datasetId": 1})
        time.sleep(0.5)

        rows = _override_map(api_client)
        assert rows[TEMP_UID]["canonical"] is None

        path = str(tmp_path / "orphaned.ssproj")
        api_client.command("project.save", {"filePath": path})
        api_client.command("project.new", {})
        api_client.command("project.open", {"filePath": path})
        time.sleep(0.5)
        assert api_client.get_project_status()["title"] == "Widget display fixture"

    def test_canonical_titles_unaffected(
        self, api_client, clean_state, device_simulator
    ):
        """AC3: exports and project responses never see the override."""
        _load_fixture(api_client)

        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": RPM_UID, "title": "Engine Speed"},
        )
        api_client.command(
            "project.dashboard.setWidgetTitle",
            {"uniqueId": ENGINE_GROUP_UID, "title": "Powertrain"},
        )

        exported = api_client.command("project.exportJson", {})["config"]
        engine = exported["groups"][0]
        assert engine["title"] == "Engine"
        assert engine["datasets"][0]["title"] == "RPM"

        # The dashboard drops frames while no stream is open, so feed one
        # real frame through the TCP loopback before reading it back.
        api_client.configure_network(host="127.0.0.1", port=9000, socket_type="tcp")
        api_client.connect_device()
        assert device_simulator.wait_for_connection(timeout=5.0)
        device_simulator.send_frames([b"10,20,30\n"], interval_seconds=0.1)
        time.sleep(1.0)

        frame = api_client.command("dashboard.getData", {})["frame"]
        api_client.disconnect_device()
        group_titles = {g["title"] for g in frame.get("groups", [])}
        assert "Engine" in group_titles
        assert "Powertrain" not in group_titles
        dataset_titles = {
            d["title"] for g in frame.get("groups", []) for d in g.get("datasets", [])
        }
        assert "RPM" in dataset_titles
        assert "Engine Speed" not in dataset_titles
