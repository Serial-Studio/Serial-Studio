---
spec: 0042-license-token-hardening
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-28
---

# Tasks 0042 — CommercialToken lifecycle hardening

> **Phase 3 of 4 — the ordered checklist.** `/ss-implement` works this list top to bottom.
> Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** = how this unit is confirmed before moving on.
- **Deps** = task IDs that must land first.

## Tasks

### T1 — R1 revalidation gate (landed as approved hotfix)

- **Files:** `app/src/Misc/ModuleManager.cpp`
- **Does:** Startup revalidation gates on `lemonSqueezy.canActivate()` instead of
  `!licensingData().isEmpty()`, so a failed cached restore still triggers the online
  verdict. Already applied with maintainer approval.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`;
  AC1 runtime check by maintainer.
- **Deps:** none
- [x] done

### T2 — R2 move licensing block first

- **Files:** `app/src/Misc/ModuleManager.cpp`
- **Does:** Move the `#ifdef BUILD_COMMERCIAL` block (`MachineID`, `LemonSqueezy`,
  `OfflineLicense`, `Trial`) from after `adoptAppState` to directly after
  `(void)Misc::Translator::instance()` in `instantiateCoreModules()`; update the
  function's `@brief`. Binding invariant: composition-root order is a protected surface —
  licensing ctors must reach nothing constructed later (verified: MachineID / SimpleCrypt /
  QSettings / QNAM / qApp / each other only); `SessionContext::shutdown()` order is
  untouched because none of the four are context-adopted modules.
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`;
  read-back: licensing precedes ProjectModel adoption; shutdown() untouched.
- **Deps:** none
- [x] done

### T3 — R2 ctor-edge proof note

- **Files:** `doc/claude/specs/0001-composition-root/` (append note file or extend the
  existing proof doc)
- **Does:** Record the re-run edge audit for the new order: per licensing ctor, the list
  of reached modules and the conclusion that none is constructed later. Dated, referencing
  spec 0042.
- **Verify:** Read-back; proof names all four ctors.
- **Deps:** T2
- [x] done

### T4 — R3 consumer inventory

- **Files:** `doc/claude/specs/0042-license-token-hardening/consumers.md`
- **Does:** Table of every TU from `grep -rl "CommercialToken::current()" app/src`:
  classification (sample-per-op / bakes-state), and for bakes-state the exact
  `activatedChanged` wire site (file:line). Flag any unwired bakes-state consumer.
- **Verify:** Re-run the grep; every hit appears in the table.
- **Deps:** none
- [x] done

### T5 — R3 wire any gap found

- **Files:** whichever TU(s) T4 flags (expected zero to two; named in chat before edit)
- **Does:** Add the missing `LemonSqueezy::activatedChanged` connection so the consumer
  re-derives its gated state. Skip entirely if T4 finds no gap.
- **Verify:** `python scripts/code-verify.py --check <files>`; consumers.md updated.
- **Deps:** T4
- [x] done

### T6 — R4 entitlement problem checker

- **Files:** checker TU per ProblemCenter registration pattern (locate exact home first;
  expected `app/src/Misc/` alongside the diagnostics checkers) + its registration site
- **Does:** Synchronous checker: for each `ProjectModel::sources()` entry whose bus is
  commercial-gated, when `CommercialToken::current().isValid()` is false, report
  "This data source requires Serial Studio Pro or an active trial." Binding invariants:
  checkers return synchronously, never touch driver instances or config, nothing per
  frame (spec 0035); finding clears on the next poll once the token is valid.
- **Verify:** `python scripts/code-verify.py --check <files>`; read-back against the
  spec-0035 rules; AC2 runtime check by maintainer.
- **Deps:** T2
- [x] done

### T7 — R5 remove diagnostics

- **Files:** `app/src/IO/ConnectionManager.cpp`, `app/src/IO/Drivers/MQTT.cpp`
- **Does:** Remove the five `[mqtt-debug]` blocks (restore original bodies exactly) and
  the diagnostic `qDebug` in the driver's `configurationOk()` (agreed cleanup of the
  bug-hunt probe).
- **Verify:** `grep -rn "mqtt-debug" app/` empty; `python scripts/code-verify.py --check`
  both files; git diff shows only diagnostic-line removal in those hunks.
- **Deps:** T2 (avoid edit collisions), T6
- [x] done

### T8 — CLAUDE.md startup note + self-review

- **Files:** `CLAUDE.md` (+ read-only diff pass over all touched files)
- **Does:** One-line update: licensing block is now first (after Translator) in
  `instantiateCoreModules()`; keep the late-token history note. Then the counterfactual
  self-check: rule most at risk = composition-root reorder without proof — evidence = T3
  note + unchanged shutdown(); second risk = checker violating the pull rule — evidence =
  T6 read-back.
- **Verify:** `python scripts/code-verify.py --check` on all touched; findings named in
  chat.
- **Deps:** T2-T7
- [x] done

## Definition of Done

- [ ] AC1/AC2/AC4/AC5 handed to maintainer with their concrete runtime checks; AC3
      (consumers.md) and AC6 (no mqtt-debug) verified in-repo.
- [ ] `python scripts/code-verify.py --check` clean on all changed files.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] Hotpath untouched; CI gate confirms.
- [ ] `sanitize-commit.py` at commit time (tree carries unrelated uncommitted campaigns).
- [ ] Diff is what was asked, and only that.
- [ ] `spec.md` status set to `done` after maintainer ACs pass.
