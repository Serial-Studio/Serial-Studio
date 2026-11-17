---
spec: 0124-measured-harvest
phase: tasks
updated: 2026-11-17
---

# Tasks 0124 — Measured harvest

> **The ordered checklist.** Four packets of disjoint files, worked by the main session by
> hand. An item that fails its check is dropped whole (R7) and noted under its task.

## Tasks

### T1 — Driver deletions and folds

- **Files:** `core/Devices/IO/Drivers/Iec104.{h,cpp}`, `Modbus.{h,cpp}`,
  `Modbus/ModbusRedial.{h,cpp}`, `MQTT.cpp`, `HID.cpp`
- **Does:** Delete the unused accessors, inline `handleDialSetback()`, fold the repeated
  bodies. Binding: no `connect` edited, no assertion removed, each setter keeps its same-value
  guard, the Iec104 helper's name does not start with `set`.
- **Verify:** `syntax-check.py` and `code-verify.py --check` on each file; a tree-wide grep
  for each deleted name.
- **Deps:** none
- [x] done

### T2 — Sessions storage deletions

- **Files:** `core/Storage/Sessions/Player.{h,cpp}`, `Player/ReplayClock.{h,cpp}`,
  `Player/ReplaySynthesis.{h,cpp}`, `DatabaseWorker.{h,cpp}`, `Export.{h,cpp}`
- **Does:** Delete `filename()`, `base()`, the pass-through `buildFrameAt()` and
  `ExportWorker::createSchema`; inline `ensureSchemaInternal()` at its one call site.
- **Verify:** as T1, plus the unit tests under `app/tests` grepped for each name.
- **Deps:** none
- [x] done

### T3 — Shared API status indicator

- **Files:** `app/qml/Widgets/ApiStatusIndicator.qml` (new), `app/qml/MainWindow/Panes/Setup.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`, `app/CMakeLists.txt`
- **Does:** Move the indicator both panes carry into one component. Binding: string text and
  every `objectName` unchanged.
- **Verify:** the two source blocks are byte-identical before removal; `code-verify.py --check`.
- **Deps:** none
- [x] done

### T4 — Shared digital page

- **Files:** `app/qml/Widgets/Dashboard/DigitalPage.qml` (new), `Gauge.qml`, `Meter.qml`,
  `Bar.qml`, `app/CMakeLists.txt`, `scripts/code_verify_rules.py`
- **Does:** One digital readout page with declared inputs; the Meter takes the contrast ink
  (maintainer, 2026-11-17). Binding: the HDR boost stays with the item it boosts.
- **Verify:** `code-verify.py --check`; every name the page reads is declared in it.
- **Deps:** T3 (shared `CMakeLists.txt`)
- [x] done

### T5 — Page dots into the swipe helper

- **Files:** `app/qml/Widgets/Dashboard/SwipePages.qml`, `Gauge.qml`, `Meter.qml`, `Bar.qml`,
  `Compass.qml`
- **Does:** The helper draws the dots and exposes their height; each widget's bottom margin
  reads it. Clock untouched.
- **Verify:** `code-verify.py --check`; no `pageIndicator` id left in the four widgets.
- **Deps:** T4
- [x] done

### T6 — Ledger packet

- **Files:** `doc/claude/common-mistakes.md`, `scripts/ledger-baseline.json`
- **Does:** Five rows codified, two deleted, the `HAL_Driver` row corrected.
- **Verify:** `ledger-verify.py --check` reads 44; `claim-verify.py` 0 errors.
- **Deps:** none
- [x] done

### T7 — Dead data file, censuses, tracker

- **Files:** `doc/claude/specs/0070-concern-classes/decomposition-data.json` and its `spec.md`
  pointer, `scripts/dup-census.json`, `doc/claude/specs/0104-consolidation-quarter/tasks.md`
  (and its `spec.md` targets, by the maintainer's answer)
- **Does:** Delete the file, re-seed the duplicate census downward, record the measured
  readings and the amended singleton and ledger targets.
- **Verify:** `code-verify.py --dup-census --check`; `documentation-verify.py`.
- **Deps:** T1–T6
- [x] done

## Definition of Done

- [x] Every acceptance criterion the session can meet is checked off in `spec.md`; AC2 and AC3
      are the maintainer's.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] C++ diff re-read as a compiler would; syntax check zero failures over the ten edited
      `.cpp` files. `qt-cpp-review` run on 2026-11-17 (six read-only agents): no finding; one
      note, the new `indexOfValue` in `MQTT.cpp` repeats the one in
      `core/Storage/MQTT/BrokerOptions.cpp`, left for the maintainer to decide.
- [x] Hotpath not touched.
- [ ] `pytest tests/scripts/ scripts/tests/`: 882 pass, 1 fails
      (`test_macos_native_window_validates_qwindow_before_winid`). The failure is in
      `NativeWindow_macOS.mm` as committed in `5a773388d`, a file this spec does not edit.
- [x] Diff is *what was asked, and only that*.
- [ ] `spec.md` status set to `done` after the maintainer's build.

## Result

- Audit line count 467,338 → 466,879: −459 code lines (approved −426).
- Duplicate-window census 1,270 → 906; open ledger rows 51 → 44.
- Beyond the file table: the unused `getPaddedText` wrapper left Gauge, Meter and Bar with the
  digital page; `doc/claude/specs/0104-consolidation-quarter/spec.md` carries the amended
  singleton and ledger targets the maintainer approved.
- The digital page's swipe opacity stays bound at each use site, so no attached property is
  read on a component root.
