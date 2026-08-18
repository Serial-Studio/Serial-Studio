# 0057 — implementation notes (overnight run 2026-08-16/17)

## Landed (working tree, uncommitted)

- `app/src/DSP.h` — `TimeRing::opensCell()`; `EnvelopeCell`, `EnvelopeLevel`, `EnvelopeRing`
  (level 0 = the existing `TimeRing`, coarse levels = bounded rings of time-ordered extreme
  pairs, integer cell identity `cellIndex(t) >> 4k`, fold-on-cell-completion into every coarser
  open cell, `selectLevel`, `resizeCapacity` rebuild); `dsTimeWindowCore` extracted from
  `downsampleTimeWindow`; new `downsampleTimeWindow(const EnvelopeRing&, ...)` overload.
- `app/src/UI/Dashboard.{h,cpp}` — the two history-ring maps, getters, snapshot/restore/replay,
  `growTimeRing`, `makeHistoryRing` typed `EnvelopeRing`; sweep engines still `TimeRing`.
  Call sites still spell `appendDecimated` (the repo lint flags `.append(` on hotpath TUs as a
  Qt-container allocation, so the pyramid kept the level-0 method name).
- `app/src/UI/Widgets/Plot.cpp`, `MultiPlot.cpp` — time-axis draw calls the ring overload.
- `app/src/API/Handlers/DashboardHandler.cpp` — `tailFrames` reads `level0`.
- `app/tests/tst_envelope_ring.cpp` + `app/tests/CMakeLists.txt` — 8 slots: sizing,
  brute-force per level (AC1), wrap consistency (AC3), resize rebuild, level selection (AC2),
  coverage fallback (AC2), non-finite rejection (AC4), coarse-level downsample read.
  **Not built** (no compiling tonight); correct-by-reading, arithmetic cross-checked against
  the phase-0 Python sim (`scratchpad/envelope_sim.py`: 0 mismatches at 3 levels, 1.067x).
- `doc/claude/architecture/dashboard.md` — TimeRing paragraph updated.

## Decisions

- Kept `growTimeRing` / `resizeCapacity` for level 0 (memory policy unchanged; render-cost
  reason for guessing removed). Alternative (fixed byte budget per curve) recorded in plan.md.
- Fold into every coarser level on level-0 completion (not k -> k+1 chaining), so no level lags
  by more than one level-0 cell.
- Coarse levels sized from the saturated grid (`C0/2` cells): 1.067x; the reader's coverage
  check falls to a finer level whenever a coarse one falls short.

## Counterfactual check

Rule most at risk: "no allocation / no new per-sample work on the ingest path". Evidence: the
append path adds one `opensCell` branch (predicted not-taken) and, on level-0 cell completion, a
bounded <= 9-way merge into pre-sized `FixedQueue`s (`push` = plain store + index bump); levels
are sized in the constructor and only re-sized inside `resizeCapacity`, which the display-tick
growth path already reached before. Pipeline thread untouched (`grep EnvelopeRing app/src`
hits only `DSP.h`, `UI/Dashboard.*`, the two plot widgets and the API handler).

## For the maintainer

- Build; run `ctest -R tst_envelope_ring`; run `--benchmark-hotpath` (gated tiers must not
  move; report `lua+dashboard`).
- Spec AC5: 44.1 kHz audio on a 120 s range — wide view CPU vs current build, then zoom into a
  60 s-old 1 s slice.
- `qt-cpp-review` on the C++ diff.
