---
spec: 0107-ship-artifact-proof
phase: plan
status: approved     # approved 2026-10-30
updated: 2026-10-30
---

# Plan 0107 — Ship-Artifact Proof

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Everything lands in `ci.yml` and its shape test; no C++ and no CMake changes. The self-test
entry points already sit behind the existing `SS_INAPP_TESTS` configure option (default off,
distinct from `SS_BUILD_TESTS`, which builds the 206-binary ctest tier), and the two Linux
legs already pass it; R1 is three more `-DSS_INAPP_TESTS=ON` lines on the shipping configures
of `build-macos-arm64`, `build-macos-intel` and `build-windows`. R2 to R4 are one inline
`run:` step per package job, placed after the package exists and before that package's
upload: the AppImage launched as the file with the runtime's extract-and-run fallback, the
stapled DMG mounted read-only and its app run natively and under `arch -x86_64`, the MSI
installed silently, run from `Program Files`, uninstalled and its directory asserted gone.
R7 is one parametrized assertion per leg in `scripts/tests/test_ci_workflow.py`, keyed on
the step's `run` text the same way the umbrella's `ship` census will read it. R9 is two
commits: the steps land with `continue-on-error: true`, the probe run's URL and durations
go into `spec.md`, and the next commit drops the marker and lands the shape test.

## Affected subsystems & files

| File | Change |
|------|--------|
| `.github/workflows/ci.yml` | `-DSS_INAPP_TESTS=ON` on the shipping configure of `build-macos-arm64` (PGO stage 2, line ~990), `build-macos-intel` (~1194) and `build-windows` (PGO stage 2, ~1616). Four new steps: one in `build-linux` and one in `build-linux-arm64` between `📦 Package for Linux` and `📤 Upload artifact: AppImage`; one in `build-macos` between `📌 Staple` and `📤 Upload artifact: DMG`; one in `build-windows` between `📦 Create MSI installer` and `📦 Create portable TGZ` (so it precedes `📤 Upload artifact: MSI installer` by construction). |
| `scripts/tests/test_ci_workflow.py` | `PROOF_LEGS` table (job, package suffix, the step the proof follows, the upload it precedes) and four tests: every leg has exactly one proof step in the right window with no `continue-on-error`; the macOS step runs both slices; the Windows step installs and uninstalls; the three shipping configures carry `SS_INAPP_TESTS`. |
| `doc/claude/specs/0107-ship-artifact-proof/spec.md` | Probe and hardening run URLs, per-leg added seconds, AC boxes, status. |
| `doc/claude/specs/0104-consolidation-quarter/tasks.md` | Tracker row C1: spec `0107`, status, `ship` reading; T2 and (after hardening) T4's C1 half. |

Not touched, on purpose: `app/CMakeLists.txt`, `app/src/Misc/CLI.*`, `app/src/SelfTest/`
(the suites compile into every build; only the flag registration is behind the define),
`.github/actions/package-linux/` and `scripts/ci/linux-smoke-packages.sh` (the extracted-tree
glibc smoke stays), `.github/workflows/test-suite.yml` (non-gating by design).

## Architecture & data flow

Control only; no data path changes. Per leg:

- **Linux (`build-linux`, `build-linux-arm64`).** The composite action leaves
  `<EXECUTABLE>-<VERSION>-Linux-<arch>.AppImage` at the workspace root and its GPG signature
  detached beside it. The step runs that file with `APPIMAGE_EXTRACT_AND_RUN=1` (the
  `ubuntu-24.04` images carry no `libfuse2`; the runtime extracts to a temp dir and execs
  `AppRun` from there, which is still the uploaded bytes being launched) and
  `QT_QPA_PLATFORM=offscreen`, arguments `--headless --selftest-suite qml`. The AppDir already
  bundles `libqoffscreen.so` (`scripts/ci/linux-create-appdir.sh`), proven by the existing
  extracted-tree smoke on the same runner.
