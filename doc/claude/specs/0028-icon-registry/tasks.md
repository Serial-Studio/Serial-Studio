---
spec: 0028-icon-registry
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-21
---

# Tasks 0028 — Centralized icon & command registry

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units
> that are small, ordered, and *individually verifiable*. `/ss-implement` works this list
> top to bottom and keeps the status boxes current. Gate: do not start `/ss-implement`
> until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. QML literal sweeps (T8-T10) exceed 3 files by
  design: they repeat one mechanical pattern per surface and are reviewed as such.
- **Verify** is how *this* unit is confirmed — `code-verify.py --check` on touched files is
  implied everywhere and not repeated; listed verifies are the additional checks.
- Two **maintainer gates** (T1 sign-off, and the running-app ACs) are marked; nothing that
  depends on them starts early. The implementer never builds or runs the app.

## Tasks

### Phase A — icon tree + icon registry

### T1 — Icon audit manifest (MAINTAINER GATE)

- **Files:** `scripts/icon-migrate.py` (audit mode),
  `doc/claude/specs/0028-icon-registry/icon-map.csv`
- **Does:** Hash+name analysis of `app/rcc/icons/**` emitting the migration manifest: old
  path → `category/tier/name`, dup-group and same-glyph merge verdicts, viewBox→tier
  assignment, flagged between-tier cases (spec Q1-Q3; category table from `plan.md`).
- **Verify:** Audit re-run is byte-identical (deterministic); totals reconcile (448 files,
  64 dup groups, ~260 logical ids, `buttons/` absent); **maintainer signs off the CSV**.
- **Deps:** none
- [x] done — signed off 2026-07-21 (60 groups outside `buttons/`; 254 logical ids;
  precedence amendment re-confirmed)

### T2 — Apply tree migration with qrc compat aliases

- **Files:** `app/rcc/icons/**` (moves/deletes), `app/rcc/rcc.qrc`,
  `app/rcc/icons/system/16/missing.svg` (placeholder), `scripts/icon-migrate.py` (apply)
- **Does:** Executes the signed-off manifest: restructures to `<category>/<tier>/<name>
  .svg`, drops byte-identical dups, rewrites the qrc icons block, and emits `<file
  alias="icons/<old-path>">` entries so all 804 existing references keep resolving.
- **Verify:** `icon-migrate.py --apply` refuses on any manifest/tree mismatch; post-apply
  script check confirms every pre-migration qrc path still resolves (via alias) and no
  file outside `app/rcc/` changed.
- **Deps:** T1
- [x] done — 302 moved / 79 dropped / placeholder added; 381 aliases, 0 dangling; 6
  same-name-different-glyph cases kept as `-alt` names (recorded in `NAME_OVERRIDES`)

### T3 — registry-verify.py (icon half)

- **Files:** `scripts/registry-verify.py`
- **Does:** Lints the tree: layout conformance (tier in {16,24,32,48}), byte-dup sweep
  outside `buttons/` (AC2), qrc<->disk sync, alias inventory with per-alias source-ref
  counts (feeds T21).
- **Verify:** Clean run on the migrated tree; planted violations (dup file, stray tier,
  unregistered file) are each caught.
- **Deps:** T2
- [x] done — CLEAN; self-test caught planted dup/stray-dir; baseline 293 referenced
  aliases / 598 source refs; 4 snake_case names kebab-normalized during bring-up

### T4 — Misc::IconRegistry

- **Files:** `app/src/Misc/IconRegistry.h/.cpp`, `app/src/Misc/ModuleManager.cpp`,
  `app/CMakeLists.txt`
- **Does:** Catalog scan + `icon(category, name, px)` nearest-at-or-above resolution +
  placeholder + once-per-key warning; context property `Cpp_Misc_IconRegistry` (lazy
  first-touch; **not** in `instantiateCoreModules()`).
- **Verify:** Read-back against plan contract (R2 tier rule, R4 failure path);
  `qt-cpp-review` marked for the phase-A C++ diff.
- **Deps:** T2
- [x] done — `code-verify` clean; catalog ctor + nearest-at-or-above + warn-once
  placeholder; `Cpp_Misc_IconRegistry` registered lazily

### T5 — dashboardWidgetIcon shim

- **Files:** `app/src/SerialStudio.cpp`
- **Does:** Body of `dashboardWidgetIcon(w, large)` becomes enum→name switch + registry
  call (`"widgets"`, 32/16). Signature untouched.
