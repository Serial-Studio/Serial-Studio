---
spec: 0100-new-user-clarity
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-15
---

# Plan 0100 — New-User Clarity

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The work ships in five milestones, each leaving the tree consistent: **M1** first-run copy and
Pro notice (A + G), **M2** mode model and Save as Project (B), **M3** parser contract and Group
Widget rules (C + D), **M4** naming sweep (E), **M5** manual and examples (F). Almost all of it
is strings, QML and small C++ seams. The two real designs are these:

- **Save as Project** is a pure *Quick Plot structure → project JSON* builder. It is a new
  sub-object of the `ProjectModel` facade, fed through the existing generated-project load +
  save path, which is atomic, undo-free and already macOS-dialog-safe.
- **The mode trap** is closed by one `AppState::requestOperationMode()` guard. It defers
  *implicit* switches while connected, and every load/save/generate path calls it.

The live short-frame finding (R12) already exists (`short-frames`, spec 0088). R12 extends its
reporting and adds Go To, with **no hotpath edit**. Terminology becomes a re-runnable gate
(`scripts/term-verify.py`). The manual keeps every help **id** stable and renames only `file`
fields, with stub pages left at the old paths.

## Affected subsystems & files

Grep-confirmed during planning. Generated files are regenerated, never hand-edited.

### M1 — First run + Pro notice (R1–R6, R31)

| File | Change |
|------|--------|
| `core/Ui/UI/Widgets/Terminal.cpp/.h` | `loadWelcomeGuide()` prepends a `tr()` getting-started block (≤ 12 lines, step 3 tailored to the current mode) above the logo and licence text. Adds the R5 no-data hint: a 10 s single-shot `QTimer` armed on connect, which appends one hint line if `Dashboard::available()` is still false and the mode is not ConsoleOnly. The hint distinguishes "no bytes" from "bytes but no frames" using the link stats. |
| `app/qml/MainWindow/Panes/Setup.qml` | Group label "Operation Mode". Radios read "Console Only", "Quick Plot", "Project File", each with a wrapped description `Label` (R2). Radios and export toggles get reason tooltips (R4). The inner "Device Setup:" row title is dropped (R20) and the combo label changes (see M4 string table). |
| `app/qml/Commands/AppCommandBindings.qml` | `cmdIoToggleConnection` gets a dynamic `tooltip` naming the blocking reason (no device selected / invalid settings / playback open / mirror attached). |
| `app/qml/MainWindow/Panes/Toolbar.qml` | Connect button uses `entry.tooltip` instead of its hard-coded `qsTr`. |
| `app/rcc/commands/layouts/main-toolbar.json` | Adds `app.problems` (R6). Its binding already exists. |
| `app/qml/Widgets/ProNotice.qml` + the 4 users (`DatasetView.qml`, `GroupView.qml`, `ProjectView.qml`, `DashboardCanvas.qml`) | The GPL-build notice lists the project's Pro items and the action (R31). |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | New read-only `QStringList commercialFeatureSummary` property. Its notify signal is the existing `containsCommercialFeatures` one. It is built from `SerialStudio::proContentSummary` plus Pro widgets, multi-device and Pro bus. |
| `app/src/Misc/ProFeatureNotice.cpp` | Verify only: the commercial connect refusal already names transforms/tables (`contentDetail`). Wording stays unless AC15 finds a licence state without an action. **No licensing logic touched.** |

### M2 — Mode model + Save as Project (R7–R9)

