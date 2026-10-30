---
spec: 0106-quick-plot-multiformat-input
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-30
---

# Plan 0106 — Quick Plot multiformat input

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

One tokenizer, two callers. A new pure module, `LineFormat`, detects a line's shape and
tokenizes a line into a bounded array of field spans (name, number, unit), and a bounded
one-line `JsonLeafScanner` emits numeric leaves in text order with dotted and indexed paths.
The live Quick Plot lane drives both through a new pipeline-thread state object,
`QuickPlotFormatState` (the established format, the channel registry keyed by name, the header
candidate, the failure-triggered re-evaluation), owned by `QuickPlotBuilder`, which builds
titles and units from the registry and writes per-frame values. A new Built-In template,
`line_values`, wraps the same tokenizer with a fixed format, so Save as Project emits a project
that parses identically by construction; plain comma streams keep emitting `delimited`.
`FrameBuilder.cpp` only shrinks: its Quick Plot function becomes strip, tokenize, capture, hand
to the builder, stage. Replay keeps its comma-row codec and feeds the builder through a pinned
positional format, so playback is untouched.

## Affected subsystems & files

Confirmed by grep on 2026-10-30. Line numbers are the current tree.

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/Scripting/LineFormat.h/.cpp` | **New.** `LineFormat` (kind Delimited or Json, separator, decimalComma, named, wrapper), `LineField` (name, value and unit spans, number, numeric and named flags), `LineFields` (bounded scratch, `kMaxLineFields = 512`), `stripWrapper`, `detectLineFormat` (R1, R2, R9, R10 scoring), `tokenizeLine`, `parseFieldNumber` (fast_float prefix, decimal comma through a 64-byte stack buffer, unit grammar). Pure functions, allocation-free. |
| `core/Pipeline/DataModel/Scripting/JsonLeafScanner.h/.cpp` | **New.** Bounded recursive-descent scanner over one line (depth cap 8, field cap from `LineFields`): numeric leaves in text order, paths `imu.ax` and `acc[0]` written into a per-call path arena; strings, booleans and nulls skipped; malformed input returns false. Shared by the live lane and the template. |
| `core/Pipeline/DataModel/FrameBuilder/QuickPlotFormatState.h/.cpp` | **New.** Pipeline-thread state with no Qt object and no bus reach, so it links lean in a ctest: `established`, the `LineFormat`, the channel registry (ordered names or positions, units, positional count), the header candidate, `observe(fields)` returning `Text`, `Header`, `Rebuild` or `Apply`, `indexOf(field)` with a last-position fast path, `reset()`, `pinPositional(count)` for replay and audio. |
| `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.h/.cpp` | Owns the state. `build()` reads the registry (title = name, header or "Channel N"; `units` from the establishing line). New `applyFields(const LineFields&)` writes `value` in place, `numericValue` and `isNumeric` per dataset; a missing or text field writes NaN with `isNumeric = false`. `setHeaders` keeps the player API; `buildAudio` pins the state positional. |
| `core/Pipeline/DataModel/FrameBuilder.cpp/.h` | `parseQuickPlotFrame` shrinks: live frames go through the state; replay frames go through `splitReplayChannels` and a `fieldsFromReplayRow` adapter over a reused UTF-8 scratch; `captureLatestChannelSpans` replaces `captureLatestChannels`; then `applyFields` and `m_stager.stage`. `m_quickPlotChannels` is replaced by `m_quickPlot.reset()` at the three sites (ctor 135, `onOperationModeChanged` 1062, `onConnectedChanged` 1436) and `publishQuickPlotAudioTemplate` 1134. New `quickPlotFormat()` accessor. Net line count goes down (TU census has 14 lines of headroom). |
| `core/Pipeline/DataModel/Scripting/FrameParserPipeline.cpp/.h` | `splitQuickPlotChannels` becomes a stateless detect-and-tokenize preview for the dry-run API: raw trimmed cells under the detected separator, wrapper stripped. Signature unchanged. |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/TextLineValues.h/.cpp` | **New** template, id `line_values`, label "Line Values". Params: `format` Enum {delimited, json}, `separator` Enum {comma, semicolon, tab, space}, `decimalComma` Bool, `fields` String (ordered names or paths, empty = positional), `stripWrapper` Bool, `unitSuffix` Bool. `parseSpans` for positional delimited (the span lane), latching `parseText` on `NativeLatchParser` for named and JSON. Output cells are normalized numbers (unit stripped, decimal point). |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/TextTemplates.cpp:45` | Register `line_values` after `delimited`. |
| `core/Pipeline/DataModel/Project/QuickPlotProject.h/.cpp` | `build(groups, busType, settings, const LineFormat&)`. Plain positional comma without decimal comma or units keeps today's `delimited` source verbatim; every other format emits `line_values` with the matching params, `fields` taken from dataset titles in index order for named and JSON streams. |
| `core/Pipeline/DataModel/ProjectModel.cpp:434` | `saveQuickPlotAsProject` reads the format in the same `invokeOnBuilderThreadBlocking` hop that copies the groups, and passes it to `build`. (`ProjectWorkspaces.cpp:886` reads groups only and does not change.) |
| `core/Pipeline/CMakeLists.txt` | Add the five new translation units beside lines 63, 105 and 152. |
| `core/Ui/UI/Dashboard/DashboardIngest.cpp:83` | `push_ring_tail` skips non-finite samples. The time rings already do (`DSP.h:289`, `686`, `874`); `AxisData` is a `FixedQueue<ssfp_t>` with no guard. |
| `app/qml/MainWindow/Panes/Setup.qml:236-240` | Label "Quick Plot (Auto-Detected Format)" and a tooltip listing the shapes with one example. |
| `core/Core/EnumLabels.cpp:211`, `app/tests/tst_enum_labels.cpp:358` | API label "Quick Plot (auto CSV plotting)" becomes "Quick Plot (auto-detected format)". |
| `app/src/Misc/CLI.h:98-99`, `doc/help/Command-Line-Interface.md:24` | "auto-detect CSV data" becomes "auto-detected line format". |
| `app/rcc/commands/app.json:544` | Palette tooltip; then regenerate `core/Ui/UI/CommandStrings.cpp` with `scripts/generate-command-strings.py` (never hand-edit). |
| `core/Api/API/Handlers/ProjectFileCommands.cpp:859-862`, `ProjectDryRunCommands.cpp:173-174` | Command descriptions; then regenerate `app/rcc/api/api-schema.json` with `scripts/generate-sdk.py`. |
| `core/Ui/Console/WelcomeText.cpp:137-149` | `noDataHint()` gains one clause with a `name:value` example. |
| `app/rcc/ai/skills/project_basics.md:115` | Quick Plot line; then regenerate `app/rcc/ai/search_index.json` with `app/rcc/ai/build_search_index.py`. |
| `doc/help/Operation-Modes.md`, `Getting-Started.md`, `FAQ.md`, `Concepts.md`, `Glossary.md`, `Toolbar-Reference.md`, `Data-Flow.md`, `Data-Hotpath.md`, `Drivers-MQTT.md`, `Use-Cases.md`, `API-Reference.md:3686` | The comma-only claims listed in the surface census; Getting Started leads with the `name:value` sketch (R14). Via the `ss-docs` skill. |
| `README.md:28,91,146`, `examples/NI DAQmx/README.md:34` | Comma wording and the old label. |
| `doc/claude/architecture/dataflow.md` | New "Quick Plot lane" subsection: the state machine, the shared tokenizer, the replay pin. The doc has no Quick Plot coverage today. |
| `CLAUDE.md` | One Subsystem Contracts row (hazard: a second tokenizer makes Save as Project diverge). Check `scripts/context-budget.json` after. |
| `app/tests/tst_line_format.cpp` | **New.** Detector, tokenizer, number grammar, JSON scanner; corpus-driven plus malformed input. |
| `app/tests/tst_quickplot_format_state.cpp` | **New.** The 35-stream corpus through the state machine: outcomes, titles, units, 4 rebuilds (AC1). |
| `app/tests/tst_quickplot_roundtrip.cpp` | **New.** Per corpus format: `QuickPlotProject::build`, instantiate the emitted template by id with the emitted params, feed the same lines, compare rows to the live values (AC5). |
| `app/tests/tst_quickplot_project.cpp`, `app/tests/CMakeLists.txt` | Format parameter; register the three suites with `ss_add_unit_test`. |
| `tests/scripts/test_cpp_regressions.py:935-983` | Retire the two Quick Plot source-regex tests; their own text names the new ctests as the retirement condition. |
| `tests/integration/test_quick_plot_formats.py` | **New** (maintainer runs): AC2, AC3, AC4 and dry-run previews over live TCP. |
| `app/translations/ts/*.ts` | **Not touched.** 21 files carry the old label; the maintainer regenerates with lupdate. |

## Architecture & data flow

Everything below runs on the pipeline thread inside `FrameBuilder::parseQuickPlotFrame`.
`FrameReader` is untouched: a frame is one line (CR, LF, CRLF, `AppState::deriveFrameConfig`).

1. **Strip and tokenize.** `stripWrapper` removes a `/* */` pair. Live: when the state is not
   established, `detectLineFormat` scores the candidates in order `;` (decimal comma allowed),
   tab, `,`, space (decimal comma allowed), single field; a candidate's score is the fraction of
   fields that are numeric or named-numeric; the first candidate scoring 1.0 wins, otherwise
   the best candidate at or above 0.5; a line starting with `{` that the scanner accepts with at
   least one leaf is JSON. Below 0.5 the line is text (R10). Once established, `tokenizeLine`
   runs with the fixed format and no scoring.
2. **Observe.** `QuickPlotFormatState::observe(fields)` yields `Text` (zero numeric fields and
   no qualifying re-detection), `Header` (not established, all fields text: stored as the
   candidate, replacing an earlier one), `Rebuild` (a positional count change, a new name or a
   new JSON path, or the establishing line itself) or `Apply`. The re-evaluation of R9 is inside
   `observe`: a line with zero numeric fields under the fixed format runs `detectLineFormat`
   once; if that qualifies, the state switches format, clears the registry and returns
   `Rebuild`. Replay (`m_playerOpen`) and audio pin a positional format, so `observe` never
   detects there.
3. **Act.** `Text`: capture the latest channels for the API, return without staging (R7).
   `Header`: return. `Rebuild`: `invalidateFramePool()`, `m_quickPlot.build()`, then fall
   through. `Apply`: `applyFields` writes each dataset from its field (`indexOf` maps a named
   field to its channel, position otherwise), NaN and `isNumeric = false` for a missing or text
   field (R11), then `m_stager.stage(...)` with the driver timestamp.
4. **Registry.** Positional channels are positions; named channels are names in order of
   first appearance; mixed lines give unnamed fields positional slots titled "Channel N". The
   header candidate is accepted at establishment only when its field count under the detected
   separator equals the positional count (R8); names on data lines win over header titles.
   Units are read from the establishing line and never re-read (R4).
5. **Save as Project.** `saveQuickPlotAsProject` copies groups and format in its existing
   blocking hop; `QuickPlotProject::build` emits `delimited` for plain comma positional
   streams and `line_values` otherwise. The template re-tokenizes with the fixed format, so the
   saved project's values equal the live ones by construction; a missing key latches, as the
   other key-addressed templates do (R12).
6. **Dry run.** `project.frameParser.dryRun` with `operationMode: 2` keeps returning rows of
   cells, now split under the detected separator with the wrapper removed. `test_frame_parsing.py:1271`
   (`"1,2,3\n"`) is unchanged.
7. **Replay.** The CSV player keeps emitting comma rows (`RowCodec::quickPlotPayload`), the
   Historian player keeps `replayBlock`; `fieldsFromReplayRow` converts the quote-aware cells
   into `LineFields` through a reused UTF-8 scratch, so one `applyFields` serves both lanes.

## Hotpath & threading impact

- **Touches the hotpath? Yes.** The Quick Plot lane of `FrameBuilder` and one dashboard
  ingest helper. `ss-hotpath` was invoked; `QuickPlotBuilder.cpp`, `QuickPlotProject.cpp`,
  the Quick Plot regions of `FrameBuilder.cpp` and `DashboardIngest.cpp` were read. The binding
  rules and how they hold:
  - **No per-line allocation.** Fields are `QByteArrayView` spans into the frame bytes held in
    a `LineFields` scratch allocated once (512 fields, about 24 KB); numbers parse with
    fast_float on the span; a decimal comma is rewritten in a 64-byte stack buffer; dataset
    strings are written with `assign_utf8_in_place`. Today's lane builds a `QStringList` per
    line, so the common comma path allocates less than before.
  - **Detection is not per line.** Scoring runs at establishment and on the R9 failure
    trigger only, at most five passes over one line. The registry lookup is a linear compare
    with a last-position fast path.
  - **Rebuild discipline unchanged.** Every structure change goes through the existing
    `invalidateFramePool()` + `build()` pair, so `structureGeneration` stamps stay correct.
  - **Bounded loops, hotpath asserts.** Field loops are capped by `kMaxLineFields`; the JSON
    scanner caps depth at 8; per-field conditions use `SS_ASSERT_HOTPATH`, device-derived
    conditions keep `SS_ASSERT` with a recovery.
  - **Benchmark.** `--benchmark-hotpath` never reaches `parseQuickPlotFrame` (it drives
    Project File parsers), so the nine gates measure only the `push_ring_tail` guard, one
    `std::isfinite` per sample on the sample-ring path of `lua+dashboard`. The gate must stay
    green; no new gate is added.
- **New cross-thread signal/slot? No.** `quickPlotFormat()` is read inside the existing
  `invokeOnBuilderThreadBlocking` hop of `saveQuickPlotAsProject`; `registerQuickPlotHeaders`
  keeps its self-marshal.
- **New input to a cached hotpath flag? No.** `m_playerOpen` (existing `Cached<bool>`) selects
  the pinned replay format; `m_captureLatestFrame` gates the capture as today.
- **Timestamp ownership.** `data->timestamp` from the driver boundary is passed to the stager
  unchanged.

## Data model & persistence

- No `Frame.h` key changes. `Dataset.title`, `Dataset.units` and the positional
  `Dataset.index` carry everything; key-addressed templates map key to position through their
  ordered `fields` param, as `key_value` and `json_data` do today.
- Project JSON: a Quick Plot session saved from a non-comma stream carries
  `frameParserTemplate: "line_values"` and its params. The loader resolves templates by id, so
  nothing else changes. Forward-only: an older build opening such a project falls back to the
  registry default (`delimited`, comma) and shows misaligned channels. Acceptable for a new
  template; the release notes say so.
- Sessions DB, MDF4, CSV export: untouched. Replay reproduces the live structure because the
  player path is pinned positional (AC8).

## API / SDK surface

- No new command, so no `CommandCapabilities`, `SealedPolicy` or tier entries.
- `project.frameParser.dryRun` (`operationMode: 2`) splits under the detected separator;
  description text at `ProjectDryRunCommands.cpp:173-174` changes and `api-schema.json` is
  regenerated. `ProjectFileCommands.cpp:859-862` summary text changes.
- `EnumLabels::operationModeLabel` for Quick Plot becomes "Quick Plot (auto-detected format)";
  `tst_enum_labels` follows.
- SDK: regenerated from the registry; no field changes.

## QML / UI

- `Setup.qml`: label and tooltip strings only. No new control (the spec rules out a manual
  override).
- Project Editor: the `line_values` template appears in the template list with a form generated
  from `params()`; no editor code changes.
- Command palette: tooltip text in `app.json` and the regenerated `CommandStrings.cpp`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the parse logic lives | (A) one shared tokenizer, live lane plus a new `line_values` template; (B) extend `delimited`, `key_value` and `json_data` and pick one at save time; (C) Save as Project emits a generated JS parser | **A.** R12 holds by construction with one tokenizer; B changes three templates' behavior (a spec non-goal) and leaves R12 to tests; C ships user code and runs at JS speed. |
| Per-line field representation | spans into the frame bytes; `QStringList` as today | **Spans.** Allocation-free, matches the span lane, and the comma path gets cheaper. |
| JSON parsing | bounded hand-rolled scanner; `QJsonDocument` | **Scanner.** `QJsonObject` sorts keys, so device order needs a text scan anyway; the scanner is allocation-free per line and shared with the template. |
| A missing named field (R11) | NaN with `isNumeric = false`; repeat the last value; a new "absent" marker in `DataBlock` | **NaN.** Time rings already skip non-finite samples, the data grid keeps the last string, and one guard covers the sample ring; a block-layout change touches every sink. |
| Save as Project for a plain comma stream | keep `delimited`; always `line_values` | **Keep `delimited`.** Existing users' saved projects do not change. |
| Header candidate | accept only when its field count matches the establishing line; last all-text line wins (today) | **Count match.** Today a `WiFi connected` line before data titles channel 1; the match rule drops it at no per-line cost. |
| Dry-run output | raw cells under the detected separator; structured fields | **Raw cells.** No API shape change; the preview still shows what Quick Plot splits. |
| Replay lane | pinned positional comma through an adapter; run detection on replay rows | **Pinned.** Playback behavior is a spec constraint; one `applyFields` serves both lanes. |
| Field cap | 512; 128 (`kMaxSpanFields`) | **512.** CSV replays wider than the span lane keep working; the scratch is about 24 KB once. |

## Risks & mitigations

- **Mis-detection on a real device.** The corpus is 35 streams, not the world. The R9
  re-evaluation and Save as Project are the escape hatches; every report adds a corpus line and
  a `tst_quickplot_format_state` case.
- **Allocation creeping back onto the lane.** Spans and in-place writes are the rule; review
  against the ss-hotpath checklist, `code-verify.py --check` on the hotpath TUs, and the
  alloc-stats build is the maintainer's spot check. The lane is not under a benchmark gate, so
  review carries this one.
- **Rebuild churn on a named stream that keeps inventing keys.** Bounded by registry growth,
  once per new name; arrays that grow then settle rebuild a few times (spec R5 accepts it).
- **`FrameBuilder.cpp` over the TU census.** The baseline leaves 14 lines of headroom; the
  Quick Plot function must shrink, which the sub-object split guarantees. Check
  `code-verify.py --tu-census` before handoff; no `--accept`.
- **Sample rings fed NaN.** `push_ring_tail` guard; FFT inputs never see a NaN.
- **Generated artifacts hand-edited.** `CommandStrings.cpp`, `api-schema.json` and
  `search_index.json` are regenerated by their scripts, never edited.
- **Translations.** 21 `.ts` files hold the old label; the maintainer runs lupdate. Not touched
  here (Trust Contract).
- **Source-regex tests.** `test_cpp_regressions.py:935-983` fails once the lane stops calling
  `splitQuickPlotChannels`; retired as its own text prescribes.
- **Silent-breakage classes** (`common-mistakes.md`): no queued hop added; timestamp captured at
  the driver and passed through; the registry uses explicit append, never `operator[]`
  insertion; no new cached flag.

## Test & verification plan

| Criterion | Check | Who runs |
|-----------|-------|----------|
| AC1 corpus | `ctest -R tst_quickplot_format_state` (35 streams, outcomes, titles, units, 4 rebuilds) and `tst_line_format` (detector, grammar, JSON scanner, malformed input) | maintainer builds; I run `ctest` against the build dir |
| AC2 late Connect | `tests/integration/test_quick_plot_formats.py::test_named_lines_label_without_header`: TCP device sends `temp: 23.5, hum: 60` before connect; `dashboard.getData` titles are `temp`, `hum` | maintainer (app up, API on) |
| AC3 units | same file, `test_unit_suffix_reaches_dataset`: `temp:23.5°C;pressure:1013hPa`; `dashboard.getData` units `°C`, `hPa` | maintainer |
| AC4 inert text line | same file, `test_text_line_leaves_structure`: send data, `WiFi reconnecting...`, data; `datasetCount` and dataset `uniqueId`s unchanged, `dashboard.tailFrames` continuous; maintainer also watches for flicker | maintainer |
| AC5 round-trip | `ctest -R tst_quickplot_roundtrip`: per format, build the project, instantiate the emitted template with its params, parse the corpus lines, compare to the live lane | maintainer builds; I run |
| AC6 gate | `serial-studio-pro --headless --benchmark-hotpath --min-fps 256000` green; `push_ring_tail` is the only touched row | maintainer |
| AC7 surfaces | `python scripts/term-verify.py`, `claim-verify.py`, `documentation-verify.py`, `registry-verify.py`; the surface census list is the checklist | me |
| AC8 replay | existing `tst_replay_row_codec`, `tst_csv_row_codec`, `tst_csv_rows`, `tests/integration/test_csv_separator_detection.py`, `test_csv_player.py` stay green | maintainer / me for ctest |
| Static | `scripts/syntax-check.py` on every edited C++ file (hook), `code-verify.py --check` on all edited files, `code-verify.py --tu-census` not above baseline, `qt-cpp-review` on the new TUs before handoff, `sanitize-commit.py` before any commit | me |
| Unit I can run now | `pytest tests/scripts/test_cpp_regressions.py` after the retirement edit | me |