- **Verify:** Grep: all 11 call sites unchanged; removed `dashboard-large|small` literals.
- **Deps:** T4
- [x] done — enum-to-name switch + registry call behind the unchanged signature;
  `code-verify` clean; all widget names confirmed present at 16+32 tiers

### T6 — Taskbar C++ icons + logical iconId role

- **Files:** `app/src/UI/Taskbar.h/.cpp`
- **Does:** Folder/workspace fallbacks (`Taskbar.cpp:1509,:1522-1527`) via registry; adds
  the `iconId` role per plan's model contract (user-picked icons keep the resolved-URL
  role). Read existing signal wiring in full first (house rule).
- **Verify:** Read-back: queued `buildTreeModel` conventions untouched; role additions
  covered in both `roleNames` and populate paths.
- **Deps:** T4
- [x] done — `IconIdRole`/"iconId" appended (role ints preserved); widget items carry
  `dashboardWidgetIconId()`; workspace/folder nodes carry iconId + registry-resolved
  16 px fallback (user inline icons untouched, iconId empty); `iconById()` helper added;
  `code-verify` clean, no signal wiring touched

### T7 — Remaining C++ icon consumers

- **Files:** `app/src/DataModel/Project/ProjectEditorTree.cpp`, `ProjectEditorForms.cpp`,
  `ProjectEditorSummaries.cpp`, `app/src/UI/Widgets/DataGrid.cpp`,
  `app/src/Platform/CSD.cpp`
- **Does:** Literal paths → registry calls; CSD adopts the merged `window` category
  (csd/miniwindow glyph unification).
- **Verify:** Grep: zero `:/icons/` literals remain in these files.
- **Deps:** T4
- [x] done — 48 swaps (44 icon(), 4 iconPath() in CSD) via Opus sweep; plus two
  follow-ups closed by hand: dead `treeview/output-widget` ref remapped to
  `editor/widget`, and the 103-line function advisory reshaped under the cap

### T8 — QML icon sweep: dashboard surfaces

- **Files:** `app/qml/MainWindow/Panes/Dashboard/WidgetDelegate.qml`, `Taskbar.qml`,
  `ExternalWidgetWindow.qml`, `app/qml/Widgets/MiniWindow.qml` (+ siblings found by grep)
- **Does:** Literal → `Cpp_Misc_IconRegistry.icon(...)` at each delegate's own px;
  taskbar/palette delegates adopt the `iconId` contract (R6/R15 size split).
- **Verify:** Grep: zero non-exempt `qrc:/icons/` literals in swept files; maintainer
  screenshot item queued (AC1).
- **Deps:** T4, T6
- [x] done — Opus sweep: 14 literal swaps + iconId adoption at 32/18/16 px
  (palette cells / search rows / taskbar popup); iconId propagated through
  PaletteModel flatten helpers; ExternalWidgetWindow needed no change. Also closed
  by hand beyond the plan's C++ list (R7 grep): ProjectEditorShared.h busTypeIcon
  table, ProjectEditorMqtt, FrameParserModel, ImporterCommon, ProjectModelWorkspaces
  defaults, AI/API doc examples — app/src now has zero old-form literals.

### T9 — QML icon sweep: project editor

- **Files:** `app/qml/ProjectEditor/Views/FlowDiagram.qml` (90 refs) + ProjectEditor QML
  found by grep
- **Does:** Mechanical map/literal swap to registry calls.
- **Verify:** Grep zero non-exempt literals in swept files.
- **Deps:** T4
- [x] done — 262 swaps across 25 files (Opus sweep) + 3 dynamic-base helpers in
  FlowDiagram hand-mapped (incl. the `-alt` rename trap); grep gate 0, lint clean

### T10 — QML icon sweep: remaining surfaces

- **Files:** panes/console/database/licensing/notifications/code-editor QML by grep
- **Does:** Same swap; toolbar/StartMenu literals are *left alone* (they die wholesale in
  phases C-E behind the aliases).
- **Verify:** Grep: non-exempt literals remain only in `Toolbar.qml`,
  `ProjectToolbar.qml`, `StartMenu.qml`, the three provider files, and exempt sets.
- **Deps:** T4
- [x] done — verified at HEAD: repo-wide grep for `qrc:/icons/`/`:/icons/` in
  `app/qml/**` and `app/src/**` finds zero non-exempt hits (only `icons/buttons/*`
  literals and two doc-string examples in `ContextBuilder.cpp`/`WorkspacesHandler.cpp`);
  now covered together with the later Toolbar/StartMenu/provider cutovers (T15/T16/T18/
  T19) which have since landed.

