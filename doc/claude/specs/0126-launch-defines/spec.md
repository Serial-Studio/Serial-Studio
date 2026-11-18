---
spec: 0126-launch-defines
title: Launch defines (--define KEY=VALUE, read from scripts)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-11-18
author: Alex Spataru
---

# Spec 0126 — Launch defines

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

## Problem / Motivation

An external launcher starts Serial Studio on a fixed project in runtime or kiosk mode. The
project's control script talks to a test-cell controller, and it now needs one value per session
that is not in the project file: which engine the operator registered for this test. The
controller refuses to arm until the script has sent that value, so it must reach the script
reliably, and only the value given on this launch.

Today the only way is to edit the project file before every launch, which changes a file that is
meant to be fixed and leaves the last session's value behind for the next one.

## Goals

- A launcher passes named text values on the command line, and a script reads them by name.
- A value given on one launch is never seen by a later launch that did not give it.
- A malformed value stops the launch with a non-zero exit code; it is never dropped silently.
- A launcher can tell from `--help` whether a given Serial Studio supports the option.

## Non-Goals

- No way to set or change a define after the process has started, from a script or the API.
- No typed values: every value is text and the script converts it.
- No define stored in a project, a deployment package, a shortcut or the settings.
- No substitution of defines into project fields (titles, addresses, parser code).
- No environment-variable form.

## Requirements

1. **R1** — `--define KEY=VALUE` may be given any number of times. The key is 1 to 64
   characters of `A-Z a-z 0-9 _ . -`. The value is everything after the first `=`, taken
   literally: it may be empty, may contain `=`, and is at most 256 characters (Unicode code
   points).
2. **R2** — A define with no `=`, an empty or invalid key, or a value over the limit ends the
   process before any window opens, with one line on standard error naming the offending define
   and a non-zero exit code.
3. **R3** — The same key given twice: the last one wins.
4. **R4** — Defines last for the life of the process and are held in memory only. Nothing
   writes them to the project file, the settings or any other file.
5. **R5** — `system.getDefine` with `{"key": "<name>"}` answers
   `{"key": "<name>", "defined": true|false, "value": "<text>"}`. An unknown key is not an
   error: `defined` is false and `value` is empty. A missing or non-text `key` is the usual
   missing-parameter error.
6. **R6** — `system.listDefines` answers `{"defines": {"<key>": "<value>", ...}}`, with an
   empty object when none were given.
7. **R7** — Both commands are read-only, never raise a consent or permission prompt, and work
   in every mode: normal, `--runtime`, `--kiosk`, `--headless` and a deployment package.
8. **R8** — Both commands answer from the first line of a control script, before any frame has
   arrived, and from every other place a script may call the API.
9. **R9** — `--define` is available in every build and appears verbatim in `--help`.
10. **R10** — Without `--define` nothing changes, and `system.getDefine` answers
    `defined: false` for every key.

## Acceptance Criteria

- [ ] **AC1** — `--help` lists `--define`. (Maintainer, built binary.)
- [ ] **AC2** — `--define engine=2`: a control script calling `system.getDefine` for `engine`
  gets `defined: true, value: "2"`; for `other` it gets `defined: false, value: ""`.
  (`pytest`, live app.)
- [ ] **AC3** — `--define a=1 --define a=2` gives `"2"`; `--define "x=a=b"` gives `"a=b"`;
  `--define empty=` gives `defined: true, value: ""`. (Unit test of the parser; `pytest`.)
- [ ] **AC4** — `--define novalue`, `--define "bad key=1"` and a 300-character value each end
  the process with one line on standard error and a non-zero exit code. (Unit test of the
  parser; `pytest` against the binary.)
- [ ] **AC5** — Start with `--define engine=2`, quit, start again without it: `engine` is
  undefined, and the project file and the settings are byte-identical before and after both
  runs. (Maintainer.)
- [ ] **AC6** — With `--runtime --kiosk` the call returns the value and no dialog appears.
  (Maintainer, test cell.)
- [ ] **AC7** — The call works in the control script's top-level code, before any frame has
  arrived. (Maintainer, test cell.)
- [ ] **AC8** — On Windows, `--define "note=two words"` gives `"two words"`. (Maintainer.)
- [ ] **AC9** — The command tables agree: every classification test and generator check passes
  with the two new commands. (CI.)

## Constraints & Invariants

- The deciding constraint: a script's call must never prompt and never depend on the mode, or
  the controller cannot be armed from a locked kiosk.
- Defines are launch facts, not project content: no project load, save or package may carry
  them.
- No new top-level API namespace: each one becomes a global name in every script engine and can
  collide with a variable an existing script declares.
- No growth in the singleton census, and no file over the 1,500-line cap (the command-line
  source is at 1,392).
- Not on the acquisition pipeline.

## Premise Check

- **Question:** can a script call a plain read-only command with no prompt in runtime, kiosk
  and package sessions, and is the value in place before the first script line runs?
- **Measured:** by reading the dispatch path, not by a run. Script calls go through one policy
  that refuses a fixed list and never prompts; the only script-side consent is the helper-process
  launcher. `system.projectDir` is the working precedent: plain, allowed in a package, called
  from control scripts today. The command line is processed before the composition root is
  built, so the values exist before any script engine does.
- **Verdict:** holds. Unmeasured: whether a Windows build with no console delivers the error
  line to a launcher reading a pipe; the exit code is the signal a launcher can rely on.

## Design

### Approach

The values live in the existing process-wide launch-facts holder, filled once by the
command-line code after parsing and validation, and read by two new commands added to the
existing `system.*` handler. Validation is one pure function beside the other command-line
value parsers, so it is unit-tested without the application. The two commands are classified
like `system.projectDir` (plain, safe, allowed in a package), minus its package refusal, and are
added to every command table and generated surface.

