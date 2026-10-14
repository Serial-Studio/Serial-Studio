"""
Loading a project from caller-supplied material asks the script-change consent.

Spec 0098, AC2 and AC3 (R6, R7). `sessions.regress`, `sessions.verify`, `sessions.replay` and
`assistant.restore` load a project the caller chose (a database's stored project, a candidate
document or path, a snapshot file), so they need the same script-change consent as
`project.open`. Restoring a checkpoint also accepts only checkpoints the app recorded for the
open project, and never touches the open project's file for anything else.

Two instance setups, selected by environment variable so a run never guesses:

  SS_TEST_HEADLESS_NO_CONSENT=1
      A headless (offscreen) Serial Studio with the API server on and WITHOUT
      SERIAL_STUDIO_API_AUTO_CONSENT. Nothing can answer a prompt, so every consent gate
      answers "denied by the user" deterministically. Runs the refusal cases.

  SS_TEST_AUTO_CONSENT=1
      A Serial Studio started WITH SERIAL_STUDIO_API_AUTO_CONSENT=1 (every gate granted).
      Runs the positive cases: a foreign file is still not a checkpoint, and the `backupPath`
      a mutation returns still restores.

With neither variable the module skips. The sessions.* commands exist only in commercial
builds and skip elsewhere. Each test starts and ends on `project.new`.
"""

import hashlib
import json
import os
import sys
import time
from dataclasses import dataclass
from pathlib import Path

import pytest

sys.path.insert(0, str(Path(__file__).parent.parent))
from utils.api_client import APIError

HEADLESS_NO_CONSENT = os.environ.get("SS_TEST_HEADLESS_NO_CONSENT") == "1"
AUTO_CONSENT = os.environ.get("SS_TEST_AUTO_CONSENT") == "1"

pytestmark = [
    pytest.mark.security,
    pytest.mark.skipif(
        not (HEADLESS_NO_CONSENT or AUTO_CONSENT),
        reason="set SS_TEST_HEADLESS_NO_CONSENT=1 (headless, no AUTO_CONSENT) or "
        "SS_TEST_AUTO_CONSENT=1 (AUTO_CONSENT instance); see the module docstring",
    ),
]

needs_no_consent = pytest.mark.skipif(
    not HEADLESS_NO_CONSENT
    or AUTO_CONSENT
    or os.environ.get("SERIAL_STUDIO_API_AUTO_CONSENT") == "1",
    reason="needs SS_TEST_HEADLESS_NO_CONSENT=1 and an instance without AUTO_CONSENT",
)
needs_auto_consent = pytest.mark.skipif(
    not AUTO_CONSENT, reason="needs SS_TEST_AUTO_CONSENT=1 and an AUTO_CONSENT instance"
)

SETTLE_SECONDS = 0.6
AUTOSAVE_SETTLE_SECONDS = 2.0
MARKER_SNIPPET = "import sys; open(sys.argv[1], 'a').write('x')"
CANDIDATE_TITLE = "Candidate (must never load)"
OPEN_TITLE = "Open project (spec 0098)"


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------


@dataclass(frozen=True)
class Candidate:
    """A project full of marker code, as a document, a JSON string and a file."""

    marker: Path
    project: dict
    path: Path

    @property
    def json_text(self) -> str:
        return json.dumps(self.project)


def _exec_call(marker: Path) -> str:
    args = json.dumps(["-c", MARKER_SNIPPET, str(marker)])
    return f'apiCall("system.exec", {{program: {json.dumps(sys.executable)}, args: {args}}});'


def _candidate_project(marker: Path) -> dict:
    """A project whose parser, transform and control script each try to start a program."""
    call = _exec_call(marker)
    return {
        "title": CANDIDATE_TITLE,
        "frameStart": "/*",
        "frameEnd": "*/",
        "decoder": 0,
        "decoderMethod": 0,
        "checksum": "",
        "checksumAlgorithm": "",
        "frameDetection": 1,
        "controlScriptCode": f"{call}\nfunction setup() {{ {call} }}\nfunction loop() {{}}\n",
        "sources": [
            {
                "sourceId": 0,
                "title": "Device A",
                "busType": 0,
                "frameStart": "/*",
                "frameEnd": "*/",
                "frameDetection": 1,
                "checksumAlgorithm": "",
                "decoderMethod": 0,
                "frameParserCode": f'{call}\nfunction parse(frame) {{ return frame.split(","); }}\n',
                "frameParserLanguage": 0,
                "connectionSettings": {},
            }
        ],
        "groups": [
            {
                "title": "G",
                "widget": "",
                "datasets": [
                    {
                        "title": "X",
                        "units": "",
                        "value": "%1",
                        "index": 1,
                        "min": 0,
                        "max": 100,
                        "transformCode": f"{call}\nfunction transform(v) {{ return v; }}\n",
                    }
                ],
            }
        ],
        "actions": [],
    }


def _sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _assert_script_change_refused(error: APIError) -> None:
    """The gate's refusal: retryable 'need consent' or final 'denied', naming script changes."""
    assert error.code in ("EXECUTION_ERROR", "CONSENT_REQUIRED"), error
    text = error.message.lower()
    assert "script changes" in text, error
    assert "denied by the user" in text or "need the user's consent" in text, error


def _require_command(client, name: str) -> None:
    if not client.command_exists(name):
        pytest.skip(f"{name} is not registered in this build")


def _marker_stays_absent(candidate: Candidate) -> None:
    deadline = time.monotonic() + SETTLE_SECONDS
    while time.monotonic() < deadline and not candidate.marker.exists():
        time.sleep(0.05)

    assert not candidate.marker.exists(), "candidate marker code ran"


