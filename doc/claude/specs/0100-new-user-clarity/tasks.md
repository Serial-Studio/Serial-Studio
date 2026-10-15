---
spec: 0100-new-user-clarity
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-10-15
---

# Tasks 0100 — New-User Clarity

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** for every C++ edit includes `python scripts/syntax-check.py <files>` and
  `python scripts/code-verify.py --check <files>`. These are not repeated per task below.
- **Deps** lists task IDs that must land first.
- Every user-visible string uses `qsTr`/`tr`. Never touch `.ts`/`.qm` or any `Generated/`
  file by hand.
- **[S]** marks tasks delegated to a Sonnet subagent with a half-page brief. All others stay in
  the main session.

## M1 — First run + Pro notice (R1–R6, R31)

### T1 — Console getting-started block

- **Files:** `core/Ui/UI/Widgets/Terminal.cpp`, `core/Ui/UI/Widgets/Terminal.h`,
  `core/Ui/Console/WelcomeText.cpp/.h` *(added in implementation: the logo, block and tip moved
  there so Terminal.cpp shrinks under its TU-census baseline)*
- **Does:** `loadWelcomeGuide()` prepends a `tr()` block of at most 12 lines: choose a device,
  set it up, press Connect; the Quick Plot format with a one-line example; where Examples and
  Help live. Step 3 is tailored to the current operation mode. The licence/edition text
  follows, unchanged.
- **Verify:** read-back of the line budget (≤ 12). AC1 walkthrough in each edition state.
- **Revision (maintainer, 2026-10-15):** the guide must fit 80x24. The block was dropped and the tip line now carries the start hint. All 84 edition texts were trimmed: the Learn-more link (GPL, unlicensed), the scheduling link (unlicensed), and the blank line under the copyright are gone, and paragraphs were reflowed to 78 display columns. A blank line separates the text from the tip, so the GPL texts also dropped the GNU licence URL line (the source link stays), and the German unlicensed paragraph was tightened by hand. Every edition and language was measured within 80x24.
- **Deps:** none
- [x] done

### T2 — No-data console hint

- **Files:** `core/Ui/UI/Widgets/Terminal.cpp`, `core/Ui/UI/Widgets/Terminal.h`, **new**
  `core/Ui/Console/SilentLinkWatch.h/.cpp`, `core/Ui/CMakeLists.txt` *(added: the timer lives
  in its own sub-object to keep Terminal.cpp under its TU baseline)*
- **Does:** A 10 s single-shot `QTimer` is armed on connect and stopped on disconnect. When it
  fires, if `Dashboard::available()` is false and the mode is not ConsoleOnly, the console
  appends one hint line. The hint says "no bytes received" or "bytes but no frames", using the
  link stats, and names the likely causes plus the Problem Center. Signal wiring: read the
  existing `connectedChanged` handler first; this adds no per-frame work (it is
  connection-rate only).
- **Verify:** AC3 walkthrough (silent TCP source, then a source sending garbage).
- **Deps:** T1
- [x] done

### T3 — Operation Mode group: labels and descriptions

- **Files:** `app/qml/MainWindow/Panes/Setup.qml`
- **Does:** The group label becomes "Operation Mode". The radios read "Console Only",
  "Quick Plot", "Project File", each followed by a wrapped description `Label` (R2/R3). Disabled
  radios and export toggles get reason tooltips (R4).
- **Verify:** `code-verify.py --check` on the QML. AC2 walkthrough.
- **Note:** Disabled reasons are shown as visible captions under the mode group and the export section rather than hover tooltips. Disabled Qt Quick controls receive no hover, so a caption is the only reliable way to show them.
- **Revision (maintainer, 2026-10-15):** the mode descriptions are radio tooltips, not inline labels.
- **Deps:** none
- [x] done

### T4 — Connect button: dynamic disabled reason

