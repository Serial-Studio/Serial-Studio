---
spec: 0114-led-panel-group-widget
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-11-07
---

# Tasks 0114 — LED Panel as a Group Widget

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
- Every C++ edit runs `scripts/syntax-check.py` (the hook does it per edit; a SKIP means
  "unverified", say so). Line numbers in plan.md are hints: re-read the slice first.
- The `Dataset::led` field survives until T17 so every earlier task compiles; T17 is the one
  step that breaks compilation on purpose and lands with the generated artifacts in one commit.

## Tasks

### Phase A — enum, resolution, helper

### T1 — `GroupWidget::LEDPanel` and its slug

- **Files:** `core/Core/SerialStudio.h`, `core/Core/SerialStudio.cpp`, `core/Core/EnumLabels.cpp`
- **Does:** Append `LEDPanel` after `BarPanel` in `GroupWidget` (ordinal 11 leaks into the
  JSON API, so append only). `groupWidgetId` → `"ledpanel"`, `groupWidgetFromId` accepts it,
  `groupWidgetRequirement` returns `{1, -1, {}}` with matching text, every `BarPanel` switch
  site gains the case. `EnumLabels`: label tr("LED Panel"), slug `ledpanel`, parse also accepts
  `led-panel` like `bar-panel`. `DashboardLED = 8` and `DatasetLED` untouched here.
- **Verify:** `python scripts/code-verify.py --check` on the three files; syntax-check.
- **Deps:** none
- [x] done

### T2 — Resolution and the one band rule

- **Files:** `core/Pipeline/DataModel/WidgetResolution.cpp`
- **Does:** `getDashboardWidget(Group)`: `"ledpanel"` → `DashboardLED` beside `barpanel`.
  `getDashboardWidgets(Dataset)`: delete the `dataset.led` branch (LED datasets stop producing
  a panel from here on; T5 removes the builder that consumed them). `datasetRendersAlarmBands`:
  first line becomes `key == DashboardBarPanel || key == DashboardLED`, the dataset line drops
  `DashboardLED`. **Invariant named:** applicability has ONE home; no string compare, no second
  copy elsewhere (spec 0093).
- **Verify:** code-verify; syntax-check; read the `@brief` and update it to name both panels.
- **Deps:** T1
- [x] done

### T3 — `DataModel::deriveLedPanelGroup` helper with its unit

- **Files:** `core/Core/DataModel/LedPanelGroup.h`, `core/Core/DataModel/LedPanelGroup.cpp`,
  `core/Core/CMakeLists.txt`, `app/tests/tst_led_panel_group.cpp`, `app/tests/CMakeLists.txt`
- **Does:** `Group deriveLedPanelGroup(const Group& source, const std::vector<int>& positions)`:
  value-copies the named datasets, clears `outputWidgets`, keeps `index`, sets `uniqueId = -1`,
  `widget = "ledpanel"`, `groupType` of the source, title
  `QCoreApplication::translate("DataModel", "%1 LEDs")`; empty `positions` returns a group with
  no datasets (callers skip it). SPDX header, `SS_ASSERT` density, `[[nodiscard]]`. The ctest
  covers copy fidelity, stripped outputs, title, uid sentinel, out-of-range position ignored.
  New TU: borrow a same-library donor's compile entry for the syntax check.
- **Verify:** code-verify; syntax-check on the new TU via the donor entry; `ctest -R
  tst_led_panel_group` once the maintainer builds.
- **Deps:** T1
- [x] done

### Phase B — remove every reader of the flag (field still exists)

