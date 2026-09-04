"""
Project Document Integrity Tests (spec 0075, WP-F / R7)

Covers the invariants the 2026-09-01 review found broken in the project layer:

  H1 / R7.1  a display-setting change never writes the project document
  H4 / R7.6  an action payload edit dirties the document and reaches auto-save
  H5 / R7.2  a parser template pick is exactly one undo step
  H7 / R7.5  a new source's parser starts empty, never showing source 0's script
  H9 / R7.8  Dataset.sourceId follows its group on every mutation path
  loader     the legacy fixtures under tests/fixtures/projects/legacy/ still migrate

Two cases in AC7 are GUI-only and are skipped here with the manual recipe in the
skip message: the point-count spin box (ProjectModel::setPointCount is reachable
from QML only) and the reload-after-external-change prompt (the API path keeps the
in-memory document by design and never reaches promptDiskFileReload).

Copyright (C) 2020-2026 Alex Spataru
SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""

import hashlib
import json
import time
from pathlib import Path

import pytest

FIXTURES = Path(__file__).resolve().parents[1] / "fixtures" / "projects" / "legacy"

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


def _pace() -> None:
    """Stay under the API server's per-client message-rate cap."""
    time.sleep(0.007)


def _snapshot(api_client) -> dict:
    """The `snapshot` object of project.snapshot (title, filePath, modified, ...)."""
    _pace()
    return api_client.command("project.snapshot", {"sections": []})["snapshot"]


def _file_hash(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _undo_depth(api_client, limit: int = 50) -> int:
    """Undoes until history is empty, returning the number of steps consumed."""
    steps = 0
    for _ in range(limit):
        _pace()
        if not api_client.command("project.undo").get("performed"):
            return steps
        steps += 1
    raise AssertionError(f"undo did not drain history within {limit} steps")


def _save_to(api_client, path: Path) -> None:
    _pace()
    api_client.command("project.save", {"filePath": str(path)})
    assert path.exists(), f"project.save did not create {path}"


def _seed_saved_project(api_client, tmp_path: Path, title: str = "Integrity") -> Path:
    """A minimal saved project: one group, one dataset, one action, on disk."""
    _pace()
    api_client.command("project.new")
    _pace()
    api_client.command("project.setTitle", {"title": title})
    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)
    _pace()
    api_client.command("project.action.add", {})

    path = tmp_path / f"{title}.ssproj"
    _save_to(api_client, path)
    return path


# ---------------------------------------------------------------------------
# R7.1 -- the document on disk changes only through a save
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_display_setting_never_writes_the_document(api_client, clean_state, tmp_path):
    """A dashboard display setting must not flush unsaved edits to disk (H1).

    The shipped bug wrote the whole in-memory document from the Dashboard
    pointsChanged handler, bypassing setModified/markSaved/undo and the project
    lock; "Discard changes" then reloaded the file and the discarded edits came
    back. dashboard.setFps is the same class of change and is API-reachable.
    """
    path = _seed_saved_project(api_client, tmp_path, "DisplaySettings")
    before = _file_hash(path)

    _pace()
    api_client.command("project.setTitle", {"title": "Unsaved Edit"})
    assert _snapshot(api_client)["modified"] is True

    api_client.set_dashboard_fps(24)
    time.sleep(0.3)

    assert _file_hash(path) == before, "a display setting wrote the project file"
    assert _snapshot(api_client)["modified"] is True


@pytest.mark.project
def test_point_count_change_leaves_the_file_untouched():
    """GUI-only counterpart of the case above (AC7)."""
    pytest.skip(
        "ProjectModel::setPointCount is reachable from QML only. Manual check: "
        "open a saved project, edit the title without saving, change Points in "
        "Settings > Plotting, and confirm the .ssproj on disk is byte-identical "
        "and the title bar still shows unsaved changes."
    )


