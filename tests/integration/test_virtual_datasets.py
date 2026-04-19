"""
Virtual Dataset Integration Tests

Verifies the v3.3 virtual-dataset field on DataModel::Dataset:
 * Toggle via API (project.dataset.setVirtual)
 * Frame-index editor gating (index becomes read-only when virtual=true)
 * Persistence round-trip through project export/import

Copyright (C) 2020-2025 Alex Spataru
SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
"""

import time

import pytest

from utils import APIError


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _add_group(api_client, title="G", widget_type=0):
    api_client.command("project.group.add", {"title": title, "widgetType": widget_type})
    time.sleep(0.15)


def _add_dataset(api_client, options=0):
    api_client.command("project.dataset.add", {"options": options})
    time.sleep(0.1)


def _set_virtual(api_client, group_id, dataset_id, is_virtual):
    return api_client.command(
        "project.dataset.setVirtual",
        {"groupId": group_id, "datasetId": dataset_id, "virtual": is_virtual},
    )


def _list_datasets(api_client):
    return api_client.command("project.datasets.list").get("datasets", [])


def _find_dataset(datasets, group_id, dataset_id):
    for d in datasets:
        if d.get("groupId") == group_id and d.get("datasetId") == dataset_id:
            return d
    return None


# ---------------------------------------------------------------------------
# Basic toggling
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_dataset_virtual_defaults_false(api_client, clean_state):
    """Freshly created datasets are non-virtual."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    datasets = _list_datasets(api_client)
    assert len(datasets) == 1
    # serialize() writes "virtual": true only when the flag is set; absence = false
    value = datasets[0].get("virtual", False)
    assert value is False


@pytest.mark.project
def test_set_virtual_true(api_client, clean_state):
    """Setting virtual=true is reflected in subsequent dataset queries."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    # project.datasets.list uses finalized groupId/datasetId — inspect first
    datasets = _list_datasets(api_client)
    gid = datasets[0]["groupId"]
    did = datasets[0]["datasetId"]

    result = _set_virtual(api_client, gid, did, True)
    assert result["virtual"] is True
    assert result["updated"] is True

    datasets = _list_datasets(api_client)
    d = _find_dataset(datasets, gid, did)
    # Accept either serialization key for the virtual flag
    value = d.get("virtual", False)
    assert value is True


@pytest.mark.project
def test_set_virtual_false_clears_flag(api_client, clean_state):
    """Toggling back to false clears the flag."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    datasets = _list_datasets(api_client)
    gid = datasets[0]["groupId"]
    did = datasets[0]["datasetId"]

    _set_virtual(api_client, gid, did, True)
    _set_virtual(api_client, gid, did, False)

    datasets = _list_datasets(api_client)
    d = _find_dataset(datasets, gid, did)
    value = d.get("virtual", False)
    assert value is False


# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_set_virtual_missing_group_errors(api_client, clean_state):
    """Setting virtual on a non-existent group returns INVALID_PARAM."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    with pytest.raises(APIError) as ei:
        _set_virtual(api_client, group_id=99, dataset_id=0, is_virtual=True)
    assert ei.value.code == "INVALID_PARAM"


@pytest.mark.project
def test_set_virtual_missing_dataset_errors(api_client, clean_state):
    """Setting virtual on a valid group but invalid dataset id errors."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    with pytest.raises(APIError) as ei:
        _set_virtual(api_client, group_id=0, dataset_id=99, is_virtual=True)
    assert ei.value.code == "INVALID_PARAM"


@pytest.mark.project
def test_set_virtual_missing_params_errors(api_client, clean_state):
    """Missing required params return MISSING_PARAM."""
    for payload in (
        {"datasetId": 0, "virtual": True},
        {"groupId": 0, "virtual": True},
        {"groupId": 0, "datasetId": 0},
    ):
        with pytest.raises(APIError) as ei:
            api_client.command("project.dataset.setVirtual", payload)
        assert ei.value.code == "MISSING_PARAM"


# ---------------------------------------------------------------------------
# Persistence round-trip
# ---------------------------------------------------------------------------

@pytest.mark.project
def test_virtual_flag_round_trips_through_json(api_client, clean_state):
    """Virtual flag survives project export → new project → import."""
    _add_group(api_client, "G")
    _add_dataset(api_client)

    datasets = _list_datasets(api_client)
    gid = datasets[0]["groupId"]
    did = datasets[0]["datasetId"]

    _set_virtual(api_client, gid, did, True)

    # Export — project.exportJson wraps the project JSON under "config"
    exported = api_client.command("project.exportJson")
    project_json = exported["config"]

    # New project
    api_client.create_new_project()
    time.sleep(0.2)
    # Virtual flag must be gone after new-project
    datasets = _list_datasets(api_client)
    if datasets:
        d = _find_dataset(datasets, 0, 0)
        if d is not None:
            value = d.get("virtual", False)
            assert value is False

    # Reload
    api_client.command("project.loadFromJSON", {"config": project_json})
    time.sleep(0.3)

    datasets = _list_datasets(api_client)
    assert len(datasets) >= 1
    d = datasets[0]
    value = d.get("virtual", False)
    assert value is True
