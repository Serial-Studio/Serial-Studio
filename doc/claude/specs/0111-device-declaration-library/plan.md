---
spec: 0111-device-declaration-library
phase: plan
status: approved     # approved 2026-11-03 (maintainer): amendments 1-9, every recommended answer, R14 kept as M8 after the text-line milestones
updated: 2026-11-03
---

# Plan 0111 — Device Declarations and the Firmware Library

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

A declaration is a **sparse project document on one line**: `#ss1` followed by compact JSON
whose keys are the project file's own keys (`title`, `groups`, `datasets`, `units`, `widget`,
`plotMin`, `alarmBands`, ...). The app reads it with the readers a project load already uses
(`DataModel::read(Group&)`, the generated `DataModel::read(Dataset&)`), behind an allow-list
**generated from the registry**: every property in `dataset.json` gains a required `wire` entry
(included, or excluded with a reason), and a small `declaration.json` names the envelope keys
(frame, group, source, control) as existing `Keys::` constants. The same generator writes the
app's allow-list header and the three libraries' vocabulary files, so the wire, the libraries and
the editor cannot drift (R1, AC9). In Quick Plot the pipeline intercepts the `#ss` prefix before
spec 0106's detector, parses at most once per content change, and pins the 0106 channel registry
to the declared datasets, so frames keep the one tokenizer and Save as Project keeps
`line_values` (R2, R5, R9). In Project File mode a GUI-thread raw-byte tap sees the line, compares
it to the loaded project and offers the declared project (R3). The libraries use a
**declare-in-loop** API: the same `channel()` call that sends a value also declares it, and the
library emits the declaration instead of a frame when one is due (at boot, or on the app's
`#ss?` request). Binary frames (R14) need a new `packed_struct` Built-In template and a
session-scoped framing override. They form their own last milestone.

## Spec amendment candidates (ground truth contradicts the spec as written)

1. **AC8 cannot hold against the current release.** v4.2.1 (tagged 2026-10-28) predates spec
   0106 (landed 2026-10-30). Its Quick Plot lane rebuilds on any comma-count change and takes an
   all-text first line as channel titles (`git show v4.2.1:core/Pipeline/DataModel/FrameBuilder.cpp`,
   lines 2580-2602). No line shape survives that. **Proposed:** AC8 and R11 name "the first
   release carrying spec 0106". The inertness ctest proves the claim against 0106's grammar by
   construction, and the library README names the minimum app version.
2. **R3 says an adopt is a load the user can undo.** A project load clears the undo history
   (`ProjectLoader.cpp:723`). **Proposed:** adopting opens the declared project as a new,
   unsaved project and leaves the loaded file untouched on disk, so reopening the file is the
   undo. Alternative: an undoable whole-document replace inside a `ProjectUndoScope`. That
   follows the `applyHistorySnapshot` shape but touches the spec-0031 undo machinery.
3. **R14 assumes a Built-In binary layout vocabulary that does not exist.** Two gaps:
   - No Built-In template decodes a packed struct of mixed types or floats. `raw_bytes` has
     uniform-width integer fields only (`BinaryRaw.cpp:113-136`), and `base64_encoded` takes no
     params at all (`BinaryBase64.cpp:104`).
   - Quick Plot's reader frames on line ends only and drops any checksum outside Project File
     mode (`FrameReader.cpp:148-150`, `:306-313`; `AppState.cpp:299-304`).

   As written, R14 needs a new `packed_struct` template and a session-scoped framing override.
   Also, inside a binary session a re-declaration (R4) is only visible to a GUI-thread sniffer,
   not to the pipeline. **Cheaper alternative:** carry packed frames as base64 text lines. That
   costs 1.33x bandwidth, but it needs no framing override and every R2-R11 mechanism stays
   identical. The new template is needed either way. M8 below designs the as-written version.
4. **R1 "dataset order is frame order" cannot express shared channels.** HexadecimalADC, one of
   the premise examples, shows the same six channels in two groups (indices 1-6 twice).
   **Proposed:** `index` (a registry property) is allowed on the wire. A dataset without it
   takes the next frame position; a dataset with it reuses that position.
5. **There is no group, output-control or source registry.** Only `dataset.json` exists (spec
   0036 is `"entity": "Dataset"`). The envelope vocabulary therefore comes from
   `declaration.json`, which names existing `Keys::` constants and generates into the same
   tables. It defines no new properties.
6. **R7 lists units and a numeric text field, and spec 0109 supplies both.** `OutputWidget` has
   no units field (`Frame.h:182-208`), and the text field has no Number kind. Version 1 declares
   button, toggle, slider and knob. The text field and units follow 0109 (M9).
7. **AC1's Save as Project half has no API path.** Save as Project runs through a file dialog
   (`ProjectModel.cpp:449-455`), and no command returns the Quick Plot project. **Proposed:** a
   ctest round trip, the `tst_quickplot_roundtrip` pattern of spec 0106. The alternative is a
   read-only `project.quickPlot.exportJson` verb.
