---
spec: 0123-package-resources
title: Deployment package resources and helper programs
status: in-progress  # draft -> approved -> in-progress -> done | shelved (approved 2026-11-14, re-approved at about 1,520 lines)
created: 2026-11-14
author: Alex Spataru
---

# Spec 0123 — Deployment package resources and helper programs

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

## Problem / Motivation

The APU Test Cell simulator project starts a Python helper from its control script when the
operator connects: it asks for the project's folder, then starts `python3 Simulator.py` there.
Opened from the project file this works. Exported as a deployment package it does nothing, and
the operator gets no explanation. Three things stop it. A package session refuses both commands
the script uses, a package has no folder on disk, and the package does not hold the helper file.

Spec 0096 chose this on purpose: a package carried no programs and started none. That leaves a
project with a helper (a simulator, a bridge, a logger) with only the shortcut deployment, which
ships the readable project. This spec is not part of 4.2.2.

## Goals

- A project whose control script starts a helper works from a package without changing the script.
- The author can put helper files and folders inside the package.
- The author can instead leave the helper on the target machine, next to the package.
- The operator is asked once per package before it starts any program, and never again for the
  same package.
- The project itself stays as sealed as it is today.
- A package whose author did not ask for helper programs behaves exactly as it does today.

## Non-Goals

- Carrying the interpreter or runtime (Python, Node). The target machine provides it.
- Keeping a carried file's executable permission. A carried helper runs through an interpreter.
- Hiding resources from the operator. A resource is a plain file on disk while the package runs.
- Resources from outside the project's folder.
- Changing what the Process driver does in a package session.
- Starting a package's programs without the operator's consent on an unattended machine, beyond
  the existing automatic-consent setting.

## Requirements

**Export**

0. **R0:** The Deploy dialog has an option, off by default, that lets the package start helper
   programs (maintainer decision, 2026-11-14). With it off, the Resources list is unavailable,
   nothing is copied to disk, and the package session refuses the four commands as it does
   today. R1 to R12 apply only with it on.
1. **R1:** The Deploy dialog has a Resources list. The author adds files or folders from the
   project's folder; each entry shows its path and size and can be removed.
2. **R2:** The dialog says, next to the list, that resources are copied to the operator's disk
   while the package runs and can be read there.
3. **R3:** Resources count toward the package's existing size and file-count limits; an export
   over a limit is refused with the reason.

**Opening**

4. **R4:** Opening a package with resources copies them to a private folder for that session,
   keeping each one's path relative to the project's folder. Nothing else from the package is
   copied to disk.
5. **R5:** The folder is removed when the session ends, after the session's helper programs
   have stopped. A folder left by a crashed session is removed the next time any package opens.
6. **R6:** A resource that cannot be written stops the package from opening, with a message
   that names the reason.

**Scripts**

7. **R7:** In a package session, the project-folder command answers the project's own script
   with the resources folder when the package carries resources, and otherwise with the folder
   that holds the package file. It also always gives the package file's folder under its own
   name.
8. **R8:** In a package session, the project's own control script can start, list and stop
   helper programs with the same commands it uses in a project session. With no working folder
   given, a program starts in the folder R7 answers.
9. **R9:** A client connected over the network is refused all four commands (the project-folder
   command with the sealed-session answer, the other three as script-only commands), and cannot
   read the resources folder through any file-reading command.

**Consent**

10. **R10:** The first time a package tries to start a program, the operator is asked, and the
    question names the program and its arguments. Nothing starts before the answer.
11. **R11:** The answer is remembered for that exact package. A package whose contents differ
    in any way is a different package and asks again.
12. **R12:** No package starts granted: opening one from the command line does not skip the
    question.

## Acceptance Criteria

- [ ] **AC1:** (R1, R3) Unit test: a file and a folder of resources map to entries with their
  relative paths. The size limit rests on the export's existing check, which has no unit test.
- [ ] **AC2:** (R4, R6) Unit test: extraction writes exactly the resource entries, with their
  relative layout, into a folder only the user can read, and reports failure on an unwritable
  target.
- [ ] **AC3:** (R4) Integration test: the existing "no marker reaches disk" test still passes
  for a package that also carries a resource, and the resource is on disk.
- [ ] **AC4:** (R5) Unit test: a folder whose owning session is gone is removed by the sweep; a
  folder whose session is alive is kept.
- [ ] **AC5:** (R7, R8) Integration test: a package whose control script starts a carried
  script through the project-folder command runs it, and the script's output file appears in
  the resources folder.
- [ ] **AC6:** (R7) Covered by AC10: a package with no resources gets the package file's folder.
  No automated test reaches it, because only the project's script may ask.
