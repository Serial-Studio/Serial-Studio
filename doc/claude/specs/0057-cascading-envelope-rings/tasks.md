---
spec: 0057-cascading-envelope-rings
phase: tasks
status: approved     # gate auto-approved, overnight run 2026-08-16 (unattended; maintainer re-reviews)
updated: 2026-08-16
---

# Tasks 0057 — Cascading envelope rings for plot history

> **Phase 3 of 4 — the ordered checklist.** Gate auto-approved for the unattended overnight run
> of 2026-08-16.

## Conventions

- One task = one focused, reviewable change.
- **Verify** = `python3 scripts/code-verify.py --check <files>` plus a read-back; the ctest
  suite cannot be built tonight and is left correct-by-reading.

## Tasks

### T1 — `TimeRing::opensCell()` predicate

- **Files:** `app/src/DSP.h`
- **Does:** Adds the one-line predicate `appendDecimated` already evaluates inline and uses it
  there, so the pyramid and level 0 agree on "this sample opens a new cell" by construction.
- **Verify:** code-verify; `appendDecimated` behaviour unchanged by reading.
- **Deps:** none
- [x] done

### T2 — `EnvelopeCell` / `EnvelopeLevel` / `EnvelopeRing`

- **Files:** `app/src/DSP.h`
- **Does:** The pyramid struct: construction sizes the coarse levels from level 0's capacity;
  `append` sanitises, folds the completed level-0 cell into every coarser open cell, then
  delegates to `appendDecimated`; `clear`, `resizeCapacity` (rebuilds levels from level 0),
  `levelSpanSec`, `selectLevel`.
- **Verify:** code-verify; arithmetic cross-checked against the phase-0 sim (1.067x, wrap
  consistent).
- **Deps:** T1
- [x] done

### T3 — Envelope-aware `downsampleTimeWindow`

- **Files:** `app/src/DSP.h`
- **Does:** Extracts the body of `downsampleTimeWindow` after its accessors into
  `dsTimeWindowCore(n, xAbs, yAt, newest, ...)`; the existing overload calls it; a new overload
  taking `const EnvelopeRing&` selects the level and feeds either level 0's queues or the coarse
  cells' `2 * n` extreme points, rebased to level 0's newest sample.
- **Verify:** code-verify; existing overload's output identical by reading (same statements,
  same order).
- **Deps:** T2
- [x] done

### T4 — Dashboard owns `EnvelopeRing`s

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** Ring maps, getters, snapshot/restore/replay, `growTimeRing`, `makeHistoryRing`,
  every history-ring `appendDecimated` -> `append`. Sweep rings untouched.
- **Verify:** `grep -n appendDecimated Dashboard.cpp` shows only sweep-engine sites; code-verify.
- **Deps:** T2
- [x] done

### T5 — Widget and API readers

- **Files:** `app/src/UI/Widgets/Plot.cpp`, `app/src/UI/Widgets/MultiPlot.cpp`,
  `app/src/API/Handlers/DashboardHandler.cpp`
- **Does:** Time-axis draws call the ring overload; `tailFrames` reads `level0`.
- **Verify:** `grep -rn "plotTimeRing\|multiplotTimeRings" app/src` — every site updated;
  code-verify.
- **Deps:** T3, T4
- [x] done

### T6 — ctest suite

- **Files:** `app/tests/tst_envelope_ring.cpp`, `app/tests/CMakeLists.txt`
- **Does:** AC1-AC4 as Qt Test slots; registered with the AppPlatform link set DSP.h needs.
- **Verify:** code-verify; correct-by-reading (not built tonight).
- **Deps:** T2, T3
- [x] done

### T7 — Docs

- **Files:** `doc/claude/architecture/dashboard.md`
- **Does:** The TimeRing paragraph names the pyramid, the fold rule, the selection rule and the
  memory bound.
- **Verify:** read-back.
- **Deps:** T5
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC6 need the
      maintainer's build; AC7 done).
- [x] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (maintainer, morning).
- [x] `--benchmark-hotpath` not regressed (maintainer).
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done` (after the maintainer's ACs).
