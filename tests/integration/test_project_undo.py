"""
Project Undo/Redo Integration Tests (spec 0031)

Covers the transactional undo history: randomized mutation round-trips,
atomic composite undo (group delete cascades, project.batch), the
project.undo / project.redo API verbs, empty-history responses, and
save-point tracking of the modified flag.

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import json
import random
import time

import pytest

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _export(api_client) -> str:
    """Canonical serialization of the current project document.

    `treeExpansion` is dropped: it is editor view state keyed by the *current*
    project title and rewritten by the debounced tree rebuild, so it drifts on
    its own clock and says nothing about whether undo restored the document.
    """
    result = api_client.command("project.exportJson")
    assert "config" in result
    config = dict(result["config"])
    config.pop("treeExpansion", None)
    return json.dumps(config, sort_keys=True)


def _pace() -> None:
    """Stay under the API server's 200-messages-per-second-per-client cap.

    The randomized round-trip issues hundreds of commands back to back; without
    pacing the server treats the burst as abuse and drops the connection, which
    surfaces as `EXECUTION_ERROR: API rate limit exceeded` mid-test.
    """
    time.sleep(0.007)


def _undo(api_client) -> dict:
    _pace()
    return api_client.command("project.undo")


def _redo(api_client) -> dict:
    _pace()
    return api_client.command("project.redo")


def _undo_all(api_client, limit: int = 300) -> int:
    """Undo until history is empty; returns the number of steps undone."""
    steps = 0
    for _ in range(limit):
        result = _undo(api_client)
        if not result.get("performed"):
            return steps
        steps += 1
    raise AssertionError(f"undo did not drain history within {limit} steps")


def _redo_all(api_client, limit: int = 300) -> int:
    steps = 0
    for _ in range(limit):
        result = _redo(api_client)
        if not result.get("performed"):
            return steps
        steps += 1
    raise AssertionError(f"redo did not drain history within {limit} steps")


def _random_mutations(api_client, rng: random.Random, count: int) -> int:
    """Apply count randomized single-op mutations; returns ops issued."""
    issued = 0
    for i in range(count):
        _pace()
        choice = rng.randrange(6)
        groups = api_client.list_groups()
        if choice == 0 or not groups:
            api_client.add_group(f"G{i}")
        elif choice == 1:
            gid = rng.randrange(len(groups))
            api_client.add_dataset(gid)
        elif choice == 2:
            gid = rng.randrange(len(groups))
            api_client.command(
                "project.group.update",
                {"groupId": gid, "title": f"Renamed {i}"},
            )
        elif choice == 3:
            api_client.command("project.setTitle", {"title": f"Project {i}"})
        elif choice == 4:
            datasets = api_client.list_datasets()
            if not datasets:
                api_client.add_group(f"G{i}")
            else:
                ds = datasets[rng.randrange(len(datasets))]
                api_client.update_dataset(
                    ds["groupId"], ds["datasetId"], title=f"DS {i}", units="V"
                )
        else:
            gid = rng.randrange(len(groups))
            api_client.command("project.group.duplicate", {"groupId": gid})

        issued += 1

    return issued


# ---------------------------------------------------------------------------
# AC1 -- randomized round-trip
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_random_mutation_undo_redo_round_trip(api_client, clean_state):
    """N random mutations, undo all -> baseline; redo all -> final state."""
    rng = random.Random(20260725)

    api_client.command("project.new")
    baseline = _export(api_client)
    issued = _random_mutations(api_client, rng, 50)
    assert issued == 50

    final = _export(api_client)
    assert final != baseline

    undone = _undo_all(api_client)
    assert undone > 0
    assert _export(api_client) == baseline

    redone = _redo_all(api_client)
    assert redone == undone
    assert _export(api_client) == final


# ---------------------------------------------------------------------------
# AC2 -- atomic composite undo of a cascading delete
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_group_delete_undo_restores_datasets_and_ids(api_client, clean_state):
    """Undoing a group delete restores order, uniqueIds, and every field."""
    gid = api_client.add_group("Telemetry")
    for i in range(5):
        api_client.add_dataset(gid)
        datasets = api_client.list_datasets()
        ds = datasets[-1]
        api_client.update_dataset(
            ds["groupId"], ds["datasetId"], title=f"Channel {i}", units=f"u{i}"
        )

    before = _export(api_client)

    api_client.delete_group(gid)
    assert _export(api_client) != before

    result = _undo(api_client)
    assert result.get("performed") is True
    assert _export(api_client) == before


# ---------------------------------------------------------------------------
# AC5 -- batch atomicity + empty-history responses
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_batch_undoes_as_single_step(api_client, clean_state):
    """A project.batch of mixed ops reverts with one project.undo."""
    baseline = _export(api_client)

    ops = [
        {"command": "project.group.add", "params": {"title": "A", "widgetType": 0}},
        {"command": "project.group.add", "params": {"title": "B", "widgetType": 0}},
        {"command": "project.dataset.add", "params": {"groupId": 0, "options": 0}},
        {"command": "project.setTitle", "params": {"title": "Batched"}},
    ]
    result = api_client.command("project.batch", {"ops": ops})
    assert result.get("failed") == 0

    mutated = _export(api_client)
    assert mutated != baseline

    undo_result = _undo(api_client)
    assert undo_result.get("performed") is True
    assert undo_result.get("undone") == "project.batch"
    assert _export(api_client) == baseline

    redo_result = _redo(api_client)
    assert redo_result.get("performed") is True
    assert _export(api_client) == mutated


@pytest.mark.project
def test_empty_history_responses_are_wellformed(api_client, clean_state):
    """undo/redo on empty history: success with performed:false, not errors."""
    api_client.command("project.new")
    result = _undo(api_client)
    assert result.get("performed") is False
    assert result.get("reason")

    result = _redo(api_client)
    assert result.get("performed") is False
    assert result.get("reason")


@pytest.mark.project
def test_new_mutation_discards_redo_tail(api_client, clean_state):
    """A mutation after undo forks history: redo reports nothing to redo."""
    api_client.add_group("First")
    _undo(api_client)
    api_client.add_group("Second")

    result = _redo(api_client)
    assert result.get("performed") is False


@pytest.mark.project
def test_multi_field_dataset_update_is_one_undo_step(api_client, clean_state):
    """Spec 0036 AC5: a registry-derived multi-field patch is a single undo step.

    Every field the generated applier writes lands inside the one ProjectUndoScope
    ProjectModel::updateDataset opens, so one undo must restore all of them.
    """
    api_client.add_group("Registry")
    api_client.add_dataset(0)

    baseline = _export(api_client)
    api_client.update_dataset(
        0,
        0,
        title="Patched",
        units="V",
        pltMin=-5.0,
        pltMax=5.0,
        fftSamplingRate=250,
        overviewDisplay=True,
    )

    mutated = _export(api_client)
    assert mutated != baseline

    assert _undo(api_client).get("performed") is True
    assert _export(api_client) == baseline

    assert _redo(api_client).get("performed") is True
    assert _export(api_client) == mutated


# ---------------------------------------------------------------------------
# AC6 -- save-point tracking of the modified flag
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_modified_flag_tracks_save_point(api_client, clean_state, tmp_path):
    """Undo past the save point re-dirties; redo back to it cleans."""
    api_client.add_group("Pre A")
    api_client.add_group("Pre B")
    api_client.add_dataset(0)

    proj_path = tmp_path / "undo_savepoint.ssproj"
    save_result = api_client.command("project.save", {"filePath": str(proj_path)})
    assert save_result.get("saved") is True
    assert api_client.get_project_status()["modified"] is False

    api_client.add_group("Post save")
    assert api_client.get_project_status()["modified"] is True

    assert _undo(api_client).get("performed") is True
    assert api_client.get_project_status()["modified"] is False

    assert _undo(api_client).get("performed") is True
    assert api_client.get_project_status()["modified"] is True

    assert _redo(api_client).get("performed") is True
    assert api_client.get_project_status()["modified"] is False

    time.sleep(0.1)