| File | Change |
|------|--------|
| `core/Pipeline/AppState.h/.cpp` | New `requestOperationMode(mode)`. If disconnected it calls `setOperationMode` as before. If connected it stores `m_pendingMode` and posts a NotificationCenter notice ("Switches to Project File mode when you disconnect"). It subscribes to the `ConnectionStateChanged` bus topic to apply the pending mode on disconnect. `setOperationMode` itself is unchanged, including its signals and persistence. |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | `openJsonFile(path)` (:613) and `loadGeneratedProject` (:649) call `requestOperationMode`. The failure-restore path keeps using the direct setter. |
| `core/Pipeline/DataModel/Project/ProjectPersistence.cpp` | `finalizeProjectSave` (:448) calls `requestOperationMode`. `askSave` (:288) treats a pending Project File switch as Project File mode, so a deferred, unsaved generated project prompts instead of being discarded. |
| `app/src/Misc/FileOpenEventFilter.cpp`, `app/qml/.../FileDropArea.qml`, `app/qml/main.qml` (examples), `app/qml/Commands/AppCommandBindings.qml` (Open Project) | Route through `requestOperationMode` (the QML side via a new `Cpp_AppState` slot). |
| `app/qml/Commands/ProjectEditorCommandBindings.qml` | Editor "New" also requests Project File mode. `newJsonFile()` stays mode-neutral because startup resets use it. |
| `app/qml/ProjectEditor/ProjectEditor.qml` | *(Amended.)* Opening the editor while disconnected requests Project File mode, which is immediate and posts a notice. The mode overlay shows only while connected in another mode, with reworded copy and a "Disconnect and Edit" button. The lock overlay is unchanged. Reason: about 30 workspace/presentation operations return early outside Project File mode, so an editable editor in another mode would silently drop edits. |
| **new** `core/Pipeline/DataModel/Project/QuickPlotProject.h/.cpp` | Pure static `build(groups, busType, connSettings) → QJsonObject`. It produces one device with Quick Plot framing (`EndDelimiterOnly`, `frameEnd "\n"`, PlainText, Built-In `delimited` template, `{separator:",", trimFields:true}`) and the Quick Plot groups serialized as-is (titles from detected headers, indices 1..N, plots on). |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | New public slot `saveQuickPlotAsProject()` and a `quickPlotSaveAvailable` property (QuickPlot mode, `Dashboard`-side frame seen, bus ≠ Audio). The slot copies `quickPlotFrame().groups` via `invokeOnBuilderThreadBlocking` (precedent `ProjectWorkspaces.cpp:884`) and reads `bus.latest<ActiveUiDriverSettings>()`. It then calls `QuickPlotProject::build` → `loadGeneratedProject` + `saveJsonFile(true)`. |
| `app/rcc/commands/app.json` + `AppCommandBindings.qml` + `Setup.qml` | New command `quickplot.saveAsProject` ("Save as Project"), shown as a button under the Quick Plot radio and in the palette. Its tooltip explains why it is disabled. `scripts/generate-command-strings.py` is re-run. |
| `core/CMakeLists` (Pipeline target source list) | Adds `QuickPlotProject.cpp`. |

### M3 — Parser contract + Group Widget rules (R10–R16)

| File | Change |
|------|--------|
| `app/rcc/properties/dataset.json:770` | Index description: "Position of this value in the parser's output (first value = 1). 0 = unassigned." Then re-run `scripts/generate-property-registry.py` (regenerates `Generated/DatasetForm.cpp`, `DatasetApiFields.cpp`, …). The maintainer re-dumps `api-schema.json` after building (`--dump-api-schema`). |
| `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp/.h` | `emitPreview` adds `datasets`, a map from frame index to dataset titles for this device (enabled, non-virtual; same walk as `LinkCheckers.cpp:451`), plus `maxIndex`. |
| `app/qml/Dialogs/FrameParserTest.qml` | `rebuildTree` labels cells `[3] Temperature`. It appends flagged rows for indices above the row length: "[5] Humidity: not in this frame". |
| `core/Ui/Misc/Problems/LinkCheckers.cpp` | `reportShortFrames` reports every offending device (one finding each, `entityUniqueId` = device) and sets `jump` = first starved dataset (`kind "dataset"`). `makeLinkFinding` gains optional jump fields. |
| `core/Ui/Misc/Problems/ProjectCheckers.cpp` | Duplicate-index key adds the dataset widget type (R13). New `project.group-widget` checker that uses the shared requirement helper, with group jump (R16). |
| `core/Core/SerialStudio.h/.cpp` | New pure `groupWidgetRequirement(GroupWidget) → {minCount, maxCount, roles}`. This is the single home that replaces the scattered literal 3s for validation. The runtime guards in `Accelerometer.cpp`/`Gyroscope.cpp` keep their own check, because they are hot-ish draw code and we don't touch them. The GPS rule (2–3 vs 3) is confirmed against `DashboardIngest.cpp` before it is coded. |
| `app/qml/ProjectEditor/Views/GroupView.qml` + `core/Ui/ProjectEditor/ProjectEditor.h/.cpp` | A requirement line under the group header ("Needs 3 datasets: X, Y, Z in this order") and a warning banner while it is unmet. These come from a new `Q_INVOKABLE groupRequirementText()` / `groupRequirementMet()` on the editor (R15). |
| `app/qml/ProjectEditor/Views/DatasetView.qml` | A notice when the dataset is computed: "Computed: its transform doesn't read `value`, so it has no frame index" (R14). |
| `doc/help/Problem-Center.md` | Documents `short-frames`, the narrowed duplicate rule and `project.group-widget`. |