### Phase B — command registry core

### T11 — Command + layout manifests

- **Files:** `app/rcc/commands/{app,dashboard,projecteditor}.json`,
  `app/rcc/commands/layouts/{main-toolbar,project-toolbar,start-menu}.json`,
  `app/rcc/rcc.qrc`, `doc/claude/specs/0028-icon-registry/shortcut-checklist.md`
- **Does:** Lifts every command 1:1 from the three `*Actions.qml`, StartMenu, both
  toolbars, and the 31 Shortcut blocks into manifests (ids `scope.verb`, icons as new
  `category/name`, shortcuts recorded); layouts mirror today's exact structure. The
  shortcut checklist snapshots all 31 sequences for AC11 *before* anything is deleted.
- **Verify:** Manual cross-count: every provider entry / toolbar button / menu item /
  shortcut appears exactly once; icon refs all exist in `icon-map.csv` output.
- **Deps:** T2
- [x] done — 84 commands / 6 manifests; palette sets = historic 8/12/32 exactly; all
  icon + layout refs resolve; no per-window shortcut dupes; 20 of 31 shortcuts absorbed
  (Tab/Backtab/Ctrl+1-9 retained by design, see shortcut-checklist.md)

### T12 — UI::CommandRegistry

- **Files:** `app/src/UI/CommandRegistry.h/.cpp`, `app/src/Misc/ModuleManager.cpp`,
  `app/CMakeLists.txt`
- **Does:** Manifest load/validation (ThemeManager error pattern), GPL `pro` filtering,
  `commands(context)` / `layout(surface)` / shortcut display text, translate-at-query,
  `commandsChanged` on `Translator::languageChanged`; `Cpp_UI_CommandRegistry`.
- **Verify:** Read-back against plan contract; `qt-cpp-review` marked for phase-B C++.
- **Deps:** T11
- [x] done — loader + build-tier filters + ordered context queries + enriched layout
  trees (quiet by-design absence for tier-dropped ids) + StandardKey expansion +
  duplicate-shortcut warning + Translator retranslate hook; lint 0/0; manifests
  re-validated post-order fields (84 ids, sets 8/12/32, no dupes)

### T13 — Translation generator + lint completion

- **Files:** `scripts/generate-command-strings.py`, `app/src/UI/CommandStrings.cpp`
  (generated), `scripts/sanitize-commit.py`, `app/translations/translation_manager.py`
  (source list), `scripts/registry-verify.py` (manifest half)
- **Does:** Manifests → `QT_TRANSLATE_NOOP("Commands", ...)` dummy source, hooked into
  sanitize-commit; adds manifest lint (schema, unique ids, icon refs resolve, per-context
  duplicate shortcuts, commercial-symbol grep of non-commercial bindings).
- **Verify:** `--check` mode detects a planted manifest/generated drift; registry-verify
  clean; generated file present in the lupdate `@list`.
- **Deps:** T11, T12
- [x] done — verified at HEAD: `scripts/generate-command-strings.py` exists with a
  `--check` mode; `app/src/UI/CommandStrings.cpp` is the generated
  `QT_TRANSLATE_NOOP("Commands", ...)` stub (auto-picked up by `translation_manager.py`'s
  recursive `.cpp` walk, no separate registration needed); `sanitize-commit.py:261` runs
  the regen step; `registry-verify.py` has the manifest-half lint (`check_manifests`,
  `check_layouts` duplicate-shortcut check, `check_binding_guards` commercial-symbol
  scan) and a live run is CLEAN.

### T14 — CommandModel + per-context bindings

- **Files:** `app/qml/Commands/CommandModel.qml`, `AppCommandBindings.qml`,
  `DashboardCommandBindings.qml`, `ProjectEditorCommandBindings.qml`,
  `CommercialCommandBindings.qml`, `app/CMakeLists.txt`
- **Does:** The join layer (registry metadata + `{run, enabled, checked, visible}`
  entries); run bodies moved verbatim from the providers/StartMenu/toolbars; commercial
  entries isolated behind the `Cpp_CommercialBuild` Loader (R14).
- **Verify:** registry-verify commercial-symbol grep clean; every manifest id in each
  context has a bindings entry or a recorded reason (R11 read-back).
- **Deps:** T12, T13
- [x] done - CommandModel.qml (mine) + three bindings files (Sonnet swarm, 31/20/37
  entries, completeness OK); guard scan + lint clean

### T15 — Palette + search cutover; delete providers

