---
spec: 0108-dataset-formats-and-labels
phase: plan
status: approved     # approved 2026-11-03: decode what feeds the dataset, AC4 reworded, R6 DBC half only
updated: 2026-11-03
---

# Plan 0108 — Dataset Value Formats and Labels

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Three new things land on the dataset, each in the place the spec puts it. **A decoder**
(fixed point, bit field, linear scale) is eight typed registry properties on `Dataset` and one
header-only, allocation-free `decode_value()` that the three per-dataset value tails of
`FrameBuilder` and the stream lane call between the numeric parse and the transform; a decoded
dataset carries the pre-decode raw twin under the existing spec-0085 rule, so MDF4 and the
Historian keep the wire value for free. **Notation** (hexadecimal, binary) is two more entries
in the existing `displayFormats` option domain. **Labels** are a sub-entity list plus a
catch-all string, looked up only at presentation through ONE new formatter,
`DataModel::ValueFormat`, which replaces the scattered per-site formatting (`FMT_VAL`,
`DataGrid::formatValue`, the extension-row formatter, the bar panel, the alarm message, and
the QML `ValueFormat.js` which becomes a thin call into it). Because labels never enter
`Dataset::value`, every numeric consumer and every export keeps the number by construction;
the API frame gains an additive `label` key for remote clients. The DBC importer emits labels
instead of its Lua label chunk and lets the signal plot; the Modbus importer is the one spec
question below. Nothing is counted by the connect gate, which reads only transform code and
user tables.

## Spec questions raised by the ground truth (decided 2026-11-03, all as proposed; spec amended)

1. **R5 order for computed datasets.** A virtual dataset has no parsed value: its transform
   *is* its source (`FrameBuilder.cpp:2250-2253` clears the value, then the transform fills
   it). "Decoding before any transform" is undefined there. Proposed rule: *the decoder
   applies to the value that feeds the dataset: the parsed channel for a channel dataset (then
   the transform runs on the decoded value), the transform's result for a computed dataset.*
2. **AC4 wording.** Every importer dataset is a computed dataset reading a data table through
   `function transform(value) return tableGet(T, R) or 0 end` (`ImporterCommon.h:144-151`);
   positional channels were removed on purpose. "Zero transform code for the gear signal"
   cannot hold. Proposed: *the gear dataset carries no label code: its transform is the same
   table read every imported dataset carries, it holds a label table, and it is plotted.*
