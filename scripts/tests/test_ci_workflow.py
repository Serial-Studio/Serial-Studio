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

# The pytest suite is a reusable workflow called once per platform, so every gate written for the
# old in-line 'test' job has to follow it here: a workflow one file away is not a smaller surface.
TEST_SUITE_YML = WORKFLOWS / "test-suite.yml"

# The repeated build / profiling / packaging sequences live in repo-local composite actions, so
# the supply-chain and secret-handling gates below have to walk them too: a third-party action
# pinned in ci.yml and floating one directory away is the same hole.
ACTIONS = sorted((REPO / ".github" / "actions").glob("*/action.yml"))
PINNED_FILES = [CI_YML, DOCS_YML, TEST_SUITE_YML] + ACTIONS

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

# The jobs that call test-suite.yml. One per packaged platform, each waiting on its own build
# only: a shared 'needs' block made every leg wait for the slowest build in the run.
TEST_CALLER_JOBS = ("test-linux", "test-linux-arm64", "test-macos", "test-windows")

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
    data = _load(path)
    if "jobs" not in data:
        for step in data.get("runs", {}).get("steps", []) or []:
            yield path.parent.name, step
        return
    for name, job in data["jobs"].items():
        for step in _steps(job):
            yield name, step


def _file_id(path):
    """Test id: the file name for a workflow, the directory name for a composite action."""
    return path.name if path.parent.name == "workflows" else path.parent.name


@pytest.fixture(scope="module")
def ci():
    return _jobs(CI_YML)


@pytest.fixture(scope="module")
def suite():
    """The single job inside the reusable pytest workflow."""
    return _jobs(TEST_SUITE_YML)["test"]


# --------------------------------------------------------------------------------------------
# Supply chain (L3)
# --------------------------------------------------------------------------------------------


@pytest.mark.parametrize("path", PINNED_FILES, ids=_file_id)
def test_every_action_is_sha_pinned(path):
    """A mutable tag can be retargeted at any commit; these actions see the signing secrets.

    A './.github/actions/<name>' reference is exempt: it resolves inside this commit, so there
    is no third party to retarget it -- and the action's own 'uses' entries are gated here too.
    """
    unpinned = [
        (job, step["uses"])
        for job, step in _all_steps(path)
        if "uses" in step
        and not step["uses"].startswith("./.github/actions/")
        and not re.fullmatch(r"[^@]+@[0-9a-f]{40}", step["uses"])
    ]
    assert not unpinned, f"actions must be pinned to a 40-hex commit SHA: {unpinned}"


@pytest.mark.parametrize("path", PINNED_FILES, ids=_file_id)
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


@pytest.mark.parametrize(
    "path", [CI_YML, DOCS_YML, TEST_SUITE_YML], ids=lambda p: p.name
)
def test_every_job_declares_permissions(path):
    """Without an explicit block a job inherits the repository default token scope."""
    missing = [name for name, job in _jobs(path).items() if "permissions" not in job]
    assert not missing, f"jobs without a permissions block: {missing}"


@pytest.mark.parametrize("path", PINNED_FILES, ids=_file_id)
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
    assert not needs & set(TEST_CALLER_JOBS)
    for job in TEST_CALLER_JOBS:
        assert "upload" not in str(ci[job]["needs"])
    assert "always()" not in str(ci["upload"].get("if", ""))


def test_every_packaged_platform_is_tested_against_its_own_build(ci):
    """
    Each leg waits on the one build whose artifact it downloads. A shared needs block (the
    pre-split shape) made the Linux suite idle until the slowest macOS package existed.
    """
    callers = {job: ci[job] for job in TEST_CALLER_JOBS}
    assert set(PACKAGE_JOBS) == {
        str(job["needs"]) for job in callers.values()
    }, "every packaged platform needs a test leg, waiting on that build alone"
    for name, job in callers.items():
        assert job["uses"].endswith("/test-suite.yml"), name
        assert job["name"], f"{name} has no name; the check renders as the job id"


def test_the_test_job_consumes_build_artifacts_not_a_release(suite):
    """Downloading from the Release is what forced test to run after upload."""
    steps = _steps(suite)
    assert any("download-artifact" in step.get("uses", "") for step in steps)
    assert not any("gh release download" in str(step.get("run", "")) for step in steps)