### T4 — Loader migration

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.h`,
  `core/Pipeline/DataModel/Project/ProjectLoader.cpp`, plus one whitelist line in
  `scripts/code-verify.py` (the undo-scope lint's own mechanism for a load path that calls
  `setModified(true)`; `loadGeneratedProject` is the precedent). Added during implementation.
- **Does:** `migrateLegacyLedFlags(const QJsonObject& json)` in the "Legacy migrations" block:
  walk raw `groups[g].datasets[i]`, match loaded datasets by position verified against `title`
  and `index` (fall back to the pair when counts differ), collect positions where `led` is true
  and `hideOnDashboard` is false, insert `deriveLedPanelGroup(...)` right after the source,
  `uniqueId`s from `m_model.allocateUniqueId()`, `renumberGroupIds()`, `datasetId` by position.
  Call site after `migrateLegacyWaterfallYAxisIds()` and before `rebindWidgetRefs()`. New
  `DocumentLoadFlags::ledPanelsMigrated`; in `loadFromJsonDocument` call `setModified(true)`
  after the existing `setModified(false)` and before autosave resumes, and post one info
  notification shaped like the older-schema notice. **Invariants named:** `kSchemaVersion`
  stays 3 (the older-schema block clears workspaces); insertion runs after the positional
  legacy-id migrations; nothing here is inside a `ProjectUndoScope`.
- **Verify:** code-verify (function ≤ 80 lines, split a raw-walk helper if needed);
  syntax-check; read-back of the call order against plan.md "Data model & persistence".
- **Deps:** T3
- [x] done

### T5 — Dashboard assembly stops synthesizing panels

- **Files:** `core/Ui/UI/Dashboard/WidgetMapBuilder.h`, `core/Ui/UI/Dashboard/WidgetMapBuilder.cpp`,
  `core/Ui/UI/DashboardWidget.cpp`
- **Does:** Drop the `ledPanel` out-parameter from `processDatasetIntoWidgetMaps`, the
  collection branch, the phantom-group build in `buildWidgetGroups`, and the `"led-panel"`
  special case in `applyDisplayTitles`. `builtinWidgetId(DashboardLED)` → `"ledpanel"`.
  **Invariant named:** rebuild rate only; the bucket maps' `operator[]` inserts stay as they are.
- **Verify:** code-verify; syntax-check; grep `led-panel` in `core/Ui/UI/Dashboard` returns
  nothing.
- **Deps:** T2
- [x] done

### T6 — Extension reserved ids

- **Files:** `core/Ui/UI/WidgetExtensions.cpp`, `app/rcc/extensions/schema/widget-manifest.json`
- **Does:** Add `ledpanel` to `reservedIds()` and the schema enum; keep `led-panel` reserved.
- **Verify:** `python scripts/registry-verify.py` (it diffs the two lists).
- **Deps:** T5
- [x] done

### T7 — Workspace plumbing loses its LED special cases

- **Files:** `core/Pipeline/DataModel/Project/ProjectWorkspaceRefs.cpp`,
  `core/Pipeline/DataModel/Project/WorkspaceKeys.cpp`,
  `core/Pipeline/DataModel/Project/ProjectWorkspaces.cpp`
- **Does:** Delete the LED aggregate anchor, `groupHasLed`, the `isLedPanel` lookup entry, the
  LED dataset-ref returns and the `pushTrackedRef(DashboardLED)`; a `ledpanel` group rides the
  existing group path. Re-read each slice: the plan's line numbers are hints.
- **Verify:** code-verify; syntax-check; grep `DashboardLED\|\.led\b` in the three files is
  empty.
- **Deps:** T2
- [x] done

### T8 — Workspace API handler and editor summaries

- **Files:** `core/Ui/ApiHandlers/WorkspacesHandler.cpp`, `core/Ui/ProjectEditor/EditorSummaries.cpp`
- **Does:** Delete the `isLed` branches and the `ds.led` loop in the relative-index code, the
  LED filter, and the LED summary row; the unlock hint says to set the group widget to
  `ledpanel`.
- **Verify:** code-verify; syntax-check.
- **Deps:** T7
- [x] done

### T9 — Importers derive an LED Panel group

- **Files:** `core/Pipeline/DataModel/Importers/ImporterCommon.h`,
  `core/Pipeline/DataModel/Importers/ProtoImporter.cpp`,
  `core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp`,
  `core/Pipeline/DataModel/Importers/DBCImporter.cpp`
- **Does:** Delete `hasLed`; each importer collects boolean-field positions per group and
  appends `deriveLedPanelGroup` after the group when non-empty, keeping `ledHigh = 1` and
  `wgtMax = 1` on the copies; nothing sets `led`.
- **Verify:** code-verify; syntax-check; `tst_proto_importer` expectation moves in T22.
- **Deps:** T3
- [x] done

### T10 — Driver project generators, PLC family

- **Files:** `core/Devices/IO/Drivers/S7.cpp`, `core/Devices/IO/Drivers/Iec104.cpp`,
  `core/Devices/IO/Drivers/EthernetIp.cpp`
- **Does:** `datasetFor`-style helpers stop setting `led`; the group builder appends the
  derived LED Panel group after a group with boolean tags. Same copy rule as T9.
- **Verify:** code-verify; syntax-check.
- **Deps:** T3
- [x] done

### T11 — Driver project generators, OPC UA / Sparkplug / Modbus

- **Files:** `core/Devices/IO/Drivers/OpcUa/OpcUaProjectBuilder.cpp`,
  `core/Devices/IO/Drivers/MQTT/MQTTSparkplug.cpp`,
  `core/Devices/IO/Drivers/Modbus/ModbusProjectGenerator.cpp`
- **Does:** Same as T10.
- **Verify:** code-verify; syntax-check; `tst_opcua_project_builder` and
  `tst_modbus_generation` expectations move in T22.
- **Deps:** T3
- [x] done

### T12 — Project model entities and hooks

- **Files:** `core/Pipeline/DataModel/Project/ProjectEntities.cpp`,
  `core/Pipeline/DataModel/Project/PropertyHooks.h`,
  `core/Pipeline/DataModel/Project/PropertyHooks.cpp`
- **Does:** `acceptsDatasets`, `compatibleTarget/Source`, `applyGroupWidget`/`addGroup` admit
  `LEDPanel`; delete the `DatasetLED` add case and option toggle. `ledEnabled` → the dataset's
  group resolves to `DashboardLED` through `getDashboardWidget` (never a string compare);
  `ledBandsAbsent` unchanged; the `barpanel` free-edit predicate also admits `ledpanel`.
  **Invariant named:** mutators here are two-phase undo (`ProjectUndoScope` + first
  `setModified(true)`); the deleted toggle must not leave a half-staged scope.
- **Verify:** code-verify (`undo-scope-missing`); syntax-check.
- **Deps:** T1
- [x] done

### T13 — Project Editor C++

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`,
  `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/ProjectEditor/EditorMultiSelect.cpp`,
  `core/Ui/ProjectEditor/EditorVocabulary.cpp`
