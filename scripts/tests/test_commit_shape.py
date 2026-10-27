# SPDX-FileCopyrightText: 2026 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Behavior tests for scripts/commit-shape.py: the rule engine on synthetic payloads, the
message parser on the command shapes Claude actually issues, and the CLI against a
throwaway git repository.

Run: pytest scripts/tests/test_commit_shape.py
"""

import importlib.util
import json
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
ENGINE = REPO / "scripts" / "commit-shape.py"
CONFIG = json.loads(
    (REPO / "scripts" / "commit-shape.json").read_text(encoding="utf-8")
)
TRAILER = "Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>"
MESSAGE = f"feat(ui): add a thing\n\nWhy the thing.\n\n{TRAILER}\n"


def _load():
    spec = importlib.util.spec_from_file_location("commit_shape", ENGINE)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


cs = _load()


def _payload(*paths, status="M"):
    return [(status, path) for path in paths]


def _rules(findings):
    return sorted(finding.split(":", 1)[0] for finding in findings)


def test_small_plain_commit_passes():
    payload = _payload("core/Ui/UI/Dashboard.cpp", "core/Ui/UI/Dashboard.h")
    assert cs.evaluate(payload, MESSAGE, CONFIG, require_coauthor=True) == []


def test_size_rule_needs_the_mechanical_trailer():
    paths = [f"core/Ui/Widgets/W{i}.cpp" for i in range(CONFIG["max_files"] + 1)]
    assert _rules(cs.evaluate(_payload(*paths), MESSAGE, CONFIG)) == ["size"]
    swept = MESSAGE + "\nMechanical: clang-format 19.1.0 over core/\n"
    assert cs.evaluate(_payload(*paths), swept, CONFIG) == []


def test_size_rule_passes_at_the_limit():
    paths = [f"core/Ui/Widgets/W{i}.cpp" for i in range(CONFIG["max_files"])]
    assert cs.evaluate(_payload(*paths), MESSAGE, CONFIG) == []


def test_isolation_licensing_lands_alone():
    mixed = _payload("app/src/Licensing/MachineID.cpp", "core/Ui/UI/Dashboard.cpp")
    assert _rules(cs.evaluate(mixed, MESSAGE, CONFIG)) == ["isolation"]
    alone = _payload(
        "app/src/Licensing/MachineID.cpp",
        "app/tests/Licensing/tst_machine_id.cpp",
        "doc/claude/specs/0042-license-token-hardening/consumers.md",
        "CLAUDE.md",
    )
    assert cs.evaluate(alone, MESSAGE, CONFIG) == []


def test_isolation_api_auth_and_workflows():
    assert _rules(
        cs.evaluate(
            _payload("core/Api/API/Server/ServerAuth.cpp", "core/Ui/A.cpp"),
            MESSAGE,
            CONFIG,
        )
    ) == ["isolation"]
    assert _rules(
        cs.evaluate(
            _payload(".github/workflows/ci.yml", "core/Ui/A.cpp"), MESSAGE, CONFIG
        )
    ) == ["isolation"]
    ci_alone = _payload(
        ".github/workflows/ci.yml",
        "scripts/ci/macos-verify-bundle.py",
        "scripts/tests/test_ci_workflow.py",
    )
    assert cs.evaluate(ci_alone, MESSAGE, CONFIG) == []


def test_two_isolated_groups_may_share_a_commit():
    payload = _payload(
        "app/src/Licensing/Trial.cpp", "core/Api/API/CommandCapabilities.cpp"
    )
    assert cs.evaluate(payload, MESSAGE, CONFIG) == []


def test_mechanical_trailer_never_lifts_isolation():
    """The 2026-09-03 shape: a sweep over the licensing surface stays refused."""
    paths = ["app/src/Licensing/MachineID.cpp"] + [
        f"core/Ui/W{i}.cpp" for i in range(70)
    ]
    swept = MESSAGE + "\nMechanical: review remediation sweep\n"
    assert _rules(cs.evaluate(_payload(*paths), swept, CONFIG)) == ["isolation"]


def test_spec_born_lands_alone():
    born = [("A", "doc/claude/specs/0104-thing/spec.md"), ("M", "core/Ui/A.cpp")]
    assert _rules(cs.evaluate(born, MESSAGE, CONFIG)) == ["spec-born"]
    docs_only = [
        ("A", "doc/claude/specs/0104-thing/spec.md"),
        ("A", "doc/claude/specs/0104-thing/plan.md"),
        ("A", "doc/claude/specs/0104-thing/tasks.md"),
    ]
    assert cs.evaluate(docs_only, MESSAGE, CONFIG) == []


def test_spec_edits_may_ride_with_the_implementation():
    payload = [("M", "doc/claude/specs/0104-thing/tasks.md"), ("M", "core/Ui/A.cpp")]
    assert cs.evaluate(payload, MESSAGE, CONFIG) == []


def test_ci_fix_cites_the_failing_run():
    bare = f"fix(ci): deploy the slice\n\nWhy.\n\n{TRAILER}\n"
    assert _rules(cs.evaluate(_payload(".github/workflows/ci.yml"), bare, CONFIG)) == [
        "ci-fix"
    ]
    cited = bare.replace(
        "Why.", "Why. https://github.com/Serial-Studio/Serial-Studio/actions/runs/1234"
    )
    assert cs.evaluate(_payload(".github/workflows/ci.yml"), cited, CONFIG) == []
    feature = f"ci: add the ledger ratchet to the lint job\n\n{TRAILER}\n"
    assert cs.evaluate(_payload(".github/workflows/ci.yml"), feature, CONFIG) == []


def test_coauthor_only_when_required():
    plain = "feat(ui): add a thing\n\nWhy.\n"
    assert cs.evaluate(_payload("core/Ui/A.cpp"), plain, CONFIG) == []
    assert _rules(
        cs.evaluate(_payload("core/Ui/A.cpp"), plain, CONFIG, require_coauthor=True)
    ) == ["coauthor"]
    assert _rules(
        cs.evaluate(_payload("core/Ui/A.cpp"), None, CONFIG, require_coauthor=True)
    ) == ["coauthor"]


@pytest.mark.parametrize(
    "command, expected",
    [
        ('git commit -m "feat: x"', "feat: x"),
        ('git commit -m "feat: x" -m "body here"', "feat: x\n\nbody here"),
        ('git commit --message="feat: y"', "feat: y"),
        ("git -c core.autocrlf=false commit -q -m'feat: z'", "feat: z"),
        (
            "git commit -m \"$(cat <<'EOF'\nfeat: h\n\nbody\n\n"
            + TRAILER
            + '\nEOF\n)"',
            "feat: h\n\nbody\n\n" + TRAILER,
        ),
        ("git commit", None),
    ],
)
def test_message_from_command(command, expected):
    assert cs.message_from_command(command) == expected


def test_unreadable_message_file_is_unknown():
    """A -F path the hook cannot open (a shell variable) is an unknown message, not an
    empty one, so the finding says so instead of blaming the trailer."""
    assert cs.message_from_command('git commit -F "$S/missing.txt"') is None


def test_message_from_file(tmp_path):
    note = tmp_path / "msg.txt"
    note.write_text("feat: f\n\nbody\n", encoding="utf-8")
    assert cs.message_from_command(f'git commit -F "{note}"') == "feat: f\n\nbody\n"


def _git(root, *args):
    subprocess.run(
        ["git", "-c", "user.email=t@t", "-c", "user.name=t", *args],
        cwd=str(root),
        check=True,
        capture_output=True,
    )


@pytest.fixture
def repo(tmp_path):
    """Throwaway repository carrying the real rule table."""
    root = tmp_path / "repo"
    (root / "scripts").mkdir(parents=True)
    (root / "scripts" / "commit-shape.json").write_text(
        json.dumps(CONFIG), encoding="utf-8"
    )
    subprocess.run(["git", "init", "-q"], cwd=str(root), check=True)
    _git(root, "add", ".")
    _git(root, "commit", "-q", "-m", "init")
    assert root.resolve() != REPO
    return root


def _stage(root, *rels):
    for rel in rels:
        path = root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("x\n", encoding="utf-8")
        _git(root, "add", rel)


def test_judge_command_reads_the_real_index(repo):
    _stage(repo, "core/Ui/A.cpp", "core/Ui/A.h")
    assert cs.judge_command(repo, f'git commit -m "feat: a" -m "{TRAILER}"', True) == []
    _stage(repo, "app/src/Licensing/Trial.cpp")
    findings = cs.judge_command(repo, f'git commit -m "feat: a" -m "{TRAILER}"', True)
    assert _rules(findings) == ["isolation"]


def test_judge_command_sees_the_worktree_for_dash_a(repo):
    _stage(repo, "core/Ui/A.cpp")
    _git(repo, "commit", "-q", "-m", "a")
    (repo / "core" / "Ui" / "A.cpp").write_text("y\n", encoding="utf-8")
    _stage(repo, "app/src/Licensing/Trial.cpp")
    assert cs.judge_command(repo, 'git commit -m "x"') == []
    assert _rules(cs.judge_command(repo, 'git commit -am "x"')) == ["isolation"]


def test_amend_without_a_message_reuses_head(repo):
    _stage(repo, "core/Ui/A.cpp")
    _git(repo, "commit", "-q", "-m", "feat: a", "-m", TRAILER)
    _stage(repo, "core/Ui/B.cpp")
    assert cs.judge_command(repo, "git commit --amend --no-edit", True) == []


def _cli(root, *args):
    return subprocess.run(
        [sys.executable, str(ENGINE), "--root", str(root), *args],
        capture_output=True,
        text=True,
    )


def test_cli_range_reports_each_offending_commit(repo):
    _stage(repo, "core/Ui/A.cpp")
    _git(repo, "commit", "-q", "-m", "feat: fine")
    _stage(repo, ".github/workflows/ci.yml")
    _git(repo, "commit", "-q", "-m", "fix(ci): no run cited")
    done = _cli(repo, "--range", "HEAD~2..HEAD")
    assert done.returncode == 1
    assert "ci-fix" in done.stdout
    assert "feat: fine" not in done.stdout
    assert "1 commit(s) with findings" in done.stdout


def test_cli_staged_with_message(repo):
    _stage(repo, "core/Ui/A.cpp")
    done = _cli(repo, "--staged", "--message", MESSAGE, "--require-coauthor")
    assert (done.returncode, done.stdout.strip()) == (0, "commit-shape: no findings")
    done = _cli(repo, "--staged", "--message", "feat: bare", "--require-coauthor")
    assert done.returncode == 1
    assert "coauthor" in done.stderr