# ---------------------------------------------------------------------------
# R7.6 -- an action edit reaches the dashboard and the auto-save
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_action_payload_edit_dirties_and_autosaves(api_client, clean_state, tmp_path):
    """Editing an action's payload marks the document modified (H4).

    updateAction used to emit nothing when rebuildTree was false, so nothing was
    wired to markDirty/scheduleAutoSave: the edit sat in memory until an
    unrelated structural change happened to dirty the project.
    """
    path = _seed_saved_project(api_client, tmp_path, "ActionEdit")

    _pace()
    api_client.command(
        "project.action.update",
        {"actionId": 0, "txData": "PING\\n", "timerIntervalMs": 250},
    )

    assert _snapshot(api_client)["modified"] is True

    _save_to(api_client, path)
    saved = json.loads(path.read_text())
    assert saved["actions"][0]["txData"] == "PING\\n"
    assert saved["actions"][0]["timerIntervalMs"] == 250


# ---------------------------------------------------------------------------
# R7.2 -- one editor action is one undo step
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_parser_template_pick_undoes_template_and_params_together(
    api_client, clean_state
):
    """A template pick is one undo step and reverts its parameters with it (H5).

    Two scopes meant one Ctrl+Z left the new template's parameters sitting on the
    previous template's id, and the native parser rebuilt against a pair that
    never existed. The API path is already atomic (CommandRegistry wraps every
    command in a ProjectUndoFrame), so what this pins is the pairing itself; the
    Project Editor's own picker now goes through the same compound mutator.
    """
    _pace()
    api_client.command("project.new")
    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)

    _pace()
    templates = api_client.command("project.frameParser.listTemplates")
    ids = [t["id"] for t in templates.get("templates", [])]
    assert len(ids) >= 2, "need at least two native templates to switch between"

    _pace()
    api_client.command("project.frameParser.setTemplate", {"template": ids[0]})
    _undo_depth(api_client)

    _pace()
    api_client.command("project.frameParser.setTemplate", {"template": ids[0]})
    _pace()
    before = api_client.command("project.frameParser.getTemplate")

    target = next(t for t in ids if t != ids[0])
    _pace()
    api_client.command("project.frameParser.setTemplate", {"template": target})

    assert _undo_depth(api_client) == 1

    _pace()
    after = api_client.command("project.frameParser.getTemplate")
    assert after.get("template") == before.get("template")
    assert after.get("params") == before.get("params")


# ---------------------------------------------------------------------------
# R7.5 -- a new source's parser starts empty
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_new_source_parser_is_not_source_zero_script(api_client, clean_state):
    """A second source must not display source 0's frame parser (H7)."""
    _pace()
    api_client.command("project.new")
    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)

    marker = "/* source-zero-marker */\nfunction parse(frame) { return [frame]; }"
    _pace()
    api_client.command(
        "project.source.setFrameParserCode", {"sourceId": 0, "code": marker}
    )

    _pace()
    added = api_client.command("project.source.add", {})
    if not added:
        pytest.skip("multi-source requires a commercial build")

    sources = api_client.command("project.source.list").get("sources", [])
    if len(sources) < 2:
        pytest.skip("multi-source requires a commercial build")

    _pace()
    code = api_client.command("project.source.getFrameParserCode", {"sourceId": 1}).get(
        "code", ""
    )
    assert "source-zero-marker" not in code


# ---------------------------------------------------------------------------
# R7.8 -- derived dataset fields are normalised on every path
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_group_source_change_normalises_dataset_source_ids(api_client, clean_state):
    """Datasets follow their group's sourceId through the API path (H9)."""
    _pace()
    api_client.command("project.new")
    gid = api_client.add_group("Sensors")
    api_client.add_dataset(gid)

    _pace()
    added = api_client.command("project.source.add", {})
    sources = api_client.command("project.source.list").get("sources", [])
    if not added or len(sources) < 2:
        pytest.skip("multi-source requires a commercial build")

    _pace()
    api_client.command("project.group.update", {"groupId": gid, "sourceId": 1})

    config = api_client.command("project.exportJson")["config"]
    group = config["groups"][gid]
    assert group["sourceId"] == 1
    for dataset in group["datasets"]:
        assert dataset.get("sourceId", 1) == 1


