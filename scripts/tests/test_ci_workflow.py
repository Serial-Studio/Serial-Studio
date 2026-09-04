# SPDX-FileCopyrightText: 2020-2025 Alex Spataru
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
"""
Structural gates over the GitHub Actions workflows and the pytest xfail policy.

The workflows are 3600 lines that nothing reviewed mechanically, and the 2026-09-01 source
review (spec 0075, findings L2, L3, L6, L7, L12, L14) found publication running before the
tests, a release republished from every branch, secrets interpolated into shell text, unpinned
third-party actions holding the Apple signing identity, a ctest tier nobody ran, and xfails with
no tracking reference. Every assertion here pins one of those.

The tier's platform spread is a separate question from its existence: it ran on macOS and Windows
too until 2026-09-03, when both legs were dropped as a second configure and build per platform for
suites that are toolchain-agnostic. What is pinned here is that it still runs on both Linux arches.

Run: pytest scripts/tests/test_ci_workflow.py
"""

import re
from pathlib import Path

import pytest
import yaml

REPO = Path(__file__).resolve().parents[2]
WORKFLOWS = REPO / ".github" / "workflows"
CI_YML = WORKFLOWS / "ci.yml"
DOCS_YML = WORKFLOWS / "docs.yml"

# The jobs that compile and run the throughput gate.
BUILD_JOBS = ("build-linux", "build-linux-arm64", "build-macos-arm64", "build-windows")

# The jobs that publish a self-contained package the test job consumes; the macOS gate runs in
# build-macos-arm64 but the DMG is assembled by build-macos. The benchmark-retry jobs were
# dropped on 2026-09-04 (531c62427): the throughput gate is one hard step per build job.
PACKAGE_JOBS = ("build-linux", "build-linux-arm64", "build-macos", "build-windows")

# The jobs that configure and run the ctest tier. Both Linux arches, because DSPSimd.h picks its
# SIMD lane from the target architecture; the macOS and Windows legs were dropped on 2026-09-03
# (they cost a second configure and build per platform for suites that are toolchain-agnostic).
CTEST_JOBS = ("build-linux", "build-linux-arm64")

# ASan and TSan cannot coexist in one build, so each sanitizer leg is its own job and they run in
# parallel; build-gpl3 is the only job left that compiles the GPL application.
SANITIZER_JOBS = ("sanitize", "sanitize-tsan")

# A tracking reference: a spec finding id ("0075 I1"), a GitHub issue, an upstream bug id, or
# a URL. "By design, not a finding" is not a reason to xfail -- delete the test instead.
TRACKING_REFERENCE = re.compile(r"(\b\d{4}\s+[A-Z]\d{1,2}\b|#\d+|QTBUG-\d+|https?://)")


def _load(path):
    """Parse a workflow file, with 'on:' surviving YAML 1.1 boolean coercion."""
    data = yaml.safe_load(path.read_text(encoding="utf-8"))
    return data


def _jobs(path):
    return _load(path)["jobs"]


def _steps(job):
    return job.get("steps", []) or []


def _all_steps(path):
    for name, job in _jobs(path).items():
        for step in _steps(job):
            yield name, step


@pytest.fixture(scope="module")
def ci():
    return _jobs(CI_YML)


# --------------------------------------------------------------------------------------------
# Supply chain (L3)
# --------------------------------------------------------------------------------------------


@pytest.mark.parametrize("path", [CI_YML, DOCS_YML], ids=lambda p: p.name)
def test_every_action_is_sha_pinned(path):
    """A mutable tag can be retargeted at any commit; these actions see the signing secrets."""
    unpinned = [
        (job, step["uses"])
        for job, step in _all_steps(path)
        if "uses" in step and not re.fullmatch(r"[^@]+@[0-9a-f]{40}", step["uses"])
    ]
    assert not unpinned, f"actions must be pinned to a 40-hex commit SHA: {unpinned}"


@pytest.mark.parametrize("path", [CI_YML, DOCS_YML], ids=lambda p: p.name)
def test_every_pin_names_its_version(path):
    """A bare SHA is unreviewable; the trailing comment says which tag it was."""
    text = path.read_text(encoding="utf-8")
    bare = [
        line.strip()
        for line in text.splitlines()
        if re.search(r"uses:\s*[^@]+@[0-9a-f]{40}\s*$", line)
    ]
    assert not bare, f"SHA pins must carry a '# vX.Y.Z' comment: {bare}"


# --------------------------------------------------------------------------------------------
# Least privilege and secret handling (L12)
# --------------------------------------------------------------------------------------------


@pytest.mark.parametrize("path", [CI_YML, DOCS_YML], ids=lambda p: p.name)
def test_every_job_declares_permissions(path):
    """Without an explicit block a job inherits the repository default token scope."""
    missing = [name for name, job in _jobs(path).items() if "permissions" not in job]
    assert not missing, f"jobs without a permissions block: {missing}"