- [ ] **AC7:** (R9) Integration test: the four commands are refused to a network client. Unit
  test: once sealed, no API path inside the resources folder is allowed.
- [ ] **AC8:** (R10, R11, R12) Unit test: consent bound to a package identity asks once, holds
  for the same identity, asks again for a different one, and is not granted by the
  command-line project rule.
- [ ] **AC11:** (R0) Integration test against the marker package (option off): no resources
  folder holds its files. The script-side refusal with the option off has no automated test.
- [ ] **AC9:** (R2) The maintainer reads the Resources section in the running Deploy dialog.
- [ ] **AC10:** The maintainer exports the APU simulator project with its package placed in the
  project's folder and sees the simulator start on connect after one question.

## Constraints & Invariants

- **The deciding constraint:** only what the author listed as a resource reaches disk. The
  project, its scripts, certificates, credentials and extensions stay in memory.
- A network client gains nothing: it can neither start a program in a package session nor read
  the resources back.
- Consent is asked at most once per package and never per command (maintainer rule,
  2026-11-11).
- A package's programs are killed before its folder is removed.
- Pro-only, like the rest of deployment packages; the trial has it.
- No change to the package container format or its limits.
- No hotpath change.

## Premise Check

- **Question:** does carrying `Simulator.py` alone make the motivating project work from a
  package?
- **Measured:** read `Simulator.py` on 2026-11-14. It resolves the repository root two folders
  above itself and imports six modules from `Scripts/` (4.2 MB) and
  `Firmware/Emulation/Engine/` (568 KB), plus PySide6 and Renode from the machine. Those folders
  are outside the project's folder.
- **Verdict:** carrying the one file does not start this simulator. On a machine with the
  repository installed, the package placed in the project's folder does (R7's second case). The
  carried-resources half serves projects whose helper is self-contained under the project
  folder; folders are in R1 because a lone file is rarely enough. Both halves stay in scope.

## Design

### Approach

Resources are a third class of carried file, stored under a `resources/` prefix beside the
existing `project/`, `external/` and `certs/` entries, so the container format is unchanged.
`DeploymentSession::open` writes those entries to a `QTemporaryDir`-style folder before it clears
the decoded file list, then passes the folder and the package's header digest to
`Core::Runtime::seal`. `system.projectDir`, `system.exec`, `system.kill` and
`system.runningProcesses` move to the sealed allow table; the three process commands are already
control-script only, and `system.projectDir` refuses a remote caller when sealed. All four refuse every caller unless
the package's `helperPrograms` option is on. The launch
consent binds to `ssdeploy:<digest>` in a sealed session, so its stored key is per package.

Candidates considered:

- **Beside only:** allow the four commands, answer the package's folder, carry nothing. About a
  third of the size; drops R1 to R6. Covers the APU case and nothing self-contained.
- **Carried resources** (chosen, with Beside as its no-resources case).
- **Mounted, not extracted:** run helpers from the in-memory mount. Breaks the assumption that
  an interpreter needs a real path; no interpreter can read a Qt resource, so it cannot work.

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `core/Core/Deploy/PackageFormat.h` | `resources/` prefix constant | +3 |
| `core/Core/Deploy/ResourceExtractor.h/.cpp` (new) | Write resource entries to a 0700 folder, lock file, stale sweep, removal | +190 |
| `core/Core/Runtime.h/.cpp` | `seal` takes the resources folder, package folder and package identity; three accessors | +35 |
| `app/src/Misc/DeploymentSession.h/.cpp` | Extract before the file list is cleared; refuse on failure; remove at exit | +45 |
| `core/Ui/Misc/DeploymentPackager.h/.cpp` | Resource list, add file or folder, carry under `resources/` | +90 |
| `app/qml/Dialogs/ShortcutGenerator.qml` | Resources section; reword the Canvas-images hint | +70 |
| `core/Api/API/SealedPolicy.cpp` | Move four commands from denied to allowed | 0 |
| `core/Api/API/Handlers/SystemHandler.cpp` | Sealed answer for `projectDir`, remote refusal, default working folder | +40 |
| `core/Api/API/ProcessLauncher.cpp` | Bind consent to the package identity when sealed | +12 |
| `core/Api/API/PathPolicy.cpp` | Refuse the resources folder when sealed | +6 |
| `app/tests/tst_package_resources.cpp` (new), `app/tests/CMakeLists.txt` | AC1, AC2, AC4 | +220 |
| `app/tests/tst_launch_consent.cpp` | AC8 | +45 |
| `tests/integration/test_sealed_session.py` | AC3, AC5, AC6, AC7 | +110 |
| `doc/help/Operator-Deployments.md` | Resources, helper programs, the consent question | +35 |
| `doc/claude/architecture/io.md` | "Sealed Sessions": what reaches disk, what runs | +8 |
| `doc/claude/specs/0096-deployment-packages/spec.md` | Note that this spec amends the programs non-goal | +2 |
| **Total** | code 491 / tests 375 / docs 45 | **+911** |