### M4 — One name per concept (R17–R21)

| File | Change |
|------|--------|
| **new** `scripts/term-verify.py` + `scripts/term-rules.json` | One banned-synonym list with allowlists (code spans, identifiers, API Reference, Modbus/HID "registers"/"tables", Data Grid, "X-Axis Source", the "Session Databases" folder, technical "native"). Scans `qsTr`/`tr` literals, `app/rcc/commands/**`, `doc/help`, `README.md`, `examples/**/README.md` and `app/rcc/ai/**` (not `search_index.json`). Exit 1 on hit. Wired into the `docs.yml` job and `ci.yml` lint step. |
| ~6 QML files (`Setup.qml`, `RuntimeReconfigure.qml`, `ProjectView.qml`, `TriggerDialog.qml`, `DataTablesView.qml`, `TableFolderView.qml`, `UserTableView.qml`, `DiagramLayout.qml`, `DatasetView.qml`, `UART.qml`) | Renames per the string table below. "COM Port" becomes platform-conditional (R19). Tooltips go on Alarm Sequence (`ProjectView.qml:292`), Send DTR Signal (`UART.qml:294`) and Data Conversion Method (R20). |
| `core/Ui/ProjectEditor/EditorForms.cpp:448`, `ProjectSources.cpp:294`, `ProjectModel.cpp:970`, `ProjectTables.cpp`, `ProjectFolders.cpp`, `EnumLabels.cpp:487,527`, `SerialStudio.cpp:238` | `tr()` strings, including undo labels. `EnumLabels` labels are user-visible only and leave the slugs unchanged (tst_enum_labels pins slugs, so verify). |
| `app/rcc/commands/projecteditor.json`, `layouts/project-toolbar.json` | "Add Data Source" → "Add Device". "Shared Table" → "Variables Table". "Level Indicator" → "Bar". Then regenerate `CommandStrings.cpp`. |
| `app/rcc/properties/dataset.json:1801` | apiDescription stays in API vocabulary (spec R18 amendment), so no change. |
| `app/rcc/ai/skills/*.md`, `app/rcc/ai/docs/*.md` | Prose uses user names when talking to the user, with API names in code spans. Then regenerate `search_index.json` via `app/rcc/ai/build_search_index.py`. Corpus file names stay. |

**String table (for review):**

| Today | After |
|-------|-------|
| "I/O Interface: %1" | "Connection: %1" |
| inner "Device Setup:" row | removed (pane title stays "Device Setup") |
| "Sources" / "Source" (ProjectView, TriggerDialog, Setup redirect) | "Devices" / "Device" |
| "Add Data Source", "Rename Data Source" | "Add Device", "Rename Device" |
| "Composite Widget" | "Group Widget" |
| "Edit Control Script" | "Edit Control Loop" |
| "Add Shared Table", "Add Table", "Table" (Variables sense) | "Add Variables Table", "Variables Table" |
| "Register access code copied" | "Variable access code copied" |
| "Painter (custom canvas)" | "Canvas Widget" |
| "Bar/Level", "Level Indicator", "Bar (level meter)" | "Bar" (dataset widget). "Bar Panel" stays (group widget). |
| "COM Port" | Windows "COM Port", elsewhere "Serial Port" |

### M5 — Manual + examples (R22–R30)

