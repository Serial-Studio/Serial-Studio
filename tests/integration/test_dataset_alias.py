"""
Dataset alias API integration tests.

Exercises the alias feature end-to-end against a running Serial Studio (API server
on localhost:7777):
  - project.dataset.getByUniqueId resolves a string alias equivalently to the
    numeric uniqueId, and reports a "not found" error naming an unknown alias.
  - project.dataset.update sets an alias and rejects a duplicate (uniqueness).

Requires the app up with Settings -> Miscellaneous -> Enable API Server.

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils.api_client import APIError


def _project():
    """A minimal project: one group, two datasets, the first carrying an alias."""
    return {
        "title": "Alias test",
        "frameEnd": "\n",
        "frameDetection": 0,
        "decoder": 0,
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {"title": "Channel One", "alias": "ATAM1-CH1", "index": 1},
                    {"title": "Channel Two", "index": 2},
                ],
            }
        ],
        "actions": [],
    }


@pytest.mark.integration
@pytest.mark.project
class TestAliasResolution:
    def test_alias_resolves_like_uniqueid(self, api_client, clean_state):
        """AC5: getByUniqueId with the string alias returns the same dataset as
        the numeric uniqueId."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)

        by_alias = api_client.command(
            "project.dataset.getByUniqueId", {"uniqueId": "ATAM1-CH1"}
        )
        assert by_alias["title"] == "Channel One"

        uid = by_alias["uniqueId"]
        assert isinstance(uid, int)

        by_uid = api_client.command("project.dataset.getByUniqueId", {"uniqueId": uid})
        assert by_uid["uniqueId"] == uid
        assert by_uid["title"] == "Channel One"

    def test_numeric_string_is_not_a_uniqueid(self, api_client, clean_state):
        """R3: a JSON string is always an alias, never a uniqueId; the numeric
        uniqueId passed as a string must not resolve."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)

        uid = api_client.command(
            "project.dataset.getByUniqueId", {"uniqueId": "ATAM1-CH1"}
        )["uniqueId"]

        with pytest.raises(APIError) as exc:
            api_client.command("project.dataset.getByUniqueId", {"uniqueId": str(uid)})
        assert "alias" in exc.value.message.lower()

    def test_unknown_alias_error_names_it(self, api_client, clean_state):
        """AC5: an unknown alias yields a not-found error that names the alias."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)

        with pytest.raises(APIError) as exc:
            api_client.command(
                "project.dataset.getByUniqueId", {"uniqueId": "no-such-alias"}
            )
        assert "no-such-alias" in exc.value.message


@pytest.mark.integration
@pytest.mark.project
class TestAliasUpdate:
    def test_update_sets_alias(self, api_client, clean_state):
        """R7: project.dataset.update accepts the alias field, then the alias
        resolves through getByUniqueId."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)

        api_client.command(
            "project.dataset.update",
            {"groupId": 0, "datasetId": 1, "alias": "ATAM1-CH2"},
        )
        time.sleep(0.2)

        resolved = api_client.command(
            "project.dataset.getByUniqueId", {"uniqueId": "ATAM1-CH2"}
        )
        assert resolved["title"] == "Channel Two"

    def test_duplicate_alias_rejected(self, api_client, clean_state):
        """R2: assigning an alias already used by another dataset is rejected."""
        api_client.load_project_from_json(_project())
        time.sleep(0.3)

        # Dataset 0 already has alias "ATAM1-CH1"; reusing it on dataset 1 fails.
        with pytest.raises(APIError) as exc:
            api_client.command(
                "project.dataset.update",
                {"groupId": 0, "datasetId": 1, "alias": "ATAM1-CH1"},
            )
        assert "ATAM1-CH1" in exc.value.message