8. **AC5 has nothing to drive yet.** Output controls set no `objectName`, and there is no
   set-value command. Spec 0109's plan (draft, 2026-11-03) names each cell
   `dashboard.output.<groupId>.<widgetId>`, with `.slider`, `.knob` and `.input` suffixes, and
   gives the slider arrow-key steps. AC5 reuses both. If 0111 lands first, M5 adds the same
   names. **Provisional on 0109.**
9. **R13: the Arduino Library Manager requires `library.properties` at the repository root**
   (arduino/library-registry FAQ). CI therefore publishes `firmware/arduino/` to a mirror
   repository and tags it there. The maintainer creates and registers that repository.

## Milestones

Each milestone ships alone and has its own file set. A few files are shared across milestones:
the registration lists `core/Pipeline/CMakeLists.txt`, `app/tests/CMakeLists.txt` and
`app/rcc/rcc.qrc`, plus `REUSE.toml`, `.github/workflows/ci.yml`, `doc/help/help.json`,
`examples/examples.json`, `CLAUDE.md`, `scripts/term-rules.json` and
`app/rcc/properties/declaration.json`. **The integrator owns these**, and each executor brief
lists the one-line additions it needs there.

| # | Milestone | Requirements / ACs | Depends on | Executor donor |
|---|-----------|-------------------|------------|----------------|
| M1 | Wire vocabulary in the registry, generator, gate | R1 (vocabulary), R11 (prefix/version), AC9 | none; order-free with 0108 (whichever lands second adds `wire` to 0108's properties; the gate forces it) | `render_api` / `check_manifest_keys` in the same scripts |
| M2 | Declared Quick Plot session (text), Save as Project | R1, R2, R4, R5, R9, R10, R11; AC2, AC7, AC8 (unit), AC1 (save half) | M1 | `QuickPlotFormatState::pinPositional`, `QuickPlotProject.cpp`, `tst_quickplot_roundtrip.cpp` |
| M3 | Libraries: Arduino header, C99 file, Python module (declaration, frames, `#ss?`, `#ss!` parsing) | R6 (device half), R7 (device half), R8, R11 (encoder) | M1; M2's golden fixtures are the contract. Sugar for formats and labels after 0108 | `tests/utils/device_simulator.py` (Python transport); golden fixtures |
| M4 | GUI side: request on connect, Project File watch, adopt, two API verbs | R3, R6, R10; AC3, AC4 | M2 | `ProjectPainterCommands.cpp`, `Setup.qml:312-331`, `NotificationCenter::postInfo` |
| M5 | Declared controls: button, toggle, slider, knob | R7 (v1); AC5 | M2, M3; output `objectName`s from 0109 if it lands first (**provisional**) | `app/rcc/scripts/output/simple_command.js` + `templates.json` entry |
| M6 | Examples, manual, assistant corpus, terminology | R12; AC6 | M2-M5 | `doc/help/Getting-Started.md` voice; `examples/MPU6050/` |
| M7 | CI checks and publishing | R13 | M3 | `ci.yml` `upload` job (`:1997-2081`) |
| M8 | Binary frames | R14; AC10 | M2, M3, M4; reuses 0108's `render_number_in_place` if landed | `BinaryRaw.cpp`, `TextLineValues.cpp` (shared decoder, two callers) |
| M9 | Numeric text field and units on declared controls | R7 (rest) | **0109 landed (provisional)** + M5 | 0109's control fields |
| M10 | Token echo and replies | R7 (reply) | **0110 landed (provisional)** + M5 | 0110's reply-source fields |

## Affected subsystems & files

Line numbers are the 2026-11-03 tree.

| File | Milestone | Change |
|------|-----------|--------|
| `app/rcc/properties/dataset.json` | M1 | Every property and sub-entity gains `wire`: `{"include": true}` or `{"exclude": "<reason>"}`. Excluded: `Virtual`, `TransformCode`, `TransformLanguage`, `transformParams` (no code); `XAxis`, `WaterfallYAxis` (cross-dataset runtime ids); `GroupId`, `DatasetId`, `UniqueId`, `SourceId`, `Enabled` (structure). Sub-entities carry their inner key list: `alarmBands` drops `sound`, because `AlarmBand::sound` is a WAV path (`Frame.h:298`). Legacy `min`/`max` (`legacyKeys`) ride along. |
| `app/rcc/properties/schema.json` | M1 | `wire` is required on a property and a sub-entity. |
| `app/rcc/properties/declaration.json` (new) | M1 | Prefix `#ss`, wire version 1, request `#ss?`, command `#ss!`; caps (line 16 KiB, groups 64, datasets 512 = `kMaxLineFields`, labels 256, bands 16, controls 64, string 64 B); envelope keys as `Keys::` names (frame: `title`, `groups`, `sources`; group: `title`, `widget`, `datasets`, `groupType`, `outputWidgets`, `outputColumns`; source: framing keys only, never `connection`; control: `title`, `outputType`, `outputMin`, `outputMax`, `outputStep`, `initialValue`, `onLabel`, `offLabel`, `checkable`); allowed group widgets (no `painter`, `webview` or `image`: code, URL and image framing); library version. |
| `scripts/generate-property-registry.py` | M1 | Four renderers added to `render_all` (`:2182`): `DeclarationVocabulary.h`; the C header (written to both library trees, MIT banner); the Python module; `firmware/arduino/library.properties` (version line). The wire projection is ONE function beside `schema_props_for`. |
| `core/Pipeline/DataModel/Generated/DeclarationVocabulary.h` (generated) | M1 | `constexpr` key tables (dataset, sub-entity inner, envelope), caps, prefix and version; `inline void sanitizeDatasetWire(Dataset&)`, which applies each property's `clamp.api` and static option domain (so `fftSamples` stays in `fftSampleCounts`). |
| `firmware/c/ss_vocabulary.h`, `firmware/arduino/src/ss_vocabulary.h`, `firmware/python/serialstudio/_vocabulary.py` (generated) | M1 | Key macros/constants, caps, prefix, wire and library versions, group/dataset widget ids. |
| `scripts/registry-verify.py` | M1 | `check_wire_vocabulary()`, wired into `check_property_manifests()`: `wire` on every property and sub-entity; envelope names resolve to `Keys::`; group widgets resolve in `SerialStudio::groupWidgetId` (`SerialStudio.cpp:353-381`); every key in `firmware/tests/golden/*.decl` is in the generated tables. |
| `scripts/tests/test_registry_verify_wire.py` (new) | M1 | Fixtures: a property without `wire` fails; a golden line with a non-registry key fails (AC9). |
| `core/Pipeline/DataModel/Project/DeviceDeclaration.h/.cpp` (new) | M2 | Pure, no Qt object, no bus reach. `parseDeclaration(QByteArrayView)` covers prefix, version, size cap, `QJsonDocument`, allow-list filter, caps, `read(Group&)` / `read(Dataset&)`, `sanitizeDatasetWire`, index assignment (amendment 4), `groupMeetsWidgetRequirement` (`FrameSupport.cpp:146`; the widget is downgraded with a message), and the binary source block for M8. `DeclarationState::offer(line)` returns `Unchanged` (byte-identical to the last accepted or rejected line), `Rebuild` or `Rejected` with a reason. `wireProjection(groups)` is the canonical comparison M4 uses. |
| `core/Pipeline/DataModel/FrameBuilder/QuickPlotFormatState.h/.cpp` | M2 | `pinDeclared(channels)` gives each channel a name (title) AND a position. While declared, `establish()` (`:263-274`) fixes the format but keeps the registry, and `grow()` (`:222-258`) returns false. `reset()` clears it. |
| `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.h/.cpp` | M2 | Owns `DeclarationState` and the declared groups. `declare(line)` returns the outcome. In declared mode `build()` (`:193-257`) copies the declared groups and title (unique ids as today, no automatic grid or multi-plot). `applyFields()` (`:288-305`) is unchanged. |
| `core/Pipeline/DataModel/FrameBuilder.cpp` | M2 | In `parseQuickPlotFrame` (`:2569`), before `observe` and only when `!m_playerOpen`: a prefix test. On `Rebuild` it runs `invalidateFramePool()`, `build()` and `ensureStructurePublished` (the audio precedent, `:1144-1146`), then returns. One `qInfo` per accepted or rejected content (MessageHandler queues it to the console, `ModuleManager.cpp:255-268`). TU census headroom is 21 lines (baseline 2911, now 2890). |
| `core/Pipeline/DataModel/Project/QuickPlotProject.h/.cpp` | M2 | Declared variant: the declared title; output groups keep `groupType`, `outputWidgets` and `columns` (`projectGroup`, `:166-194`, copies datasets only); the group cap `kMaxQuickPlotGroups` (`:36`, 8) becomes the declaration cap; always `line_values`, never `delimited`; `fieldList` (`:85-106`) spans all groups by index. |
| `core/Pipeline/DataModel/ProjectModel.cpp` | M2 | `saveQuickPlotAsProject` (`:431-445`) passes the declared state in the same blocking hop. |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/TextLineValues.cpp` | M2 | Both parsers return no frame for a line carrying the declaration prefix (`parseSpans` returns 0 (`:184-185`), `parseUtf8` returns `{}`). A saved project then ignores the device's boot declaration exactly as the live lane did (R9). |
| `app/tests/tst_device_declaration.cpp`, `tst_declaration_inert.cpp`, `tst_declared_roundtrip.cpp` (new); `tst_quickplot_format_state.cpp` | M2 | See the test plan. |
| `firmware/tests/golden/*.decl` + `*.frames` (new) | M2 | Hand-written reference lines for the MPU6050, TinyGPS and HexadecimalADC declarations (plus labels and controls). The contract that M3's three libraries reproduce byte for byte. |
| `firmware/arduino/` (new) | M3 | `library.properties`, `src/SerialStudio.h` (header-only, templated on `Stream`, no heap, `F()` strings, a 48 B input line buffer, ~12 B of state), `examples/Minimal/Minimal.ino`. |
| `firmware/c/serialstudio.h/.c` (new) | M3 | C99 with a write callback and a `feed(bytes)` input; the same state machine as the header. |
| `firmware/python/` (new) | M3 | `pyproject.toml` (version read from `_vocabulary.py`), `serialstudio/__init__.py` (stdlib only, transport-agnostic: `write` callable plus `feed`, with a `tcp_server` helper for tests), `tests/` (`unittest`). |
| `firmware/tests/check_golden.py` (new) | M3 | Runs the Python module (and, in CI, the C and Arduino host-shim builds) over the golden sketches and byte-compares their output. |
| `firmware/README.md`, `firmware/LICENSE` (MIT) | M3 | The library's own README (PyPI and Arduino require one). |
| `core/Pipeline/DataModel/Project/DeclarationWatch.h/.cpp` (new) | M4 | GUI thread; a `ProjectModel` sub-object; implements `IO::IRawByteTap`. On bus `ConnectionStateChanged`, connected and Quick Plot with the toggle on, it writes `#ss?\n` through `requireDeviceWriter()`. In Project File mode it scans each chunk for the prefix (carry buffer capped at the line cap), compares `wireProjection` with the loaded project, posts one `NotificationCenter::postInfo` per project and declaration content, and holds the pending document. `adopt()` disconnects and loads the declared groups with the loaded project's source 0 connection through `loadGeneratedProject` (`ProjectLoader.cpp:645-656`). It is inert in a sealed session. Toggle: QSettings `QuickPlot/RequestDeclaration`, default true. |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | M4 | Owns the watch; `setupExternalConnections` binds it (no work in the ctor closure). |
| `app/src/Misc/ModuleManager.cpp` | M4 | Adds the watch to `taps` (`:1006`) and `registry.add("Cpp_JSON_DeclarationWatch", ...)` beside `:1324`. **Composition root**, outside the pinned construction order. |
| `app/qml/MainWindow/Panes/Setup.qml` | M4 | Quick Plot toggle `setup.quickPlot.requestDeclaration` ("Ask the device for its dashboard"); in Project File mode with a pending difference, a `setup.declaration.adopt` button ("Use Device Dashboard"). Donor `:312-331`. |
| `core/Api/API/Handlers/ProjectDeclarationCommands.h/.cpp` (new), `ProjectHandler.cpp/.h` | M4 | `project.declaration.get` (the last declaration: status, reason, summary, pending difference) and `project.declaration.adopt`. Registered last in `registerAll` (`:56-75`, append-only). |
| `core/Api/API/CommandCapabilities.cpp`, `SealedPolicy.cpp`, `app/rcc/ai/command_safety.json`, regenerated `CommandSafetyTable.cpp` | M4 | adopt: `LoadsProject` (beside `:68-69`), denied when sealed (`kDenied`, `:241`), assistant tier `confirm`. get: `NoCapability`, allowed, `safe`. |
| `core/Pipeline/DataModel/Project/DeclaredControls.h/.cpp` (new) | M5 | Builds the `OutputWidget`s of a declared control group. `transmitFunction` comes from the app-side template, with the control name JSON-encoded into one marked line. Names are restricted to `[A-Za-z0-9_ .-]{1,32}`. `DeviceDeclaration.cpp` gains a three-line call (M2 drops output groups with a console note until then). |
| `app/rcc/scripts/output/serial_studio_command.js` (new) + `templates.json` entry | M5 | `transmit(value)` returns `"#ss!" + CONTROL + "=" + value + "\n"`. It is also listed in the editor's catalog. |
| `app/qml/Widgets/Dashboard/Output/DashboardOutputPanel.qml` | M5, only if 0109 has not landed | The cell `objectName` `"dashboard.output." + groupId + "." + widgetId` and the `.slider` / `.knob` suffixes, exactly as 0109's plan specifies (**provisional**). |
| `serial_studio_command.js`, `DeclaredControls.cpp`, `firmware/**` | M10 | Follows 0110's draft plan (2026-11-03): `transmit(value, token)` sends `#ss!<name>=<value>@<token>`. When a declaration has controls, the library appends three reply fields (token, code, message) to every frame. `DeclaredControls` declares them as hidden datasets (`hideOnDashboard`) and binds each control's `replyTokenDatasetId` / `replyCodeDatasetId` / `replyMessageDatasetId` to the unique ids the builder assigns. The v1 library already ignores an `@<token>` suffix, so v1 sketches keep working (**provisional**). |
| `app/rcc/properties/declaration.json` (control keys) | M9 | 0109's `inputKind`, `outputDecimals`, `outputUnits`, `quantizeToStep` (its plan's `FrameKeys.h` additions) join the control allow-list. Admission reuses 0109's `admitOutputValue` (**provisional**). |
| `examples/MPU6050/MPU6050_Declared/MPU6050_Declared.ino`, `examples/TinyGPS/TinyGPS_Declared/TinyGPS_Declared.ino`, READMEs | M6 | Declared variants; each sketch pins the library with a `#if SS_LIBRARY_VERSION_MAJOR != 1` / `#error`. |
| `doc/help/Device-Declarations.md` (new), `Getting-Started.md` (leads with it, before `:105`), `Operation-Modes.md`, `Output-Controls` page, `help.json` | M6 | Wire format, library calls, request toggle, adopt, Pro controls with the trial named. Written through the `ss-docs` skill. |
| `app/rcc/ai/skills/project_basics.md` + rebuilt `search_index.json` | M6 | The declaration, the prefix, and that keys are project keys. |
| `doc/claude/architecture/dataflow.md` ("The Quick Plot Lane"), `CLAUDE.md` Subsystem Contracts row | M6 | The declared branch, the prefix-before-detect rule, generated vocabulary. Hazard: a hand-typed key list. |
| `.github/workflows/ci.yml` | M7 | Job `firmware`: C99 `-Wall -Wextra -Werror -pedantic` build and host test, Arduino host-shim golden test, Python `unittest`, `check_golden.py`, and the `arduino-cli` Uno compile of `Minimal.ino` with a RAM report (pinned version and checksum). Job `publish-firmware` on tags, needing `firmware`, idempotent (skips when the library version is already published): PyPI by trusted publishing (`pypa/gh-action-pypi-publish` pinned by SHA, `id-token: write`); push the `firmware/arduino` subtree to the mirror repository and tag it `v<library version>`. |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/BinaryPackedStruct.h/.cpp` (new), `BinaryTemplates.cpp`, `NativeTemplateSupport.h` | M8 | Template `packed_struct` with params `fields` (CSV of `i8,u8,i16,u16,i32,u32,i64,u64,f32,f64`) and `endianness`. One allocation-free `decodePackedStruct(QByteArrayView, double*, n)` serves the template's `parseBinary` and the Quick Plot lane. The support header gains the 64-bit and f64 readers. A wrong frame length is rejected. |
| `core/Core/Bus/Messages.h`, `core/Pipeline/AppState.h/.cpp` | M8 | Retained topic `DeclaredFraming`. In `deriveFrameConfig` (`:294-342`) the Quick Plot branch returns the declared framing with `cfg.operationMode = ProjectFile`, which is the reader's framing policy. FrameReader is unchanged, and `PipelineHost::attach` keeps the checksum (`:242-246`). It is cleared on disconnect and on a mode change. |
| `QuickPlotBuilder.cpp`, `FrameBuilder.cpp`, `DeclarationWatch.cpp`, `QuickPlotProject.cpp`, `firmware/**` | M8 | Binary branch in the lane (decode, write in place, stage). In a declared binary session the watch forwards prefixed lines to a self-marshalling `FrameBuilder::submitDeclarationLine`. Save as Project emits the declared source. The libraries emit start-delimited frames with a CRC. |

## Architecture & data flow

- **The line.** `#ss1{...}` is compact JSON: no whitespace outside strings, numbers only as
  object members, never bare in an array. Inside a string, a space, `,` or `;` that comes right
  before `[0-9+.-]` is written `\u00XX`. Under every 0106 separator (`LineFormat.cpp:466-531`), each
  token therefore starts with `#`, `"`, `[` or `{`, or is glued to a closing quote, so it never
  parses as a number (`parseLineField` `:252-279`, `isUnitSuffix` `:126-136`). The line is a 0106
  text line and never establishes a format. The MPU6050 declaration is 796 bytes (the premise
  measured 467 for a bespoke grammar), about 69 ms at 115200 baud. The app also sends two lines:
  `#ss?` asks for the declaration and `#ss!<name>=<value>` carries a command.
- **Quick Plot, pipeline thread.** `parseQuickPlotFrame` tests the prefix first. On a match,
  `QuickPlotBuilder::declare(line)` hands it to `DeclarationState::offer`:
  - `Unchanged`: nothing happens.
  - `Rejected`: one `qInfo` with the reason, and the current structure stays.
  - `Rebuild`: `pinDeclared` on the state, then `invalidateFramePool()`, `build()` and
    `ensureStructurePublished`.

  Data lines then flow through `observe` as before. The first data line detects the separator
  once and keeps the declared registry. Positional and named fields map through `indexOf`, and a
  missing field writes `kNoSample` (R5). On a connect or a mode change, `m_quickPlot.reset()`
  (`FrameBuilder.cpp:1070`, `:1444`) forgets the declaration, and the request gets it back.
- **Request on connect (R6).** `DeclarationWatch` sees `ConnectionStateChanged` on the GUI thread
  and writes `#ss?` from that slot. FrameBuilder's connect-edge reset was posted to the pipeline
  before any reply bytes can reach it, so the answer cannot be wiped. AC4 pins this ordering.
- **Project File mode (R3).** The pipeline is untouched. The watch's `onDeviceBytes`
  (`IRawByteTap`, GUI thread, every mode, chunk rate) scans each chunk for `#ss` only in Project
  File mode, and in M8 binary sessions. On a difference it posts one notice and raises
  `pendingDifference`. Adopt and Keep are offered in Setup and over the API. Adopt disconnects,
  then loads through `loadGeneratedProject`, which is already in Project File mode, so no mode
  switch happens (spec 0100).
- **Save as Project (R9).** The declared groups, the title and the 0106 format go to
  `QuickPlotProject::build`, which emits `line_values` with the same tokenizer. The template skips
  declaration lines just as the live lane does, so the saved project parses the stream (boot
  declaration included) to the live values by construction.
- **Libraries.** On boot and on `#ss?`, `declareDue` is set. During the next loop pass,
  `group()`, `channel()` and the chained setters stream the declaration JSON straight to the
  port, and `send()` closes it with `\n` instead of a frame. That pass's values are dropped.
  On every other pass, `channel()` prints the value and a separator, and the setters do nothing.
  `poll()` reads input lines and dispatches `#ss?` and `#ss!name=value` to the callback. A
  trailing `@<token>` is accepted and ignored until M10. Lines without the prefix go to an
  optional callback, so the sketch's own command parser still works.

## Hotpath & threading impact

- **Touches the hotpath? Yes.**
  - *M2:* `FrameBuilder::parseQuickPlotFrame` and `line_values` on the span lane. The binding
    rules, and how they hold:
    1. The common path pays one first-byte compare: no allocation, no lookup, no loop.
    2. Every structure change goes through `invalidateFramePool()` + `build()`, so the
       `structureGeneration` stamps stay correct.
    3. JSON parsing runs on the pipeline thread at declaration rate (at most once per content,
       bounded at 16 KiB, about 0.1 ms), never per frame.
    4. No new signal: the console line is a `qInfo` that the existing handler queues.
    5. `SS_ASSERT_HOTPATH` appears only in `FrameBuilder.cpp`. The new TUs use `SS_ASSERT` with
       a recovery.
  - *M8:* a `packed_struct` decode per binary frame, allocation-free into the existing dataset
    strings (`assign_utf8_in_place`), plus a framing override that reuses the reader's existing
    reconfigure path (`FrameConfigChanged` → `resetFrameReader` → `PipelineHost::attach`).
    `FrameReader.cpp` is not edited.
- **Benchmark.** `--benchmark-hotpath` never reaches `parseQuickPlotFrame`, and `line_values` is
  not in the synthetic project. The nine gates should not move. AC7 is a maintainer run.
- **New cross-thread signal/slot? No per-frame one.** M8 adds a bus topic at declaration rate
  (pipeline → AppState, queued by the bus) and a self-marshalling `submitDeclarationLine`
  (GUI → pipeline, at declaration rate, via `invokeOnBuilderThread`, never a plain blocking
  queued call).
- **New input to a cached hotpath flag? No.** `m_playerOpen` gates the prefix test as it gates
  replay. The declared state is local to the builder and is reset at the existing three sites.
- **Timestamp ownership.** Unchanged: `data->timestamp` reaches `m_stager.stage` as today.

## Data model & persistence

- No `Frame.h` or `Keys::` changes. Declarations reuse project keys. 0109 and 0110 add their own
  control keys, which M9 and M10 add to `declaration.json`.
- `dataset.json` and `schema.json` gain `wire`. `declaration.json` is new and is validated by
  `registry-verify`.
- Saved projects: a declared Save as Project is an ordinary project with `line_values`; M8 adds
  `packed_struct` plus delimiters and a checksum. Forward-only: an older build lacks
  `packed_struct`.
- QSettings: `QuickPlot/RequestDeclaration` (bool, default true).

## API / SDK surface

- New: `project.declaration.get` and `project.declaration.adopt` (M4). Each is classified in
  `CommandCapabilities`, `SealedPolicy` and `command_safety.json`, and the MCP hint table is
  regenerated with `generate-command-safety.py`. The maintainer's build regenerates
  `api-schema.json`.
- Unchanged: `dashboard.getData` is how AC1 and AC4 read back the structure; `notifications.list`
  is how AC3 reads the notice.

## QML / UI

- Setup: the request toggle (Quick Plot) and the adopt button (Project File, pending
  difference), each with an `objectName`.
- Dashboard: declared groups render with today's widgets. A Quick Plot frame can already carry an
  Output group (`WidgetMapBuilder` has no mode check, and transmit is not mode-gated). Output
  controls gain names (M5 or 0109).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Wire syntax | (a) prefixed JSON with project keys; (b) the bespoke compact grammar the premise measured (467 B); (c) one line per dataset | **(a).** The reader is the project reader and the vocabulary is the project file's; (b) is a second grammar, the failure the spec exists to avoid; (c) breaks R14's "always a text line" and makes atomicity a protocol. Cost: 1.7x bytes, still under 1 KB. |
| Wire key names | project `jsonKey`s; API names; short keys | **jsonKeys.** A saved project and a declaration then read the same; short keys would be a third name per property. |
| Where the parse runs in Quick Plot | (a) the pipeline, before `observe`; (b) a GUI-side tap posting to the pipeline | **(a).** It runs in order with the frames. (b) lets the next lines establish a detected dashboard first, so the first-hour view would flicker. |
| Detection in Project File mode | (a) a GUI-side `IRawByteTap`; (b) a prefix test in `parseProjectFrame` | **(a).** (b) taxes every project frame on the gated path, and framing decides whether the line is even a frame. |
| Library API shape | (a) declare-in-loop (`channel(name, value).units(...)`); (b) declare in `setup()` and `send(v1, v2, ...)` | **(a).** The goal's own wording is "a sketch declares a channel in the line where it already prints it". Re-declaring on request costs no RAM, because the calls re-run, and values and metadata cannot drift. (b) has to store the structure in RAM or take a callback. |
| Library home | `firmware/`; `lib/` | **`firmware/`.** `lib/` is vendored code tied to `osv-scan.py` and the supply-chain trigger. |
| Group and control vocabulary | (a) `declaration.json` naming `Keys::`; (b) a spec-0036 registry for groups and controls; (c) lists typed by hand in each library | **(a).** It drives the same generator and gate at a fraction of (b)'s cost; (c) is the drift this spec forbids. |
| Binary framing in Quick Plot | (a) a declared `FrameConfig` with framing policy `ProjectFile`; (b) teach FrameReader's Quick Plot branch detection modes and checksums; (c) base64 lines (amendment 3) | **(a)** as written: FrameReader is not edited and the reconfigure path already exists. (c) is the cheaper fallback if the maintainer amends R14. |
| AC1 save half | a ctest round trip; a new `project.quickPlot.exportJson` | **ctest.** No new API surface; follows the 0106 pattern. |
| Adopt UI | notification plus Setup button plus API; modal Adopt/Keep | **Non-modal.** A modal over a live session blocks tests and operators. |

## Risks & mitigations

- **Device text inside generated JS (M5).** The control name enters a transmit function. It is
  JSON-encoded into one marked line and restricted to `[A-Za-z0-9_ .-]{1,32}`. A ctest feeds
  quotes, backslashes and `</script>`-style names and expects them refused.
- **Device-named file paths.** `AlarmBand::sound` is a WAV path and is excluded on the wire. The
  M1 gate makes every future sub-entity field state its wire status.
- **Resource blow-up (R10).** Line, count and string caps are applied before `read()`.
  `sanitizeDatasetWire` applies the API clamps and domains (`fftSamples`, `decimalPoints`). A
  device that alternates two declarations rebuilds at most once per second (driver timestamp,
  ~5 lines).
- **Header coincidence on a 0106-but-pre-0111 app.** A boot declaration becomes the header
  candidate. It titles channels only when its field count under the detected separator equals
  the stream's (`QuickPlotFormatState.cpp:280-301`), for example a one-channel stream. This is
  documented. Shipping 0111 in the same release as 0106 removes the case.
- **Request side effects.** `#ss?` reaches devices without the library, such as AT modems. It is
  a Setup toggle, and Console Only and Project File never send it.
- **Reconnect race.** A device without a reset-on-open streams data before it answers `#ss?`.
  The dashboard can then be detected for one round trip, then rebuilt. The library answers
  before its next frame.
- **AC6 budget.** The Adafruit MPU6050 stack may already use much of the Uno's 1 KB half. M6's
  first task measures the existing `MPU6050.ino` with `arduino-cli`. The library's own share is
  gated in CI by the `Minimal` compile.
- **Composition root and API consent.** Two lines in `ModuleManager.cpp`, outside
  `instantiateCoreModules()`. adopt carries `LoadsProject`; classifying it `NoCapability` would
  let a remote client replace the user's project without consent. `tst_command_capability_registry`
  and `tst_sealed_policy_registry` fail on an unclassified name.
- **Trial parity.** Declared controls are an ordinary Output group, so the existing compile gate,
  `SS_LICENSE_GUARD` and the token check apply (`Base.cpp:343-349`, `:388-390`). The connect gate
  counts only transforms and tables, so nothing new is gated. The manual names the trial; the
  `trial-parity` lint enforces it.
- **TU census.** `FrameBuilder.cpp` has 21 lines of headroom, and M2 uses about 8. Check
  `--tu-census` before handoff.
- **Silent-breakage classes** (`common-mistakes.md`): no queued pipeline hop; the timestamp
  passes through; no `operator[]` inserts into the registry; no generated file edited by hand;
  `api-schema.json` is regenerated by the maintainer's build.

## Test & verification plan

| Criterion | Check | Who runs |
|-----------|-------|----------|
| AC1 | `tests/integration/test_device_declaration.py::test_declared_dashboard`: the Python library serves TCP on the device-simulator port; the app is in Quick Plot; `dashboard.getData` groups, widgets, titles, units and ranges equal the declaration. Save half: `ctest -R tst_declared_roundtrip` (golden declaration + frames → `QuickPlotProject::build` → `line_values` parse equals live `applyFields`, boot declaration included) | maintainer (app up, API on) / maintainer build, I run `ctest` |
| AC2 | `ctest -R tst_device_declaration`: malformed, oversized, unknown version, over-cap and code-key lines → `Rejected` with no rebuild; the same line twice → one `Rebuild`; a changed line → a second | ctest |
| AC3 | `test_declaration_in_project_mode`: `project.loadJson`, the device declares something different, `notifications.list` holds one entry, `project.exportJson` is unchanged, `project.declaration.adopt` → the declared title and groups | maintainer |
| AC4 | `test_reconnect_request`: the library declares only at start; `io.disconnect` then `io.connect`; the dashboard is rebuilt through `#ss?` | maintainer |
| AC5 | `test_declared_slider`: `ui.automation.key` on `dashboard.output.<g>.<w>.slider`, the Python callback receives `(name, value)`; needs `SERIAL_STUDIO_API_AUTO_CONSENT=1` and a Pro or trial build | maintainer |
| AC6 | Maintainer observation on an Uno; CI `arduino-cli` RAM line for `Minimal.ino` | maintainer / CI |
| AC7 | `--benchmark-hotpath --min-fps 256000`, nine gates unchanged | maintainer |
| AC8 | `ctest -R tst_declaration_inert`: each golden line under every 0106 candidate format (5 separators × decimal comma, plus JSON) gives zero numeric fields, and `detectLineFormat` is false; maintainer observation on the first 0106 release (amendment 1) | ctest / maintainer |
| AC9 | `registry-verify.py`, `generate-property-registry.py --check`, `pytest scripts/tests/test_registry_verify_wire.py` | me |
| AC10 | `test_declared_binary` (same values as the text variant); `ctest -R tst_packed_struct` (every type, both byte orders, wrong length) | maintainer |
| Libraries | `python -m unittest` in `firmware/python`; `firmware/tests/check_golden.py` (Python half) | me; the C and Arduino halves in CI |
| Static | `syntax-check.py` on every edited C++ file; `code-verify.py --check`; `--tu-census`; `term-verify.py`; `documentation-verify.py`; `claim-verify.py`; `reuse lint` (M1 adds the MIT block for `firmware/**`); `qt-cpp-review` before handoff; `sanitize-commit.py` before each commit | me |

## Maintainer actions (accounts and secrets; never an agent's)

1. Reserve the PyPI name. Configure **trusted publishing** for this repository, the workflow and
   a protected `pypi` environment (no stored token).
2. Create the Arduino mirror repository. Add a deploy key or fine-grained token as a secret
   (e.g. `ARDUINO_MIRROR_TOKEN`), scoped to push and tag there only.
3. Submit the one-time registration PR for the mirror to `arduino/library-registry`.
4. Approve the pinned `arduino-cli` version and checksum, and the `pypa/gh-action-pypi-publish`
   SHA, for `ci.yml`.
5. Bump the library version in `declaration.json` when a release should publish it.
6. Regenerate `api-schema.json` with a build after M4 (`--dump-api-schema`), and lupdate for
   the new strings.
7. Run AC5-AC7, AC10 and the integration files on a built app.

## Open questions (recommended answer, one-line why)

All eight were decided as recommended on 2026-11-03, and spec.md carries the nine amendments.

1. **Amendments 1-9 above.** Recommend accepting all nine as proposed. Each replaces a sentence
   the tree cannot satisfy with the nearest one it can.
2. **R14: raw packed frames (M8 as designed) or base64 lines?** Recommend raw, as decided, kept
   last and isolated. Bandwidth is why binary exists; if M8's cost surprises, base64 is a
   drop-in amendment.
3. **Unknown keys inside a known version: drop them with one console note, or reject the whole
   declaration?** Recommend dropping them. A library newer than the app (e.g. 0108 keys)
   otherwise loses its whole dashboard over one field.
4. **PyPI distribution and import name?** Recommend `serialstudio` for both if free, else
   `serial-studio-device` / `serialstudio`. Short names are what `pip install` users type.
5. **When does CI publish the library?** Recommend app release tags, skipped when the library
   version is unchanged. One release ritual, and the idempotence avoids duplicate-version
   rejections from both indexes.
6. **"Says so once" scope for R3?** Recommend once per loaded project and declaration content,
   in memory. A per-file QSettings memory adds state for a notice that can be dismissed.
7. **May the agent compile `firmware/c` with `cc -fsyntax-only -std=c99`?** Recommend extending
   `syntax-check.py` to it. CLAUDE.md sanctions only that script, and the C file would otherwise
   be checked only in CI.
8. **Should declared datasets plot by default?** Recommend the library's `channel()` emitting
   `"graph":true` unless told otherwise. The editor's default is off, and a declared stream with
   no curves would read as broken in the first hour.