- **Does:** Editability gates admit `ledpanel`; drop the `DatasetLED` option read;
  `kWidgetEnumMap` gains `ledpanel`; the band prefill from `ledHigh` gates on the group widget
  via `getDashboardWidget`; delete the multi-select LED toggle; `m_groupWidgets` gains
  `ledpanel` → tr("LED Panel").
- **Verify:** code-verify; syntax-check; `registry-verify.py` (every `groupWidgets` entry must
  be an `EditorWidget`; add the enum value only if it reports one missing).
- **Deps:** T12
- [x] done

### T14 — API handlers

- **Files:** `core/Api/API/Handlers/ProjectGroupCommands.cpp`,
  `core/Api/API/Handlers/ProjectDatasetCommands.cpp`,
  `core/Api/API/Handlers/ProjectApiSupport.cpp`
- **Does:** `widgetType` bound → `LEDPanel`, help text `11=LEDPanel`; remove bit 32 from the
  option bitmask handling, the `enabledOptions` strings and help text, other bits keep their
  values. **Invariant named:** no new command, so no capability or sealed-policy entry; a
  removed option is the ordinary unknown-property refusal (spec R9).
- **Verify:** code-verify; syntax-check; `ctest -R "tst_command_capability_registry|tst_sealed_policy_registry"`
  after the build (nothing should move).
- **Deps:** T1
- [x] done