- **macOS (`build-macos`).** After `📌 Staple`: probe Rosetta with `arch -x86_64 /usr/bin/true`;
  when it fails, `sudo softwareupdate --install-rosetta --agree-to-license`, then probe again
  (a second failure fails the step, R6). `hdiutil attach -nobrowse -readonly -mountpoint
  "$RUNNER_TEMP/ssdmg"` on the stapled DMG, a `trap` detaches on every exit. Run
  `<mount>/<APPLICATION>.app/Contents/MacOS/<EXECUTABLE> --headless --selftest-suite qml`
  twice: bare, then under `arch -x86_64`. For AC3 the step prints `lipo -archs` of the mounted
  executable and `arch -x86_64 /usr/bin/uname -m` (reads `x86_64` under translation) in the
  same log group as the translated run. The bundle already carries a universal
  `libqoffscreen.dylib` in `Contents/PlugIns/platforms`, retargeted at `Contents/Frameworks`
  in `build-macos-arm64` and verified slice by slice by `macos-verify-bundle.py` before signing.
- **Windows (`build-windows`).** PowerShell, after `📦 Create MSI installer` has moved the MSI to
  the workspace root. `msiexec /i <msi> /qn /norestart /l*v install.log`, wait, non-zero exit
  prints the log tail and throws. The install directory is deterministic:
  `$env:ProgramFiles\<APPLICATION>` (CPack `CPACK_PACKAGE_INSTALL_DIRECTORY` is the package
  name, WiX x64 roots it under `ProgramFiles64Folder`) with the executable under `bin\`
  (`RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}`). The step asserts the executable exists and
  lists the directory when it does not (R10). It launches the installed executable with
  `Start-Process -PassThru -NoNewWindow`, stdout and stderr redirected to files that are
  printed afterwards, `QT_QPA_PLATFORM=offscreen`, `QT_LOGGING_TO_CONSOLE=1` (so Qt's message
  handler writes `[selftest] …` to stderr instead of the debugger channel), and
  `QT_QPA_PLATFORM_PLUGIN_PATH` at the runner's Qt `plugins\platforms` (see Tradeoffs). The
  verdict is `$p.ExitCode` from the process handle, with the printed `[selftest] … 0 failures`
  summary as the human-readable confirmation. Then `msiexec /x <msi> /qn /norestart /l*v
  uninstall.log` and `Test-Path` on the install directory: present means fail, with a
  recursive listing of what remained (AC4, R10).
- **Ordering (R5).** Each step is a sibling of its upload step inside the same job, placed
  textually before it; a failing step fails the job; `upload` already `needs` all four package
  jobs with default (all-succeed) semantics, pinned by
  `test_publication_waits_on_the_builds_and_the_linters`.
- **Budget (R8).** Every proof step carries `timeout-minutes` (5 on Linux, 10 on macOS to
  absorb a one-time Rosetta install, 10 on Windows for two `msiexec` passes) so a hang reads
  as a timeout rather than a six-hour job.

## Hotpath & threading impact

- **Touches the hotpath?** No. No C++ changes at all.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged.

## Data model & persistence

None.

## API / SDK surface

None. The three macOS/Windows binaries gain the already-existing `--selftest` and
`--selftest-suite` options (inert unless passed; `CLI::process` exits before the GUI). The
benchmark flags are registered unconditionally today, so nothing else changes.

## QML / UI

None.

## Tradeoffs & alternatives considered

Candidate shapes, sketched before converging:

- **Inline-per-leg** (chosen): one inline `run:` step per package job. The step text is what
  the Actions log shows and what the shape test and the umbrella's `ship` census read.
- **Scripted proof**: `scripts/ci/prove-package.{sh,ps1}` called by thin steps. Cleaner YAML,
  but it violates the umbrella's pin grammar (a step whose `run` contains the suite flag and
  the package suffix), which would then have to inspect script bodies; and the macOS and
  Windows legs are already inline by convention. Rejected.
- **Gate the downstream suite**: make `test-suite.yml` blocking and let it carry the proof.
  Violates R5 (the proof lives in the package job, before upload) and the Windows caller runs
  the portable ZIP, not the MSI. Rejected.

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| R1 mechanism | (a) `-DSS_INAPP_TESTS=ON` on the three shipping configures in `ci.yml`; (b) flip the CMake default to ON | **(a)**: workflow-only, so the whole change stays on the isolated surface; local builds keep today's default; the option already exists and the Linux legs already use it. |
| PGO stage-1 configures | add the flag to both stages, or only the shipping stage-2 build | **Stage 2 only** (and the single Intel configure): the instrumented training binary never ships and registering two options cannot move a profile. |
| Windows offscreen plugin | (a) `QT_QPA_PLATFORM_PLUGIN_PATH` at the runner's Qt `plugins\platforms`; (b) an `install(FILES qoffscreen.dll)` rule so the MSI ships it like the AppImage and DMG do | **(a)** for this spec: the MSI's bytes stay exactly what users get, no CMake edit, and the plugin is harness, not product. The probe settles whether the env path is honored; if not, (b) is a one-rule follow-up named in `spec.md`, not slipped in. |
| Windows verdict | `$p.ExitCode` vs a verdict file (the benchmark's pattern) | **ExitCode**, plus the printed summary. `-PassThru` keeps the handle; the benchmark went through a file because its verdict is a number, not a status. The premise check read exit codes 0 and 1 back from a GUI-subsystem binary. |
| macOS run location | run from the read-only mount vs copy the app out (the `test-suite.yml` pattern) | **From the mount**: the bytes tested are the bytes stapled; the app writes nothing into its bundle. |
| Rosetta missing | install vs fail | **Install** (decided in `spec.md`); the re-probe after install is what makes R6 hold. |
| Shape-test anchor | hand-written predicate now vs wait for C0's `quality-score.ship_legs()` | **Hand-written now**; umbrella T4 re-points it when C0 lands, so this spec does not block on a sibling. |

## Risks & mitigations

- **Rosetta install time or refusal on the runner image.** R8 budgets it; `timeout-minutes`
  bounds it; the probe run measures it. If the image forbids `softwareupdate`, the spec's R6
  says fail, and the open question goes back to the maintainer with the log.
- **The DMG's offscreen plugin lacks an x86_64 slice.** Qt's macOS binaries are universal, and
  `macos-verify-bundle.py` already resolves both slices of every Mach-O in the bundle, so the
  translated run would have failed there first. The probe is the proof.
- **AppImage runtime ignores `APPIMAGE_EXTRACT_AND_RUN`.** Both AppImage runtimes honor it;
  if the probe shows otherwise, the fallback is `--appimage-extract` to a temp dir plus
  `AppRun`, which the smoke already does and which still launches the uploaded bytes.
- **The installed executable cannot find a platform plugin.** Covered by the plugin-path
  tradeoff; the step prints stderr, so "could not find the Qt platform plugin" is readable.
- **Uninstall leaves the directory behind** (a log, a settings file written next to the exe).
  R4 wants exactly that failure surfaced; the listing says what was left.
- **Silent-breakage classes** (`common-mistakes.md`): the CI workflow is an isolated surface,
  so each `ci.yml` commit carries only the workflow, the shape test and `doc/`
  (`scripts/commit-shape.json`); a `fix(ci)` subject must cite the run it answers. The
  2026-06-01 lesson (a slice nobody launched) is this spec's reason to exist; the mitigation
  is that from now on nothing is uploaded unlaunched.
- **Stale comment, adjacent, not in this lane:** `ci.yml` line ~2233 says `SS_INAPP_TESTS`
  exposes `--benchmark-hotpath`; the benchmark options are registered unconditionally
  (`CLI.cpp`). Named here for the maintainer; not edited by this spec.

## Test & verification plan

- **AC1 (R1–R4, R6):** the hardening run on master, every proof step green; URL in `spec.md`.
- **AC2 (R5, R7):** `python -m pytest scripts/tests/test_ci_workflow.py -q`; mutation check
  by hand before commit: drop one step, add `continue-on-error`, move a step after its upload,
  each must fail the suite.
- **AC3 (R3):** the `build-macos` log group for the translated run shows `uname -m` as
  `x86_64` and the suite summary.
- **AC4 (R4):** the `build-windows` log shows the executable path after install and the
  "directory is gone" line after uninstall.
- **AC5 (R8):** per-leg step durations read from the probe run's job pages, recorded in
  `spec.md`.
- **AC6 (R9):** the first run after the hardening commit, no `continue-on-error` on any proof
  step (the shape test pins it).
- **AC7:** `ship` counted by the umbrella plan's predicate by hand (four legs, four steps)
  until C0's `quality-score.py --check` reads it; 0104's AC6 ticked with `0107`.
- **Unit (you can run):** the shape test above. **Integration / hotpath:** none; no C++.
- **Static:** `python scripts/sanitize-commit.py` before each commit;
  `python scripts/commit-shape.py --range HEAD~1..HEAD` on each of the two workflow commits.