- **Files:** `app/qml/Commands/AppCommandBindings.qml`,
  `app/qml/MainWindow/Panes/Toolbar.qml`
- **Does:** `cmdIoToggleConnection` exposes a `tooltip` naming the blocking reason: no device
  selected or invalid settings, playback open, or mirror attached. Toolbar's Connect button uses
  `entry.tooltip` instead of its hard-coded string.
- **Verify:** AC2 walkthrough. `registry-verify.py`.
- **Note:** The Connect button is wrapped in an Item with a HoverHandler that stays enabled, because ToolbarButton's internal MouseArea is disabled with the button.
- **Deps:** none
- [x] done

### T5 — Problem Center on the main toolbar

- **Files:** `app/rcc/commands/layouts/main-toolbar.json`
- **Does:** Adds a `{ "type": "command", "id": "app.problems" }` item (R6).
- **Verify:** `python scripts/registry-verify.py`. Visible in the running app.
- **Note:** `app.problems` *replaces* `app.connectionDiagnostics` in the 3-row grid. Problem Center has its own Run Diagnostics button, and the palette still lists diagnostics.
- **Deps:** none
- [x] done

### T6 — Pro content summary property

- **Files:** `core/Pipeline/DataModel/ProjectModel.h`,
  `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:** Adds a read-only `QStringList commercialFeatureSummary`. Its notify signal is the
  existing `containsCommercialFeatures` one. It lists transforms and Variables tables (from
  `SerialStudio::proContentSummary`), Pro widgets, multi-device and Pro bus, as readable item
  phrases. No licensing logic is touched.
- **Verify:** syntax-check. Read-back that the notify signal fires wherever
  `containsCommercialFeatures` does.
- **Note:** The labels live in `core/Core/DataModel/FrameSupport.cpp` (`commercialFeatureLabels`), beside `groupNeedsCommercial`, so the label list and the notice trigger cannot drift. They mirror what actually drives the GPL notice (output groups, 3D, image, canvas, waterfall, notify transforms).
- **Deps:** none
- [x] done

### T7 — GPL fallback notice names the items

- **Files:** `app/qml/Widgets/ProNotice.qml`,
  `app/qml/ProjectEditor/Views/ProjectView.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** ProNotice gains an optional item list. ProjectView and DashboardCanvas pass
  `commercialFeatureSummary` and new copy: what needs Pro, what is shown instead, and "get the
  official build" (R31).
- **Verify:** AC15 (GPL half).
- **Note:** The shared title, subtitle and item line are now defaults in ProNotice.qml, so callers pass only `items` + `activationFlag`. OutputWidgetView keeps its own copy.
- **Deps:** T6
- [x] done

### T8 — GPL notice in the dataset and group views; verify commercial refusal

- **Files:** `app/qml/ProjectEditor/Views/DatasetView.qml`,
  `app/qml/ProjectEditor/Views/GroupView.qml`, `app/src/Misc/ProFeatureNotice.cpp` (read-only
  unless a gap is found)
- **Does:** Applies the same ProNotice copy in the dataset and group views. Confirms that the
  commercial connect refusal names the content and gives an action in the unlicensed and
  expired states. Any wording gap is named in chat before it is edited.
- **Verify:** AC15 (commercial half).
- **Note:** No edit. TrialGate's trial and expired prompts both append `contentDetail` and offer Start trial / Activate / Get Pro.
- **Deps:** T7
- [x] done

## M2 — Mode model + Save as Project (R7–R9)

### T9 — `AppState::requestOperationMode` with pending switch

- **Files:** `core/Pipeline/AppState.h`, `core/Pipeline/AppState.cpp`
- **Does:** New public slot `requestOperationMode(mode)`.
  - Disconnected: calls `setOperationMode` immediately.
  - Connected: stores `m_pendingMode` and posts a NotificationCenter notice.
  - A bus subscription to `ConnectionStateChanged` applies the pending mode on disconnect.
  - On quit, a pending mode is persisted as the mode.
  - **Invariant:** `m_operationMode` still changes only through `setOperationMode`, so its
    signal keeps feeding the cached hotpath flag. No startup-order change. The ctor adds no
    `instance()` reach (bus via the existing handle).