@pytest.fixture
def candidate(tmp_path):
    marker = tmp_path / "candidate.marker"
    project = _candidate_project(marker)
    path = tmp_path / "candidate.ssproj"
    path.write_text(json.dumps(project), encoding="utf-8")
    return Candidate(marker=marker, project=project, path=path)


@pytest.fixture
def open_project(api_client, tmp_path):
    """A file-backed open project, so a write to its file is observable as a hash change."""
    api_client.command("project.new")
    api_client.command("project.setTitle", {"title": OPEN_TITLE})
    target = tmp_path / "open.ssproj"
    try:
        api_client.command("project.save", {"filePath": str(target)})
    except APIError as error:
        pytest.skip(f"cannot create an on-disk project to hash: {error}")

    time.sleep(AUTOSAVE_SETTLE_SECONDS)
    status = api_client.command("project.getStatus")
    path = Path(status.get("filePath") or target)
    if not path.exists():
        pytest.skip("the open project has no file on disk to compare against")

    yield path

    try:
        api_client.command("project.new")
    except (APIError, ConnectionError, TimeoutError):
        pass


# ---------------------------------------------------------------------------
# No consent available: every loading command is refused
# ---------------------------------------------------------------------------

LOADING_CALLS = [
    pytest.param(
        "sessions.regress",
        lambda c: {"sessionId": 1, "projectJson": c.json_text},
        id="sessions.regress-json",
    ),
    pytest.param(
        "sessions.regress",
        lambda c: {"sessionId": 1, "projectPath": str(c.path)},
        id="sessions.regress-path",
    ),
    pytest.param("sessions.verify", lambda c: {"sessionId": 1}, id="sessions.verify"),
    pytest.param("sessions.replay", lambda c: {"sessionId": 1}, id="sessions.replay"),
    pytest.param(
        "assistant.restore",
        lambda c: {"path": str(c.path)},
        id="assistant.restore",
    ),
]


@needs_no_consent
class TestRefusedWithoutConsent:
    """AC2 and the refusal half of AC3, against an instance nobody can ask."""

    @pytest.mark.parametrize(
        "command,params",
        [
            pytest.param(
                "project.loadJson", lambda c: {"config": c.project}, id="project.loadJson"
            ),
            pytest.param(
                "project.open", lambda c: {"filePath": str(c.path)}, id="project.open"
            ),
        ],
    )
    def test_environment_denies_script_changes(
        self, api_client, candidate, command, params
    ):
        """Control: today's loaders are refused here, so the instance really has no consent."""
        with pytest.raises(APIError) as caught:
            api_client.command(command, params(candidate))

        _assert_script_change_refused(caught.value)
        _marker_stays_absent(candidate)

    @pytest.mark.parametrize("command,params", LOADING_CALLS)
    def test_loading_command_is_refused(self, api_client, candidate, command, params):
        """The gate answers before the handler, so even an unknown session id is refused."""
        _require_command(api_client, command)

        with pytest.raises(APIError) as caught:
            api_client.command(command, params(candidate))

        _assert_script_change_refused(caught.value)
        _marker_stays_absent(candidate)

    def test_foreign_restore_leaves_the_open_project_untouched(
        self, api_client, candidate, open_project
    ):
        """A foreign temp file is refused and the open project's file is byte-identical."""
        before = _sha256(open_project)

        with pytest.raises(APIError) as caught:
            api_client.command("assistant.restore", {"path": str(candidate.path)})

        _assert_script_change_refused(caught.value)
        time.sleep(AUTOSAVE_SETTLE_SECONDS)
        assert _sha256(open_project) == before, "the open project's file changed"
        assert api_client.command("project.getStatus")["title"] == OPEN_TITLE
        _marker_stays_absent(candidate)


# ---------------------------------------------------------------------------
# Consent granted: the checkpoint rule still holds, and the documented undo still works
# ---------------------------------------------------------------------------


@needs_auto_consent
class TestRestoreWithConsent:
    """AC3 with every gate granted: only the open project's own checkpoints restore."""

    def test_foreign_path_is_not_a_checkpoint(self, api_client, candidate, open_project):
        """Past the consent gate the handler still refuses a file the app did not record."""
        before = _sha256(open_project)

        with pytest.raises(APIError) as caught:
            api_client.command("assistant.restore", {"path": str(candidate.path)})

        assert caught.value.code == "INVALID_PARAM", caught.value
        assert "is not a checkpoint of the open project" in caught.value.message
        time.sleep(AUTOSAVE_SETTLE_SECONDS)
        assert _sha256(open_project) == before, "the open project's file changed"
        assert api_client.command("project.getStatus")["title"] == OPEN_TITLE
        _marker_stays_absent(candidate)

    def test_backup_path_of_a_mutation_still_restores(self, api_client, open_project):
        """Passing a mutation's reply `backupPath` back to restore undoes that mutation."""
        group_id = api_client.add_group("Seed")
        api_client.add_dataset(group_id)
        time.sleep(0.3)
        assert api_client.command("project.getStatus")["datasetCount"] == 1

        reply = api_client.command(
            "project.dataset.delete", {"groupId": group_id, "datasetId": 0}
        )
        backup = reply.get("backupPath")
        assert backup, f"the delete reply names no snapshot: {reply}"
        assert api_client.command("project.getStatus")["datasetCount"] == 0

        restored = api_client.command("assistant.restore", {"path": backup})

        assert restored["restored"] is True, restored
        assert api_client.command("project.getStatus")["datasetCount"] == 1