| File | Change |
|------|--------|
| **new** `doc/help/Concepts.md` | The chain diagram (mermaid) plus short definitions linking each reference page (R22). |
| **new** `doc/help/Project-File-Tutorial.md` | A worked tutorial over the new example below (R25). |
| **new** `doc/help/Derived-Values.md` | Decision guide covering the five mechanisms, one example each, with Pro status (R26). |
| **new** `examples/Project Tutorial/` (`tutorial.ino`, `tutorial_sim.py`, `tutorial.ssproj`, `README.md`) + `examples/examples.json` | Frame `$t,h,ax,ay,az;`. Exercises start/end delimiters, a Data Grid group and an Accelerometer group (X/Y/Z). The Python UDP simulator is the no-hardware path. Difficulty Beginner. |
| `doc/help/help.json` | Inserts Concepts after Getting Started. Moves Glossary into Basics and Identity Model after Project Editor. Splits Support into "Support" (FAQ, Troubleshooting, Problem Center) and "About" (About, Comparison, Use Cases, Affiliate). Renames files with **ids unchanged**: `data-tables`→`Variables.md`, `painter-widget`→`Canvas-Widget.md`, `javascript-api`→`Frame-Parser.md`, `control-script`→`Control-Loop.md`, `session-database`→`Historian.md`, `data-sources`→`Devices.md`. Titles get the one Pro convention (R29). |
| 6 stub pages at the old filenames | A one-line pointer to the new page. Not registered in help.json, because old ids still resolve to the new files. Kept for raw-URL fetchers (older builds, `HelpFetcher`) and external links. |
| ~46 `doc/help/*.md` files | Cross-link updates to the new filenames, a canonical-term sweep, one Pro convention, and Linux permission guidance centralised in `Linux-Installation.md` (others link to it) (R27). |
| `doc/help/Getting-Started.md` | Links Concepts and Glossary first, states the modes once, drops the duplicated install steps, tags Pro features (R24). |
| `doc/help/Troubleshooting.md` | New "Parser returns the wrong number of values / widget frozen" entry (R27). |
| `examples/*/README.md` (35) + `examples/README.md` | "What you'll learn / Prerequisites" opener and a learning-order section (R30). The app's Examples Browser is unchanged. |
| `core/Ui/AI/Conversation/HelpFetcher.cpp:311`, `MetaToolCatalog.cpp:247`, `app/rcc/ai/skills/control_script.md:334`, `ControlScriptView.qml:87` (`helpPage: "Control-Script"`) | Example and fetch names move to the new filenames. `tst_help_fetcher` is checked for pinned names. |
| `scripts/documentation-verify.py` | New checks: every non-stub page is registered in help.json, every relative `.md` link resolves, and every stub points to a registered page (AC13). |
| `CLAUDE.md`, `doc/claude/architecture/scripting.md` | Update old page-name references (claim-verify). |

## Architecture & data flow

- **Pending mode.** The flow is `UI/API load or save → requestOperationMode(ProjectFile)`.
  - Disconnected: `setOperationMode` runs as before, and its retained bus topics and signals
    drive `ConnectionManager`, `Dashboard` and `ProjectModel` exactly as today.
  - Connected: the request is stored in `m_pendingMode` and a notice is posted. The
    `ConnectionStateChanged(disconnected)` bus subscription in AppState then calls
    `setOperationMode(m_pendingMode)`.
  - Only the *timing* of the existing transition changes. Explicit mode commands (radios,
    `mode.*`, API `DashboardHandler:368`, CLI) keep calling `setOperationMode` directly.
- **Save as Project** runs entirely on the GUI thread except for one command-rate blocking hop.
  1. `saveQuickPlotAsProject()` reads the Quick Plot groups via
     `invokeOnBuilderThreadBlocking`. This is a copy of a small structure; the pipeline thread
     owns `m_quickPlot`.
  2. It reads the retained `ActiveUiDriverSettings` (thread-safe `bus.latest`).
  3. `QuickPlotProject::build` creates the JSON.
  4. `loadGeneratedProject` loads it as modified, then `saveJsonFile(true)` opens the queued
     file dialog.
  5. Mode: `loadGeneratedProject` calls `requestOperationMode`, so while still connected the
     switch is pending and Quick Plot keeps running until the user disconnects.