- **Verify:** syntax-check. `--singleton-census` unchanged.
- **Note:** Also added `targetOperationMode()` (used by T11). The pending mode is persisted at request time, so quitting before the disconnect keeps the switch.
- **Deps:** none
- [x] done

### T10 — Route model-level implicit switches through the guard

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- **Does:** `openJsonFile(path)`, `loadGeneratedProject` and `finalizeProjectSave` call
  `requestOperationMode`. The failure-restore path keeps the direct setter.
- **Verify:** syntax-check. Run `pytest tests/integration` (maintainer, app up) and triage any
  test that relied on a mid-session implicit switch.
- **Note:** The import path (`ProjectLoader.cpp:920`) and askSave's discard-restore calls (`ProjectPersistence.cpp:302/328`) go through the guard too. **Pending:** the maintainer runs `pytest tests/integration`.
- **Deps:** T9
- [x] done

### T11 — `askSave` never silently discards editor changes

- **Files:** `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- **Does:** *(Amended.)* `askSave` uses `AppState::targetOperationMode()`, which returns the
  pending mode if one is set and the current mode otherwise. The early return (`:288`) then no
  longer discards a modified project waiting on a deferred switch. Files add
  `core/Pipeline/AppState.h/.cpp` for the accessor.
- **Verify:** walkthrough of New, Open and Quit with unsaved edits in each mode. No spurious
  prompts.
- **Deps:** T10
- [x] done

### T12 — Route UI-level implicit switches

- **Files:** `app/src/Misc/FileOpenEventFilter.cpp`, `app/qml/main.qml`,
  `app/qml/Commands/AppCommandBindings.qml` (plus the `FileDropArea.qml` call site)
- **Does:** Example download, file-open event, drag-drop and Open Project call
  `Cpp_AppState.requestOperationMode` instead of setting the mode directly. Explicit `mode.*`
  commands and radios are unchanged.
- **Verify:** `code-verify.py --check`. AC4 walkthrough.
- **Note:** `FileOpenEventFilter.cpp` lives in `core/Ui/Platform/`, not in `app/src/Misc/`.
- **Deps:** T9
- [x] done

### T13 — Remove the editor mode overlay

- **Files:** `app/qml/ProjectEditor/ProjectEditor.qml`,
  `app/qml/Commands/ProjectEditorCommandBindings.qml`
- **Does:** *(Amended.)* Opening the editor while disconnected requests Project File mode,
  with a notice. The mode overlay is visible only while connected in another mode, with
  reworded copy and a "Disconnect and Edit" button. The lock overlay is unchanged. Editor "New"
  also requests Project File mode. `newJsonFile()` itself stays mode-neutral.
- **Verify:** AC4 walkthrough from Quick Plot and Console Only, both connected and disconnected.
- **Note:** Editor "New" needs no change, because it can only run while the editor is interactive, which already means Project File mode.
- **Deps:** T11, T12
- [x] done

### T14 — Quick Plot → project JSON builder

- **Files:** **new** `core/Pipeline/DataModel/Project/QuickPlotProject.h`, **new**
  `QuickPlotProject.cpp`, the Pipeline target source list in CMake
- **Does:** A pure static `build(groups, busType, connSettings) → QJsonObject`. It produces one
  device:
  - `EndDelimiterOnly`, `frameEnd "\n"`, PlainText.
  - The Built-In `delimited` template with `{separator:",", trimFields:true}`.
  - The supplied connection settings.
  - Quick Plot's groups serialized with the existing serialization (indices 1..N, titles,
    plots).

  No Pro content.
- **Verify:** syntax-check. T15's unit test.
- **Deps:** none
- [x] done

### T15 — `tst_quickplot_project` unit test

- **Files:** **new** `app/tests/tst_quickplot_project.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Feeds a 3-channel structure with headers. Asserts framing, template, params,
  indices 1..N, titles, plot flags, and that `proContentSummary` is zero. Round-trips the JSON
  through the project loader.
