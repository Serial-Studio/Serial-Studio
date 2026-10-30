---
spec: 0107-ship-artifact-proof
title: Ship-artifact proof
status: approved     # draft -> approved -> in-progress -> done | shelved
approved: 2026-10-30 # ship the self-test flags on every leg; install Rosetta when missing
created: 2026-10-30
author: Alex Spataru
umbrella: 0104       # child C1 of the consolidation quarter; moves the `ship` dimension 4 -> 0
---

# Spec 0107 — Ship-Artifact Proof

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every 4.x macOS disk image aborted on launch on Intel Macs (issue #392), for months, with CI
green the whole time. CI was green because nothing in it ever launched the files it
published: the Intel slice is cross-compiled on an arm64 runner and never executed, the
Windows installer is built and uploaded but never installed, and the Linux package check
runs only the pre-root smoke suite (a Qt-version match) from an extracted tree. The
downstream test workflow launches a portable ZIP, not the installer, and is non-gating by
design. A release can therefore ship a binary that no machine in the pipeline has ever run.

The umbrella spec 0104 makes this its first consolidation item (R2) because it is the one
class of failure the quarter cannot ratchet away with a linter: it is only visible by
running the artifact. The measurement behind this spec sharpens the problem. The in-app
self-test entry points sit behind a build option that defaults to off, and among the package
legs only the two Linux ones turn it on. The macOS and Windows artifacts CI uploads today
cannot answer the self-test command at all; a Release build with the same default, run from
outside its build tree, answers `Unknown option 'selftest'`. Before any proof can run from
those files, they have to carry something to run.

## Goals

- Every file a release publishes has been launched in CI from that same file, and the
  in-app QML-instantiation self-test passed from it, before the file is uploaded.
- The macOS proof covers both slices of the universal binary: native arm64, and x86_64
  under Rosetta 2 translation on the same runner.
- The Windows proof covers the installer as a user meets it: silent install, run the
  installed program, silent uninstall, nothing left behind.
- A self-test failure fails the package job, so the artifact never reaches the continuous
  build or a tagged release.
- The proof is pinned: removing a leg's step, weakening it to non-blocking, or moving it
  after the upload fails the workflow-shape test suite.
- The quality score's `ship` dimension (spec 0104) reads 0 legs missing, from 4.

## Non-Goals

- A new self-test suite. The existing pre-root smoke suite and post-root QML suite are the
  payload; what changes is where they run from.
- The `.deb` and `.rpm` packages beyond the existing glibc-2.28 container smoke. They install
  the same binary the AppImage carries; the container checks stay as they are.
- The Windows portable ZIP and the Store package. The ZIP is the MSI's payload and the
  downstream suite already launches it; the Store package cannot be installed silently on a
  runner without sideloading, and the Store's own ingestion validates it.
- A windowed (GPU) launch on macOS or Windows. The proof runs offscreen; the Linux windowed
  GL check stays as it is.
- Replacing or gating the downstream test workflow.
- A second runner family for Intel macOS. The x86_64 slice runs under Rosetta on the arm64
  runner that already builds it.
- Any user-visible behavior. Shipping the self-test entry points adds opt-in command-line
  flags that exit before the GUI; nothing changes for a user who does not pass them.

## Requirements

1. **R1 — The shipped binary carries the self-test.** The artifact every package leg uploads
   (Linux x64 AppImage, Linux arm64 AppImage, universal macOS disk image, Windows MSI)
   answers the self-test command, as the Linux ones already do. The same build option state
   applies to every package leg; a leg without it has nothing for R2–R4 to launch.
2. **R2 — Linux: the AppImage runs itself.** Each AppImage is launched as the file CI
   uploads, not from a pre-extracted tree, headless and offscreen, and the QML suite passes
   from it. The composite glibc smoke keeps its extracted-tree run.
3. **R3 — macOS: both slices from the disk image.** The disk image that will be uploaded is
   mounted and its application launched twice, once natively and once under Rosetta 2 so the
   x86_64 slice executes, each run passing the QML suite. The bytes tested are the bytes
   uploaded: the proof runs after signing, notarization and stapling, and before upload.
4. **R4 — Windows: install, run, uninstall.** The MSI installs silently to its default
   location, the installed executable passes the QML suite headless and offscreen, the MSI
   uninstalls silently, and the install directory no longer exists afterwards. A leftover
   directory fails the step.
5. **R5 — Ordering and gating.** Each proof step runs after its package exists and before
   that package's upload, in the package job itself, and a failure fails the job. The publish
   job keeps depending on every package job, so no failed leg can be published around.
6. **R6 — Rosetta is never silently skipped.** When the runner lacks Rosetta 2, the macOS
   step installs it and then runs the x86_64 slice. A step that cannot run the Intel slice
   fails; it does not fall back to the native run alone.
7. **R7 — Pinned shape.** The workflow-shape test suite asserts, for each of the four package
   legs: a proof step is present, it launches the package file, it is ordered after the
   package step and before that package's upload, it is not marked non-blocking, and the
   macOS step runs both slices.
8. **R8 — Budget.** The proof adds at most five minutes of wall-clock to any single package
   job, measured on the probe run (the QML suite itself takes about ten seconds; the budget
   is for mounting, installing and a one-time Rosetta install).
9. **R9 — Probe first, then harden.** The first landing runs the four steps as non-blocking
   probes on master, in one CI run whose URL is recorded in this spec; the next commit
   removes the non-blocking marker and lands the shape test. This repo has no feature
   branches, so the probe is how a wall-clock premise gets measured.
10. **R10 — A failure is diagnosable from the log.** A failing proof prints the self-test
    report; a failing install or uninstall prints the installer's log; a failing mount or a
    missing Rosetta names itself. No proof step fails with only an exit code.

## Acceptance Criteria

- [ ] **AC1** — One CI run on master, URL recorded below, shows every proof step green:
      Linux x64, Linux arm64, macOS native, macOS under Rosetta, Windows install, run and
      uninstall (R1–R4, R6).
- [ ] **AC2** — `pytest scripts/tests/test_ci_workflow.py` passes and contains the pin from
      R7; editing the workflow to drop one proof step, mark it non-blocking or move it after
      its upload makes that test fail (R5, R7).
- [ ] **AC3** — The macOS job log for the Rosetta run shows the process architecture as
      x86_64, not arm64 (R3).
- [ ] **AC4** — The Windows job log shows the install directory present after install and
      absent after uninstall (R4).
- [ ] **AC5** — Per-leg added wall-clock, read from the step durations of the AC1 run, is
      within the R8 budget; the numbers are recorded in this spec.
- [ ] **AC6** — After the hardening commit, the step durations and conclusions of the first
      fully gating run are green on all four legs, with no `continue-on-error` on any proof
      step (R9).
- [ ] **AC7** — The `ship` dimension reads 0 (via `python scripts/quality-score.py --check`
      once spec C0 lands; by the plan's predicate counted by hand before that), and the
      umbrella's AC6 box is ticked with this spec's number.

## Constraints & Invariants

- **Same bytes.** The file the proof launches is the file the upload step takes, unchanged
  in between. On macOS that means after the stapled notarization ticket; on Linux the GPG
  signature is detached and the AppImage is untouched.
- The pinned macOS signing order (universal merge, verification, code signing) is not
  disturbed; the proof is appended after stapling.
- The Linux composite action's extracted-tree smoke and the windowed GL check stay as they
  are; this spec adds a step beside them and does not rewrite them.
- No new runner, runner family, or paid service. The existing runners carry every step.
- The self-test entry points stay opt-in flags that exit before the GUI; enabling them in
  the release builds must not change any default behavior, startup path, or binary other
  than registering the flags (the suites are compiled into every build already).
- The workflow change is an isolated surface: the commit that edits the CI workflow carries
  only the workflow and its shape test, and the probe landing is its own commit.
- The umbrella's commit-share rule (spec 0104 R1) counts every commit of this spec as
  consolidation work through the `umbrella: 0104` frontmatter.

## Premise Check

- **Question:** Can the artifacts CI uploads today run the in-app self-test at all, and what
  does running it cost?
- **Measured:** (1) The self-test entry points sit behind a build option that defaults to
  off. Of the package legs, only Linux x64 and Linux arm64 turn it on (the two sanitizer
  jobs do too); the macOS x86_64, arm64 and universal legs and the Windows leg do not, so
  no disk image or MSI ever built by CI can answer `--selftest`. (2) Confirmed on a binary:
  today's local Release build, same default, copied out of its build tree, answers
  `--version` in 2.1 s with exit code 0 and `Unknown option 'selftest'` in 0.3 s with exit
  code 1, both read back from the console, so a Windows GUI-subsystem executable reports
  cleanly to a CI step. (3) The QML suite from the Linux build tree took 10 s on the last
  green master run (2026-10-28, run 40791060493), and the AppImage's extracted-tree smoke
  passed in that run, so the suite and the offscreen plugin are already proven in one
  package. (4) `macos-latest` resolved to `macos-26-arm64` (image 20261021.0445.8) in that
  run; the image's published readme does not mention Rosetta, so its presence is unknown
  until the probe. (5) Today's job costs: universal macOS 5 min (notarization 202 s of it),
  Windows 39 min (MSI creation 83 s), Linux x64 78 min (packaging 749 s); R8's five minutes
  is small against every leg. (6) The MSI's default install directory is deterministic (the
  package name under Program Files), so R4 can assert its absence after uninstall.
- **Verdict:** Proceed, with one scope decision folded in as R1: the macOS and Windows
  release builds gain the self-test entry points, otherwise the proof has nothing to launch.
  Two unknowns remain that only the probe run can settle, Rosetta on the runner and the
  offscreen platform plugin inside the disk image and the MSI; R9 exists for them.

## Open Questions

- **Ship the self-test flags in the macOS and Windows release binaries?** Recommended yes:
  Linux has shipped them since the self-test tier existed, the suites are compiled into every
  build already, and the flags are inert unless passed. The alternative, a separate
  test-enabled build per platform, tests bytes that are not shipped and defeats the umbrella's
  R2. **Decided 2026-10-30: ship them.**
- **Install Rosetta on the runner when it is missing, or fail?** Recommended install: it is a
  one-time cost per run inside the R8 budget, and failing would leave the Intel slice unproven
  until someone changes the runner image. **Decided 2026-10-30: install.**
- **Probe run:** `<URL, recorded at R9>`; **hardening run:** `<URL, recorded at AC6>`;
  **per-leg added seconds:** `<recorded at AC5>`.