@pytest.mark.parametrize("path", [CI_YML, DOCS_YML], ids=lambda p: p.name)
def test_no_secret_is_interpolated_into_shell_text(path):
    """A secret expanded into a run: body reaches the shell as literal text."""
    offenders = [
        (job, step.get("name", "<unnamed>"))
        for job, step in _all_steps(path)
        if "secrets." in str(step.get("run", ""))
    ]
    assert (
        not offenders
    ), f"secrets must travel through step env:, not run: text: {offenders}"


def test_only_the_publish_job_can_write_contents(ci):
    """contents: write is the token scope that can delete and recreate a release."""
    writers = [
        name
        for name, job in ci.items()
        if job.get("permissions", {}).get("contents") == "write"
    ]
    assert writers == ["upload"], f"unexpected contents:write jobs: {writers}"


# --------------------------------------------------------------------------------------------
# Publication gating (L2)
# --------------------------------------------------------------------------------------------


def test_publication_is_guarded_to_the_default_branch_or_a_tag(ci):
    condition = ci["upload"]["if"]
    assert "refs/heads/master" in condition
    assert "refs/tags/" in condition


def test_publication_has_a_per_ref_concurrency_group(ci):
    """The release tag is mutable shared state; two runs must not recreate it at once."""
    concurrency = ci["upload"]["concurrency"]
    assert "release-" in concurrency["group"]
    assert concurrency["cancel-in-progress"] is False


def test_publication_waits_on_the_builds_and_the_linters(ci):
    """
    Since 531c62427 the release goes out as soon as the last package exists; the pytest suite
    reports on it in parallel (it consumes the same artifacts) without holding it. A red build
    or a missed throughput gate still blocks: needs uses the default all-success semantics.
    """
    needs = set(ci["upload"]["needs"])
    assert "lint" in needs
    assert set(PACKAGE_JOBS) <= needs
    assert "test" not in needs
    assert "upload" not in ci["test"]["needs"]
    assert set(PACKAGE_JOBS) <= set(ci["test"]["needs"])
    assert "always()" not in str(ci["upload"].get("if", ""))


def test_the_test_job_consumes_build_artifacts_not_a_release(ci):
    """Downloading from the Release is what forced test to run after upload."""
    steps = _steps(ci["test"])
    assert any("download-artifact" in step.get("uses", "") for step in steps)
    assert not any("gh release download" in str(step.get("run", "")) for step in steps)


def test_the_test_suite_is_not_soft(ci):
    """continue-on-error on the suite is how master stayed green over failing tests."""
    for step in _steps(ci["test"]):
        if step.get("id") == "run_tests":
            assert "continue-on-error" not in step
            break
    else:
        pytest.fail("the run_tests step is gone")


def test_no_publication_escape_hatch_remains(ci):
    """The REQUIRE_TESTS_TO_PUBLISH flag went with the test gate; a stray guard step is drift."""
    assert "REQUIRE_TESTS_TO_PUBLISH" not in _load(CI_YML).get("env", {})
    for step in _steps(ci["upload"]):
        assert "REQUIRE_TESTS_TO_PUBLISH" not in str(step.get("run", ""))


# --------------------------------------------------------------------------------------------
# Test tiers (L6, L7)
# --------------------------------------------------------------------------------------------


@pytest.mark.parametrize("job", CTEST_JOBS)
def test_the_ctest_tier_runs_on_both_linux_arches(ci, job):
    """DSPSimd.h picks its SIMD lane from the target architecture, so x86_64 alone is half."""
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci[job]))
    assert "ctest --test-dir build/unit-ci" in runs


def test_the_sanitizer_legs_are_separate_parallel_jobs(ci):
    """One job compiled the tree twice back to back and was killed mid-link before ctest ran."""
    for job in SANITIZER_JOBS:
        assert job in ci, f"missing sanitizer job: {job}"
        assert "needs" not in ci[job], f"{job} must not serialize behind another job"


def test_the_asan_leg_covers_the_fuzz_targets_and_the_pipeline(ci):
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["sanitize"]))
    assert "-DDEBUG_SANITIZER=ON" in runs
    assert "-DENABLE_FUZZERS=ON" in runs
    assert "ctest --test-dir build/asan" in runs
    assert "--benchmark-hotpath --min-fps 1" in runs


def test_the_tsan_leg_proves_the_lock_free_invariants(ci):
    """TSan is the only proof of the lock-free SPSC invariants, and nothing ran it."""
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["sanitize-tsan"]))
    assert "-DENABLE_TSAN=ON" in runs
    assert "ctest --test-dir build/tsan" in runs


@pytest.mark.parametrize("job", SANITIZER_JOBS)
def test_the_sanitizer_legs_instrument_the_pro_modules(ci, job):
    """The Pro modules are what customers pay for and no sanitizer ever looked at them."""
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci[job]))
    assert "-DBUILD_GPL3=OFF" in runs
    assert "-DBUILD_COMMERCIAL=ON" in runs


