---
spec: 0106-quick-plot-multiformat-input
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-30
---

# Tasks 0106 — Quick Plot multiformat input

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
- **Executor** follows CLAUDE.md's advisor economics: `main` for design, hotpath and protected
  surfaces; `sonnet` for mechanical work against a fixed pattern, briefed in half a page.
- `scripts/syntax-check.py` runs on every edited C++ file through the hook; a SKIP means the
  compile claim is unverified and the task says so.
- The corpus is the 35 streams in `spec.md` "Corpus for AC1"; every test task reads it from
  there, with the three Czech-free English names as written.

## Tasks

### T1 — LineFormat: detector, tokenizer, number grammar

- **Files:** `core/Pipeline/DataModel/Scripting/LineFormat.h`, `LineFormat.cpp` (new),
  `core/Pipeline/CMakeLists.txt`
- **Does:** `LineFormat` (kind Delimited or Json, separator, decimalComma, named, wrapper),
  `LineField` (name, value and unit `QByteArrayView` spans, number, numeric and named flags),
  `LineFields` (bounded scratch, `kMaxLineFields = 512`), `stripWrapper`, `tokenizeLine`,
  `parseFieldNumber` (fast_float prefix, decimal comma through a 64-byte stack buffer, unit
  grammar: 1 to 8 bytes, no digit, space, `, ; : =`), `detectLineFormat` with the R9/R10
  scoring (candidates `;`, tab, `,`, space, single; first 1.0 wins, else best at or above 0.5;
  `{` tries the JSON scanner once T2 lands, stubbed to false until then). Hotpath invariants:
  spans into the caller's bytes, no allocation, every loop bounded by `kMaxLineFields` or the
  line length, `SS_ASSERT_HOTPATH` on per-field conditions, `SS_ASSERT` with recovery on
  device-derived ones.
- **Verify:** `python scripts/code-verify.py --check` on both files; syntax-check hook passes;
  no `QString`, `QList` or `new` in the tokenize path (read-back).
- **Deps:** none
- **Executor:** main
- [x] done

### T2 — JsonLeafScanner

- **Files:** `core/Pipeline/DataModel/Scripting/JsonLeafScanner.h`, `JsonLeafScanner.cpp`
  (new), `core/Pipeline/CMakeLists.txt`
- **Does:** Bounded recursive-descent scan of one line: objects, arrays, numbers (fast_float),
  strings with escapes skipped, `true`/`false`/`null` skipped; depth cap 8; emits numeric
  leaves into `LineFields` in text order with paths `a.b` and `a[0]` written into a per-call
  path arena (fixed size, overflow ends the scan as malformed); returns false on malformed
  input or zero leaves. Wire `detectLineFormat`'s `{` branch to it. Same hotpath invariants as
  T1.
- **Verify:** `code-verify.py --check`; syntax-check; read-back that the arena and the depth
  cap are the only bounds and both are asserted.
- **Deps:** T1
- **Executor:** main
- [x] done

### T3 — tst_line_format

