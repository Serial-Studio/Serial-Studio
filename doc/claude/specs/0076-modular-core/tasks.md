---
spec: 0076-modular-core
phase: tasks
status: approved
updated: 2026-09-03
---

# Tasks 0076 — Modular core

> Owners: **M** = Fable (manager, shared files, review); **A/B/C/D** = executor agents with
> disjoint file ownership. No agent commits. Shared files (`app/CMakeLists.txt`,
> `app/tests/CMakeLists.txt`, `CLAUDE.md`) have exactly one owner per wave.

## Wave 0 — moves (M)

### T1 — Create `core/` layout and `git mv` the 39 files
- **Files:** per plan.md tables.
- **Verify:** `git status` shows renames only; old paths gone.
- [x] done

### T2 — Rewrite include lines across `app/` and `core/`
- **Does:** exact-string rewrite of the 20 known include forms to the `Core/…` / `Protocols/…`
  paths; no other content changes.
- **Verify:** grep finds zero stale forms; every moved file's `git diff -M` shows only
  `#include` lines.
- [x] done

## Wave 1 — parallel

### T3 — CMake: library targets + executable + tests (A, Opus)
- **Files:** `core/CMakeLists.txt`, `core/Core/CMakeLists.txt`, `core/Protocols/CMakeLists.txt`,
  `app/CMakeLists.txt`, `app/tests/CMakeLists.txt`, `REUSE.toml`.
- **Verify:** `python scripts/layer-verify.py` checks 3, 4, 5 clean; grep shows no test
  registration listing a moved `.cpp`; every former `SSAssert.cpp` consumer links a library.
- **Deps:** T1, T2
- [x] done

### T4 — `scripts/layer-verify.py` + CI wiring (B, Opus)
- **Files:** `scripts/layer-verify.py`, `.github/workflows/ci.yml` (lint job only),
  `doc/claude/scripts.md` (one row).
- **Verify:** script runs clean on the tree after T3; `--json` output; exit codes.
- **Deps:** T1, T2
- [x] done

### T5 — Lint/tooling roots (C, Sonnet)
- **Files:** `scripts/code-verify.py`, `scripts/claim-verify.py`,
  `app/translations/translation_manager.py`.
- **Verify:** `code-verify.py --check` walks `core/` and applies first-party rules there;
  `claim-verify.py` indexes `core/`; `translation_manager.py` lists `core/` sources.
- **Deps:** T1
- [x] done

### T6 — AI-facing docs (D, Sonnet)
- **Files:** `CLAUDE.md`, `doc/claude/directory-map.md`, `doc/claude/architecture/io.md`,
  `dataflow.md`, `kernels.md`, `architecture.md`, `tests/README.md`,
  `.claude/skills/ss-hotpath/SKILL.md`, `ss-new-driver/SKILL.md`, `qt-cpp-review/SKILL.md` +
  `references/serial-studio-rules.md`, `ss-docs/references/ground-truth-factcheck.md`.
- **Verify:** `claim-verify.py` resolves every cited path; `documentation-verify.py` clean.
- **Deps:** T1
- [x] done

## Wave 2 — integrate & review (M + reviewers)

### T7 — Run every static gate; fix loop
- **Verify:** all commands in plan.md "Static" green.
- **Deps:** T3-T6
- [x] done

### T8 — Independent review of CMake and the gate script (reviewer agents)
- **Verify:** findings triaged; blockers fixed.
- **Deps:** T7
- [x] done

### T9 — Re-seed censuses / claim baseline for moved keys; `sanitize-commit.py`
- **Deps:** T8
- [x] done

### T10 — `handoff.md` (morning checklist + triage), memory note, CLAUDE.md pointer
- **Deps:** T9
- [x] done

## Definition of Done

- [ ] AC1-AC5 in `spec.md` checked; AC6/AC7 left for the maintainer with exact commands.
- [ ] `layer-verify.py`, `code-verify.py --check`, censuses, `claim-verify.py`,
  `registry-verify.py`, `documentation-verify.py` all clean.
- [ ] `git diff -M` shows every moved file as a rename with include-only edits.
- [ ] Diff is what was asked and only that; nothing committed.

## Wave 3 — stage 2 partition + bus (2026-09-04)

### T11 — `git mv` the five subsystem trees under `core/<Layer>/` (M) — [x] done
### T12 — tests CMake `${SS_CORE_SRC}` repoint, 283 entries (M) — [x] done
### T13 — executable-scoped settings audit (F, Opus, read-only) — [x] done
### T14 — `core/<Layer>/CMakeLists.txt` ×5, `core/CMakeLists.txt`, `app/CMakeLists.txt` (A2, Opus) — [x] done
### T15 — `layer-verify.py` seven layers + per-edge ratchet + `pair-split` + `include-ambiguous` (B2, Opus) — [x] done
### T16 — tooling constants repoint (C2, Sonnet) — [x] done
### T17 — docs/skills repoint + CLAUDE.md contract row (D2, Sonnet) — [x] done
### T18 — `Core::Bus::MessageBus` + `Messages.h` + `tst_message_bus.cpp` (E, Opus) — [x] done
### T19 — `(file, condition)` conservation check; full gate run; reviewers; fixes (M) — [x] done
### T20 — handoff.md stage-2 section, migration table from the census, memory (M) — [x] done