### T15 — Assistant and MCP slug lists

- **Files:** `core/Ui/ApiHandlers/AssistantHandler.cpp`, `core/Ui/AI/Tools/ToolSchemas.cpp`,
  `core/Ui/AI/Tools/ToolCompact.cpp`, `core/Ui/AI/Tools/ToolSupport.cpp`,
  `core/Ui/AI/ContextBuilder.cpp`
- **Does:** `led` leaves the dataset-widget slug lists; `ledpanel` joins the group-widget text.
  Read `doc/claude/architecture/ai.md` "The Tier Model" first; no tier changes, strings only.
- **Verify:** code-verify; syntax-check; `tst_tool_schemas`/`tst_tool_compact` updated in T22.
- **Deps:** T1
- [x] done

### T16 — Problems checker and Painter bridge

- **Files:** `core/Ui/Misc/Problems/ProjectCheckers.cpp`, `core/Ui/UI/Widgets/PainterDataBridge.h`,
  `core/Ui/UI/Widgets/PainterDataBridge.cpp`, `core/Ui/UI/Widgets/Painter.cpp`
- **Does:** `checkLedThreshold` keys on the owning group resolving to `DashboardLED`; delete
  the `hasLed`/`led` getter from the bridge and the JS prelude, keep `ledHigh`.
- **Verify:** code-verify; syntax-check.
- **Deps:** T2
- [x] done

### Phase C — the registry cut

### T17 — Remove `Led` from the registry, regenerate, drop the field

- **Files:** `app/rcc/properties/dataset.json`, `app/rcc/properties/declaration.json`,
  `core/Core/DataModel/Frame.h`, `core/Core/DataModel/FrameKeys.h`,
  `app/src/Benchmark/HotpathBenchmark.cpp`, `core/Core/SerialStudio.h`, `core/Core/EnumLabels.cpp`,
  plus the regenerated: `core/Pipeline/DataModel/Generated/DatasetRegistry.h`,
  `core/Core/DataModel/Generated/DatasetSerialization.cpp`,
  `core/Ui/ProjectEditor/Generated/DatasetForm.cpp`, `core/Api/API/Generated/DatasetApiFields.cpp`,
  `core/Pipeline/DataModel/Generated/DeclarationVocabulary.h`, `firmware/c/ss_vocabulary.h`,
  `firmware/arduino/src/ss_vocabulary.h`, `firmware/python/serialstudio/_vocabulary.py`,
  `doc/grpc/serialstudio-typed.proto`, `app/rcc/api/proto-fields.json`
- **Does:** Manifest: remove `Led` from `formIdOrder`, the `led` builder rows and `properties`,
  retitle the section "LED Panel", fix the note at :62; `groupWidgets` += `ledpanel`. Run
  `scripts/generate-property-registry.py`. Delete `Dataset::led`; annotate `Keys::LED` as the
  legacy key the migration peeks at; drop the bench project's `Keys::LED` write; delete the
  `DatasetLED` enumerator and its `EnumLabels` rows (no readers remain after Phase B). One
  commit with the generated artifacts. **Invariant named:** never hand-edit a generated file;
  gRPC 21 moves to `reserved`, nothing renumbers.
- **Verify:** `generate-property-registry.py --check`; `code-verify.py --check`
  (`proto-field-renumbered`, `api-generated-edited`, `registry-parallel-field-map`);
  `registry-verify.py`; syntax-check on every generated TU and on `Frame.h` consumers (a full
  `grep -rn "\.led\b" core app/src` must be empty).
- **Deps:** T2, T4–T16
- [x] done

### Phase D — commands, QML, firmware

### T18 — Command manifests and bindings

- **Files:** `app/rcc/commands/projecteditor.json`, `app/rcc/commands/layouts/project-toolbar.json`,
  `app/rcc/commands/layouts/editor-menus.json`, `app/qml/Commands/ProjectEditorCommandBindings.qml`,
  `app/qml/Commands/ProjectEditorMenuBindings.qml`
