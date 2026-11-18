---
spec: 0126-launch-defines
phase: tasks
updated: 2026-11-18
---

# Tasks 0126 — Launch defines

> **The ordered checklist.** Decomposed from the Design in [`spec.md`](./spec.md).

## Tasks

### T1 — Holder for the launch defines

- **Files:** `core/Core/Runtime.h`, `core/Core/Runtime.cpp`
- **Does:** `defines()` and `setDefines()`; written once on the main thread before the event
  loop, read on the main thread.
- **Verify:** syntax check, `code-verify --check`.
- **Deps:** none
- [x] done

### T2 — Validation of one define

- **Files:** `app/src/Misc/CLI/CliSpecParsers.h`, `.cpp`, `app/tests/tst_cli_spec_parsers.cpp`
- **Does:** `parseDefine` with the key, value and length rules of R1, and its test rows.
- **Verify:** syntax check (test file through its own compile entry), `code-verify --check`.
- **Deps:** none
- [x] done

### T3 — The option

- **Files:** `app/src/Misc/CLI.h`, `app/src/Misc/CLI.cpp`
- **Does:** `--define` in the options every build has; one bad define writes one line to
  standard error and ends the process; the rest are stored. `CLI.cpp` stays under 1,500 lines.
- **Verify:** syntax check, `code-verify --check`, line count.
- **Deps:** T1, T2
- [x] done

### T4 — The two commands

- **Files:** `core/Api/API/Handlers/SystemHandler.h`, `.cpp`
- **Does:** `system.getDefine`, `system.listDefines`; no package refusal, no prompt.
- **Verify:** syntax check, `code-verify --check`, singleton census unchanged.
- **Deps:** T1
- [x] done

### T5 — Command tables and generated surfaces

- **Files:** `core/Api/API/CommandCapabilities.cpp`, `core/Api/API/SealedPolicy.cpp`,
  `app/rcc/ai/command_safety.json`, `core/Api/API/CommandSafetyTable.cpp`,
  `app/rcc/api/api-schema.json`, the SDK files, the proto files
- **Does:** classifies both commands as plain, safe and allowed in a package; schema entries
  by hand; generators re-run.
- **Verify:** each generator's `--check`; tables stay sorted.
- **Deps:** T4
- [x] done

### T6 — Integration test

- **Files:** `tests/integration/test_launch_defines.py`
- **Does:** exit codes for bad defines against the binary; the two commands against a live app.
- **Verify:** `python -m py_compile`; the maintainer runs it.
- **Deps:** T3, T4
- [x] done

### T7 — Documentation

- **Files:** `doc/help/Command-Line-Interface.md`, `doc/help/API-Reference.md`,
  `doc/help/Control-Loop.md`, `doc/claude/TODO.md`
- **Does:** documents the option, the two commands and the "not for secrets" rule.
- **Verify:** `documentation-verify.py`, `sanitize-commit.py`.
- **Deps:** T5
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `spec.md`).
- [ ] `python scripts/sanitize-commit.py` run, only if a generator input changed.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