- **Files:** `app/qml/MainWindow/Panes/Dashboard/PaletteModel.qml`, `Taskbar.qml`
  (search popup), `StartMenu.qml` (`searchableItems()` only — it delegates to
  `ToolActions` today), `app/qml/MainWindow/MainWindow.qml`, `app/qml/ProjectEditor/
  ProjectEditor.qml`, `app/qml/Widgets/CommandPalette.qml` (shortcut label); DELETE
  `ToolActions.qml`, `MainWindowActions.qml`, `ProjectEditorActions.qml`;
  `app/CMakeLists.txt`
- **Does:** Palette/taskbar-search/start-search consume `CommandModel`; palette rows show
  shortcut text; providers deleted (R9); 0027 contract preserved (palette stays
  model-driven, no dashboard symbols).
- **Verify:** Grep: zero references to the three deleted providers (AC7 static half);
  `CommandPalette.qml` still free of `Cpp_UI_Dashboard`/taskbar symbols.
- **Deps:** T14
- [x] done - palette/search/host wiring swapped in all three windows (Sonnet), palette
  rows show shortcuts, providers deleted after grep gate (verify pass)

### Phase C-E — surface cutovers

### T16 — Start menu from layout manifest

- **Files:** `app/qml/MainWindow/Panes/Dashboard/StartMenu.qml`
- **Does:** Static buttons + Tools submenu render from `layout("start-menu")`; dynamic
  submenus (Workspaces, Actions, Export) stay QML behind named slots; the string-id
  if-chain dispatch is deleted in favor of `entry.run()`.
- **Verify:** Read-back: every pre-cutover menu entry present in manifest or slot; AC10
  screenshot item queued.
- **Deps:** T14, T15
- [x] done - StartMenu renders static entries + Export/Tools from layout manifest
  (Sonnet); dynamic submenus + Settings alias preserved via stable ids; dispatch chains
  deleted; last 7 old-path literals swapped in verify pass

### T17 — CommandToolbar renderer

- **Files:** `app/qml/Widgets/CommandToolbar.qml`, `app/CMakeLists.txt`
- **Does:** Repeater-driven `RibbonSection`/`ToolbarButton` renderer over a layout tree
  (sections, commands with presentation hints, separators, grid groups, slots);
  checked → bold font generalization.
- **Verify:** Read-back against the layout schema in `plan.md`; no surface-specific
  symbols inside the renderer.
- **Deps:** T14
- [x] done — verified at HEAD: `app/qml/Widgets/CommandToolbar.qml` exists, is a
  `Repeater`-over-`RibbonSection`/`ToolbarButton` renderer driven purely by
  `Cpp_UI_CommandRegistry.layout(surface)` + `model.binding(id)` (no surface-specific
  symbols), and is consumed by `Toolbar.qml`, `ProjectToolbar.qml`, and
  `DatabaseExplorer.qml`; registered in `app/CMakeLists.txt`.

### T18 — Main toolbar cutover

- **Files:** `app/qml/MainWindow/Panes/Toolbar.qml`
- **Does:** 636-line hand-built toolbar → thin `CommandToolbar` host on
  `layout("main-toolbar")`; per-button Loader+Component scaffolding retired (commercial
  gating now via bindings, R14).
- **Verify:** Cross-count 24 buttons / 4 sections vs manifest; AC9 screenshot + reorder
  demo queued.
- **Deps:** T17
- [x] done - Toolbar.qml 636->287, CommandToolbar host + pinned buttons rewired
  through registry (Sonnet); zero icon literals

### T19 — Project editor toolbar cutover

- **Files:** `app/qml/ProjectEditor/Sections/ProjectToolbar.qml`
- **Does:** Same treatment via the shared renderer on `layout("project-toolbar")`
  (34 buttons / 8 sections).
- **Verify:** Cross-count vs manifest; AC9 item queued.
- **Deps:** T17
- [x] done - ProjectToolbar.qml 580->162 via shared renderer (Sonnet)

### Phase F — shortcuts, gates, docs

### T20 — Shortcut migration

- **Files:** `app/qml/MainWindow/MainWindow.qml`, `app/qml/ProjectEditor/
  ProjectEditor.qml` (+ a small shared instantiator component if extracted)
- **Does:** Per-window `Instantiator` of `Shortcut` items over context commands with
  shortcuts; deletes the 31 hand blocks; context-sensitive sequences (Ctrl+K) live in a
  single command's `run()` branch; any non-command chord found is kept as a hand block
  and listed in `shortcut-checklist.md`.