- **Does:** Remove `editor.addLED`, `editor.menu.addDataset.led`, `editor.menu.dataset.led`;
  add `editor.addLEDPanel` and `editor.menu.addGroup.ledPanel` beside the Bar Panel entries,
  icon `editor/add-led` (exists). Bindings: `addGroup(..., SerialStudio.LEDPanel)`. **Invariant
  named:** new command = one manifest entry + one bindings entry; icons only through the
  registry, never a `qrc:` path (spec 0028/0063).
- **Verify:** `registry-verify.py`; `generate-command-strings.py --check`;
  `code-verify.py --check` on the QML.
- **Deps:** T17
- [x] done

### T19 — Project Editor views

- **Files:** `app/qml/ProjectEditor/Views/GroupView.qml`, `app/qml/ProjectEditor/Views/DatasetView.qml`,
  `app/qml/ProjectEditor/Views/MultiSelectionView.qml`,
  `app/qml/ProjectEditor/Views/FlowDiagram/DiagramIcons.qml`
- **Does:** Remove the LED add button and the two LED toggles; add the `ledpanel` icon row
  beside `barpanel`. Load the `qt-qml` skill first.
- **Verify:** `code-verify.py --check`; qmllint if available.
- **Deps:** T18
- [x] done

### T20 — Firmware libraries and declaration docs

- **Files:** `firmware/arduino/src/SerialStudio.h`, `firmware/c/serialstudio.h`,
  `firmware/tests/golden/` (one pair gains a `ledpanel` group), `firmware/README.md`,
  `doc/help/Device-Declarations.md`
- **Does:** Delete `led(bool)` and `ss_led`, keep the `ledHigh` calls; extend one golden pair
  with a `ledpanel` group and refresh with `check_golden.py`; the group widget list in both
  docs gains `ledpanel`, the channel LED call disappears. Python needs no edit (vocabulary
  filter). **Invariant named:** the parser drops unknown dataset keys, so a stray `led` from an
  old sketch is ignored, not refused (spec R8).
- **Verify:** `python3 firmware/tests/check_golden.py` for the Python half;
  `documentation-verify.py` on the two docs.
- **Deps:** T17
- [x] done

### Phase E — tests, fixtures, examples

### T21 — C++ unit tests, enum and applicability tier

- **Files:** `app/tests/tst_alarm_band_applicability.cpp`, `app/tests/tst_group_widget_requirement.cpp`,
  `app/tests/tst_enum_labels.cpp`, `app/tests/tst_serialstudio_enums.cpp`
- **Does:** Applicability rows: `ledpanel` member renders and alarms, hidden `ledpanel` member
  renders and alarms (Bar Panel parity), dataset moved out keeps bands inert; delete the `led`
  dataset rows. `ledpanel` requirement row; labels/slugs incl. `led-panel` alias; pin
  `GroupWidget::LEDPanel == 11`; drop `DatasetLED` option-text cases.
- **Verify:** code-verify; syntax-check; `ctest -R "tst_alarm_band_applicability|tst_group_widget_requirement|tst_enum_labels|tst_serialstudio_enums"` after the build.
- **Deps:** T17
- [x] done

### T22 — C++ unit tests, generation and tool tier

- **Files:** `app/tests/tst_frame_serialization.cpp`, `app/tests/tst_opcua_project_builder.cpp`,
  `app/tests/tst_modbus_generation.cpp`, `app/tests/tst_proto_importer.cpp`,
  `app/tests/tst_tool_schemas.cpp`, `app/tests/tst_tool_compact.cpp`
- **Does:** Expect a `ledpanel` group after the boolean-tag group instead of `led: true`;
  serialization round-trip has no `led` key; slug lists.
- **Verify:** code-verify; syntax-check; the matching `ctest -R` after the build.
- **Deps:** T17
- [x] done

### T23 — Legacy fixtures and the migration integration test