- **Verify:** maintainer builds, then `ctest -R tst_quickplot_project` against the build dir.
- **Note:** The round trip goes through the `DataModel::read` readers rather than the full loader, which would need the pipeline. **Pending:** maintainer build + ctest.
- **Deps:** T14
- [x] done

### T16 — `ProjectModel::saveQuickPlotAsProject`

- **Files:** `core/Pipeline/DataModel/ProjectModel.h`,
  `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:**
  - New public slot `saveQuickPlotAsProject()` and a `quickPlotSaveAvailable` property
    (QuickPlot mode, a frame seen, bus ≠ Audio).
  - Copies `quickPlotFrame().groups` via `invokeOnBuilderThreadBlocking`.
    **Invariant:** command-rate only, never from a script API, and no `lua_*` inside.
  - Reads `bus.latest<ActiveUiDriverSettings>()`, then `QuickPlotProject::build` →
    `loadGeneratedProject` → `saveJsonFile(true)`. The dialog is queued; follow the macOS
    reentrancy rule.
- **Verify:** syntax-check. `code-verify.py --tu-census` shows no facade growth beyond the
  delegate.
- **Note:** The `quickPlotSaveAvailable` C++ property was dropped. Availability is a QML binding (Quick Plot mode and connected), and the slot re-checks the mode and the structure. Revised 2026-10-15: the first binding used `Cpp_UI_Dashboard.available`, which never re-notified on stream changes. The Audio exclusion was also dropped: Audio emits comma-separated lines, and the builder now copies whole datasets, so the FFT rate, size and ranges carry over.
- **Deps:** T10, T14
- [x] done

### T17 — "Save as Project" command and button

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`,
  `app/qml/MainWindow/Panes/Setup.qml`
- **Does:** New command `quickplot.saveAsProject`, bound to `saveQuickPlotAsProject()` and
  enabled by `quickPlotSaveAvailable`, with a tooltip explaining why it is disabled. The button
  sits under the Quick Plot radio and the command is also in the palette. Re-run
  `scripts/generate-command-strings.py`.
- **Verify:** `registry-verify.py`. AC5 walkthrough (same values before and after reconnect;
  connects unlicensed).
- **Revision (maintainer, 2026-10-15):** Save as Project is a dashboard start-menu entry (`start-menu.json` + `DashboardCommandBindings.qml` + `StartMenu.qml`), not a Setup-pane button. The Quick Plot structure exists only once the dashboard is populated. Second revision: the save dialog comes first. Only an accepted path disconnects (bus `DisconnectRequested`, delivered synchronously), switches to Project File explicitly, loads the generated project in that mode (so auto workspaces and framing derive), and writes it through `finalizeProjectSave`. It never reconnects. The builder also writes `customizeWorkspaces: false`.
- **Deps:** T3, T16
- [x] done

## M3 — Parser contract + Group Widget rules (R10–R16)

### T18 — Frame index description

- **Files:** `app/rcc/properties/dataset.json`, regenerated outputs of
  `scripts/generate-property-registry.py`
- **Does:** The description becomes "Position of this value in the parser's output (first
  value = 1). 0 = unassigned." Regenerate. The maintainer re-dumps `api-schema.json` after the
  build.
- **Verify:** `generate-property-registry.py --check`. AC6 grep after the re-dump.
- **Note:** **Pending:** the maintainer re-dumps `app/rcc/api/api-schema.json` (`--dump-api-schema`) after building. The checked-in snapshot still carries the old wording until then (AC6).
- **Deps:** none
- [x] done