### Hotpath & threading impact

- **Touches the hotpath?** No. Extraction runs once, before the composition root.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership:** unchanged.

### Data model, API and UI

- One new deploy option, `helperPrograms` (boolean, default false), stored with the package's
  other options and read into the sealed runtime state at open.
- No new API command. `system.projectDir` gains a `packageDirectory` field in a sealed session;
  `directory` follows R7, `filePath` and `fileName` name the package file.
- No project-JSON change. The resource list lives in the Deploy dialog's state, like the
  existing extra files.
- The resources folder is `<temp>/SerialStudio-package-XXXXXX`, holding a lock file the sweep
  tests.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| What `directory` answers | Always resources folder; always package folder; resources folder if any, else package folder | The last: an unchanged script works in both deployments. Cost: adding the first resource changes where a beside-the-package helper is looked for. |
| Consent key | Package path; package digest | Digest: a file swapped in place cannot inherit a grant. Cost: every re-export asks once again. |
| Unattended machines | Pre-grant command-line packages; no pre-grant | No pre-grant: a package is a file anyone can craft. The automatic-consent setting remains for kiosks. |
| Default for the option | On; off | Off: every package exported before this spec, and every new one whose author does not ask, keeps the 0096 behavior. |
| Which carried files extract | All; only resources | Only resources, to keep the deciding constraint. |
| Executable permission | Preserve; drop | Drop: preserving needs a format change, and helpers run through an interpreter. |

### Risks

- **A crafted package runs a program after one click.** v1 packages are encoded, not encrypted,
  so anyone can build one. The consent question is the only gate; it must name the program and
  arguments, and nothing may pre-grant it (R12, AC8).
- **Read-back.** The temp folder is inside the API's default allowed paths; without the
  `PathPolicy` refusal a network client could read resources (AC7).
- **A helper beside the package is not checked.** Whoever can write that folder chooses what
  runs. Spec 0096 already excludes the machine's administrator; a package opened from a shared
  download folder is the case to document.
- **Leftover folders** after a crash hold resources until the next package opens.
- A new sealed exit: the resources folder is the first package content on disk. Certificates and
  extensions must not be reachable through the `resources/` prefix; the packager only writes that
  prefix from the author's list.

### Tests

- AC1, AC2, AC4: `tst_package_resources` (ctest).
- AC8: `tst_launch_consent` (ctest).
- AC3, AC5, AC6, AC7: `tests/integration/test_sealed_session.py` against a running package
  session.
- `tst_sealed_policy_registry` keeps every command in exactly one table.
- AC9, AC10: maintainer, in the running app.

### Changes during implementation (2026-11-14)

- The option (R0) was added at approval, at the maintainer's request.
- `core/Core/CMakeLists.txt` registers the new source, and
  `tests/fixtures/deploy/helper.ssproj` and `helper.py` are new fixtures; none was in the table.
- `RemoteDispatchScope::gated()` was added (`core/Api/API/CommandRegistry.h/.cpp`, a read-only
  query): a script's dispatch also opens a scope, so "a scope is active" did not mean "remote",
  and the project's own script would have been refused the project-folder command.
- From review: a write the disk did not take fails the open; the sweep runs for every package,
  touches only this user's folders and re-checks them before removal; entry names that are
  Windows device names or end in a dot or space are refused.
- **Size:** the change came to about 1,520 net lines against the approved 911 (code 700, tests
  and fixtures 690, docs 130), over the 1.5 times limit, so the spec is back to `draft` for
  re-confirmation. The overrun is the option, the review hardening and tests that are twice the
  estimate. The macOS launch fix below is a separate request of about 75 lines.
- macOS double-click: the launch file is now caught before any window is built and the package
  opens in the same process (`main.cpp`, `CLI`, `FileOpenEventFilter`, `AppPlatform.h`).

## Open Questions

- Should `directory` follow the chosen rule, or always be the package file's folder with the
  resources folder under a second name? The second keeps one meaning but needs the script to
  change for carried helpers.
- Does the Process driver start its program in a package session today? Not checked; it shares
  the launch consent, so the per-package key would apply to it too.