- **Files:** `tests/fixtures/projects/legacy/led-flag-all.ssproj` (pre-change CAN Bus),
  `tests/fixtures/projects/legacy/led-flag-mixed.ssproj` (pre-change Modbus PLC Simulator),
  `tests/fixtures/projects/legacy/led-flag-hidden.ssproj` (hand-made: one hidden flagged
  dataset, one workspace), `tests/integration/test_led_panel_migration.py`
- **Does:** Copy the two examples from `git show HEAD:` before T24 rewrites them. The test
  loads each through the API, asserts the widget multiset (one Data Grid plus one LED Panel per
  affected group), member titles and thresholds, LED Panel order, that the hidden dataset got
  no copy, then saves, reloads and asserts no change and the workspace intact (AC2). Read
  `tests/README.md` first; mark it `integration`.
- **Verify:** `pytest tests/integration/test_led_panel_migration.py` with the app up
  (maintainer, listed in TODO); `python -m pyflakes` or `pytest --collect-only` locally.
- **Deps:** T17
- [x] done

### T24 — Example projects migrated

- **Files:** the seven `.ssproj` under `examples/CAN Bus Example/`, `examples/EtherNet-IP Example/`,
  `examples/IEC 104 Example/`, `examples/Modbus PLC Simulator/`, `examples/OPC UA PLC Simulator/`,
  `examples/S7 PLC Example/`, `examples/Sparkplug Example/`; scratchpad script (not committed)
- **Does:** A scratchpad Python script applies the loader's rule (flagged-and-not-hidden
  positions, derived group right after the source, title "<Group> LEDs" in English, fresh
  `uniqueId`s from `nextUniqueId`, `nextUniqueId` advanced, `led` key dropped everywhere) and
  rewrites each file preserving key order and indentation. Rides in the single commit (the
  spec's Q4 own-commit answer was overridden on 2026-11-07); the `ssproj-app-saved-state` rule
  does not apply: this is the planned migration, not an app side effect.
- **Verify:** The script re-reads each output and reports the widget multiset against the
  input; `git diff --stat` shows only the seven files; after the build the maintainer opens
  each and sees no migration notification (R12).
- **Deps:** T23 (fixtures copied first)
- [x] done

### T25 — Other integration tests and the baseline note

- **Files:** `tests/integration/test_dual_serial.py`, `tests/integration/test_lua_script_engine.py`,
  `tests/integration/test_opcua_driver.py`, `tests/integration/test_property_registry.py`
- **Does:** Drop `led` literals and reads; in `test_property_registry.py` add a one-line
  docstring note that baselines were recaptured after spec 0114 (the recapture itself is a
  maintainer step: `SS_CAPTURE_BASELINES=1`, app up). Mirror fixtures regenerate with
  `tests/fixtures/mirror/generate_fixtures.py` (maintainer, app up); `kWireVersion` stays 1.
- **Verify:** `pytest --collect-only tests/integration`; the maintainer runs the three files.
- **Deps:** T24
- [x] done

### Phase F — docs and corpus

### T26 — Manual: the three pages that define the widget

- **Files:** `doc/help/Widget-Reference.md`, `doc/help/Project-Editor.md`, `doc/help/Toolbar-Reference.md`
- **Does:** LED Panel is a group widget with key `ledpanel`, "1+" datasets, bands or `ledHigh`;
  the "auto" row, the checkbox section and the LED Indicator button go; one migration note
  ("projects saved before 4.x gain a `<Group> LEDs` group next to each group that had LEDs");
  Hide on Dashboard parity with Bar Panel stated once. Load `ss-docs` first.
- **Verify:** `documentation-verify.py`, `term-verify.py`; grep for "Show in LED Panel" and
  "LED Indicator" in `doc/help` returns nothing but the migration note.
- **Deps:** T17
- [x] done

### T27 — Manual: remaining pages

- **Files:** `doc/help/API-Reference.md`, `doc/help/Canvas-Widget.md`, `doc/help/Aural-Alerts.md`,
  `doc/help/Getting-Started.md`, `doc/help/Pro-vs-Free.md`, `doc/help/FAQ.md`, `doc/help/Use-Cases.md`,
  `doc/help/Operation-Modes.md`, `doc/help/Drivers-IEC-104.md`, `doc/help/Drivers-S7.md`,
  `doc/help/Drivers-OPC-UA.md`, `doc/help/Drivers-EtherNet-IP.md`, `README.md`
- **Does:** Every mention of `led: true`, the checkbox or the option bit becomes the group
  widget; driver pages say boolean tags land in a generated LED Panel group; the Canvas
  `hasLed` getter is removed from the property table, `ledHigh` stays. Mechanical sweep, fit
  for a Sonnet executor with T26 as the donor.
- **Verify:** `documentation-verify.py`, `term-verify.py`; `grep -rn '"led"\|led: true' doc/help README.md` empty.
- **Deps:** T26
- [x] done

### T28 — Assistant corpus and architecture docs

- **Files:** `app/rcc/ai/skills/dashboard_layout.md`, `app/rcc/ai/skills/api_semantics.md`,
  `app/rcc/ai/docs/painter_js.md`, `doc/claude/architecture/dashboard.md`,
  `doc/claude/architecture/project.md`, `CLAUDE.md`
- **Does:** Corpus: `led` leaves the dataset widget table and the option table, `ledpanel`
  joins the group widget list with `barpanel` as the pattern. `dashboard.md` "Alarm Bands":
  the contract names "Bar Panel or LED Panel member", the phantom-panel sentence and the
  "`led: true` double-fire" note go. CLAUDE.md's Alarm Bands row: "(Bar, Gauge, Meter, or a Bar
  Panel / LED Panel member)". `search_index.json` regenerates in `sanitize-commit`.