### T19 — Parser test: index-to-dataset map

- **Files:** `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp`,
  `core/Ui/ProjectEditor/Editors/FrameParserModel.h`
- **Does:** `emitPreview` adds a `datasets` map (frame index → titles for this device's
  enabled, non-virtual datasets) and `maxIndex` to the payload. The dataset walk runs on the GUI
  thread.
- **Verify:** syntax-check.
- **Note:** Exposed as `Q_INVOKABLE datasetIndexMap()` instead of a new `previewReady` argument, so the existing QML handlers keep their signature.
- **Deps:** none
- [x] done

### T20 — Parser test dialog shows names and flags

- **Files:** `app/qml/Dialogs/FrameParserTest.qml`
- **Does:** Cells read `[n] Title`. Indices above a row's length get flagged rows, "[n] Title:
  not in this frame" (R11).
- **Verify:** AC7 walkthrough.
- **Deps:** T19
- [x] done

### T21 — `short-frames` finding: every device + Go To

- **Files:** `core/Ui/Misc/Problems/LinkCheckers.cpp`
- **Does:** `reportShortFrames` loops over all offending stats and emits one finding per
  device, with `entityUniqueId` = device and `jump` = first starved dataset (`"dataset"`).
  `makeLinkFinding` gains optional jump arguments. The sustain counter becomes per-device. This
  is read-only over existing `shortFrameStats()` on the 1 Hz tick, so `FrameBuilder` is not
  edited.
- **Verify:** syntax-check. Covered by T25.
- **Note:** One sustain counter for "any source short", then one finding per offending source.
- **Deps:** none
- [x] done

### T22 — Group widget requirement helper + unit test

- **Files:** `core/Core/SerialStudio.h`, `core/Core/SerialStudio.cpp`, **new**
  `app/tests/tst_group_widget_requirement.cpp` (+ CMakeLists)
- **Does:** A pure `groupWidgetRequirement(GroupWidget) → {minCount, maxCount, roles}`. Confirm
  the GPS rule against `DashboardIngest.cpp` and `GPS.cpp` first, and name the result in chat.
  The runtime widget guards are untouched.
- **Verify:** `ctest -R tst_group_widget_requirement`.
- **Note:** The dashboard binds by role **tag** (`x/y/z`, `pitch/roll/yaw`, `lat/lon/alt`), not by order, so the spec's "in this order" wording does not apply. The requirement checks count + tags. GPS = lat + lon required, alt optional. `groupWidgetRequirementText()` is the single translated sentence.
- **Deps:** none
- [x] done

### T23 — Problem Center: narrowed duplicate rule + group-widget checker

- **Files:** `core/Ui/Misc/Problems/ProjectCheckers.cpp`
- **Does:** The duplicate-index key includes the dataset widget type (R13). The new
  `project.group-widget` checker uses `groupWidgetRequirement` and checks count plus X/Y/Z
  roles, with a group jump (R16).
- **Verify:** syntax-check. Covered by T25.
- **Note:** The duplicate key is (source, index, group widget, dataset widget). The shared `SerialStudio::groupMeetsWidgetRequirement(group)` lives in FrameSupport.cpp.
- **Deps:** T22
- [x] done

