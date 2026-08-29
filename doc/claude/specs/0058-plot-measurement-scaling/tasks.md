---
spec: 0058-plot-measurement-scaling
phase: tasks
status: approved     # gate auto-approved, overnight run 2026-08-16 (unattended; maintainer re-reviews)
updated: 2026-08-16
---

# Tasks 0058 — Plot measurement and scaling

> **Phase 3 of 4 — the ordered checklist.** Gate auto-approved for the unattended overnight run
> of 2026-08-16. Each task stands alone; a failure in one does not block the others.

## Tasks

### T1 — Convergent tick spacing (A2.1)

- **Files:** `app/qml/Widgets/PlotWidget.qml`
- **Does:** `fitInterval()` over the 1-2-5 sequence with measured labels; X and Y linear axes use
  it; `subTickCount` follows the mantissa; `xPrecision`/`yPrecision` derive from the interval.
- **Verify:** code-verify; read-back for binding loops (function reads only, writes nothing).
- **Deps:** none
- [x] done

### T2 — Cursor frequency readout (A2.2)

- **Files:** `app/qml/Widgets/PlotWidget.qml`
- **Does:** `frequencyLabel(hz)` SI formatter; delta label appends `1/ΔX` on time axes when
  `ΔX != 0`; bounded fit-to-width candidate list.
- **Verify:** code-verify; `qsTr()` on every string.
- **Deps:** none
- [x] done

### T3 — Quantized Y autoscale (A2.3)

- **Files:** `app/src/UI/Widgets/PlotAutoScale.h` (new), `Plot.h/.cpp`, `MultiPlot.h/.cpp`,
  `app/tests/tst_plot_autoscale.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ladder + hysteresis quantizer; both `padDerivedRange`s use it; unit test.
- **Verify:** code-verify; test correct-by-reading.
- **Deps:** none
- [x] done

### T4 — Ruler / time-axis UX (A2.4)

- **Files:** `app/qml/Widgets/PlotWidget.qml`, `app/qml/Widgets/Dashboard/Plot.qml`,
  `app/qml/Widgets/Dashboard/MultiPlot.qml`
- **Does:** `xZero` / `markers` / `hoverMarkerEnabled` state, right-click menu, rendering,
  relative labels; persistence in both hosts.
- **Verify:** code-verify; read-back.
- **Deps:** none
- [x] done

## Definition of Done

- [x] ACs in `spec.md` (AC1-AC4 maintainer; AC5 structural).
- [x] `python3 scripts/code-verify.py --check` clean on all changed files.
- [x] Diff is what was asked, and only that.