3. **R6, the Modbus halves.** The Modbus map importer writes no scale, offset or bit logic
   into any dataset transform: they live as one declarative line per register in the
   generated Lua parser's spec table (`ModbusMapImporter.cpp:916-949`, decode at `:804`,
   `:833`, `:836`), and the parser publishes the physical value to the table. Moving them to
   dataset Formats means the parser publishes raw words, the decoder runs after the table
   read (question 1), the codegen and its pinned regressions (`test_cpp_regressions.py` R14,
   R15) change, and the user sees the same `tableGet` transform either way. **Recommended:
   amend R6 to the DBC half only** and keep the Modbus parser as the declarative home of its
   scale and bits (alternative A in Tradeoffs delivers the halves as written).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/rcc/properties/dataset.json` | Section `format` ("Value Format", after `general`); option source `valueFormats` (`staticMap`, int: 0 None, 1 Fixed point, 2 Bit field, 3 Scale); properties `format` (int, ComboBox), `formatWidth` (int 1-64, default 16, `visibleWhen formatHasWidth`), `formatSigned` (bool, `visibleWhen formatHasWidth`), `formatFractionBits` (int 0-63, `visibleWhen formatIsFixed`, `validate formatFractionFitsWidth`), `formatBitOffset` and `formatBitWidth` (int, `visibleWhen formatIsBits`, `validate formatBitsFitWidth`), `formatScale` (double, default 1, `visibleWhen formatIsScale`), `formatOffset` (double, default 0, `visibleWhen formatIsScale`); `displayFormats` gains `hex` "Hexadecimal" and `bin` "Binary"; sub-entity `labels` (jsonKey `Labels`, read/write hooks, apiName `labels`); property `labelsDefault` (string, `persist whenNonEmpty`, no form row: edited in the labels dialog); runtime field `label` (string, jsonKey `Label`, `persist whenNonEmpty`). |
| `scripts/generate-property-registry.py` run | Regenerates `core/Pipeline/DataModel/Generated/DatasetRegistry.h`, `core/Core/DataModel/Generated/DatasetSerialization.cpp`, `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`, `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json` (append-only numbers), `doc/grpc/serialstudio-typed.proto`. Never hand-edited. |
| `core/Core/DataModel/FrameKeys.h` | `Keys::` constants for the new jsonKeys (registry-verify proves each exists). |
| `core/Core/DataModel/Frame.h` | `struct ValueLabel { qint64 value; QString text; }`; `Dataset` gains the eight format fields, `std::vector<ValueLabel> labels`, `QString labelsDefault`, `QString label` (runtime, the current label text). |
| `core/Core/DataModel/Frame.cpp` | `readDatasetLabels` / `writeDatasetLabels` hooks (donor `readDatasetAlarmBands`, `:384-393`); the reader sorts by value and drops a non-integer or duplicate key. |
| `core/Core/DataModel/ValueDecoder.h` (new) | Header-only: `has_decoder(const Dataset&)`, `decode_value(const Dataset&, double) noexcept` (integer reinterpretation at the declared width, sign extension, bit extraction, `scale * v + offset` with the fixed-point scale as `std::ldexp(1.0, -fractionBits)`), `render_number_in_place(QString&, double)` (`std::to_chars` into a stack buffer, then `assign_utf8_in_place`). Bit-exact, no allocation, `SS_FORCE_INLINE`. |
| `core/Core/DataModel/ValueFormat.h` / `.cpp` (new) | The ONE presentation formatter: `formatNumber(double, const Dataset&)` (decimals, scientific, hexadecimal, binary with the digit width from `formatWidth` when a decoder declares one), `labelFor(const Dataset&, double)` (round to integer, binary search the sorted labels, fall back to `labelsDefault`, else empty), `formatValue(const Dataset&)` (label when one matches, else `formatNumber`, text verbatim when not numeric). |
| `core/Core/DataModel/DataBlock.h` | `dataset_carries_raw()` adds `has_decoder(dataset)` to the ONE rule. |
| `core/Pipeline/DataModel/Project/PropertyHooks.h` / `.cpp` | Predicates `formatHasWidth`, `formatIsFixed`, `formatIsBits`, `formatIsScale`; validators `formatFractionFitsWidth`, `formatBitsFitWidth`; the `valueFormats` option provider if the static map needs translation. |
| `core/Pipeline/DataModel/FrameBuilder.cpp` | The decoder call in the three tails: `applyDatasetToken` (`:2238`, span and cell lanes), `applyDatasetValue` (`:2098`, list lane), `reprocessDatasetValues` (`:2384`, re-decodes from `rawValue`); the computed-dataset order of question 1. |
| `core/Pipeline/IO/StreamWorker.cpp` | `StreamProcessor::processChannel` (`:622`) decodes each deinterleaved sample before the block or sample transform, on the worker thread. |
| `app/src/Benchmark/HotpathBenchmark.cpp` | One ungated row, `native(numeric, decoded)`: the synthetic project with a fixed-point decoder on every dataset, printed beside `native(numeric)` (AC6). |
| `core/Ui/UI/Dashboard.h` / `.cpp` | `FMT_VAL` and `Dashboard::formatValue` delegate to `ValueFormat`; `Q_INVOKABLE` `formatNumber(double, int uniqueId)` and `labelFor(int uniqueId, double)` for QML. |
| `core/Ui/UI/Widgets/DataGrid.cpp` | `formatValue` (`:298-313`) becomes `ValueFormat::formatValue`. |
| `core/Ui/UI/Widgets/ExtensionData.cpp` / `.h` | `formatDatasetValue` (`:69-84`) becomes `ValueFormat::formatValue`; the row gains a `label` role so a package renders what the dashboard does (R9). |
| `core/Ui/UI/Widgets/BarPanel.cpp` | Caption text (`:354-359`) through the formatter. |
| `core/Ui/UI/Widgets/LEDPanel.cpp` | A labelled dataset's LED shows `title: label`; the band logic (`:232`) keeps the number. |
| `core/Ui/UI/AlarmMonitor.cpp` | The notification message (`:344-346`) quotes `formatValue`, so a labelled value reads as its label. |
| `app/qml/Widgets/Dashboard/ValueFormat.js` | Becomes a thin call to `Cpp_UI_Dashboard.formatNumber`; Gauge, Bar, Meter ticks keep calling it; their captions call `formatValue` through the widget model's new `displayText` property. |
| `core/Ui/UI/Widgets/Bar.cpp`, `Gauge.cpp`, `Compass.cpp` (+ `.h`) | `displayText` `Q_PROPERTY` computed by the formatter for the caption; the needle and fill keep `numericValue`. |
| `app/qml/Widgets/Dashboard/Gauge.qml`, `Bar.qml`, `Meter.qml`, `Compass.qml`, `VisualRange.qml` | Caption binds `displayText`; ticks unchanged except the notation now reaching them through `ValueFormat.js`. |
| `app/qml/Widgets/PlotWidget/PlotFormatter.qml` | The tooltip appends the label when `labelFor` returns one. |
| `core/Ui/ProjectEditor/ProjectEditor.cpp` | The hand-written duplicate of the format list (`:762-768`) gains the two notations (a drift magnet the tasks phase names in the ledger). |
| `core/Ui/ProjectEditor/EditorCommit.cpp` / `.h` | The index guard (`:1018-1019`) widened; `commitValueLabels(rows, defaultText)` (donor `commitAlarmBands`, `:757-783`). |
| `core/Ui/ProjectEditor/EditorForms.cpp` | The "Labels…" ribbon action and its open signal (donor `:1131-1151`). |
| `app/qml/ProjectEditor/Views/DatasetView.qml` | Ribbon button and the dialog Loader (donor `:102-127`). |
| `app/qml/ProjectEditor/Dialogs/ValueLabelsEditor.qml` (new) | Two-column table (value, text) plus the catch-all row; donor `ChannelSoundsEditor.qml`. |
| `core/Api/API/Handlers/ProjectDatasetFieldCommands.cpp` | `project.dataset.getLabels` / `setLabels` (donor the alarm-band verbs, `:452`, `:501`); `setLabels` validates integer keys. |
| `app/src/API/GRPC/ConversionUtils.cpp` | `frameToProtoStruct` (`:184`) emits `label` beside `value`. |
| `core/Pipeline/DataModel/Importers/DBCImporter.cpp` | `buildDatasetFromSignal` (`:320`): value descriptions become `labels` (keys are raw values, so only when `plainScaling`, as today), the `tableGet` transform stays, `widget` and `plt` are set like any other signal; `enumTransformCode` (`:771-790`) goes. |
| `core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp` | Untouched under the recommended R6 amendment (alternative A: `luaEntryLine` publishes raw words, `buildDatasetFromEntry` sets Scale or Bit field, R14/R15 pins regenerated). |
| The CAN Bus example project, `ecu_simulator.ssproj` | `CurrentGear` (`:268-269`): the plain table read, a label table, `plt` on (AC7); `tests/integration/baselines/ecu_simulator.f3a00921.json` recaptured (`SS_CAPTURE_BASELINES=1`). |
| `app/tests/tst_value_decoder.cpp` (new) + `app/tests/CMakeLists.txt` | AC1: fixed point and bit field bit-exact, signed widths, full-width field, top bit, out-of-range declarations refused by the validators. |
| `app/tests/tst_value_format.cpp` (new) | Notation digit widths, two's complement at the declared width, label lookup, catch-all, non-integer rounding, unlisted value. |
| `app/tests/tst_pro_content.cpp` | A dataset with every format and a label table counts zero transforms (AC5's unit half). |
| `app/tests/tst_dbc_importer.cpp` | AC4: the gear signal yields labels, the shared table read, `plt` true. Its `ss_add_unit_test` is commented out (`CMakeLists.txt:1601`); the tasks phase finds out why before relying on it. |
| `tests/integration/test_dataset_formats.py` (new) | AC2 and AC3 over the API with the device simulator. |
| `tests/integration/test_property_registry.py` | Baselines recaptured for the changed example. |
| `doc/help/Project-Editor.md`, `Dataset-Transforms.md`, `API-Reference.md`, `Widget-Extension-Development.md` | The Value Format section, the labels table, notation, the `label` field; "no license" stated where transforms are described as Pro. |
| `app/rcc/ai/skills/project_basics.md` | The new dataset fields (the corpus lint checks field names against the registry). |
| `doc/claude/architecture/dataflow.md`, `project.md` | The decoder in the value tail, the raw rule's third case, the formatter as the one presentation home. |

## Architecture & data flow

- **Decoder, pipeline thread.** In `applyDatasetToken` the order becomes: token to `value`
  in place, `numericValue` from the hint or `toDouble`, `mirror_raw_value` (now also for a
  decoded dataset, so the raw twin is the wire integer), raw capture, **`decode_value`** when
  `has_decoder` (numeric input only; a text token is left alone, the dataset stays
  non-numeric), the decoded number rendered into `value` in place, then the transform, the
  clamp, the expression publish and the final capture exactly as today. The list lane mirrors
  it. `reprocessDatasetValues` re-decodes from `rawValue` before the transform for channel
  datasets. For a computed dataset the decoder runs on the transform's result (question 1).
  The stream lane decodes each sample in `processChannel` before the block transform.
- **Decoder arithmetic.** `v = std::llround(raw)` reinterpreted at `formatWidth` bits:
  unsigned masks to the width, signed sign-extends from bit `width - 1`; a bit field then
  shifts by `formatBitOffset` and masks `formatBitWidth` bits; fixed point multiplies by
  `ldexp(1, -formatFractionBits)`; scale computes `v * formatScale + formatOffset` on the
  double without reinterpretation. Widths above 53 bits lose integer precision through the
  double parse; the validator caps `formatWidth` at 64 and the manual says 53 is exact.
- **Notation and labels, GUI thread.** Nothing on the pipeline knows about them. Every site
  that turns a dataset into text calls `ValueFormat`; the plot curves, needles, fills, bands,
  FFT and exports read `numericValue` or the block's `values` as today and never see a label.
  The block's `text` column keeps carrying the rendered number, so CSV (`text`, else the
  number) and the Historian are unchanged.
- **The `label` runtime field.** Set by `DashboardIngest::applyBlockValues` after the sample
  lands (`labelFor`), carried on the GUI copy of the dataset, serialized by the generated
  writer under `persist whenNonEmpty`, so `dashboard.getData`, the API frame stream and
  gRPC (`frameToProtoStruct`) expose it beside `value` and `numericValue` (R9). Projects
  without labels never gain the key.
- **Editor.** The generated form renders the `format` section from the registry with the
  `visibleWhen` predicates hiding rows that do not apply; the labels dialog is the alarm
  bands pattern (Loader, open signal, `ListModel` working copy, commit through one
  `EditorCommit` method that calls `updateDataset` under the undo hint).
- **Importers.** The DBC importer builds `labels` from `messageValueDescriptions()` and no
  longer returns early before `widget`/`plt`. Both importers keep `applyTableTransform`.

## Hotpath & threading impact

- **Touches the hotpath?** Yes: `FrameBuilder`'s three per-dataset value tails and the stream
  worker's per-sample loop. Rules preserved: the decoder is an inline function over the
  dataset's own fields (no lookup, no table, no `std::map`), allocation-free (`std::to_chars`
  into a stack buffer, `assign_utf8_in_place` into the owned string), bounded (one sample,
  no loop), `SS_ASSERT_HOTPATH` only where a guard already ran; a project without decoders
  pays one `format != 0` compare per dataset per frame. No pool, SPSC, connection or
  structure-generation change. Benchmark plan: the nine gated rows run the same synthetic
  project as today and must not move; the new ungated `native(numeric, decoded)` row prints
  the decoder's cost next to `native(numeric)` and the tasks phase records it in the spec.
- **New cross-thread signal/slot?** No. The formatter runs on the GUI thread on data it
  already holds; the editor and API paths are command rate.
- **New input to a cached hotpath flag?** No. The decoder is per-dataset configuration
  carried by the project snapshot, like `transformCode`; `dataset_carries_raw` is a per-column
  bind-time rule, not a cached flag.
- **Timestamp ownership** — unchanged; no block, frame or stamp is touched.

## Data model & persistence

- `Keys::` gains `Format`, `FormatWidth`, `FormatSigned`, `FormatFractionBits`,
  `FormatBitOffset`, `FormatBitWidth`, `FormatScale`, `FormatOffset`, `Labels`,
  `LabelsDefault`, `Label`. All persist only when set (`whenNonDefault` / `whenNonEmpty`),
  so a project without them round-trips byte-identical (AC3) and an older release ignores
  unknown keys and shows the undecoded number (R11): the loader reads named keys only.
- `Labels` is a JSON array of `{"value": int, "text": str}` sorted by value on read;
  `LabelsDefault` the catch-all text. No schema version bump: additive keys.
- No Sessions DB change: the Historian stores the block's values and text as today.

## API / SDK surface

- `project.dataset.update` accepts every new property through the generated appliers;
  `project.dataset.getLabels` / `setLabels` are the dedicated verbs (alarm bands precedent).
- The frame JSON (`dashboard.getData`, the API stream, the assistant's view) gains `label`
  on a labelled dataset; gRPC's frame struct gains the same key. gRPC field numbers for the
  new `project.dataset.update` parameters are appended by the ledger.
- `api-schema.json` is refreshed by the maintainer's build (`--dump-api-schema`); the SDK
  comments regenerate from it (`generate-sdk.py`); `--check-snapshot` is strict in CI.
- Widget extensions read `label` as a new row role.

## QML / UI

- Dataset page: a "Value Format" section (combo plus the fields that apply) and a "Labels…"
  ribbon button; the Label Format combo lists Hexadecimal and Binary.
- Dashboard: captions, the grid, LED titles, the bar panel, alarm messages and the plot
  tooltip show the label; ticks show the notation. Nothing changes for a dataset using none.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the decoder's configuration lives | (a) eight typed registry properties read per sample; (b) one compact text property (`fixed:s16.4`) parsed per sample; (c) typed properties compiled into a runtime POD cached on the dataset | **(a).** The registry generates the form, the serializer and the API from typed fields; the decode is a handful of integer ops, so (c)'s cache buys nothing and adds a runtime field the registry schema does not type; (b) is spec 0111's wire form, which that plan parses *into* (a). |
| Decoder position relative to the raw twin | (a) raw = wire integer, decode after `mirror_raw_value`; (b) raw = decoded value | **(a).** "Exactly as it would see a transform's result" (R2): MDF4's raw channels and the Historian's blocks keep the wire value, and `dataset_carries_raw` gains its third case instead of a second rule. |
| Computed datasets | (a) decoder after the transform; (b) no decoder on computed datasets | **(a)**, as question 1; (b) forbids the status-word-in-a-table case for no gain. |
| Where labels apply | (a) at presentation through one formatter; (b) the pipeline writes the label into `value` and keeps `numericValue` | **(a).** R5 says presentation; (b) would put the label into the block's text column, where CSV, the Historian and the API's `value` would record it, and would need a per-sample lookup the constraints forbid. |
| Formatter home | (a) C++ `DataModel::ValueFormat` in `core/Core`, QML delegating to it; (b) keep `ValueFormat.js` and add hex/binary there too | **(a).** One implementation; the JS copy is already the only site honouring `displayFormat`, which is how the grid came to ignore it. |
| Exposing the label remotely | (a) additive `label` key beside `value`; (b) `value` becomes the label | **(a).** gRPC emits only `value` today, so (b) would take the number away from gRPC clients. |
| Hexadecimal and binary rendering | (a) `0x`/`0b` prefix, two's complement at the declared width, minimal digits otherwise; (b) bare digits | **(a).** A bare `10` is ambiguous in a grid that also shows decimals. |
| Catch-all row | (a) a `labelsDefault` string beside the list; (b) a flagged row in the list | **(a).** One optional text, no sentinel key, and the dialog shows it as a fixed last row. |
| R6's Modbus halves | (a) deliver as written: the parser publishes raw words, datasets carry Scale or Bit field (decoder after the table read), codegen and R14/R15 pins regenerated; (b) amend R6 to the DBC half | **(b)**, as question 3: the register's scale and bit already sit in a declarative spec line, the dataset keeps a `tableGet` transform either way, and (a) reworks pinned codegen for no visible change. |
| Benchmark coverage | (a) one ungated decoded row; (b) decoders inside the gated synthetic project | **(a).** The gates stay comparable with history; the row makes the cost visible without moving a calibrated tier. |

## Risks & mitigations

- **Hotpath cost.** One compare per dataset per frame when no decoder is set; measured by the
  new row otherwise. Mitigation: AC6 run before merge, the decoded row recorded in the spec.
- **A labelled value that is not an integer.** Rounded before lookup (R4); the plot keeps the
  exact number.
- **`api-schema.json` drift.** The snapshot needs a build; CI's `--check-snapshot --strict`
  fails until the maintainer regenerates it. Mitigation: the tasks phase makes it the last
  maintainer step before push.
- **The duplicate format list** in `ProjectEditor.cpp:762-768` and the guard in
  `EditorCommit.cpp:1018`. Mitigation: both edited in the same task; a ledger row names the
  duplicate for a later registry-driven cleanup.
- **The assistant corpus lint.** `project_basics.md` must name the new fields exactly as the
  registry spells them.
- **`tst_dbc_importer` is not built.** Its registration is commented out pending a dependency
  (the comment above its registration in `app/tests/CMakeLists.txt`, line 1601), and the comment names the codegen assertion set in
  `tests/scripts/test_cpp_regressions.py` (R15, R17) as the coverage that runs. Mitigation:
  AC4's DBC half lands as a new assertion in that set over `projectFromMessages` output, not
  in the dormant ctest.
- **The example baseline hash.** Editing `ecu_simulator.ssproj` changes its baseline file
  name; `test_property_registry.py` recaptures it on the pre-change build per its README row.
- **Silent-breakage classes** (`common-mistakes.md`): the `.toDouble()` rule (the decoder
  uses `SerialStudio::toDouble` output, never re-parses), the undo two-phase memento
  (`commitValueLabels` goes through `updateDataset`), the generated-file rule (never edit the
  six artifacts), the hotpath assert scope. None of the cached-flag, queued-hop or timestamp
  classes apply.

## Test & verification plan

- **C++ unit (maintainer builds, `ctest`):** `tst_value_decoder` (AC1: fixed point with
  signed 8/16/32 widths, full-width field, top bit, fraction bits 0 and width-1; validator
  refusals), `tst_value_format` (notation widths, labels, catch-all, rounding),
  `tst_pro_content` (AC5 unit half: formats and labels count zero), `tst_dbc_importer` (AC4
  DBC half), `tst_frame_builder_staging` unchanged (raw twin for a decoded column, one added
  case).
- **Integration (app up, API server on):** `tests/integration/test_dataset_formats.py`:
  AC2 loads a project with a labelled, a fixed-point and a bit-field dataset through
  `project.loadJson`, streams one frame through the device simulator, reads
  `dashboard.getData` and asserts `numericValue`, `label` and `isNumeric` true, and that the
  dataset is still `plt`; AC3 saves through `project.exportJson`, reloads and compares, and
  a project without the keys round-trips byte-identical; AC5's integration half asserts the
  connect gate's refusal reason is absent on a project with formats and no transform (on a
  licensed build the summary itself is asserted zero through `project.validate`).
- **Benchmark (maintainer):** `--benchmark-hotpath`; the nine gates unchanged, the
  `native(numeric, decoded)` row recorded (AC6).
- **Maintainer observation:** the regenerated CAN Bus example shows "Reverse" in the grid
  and the gear curve on the plot (AC7).
- **Static:** `scripts/syntax-check.py` on every C++ file; `code-verify.py --check`;
  `registry-verify.py` (manifest, `Keys::`, hooks, corpus lint); `generate-property-registry.py
  --check`; `documentation-verify.py` on the manual pages; `qt-cpp-review` before handoff;
  `sanitize-commit.py` before each commit.