def test_the_test_suite_gates_the_run_but_not_the_release(ci, suite):
    """
    The suite was continue-on-error from 2e3a55533 (2026-09-05) to 2026-11-10, and a round-trip
    baseline drift stayed green behind a warning for days. A failure now reddens its leg; the
    release still ships because upload never waits on a test leg. Runner-timing flakes rerun
    through conftest's _FLAKY_TESTS rather than softening the step.
    """
    steps = _steps(suite)
    run_tests = next((step for step in steps if step.get("id") == "run_tests"), None)
    assert run_tests is not None, "the run_tests step is gone"
    assert "continue-on-error" not in run_tests, "the pytest step is soft again"

    outcome_readers = [
        step
        for step in steps
        if "steps.run_tests.outcome == 'failure'" in str(step.get("if"))
    ]
    assert (
        len(outcome_readers) >= 3
    ), "annotation plus the two hang-capture steps read the outcome"

    upload_needs = set(ci["upload"].get("needs", []))
    assert not upload_needs & set(
        TEST_CALLER_JOBS
    ), "a red test leg would hold the release"


@pytest.mark.parametrize("job", SANITIZER_JOBS)
def test_a_sanitizer_finding_fails_its_job_but_not_the_release(ci, job):
    """
    Both legs were job-level continue-on-error until 2026-11-10, so TSan, the only proof of the
    lock-free SPSC invariants, rendered green whatever it found. The run steps stay soft so one
    finding cannot skip the others; the annotation step exits non-zero instead.
    """
    assert "continue-on-error" not in ci[job], f"{job} is soft at the job level again"
    annotate = next(
        (step for step in _steps(ci[job]) if "Annotate" in str(step.get("name", ""))),
        None,
    )
    assert annotate is not None, f"{job} lost its annotation step"
    annotate_run = str(annotate.get("run", ""))
    assert "exit" in annotate_run, f"{job}: the annotation never fails the job"

    soft_steps = [s for s in _steps(ci[job]) if s.get("continue-on-error")]
    assert soft_steps, f"{job}: no soft run steps left, so this contract is stale"
    for step in soft_steps:
        assert (
            f"steps.{step.get('id')}.outcome" in annotate_run
        ), f"{job}: soft step {step.get('name')} can fail without failing the job"
    assert job not in set(
        ci["upload"].get("needs", [])
    ), f"{job} would hold the release"


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


def test_the_x86_64_slice_is_deployed_before_the_merge(ci):
    """
    lipo merges each slice's load commands untouched, so a slice lifted straight out of the
    build tree ships naming the runner's Qt directory in its rpath. Issue #392: every 4.x
    universal DMG aborted in dyld on Intel Macs while the arm64 half, the signature, the
    notarization and the arm64 test job all passed.
    """
    runs = [str(step.get("run", "")) for step in _steps(ci["build-macos-intel"])]
    deploy = [i for i, run in enumerate(runs) if "cmake --install build" in run]
    assert deploy, "build-macos-intel never deploys its bundle"
    lift = [
        i for i, run in enumerate(runs) if run.startswith("cp ") and "-x86_64" in run
    ]
    assert lift, "build-macos-intel never lifts the x86_64 slice"
    assert deploy[0] < lift[0], "the slice is lifted before the bundle is deployed"
    assert "build/app/" not in runs[lift[0]], "the slice is lifted from the build tree"


def test_the_offscreen_plugin_gets_the_bundle_rpath(ci):
    """
    libqoffscreen.dylib is copied raw from the Qt install for the headless tests, so its rpath
    still named the install's lib/ and the slice verifier refused both slices on its first real
    run (40511998448). The copy step must retarget it at Contents/Frameworks.
    """
    runs = [str(step.get("run", "")) for step in _steps(ci["build-macos-arm64"])]
    copy = [run for run in runs if "libqoffscreen.dylib" in run]
    assert copy, "build-macos-arm64 never bundles the offscreen plugin"
    assert (
        "install_name_tool -rpath" in copy[0]
    ), "the raw copy keeps the Qt install rpath"
    assert "@loader_path/../../Frameworks" in copy[0]


def test_unused_sql_driver_plugins_are_pruned_before_the_merge(ci):
    """
    macdeployqt ships every sqldrivers plugin; libqsqlmimer links /usr/local/lib/libmimerapi,
    which exists on no user's machine, and the app only opens SQLite.
    """
    steps = _steps(ci["build-macos-arm64"])
    runs = [str(step.get("run", "")) for step in steps]
    prune = [
        i for i, run in enumerate(runs) if "sqldrivers" in run and "-delete" in run
    ]
    assert prune, "build-macos-arm64 never prunes the SQL driver plugins"
    assert "libqsqlite.dylib" in runs[prune[0]], "the prune must keep the SQLite driver"
    archive = [i for i, run in enumerate(runs) if "COPYFILE_DISABLE" in run]
    assert archive and prune[0] < archive[0], "the prune must precede the bundle upload"