- **Verify:** `registry-verify.py` (corpus field/enum lint); `claim-verify.py`;
  `documentation-verify.py`.
- **Deps:** T17
- [x] done

### T29 — TODO, spec close-out

- **Files:** `doc/claude/TODO.md`, `doc/claude/specs/0114-led-panel-group-widget/spec.md`
- **Does:** Add the maintainer steps: build; `--dump-api-schema` then `generate-sdk.py` (the
  hand-kept `led: 32` / `ledpanel: 11` maps in `SerialStudio.js` and `prelude.js` edited in the
  same pass); `SS_CAPTURE_BASELINES=1 pytest tests/integration/test_property_registry.py`;
  `tests/fixtures/mirror/generate_fixtures.py`; `pytest tests/integration/test_led_panel_migration.py`;
  the `ctest` list from T3/T21/T22; CAN Bus screenshots (AC4); `--benchmark-hotpath` (AC9);
  then the Arduino registry PR (spec 0111 T46) only after this lands. Tick the spec's ACs that
  are met without a build and set `status: done` once the maintainer confirms the rest.
- **Verify:** `ledger-verify.py --check`; read-back.
- **Deps:** T28
- [x] done

## Commit shape

One commit for everything (maintainer's call, 2026-11-07): tasks.md, Phases A to F, the
generated artifacts, the fixtures and the seven migrated examples, with a `Mechanical:` trailer
for the file count. One `sanitize-commit.py` run, one `commit-batch.py` plan with one entry.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC2, AC4, AC9 and
      the ctest ACs await the maintainer's build and are listed in TODO.md).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the loader, `WidgetResolution`, `WidgetMapBuilder` and the helper;
      findings addressed or noted.
- [x] Hotpath untouched; `--benchmark-hotpath` listed for the maintainer as a no-change check.
- [x] `pytest` targets identified for the maintainer (`test_led_panel_migration.py`,
      `test_property_registry.py` after recapture, the three edited integration files).
- [x] Per-file lint, syntax-check, censuses, registry, generator, layer, term and claim gates run clean (the per-commit sanitize step was retired on 2026-11-07; the search index was rebuilt).
- [x] Diff is *what was asked, and only that*: the one file outside plan.md's table is the
      `scripts/code-verify.py` whitelist line named in T4; everything lands in one commit.
- [ ] `spec.md` status set to `done`.
