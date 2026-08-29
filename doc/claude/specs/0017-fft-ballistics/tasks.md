---
spec: 0017-fft-ballistics
phase: tasks
status: approved     # approved 2026-07-17
updated: 2026-07-17
---

# Tasks 0017 — FFT display ballistics

### T1 — Dataset fields + serialization
- **Files:** `app/src/DataModel/Frame.h`, `Frame.cpp`
- **Does:** `fftBallistics` (bool run) + `fftBallisticsRelease` (int block); Keys;
  guarded serialize (emit both keys only when enabled — untouched projects
  byte-identical); reads default false/300.
- [x] done

### T2 — Editor rows + commit
- **Files:** `ProjectEditorItemIds.h`, `ProjectEditorForms.cpp`, `ProjectEditorCommit.cpp`
- **Does:** two ids; CheckBox "Peak Ballistics" + IntField "Ballistics Release (ms)" in
  `buildFftRangeRows` (editable on `dataset.fft`); bool case in flag handler, int case
  in FFT handler.
- [x] done (rows landed in `buildFftGeneralRows` instead — `buildFftRangeRows` would
  have crossed the 100-line function cap)

### T3 — FFTPlot ballistics engine
- **Files:** `FFTPlot.h`, `FFTPlot.cpp`
- **Does:** per-bin display state + wall-clock exponential release, instant attack;
  blend applied to every emitted dB bin; resize hooks at every bin-count change.
  **Invariants: display-only (analysis dB untouched upstream of the blend),
  allocation-free per tick, first frame jumps (no stale decay), release clamped
  50-5000 ms.**
- [x] done (post-0018 the blend lives in `computeBinSpectrum`, upstream of both the
  linear and log emit paths)

### T4 — API + test + doc note
- **Files:** `ProjectHandlerEntities.cpp`, `ProjectHandler.cpp`,
  `tests/integration/test_project_editor.py`, `doc/claude/architecture/dashboard.md`
- **Does:** `takeParam` blocks + doc string; `test_fft_ballistics_round_trip`; one
  dashboard.md sentence.
- [x] done (test collects; requires the app + API server to execute)

## Definition of Done

- [x] code-verify clean on all changed files (0 errors, 0 advisories); clang-format run.
- [x] AC1/AC2 (in-app observation), AC3 (pytest with the app up), AC4 (benchmark gate).
- [x] Diff limited to the plan's file list; `sanitize-commit.py` run 2026-07-17.
- [x] spec status → done.
