"""
Behavior tests for scripts/commit-batch.py: a planned series of commits against a throwaway git
repository carrying the real rule table and both engines, so the batch and the commit hooks are
proven to agree.

Run: pytest scripts/tests/test_commit_batch.py
"""

import json
import shutil
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[2]
TRAILER = "Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"


def _git(root, *args):
    return subprocess.run(
        ["git", *args], cwd=str(root), check=True, capture_output=True, text=True
    ).stdout


@pytest.fixture
def repo(tmp_path):
    """Throwaway repository with commit-batch, commit-shape, its rules and the sanitize hook."""
    root = tmp_path / "repo"
    (root / "scripts").mkdir(parents=True)
    (root / ".claude" / "hooks").mkdir(parents=True)
    for rel in (
        "scripts/commit-batch.py",
        "scripts/commit-shape.py",
        "scripts/commit-shape.json",
        ".claude/hooks/commit-sanitize-gate.py",
    ):
        shutil.copy(REPO / rel, root / rel)

    subprocess.run(["git", "init", "-q"], cwd=str(root), check=True)
    _git(root, "config", "user.email", "test@example.com")
    _git(root, "config", "user.name", "Test")
    _git(root, "add", ".")
    _git(root, "commit", "-q", "-m", "init")
    return root


def _write(root, *rels):
    for rel in rels:
        path = root / rel
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("x\n", encoding="utf-8")


def _run(root, plan, *flags):
    (root / "plan.json").write_text(json.dumps({"commits": plan}), encoding="utf-8")
    return subprocess.run(
        [
            sys.executable,
            str(root / "scripts" / "commit-batch.py"),
            "plan.json",
            *flags,
        ],
        cwd=str(root),
        capture_output=True,
        text=True,
    )


def _subjects(root):
    return _git(root, "log", "--format=%s", "-n", "10").splitlines()


def test_a_series_lands_in_order_and_leaves_a_clean_tree(repo):
    spec = "doc/claude/specs/0999-x"
    _write(repo, f"{spec}/spec.md", f"{spec}/plan.md", "core/Ui/A.cpp", "core/Ui/A.h")
    plan = [
        {"message": f"docs(spec): x\n\n{TRAILER}", "paths": [f"{spec}/"]},
        {"message": f"feat(ui): x\n\n{TRAILER}", "rest": True},
    ]
    result = _run(repo, plan, "--require-coauthor")
    assert result.returncode == 0, result.stdout + result.stderr
    assert _subjects(repo)[:2] == ["feat(ui): x", "docs(spec): x"]
    assert _git(repo, "status", "--porcelain").strip() == "?? plan.json"


def test_a_spec_born_with_code_is_refused_and_nothing_is_staged(repo):
    _write(repo, "doc/claude/specs/0999-x/spec.md", "core/Ui/A.cpp")
    plan = [{"message": f"feat: x\n\n{TRAILER}", "rest": True}]
    result = _run(repo, plan, "--require-coauthor")
    assert result.returncode == 1
    assert "refused" in result.stdout + result.stderr
    assert _git(repo, "diff", "--cached", "--name-only").strip() == ""
    assert _subjects(repo) == ["init"]


def test_isolated_surface_must_land_alone(repo):
    _write(repo, "core/Api/API/SealedPolicy.cpp", "core/Ui/A.cpp")
    together = [{"message": f"feat: x\n\n{TRAILER}", "rest": True}]
    assert _run(repo, together, "--require-coauthor").returncode == 1

    apart = [
        {
            "message": f"feat(api): x\n\n{TRAILER}",
            "paths": ["core/Api/API/SealedPolicy.cpp"],
        },
        {"message": f"feat(ui): x\n\n{TRAILER}", "rest": True},
    ]
    result = _run(repo, apart, "--require-coauthor")
    assert result.returncode == 0, result.stdout + result.stderr
    assert _subjects(repo)[:2] == ["feat(ui): x", "feat(api): x"]


def test_plan_errors_stop_before_any_commit(repo):
    _write(repo, "core/Ui/A.cpp", "core/Ui/B.cpp")
    missing = [{"message": f"feat: x\n\n{TRAILER}", "paths": ["core/Ui/A.cpp"]}]
    assert _run(repo, missing, "--require-coauthor").returncode != 0

    no_trailer = [{"message": "feat: x", "rest": True}]
    assert _run(repo, no_trailer, "--require-coauthor").returncode != 0

    overlap = [
        {"message": f"feat: a\n\n{TRAILER}", "paths": ["core/Ui/"]},
        {"message": f"feat: b\n\n{TRAILER}", "paths": ["core/Ui/A.cpp"]},
    ]
    assert _run(repo, overlap, "--require-coauthor").returncode != 0
    assert _subjects(repo) == ["init"]


def test_check_mode_commits_nothing(repo):
    _write(repo, "core/Ui/A.cpp")
    plan = [{"message": f"feat: x\n\n{TRAILER}", "rest": True}]
    result = _run(repo, plan, "--check", "--require-coauthor")
    assert result.returncode == 0, result.stdout + result.stderr
    assert _subjects(repo) == ["init"]


def test_a_pre_staged_index_is_refused(repo):
    _write(repo, "core/Ui/A.cpp")
    _git(repo, "add", "core/Ui/A.cpp")
    plan = [{"message": f"feat: x\n\n{TRAILER}", "rest": True}]
    assert _run(repo, plan, "--require-coauthor").returncode != 0