- **Short frames.** The flow is `ShortFrameWatch` (pipeline writer, atomic) → 1 Hz
  `link.statistics` poll → findings. This already exists. The change is the report loop and the
  jump fields.
- **Parser test.** `FrameParserModel::dryRun` runs off the live pipeline. The dataset map is read
  from `ProjectModel` on the GUI thread.

## Hotpath & threading impact

- **Touches the hotpath?** **No.**
  - `FrameBuilder`, `FrameReader`, `CircularBuffer`, the span lane and Dashboard draw code are
    not edited.
  - R12 reads the existing `shortFrameStats()` on the existing 1 Hz tick.
  - The Accelerometer/Gyroscope runtime guards are untouched.
  - `--benchmark-hotpath` is still run once at the end (AC9) as the regression guard.
- **New cross-thread signal/slot?** No new connection. AppState's new subscription is a bus
  subscription on the GUI thread (AppState's thread), at connection-state rate. The Quick Plot
  read uses the existing `invokeOnBuilderThreadBlocking` helper at command rate, never from a
  script API.
- **New input to a cached hotpath flag?** No. `m_operationMode` still changes only through
  `setOperationMode`, so its existing change signal still feeds the cache refresh. The pending
  mode is not a hotpath input.
- **Timestamp ownership:** unaffected.

## Data model & persistence

- No `Frame.h` key, schema version or project-format change. The generated project uses
  today's keys and the current `kSchemaVersion`.
- `m_pendingMode` is not persisted. On quit with a pending switch, the persisted mode is
  written as the pending one, so the next launch opens the saved project in Project File mode.
- Help ids are unchanged. Only help.json `file` fields change.

## API / SDK surface

- No new API command. The dataset `index` description changes through the manifest, which
  updates the generated `DatasetApiFields.cpp`. `api-schema.json` must be re-dumped by the
  maintainer after building, and the SDK regenerated if its staleness check requires it.
- **Behaviour change:** an API `project.*` load or save while connected now defers the implicit
  mode switch instead of switching mid-session. API clients that want an immediate switch call
  the explicit mode command, which is unchanged. This is documented in API-Reference.md.

## QML / UI

- No new popups. The pending-mode notice uses NotificationCenter, the R5 hint is a console line,
  and the requirement/computed notices reuse the inline `ProNotice`-style banner. The motion
  contract is therefore untouched.
- Description labels in `Setup.qml` wrap within `maxItemWidth`, and the pane height follows
  `implicitHeight`.
- All new strings use `qsTr`/`tr`. Translation catalogues are not touched; the maintainer
  refreshes them.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Save-as-Project seam | (a) facade sub-object + `loadGeneratedProject`; (b) Ui-layer exporter via bus `GeneratedProjectRequest`; (c) mutate live model with `addGroup`/`addDataset` | **(a).** Atomic, dialog-safe, no new composition-root module, pure builder is unit-testable. (c) leaks undo steps and isn't atomic. |
| Mode-trap guard | (a) one `requestOperationMode` in AppState; (b) guards at QML call sites; (c) allow mid-session switch | **(a).** One home covers UI and API. (c) rebuilds devices under a live link, which is the constraint the spec keeps. |
| API load while connected | defer (uniform) vs keep immediate for API | **Defer.** One rule. Explicit mode command remains the escape hatch. Risk is covered by the integration suite. |
| Quick Plot CRLF / CR framing | single `frameEnd "\n"` + `trimFields` vs extending projects to multiple end delimiters | **`"\n"` + trim.** Covers LF and CRLF with no format change. Bare-CR devices are rare; documented in the tutorial and FAQ. |
| Generated layout | serialize Quick Plot's own groups vs a fresh single group | **Serialize Quick Plot's groups.** Gives the closest match to what the user saw (AC5). |
| Group widget rule home | new pure helper used by editor + checker vs editing the runtime guards | **Helper only.** Runtime draw code stays untouched. |
| Computed / requirement hints | QML notice banner vs extending the property generator with a hint field | **QML banner.** No generator or schema change for two hints. |
| Terminology gate | one new `term-verify.py` vs rules split across `documentation-verify.py` and `code-verify.py` | **One script, one list.** Docs, QML, C++ and corpus are all checked against the same banned/allow table. |
| Help renames | keep ids + new files + unregistered stubs vs register stubs as entries | **Unregistered stubs.** No duplicate sidebar entries. Old ids resolve in-app, and stubs cover raw-URL fetches. |
| Pro signalling | badges vs copy | Copy only, per spec Non-Goals. |

## Risks & mitigations

- **Editing in a non-Project mode:** `askSave` currently skips the prompt outside Project File
  mode. Fix it, and verify Quick Plot's auto workspaces don't mark the model modified, which
  would cause spurious prompts. Mitigation: walkthrough of quit / New / Open in each mode.
- **Load while connected in Quick Plot:** the project is loaded into the model while the
  pipeline still runs Quick Plot. Verify that no structure snapshot from the loaded project
  disturbs the live Quick Plot dashboard before the deferred switch. This is part of the AC4
  and AC5 walkthrough. If it does, defer the load as well as the mode.
- **Integration tests relying on mid-session implicit switch:** run `pytest tests/integration`
  after M2. Any failure is triaged as test-adjust (call the explicit mode command) vs design
  flaw.
- **Generated artifacts:** never hand-edit `Generated/*`, `CommandStrings.cpp` or
  `search_index.json`. The generators' `--check` modes run in CI.
- **Help renames:** older app builds default to the dev ref help.json. Ids are unchanged, so
  their in-app links still resolve. `HelpFetcher` 404s fall back to help.json, and stubs cover
  direct fetches.
- **Rename churn hitting identifiers:** term-verify allowlists code spans and identifiers, and
  the sweep agents are briefed "strings only". `tst_enum_labels` and `registry-verify.py` catch
  slug or id drift.
- **Translations:** changed source strings go stale in `.ts`. That is expected, and the
  maintainer refreshes them. `.ts`/`.qm` files are never touched.
- **ProjectModel TU growth:** the new logic lives in `QuickPlotProject.cpp`, and the facade only
  delegates. Run `code-verify.py --tu-census` to confirm.

## Test & verification plan

- **Unit (ctest, maintainer builds; I run against the existing build dir):**
  - new `tst_quickplot_project` (AC5): structure → JSON → reload. Checks framing, Built-In
    template, indices 1..N, titles, plots, and that `proContentSummary` is empty.
  - new `tst_group_widget_requirement` (AC8/AC10): the requirement table per widget.
  - existing `tst_enum_labels`, `tst_help_fetcher`.
- **Integration (maintainer, app up with API on 7777):**
  - new `tests/integration/test_problem_findings.py` (AC8). It loads a 3-index project, streams
    2-value frames and asserts a `short-frames` finding with a jump target per device. It
    asserts a `project.group-widget` finding for a 2-dataset Accelerometer, and that both
    clear when fixed.
  - the full `pytest tests/integration` after M2 (risk above).
- **Hotpath:** `--benchmark-hotpath` once after M3 (AC9).
- **Static:**
  - `scripts/syntax-check.py` on every edited C++ file.
  - `code-verify.py --check`.
  - `generate-property-registry.py --check`, `generate-command-strings.py` check.
  - `registry-verify.py`.
  - `term-verify.py` (AC11), `documentation-verify.py` (AC13), `claim-verify.py`.
  - `qt-cpp-review` / `qt-qml-review` before handoff.
- **Maintainer walkthroughs:**
  - AC1: launch in each edition state.
  - AC2: mode descriptions and disabled-reason tooltips.
  - AC3: silent TCP source gives the hint after 10 s.
  - AC4: editor with no overlay, plus the deferred switch notice.
  - AC5: Quick Plot vs saved project values.
  - AC7: parser test flags index 3 on a 2-value sample.
  - AC10: requirement line + warning, computed notice.
  - AC12: port label per OS.
  - AC14: tutorial end to end on hardware or the simulator.
  - AC15: GPL notice; commercial refusal unlicensed / expired.
- **AC6:** grep for the old index wording after regeneration and re-dump.

## Delegation

Main session: M2 and M3 C++, plus review gates. Sonnet subagents with half-page briefs:

- the M4 string sweep (after `term-verify.py` exists, so it measures them);
- the M5 cross-link and term sweep;
- the 35 example README openers;
- the stub pages.
