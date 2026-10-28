# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Behavior tests for the seven Claude Code hooks under .claude/hooks (spec 0101).

Each hook runs as a real subprocess with the event JSON on stdin, against a throwaway git
project (CLAUDE_PROJECT_DIR, TMPDIR and cwd all point into pytest's tmp_path) so nothing
touches the real repository. SS_HOOKS_UNDER_TEST re-aims the suite at a patched copy of the
hooks, which is how the claim-check mutation script proves each guard is pinned by a test.

Run: pytest scripts/tests/test_hooks.py
"""

import json
import os
import subprocess
import sys
import time
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).resolve().parent / "hook-fixtures"
ALL_HOOKS = [
    "canary-check.py",
    "claim-check.py",
    "commit-sanitize-gate.py",
    "commit-shape-gate.py",
    "protect-dirty-files.py",
    "syntax-on-save.py",
    "verify-on-save.py",
]
STUB = (
    "import sys\n"
    "from pathlib import Path\n"
    "here = Path(__file__).resolve().parent\n"
    "out = here / 'stub-out'\n"
    "code = here / 'stub-exit'\n"
    "sys.stdout.write(out.read_text() if out.is_file() else '')\n"
    "sys.exit(int(code.read_text()) if code.is_file() else 0)\n"
)


class HookRunner:
    """Callable that spawns a hook in the tmp project and records every spawn."""

    def __init__(self, hook_dir, project, tmp):
        self.hook_dir = hook_dir
        self.project = project
        self.tmp = tmp
        self.calls = []

    def __call__(self, name, event, env_overrides=None, stdin_text=None):
        """Run hook `name`; return (returncode, stdout, stderr)."""
        env = {k: v for k, v in os.environ.items() if k != "SS_CLAIM_CHECK"}
        env.update(
            CLAUDE_PROJECT_DIR=str(self.project),
            TMPDIR=str(self.tmp),
            SS_HOOK_RETRIES="1",
        )
        env.update(env_overrides or {})
        text = stdin_text if stdin_text is not None else json.dumps(event)
        self.calls.append((str(self.project), env["CLAUDE_PROJECT_DIR"]))
        done = subprocess.run(
            [sys.executable, str(self.hook_dir / name)],
            input=text,
            capture_output=True,
            text=True,
            env=env,
            cwd=str(self.project),
            timeout=60,
        )
        return done.returncode, done.stdout, done.stderr


START = {"hook_event_name": "SessionStart", "session_id": "hooktest"}
COMMIT = {"tool_input": {"command": "git commit -m x"}}


def _git(project, *args):
    """Run git in the tmp project with a throwaway identity."""
    subprocess.run(
        ["git", "-c", "user.email=t@t", "-c", "user.name=t", *args],
        cwd=str(project),
        check=True,
        capture_output=True,
    )


@pytest.fixture
def hook_dir():
    """Directory holding the hooks under test."""
    override = os.environ.get("SS_HOOKS_UNDER_TEST")
    return Path(override) if override else REPO / ".claude" / "hooks"


@pytest.fixture
def project(tmp_path):
    """Throwaway git repo with stub linters, a .code-report and a private TMPDIR."""
    root = tmp_path / "proj"
    (root / "scripts").mkdir(parents=True)
    (tmp_path / "tmp").mkdir()
    for stub in ("syntax-check.py", "code-verify.py"):
        (root / "scripts" / stub).write_text(STUB, encoding="utf-8")
    (root / ".code-report").write_text("report\n", encoding="utf-8")
    subprocess.run(["git", "init", "-q"], cwd=str(root), check=True)
    _git(root, "commit", "-q", "--allow-empty", "-m", "init")
    assert root.resolve() != REPO
    return root


@pytest.fixture
def run_hook(hook_dir, project, tmp_path):
    """Runner bound to the tmp project."""
    return HookRunner(hook_dir, project, tmp_path / "tmp")


def _stub(project, code, out=""):
    """Set the exit code and stdout the stub linters report."""
    (project / "scripts" / "stub-exit").write_text(str(code), encoding="utf-8")
    (project / "scripts" / "stub-out").write_text(out, encoding="utf-8")


def _json(stdout):
    """Parse hook stdout that must be one JSON object."""
    return json.loads(stdout)


@pytest.mark.parametrize("name", ALL_HOOKS)
def test_fail_open_non_json_stdin(run_hook, name):
    """Garbage on stdin exits 0 with nothing on stdout."""
    code, out, _ = run_hook(name, None, stdin_text="not json {")
    assert code == 0
    assert out == ""


@pytest.mark.parametrize("name", ["canary-check.py", "claim-check.py"])
def test_fail_open_missing_transcript(run_hook, name):
    """A Stop event whose transcript is gone exits 0 with nothing on stdout."""
    event = {"transcript_path": "/nonexistent/none.jsonl", "session_id": "s1"}
    code, out, _ = run_hook(name, event)
    assert code == 0
    assert out == ""


def test_hooks_never_aimed_at_real_repo(run_hook, project):
    """Every spawn used the tmp project as CLAUDE_PROJECT_DIR and cwd, never the repo."""
    run_hook("commit-sanitize-gate.py", {"tool_input": {"command": "ls"}})
    run_hook(
        "protect-dirty-files.py", {"hook_event_name": "SessionStart", "session_id": "x"}
    )
    run_hook("canary-check.py", None, stdin_text="x")
    assert len(run_hook.calls) == 3
    for cwd, root in run_hook.calls:
        assert Path(cwd).resolve() == project.resolve()
        assert Path(root).resolve() == project.resolve()
        assert Path(cwd).resolve() != REPO
        assert Path(root).resolve() != REPO


def _canary(run_hook, name, **extra):
    """Run the canary hook on fixture `name` with any extra event fields; return stdout."""
    event = {"transcript_path": str(FIXTURES / name), "session_id": "s1"}
    event.update(extra)
    code, out, _ = run_hook("canary-check.py", event)
    assert code == 0
    return out


def test_canary_healthy(run_hook):
    """An exact canary line is silent."""
    assert _canary(run_hook, "canary-healthy.jsonl") == ""


def test_canary_healthy_backticks(run_hook):
    """A backtick-wrapped but value-perfect canary is still healthy."""
    assert _canary(run_hook, "canary-healthy-backticks.jsonl") == ""


def test_canary_lost(run_hook):
    """`canary: lost` raises CANARY LOST."""
    out = _canary(run_hook, "canary-lost.jsonl")
    assert "CANARY LOST" in _json(out)["systemMessage"]


def test_canary_mutated(run_hook):
    """A drifted value raises CANARY MUTATED quoting the bad line."""
    message = _json(_canary(run_hook, "canary-mutated.jsonl"))["systemMessage"]
    assert "CANARY MUTATED" in message
    assert "6.11.1" in message


def test_canary_missing(run_hook):
    """No canary line at all raises CANARY MISSING."""
    out = _canary(run_hook, "canary-missing.jsonl")
    assert "CANARY MISSING" in _json(out)["systemMessage"]


def _healthy_text():
    """The canary-bearing final message of the healthy fixture, as the event would carry it."""
    last = (
        (FIXTURES / "canary-healthy.jsonl")
        .read_text(encoding="utf-8")
        .strip()
        .splitlines()[-1]
    )
    return json.loads(last)["message"]["content"][0]["text"]


def test_canary_event_text(run_hook):
    """The event's last_assistant_message is judged, not a transcript that has not flushed it."""
    out = _canary(
        run_hook, "canary-missing.jsonl", last_assistant_message=_healthy_text()
    )
    assert out == ""


def test_canary_event_text_missing(run_hook):
    """A final message without the canary warns even when the transcript's last entry has one."""
    out = _canary(
        run_hook, "canary-healthy.jsonl", last_assistant_message="Nothing at the end."
    )
    assert "CANARY MISSING" in _json(out)["systemMessage"]


def _dirty_tracked(project):
    """Commit tracked.txt, then modify it so git reports it dirty; return its path."""
    path = project / "tracked.txt"
    path.write_text("one\n", encoding="utf-8")
    _git(project, "add", "tracked.txt")
    _git(project, "commit", "-q", "-m", "add")
    path.write_text("two\n", encoding="utf-8")
    return path


def _guard_event(kind, path):
    """Build a protect-dirty-files event."""
    return {
        "hook_event_name": kind,
        "session_id": "hooktest",
        "tool_input": {"file_path": str(path)},
    }


def test_dirty_guard_asks_then_approves(run_hook, project):
    """A pre-dirty file asks once, PostToolUse approves it, the next pre-hook is silent."""
    dirty = _dirty_tracked(project)
    run_hook("protect-dirty-files.py", START)

    code, out, _ = run_hook("protect-dirty-files.py", _guard_event("PreToolUse", dirty))
    assert code == 0
    assert _json(out)["hookSpecificOutput"]["permissionDecision"] == "ask"

    _, post, _ = run_hook("protect-dirty-files.py", _guard_event("PostToolUse", dirty))
    assert post == ""
    _, again, _ = run_hook("protect-dirty-files.py", _guard_event("PreToolUse", dirty))
    assert again == ""


def test_dirty_guard_clean_file_silent(run_hook, project):
    """A file that was clean at session start is never asked about."""
    _dirty_tracked(project)
    clean = project / "clean.txt"
    run_hook("protect-dirty-files.py", START)
    _, out, _ = run_hook("protect-dirty-files.py", _guard_event("PreToolUse", clean))
    assert out == ""


def _stage(project, name, newer):
    """Stage a source file whose mtime is after (newer) or before .code-report."""
    source = project / name
    source.write_text("int x;\n", encoding="utf-8")
    report = project / ".code-report"
    now = time.time()
    os.utime(report, (now, now))
    stamp = now + 100 if newer else now - 100
    os.utime(source, (stamp, stamp))
    _git(project, "add", name)


def test_commit_gate_blocks_stale(run_hook, project):
    """A commit carrying a source newer than .code-report exits 2 with guidance."""
    _stage(project, "a.cpp", newer=True)
    code, _, err = run_hook("commit-sanitize-gate.py", COMMIT)
    assert code == 2
    assert "sanitize" in err


def test_commit_gate_fresh_commit_silent(run_hook, project):
    """A staged source older than .code-report passes."""
    _stage(project, "a.cpp", newer=False)
    code, out, err = run_hook("commit-sanitize-gate.py", COMMIT)
    assert (code, out, err) == (0, "", "")


def test_commit_gate_non_commit_silent(run_hook, project):
    """A non-commit command is ignored even with a stale file staged."""
    _stage(project, "a.cpp", newer=True)
    code, out, err = run_hook(
        "commit-sanitize-gate.py", {"tool_input": {"command": "ls"}}
    )
    assert (code, out, err) == (0, "", "")


SHAPE_ENGINE = ("commit-shape.py", "commit-shape.json")
COAUTHORED = {
    "tool_input": {
        "command": 'git commit -m "feat: x" -m "Co-Authored-By: Claude <noreply@anthropic.com>"'
    }
}


def _install_shape_engine(project):
    """Copy the real engine and rule table beside the stub linters."""
    for name in SHAPE_ENGINE:
        (project / "scripts" / name).write_bytes((REPO / "scripts" / name).read_bytes())


def _stage_paths(project, *rels):
    """Stage throwaway files at the given repo-relative paths."""
    for rel in rels:
        path = project / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("x\n", encoding="utf-8")
        _git(project, "add", rel)


def test_shape_gate_blocks_a_commit_without_the_trailer(run_hook, project):
    """Every commit the hook sees is Claude's, so the co-author trailer is mandatory."""
    _install_shape_engine(project)
    _stage_paths(project, "core/Ui/A.cpp")
    code, _, err = run_hook("commit-shape-gate.py", COMMIT)
    assert code == 2
    assert "coauthor" in err


def test_shape_gate_passes_a_small_coauthored_commit(run_hook, project):
    _install_shape_engine(project)
    _stage_paths(project, "core/Ui/A.cpp")
    assert run_hook("commit-shape-gate.py", COAUTHORED) == (0, "", "")


def test_shape_gate_blocks_a_mixed_licensing_commit(run_hook, project):
    """The 2026-09-03 shape: a licensing change inside a package of other files."""
    _install_shape_engine(project)
    _stage_paths(project, "app/src/Licensing/MachineID.cpp", "core/Ui/A.cpp")
    code, _, err = run_hook("commit-shape-gate.py", COAUTHORED)
    assert code == 2
    assert "isolation" in err
    assert "commit-shape.py" in err


def test_shape_gate_blocks_staging_in_the_commit_command(run_hook, project):
    """A `git add && git commit` chain hides the real index from a PreToolUse hook."""
    _install_shape_engine(project)
    event = {"tool_input": {"command": 'git add a.cpp && git commit -m "feat: x"'}}
    code, _, err = run_hook("commit-shape-gate.py", event)
    assert code == 2
    assert "staging" in err


def test_shape_gate_fails_open_without_the_engine(run_hook, project):
    """A project without scripts/commit-shape.py is not this repo; stay silent."""
    _stage_paths(project, "app/src/Licensing/MachineID.cpp", "core/Ui/A.cpp")
    assert run_hook("commit-shape-gate.py", COMMIT) == (0, "", "")


def test_shape_gate_non_commit_silent(run_hook, project):
    _install_shape_engine(project)
    _stage_paths(project, "app/src/Licensing/MachineID.cpp", "core/Ui/A.cpp")
    event = {"tool_input": {"command": "git status"}}
    assert run_hook("commit-shape-gate.py", event) == (0, "", "")


def _save_event(path):
    """Build a PostToolUse event for an edited file."""
    return {"hook_event_name": "PostToolUse", "tool_input": {"file_path": str(path)}}


@pytest.mark.parametrize("name", ["syntax-on-save.py", "verify-on-save.py"])
def test_save_hook_fires_on_failure(run_hook, project, name):
    """Stub exit 1 with text surfaces that text as additionalContext."""
    source = project / "a.cpp"
    source.write_text("int x;\n", encoding="utf-8")
    _stub(project, 1, "error: boom here\n")
    code, out, _ = run_hook(name, _save_event(source))
    assert code == 0
    assert "error: boom here" in _json(out)["hookSpecificOutput"]["additionalContext"]


@pytest.mark.parametrize("name", ["syntax-on-save.py", "verify-on-save.py"])
def test_save_hook_clean_silent(run_hook, project, name):
    """Stub exit 0 stays silent."""
    source = project / "a.cpp"
    source.write_text("int x;\n", encoding="utf-8")
    _stub(project, 0, "")
    assert run_hook(name, _save_event(source))[1] == ""


@pytest.mark.parametrize("name", ["syntax-on-save.py", "verify-on-save.py"])
def test_save_hook_non_cpp_silent(run_hook, project, name):
    """A .py path is not checked even when the stub would fail."""
    source = project / "a.py"
    source.write_text("x = 1\n", encoding="utf-8")
    _stub(project, 1, "error: boom here\n")
    assert run_hook(name, _save_event(source))[1] == ""


@pytest.mark.parametrize("name", ["syntax-on-save.py", "verify-on-save.py"])
def test_save_hook_missing_file_silent(run_hook, project, name):
    """A .cpp that does not exist is not checked."""
    _stub(project, 1, "error: boom here\n")
    assert run_hook(name, _save_event(project / "gone.cpp"))[1] == ""


def _claim(run_hook, fixture, session="s1", env=None, **extra):
    """Run claim-check on claim-<fixture>.jsonl; return its stdout."""
    event = {
        "transcript_path": str(FIXTURES / f"claim-{fixture}.jsonl"),
        "session_id": session,
    }
    event.update(extra)
    code, out, _ = run_hook("claim-check.py", event, env_overrides=env)
    assert code == 0
    return out


def _warns(out, uncredited=True):
    """Assert the output is a CLAIM WITHOUT EVIDENCE warning; optionally the nearest-run line."""
    message = _json(out)["systemMessage"]
    assert message.startswith("CLAIM WITHOUT EVIDENCE")
    if uncredited:
        assert "Nearest verification run not credited" in message
    return message


def test_claim_plain_pytest(run_hook):
    """A plain clean pytest run credits the claim."""
    assert _claim(run_hook, "plain-pytest") == ""


def test_claim_no_edit_claim(run_hook):
    """A claim without any edit is not a defect."""
    assert _claim(run_hook, "no-edit-claim") == ""


def test_claim_edit_no_verify(run_hook):
    """Edit then claim with no run warns."""
    _warns(_claim(run_hook, "edit-no-verify"), uncredited=False)


def test_claim_errored_run(run_hook):
    """A run whose result is_error is not credited."""
    _warns(_claim(run_hook, "errored-run"))


def test_claim_missing_result(run_hook):
    """A run with no recorded result is not credited."""
    _warns(_claim(run_hook, "missing-result"))


def test_claim_masked_interrupted(run_hook):
    """An interrupted run is not credited."""
    _warns(_claim(run_hook, "masked-interrupted"))


def test_claim_masked_background(run_hook):
    """A backgrounded run is not credited."""
    _warns(_claim(run_hook, "masked-background"))


def test_claim_masked_timeout(run_hook):
    """A run that hit the tool timeout is not credited."""
    _warns(_claim(run_hook, "masked-timeout"))


def test_claim_masked_reinterpreted(run_hook):
    """A run whose exit code was reinterpreted as success is not credited."""
    _warns(_claim(run_hook, "masked-reinterpreted"))


def test_claim_piped_pytest(run_hook):
    """A piped pytest run is not credited."""
    _warns(_claim(run_hook, "piped-pytest"))


def test_claim_or_true(run_hook):
    """`|| true` masks the exit status."""
    _warns(_claim(run_hook, "or-true"))


def test_claim_semicolon(run_hook):
    """`;` masks the exit status."""
    _warns(_claim(run_hook, "semicolon"))


def test_claim_comment(run_hook):
    """A trailing comment makes the line not plain."""
    _warns(_claim(run_hook, "comment"))


def test_claim_glob(run_hook):
    """An unquoted glob is refused."""
    _warns(_claim(run_hook, "glob"))


def test_claim_no_run_flag(run_hook):
    """`--collect-only` runs nothing and earns no credit."""
    _warns(_claim(run_hook, "no-run-flag"))


def test_claim_other_session(run_hook):
    """A verification run written by another session is not credited."""
    _warns(_claim(run_hook, "other-session"), uncredited=False)


def test_claim_subagent_run(run_hook):
    """A run on a subagent (sidechain) thread is not credited."""
    _warns(_claim(run_hook, "subagent-run"), uncredited=False)


def test_claim_backtick_claim(run_hook):
    """A claim phrase inside backticks is quoted, not asserted."""
    assert _claim(run_hook, "backtick-claim") == ""


def test_claim_blockquote_claim(run_hook):
    """A claim phrase in a blockquote is quoted, not asserted."""
    assert _claim(run_hook, "blockquote-claim") == ""


def test_claim_question_claim(run_hook):
    """A claim phrase inside a question is not a claim."""
    assert _claim(run_hook, "question-claim") == ""


def test_claim_fenced_claim(run_hook):
    """A claim phrase inside a code fence is not a claim."""
    assert _claim(run_hook, "fenced-claim") == ""


def test_claim_stop_hook_active(run_hook):
    """A re-entered Stop hook stays silent."""
    assert _claim(run_hook, "stop-hook-active", stop_hook_active=True) == ""


def test_claim_env_off(run_hook):
    """SS_CLAIM_CHECK=off disables the hook."""
    assert _claim(run_hook, "env-off", env={"SS_CLAIM_CHECK": "off"}) == ""


def test_claim_hidden_claim(run_hook):
    """A zero-width or soft-hyphen character inside the claim word does not hide the claim."""
    _warns(_claim(run_hook, "hidden-claim"), uncredited=False)


def test_claim_final_message(run_hook):
    """The event's last_assistant_message is the claim's text, not a lagging transcript entry."""
    out = _claim(
        run_hook, "final-message", last_assistant_message="Done. All tests pass."
    )
    _warns(out, uncredited=False)


def test_claim_event_text_preferred(run_hook):
    """A final message without a claim is silent even when an earlier flushed entry claimed."""
    assert (
        _claim(run_hook, "edit-no-verify", last_assistant_message="Still looking.")
        == ""
    )