Candidates considered:

- **Launch facts in `Core::Runtime` + `system.*` (chosen).** No new file, no new singleton, no
  line in the composition root.
- **New `app.*` namespace, as the contract writes it.** Rejected: a new top-level namespace
  becomes a global `app` in every script engine, and the main QML root is already called `app`
  in this code base, which invites confusion. The contract allows following the convention.
- **Inject defines as script globals (`DEFINES.engine`).** Breaks the assumption that scripts
  only see the API: touches every script engine, and a missing value fails as a script error.
- **Smallest version: `system.getDefine` only**, about 40 lines fewer; drops R6 (the
  diagnostic list).

### Affected files

| File | Change | ± lines |
|------|--------|---------|
| `core/Core/Runtime.h`, `Runtime.cpp` | `defines()`, `setDefines()` and their storage | +30 |
| `app/src/Misc/CLI/CliSpecParsers.h`, `.cpp` | `parseDefine(text, key, value, error)` | +50 |
| `app/src/Misc/CLI.h`, `CLI.cpp` | the option; collect, validate, fail or store | +40 |
| `core/Api/API/Handlers/SystemHandler.h`, `.cpp` | `getDefine`, `listDefines`, registration | +70 |
| `core/Api/API/CommandCapabilities.cpp` | two plain rows | +2 |
| `core/Api/API/SealedPolicy.cpp` | two allowed rows | +2 |
| `app/rcc/ai/command_safety.json` | two `safe` names | +2 |
| `core/Api/API/CommandSafetyTable.cpp` | regenerated | +2 |
| `app/rcc/api/api-schema.json` | two entries, added by hand | +25 |
| `app/rcc/api/SerialStudio.js`, `SerialStudio.lua`, `sdk-symbols.json` | regenerated | +45 |
| `app/rcc/api/proto-fields.json`, `doc/grpc/serialstudio-typed.proto` | regenerated | +25 |
| `app/rcc/ai/search_index.json` | regenerated | ±0 |
| `app/tests/tst_cli_spec_parsers.cpp` | `parseDefine` rows | +60 |
| `tests/integration/test_launch_defines.py` | exit codes and the two commands | +90 |
| `doc/help/Command-Line-Interface.md` | option row and a short section | +25 |
| `doc/help/API-Reference.md`, `doc/help/Control-Loop.md` | the two commands | +30 |
| `doc/claude/TODO.md` | maintainer checks | +4 |
| **Total** | code 196 / generated 99 / tests 150 / docs 59 | **+504** |

### Hotpath & threading impact

- **Touches the hotpath?** No.
- **New cross-thread signal/slot?** No. The values are written once on the main thread before
  the event loop starts; command handlers run on the main thread, and scripts on other threads
  already reach them through the existing blocking hop.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged.

### Data model, API and UI

- No project-file key and no UI.
- `system.getDefine {key: string}` → `{key, defined, value}`; `system.listDefines {}` →
  `{defines: {...}}`. Both: no capability, tier `safe`, allowed in a sealed session, open to
  remote API clients like any other read-only command.
- The parameter is named `key`; the path-policy test exempts that name.
- `--define` sits in the options every build has.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Command names | `app.*`; `system.*` | `system.*`. No new global in script engines; the contract permits it. |
| Who may read | every API client; control script only | Every client. Read-only launch facts, five fewer tables to edit; the launcher must not pass secrets as defines, and the docs say so. |
| Lifetime | life of the process; cleared when another project opens | Life of the process. Simplest correct rule; a runtime or kiosk session cannot open another project anyway. |
| Bad define | exit non-zero; warn and continue like other bad values | Exit. The controller arms on this value, so a dropped define must not start a session. |
| Value length unit | UTF-16 units; code points | Code points, what a person counts as characters. |
| Error output | Qt logging; direct write to standard error | Direct write, one line. Qt's default handler sends a console-less Windows process's messages to the debugger, not to the pipe. |
| Package sessions | allow; refuse like `system.projectDir` | Allow. A define reveals nothing about the package. |

### Risks

- A command missing from one table is not caught by comparing the live registry (nothing
  does); the committed schema file is hand-edited here, so it is re-dumped by the maintainer
  after the first build to confirm it matches.
- Standard error on a Windows build without a console is unverified.
- A launcher that passes a secret as a define exposes it to any API client the operator allowed.

### Tests

- AC3, AC4: `tst_cli_spec_parsers` rows (valid, `=` in value, empty value, no `=`, empty key,
  bad key character, 64 and 65 character keys, 256 and 257 code-point values).
- AC2, AC3, AC4: `tests/integration/test_launch_defines.py` (needs the built binary).
- AC9: the capability, sealed-policy, safety-table and path-policy tests, and the generators'
  `--check` runs, in CI.
- AC1, AC5 to AC8: maintainer.

## Deviations from the launcher's contract

1. The commands are `system.getDefine` and `system.listDefines`, not `app.*`. Result fields are
   as written: `key`, `defined`, `value`, and `defines`.
2. Defines last for the life of the process, also when another project is opened in it.
3. A call with no `key` is a missing-parameter error, not `defined: false`.
4. The value limit counts Unicode code points.
5. Any API client can read the defines, not only the control script.
6. An in-app restart (the one a settings change offers) relaunches with the same command line,
   so the restarted process has the same defines. Shortcuts are built from their own options
   and never carry a define.

## Open Questions

- Command names: `system.*` as recommended, or `app.*` as the contract writes them?
- Readable by every API client as recommended, or by the control script only?
