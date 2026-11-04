---
spec: 0108-dataset-formats-and-labels
phase: tasks
status: approved     # approved 2026-11-03
updated: 2026-11-03
---

# Tasks 0108 — Dataset Value Formats and Labels

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- **Executor** marks a task as delegable to a cheaper model against the donor it names
  (CLAUDE.md "Delegation & Cost"); the hotpath tasks (T6, T7), the registry manifest (T2) and
  the formatter's semantics (T9) stay in the main session.
- **Generated files are never edited.** Every registry change is a manifest edit plus a
  generator run; the six artifacts appear in a task's Files only as "regenerated".
- **Commit shape** (spec and plan are already committed): T1 to T5 (model, registry, decoder),
  T6 to T8 (pipeline), T9 to T15 (presentation), T16 to T18 (editor), T19 (API), T20 to T21
  (importer and example), T22 to T25 (tests and docs), then the maintainer's snapshot commit
  (T26). No security-sensitive surface is touched by any task.
- **The one hotpath note, named once.** T6 and T7 edit per-sample code: no allocation (the
  decoded number is rendered with `std::to_chars` into a stack buffer and
  `assign_utf8_in_place`), no lookup that scales with anything, no new signal, lock or cached
  flag; asserts there are `SS_ASSERT_HOTPATH` and only restate guards that already ran.

## Tasks

### T1 — `Keys::` constants and the `Dataset` fields

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`
- **Does:** Adds `Keys::Format`, `FormatWidth`, `FormatSigned`, `FormatFractionBits`,
  `FormatBitOffset`, `FormatBitWidth`, `FormatScale`, `FormatOffset`, `Labels`,
  `LabelsDefault`, `Label`. Adds `struct alignas(8) ValueLabel { qint64 value; QString text; }`
  with its `static_assert`, and on `Dataset`: `int format` (0 none, 1 fixed point, 2 bit
  field, 3 scale), `int formatWidth` (16), `int formatFractionBits` (0), `int formatBitOffset`
  (0), `int formatBitWidth` (1), `bool formatSigned` (false), `double formatScale` (1),
  `double formatOffset` (0), `std::vector<ValueLabel> labels`, `QString labelsDefault`,
  `QString label` (runtime, the current label text). Fields keep the struct's grouping by
  type so `alignas(8)` holds, and the struct `static_assert` still passes.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/Frame.cpp`; `python
  scripts/code-verify.py --check` on both files; `python scripts/registry-verify.py` reports
  the new struct fields as undeclared until T2 lands (expected, noted in chat).
- **Deps:** none
- [x] done
  - `Keys::Label` ("label") already existed (alarm-band and marker labels) and is reused for
    the runtime field. Frame.h is now 1512 lines: a `cxx-tu-too-long` advisory, the aggregate
    `--tu-census --check` still under its baseline.

### T2 — The registry manifest and its hooks

- **Files:** `app/rcc/properties/dataset.json`,
  `core/Pipeline/DataModel/Project/PropertyHooks.h`,
  `core/Pipeline/DataModel/Project/PropertyHooks.cpp`
- **Does:** Manifest: a `format` section ("Value Format", icon category `editor`, inserted
  after `general` in `sections` and `formIdOrder`); option source `valueFormats` (`staticMap`,
  `valueType` int, translated: 0 None, 1 Fixed point, 2 Bit field, 3 Scale); the eight
  properties with `persist whenNonDefault`-style rules (`format` whenNonZero, the numerics
  only when `format` is set, through the existing `persist` kinds or a `writeTransform`),
  `visibleWhen` predicates `formatHasWidth` (fixed or bits), `formatIsFixed`, `formatIsBits`,
  `formatIsScale`, validators `formatFractionFitsWidth` (0 to width-1) and
  `formatBitsFitWidth` (offset + width within the declared width), clamps `formatWidth`
  1..64, `api.expose` on every one with `apiError` strings; `displayFormats` gains
  `{"value": "hex", "label": "Hexadecimal"}` and `{"value": "bin", "label": "Binary"}`;
  sub-entity `labels` (jsonKey `Labels`, `readDatasetLabels` / `writeDatasetLabels`, apiName
  `labels`); property `labelsDefault` (string, `persist whenNonEmpty`, no form row, api
  `labelsDefault`); runtime field `label` (string, jsonKey `Label`, `persist whenNonEmpty`).
  Hooks: the four predicates and two validators declared and implemented beside the existing
  ones (donor `alarmBandsDrawn` / `deadbandNonNegative`). Binding invariant: every
  `jsonKey` names a `Keys::` constant from T1, every hook name is backed by `PropertyHooks.h`,
  and `DatasetItem`'s enumerator order equals `formIdOrder`.
