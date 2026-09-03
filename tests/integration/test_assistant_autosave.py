"""The assistant never writes the project file behind the user (spec 0075, J2).

Before this spec, every successful non-Safe tool call armed an 800 ms timer that saved the open
`.ssproj` with message boxes suppressed. With auto-approve on, a Confirm-tier edit therefore
reached disk with no human step at any point -- and telemetry-borne text could reach that edit
through Safe-tier tools.

The contract now: an assistant edit lands in the in-memory document and a checkpoint under the
backups folder; the file on disk changes only when the user saves, or when the model calls the
Confirm-tier `project.save`. These tests assert the file's hash, because that is the thing the
user cares about.

Requires a running instance with the API server enabled (Settings -> Miscellaneous).
"""

import hashlib
import time
from pathlib import Path

import pytest

pytestmark = [pytest.mark.integration, pytest.mark.project]


def file_hash(path: Path) -> str:
    """Return the SHA-256 of a project file, or "" when it does not exist."""
    if not path.exists():
        return ""
    return hashlib.sha256(path.read_bytes()).hexdigest()


@pytest.fixture
def saved_project(api_client, temp_dir):
    """A project saved to disk, so a later write is observable as a hash change."""
    path = Path(temp_dir) / "assistant_autosave.ssproj"
    api_client.create_new_project(title="Autosave guard")
    time.sleep(0.3)

    result = api_client.command("project.save", {"filePath": str(path)})
    if not result:
        pytest.skip("project.save unavailable; cannot pin the on-disk file")

    time.sleep(0.5)
    if not path.exists():
        pytest.skip("project file was not written; nothing to compare against")

    return path


def test_editor_mutation_does_not_touch_the_file(api_client, saved_project):
    """A project mutation marks the document modified; the file is untouched until a save."""
    before = file_hash(saved_project)

    api_client.command("project.group.add", {"title": "Added by test", "widgetType": 0})
    time.sleep(2.0)  # longer than the assistant's 800 ms debounce

    assert file_hash(saved_project) == before, "the document was written without a save"

    status = api_client.command("project.getStatus") or {}
    assert status.get(
        "modified", True
    ), "an unsaved mutation must leave the document modified"


def test_explicit_save_is_the_only_write(api_client, saved_project):
    """project.save is the one explicit, Confirm-tier path that changes the file."""
    before = file_hash(saved_project)

    api_client.command("project.group.add", {"title": "Second group", "widgetType": 0})
    time.sleep(1.5)
    assert file_hash(saved_project) == before

    api_client.command("project.save")
    time.sleep(1.0)
    assert file_hash(saved_project) != before, "project.save must write the document"


def test_checkpoints_are_available_after_an_edit(api_client, saved_project):
    """The edit is recoverable: the checkpoint list is the assistant's undo of last resort."""
    api_client.command("project.group.add", {"title": "Checkpoint me", "widgetType": 0})
    time.sleep(1.5)

    result = api_client.command("assistant.checkpoint", {"label": "spec-0075"})
    if not result:
        pytest.skip("assistant.checkpoint unavailable in this build")

    assert result.get("path"), "a checkpoint must report where it was written"

    listing = api_client.command("assistant.listCheckpoints") or {}
    entries = listing.get("checkpoints", listing.get("entries", []))
    assert entries, "the checkpoint just taken should be listed"