# ---------------------------------------------------------------------------
# Loader migrations -- the hand-written legacy corpus
# ---------------------------------------------------------------------------


@pytest.mark.project
@pytest.mark.parametrize(
    "fixture",
    ["separator", "xaxis-index", "layout-keys", "schema-v0", "uid-dedup"],
)
def test_legacy_project_loads(api_client, clean_state, fixture):
    """Every legacy fixture still loads and lands on the current schema."""
    path = FIXTURES / f"{fixture}.ssproj"
    assert path.exists(), f"missing fixture {path}"

    _pace()
    api_client.command("project.open", {"filePath": str(path)})

    config = api_client.command("project.exportJson")["config"]
    assert config["groups"], f"{fixture} loaded with no groups"
    assert config.get("schemaVersion", 0) >= 3


@pytest.mark.project
def test_legacy_separator_parser_is_migrated(api_client, clean_state):
    """The pre-3.0 parse(frame, separator) body is rewritten on load."""
    _pace()
    api_client.command("project.open", {"filePath": str(FIXTURES / "separator.ssproj")})

    _pace()
    code = api_client.command("project.source.getFrameParserCode", {"sourceId": 0}).get(
        "code", ""
    )
    assert "separator" not in code.split("*/")[-1]
    assert 'frame.split(",")' in code or "frame.split(',')" in code


@pytest.mark.project
def test_legacy_xaxis_index_becomes_a_unique_id(api_client, clean_state):
    """An index-based xAxis is rebound to the referenced dataset's uniqueId."""
    _pace()
    api_client.command(
        "project.open", {"filePath": str(FIXTURES / "xaxis-index.ssproj")}
    )

    config = api_client.command("project.exportJson")["config"]
    datasets = config["groups"][0]["datasets"]
    base, signal = datasets[0], datasets[1]

    assert signal["xAxis"] == base["uniqueId"]
    assert signal["xAxis"] not in (-1, -2, 1)


@pytest.mark.project
def test_legacy_layout_keys_are_canonicalised(api_client, clean_state):
    """__layout__:N__ becomes layout:N and keeps only its data member."""
    _pace()
    api_client.command(
        "project.open", {"filePath": str(FIXTURES / "layout-keys.ssproj")}
    )

    config = api_client.command("project.exportJson")["config"]
    settings = config.get("widgetSettings", {})
    assert "__layout__:0__" not in settings
    assert "layout:0" in settings
    assert set(settings["layout:0"].keys()) == {"data"}


@pytest.mark.project
def test_legacy_duplicate_unique_ids_are_separated(api_client, clean_state):
    """Groups and datasets sharing a uniqueId are given distinct ones."""
    _pace()
    api_client.command("project.open", {"filePath": str(FIXTURES / "uid-dedup.ssproj")})

    config = api_client.command("project.exportJson")["config"]
    group_uids = [g["uniqueId"] for g in config["groups"]]
    dataset_uids = [d["uniqueId"] for g in config["groups"] for d in g["datasets"]]

    assert len(set(group_uids)) == len(group_uids)
    assert len(set(dataset_uids)) == len(dataset_uids)


# ---------------------------------------------------------------------------
# R7.4 -- a failed reload keeps the document attached (GUI prompt path)
# ---------------------------------------------------------------------------


@pytest.mark.project
def test_corrupt_external_write_keeps_document_attached(
    api_client, clean_state, tmp_path
):
    """An external write to the project file never detaches the document (H6).

    The reload prompt itself is GUI-only (API mode keeps the in-memory document
    by design), so this covers the API half: the watcher fires, the document
    stays attached to its path and stays saveable.
    """
    path = _seed_saved_project(api_client, tmp_path, "ExternalWrite")

    path.write_text("{ this is not valid json", encoding="utf-8")
    time.sleep(1.0)

    snapshot = _snapshot(api_client)
    assert snapshot["filePath"] == str(path)
    assert snapshot["modified"] is True

    _save_to(api_client, path)
    assert json.loads(path.read_text())["title"] == "ExternalWrite"