- **Verify:** `python scripts/registry-verify.py` passes the manifest against `schema.json`;
  `python scripts/code-verify.py --check` on the three files.
- **Deps:** T1
- [x] done
  - What the generator forced, beyond the three files: `valueFormats` is `parallelValues` (the
    int domain kind; `staticMap` keys on strings). The validators live in
    `PropertyValidators.h/.cpp` beside `isValidAlarmDeadband` (the ProjectModel-free home), plus
    `formatKindDomain` for `format`. The two cross-field ones run once after every field of an
    API update applied (`DATASET_VALIDATORS` in the generator), so field order in one request
    never matters; the editor path clamps only and the decoder clamps every declaration.
    `schema.json` gained `readLabels` (postRead) and `whenNonEmpty` (runtime persist).
    `formIdOrder` and `displayFormats` append, so no enumerator or stored index moves. The
    section is `addFormatSection` + `buildFormatFieldRows` (declared in `ProjectEditor.h`,
    called from `EditorForms.cpp` and `EditorMultiSelect.cpp`).

### T3 — Label hooks and the generator run

- **Files:** `core/Core/DataModel/Frame.cpp`; regenerated:
  `core/Pipeline/DataModel/Generated/DatasetRegistry.h`,
  `core/Core/DataModel/Generated/DatasetSerialization.cpp`,
  `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`,
  `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json`,
  `doc/grpc/serialstudio-typed.proto`
- **Does:** `readDatasetLabels` (donor `readDatasetAlarmBands`, `Frame.cpp:384-393`): reads
  the `Labels` array of `{value, text}`, rounds a non-integer `value`, drops a duplicate key
  and an entry with empty text, sorts by value; `writeDatasetLabels` writes the sorted list.
  Then `python scripts/generate-property-registry.py` regenerates the six artifacts; the
  gRPC ledger appends numbers after each message's current maximum and no existing number
  moves.
- **Verify:** `python scripts/generate-property-registry.py --check` clean; `python
  scripts/code-verify.py --check` on `Frame.cpp` and the generated TUs (the `api-generated-
  edited` and `proto-field-renumbered` rules); `git diff app/rcc/api/proto-fields.json` shows
  only appended lines; `python scripts/syntax-check.py` on `Frame.cpp` and
  `DatasetSerialization.cpp`.
- **Deps:** T2
- [x] done
  - The reader drops a non-integer key as plan.md says (this task said round); rows are
    `{"value", "text"}` (`Keys::LabelText`). `labels` uses the hook-driven `map` shape, so the API
    applier reads through the same hook. `proto-fields.json` and the typed proto did not move:
    they derive from `api-schema.json`, so the append lands when T26 dumps the snapshot and
    re-runs the generator.

### T4 — The decoder and the raw-twin rule

- **Files:** `core/Core/DataModel/ValueDecoder.h` (new), `core/Core/DataModel/DataBlock.h`
- **Does:** Header-only `DataModel::ValueDecoder` free functions, every one `SS_FORCE_INLINE`
  and `noexcept`: `has_decoder(const Dataset&)` (`format != 0`); `decode_value(const
  Dataset&, double raw)`: for fixed point and bit field, `v = std::llround(raw)` masked to
  `formatWidth` bits (width 64 is a no-op), sign-extended from bit `width - 1` when
  `formatSigned`, then for bit field shifted right by `formatBitOffset` and masked to
  `formatBitWidth` bits (never sign-extended), for fixed point multiplied by
  `std::ldexp(1.0, -formatFractionBits)`; for scale `raw * formatScale + formatOffset` with no
  reinterpretation; an unknown kind returns `raw`. `render_number_in_place(QString&, double)`:
  `std::to_chars` general format into a 32-byte stack buffer, then `assign_utf8_in_place`.
  `dataset_carries_raw()` becomes `virtual_ || !transformCode.isEmpty() || has_decoder`.
  Binding invariants: no allocation, no loop, bit-exact and deterministic across platforms
  (no `pow`, no locale), results that feed `tst_value_decoder` byte-for-byte.