@pytest.mark.parametrize("job", SANITIZER_JOBS)
def test_the_sanitizer_legs_link_within_the_runner_memory(ci, job):
    """Full DWARF through GNU ld got the runner OOM-killed twice on 2026-09-03."""
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci[job]))
    assert "-DSS_SANITIZER_DEBUG_LEVEL=-gline-tables-only" in runs
    assert "-fuse-ld=lld" in runs


def test_the_gpl3_application_still_builds_somewhere(ci):
    """Both sanitizer legs are Pro now, so this is the only GPL compile left in the workflow."""
    assert "build-gpl3" in ci
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["build-gpl3"]))
    assert "-DBUILD_GPL3=ON" in runs
    assert "--selftest-suite qml" in runs


def test_the_fuzz_corpora_replay_in_ci(ci):
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["sanitize"]))
    assert "-R '^fuzz_'" in runs


def test_no_benchmark_retry_job_remains(ci):
    """The retry jobs masked noisy runners; a reappearing one needs its own decision."""
    assert not [job for job in ci if job.startswith("benchmark-retry")]


@pytest.mark.parametrize("job", BUILD_JOBS)
def test_every_build_job_runs_the_throughput_gate_as_a_hard_step(ci, job):
    """256 kHz is a CI gate: one --benchmark-hotpath step per build job, never soft."""
    gates = []
    for step in _steps(ci[job]):
        run = str(step.get("run", "")).replace("'", "").replace(",", " ")
        if "--benchmark-hotpath" in run and "--min-fps 256000" in run:
            gates.append(step)
    assert gates, f"{job} runs no 256 kHz benchmark step"
    for step in gates:
        assert "continue-on-error" not in step


# --------------------------------------------------------------------------------------------
# Hardening (L1)
# --------------------------------------------------------------------------------------------


def test_the_fortify_level_is_asserted_from_the_real_compile_line(ci):
    """On-paper reasoning missed that the last -U cancels the level; CI reads the flags."""
    names = [str(step.get("name", "")) for step in _steps(ci["build-linux"])]
    assert any("FORTIFY" in name for name in names)
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["build-linux"]))
    assert "compile_commands.json" in runs
    assert "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON" in runs


# --------------------------------------------------------------------------------------------
# Dependency pinning (L4)
# --------------------------------------------------------------------------------------------


@pytest.mark.parametrize("path", [CI_YML, DOCS_YML], ids=lambda p: p.name)
def test_python_dependencies_install_from_the_hashed_lock(path):
    """A >= floor makes every job's dependency set a function of the day it ran."""
    installs = [
        str(step.get("run", ""))
        for _, step in _all_steps(path)
        if "tests/requirements" in str(step.get("run", ""))
    ]
    assert installs, "no step installs the repo's Python dependencies"
    for run in installs:
        assert "--require-hashes" in run and "requirements.lock" in run, run
    unpinned = [
        str(step.get("run", ""))
        for _, step in _all_steps(path)
        if re.search(
            r"^\s*pip install (?!--require-hashes)[a-zA-Z]",
            str(step.get("run", "")),
            re.M,
        )
    ]
    assert not unpinned, f"bare 'pip install <name>' is unpinned: {unpinned}"


# --------------------------------------------------------------------------------------------
# xfail policy (L14)
# --------------------------------------------------------------------------------------------


def _xfail_markers():
    """Yields (path, line, reason) for every @pytest.mark.xfail in tests/."""
    pattern = re.compile(r"@pytest\.mark\.xfail\((.*?)\)\s*\n", re.S)
    for path in sorted((REPO / "tests").rglob("*.py")):
        text = path.read_text(encoding="utf-8")
        for match in pattern.finditer(text):
            line = text.count("\n", 0, match.start()) + 1
            yield path.relative_to(REPO), line, match.group(1)


def test_every_xfail_names_a_finding_or_an_issue():
    """
    An xfail with no tracking reference is a defect nobody owns, and three of the four in the
    tree were 'by design, not a finding' -- which is a reason to delete the test, not to xfail
    it (finding L14).
    """
    offenders = [
        f"{path}:{line}"
        for path, line, body in _xfail_markers()
        if "reason=" not in body or not TRACKING_REFERENCE.search(body)
    ]
    assert not offenders, (
        "every @pytest.mark.xfail needs reason= naming a finding id, issue, or upstream bug: "
        f"{offenders}"
    )


def test_the_dos_marker_is_registered():
    """CI deselects '-m not dos'; an unregistered marker under --strict-markers is an error."""
    ini = (REPO / "tests" / "pytest.ini").read_text(encoding="utf-8")
    assert re.search(r"^\s+dos:", ini, re.M)