### T24 — Group editor requirement line and computed-dataset notice

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/ProjectEditor.cpp`,
  `app/qml/ProjectEditor/Views/GroupView.qml` (+ `DatasetView.qml`)
- **Does:**
  - `Q_INVOKABLE groupRequirementText()` / `groupRequirementMet()` feed a requirement line and a
    warning banner in GroupView (R15).
  - DatasetView shows the computed explanation when the dataset is virtual (R14).
- **Verify:** AC10 walkthrough.
- **Note:** `ProjectEditor.cpp` sits at 1500 lines, so the getters are inline delegates to new `EditorSelection` methods. The properties are `selectedGroupRequirementMet/Text` and `selectedDatasetComputed`.
- **Deps:** T22
- [x] done

### T25 — Integration test for the new findings

- **Files:** **new** `tests/integration/test_problem_findings.py`
- **Does:**
  - A 3-index project fed 2-value frames gives a `short-frames` finding with a dataset jump.
  - A 2-dataset Accelerometer gives `project.group-widget`.
  - Duplicate indices with different widgets give no warning.
  - All findings clear when fixed.
- **Verify:** maintainer runs it with the app up on 7777 (AC8).
- **Note:** The tests are appended to the existing `tests/integration/test_problem_center.py`, reusing its helpers, instead of a new file. **Pending:** maintainer run.
- **Deps:** T21, T23
- [x] done

### T26 — Problem Center page + hotpath guard run

- **Files:** `doc/help/Problem-Center.md`
- **Does:** Documents `short-frames`, the narrowed duplicate rule and `project.group-widget`.
  The maintainer runs `--benchmark-hotpath` at this clean point (AC9).
- **Verify:** `documentation-verify.py`. The benchmark passes all gates.
- **Note:** **Pending:** maintainer `--benchmark-hotpath` run (AC9). No hotpath file was edited.
- **Deps:** T21, T23
- [x] done

## M4 — One name per concept (R17–R21)

### T27 — `term-verify.py` gate

- **Files:** **new** `scripts/term-verify.py`, **new** `scripts/term-rules.json`,
  `.github/workflows/docs.yml`, `.github/workflows/ci.yml` (lint step),
  `doc/claude/scripts.md`
- **Does:** Holds the banned-synonym and allowlist table from the plan. Scans `qsTr`/`tr`
  literals, `app/rcc/commands/**`, `doc/help`, `README.md`, example READMEs and
  `app/rcc/ai/**` (excluding `search_index.json`). Exits 1 on any hit. CWD-independent, LF
  output.
- **Verify:** It runs and reports today's baseline hits, which become T28–T31's worklist.
- **Note:** Baseline at creation was 218 hits (QML 17, C++ 15, JSON 14, docs 161, corpus 11).
- **Deps:** none
- [x] done

### T28 — [S] QML string renames

- **Files:** the QML files in the plan's M4 table
- **Does:** Applies the string table (Connection, Devices, Variables Table, Variable access
  code, Bar), the platform-conditional "COM Port"/"Serial Port", and tooltips on Alarm Sequence,
  Send DTR Signal and Data Conversion Method. Strings only, no identifiers.
- **Verify:** `term-verify.py` clean for QML. `code-verify.py --check`.
- **Note:** Done by a Sonnet subagent and reviewed. TriggerDialog's "Source:" is the Multi-Plot X-source, not a device, so it was left as is.
- **Deps:** T27, T3
- [x] done

### T29 — C++ `tr()` and command-manifest renames

- **Files:** `EditorForms.cpp`, `ProjectSources.cpp`, `ProjectModel.cpp`, `ProjectTables.cpp`,
  `ProjectFolders.cpp`, `EnumLabels.cpp`, `SerialStudio.cpp`,
  `app/rcc/commands/projecteditor.json`, `layouts/project-toolbar.json`
- **Does:** Renames per the table. `EnumLabels` changes labels only, never slugs. Regenerate
  `CommandStrings.cpp`.
- **Verify:** syntax-check. `ctest -R tst_enum_labels`. `registry-verify.py`. `term-verify.py`
  clean for C++/JSON.
- **Note:** EnumLabels and the API descriptions are left in API vocabulary (spec R18 amendment). "Add Data Table" became "Add Data Grid" (the widget). Two pre-existing `&m_editor` typos in user strings (`EditorForms.cpp`, `EditorSummaries.cpp`) were fixed, because they sat in strings this task was already rewording.
- **Deps:** T27
- [x] done

### T30 — [S] Assistant corpus terms

- **Files:** `app/rcc/ai/skills/*.md`, `app/rcc/ai/docs/*.md`, regenerated
  `app/rcc/ai/search_index.json`
- **Does:** User names in prose, API names in code spans. Corpus file names are unchanged.
  Regenerate via `build_search_index.py`.
- **Verify:** `term-verify.py` clean for the corpus.
- **Note:** Done by a Sonnet subagent; `search_index.json` was regenerated.
- **Deps:** T27
- [x] done

## M5 — Manual + examples (R22–R30)

### T31 — Help file renames, stubs and referrers

- **Files:** `doc/help/help.json`, the 6 renamed pages plus 6 stubs at the old names,
  `core/Ui/AI/Conversation/HelpFetcher.cpp`, `MetaToolCatalog.cpp`,
  `app/qml/ProjectEditor/Views/ControlScriptView.qml`,
  `app/rcc/ai/skills/control_script.md`
- **Does:** `git mv` to Variables, Canvas-Widget, Frame-Parser, Control-Loop, Historian and
  Devices, with **ids unchanged**. Stubs are unregistered one-line pointers. Code and corpus
  references move to the new names.
- **Verify:** `ctest -R tst_help_fetcher`. Help Center opens each old id in-app.
- **Note:** `Data-Sources.md` was renamed to `Devices.md` (title "Devices") along with the other five. ControlScriptView's `helpPage` now uses the id `control-script`. `tool_discovery.md` lists `Devices`. **Pending:** maintainer `ctest -R tst_help_fetcher`.
- **Deps:** none
- [x] done

### T32 — Doc link and registration checks

- **Files:** `scripts/documentation-verify.py`
- **Does:** Checks that every non-stub page is in help.json, every relative `.md` link resolves,
  and every stub targets a registered page.
- **Verify:** It runs and lists the broken links T33 must fix.
- **Note:** Added a fourth kind, `help-link-to-stub`, because the stubs make old links "resolve". It found 129 such links, all rewritten by script.
- **Deps:** T31
- [x] done

### T33 — [S] Manual cross-link and terminology sweep

- **Files:** ~46 `doc/help/*.md`, `README.md`, `CLAUDE.md` and
  `doc/claude/architecture/scripting.md` (page-name references only)
- **Does:** Updates links to the new filenames and applies the canonical terms. API Reference
  keeps API vocabulary. One Pro convention in titles and section listings (R29). Linux
  permission guidance is centralised in `Linux-Installation.md`, with links elsewhere.
- **Verify:** `term-verify.py`, `documentation-verify.py` and `claim-verify.py` clean.
- **Note:** Sonnet subagent, reviewed. "(Pro)" suffixes went to whole-feature Pro pages and the Pro drivers. The FAQ anchor became `#devices-and-protocols`. Left: the transform template name "Calibration from Shared Table" in `app/rcc/scripts/transforms/templates.json` (outside the plan, per-language strings) is named in chat.
- **Deps:** T27, T32
- [x] done

### T34 — help.json restructure

- **Files:** `doc/help/help.json`
- **Does:** Concepts after Getting Started. Glossary into Basics. Identity Model after Project
  Editor. Support splits into "Support" and "About". Pro marking on titles.
- **Verify:** `documentation-verify.py` registration check.
- **Deps:** T31, T35
- [x] done

### T35 — Concepts page and Glossary API mapping

- **Files:** **new** `doc/help/Concepts.md`, `doc/help/Glossary.md`
- **Does:** The chain diagram plus definitions linking each reference page (R22). The Glossary
  maps API terms (`source`, `table`, `painter`, `Native`, `sessions`) to user names (R18). Run
  through the ss-docs procedure.
- **Verify:** `documentation-verify.py`. ss-docs tell-check.
- **Deps:** T31
- [x] done

### T36 — Getting Started and Troubleshooting

- **Files:** `doc/help/Getting-Started.md`, `doc/help/Troubleshooting.md`
- **Does:**
  - Getting Started links Concepts and Glossary first, states the modes once (with the new
    labels and Save as Project), drops the duplicated install steps, and tags Pro features
    (R24).
  - Troubleshooting gains the "wrong number of values / frozen widget" entry tied to
    `short-frames` (R27).
- **Verify:** `documentation-verify.py`. Tell-check.
- **Deps:** T35, T17, T21
- [x] done

### T37 — Tutorial example

- **Files:** **new** `examples/Project Tutorial/` (`tutorial.ino`, `tutorial_sim.py`,
  `tutorial.ssproj`, `README.md`), `examples/examples.json`
- **Does:** A `$t,h,ax,ay,az;` frame with start/end delimiters, a Data Grid group and an
  Accelerometer group (X/Y/Z), and a UDP simulator for the no-hardware path. Beginner level, no
  Pro content.
- **Verify:** `pytest`-free check: the simulator runs and the project loads in the app
  (walkthrough).
- **Note:** The project file was generated from the MPU6050 example's dataset shape. It omits uniqueIds so the loader assigns them. Category "Getting Started", placed first in `examples.json`.
- **Deps:** T22
- [x] done

### T38 — Project File tutorial and Derived Values guide

- **Files:** **new** `doc/help/Project-File-Tutorial.md`, **new** `doc/help/Derived-Values.md`,
  `doc/help/help.json`
- **Does:**
  - The tutorial walks T37's example to the exact dashboard the reader should see (R25).
  - The decision guide covers the five mechanisms, with one example each and Pro status (R26).
  - Both pages are registered.
- **Verify:** `documentation-verify.py`. Tell-check. AC14 walkthrough.
- **Deps:** T37, T34
- [x] done

### T39 — [S] Example README openers and learning order

- **Files:** `examples/*/README.md` (35), `examples/README.md`
- **Does:** A "What you'll learn / Prerequisites" opener on each README. A beginner → advanced
  learning-order section in the index (R30).
- **Verify:** a script check that every README has the opener. `term-verify.py`.
- **Note:** Sonnet subagent. It flagged an existing mismatch: the README table marks API Test as Pro but `examples.json` says `requiresPro: false`. Left for the maintainer.
- **Deps:** T37
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` (AC1–AC15) is met and checked off there.
- [x] `code-verify.py --check` is clean on all changed C++/QML, with no new errors.
  `--tu-census` shrank (baseline re-seeded) and `--singleton-census` is unchanged.
  `syntax-check.py` was SKIPPED: there is no compile database under `build/`, so
  compile-correctness is unverified until the maintainer builds.
- [ ] Generators' `--check` modes are green (property registry, command strings, search index).
  The maintainer has re-dumped `api-schema.json`.
- [x] `term-verify.py`, `documentation-verify.py`, `claim-verify.py` and `registry-verify.py`
  are clean.
- [x] `qt-cpp-review` has been run (6 agents; 6 confirmed findings fixed: stale pending mode overridden by an explicit choice, rollback losing a prior pending mode, discard path leaving a pending mode, connecting-state toast race, `default:` over GroupWidget, hand-built plural; unused include removed). `qt-qml-review` has not been run.
- [ ] `--benchmark-hotpath` passes all gates (T26).
- [ ] The maintainer has run `ctest` (`tst_quickplot_project`, `tst_group_widget_requirement`,
  `tst_enum_labels`, `tst_help_fetcher`) and `pytest tests/integration` (including
  the new spec-0100 tests in `test_problem_center.py`).
- [x] `python scripts/sanitize-commit.py` has been run.
- [x] The diff is what was asked and only that. No `.ts`/`.qm` touched, no foreign files.
- [x] CLAUDE.md is updated for the architectural additions: `requestOperationMode` as the
  implicit-switch rule, and term-verify as a gate.
- [ ] `spec.md` status is set to `done`.