- **Files:** `app/tests/tst_line_format.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** `ss_add_unit_test(tst_line_format SOURCES tst_line_format.cpp
  ${SS_CORE_SRC}/Pipeline/DataModel/Scripting/LineFormat.cpp
  ${SS_CORE_SRC}/Pipeline/DataModel/Scripting/JsonLeafScanner.cpp LIBS Qt6::Core
  SerialStudio::Core)`. Data-driven cases: every single-line corpus stream through
  `detectLineFormat` + `tokenizeLine` (format, field count, names, numbers, units); the number
  grammar edge cases (`+7`, `1.5e3`, `-12.5`, `23,5` under `;`, `0x1F` rejected, `12:30:45`
  rejected, unit longer than 8 bytes rejected); the scanner on nested, mixed-type and malformed
  JSON (`{"a":`, `{"a":[1,}`, depth 9) and on the `{"imu":...}` corpus line in text order.
- **Verify:** `code-verify.py --check`; syntax-check; maintainer builds the unit tier, then
  `ctest -R tst_line_format` against the build dir.
- **Deps:** T1, T2
- **Executor:** sonnet (brief: the corpus table, the API from T1/T2 headers, the
  `tst_quickplot_project.cpp` suite as the shape to copy)
- [x] done

### T4 — QuickPlotFormatState

- **Files:** `core/Pipeline/DataModel/FrameBuilder/QuickPlotFormatState.h`,
  `QuickPlotFormatState.cpp` (new), `core/Pipeline/CMakeLists.txt`
- **Does:** Pipeline-thread state with no Qt object and no bus or host include: `established`,
  the `LineFormat`, the channel registry (ordered entries that are a name or a position, each
  with a unit, plus the positional count), the header candidate, `observe(const LineFields&)`
  returning `Text`, `Header`, `Rebuild` or `Apply` per plan step 2 (including the R9
  re-evaluation on zero numeric fields and the R8 count-matched header acceptance),
  `indexOf(const LineField&)` with a last-position fast path and linear compare, `reset()`,
  `pinPositional(count)`. Invariants: registry append only, never `operator[]` insertion; the
  linear compare is bounded by the registry size; detection runs only inside the establishing
  and re-evaluation branches.
- **Verify:** `code-verify.py --check`; syntax-check; read-back that `observe` has exactly two
  call sites of `detectLineFormat`.
- **Deps:** T1, T2
- **Executor:** main
- [x] done

### T5 — tst_quickplot_format_state (AC1)

- **Files:** `app/tests/tst_quickplot_format_state.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Register with the three Scripting/FrameBuilder TUs from T1, T2, T4 and
  `SerialStudio::Core`. Feed all 35 corpus streams line by line through
  `detectLineFormat`/`tokenizeLine` + `observe`; assert per line the outcome and the
  (title, value) pairs via `indexOf`, the units captured at establishment, the header
  acceptance for the `Temperature,Pressure,Humidity` stream and its rejection for the
  `WiFi connected` stream, and 4 `Rebuild` outcomes after the first structure across the corpus.
  Add the replay pin: `pinPositional(3)` then a `temp:1;hum:2` line stays positional.
- **Verify:** `code-verify.py --check`; syntax-check; `ctest -R tst_quickplot_format_state`
  after the maintainer builds.
- **Deps:** T4
- **Executor:** sonnet (brief: the corpus table with expected outcomes, the T4 header)
- [x] done

### T6 — QuickPlotBuilder owns the state and writes values

- **Files:** `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.h`, `QuickPlotBuilder.cpp`
- **Does:** Add the `QuickPlotFormatState` member and forward `reset()`, `pinPositional()`,
  `observe()`, `format()`. `build()` takes no argument and reads the registry: title = name,
  header title or `tr("Channel %1")`, `units` from the registry entry; datagrid + multiplot
  layout unchanged. New `applyFields(const LineFields&)`: for each dataset, `indexOf` picks the
  field; present and numeric writes `numericValue`, `assign_utf8_in_place(value, span)`,
  `isNumeric = true`; present and text writes the span, NaN, `false`; absent writes NaN,
  `false`, value untouched (R11). `setHeaders` stays for the players; `buildAudio` calls
  `pinPositional(channels)`. Hotpath invariants: no allocation in `applyFields`
  (`assign_utf8_in_place`, never share-assign), `SS_ASSERT_HOTPATH` in the per-dataset loop,
  rebuild only through the caller's `invalidateFramePool()` + `build()`.
- **Verify:** `code-verify.py --check`; syntax-check; read-back that `applyFields` contains no
  `QString` construction and no `=` from a `QString` temporary.
- **Deps:** T4
- **Executor:** main
- [x] done

### T7 — FrameBuilder lane rewrite

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`, `FrameBuilder.h`
- **Does:** `parseQuickPlotFrame`: strip, then live frames tokenize through the state (detect
  when not established, else fixed), replay frames (`m_playerOpen`) go through
  `splitReplayChannels` and a new `fieldsFromReplayRow` adapter over a reused `QByteArray`
  scratch into `LineFields`; `captureLatestChannelSpans` from the value spans when
  `m_captureLatestFrame`; then `observe`: `Text` and `Header` return without staging,
  `Rebuild` runs `invalidateFramePool()` + `m_quickPlot.build()`, `Apply` runs
  `applyFields` + `m_stager.stage(...)` with `data->timestamp`. Replace `m_quickPlotChannels`
  with `m_quickPlot.reset()` in the ctor (135), `onOperationModeChanged` (1062),
  `onConnectedChanged` (1436) and with `pinPositional` in `publishQuickPlotAudioTemplate`
  (1134); remove `captureLatestChannels(int, const QStringList&)` if it has no other caller.
  Add `quickPlotFormat()` (pipeline thread). Invariants named at edit time: pipeline thread
  only, no new signal, no per-line allocation, `m_playerOpen` is the existing cached flag,
  timestamp passed through, the TU must end shorter than it started.
- **Verify:** `code-verify.py --check` on both files; syntax-check; `wc -l FrameBuilder.cpp`
  below 2911; `python scripts/code-verify.py --tu-census` excess not above the baseline.
- **Deps:** T6
- **Executor:** main
- [x] done

### T8 — Sample ring guard

- **Files:** `core/Ui/UI/Dashboard/DashboardIngest.cpp`
- **Does:** `push_ring_tail` skips a sample that is not finite, so a NaN "no sample" never
  reaches an `AxisData` queue or an FFT input. Invariant: one `std::isfinite` per sample on the
  sample-ring path only; the time-ring paths already guard in `DSP.h`.
- **Verify:** `code-verify.py --check`; syntax-check; this is the clean point for the
  maintainer's `--benchmark-hotpath` run (AC6).
- **Deps:** T7
- **Executor:** main
- [x] done

### T9 — Dry-run preview split

- **Files:** `core/Pipeline/DataModel/Scripting/FrameParserPipeline.cpp`, `FrameParserPipeline.h`
- **Does:** `splitQuickPlotChannels` becomes stateless: `stripWrapper`, `detectLineFormat`,
  then raw trimmed cells under the detected separator (a JSON line returns one cell per leaf as
  `path:value`); same signature, same one-row contract. Update the `@brief`.
- **Verify:** `code-verify.py --check`; syntax-check;
  `pytest tests/integration/test_frame_parsing.py -k quick_plot_needs_no_delimiters` still
  expects `[["1","2","3"]]` (maintainer, app up).
- **Deps:** T1, T2
- **Executor:** sonnet
- [x] done

### T10 — line_values Built-In template

- **Files:** `core/Pipeline/DataModel/Scripting/NativeTemplates/TextLineValues.h`,
  `TextLineValues.cpp` (new), `TextTemplates.cpp`, `core/Pipeline/CMakeLists.txt`
- **Does:** Template id `line_values`, label "Line Values"; params `format` Enum
  {delimited, json}, `separator` Enum {comma, semicolon, tab, space}, `decimalComma` Bool,
  `fields` String (ordered names or paths; empty = positional), `stripWrapper` Bool,
  `unitSuffix` Bool. Parser: positional delimited implements `parseSpans` over `tokenizeLine`
  (span lane, allocation-free, `-1` when a field needs normalizing); named and JSON extend
  `NativeLatchParser` with `fields` order, latching a missing key. Output cells are normalized
  numbers (unit stripped, decimal point), text fields verbatim. Register after `delimited` in
  `textNativeTemplates()`.
- **Verify:** `code-verify.py --check`; syntax-check; read-back that the parser calls
  `tokenizeLine`/`scanJsonLeaves` and contains no second tokenizer.
- **Deps:** T1, T2
- **Executor:** main
- [x] done

### T11 — Save as Project emits the matching template

- **Files:** `core/Pipeline/DataModel/Project/QuickPlotProject.h`, `QuickPlotProject.cpp`,
  `app/tests/tst_quickplot_project.cpp`
- **Does:** `build(groups, busType, settings, const LineFormat&)`. Plain positional comma with
  no decimal comma and no unit on any dataset emits today's `delimited` source byte for byte;
  anything else emits `line_values` with the matching params and `fields` from dataset titles
  in index order for named and JSON. Extend the suite: the existing cases pass a plain comma
  format and keep their assertions; new cases for `;` + decimal comma, named, JSON and wrapper
  check the emitted template id and params.
- **Verify:** `code-verify.py --check`; syntax-check; `ctest -R tst_quickplot_project` after
  the maintainer builds.
- **Deps:** T10
- **Executor:** main (generator), sonnet (test cases)
- [x] done

### T12 — Read the format in the save hop

- **Files:** `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:** `saveQuickPlotAsProject` copies `frameBuilder.quickPlotFormat()` inside the
  existing `invokeOnBuilderThreadBlocking` lambda beside the groups and passes it to `build`.
  Invariant: no second hop, no GUI-thread read of builder state.
- **Verify:** `code-verify.py --check`; syntax-check; read-back of the lambda.
- **Deps:** T7, T11
- **Executor:** main
- [x] done

### T13 — tst_quickplot_roundtrip (AC5)

- **Files:** `app/tests/tst_quickplot_roundtrip.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Register with the T1, T2, T4, T10 TUs, `QuickPlotProject.cpp`,
  `NativeTemplateRegistry.cpp` and `SerialStudio::Core` (add TUs until it links; the FrameBuilder
  itself stays out of the lean tier). For each single-line corpus format: run the live tokenizer
  and state to get (title, value) pairs; build groups from the registry the way the builder
  titles them; call `QuickPlotProject::build`; instantiate the emitted template by id with the
  emitted params through `nativeTemplateById`; parse the same line; assert the cells convert
  with `SerialStudio::toDouble` to the live values in index order. Complete lines only (R12's
  latch difference is documented, not tested here).
- **Verify:** `code-verify.py --check`; syntax-check; `ctest -R tst_quickplot_roundtrip`
  after the maintainer builds.
- **Deps:** T4, T10, T11
- **Executor:** sonnet (brief: the corpus table, the T1/T4/T10/T11 headers, the registry API)
- [x] done

### T14 — Retire the Quick Plot source-regex tests

- **Files:** `tests/scripts/test_cpp_regressions.py`
- **Does:** Remove `test_frame_builder_quick_plot_uses_shared_splitter` and
  `test_frame_parser_pipeline_dispatches_quick_plot_branch` (lines 935 to 983), whose own text
  names the new ctests as the retirement condition; keep the `splitReplayChannels` player test.
- **Verify:** `pytest tests/scripts/test_cpp_regressions.py -q` passes (I can run it).
- **Deps:** T7, T9
- **Executor:** sonnet
- [x] done

### T15 — Mode label, tooltip and API label

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`, `core/Core/EnumLabels.cpp`,
  `app/tests/tst_enum_labels.cpp`
- **Does:** Radio text "Quick Plot (Auto-Detected Format)" at line 236; tooltip at 239-240
  lists the shapes with one example each (`23.5, 60, 1013`, `temp:23.5, hum:60`,
  `23.5°C`, `{"t":23.5}`, separators comma, semicolon, tab or space). `EnumLabels.cpp:211`
  becomes "Quick Plot (auto-detected format)"; `tst_enum_labels.cpp:358` follows.
- **Verify:** `code-verify.py --check` on the three files; `python scripts/term-verify.py`;
  the `.ts` files are not touched (read `git status`).
- **Deps:** none
- **Executor:** sonnet
- [x] done

### T16 — CLI help and the no-data hint

- **Files:** `app/src/Misc/CLI.h`, `doc/help/Command-Line-Interface.md`,
  `core/Ui/Console/WelcomeText.cpp`
- **Does:** CLI.h 98-99 and the manual line 24: "auto-detected line format" instead of
  "auto-detect CSV data". `noDataHint()` (137-149) gains one clause: Quick Plot reads lines
  such as `23.5,60` or `temp:23.5`.
- **Verify:** `code-verify.py --check`; syntax-check on the two C++ files;
  `documentation-verify.py` on the manual page.
- **Deps:** none
- **Executor:** sonnet
- [x] done

### T17 — Command palette tooltip

- **Files:** `app/rcc/commands/app.json`, `core/Ui/UI/CommandStrings.cpp` (generated)
- **Does:** Line 544 tooltip "Switch to Quick Plot mode (auto-detected line format)". Then
  `python scripts/generate-command-strings.py`; never hand-edit the generated file.
- **Verify:** `python scripts/registry-verify.py`; `git diff` shows only the one string in
  both files.
- **Deps:** none
- **Executor:** sonnet
- [x] done

### T18 — API command descriptions

- **Files:** `core/Api/API/Handlers/ProjectFileCommands.cpp`,
  `core/Api/API/Handlers/ProjectDryRunCommands.cpp`, `app/rcc/api/api-schema.json` (generated)
- **Does:** 859-862 summary and 173-174 mode description state the auto-detected shapes
  (separators, named pairs, units, one-line JSON, wrapper). Then `python scripts/generate-sdk.py`
  for the schema; never hand-edit it.
- **Verify:** `code-verify.py --check`; syntax-check; `git diff app/rcc/api/api-schema.json`
  shows only description text.
- **Deps:** T9
- **Executor:** sonnet
- [ ] done (C++ strings landed; `api-schema.json` is dumped by the built app with `--dump-api-schema`, then `scripts/generate-sdk.py`: maintainer step after the build)

### T19 — Operation Modes and Getting Started pages

- **Files:** `doc/help/Operation-Modes.md`, `doc/help/Getting-Started.md`
- **Does:** Via `ss-docs`. Operation Modes: the "How it works", "Example input",
  "Limitations" and comparison-table rows describe the accepted shapes, the once-per-connect
  detection, text lines going to the console, and the header-row rule as clarified (R8); the
  old label at line 68 changes. Getting Started: the primary Quick Plot sketch prints
  `temp:` and `hum:` pairs (R14), the comma and header-row shapes stay as secondary examples,
  lines 107, 128, 137, 141, 143, 190 and 211 lose the comma-only claims.
- **Verify:** `python scripts/documentation-verify.py doc/help/Operation-Modes.md
  doc/help/Getting-Started.md`; `term-verify.py`; `claim-verify.py`.
- **Deps:** T7 (so the text describes landed behavior)
- **Executor:** sonnet via `ss-docs`
- [x] done

### T20 — Remaining help-page claims

- **Files:** `doc/help/FAQ.md`, `Concepts.md`, `Glossary.md`, `Toolbar-Reference.md`,
  `Data-Flow.md`, `Data-Hotpath.md`, `Drivers-MQTT.md`, `Use-Cases.md`, `API-Reference.md`
- **Does:** Via `ss-docs`, one sentence each at the lines in the plan's surface census: the
  comma-only claim becomes "numbers per line in a detected format". Nine files, one-line edits
  each; split further only if a page needs a new paragraph.
- **Verify:** `documentation-verify.py` on the nine files; `term-verify.py`; `claim-verify.py`;
  `git diff --stat` shows one or two lines per file.
- **Deps:** T19
- **Executor:** sonnet via `ss-docs`
- [x] done

### T21 — README and example README

- **Files:** `README.md`, `examples/NI DAQmx/README.md`
- **Does:** README lines 28, 91 and 146 say "print numbers, one line per sample" instead of
  "CSV / comma-separated"; the NI DAQmx README line 34 carries the new label.
- **Verify:** `documentation-verify.py README.md "examples/NI DAQmx/README.md"`;
  `term-verify.py`.
- **Deps:** none
- **Executor:** sonnet
- [x] done

### T22 — Assistant corpus

- **Files:** `app/rcc/ai/skills/project_basics.md`, `app/rcc/ai/search_index.json` (generated)
- **Does:** Line 115 describes Quick Plot's detected shapes. Then
  `python app/rcc/ai/build_search_index.py` so the index picks up this file and the T21 README
  change; never hand-edit the index.
- **Verify:** `git diff --stat` shows the skill file plus the regenerated index only;
  `grep -c "Comma Separated Values" app/rcc/ai/search_index.json` is 0.
- **Deps:** T21
- **Executor:** sonnet
- [x] done

### T23 — Architecture doc and CLAUDE.md row

- **Files:** `doc/claude/architecture/dataflow.md`, `CLAUDE.md`
- **Does:** dataflow.md gains a "Quick Plot lane" subsection: the four outcomes, the shared
  tokenizer, the replay pin, the no-sample NaN contract and where each lives. CLAUDE.md gains one
  Subsystem Contracts row: touching Quick Plot parsing or Save as Project reads dataflow.md
  "Quick Plot lane"; hazard: a second tokenizer makes the saved project diverge from the live
  parse. Check `scripts/context-budget.json` after; trim an older row's wording if the budget
  trips.
- **Verify:** `python scripts/claim-verify.py` (every path and symbol in the new text
  resolves); `documentation-verify.py` on both; the budget check in `code-verify.py --check`.
- **Deps:** T7, T11
- **Executor:** main
- [x] done

### T24 — Integration tests for AC2, AC3, AC4

- **Files:** `tests/integration/test_quick_plot_formats.py` (new)
- **Does:** Using `api_client.set_operation_mode("quickplot")`, `configure_network`,
  `connect_device` and `device_simulator.send_frame` as in `test_csv_separator_detection.py`:
  `test_named_lines_label_without_header` (lines sent before connect, titles `temp` and
  `hum` in `dashboard.getData`), `test_unit_suffix_reaches_dataset` (units `°C`, `hPa`),
  `test_text_line_leaves_structure` (`datasetCount` and dataset `uniqueId`s unchanged across a
  `WiFi reconnecting...` line, `dashboard.tailFrames` continuous), and one dry-run preview per
  shape through `project.frameParser.dryRun` with `operationMode: 2`. Register no new marker;
  reuse the existing integration fixtures.
- **Verify:** `python -m py_compile`; the maintainer runs `pytest tests/integration/test_quick_plot_formats.py -v`
  with the app up and the API server on.
- **Deps:** T7, T9
- **Executor:** sonnet (brief: the three AC texts, the fixture names from
  `test_csv_separator_detection.py`, the API field names from `DashboardHandler.cpp`)
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1, AC5 by ctest;
      AC2 to AC4, AC6, AC8 by the maintainer's runs; AC7 by the gates).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python scripts/code-verify.py --tu-census` excess is not above the baseline; no
      `--accept`.
- [x] `qt-cpp-review` run on the new TUs (T1, T2, T4, T10) and the T6/T7 diff; findings
      addressed or noted.
- [ ] `ss-hotpath` rules re-read against the T6/T7/T8 diff; the maintainer's
      `--benchmark-hotpath --min-fps 256000` is green after T8.
- [ ] Generated artifacts (CommandStrings.cpp and search_index.json regenerated; api-schema.json waits for the built app's `--dump-api-schema`) (`CommandStrings.cpp`, `api-schema.json`, `search_index.json`) were
      regenerated by their scripts, never hand-edited.
- [x] `app/translations/ts/*.ts` and the 0104 working-tree files are untouched.
- [x] `python scripts/term-verify.py`, `claim-verify.py`, `documentation-verify.py`,
      `registry-verify.py` and `layer-verify.py` pass.
- [x] Relevant `pytest` tests identified for the maintainer to run (listed in `plan.md`).
- [ ] `python scripts/sanitize-commit.py` run (clang-format applied to every file of this diff and `--check-format` reports no drift; the full sanitizer walks the tree and would touch the 0104 files, so it is left for commit time); working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