def test_the_universal_bundle_verifies_every_slice_before_signing(ci):
    """The static walk is the only gate that reads the x86_64 slice: the test job runs arm64."""
    steps = _steps(ci["build-macos"])
    ids = [step.get("id") for step in steps]
    assert (
        "verify-slice-load-commands" in ids
    ), "build-macos never checks the merged slices"
    verify = ids.index("verify-slice-load-commands")
    lipo = [i for i, s in enumerate(steps) if "lipo -create" in str(s.get("run", ""))]
    sign = [i for i, s in enumerate(steps) if "codesign" in str(s.get("run", ""))]
    assert lipo and sign
    assert (
        max(lipo) < verify < min(sign)
    ), "the slice check must follow lipo and precede codesign"


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


def test_clang_format_is_pinned_and_gated():
    """The formatter is a dependency like any other: an unpinned one restyles the whole tree on
    whoever upgrades first, and without a CI gate that restyle lands unnoticed."""
    manifest = (REPO / "tests" / "requirements.txt").read_text(encoding="utf-8")
    pin = re.search(r"^clang-format==(\S+)", manifest, re.M)
    assert pin, "tests/requirements.txt must pin clang-format to an exact version"

    lock = (REPO / "tests" / "requirements.lock").read_text(encoding="utf-8")
    assert f"clang-format=={pin.group(1)}" in lock, "the lock is stale for clang-format"

    runs = " ".join(str(step.get("run", "")) for _, step in _all_steps(CI_YML))
    assert "sanitize-commit.py --check-format" in runs, "no CI clang-format gate"


def test_the_ledger_ratchet_is_gated():
    """A prose-only ledger row is open debt; CI fails when the open count grows."""
    runs = " ".join(str(step.get("run", "")) for _, step in _all_steps(CI_YML))
    assert "ledger-verify.py --check" in runs, "no CI ledger ratchet"
    baseline = REPO / "scripts" / "ledger-baseline.json"
    assert baseline.is_file(), "scripts/ledger-baseline.json must be checked in"


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


# --------------------------------------------------------------------------------------------
# Spec 0077: the layering gates in CI
# --------------------------------------------------------------------------------------------

CORE_LIBRARIES = (
    "SerialStudioCore",
    "SerialStudioProtocols",
    "SerialStudioPipeline",
    "SerialStudioDevices",
    "SerialStudioStorage",
    "SerialStudioApi",
    "SerialStudioUi",
)


def _core_library_links():
    """The inter-library edges declared under core/, as {library: {libraries it links}}.

    Read from the CMake sources rather than from the workflow text: which targets a job
    spells out is a packaging choice, while what the job has to compile is the link graph.
    """
    links = {name: set() for name in CORE_LIBRARIES}
    for cmake in sorted((REPO / "core").glob("*/CMakeLists.txt")):
        text = cmake.read_text(encoding="utf-8")
        for block in re.findall(r"target_link_libraries\((.*?)\)", text, re.S):
            owner = re.match(r"\s*(SerialStudio\w+)", block)
            if not owner or owner.group(1) not in links:
                continue
            for dep in re.findall(r"SerialStudio::(\w+)", block):
                links[owner.group(1)].add("SerialStudio" + dep)
    return links


def _linked_closure(roots):
    """Every core library a build of `roots` has to compile, roots included."""
    links = _core_library_links()
    seen, pending = set(), list(roots)
    while pending:
        target = pending.pop()
        if target in seen:
            continue
        seen.add(target)
        pending.extend(links.get(target, ()))
    return seen


def test_the_core_libraries_build_alone_in_dependency_order(ci):
    """AC2: one job configures without the application and compiles all seven archives.

    The job names one target and lets CMake pull its dependencies, so asserting on the
    spelled-out target list pins the workflow's shape instead of the build graph: it fails
    when a step is collapsed (2026-09-12) and passes when a library drops out of the graph
    entirely. What AC2 needs is that nothing but core/ configures and that every one of the
    seven archives is reachable from what the job builds.
    """
    assert "build-core-libraries" in ci
    runs = [str(step.get("run", "")) for step in _steps(ci["build-core-libraries"])]
    configure = [run for run in runs if "cmake -G Ninja -B build/core-libs" in run]
    assert configure and "-DBUILD_GPL3=ON" in configure[0]

    targets = set(re.findall(r"--target (\S+)", " ".join(runs)))
    assert targets, "the job builds nothing"
    stray = targets - set(CORE_LIBRARIES)
    assert (
        not stray
    ), f"the library tier must build core/ targets only, got {sorted(stray)}"
    missing = set(CORE_LIBRARIES) - _linked_closure(targets)
    assert (
        not missing
    ), f"these libraries never get compiled by the job: {sorted(missing)}"


def test_the_library_job_does_not_gate_publication(ci):
    assert "build-core-libraries" not in ci["upload"]["needs"]