- **Verify:** Checklist doc updated to map all 31 old sequences → command id or retained
  block; registry-verify duplicate-sequence lint clean; AC11 runtime checklist queued.
- **Deps:** T15, T16, T18, T19
- [x] done - both windows use registry Instantiators (20 sequences); Tab/Backtab/
  Ctrl+1-9 retained as planned; counts verified 12/1

### T20b — Display-size re-tier swarm (maintainer-directed 2026-07-21)

- **Files:** `app/rcc/icons/**` (tier moves), `app/rcc/rcc.qrc`, `icon-map.csv`, migrated
  call sites' px arguments; driven by a small retier script + Opus agent swarm
- **Does:** viewBox is export scale, not detail: some large-viewBox files are 16 px-detail
  art. For every logical icon, agents read the PRE-migration usage from git HEAD (the old
  QML's `icon.width`/`iconSize`/`sourceSize` near each old literal) to recover the true
  display size; artwork moves to that tier (min observed size when used at several), and
  migrated call sites' px arguments become the actual display size instead of the CSV
  tier. Aliases + CSV updated by script; `registry-verify.py` gates the result.
- **Verify:** `registry-verify.py` CLEAN (no dangling aliases, no dups); spot-check table
  of moved tiers for maintainer; AC6 visual pass covers the result.
- **Deps:** T15, T16, T18, T19 (runs after all surfaces + command registry, before T21)
- [x] done - scripts/icon-retier.py (Sonnet, deterministic): 46 downward re-tiers
  from git-HEAD render-size evidence, 93 call-site px corrections, upward moves refused
  (window-noise), 114 no-evidence left in place; all verifiers clean

### T21 — Alias drop + grep gates

- **Files:** `app/rcc/rcc.qrc` (+ generated legacy-path map, see below)
- **Does:** Removes the compat alias block once nothing references old paths.
  **Amended 2026-07-21:** old icon paths live in USER DATA — importer-generated user
  workspaces persist `qrc:/icons/panes/overview.svg` into project JSON, and the
  workspaces API accepted arbitrary qrc paths. Before dropping aliases, generate a
  legacy-path map from `icon-map.csv` (old URL -> new URL) consulted where `ws.icon`
  is resolved (`IconEngine::resolveActionIconSource` or the Taskbar node builders), so
  saved projects keep their icons with zero aliases in the qrc.
- **Verify:** `registry-verify.py` alias-ref count = 0 beforehand (blocks otherwise);
  AC2 dup sweep = 0; AC3 grep — non-exempt `qrc:/icons/`|`:/icons/` literals = 0 outside
  `buttons/` + user action library + registry internals.
- **Deps:** T8, T9, T10, T16, T18, T19
- [x] done - 381-entry legacy map generated from icon-map.csv
  (Misc::legacyIconPath, consulted in resolveActionIconSource) so saved projects keep
  icons; alias block dropped; alias-ref count 0; AC3 sweep 0 old-form refs tree-wide

### T22 — Docs, sanitize, AC sweep

- **Files:** `CLAUDE.md` (registry pointer + scripts table rows),
  `doc/claude/architecture/` (relevant subsystem note), this spec's `spec.md` checkboxes
- **Does:** Records the architectural change (two registries, manifest locations,
  bindings pattern, generated-strings step); runs `sanitize-commit.py`; walks every AC —
  static ones checked off, runtime ones handed to the maintainer as a single checklist.
- **Verify:** `documentation-verify.py` untouched targets stay clean; counterfactual
  check named in chat (which rule the diff most risks + evidence it doesn't).
- **Deps:** T20, T21
- [x] done - CLAUDE.md registry block + scripts rows; sanitize-commit run;
  static ACs ticked, runtime ACs handed to maintainer (see final session summary)

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (runtime ACs
      confirmed by the maintainer: AC1, AC4-AC6, AC8-AC13).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new
      errors). — re-run repo-wide at HEAD: `3503 files scanned, 0 errors, 0 advisory`.
- [x] `python scripts/registry-verify.py` clean (tree, qrc, manifests, shortcuts,
      commercial-symbol sweep). — re-run at HEAD: `registry-verify: CLEAN`.
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched — no `--benchmark-hotpath` run required (re-affirm at handoff).
- [x] Translation refresh flow run by the maintainer after `CommandStrings.cpp` lands.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — exempt sets untouched, no foreign files
      touched, `TaskbarSettings` untouched.
- [x] `spec.md` status set to `done`.
