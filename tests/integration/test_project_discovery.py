"""
Project discovery API integration tests (spec 0012).

Exercises project.search, project.group.get, and offset/limit paging on the
whole-project list commands against a running Serial Studio (API server on
localhost:7777), using a synthesized 87-group / 570-dataset project shaped
like the field report that motivated the feature.

Covers spec 0012 acceptance criteria AC1-AC5. meta.search (AC8) lives on the
assistant surface, not the TCP API, so its runnable coverage is in
tests/scripts/test_ai_assistant_static.py.

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import time

import pytest

from utils.api_client import APIError

GROUP_COUNT = 87
DATASET_COUNT = 570

PRESSURE_GROUPS = {
    40: "Vacuum, Differential & Barometric",
    41: "Absolute Pressure (psia)",
    42: "Gauge Pressure (psig)",
}


def _large_project():
    """87 groups / 570 datasets; groups 40-42 mirror the field report's
    pressure instrumentation (psia units, a PT-ALPHA alias, one dataset whose
    title contains 'Pressure')."""
    groups = []
    slot = 1
    for g in range(GROUP_COUNT):
        count = 7 if g < 48 else 6
        title = PRESSURE_GROUPS.get(g, f"Instrument Bank {g:02d}")
        datasets = []
        for d in range(count):
            dataset = {"title": f"CH-{g:02d}-{d}", "index": slot}
            if g == 41:
                dataset["title"] = f"PT-{d + 1}"
                dataset["units"] = "psia"
            if g == 41 and d == 0:
                dataset["alias"] = "PT-ALPHA"
            if g == 40 and d == 0:
                dataset["title"] = "Chamber Pressure Raw"
            datasets.append(dataset)
            slot += 1
        # Sparse group uniqueIds (600+) distinct from the dense positional groupId (== index),
        # mirroring the field report where positional ids were single/double digit but the
        # workspace refs carried uniqueIds in the 600s.
        groups.append(
            {"title": title, "uniqueId": 600 + g, "widget": "", "datasets": datasets}
        )

    return {
        "title": "Discovery fixture",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "groups": groups,
        "actions": [
            {"title": "Purge Pressure Line", "txData": "PURGE"},
            {"title": "Zero All Channels", "txData": "ZERO"},
        ],
    }


def _load_fixture(api_client):
    api_client.load_project_from_json(_large_project())
    time.sleep(0.5)


def _compact_size(payload) -> int:
    return len(json.dumps(payload, separators=(",", ":")).encode("utf-8"))


@pytest.mark.integration
@pytest.mark.project
class TestProjectSearch:
    def test_search_basic(self, api_client, clean_state):
        """AC1: substring hits across groups return typed compact rows whose
        path round-trips through the exact resolver."""
        _load_fixture(api_client)

        result = api_client.command(
            "project.search", {"query": "pressure", "limit": 100}
        )
        assert result["matchCount"] >= 4
        assert result["projectEpoch"] >= 0

        rows = result["rows"]
        types = {row["type"] for row in rows}
        assert "group" in types
        assert "dataset" in types
        assert "action" in types

        dataset_rows = [r for r in rows if r["type"] == "dataset"]
        assert dataset_rows
        for row in dataset_rows:
            for field in (
                "uniqueId",
                "title",
                "path",
                "groupId",
                "groupTitle",
                "index",
            ):
                assert field in row

        probe = dataset_rows[0]
        resolved = api_client.command(
            "project.dataset.getByPath", {"path": probe["path"]}
        )
        assert resolved["uniqueId"] == probe["uniqueId"]

    def test_search_matched_field_provenance(self, api_client, clean_state):
        """AC1/R2: alias and units hits carry matchedField; title hits do not."""
        _load_fixture(api_client)

        by_alias = api_client.command(
            "project.search", {"query": "ALPHA", "type": "dataset"}
        )
        assert by_alias["matchCount"] == 1
        assert by_alias["rows"][0]["matchedField"] == "alias"

        by_units = api_client.command(
            "project.search", {"query": "psia", "type": "dataset", "limit": 100}
        )
        assert by_units["matchCount"] >= 7
        assert all(r.get("matchedField") == "units" for r in by_units["rows"])

        by_title = api_client.command("project.search", {"query": "Chamber Pressure"})
        assert by_title["matchCount"] == 1
        assert "matchedField" not in by_title["rows"][0]

    def test_search_filters_and_types(self, api_client, clean_state):
        """AC2: type filter restricts row types; groupId filter restricts
        datasets; source/workspace/table titles surface typed rows."""
        _load_fixture(api_client)

        groups_only = api_client.command(
            "project.search", {"query": "pressure", "type": "group", "limit": 100}
        )
        # Two of the three pressure groups carry "pressure" in the title; the third is
        # "Vacuum, Differential & Barometric" and correctly does not match.
        assert groups_only["matchCount"] == 2
        assert all(r["type"] == "group" for r in groups_only["rows"])

        target = next(
            r for r in groups_only["rows"] if r["title"] == "Absolute Pressure (psia)"
        )
        in_group = api_client.command(
            "project.search",
            {
                "query": "PT-",
                "type": "dataset",
                "groupId": target["groupId"],
                "limit": 100,
            },
        )
        assert in_group["matchCount"] == 7
        assert all(r["groupId"] == target["groupId"] for r in in_group["rows"])

        api_client.command(
            "project.source.update", {"sourceId": 0, "title": "Telemetry Link"}
        )
        by_source = api_client.command("project.search", {"query": "Telemetry Link"})
        assert any(r["type"] == "source" for r in by_source["rows"])

        api_client.command("project.dataTable.add", {"name": "Calibration Table"})
        by_table = api_client.command("project.search", {"query": "Calibration"})
        assert any(r["type"] == "table" for r in by_table["rows"])

        api_client.command("project.workspace.setCustomizeMode", {"enabled": True})
        api_client.command("project.workspace.add", {"title": "Pressure Overview"})
        by_workspace = api_client.command(
            "project.search", {"query": "Pressure Overview", "type": "workspace"}
        )
        assert any(r["type"] == "workspace" for r in by_workspace["rows"])

    def test_search_rejects_empty_query(self, api_client, clean_state):
        """R1: empty/whitespace query is an explicit error, never match-all."""
        _load_fixture(api_client)

        for bad in ("", "   "):
            with pytest.raises(APIError):
                api_client.command("project.search", {"query": bad})

    def test_search_paging_is_deterministic(self, api_client, clean_state):
        """R1/AC4: paging via nextOffset covers every match exactly once and
        repeat calls return identical pages."""
        _load_fixture(api_client)

        first = api_client.command(
            "project.search", {"query": "CH-", "type": "dataset", "limit": 50}
        )
        total = first["matchCount"]
        assert total > 400

        seen = []
        offset = 0
        while True:
            page = api_client.command(
                "project.search",
                {"query": "CH-", "type": "dataset", "limit": 50, "offset": offset},
            )
            seen.extend(r["uniqueId"] for r in page["rows"])
            if "nextOffset" not in page:
                break
            offset = page["nextOffset"]

        assert len(seen) == total
        assert len(set(seen)) == total

        again = api_client.command(
            "project.search", {"query": "CH-", "type": "dataset", "limit": 50}
        )
        assert again["rows"] == first["rows"]


@pytest.mark.integration
@pytest.mark.project
class TestGroupGet:
    def test_group_get_both_id_spaces(self, api_client, clean_state):
        """AC3: the same group resolves via positional groupId and via stable
        uniqueId; the response carries both ids."""
        _load_fixture(api_client)

        listing = api_client.command("project.group.list", {"limit": 60})
        probe = listing["groups"][41]
        assert probe["title"] == "Absolute Pressure (psia)"
        assert probe["groupId"] == 41

        by_pos = api_client.command("project.group.get", {"groupId": probe["groupId"]})
        # uniqueId is sparse and distinct from the positional groupId; take it from the
        # response so the test does not assume a particular assigned value.
        assert by_pos["uniqueId"] != by_pos["groupId"]
        by_uid = api_client.command(
            "project.group.get", {"uniqueId": by_pos["uniqueId"]}
        )

        assert by_pos["title"] == by_uid["title"] == probe["title"]
        assert by_pos["groupId"] == by_uid["groupId"]
        assert by_pos["uniqueId"] == by_uid["uniqueId"]
        assert by_pos["datasetCount"] == 7

        summary = by_pos["datasetSummary"]
        assert len(summary) == 7
        for row in summary:
            for field in ("datasetId", "uniqueId", "index", "title"):
                assert field in row
        assert summary[0]["units"] == "psia"

    def test_group_get_selector_errors(self, api_client, clean_state):
        """AC3: both selectors, neither, and unknown ids fail with the
        two-id-spaces hint."""
        _load_fixture(api_client)

        with pytest.raises(APIError) as both:
            api_client.command("project.group.get", {"groupId": 0, "uniqueId": 1})
        assert "uniqueId" in both.value.message

        with pytest.raises(APIError) as neither:
            api_client.command("project.group.get", {})
        assert "selector" in neither.value.message.lower()

        with pytest.raises(APIError) as unknown:
            api_client.command("project.group.get", {"uniqueId": 999999})
        assert "uniqueId" in unknown.value.message


@pytest.mark.integration
@pytest.mark.project
class TestListPaging:
    def test_list_paging(self, api_client, clean_state):
        """AC4: limit=N returns exactly N, totals stay whole-project, and the
        nextOffset walk is gap- and duplicate-free."""
        _load_fixture(api_client)

        page = api_client.command("project.dataset.list", {"limit": 50})
        assert len(page["datasets"]) == 50
        assert page["datasetCount"] == DATASET_COUNT
        assert page["window"]["total"] == DATASET_COUNT
        assert "projectEpoch" in page

        seen = []
        offset = 0
        while True:
            page = api_client.command(
                "project.dataset.list", {"limit": 50, "offset": offset}
            )
            seen.extend(d["uniqueId"] for d in page["datasets"])
            if "nextOffset" not in page:
                break
            offset = page["nextOffset"]
        assert len(seen) == DATASET_COUNT
        assert len(set(seen)) == DATASET_COUNT

        groups = api_client.command("project.group.list", {"limit": 10})
        assert len(groups["groups"]) == 10
        assert groups["groupCount"] == GROUP_COUNT

        order = api_client.command(
            "project.dataset.getExecutionOrder", {"limit": 100, "offset": 100}
        )
        assert order["count"] == DATASET_COUNT
        positions = [row["position"] for row in order["order"]]
        assert positions == list(range(100, 200))

        repeat = api_client.command(
            "project.dataset.getExecutionOrder", {"limit": 100, "offset": 100}
        )
        assert repeat["order"] == order["order"]

    def test_unpaged_calls_remain_complete(self, api_client, clean_state):
        """R4 back-compat: without limit, the wire returns the whole project
        exactly as before (plus the additive window/epoch fields)."""
        _load_fixture(api_client)

        full = api_client.command("project.dataset.list", {})
        assert len(full["datasets"]) == DATASET_COUNT
        assert "nextOffset" not in full

        order = api_client.command("project.dataset.getExecutionOrder", {})
        assert len(order["order"]) == DATASET_COUNT

    def test_compact_defaults_fit_budget(self, api_client, clean_state):
        """AC5 (binding Q2 measurement): the commands whose rows are bounded fit
        the byte budget at the assistant shim's default limits. pytest speaks raw
        TCP, so the shim defaults (ToolDispatcher::withDefaultListLimit + handler
        defaults for search/group.get) are passed explicitly; if this fails,
        shrink those defaults, not the 3800-byte bound."""
        _load_fixture(api_client)

        budget = 3800
        calls = [
            ("project.search", {"query": "pressure"}),
            ("project.group.get", {"groupId": 41}),
            ("project.dataset.list", {"limit": 4}),
            ("project.dataset.getExecutionOrder", {"limit": 20}),
        ]
        for name, params in calls:
            result = api_client.command(name, params)
            size = _compact_size(result)
            assert size < budget, f"{name} default page is {size} bytes (>{budget})"

    def test_group_list_is_paged_not_byte_bounded(self, api_client, clean_state):
        """AC5 caveat (D6): a project.group.list row embeds the group's full nested
        datasets, so a single row is unbounded and the command cannot be made
        in-budget by row-count paging (project.search / project.group.get are the
        compact in-budget paths). What group.list guarantees is that it PAGES: the
        shim's default caps the count and the reply self-identifies as a window."""
        _load_fixture(api_client)

        page = api_client.command("project.group.list", {"limit": 5})
        assert len(page["groups"]) == 5
        assert page["groupCount"] == GROUP_COUNT
        assert page["window"] == {"offset": 0, "count": 5, "total": GROUP_COUNT}
        assert page["nextOffset"] == 5