def test_the_lint_job_runs_the_bus_census(ci):
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["lint"]))
    assert "--bus-census --check" in runs


def test_the_lint_job_runs_the_clock_census_and_the_context_budget(ci):
    runs = " ".join(str(step.get("run", "")) for step in _steps(ci["lint"]))
    assert "python3 scripts/code-verify.py --clock-census --check" in runs
    assert "python3 scripts/claim-verify.py --context-budget --check" in runs


# --------------------------------------------------------------------------------------------
# Ship-artifact proof (spec 0107)
# --------------------------------------------------------------------------------------------

# Every 4.x universal DMG aborted on Intel Macs for months with CI green, because nothing in CI
# launched the files it published (issue #392). Each package job now launches the artifact it
# uploads, as that file, through the post-root QML suite: after the package exists, before its
# upload, never soft. Per leg: the package suffix, a marker of the step the proof follows, and
# the uploaded artifact name's suffix it precedes.
PROOF_LEGS = {
    "build-linux": (".AppImage", "actions/package-linux", "-Linux-x64.AppImage"),
    "build-linux-arm64": (
        ".AppImage",
        "actions/package-linux",
        "-Linux-arm64.AppImage",
    ),
    "build-macos": (".dmg", "stapler staple", "-macOS.dmg"),
    "build-windows": (".msi", "cpack --verbose", "-Windows.msi"),
}

# The configures that produce the bytes inside the DMG and the MSI; without the option the
# shipped binary answers "Unknown option 'selftest'" and the proof has nothing to launch.
SELFTEST_CONFIGURE_JOBS = ("build-macos-arm64", "build-macos-intel", "build-windows")


def _proof_steps(job, suffix):
    return [
        (i, step)
        for i, step in enumerate(_steps(job))
        if "--selftest-suite qml" in str(step.get("run", ""))
        and suffix in str(step.get("run", ""))
    ]


def _step_matches(step, marker):
    return marker in str(step.get("run", "")) or marker in str(step.get("uses", ""))


@pytest.mark.parametrize("job", sorted(PROOF_LEGS))
def test_every_package_leg_proves_the_artifact_it_uploads(ci, job):
    """One hard, bounded proof step per leg, after the package step and before its upload."""
    suffix, package_marker, upload_suffix = PROOF_LEGS[job]
    steps = _steps(ci[job])
    proofs = _proof_steps(ci[job], suffix)
    assert (
        len(proofs) == 1
    ), f"{job} needs exactly one proof step launching the {suffix}"
    index, proof = proofs[0]
    assert "continue-on-error" not in proof, f"{job}: the proof step is soft"
    assert proof.get("timeout-minutes"), f"{job}: the proof step has no timeout"
    package = [i for i, step in enumerate(steps) if _step_matches(step, package_marker)]
    assert (
        package and package[-1] < index
    ), f"{job}: the proof runs before the package exists"
    uploads = [
        i
        for i, step in enumerate(steps)
        if "upload-artifact" in str(step.get("uses", ""))
        and str((step.get("with") or {}).get("name", "")).endswith(upload_suffix)
    ]
    assert uploads, f"{job}: no upload of the {upload_suffix} artifact"
    assert index < uploads[0], f"{job}: the proof runs after the upload"


def test_the_macos_proof_runs_both_slices(ci):
    """The x86_64 slice executes under Rosetta on the same runner; a missing Rosetta is installed."""
    ((_, proof),) = _proof_steps(ci["build-macos"], ".dmg")
    run = str(proof["run"])
    assert run.count("--selftest-suite qml") == 2, "native and translated runs"
    assert "arch -x86_64" in run
    assert "--install-rosetta" in run
    assert "hdiutil attach" in run and "hdiutil detach" in run


def test_the_windows_proof_installs_runs_and_uninstalls(ci):
    """The MSI as a user meets it: silent install, the installed program, silent uninstall, gone."""
    ((_, proof),) = _proof_steps(ci["build-windows"], ".msi")
    run = str(proof["run"])
    assert "msiexec" in run and "/i " in run and "/x " in run
    assert "Test-Path $dir" in run, "a leftover install directory must fail the step"


@pytest.mark.parametrize("job", SELFTEST_CONFIGURE_JOBS)
def test_the_shipping_configures_expose_the_self_test(ci, job):
    """The last cmake configure in each job is the one whose binary ships."""
    configures = [
        str(step.get("run", ""))
        for step in _steps(ci[job])
        if "-DBUILD_COMMERCIAL=ON" in str(step.get("run", ""))
    ]
    assert configures, f"{job} has no commercial configure"
    assert (
        "-DSS_INAPP_TESTS=ON" in configures[-1]
    ), f"{job}: the shipping build lacks the flag"