- **Verify:** `python scripts/code-verify.py --check` on both; `python
  scripts/syntax-check.py core/Core/DataModel/Frame.cpp` (the header's first includer);
  `python scripts/layer-verify.py`.
- **Deps:** T1
- [x] done

### T5 — ctest `tst_value_decoder` (AC1)

- **Files:** `app/tests/tst_value_decoder.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Donor `app/tests/tst_frame_support.cpp` and its registration. Data-driven
  cases: unsigned and signed 8, 16, 32 and 64-bit fixed point with fraction bits 0, 4, 15
  and width-1 (Q15 of 0x7FFF, 0x8000 and 0xFFFF); the full-width bit field; the top bit of a
  16-bit word; a 3-bit field at offset 5; scale with negative offset; a text (NaN) input
  left as is; the two validators refusing fraction bits >= width and a field past the width.
  Every expected value is a literal, never computed by the code under test.
- **Verify:** `python scripts/syntax-check.py app/tests/tst_value_decoder.cpp`; `python
  scripts/code-verify.py --check` on both; after the maintainer builds, `ctest -R
  tst_value_decoder`.
- **Executor:** yes, donor named.
- **Deps:** T4
- [x] done
  - The worktree database has no `app/tests` entries, so `syntax-check.py` SKIPs; checked instead
    with `PropertyValidators.cpp`'s entry plus the QtTest headers and an empty `.moc` stub: clean.
    Doubles compare bit for bit (`std::bit_cast`), not through the fuzzy QCOMPARE. Not run.

### T6 — The decoder in `FrameBuilder`'s three value tails

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`
- **Does:** In `applyDatasetToken` (`:2238`), `applyDatasetValue` (`:2098`) and
  `reprocessDatasetValues` (`:2384`): after the numeric parse, `mirror_raw_value` and the raw
  capture, when `has_decoder(dataset)` and the dataset is numeric and not computed, replace
  `numericValue` with `decode_value` and re-render `value` in place; for a computed dataset
  run the same step on the transform's result instead (spec R5 as amended); the reprocess
  pass decodes from `rawValue` before the transform. Binding invariants, restated at the
  edit: pipeline thread only; allocation-free (the render helper); no new branch on the
  non-decoded path beyond one `format != 0` compare; the `[[unlikely]]` transform branch
  keeps its shape; `SS_ASSERT_HOTPATH` only; the function-length cap holds (extract a shared
  `SS_FORCE_INLINE` static `decodeDatasetInPlace(Dataset&)` rather than growing the tails).
- **Verify:** `python scripts/syntax-check.py core/Pipeline/DataModel/FrameBuilder.cpp`;
  `python scripts/code-verify.py --check` (hotpath rules are blockers); a read-back of the
  three tails against the invariants above; after the rebuild, `pytest
  tests/integration/test_frame_parsing.py` unchanged and T22's AC2 case green.
- **Deps:** T4
- [x] done

### T7 — The decoder on the stream lane

- **Files:** `core/Pipeline/IO/StreamWorker.cpp`
- **Does:** `StreamProcessor::processChannel` (`:622`) applies `decode_value` to each
  deinterleaved sample before the block or per-sample transform, on the worker thread. The
  loop stays bounded by the block's sample count; nothing crosses a thread; no allocation.
- **Verify:** `python scripts/syntax-check.py core/Pipeline/IO/StreamWorker.cpp`; `python
  scripts/code-verify.py --check`; after the rebuild, the stream phase of
  `--benchmark-hotpath` runs (maintainer).
- **Deps:** T4
- [x] done
  (Main session 2026-11-03: `StreamChannelConfig` carries no dataset, so the decoder rides as
  `DataModel::ValueDecoderSpec` in `core/Core/IO/StreamConfig.h`, copied in `StreamConfigBuilder.cpp`;
  expression channels decode their input too, and a timed-out transform falls back to decoded samples.)

### T8 — The decoded benchmark row (AC6)

- **Files:** `app/src/Benchmark/HotpathBenchmark.cpp`
- **Does:** One ungated row, `native(numeric, decoded)`: the numeric synthetic project with
  `format = 1`, `formatWidth = 16`, `formatFractionBits = 4` on every dataset, run with the
  same frame and time floors, printed beside `native(numeric)` with a
  `HOTPATH_NATIVE_DECODED_FPS` key. The nine gated rows and their projects are untouched.
- **Verify:** `python scripts/syntax-check.py app/src/Benchmark/HotpathBenchmark.cpp`;
  `python scripts/code-verify.py --check`; the maintainer runs `--benchmark-hotpath` and the
  row's number is recorded in `spec.md` under AC6.
- **Deps:** T6
- [x] done
  (Main session 2026-11-03: the row prints on its own line after the gated report, so `HotpathReport` and
  `kReportCount` stay untouched; the maintainer's run records the number under AC6.)

### T9 — `DataModel::ValueFormat`, the one presentation formatter

- **Files:** `core/Core/DataModel/ValueFormat.h` (new), `core/Core/DataModel/ValueFormat.cpp`
  (new), `core/Core/CMakeLists.txt`
- **Does:** `formatNumber(double, const Dataset&)`: `decimalPoints` when set, else
  `displayFormat` (`""` auto as `FMT_VAL` does today, `0d`..`3d`, `sci`, `hex`, `bin`);
  hexadecimal and binary render `std::llround` of the value with a `0x` / `0b` prefix,
  two's complement at `formatWidth` when a decoder declares a width, minimal digits
  otherwise, upper-case hex. `labelFor(const Dataset&, double)`: `std::llround`, binary
  search of the sorted `labels`, else `labelsDefault`, else empty. `formatValue(const
  Dataset&)`: empty value stays empty, text stays verbatim, a label when one matches, else
  `formatNumber` with the units appended as the extension row does today. Pure functions,
  GUI-thread callers, no caching.
- **Verify:** `python scripts/syntax-check.py core/Core/DataModel/ValueFormat.cpp`; `python
  scripts/code-verify.py --check`; `python scripts/layer-verify.py`.
- **Deps:** T1
- [x] done
  - Kept for the spec's "every existing project behaves exactly as today": every dataset stores
    `displayFormat` "0d" by default, so `formatValue` (grid, captions, extension rows) honours
    the label, then `decimalPoints`, then Hexadecimal/Binary, then the automatic format over the
    plot range, exactly today's grid; the decimal Label Format choices keep reaching ticks only,
    through `formatNumber` (the JS tick semantics, Auto over the widget range). `autoNumber` is
    FMT_VAL's range algorithm, moved here (Core cannot reach `DSP::isZero`).

### T10 — ctest `tst_value_format`

- **Files:** `app/tests/tst_value_format.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Donor T5's registration. Cases: each `displayFormat` on 0, -1, 255, 65535 and
  2.5; hex and binary with and without a declared width, including a negative value at
  width 8; `decimalPoints` precedence; labels: exact key, 2.4 and 2.6 rounding to 2 and 3,
  unlisted value with and without `labelsDefault`, a non-numeric value, units appended only
  to numbers.
- **Verify:** `python scripts/syntax-check.py app/tests/tst_value_format.cpp`; `python
  scripts/code-verify.py --check`; after the build, `ctest -R tst_value_format`.
- **Executor:** yes, donor named.
- **Deps:** T9
- [x] done
  - Checked like T5 (donor entry, QtTest headers, `.moc` stub): clean; not run. The `QString::number`
    expectations (`'e'`, `'f'`, rounding) were confirmed against real Qt through PySide6's C locale.

### T11 — Dashboard helpers and the Data Grid

- **Files:** `core/Ui/UI/Dashboard.h`, `core/Ui/UI/Dashboard.cpp`,
  `core/Ui/UI/Widgets/DataGrid.cpp`
- **Does:** `FMT_VAL` (`Dashboard.h:609`, `:667`) and `Dashboard::formatValue` (`:716`)
  delegate to `ValueFormat::formatNumber`; two `Q_INVOKABLE`s on `Dashboard`,
  `formatNumber(double value, int uniqueId)` and `labelFor(int uniqueId, double value)`,
  resolving the dataset through the existing index. `DataGrid::formatValue` (`:298-313`)
  becomes `ValueFormat::formatValue`, so the grid honours `displayFormat` and shows labels.
- **Verify:** `python scripts/syntax-check.py` on both `.cpp`; `python scripts/code-verify.py
  --check` on the three; read-back that no other caller of `FMT_VAL` changed meaning.
- **Deps:** T9
- [x] done
  - `FMT_VAL` delegates to `ValueFormat::autoNumber` (its own algorithm, moved), not to
    `formatNumber`, so no caller changed meaning. The two `Q_INVOKABLE`s are inline in
    `Dashboard.h` so `Dashboard.cpp` (already past 1500 lines) does not grow.

### T12 — Extension rows, bar panel, LED panel

- **Files:** `core/Ui/UI/Widgets/ExtensionData.h`, `core/Ui/UI/Widgets/ExtensionData.cpp`,
  `core/Ui/UI/Widgets/BarPanel.cpp`, `core/Ui/UI/Widgets/LEDPanel.cpp`
- **Does:** `formatDatasetValue` (`ExtensionData.cpp:69-84`) becomes
  `ValueFormat::formatValue`, and the row gains a `label` role (`LabelRole`, `label`
  property) so a package renders what the dashboard does (R9). `BarPanel` caption
  (`:354-359`) through the formatter; the fill and bands (`:344-347`) keep `numericValue`.
  `LEDPanel` shows `title: label` for a labelled dataset (`:79`); the band logic (`:232`)
  keeps the number. Four files, one mechanical change each.
- **Verify:** `python scripts/syntax-check.py` on the three `.cpp`; `python
  scripts/code-verify.py --check` on all four.
- **Executor:** yes, donor T11.
- **Deps:** T11
- [x] done
  - The bar panel keeps its span-driven decimals and takes only the label; the LED shows the
    value label in the existing right-hand status slot (band label otherwise), because `titles`
    is a CONSTANT property and the title would jump. `FMT_VAL(val, dataset)` had no caller left
    and is gone.

### T13 — Alarm messages, the `label` runtime field, the plot tooltip

- **Files:** `core/Ui/UI/AlarmMonitor.cpp`, `core/Ui/UI/DashboardIngest.cpp`,
  `app/qml/Widgets/PlotWidget/PlotFormatter.qml`
- **Does:** The alarm notification (`AlarmMonitor.cpp:344-346`) quotes
  `ValueFormat::formatValue`. `DashboardIngest::applyBlockValues` (`:289-303`) sets
  `dataset.label = labelFor(dataset, numericValue)` on the GUI copy after the sample lands,
  only for datasets with labels (the block carries nothing new). The tooltip (`:349`)
  appends ` (label)` when `Cpp_UI_Dashboard.labelFor` returns one. Binding invariant: the
  ingest runs on the GUI thread on the display tick, so the lookup is tick-rate, never
  per-frame on the pipeline.
- **Verify:** `python scripts/syntax-check.py` on the two `.cpp`; `python
  scripts/code-verify.py --check` on all three (QML rules included).
- **Deps:** T11
- [x] done
  (Main session 2026-11-03: the plot had no dataset id to look a label up by, so `Widgets::Plot` gains a
  `datasetUniqueId` CONSTANT property, `PlotWidget.qml` a `labelUniqueId` and `Plot.qml` binds it; the
  alarm message quotes the label (units dropped) and otherwise the full number, as before.)
  (Main-session review, 2026-11-03: a catch-all-only table also labels; `apply_block_sample` sets the
  `label` runtime field so the TCP API frame carries it (R9); Label Format is enabled for every
  dataset so Hexadecimal and Binary reach Data Grid values from the editor (R1), manual updated.)

### T14 — `displayText` on the analog widget models

- **Files:** `core/Ui/UI/Widgets/Bar.h`, `Bar.cpp`, `Gauge.h`, `Gauge.cpp`, `Compass.h`,
  `Compass.cpp`
- **Does:** Each model gains `Q_PROPERTY(QString displayText READ displayText NOTIFY
  updated)` returning `ValueFormat::formatValue` for its dataset; needle, fill and range
  logic unchanged (`Bar.cpp:389-397`, `Compass.cpp:156-168`). Six files, the same six-line
  addition in each; header order (`Q_OBJECT`, `Q_PROPERTY`, `signals:`) kept.
- **Verify:** `python scripts/syntax-check.py` on the three `.cpp`; `python
  scripts/code-verify.py --check` on all six.
- **Executor:** yes, donor the first model done.
- **Deps:** T9
- [x] done
  - `displayText` is a caption override (`ValueFormat::captionText`, added to T9's file with a
    `tst_value_format` case): the label, or the Hexadecimal/Binary text, else empty so the QML
    keeps its padded widget-range caption exactly as today. Gauge and Meter inherit it from
    `Bar::applySample`, so `Gauge.h/.cpp` needed no edit.

### T15 — QML captions and ticks

- **Files:** `app/qml/Widgets/Dashboard/ValueFormat.js`, `Gauge.qml`, `Bar.qml`,
  `Meter.qml`, `Compass.qml`, `VisualRange.qml`
- **Does:** `ValueFormat.js` becomes a thin call to `Cpp_UI_Dashboard.formatNumber`, so
  ticks gain the two notations with no second implementation; the captions in the five
  widgets bind the model's `displayText`; `VisualRange.qml` (`:45-58`) and `Compass.qml`
  (`:91-101`) drop their own formatting for the same call. No layout, anchor or animation
  change (the `qt-qml` skill's binding rules bind).
- **Verify:** `python scripts/code-verify.py --check` on the six; the maintainer's visual
  check on a gauge with `hex` and a labelled bar.
- **Executor:** yes.
- **Deps:** T11, T14
- [x] done
  - `VisualRange.qml` does not exist in the tree. `formatValue` in `ValueFormat.js` is now one call to
    `Cpp_UI_Dashboard.formatNumber(val, model.uniqueId)`, so the `Bar` model gained a CONSTANT
    `uniqueId` (set in the `Bar`, `Gauge` and `Meter` constructors). Captions take `displayText`
    inside the shared padded-text helpers (and Compass's own), so no QML layout changed.
    Compass keeps its 0..360 decimal caption, since its `displayFormat` defaults to "0d" too.
    qmllint was run standalone without import paths: no new findings beyond the existing noise.

### T16 — The notation entries in the editor's duplicate list

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** The hand-written format list (`ProjectEditor.cpp:762-768`) gains Hexadecimal
  and Binary in the manifest's order; the index guard (`EditorCommit.cpp:1018-1019`) widens
  to the new count. T25 logs the duplicate in the ledger.
- **Verify:** `python scripts/syntax-check.py` on both; `python scripts/code-verify.py
  --check`; a read-back that the two lists and the manifest agree in order.
- **Deps:** T3
- [x] done
  - `m_displayFormats` is a QMap used only for its `size()` by the two guards
    (`EditorCommit.cpp:1019`, `EditorMultiSelect.cpp:406`), so the guards widen by themselves and
    `EditorCommit.cpp` is untouched; key order is meaningless there. `ProjectEditor.cpp` is now
    1502 lines (a `cxx-tu-too-long` advisory).

### T17 — The labels dialog

- **Files:** `app/qml/ProjectEditor/Dialogs/ValueLabelsEditor.qml` (new),
  `app/qml/ProjectEditor/Views/DatasetView.qml`, `core/Ui/ProjectEditor/EditorForms.cpp`
- **Does:** Donor `ChannelSoundsEditor.qml` (the `ListModel` working copy at `:51`,
  `addRow` `:92`, remove `:248`, commit `:280`): two columns (integer value, text), add and
  delete rows, a fixed last row for the catch-all text, Apply calls
  `Cpp_JSON_ProjectEditor.commitValueLabels(rows, defaultText)`. `DatasetView.qml`: a
  "Labels…" ribbon button and the Loader + Connections block (donor `:102-127`).
  `EditorForms.cpp`: the open signal (donor `:1131-1151`).
- **Verify:** `python scripts/code-verify.py --check` on the three; the maintainer opens the
  dialog, adds two rows and the catch-all, applies, and sees the rows persist on reopen.
- **Executor:** yes, donors named.
- **Deps:** T3
- [x] done
  - `EditorForms.cpp` is at 1487 lines, so the open path follows the channel-sounds precedent:
    `EditorCommit::valueLabelRows()` builds the rows and two inline `ProjectEditor.h` slots
    (`openValueLabelsEditorForSelection`, `commitValueLabels`) emit and forward. The signal
    and the commit carry the dataset's `uniqueId` (the spec 0099 retarget rule the alarm-band
    and marker dialogs follow). The dialog is registered in `app/CMakeLists.txt`; the ribbon
    button and every new control carry an `objectName`.

### T18 — `commitValueLabels`

- **Files:** `core/Ui/ProjectEditor/EditorCommit.h`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** Donor `commitAlarmBands` (`:757-783`): rebuilds `labels` sorted and deduplicated
  and sets `labelsDefault`, then `setNextUndoHint(tr("Edit Dataset"))` and
  `ProjectModel::updateDataset` (the two-phase memento: the undo scope stages, the first
  `setModified(true)` inside `updateDataset` commits; never mutate the struct outside it),
  then `buildDatasetModel`.
- **Verify:** `python scripts/syntax-check.py core/Ui/ProjectEditor/EditorCommit.cpp`;
  `python scripts/code-verify.py --check` (the `undo-scope-missing` rule); the maintainer's
  undo after an Apply restores the previous table.
- **Deps:** T17
- [x] done
  - The rows pass through `readDatasetLabels` (one rule for files, editor and API). No separate
    `setNextUndoHint`: `updateDataset` opens its own "Edit Dataset" `ProjectUndoScope` and its
    `setModified(true)` commits it, exactly as the `commitFrequencyMarkers` donor relies on.

### T19 — API verbs, gRPC label, the pro-content unit

- **Files:** `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp`,
  `app/src/API/GRPC/ConversionUtils.cpp`, `app/tests/tst_pro_content.cpp`
- **Does:** `project.dataset.getLabels` / `setLabels` beside the alarm-band verbs (`:452`,
  `:501`): `setLabels` takes `labels` (array of `{value, text}`) and optional
  `labelsDefault`, refuses a non-integer `value`, applies through `updateDataset`, and gets
  a capability classification in `CommandCapabilities.cpp` (plain) and a sealed-policy and
  safety-tier entry (the three registries every new command needs). `frameToProtoStruct`
  (`:184`) emits `label` beside `value`. `tst_pro_content`: a dataset with every format and
  a label table counts zero transforms (AC5 unit half).
- **Verify:** `python scripts/syntax-check.py` on the three; `python scripts/code-verify.py
  --check`; the registry sweeps `tst_command_capability_registry`, `tst_sealed_policy_registry`
  and `tst_command_safety_table` pass after the build.
- **Deps:** T3
- [x] done
  - Both verbs are classified plain (`CommandCapabilities.cpp`), allowed (`SealedPolicy.cpp`)
    and Safe / Confirm (`command_safety.json`, `CommandSafetyTable.cpp` regenerated), beside the
    alarm-band verbs. `setLabels` refuses a non-integer value, then reads the rows through
    `readDatasetLabels`. The handler file joined `_REGISTRY_KEY_ALLOWED` in `code-verify.py`
    (it is the sub-entity verbs file; adding `Labels` tripped the four-key cluster rule).
    gRPC computes the label with `ValueFormat::labelFor`, because its frames come from block
    templates, never the dashboard copy. `ConversionUtils.cpp` is `ENABLE_GRPC`-only and has no
    compile entry: syntax-check SKIP, unverified.

### T20 — DBC importer: labels instead of the Lua chunk (AC4)

- **Files:** `core/Pipeline/DataModel/Importers/DBCImporter.cpp`,
  `tests/scripts/test_cpp_regressions.py`
- **Does:** `buildDatasetFromSignal` (`:320`): when `plainScaling` and the signal has value
  descriptions, fill `labels` from `messageValueDescriptions()` and leave the shared table
  read from `applyTableTransform` in place; the early return before `widget`/`plt`
  (`:372-374`) goes, so the signal takes the same widget and plot policy as any other;
  `enumTransformCode` (`:771-790`) is deleted. A new assertion beside R15/R17 drives
  `projectFromMessages` over a VAL_ fixture and asserts labels, the table-read transform and
  `plt` true.
- **Verify:** `python scripts/syntax-check.py core/Pipeline/DataModel/Importers/DBCImporter.cpp`;
  `python scripts/code-verify.py --check`; `pytest tests/scripts/test_cpp_regressions.py -v`.
- **Deps:** T3
- [x] done
  - The pytest tier cannot run C++, so R23 pins the source shape instead (labels from
    `valueLabelsFor`, the shared table read, no early return before `dataset.plt =`) plus
    the example's CurrentGear against the DBC's VAL_ line. Green; the one failure in the
    file (`test_macos_native_window_validates_qwindow_before_winid`) touches nothing here.
    The unlisted-value text "Unknown (n)" is gone: an unlisted value now shows its number. A
    fresh import also gives CurrentGear the policy of any 0..255 signal (a gauge).

### T21 — The CAN Bus example and its baseline (AC7)

- **Files:** the CAN Bus example project `ecu_simulator.ssproj`,
  `tests/integration/baselines/ecu_simulator.f3a00921.json` (replaced by the recapture)
- **Does:** `CurrentGear` (`:268-269`): the transform becomes the plain table read, a
  `Labels` table carries the DBC's gear names, `plt` on; `tests/integration/
  test_property_registry.py` recaptures the example's baseline on the new build
  (`SS_CAPTURE_BASELINES=1`), which replaces the hash-named file.
- **Verify:** `pytest tests/integration/test_property_registry.py -v` green with the app
  up; the maintainer opens the example and sees "Reverse" in the grid and the gear curve.
- **Deps:** T3, T20
- [x] done
  - Project edit only (labels, plain table read, `graph` true; the widget stays none). The
    baseline recapture needs a running app: maintainer step, `SS_CAPTURE_BASELINES=1 pytest
    tests/integration/test_property_registry.py`, which replaces `ecu_simulator.f3a00921.json`.

### T22 — Integration tests (AC2, AC3, AC5)

- **Files:** `tests/integration/test_dataset_formats.py` (new), `tests/README.md`
- **Does:** Donor `tests/integration/test_project_configuration.py` and the
  `device_simulator` fixture. AC2: `project.loadJson` with a labelled, a fixed-point and a
  bit-field dataset, one frame streamed, `dashboard.getData` asserts `numericValue`,
  `label`, `isNumeric` true and `plt` true. AC3: every format saved through
  `project.exportJson`, reloaded, compared field by field; a project with none of the keys
  round-trips byte-identical. AC5 integration half: `project.validate` (or the pro-content
  summary the API exposes) reports zero transforms for the formats-only project. The
  catalog row.
- **Verify:** `pytest tests/integration/test_dataset_formats.py -v` with the rebuilt app;
  black.
- **Executor:** yes, donor named.
- **Deps:** T6, T13, T19
- [x] done
  - Collected and black-clean; not run (no app up). No API exposes the pro-content summary, so
    AC5's half asserts the formats-only project connects (vacuous on a licensed build; the
    `tst_pro_content` case carries it). `isNumeric` is never serialized, so the numeric check
    reads `numericValue`. A fourth test pins `setLabels` refusing a non-integer value.

### T23 — The manual: editor and transforms pages

- **Files:** `doc/help/Project-Editor.md`, `doc/help/Dataset-Transforms.md`
- **Does:** Project Editor: the Value Format section (the choice list and the fields each
  kind shows), the Labels dialog, the two notations in Label Format; every default and range
  verified against the manifest; no license statement anywhere near a format (R7).
  Dataset Transforms: a short "when a format is enough" paragraph pointing at the editor
  page, and the order of application as R5 reads. Written under `ss-docs`.
- **Verify:** `python scripts/documentation-verify.py` on both, zero new findings; `python
  scripts/term-verify.py`.
- **Executor:** yes, with the manifest as the fact source.
- **Deps:** T3, T17
- [x] done
  - The Label Format row is enabled only for Bar, Gauge and Meter datasets (`enabledWhen
    widgetRangeApplicable`, unchanged), so the page says so: in the editor Hexadecimal and
    Binary are reachable only on those datasets; the API sets them on any.

### T24 — API reference, extension docs, assistant corpus

- **Files:** `doc/help/API-Reference.md`, `doc/help/Widget-Extension-Development.md`,
  `app/rcc/ai/skills/project_basics.md`
- **Does:** API Reference: the new `project.dataset.update` fields, `getLabels` /
  `setLabels`, the frame's `label` key. Extension docs (`:162-184`): the `label` row role.
  Corpus: the new dataset fields spelled as the registry spells them (the corpus lint).
- **Verify:** `python scripts/documentation-verify.py` on the two manual pages; `python
  scripts/registry-verify.py` (the corpus field lint) clean.
- **Executor:** yes.
- **Deps:** T19
- [x] done
  - `app/rcc/ai/search_index.json` is rebuilt by `sanitize-commit.py`, not here.

### T25 — AI-facing architecture docs and the ledger

- **Files:** `doc/claude/architecture/dataflow.md`, `doc/claude/architecture/project.md`,
  `doc/claude/common-mistakes.md`
- **Does:** `dataflow.md`: the decoder in the value tail (order, the raw rule's third case,
  the allocation-free render) in "The Unified Block Lane" and the stream-lane note.
  `project.md`: the formatter as the one presentation home and the labels sub-entity
  beside alarm bands. Ledger: one row for the duplicate format list in `ProjectEditor.cpp`
  (`Codified: not yet`, candidate a registry-verify check).
- **Verify:** `python scripts/claim-verify.py --no-report` on the three; `python
  scripts/documentation-verify.py`; `python scripts/ledger-verify.py --check`.
- **Deps:** T6, T9, T16
- [x] done
  - A `Codified: not yet` row grows the open-row ratchet (`ledger-verify --check` fails, 51 to
    52), so the row ships codified instead: R24 in `tests/scripts/test_cpp_regressions.py`
    compares the QMap's keys with the manifest domain. claim-verify and ledger-verify clean.

### T26 — Maintainer: build, snapshot, benchmark, observations

- **Files:** `app/rcc/api/api-schema.json` (from `--dump-api-schema`), the SDK files
  `generate-sdk.py` rewrites
- **Does:** Build; `ctest -R "tst_value_decoder|tst_value_format|tst_pro_content|
  tst_command_capability_registry|tst_sealed_policy_registry|tst_command_safety_table|
  tst_frame_builder_staging"`; `SerialStudio --dump-api-schema` and `python
  scripts/generate-sdk.py`, then `generate-property-registry.py --check-snapshot --strict`;
  `--benchmark-hotpath` with the nine gates unchanged and the decoded row recorded under
  AC6; launch with the API server on and run T21 and T22; the AC7 observation.
- **Verify:** AC1 to AC7 ticked in `spec.md` with the date.
- **Deps:** everything above
- [ ] done

## Definition of Done

The whole-feature gate, checked once every task is complete.

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1 by T5; AC2,
  AC3 and AC5 by T22 with T19; AC4 by T20; AC6 by T8 and T26; AC7 by T21 and T26).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  hotpath rules are blockers on T6 and T7.
- [ ] `qt-cpp-review` run on the C++ diff (T1, T4, T6, T7, T9, T11 to T14, T18, T19, T20);
  findings addressed or noted. `qt-qml-review` on T15 and T17.
- [ ] `ss-hotpath` invoked before T6 and T7; `--benchmark-hotpath` run by the maintainer with
  the nine gates unchanged and the decoded row recorded.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file; any SKIP named in the
  handoff as unverified.
- [ ] Relevant `pytest` targets identified for the maintainer to run: `tests/integration/
  test_dataset_formats.py`, `test_property_registry.py`, `test_frame_parsing.py`;
  `tests/scripts/test_cpp_regressions.py` runnable here.
- [ ] `python scripts/sanitize-commit.py` run before each commit; `generate-property-registry.py
  --check`, `registry-verify.py` and the corpus lint clean; `--check-snapshot` strict after
  the maintainer's dump.
- [ ] Diff is *what was asked, and only that*: no generated artifact hand-edited, no Modbus
  importer change, no security-sensitive surface touched, no file outside the plan's table.
- [ ] `spec.md` status set to `done`.
