---
spec: 0063-context-menu-registry
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-18
---

# Tasks 0063 — Registry-Driven Context Menus for the Project Editor

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

## Tasks

### T1 — Record the pre-change entry inventory

- **Files:** `doc/claude/specs/0063-context-menu-registry/inventory.md` (new)
- **Does:** Enumerate, from the current code, every entry of the tree's `sharedContextMenu` and of
  each of the eleven `FlowDiagram` menus (background, source, frameparser, group, dataset, output,
  output-panel, action, table, transform, controlscript) — title, icon id, gating condition, and
  the slot each one calls. This is the checklist AC9 is measured against, and it must exist
  *before* any menu is deleted.
- **Verify:** Every `MenuItem` in `ProjectStructure.qml` and `FlowDiagram.qml` appears exactly once
  in the inventory; counts match a `grep -c "MenuItem {"` per file.
- **Deps:** none
- [x] done

### T2 — Append the six new `ItemKind` values

- **Files:** `app/src/DataModel/ProjectEditor.h`
- **Does:** Append `KindProjectRoot`, `KindFrameParser`, `KindGroupsRoot`, `KindTablesRoot`,
  `KindSystemDatasets`, `KindWorkspacesRoot` to `ItemKind`. Append only — never renumber existing
  values, and keep `KindNone = 0` first so the delegate's "no menu" guard and
  `selectedTreeItems()`'s skip both keep working.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/ProjectEditor.h`; grep that
  no existing enumerator moved.
- **Deps:** T1
- [x] done

### T3 — Stamp the six unkinded tree nodes

- **Files:** `app/src/DataModel/Project/ProjectEditorTree.cpp`
- **Does:** Set `TreeItemKind` on the project `root`, `parserItem` (plus `TreeItemId` = its
  `sourceId`, so the frame-parser menu can scope to its source), `groupsRoot`, `tablesRoot`,
  `sysDsItem` and `wsRoot`. The blank spacer row keeps no kind so it stays inert (R2). Do not add
  a `buildTreeModel()` call anywhere in this file's item handlers — the rebuild-inside-a-change-
  handler trap (`common-mistakes.md`, ProjectModel section) still applies.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/Project/ProjectEditorTree.cpp`;
  read back that each of the six nodes sets exactly one kind and that the spacer sets none.
- **Deps:** T2
- [x] done

### T4 — Audit bulk operations against the new kinds

- **Files:** `app/src/DataModel/Project/ProjectModelCrud.cpp`,
  `app/src/DataModel/Project/ProjectModelFolders.cpp` (read; edit only if a guard is missing)
- **Does:** Read `deleteSelectedItems`, `duplicateSelectedItems`, `moveSelectedItemsToFolder` and
  `setItemsEnabled` and confirm an unrecognized kind is a no-op rather than a default-branch
  mutation — the six new kinds now reach `selectedTreeItems()`. Add an explicit skip only where one
  is missing. Record the finding in the task notes either way.
- **Verify:** `python scripts/code-verify.py --check` on any file touched; a written statement per
  slot of what an unknown kind does.
- **Finding:** no edit needed. `duplicateSelectedItems`, `deleteSelectedItems` and
  `moveSelectedItemsToFolder` dispatch through `switch (kind)` with `default: break`;
  `setItemsEnabled` matches only group / dataset / group-folder. An unrecognized kind is a strict
  no-op in all four. The tree additionally keeps section rows out of the counters that gate the
  bulk entries.
- **Deps:** T3
- [x] done

### T5 — Declare the `editor.menu.*` commands

- **Files:** `app/rcc/commands/projecteditor.json`
- **Does:** Add one command per menu entry the spec requires: creation (source, group templates,
  dataset types, output types, action, table, register, workspace, folders, sub-folders), item ops
  (rename, hide/show as a `toggle`, duplicate, delete, move up/down, move to folder), navigation
  (edit frame parser, edit transform, edit control loop, MQTT, help), the eight dataset
  visualization `toggle`s, and the project-wide entries (expand/collapse all, seed aliases, show all
  hidden groups, workspace cleanup/reset/clear). Each gets `contexts: []` so none reaches the
  palette, `category: "project"`, an icon id that resolves, and **no `pro` flag** (Pro entries are
  visible-but-disabled via their binding — a `pro` flag would delete them from GPL builds).
- **Verify:** `python scripts/registry-verify.py` (icon refs resolve, ids unique, kinds/categories
  known); `python scripts/generate-command-strings.py --check`.
- **Deps:** T1
- [x] done

### T6 — Menu-manifest loader in `CommandRegistry`

- **Files:** `app/src/UI/CommandRegistry.h`, `app/src/UI/CommandRegistry.cpp`
- **Does:** Add `loadMenus(const QString& path)`: parse `menus[]`, resolve `include` nodes against
  sibling menus with a depth cap and cycle guard, run each menu through the existing
  `filterLayoutNodes`, and insert it into `m_layouts` under `editor-menu/<name>`. Warn-and-skip on
  malformed input (never fatal), matching the file's existing policy. `layout()`, `commandNode()`
  and `containerNode()` stay untouched, so node enrichment and translation are inherited. Add the
  one `loadMenus` line to the ctor.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/CommandRegistry.h
  app/src/UI/CommandRegistry.cpp`; read back that the ctor's existing `loadManifest`/`loadLayout`
  order is unchanged and that no query method's signature moved.
- **Deps:** T2
- [x] done

### T7 — Fragment menus + manifest registration

- **Files:** `app/rcc/commands/layouts/editor-menus.json` (new), `app/rcc/rcc.qrc`
- **Does:** Create the manifest with the three reusable fragment menus — `add-group` (eleven
  templates), `add-dataset` (eight types), `add-output` (six controls) — and register the file in
  `rcc.qrc`. These are the single authored lists that every other menu pulls in by `include`, which
  is what makes AC5 a one-entry edit.
- **Verify:** `python scripts/registry-verify.py`; a running app is not needed — the check confirms
  every referenced command id exists.
- **Deps:** T5, T6
- [x] done

### T8 — The nineteen menu surfaces

- **Files:** `app/rcc/commands/layouts/editor-menus.json`
- **Does:** Author the per-kind menus: project root, source, frame parser, groups root, group
  folder, group, output-panel group, dataset, output widget, action, tables root, dataset values,
  table folder, shared table, workspaces root, workspace folder, workspace, control loop, MQTT
  publisher. Each pulls fragments via `include`, places `separator` nodes between concern groups,
  and carries a `dynamic` node with `role: "move-to-folder"` where the folder cascade belongs. The
  project-root menu carries creation plus project-wide entries only — no file commands (R3).
- **Verify:** `python scripts/registry-verify.py`; cross-check each menu against the spec's R1 kind
  list and against `inventory.md` so no pre-existing entry is dropped.
- **Deps:** T7
- [x] done

### T9 — Binding title override in `CommandModel`

- **Files:** `app/qml/Commands/CommandModel.qml`
- **Does:** Let `join()` prefer a binding-supplied `title` over the manifest title, so count-bearing
  bulk labels ("Delete Selected (3)") work. Counts use `.arg()` — never `%n` combined with `.arg()`
  (`common-mistakes.md`, Qt & QML UI).
- **Verify:** `python scripts/code-verify.py --check app/qml/Commands/CommandModel.qml`; confirm the
  existing toggle title/icon swap still takes precedence in the same expression order.
- **Deps:** T1
- [x] done

### T10 — `Widgets/CommandMenu.qml` renderer

- **Files:** `app/qml/Widgets/CommandMenu.qml` (new)
- **Does:** Generic layout-driven menu: `popup(surface)` pulls
  `Cpp_UI_CommandRegistry.layout(surface)`, destroys any previously built items, and materializes
  `MenuItem` / child `Menu` / `MenuSeparator` nodes, resolving behavior through the supplied
  `CommandModel`. Entries whose binding is missing or `visible === false` are not created;
  submenus with no visible child are not created; leading, trailing and consecutive separators are
  suppressed *after* visibility filtering. `dynamic` nodes call the host handler registered for
  their `role`. Items are destroyed on close so no node survives a tree rebuild.
- **Verify:** `python scripts/code-verify.py --check app/qml/Widgets/CommandMenu.qml`; structural
  read-back against `CommandToolbar.qml`'s node-walking shape.
- **Deps:** T6, T9
- [x] done

### T11 — Menu bindings: target state and creation entries

- **Files:** `app/qml/Commands/ProjectEditorMenuBindings.qml` (new)
- **Does:** Declare the target properties (kind, id, parentId, path, sourceId, groupId, folderId,
  widget type, selfEnabled) plus the three selection counters moved off `ProjectStructure.qml`, a
  `setTarget()` and a `clearTarget()`, and the `cmd…` entries for every creation command, each
  selecting its target entity first where the mutator works off "current" state. Pro entries gate
  with `enabled: Cpp_CommercialBuild`. This file must reference no `Cpp_Licensing_` /
  `Cpp_Sessions_` / `Cpp_MQTT_` symbol — the commercial-guard scan covers `Commands/*.qml`.
- **Verify:** `python scripts/code-verify.py --check app/qml/Commands/ProjectEditorMenuBindings.qml`;
  `python scripts/registry-verify.py` (guard scan).
- **Deps:** T5, T9
- [x] done

### T12 — Menu bindings: item operations, toggles and host hooks

- **Files:** `app/qml/Commands/ProjectEditorMenuBindings.qml`
- **Does:** Add the `cmd…` entries for rename, hide/show, duplicate, delete, move up/down, move to
  folder, the navigation entries, the project-wide entries, and the eight dataset visualization
  toggles (`checked` from the dataset's option bits, `run()` through `changeDatasetOption`). Bulk
  entries supply their count-bearing `title` and hide when the selection count is 1 or 0 as the
  spec's R9 requires. Add the host-hook signals for the few dialogs that live inside a view
  (painter code today; the deferred set navigates instead).
- **Verify:** `python scripts/code-verify.py --check app/qml/Commands/ProjectEditorMenuBindings.qml`;
  every id in `editor-menus.json` appears in the bindings `map`.
- **Deps:** T11, T8
- [x] done

### T13 — Registry verification for menus

- **Files:** `scripts/registry-verify.py`
- **Does:** Add `editor-menus.json` to the layout checks; validate that every menu name is a known
  surface key, that every `include` target resolves, and that every command id a menu references is
  bound in `ProjectEditorMenuBindings.qml`'s `map` (an unbound id is silently dropped at runtime, so
  this check is what turns a manifest typo into a build-time error).
- **Verify:** `python scripts/registry-verify.py` passes on the current tree; deliberately break one
  id locally and confirm the check fails, then restore.
- **Deps:** T12
- [x] done

### T14 — Tree: swap `sharedContextMenu` for the registry menu

- **Files:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- **Does:** Instantiate the bindings + `CommandModel` + `CommandMenu` trio; map row kind to surface
  key; on right-click, fill the target from the row's model roles and popup the surface. Delete the
  old `sharedContextMenu` and its hand-maintained separator-visibility expressions. Keep R8's
  selection behavior exactly as it is: right-click outside the selection collapses to that row,
  right-click inside preserves it. Clear the target on close and on `treeModelChanged` so no id
  survives a rebuild.
- **Verify:** `python scripts/code-verify.py --check app/qml/ProjectEditor/Sections/ProjectStructure.qml`;
  every entry from `inventory.md`'s tree section is reachable again.
- **Deps:** T10, T12
- [x] done

### T15 — Tree: folder cascade as a dynamic role, and background right-click

- **Files:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- **Does:** Re-expose the existing `rebuildMoveMenu` / `populateMoveMenu` cascade as the
  `move-to-folder` dynamic-role handler the renderer calls, and add a right-click handler for the
  empty area below the last row that opens the project-root menu (R2), leaving the blank spacer row
  inert.
- **Verify:** `python scripts/code-verify.py --check app/qml/ProjectEditor/Sections/ProjectStructure.qml`;
  read-back that the cascade's exclude-self and section-filtering rules are unchanged.
- **Deps:** T14
- [x] done

### T16 — Diagram: swap the eleven menus for the registry menu

- **Files:** `app/qml/ProjectEditor/Views/FlowDiagram.qml`
- **Does:** Delete the eleven inline `Menu` blocks and the shared `Action` block; keep
  `menuController`'s node-key/pin state, have `openForNode` / `openForBackground` fill the bindings
  target and popup the matching surface, and keep the `setSuppressViewChange` wrapper so a
  right-click add does not yank the view. Wire the painter-code host hook to the existing dialog.
- **Verify:** `python scripts/code-verify.py --check app/qml/ProjectEditor/Views/FlowDiagram.qml`;
  every entry from `inventory.md`'s diagram sections is reachable again.
- **Deps:** T14
- [x] done

### T17 — `GroupTemplateMenu` over the registry

- **Files:** `app/qml/ProjectEditor/Views/GroupTemplateMenu.qml`
- **Does:** Reimplement the body as a `CommandMenu` over the `add-group` fragment surface while
  keeping the `parentFolderId` property and popup API, so `GroupsView` and `GroupFolderView` are not
  touched (spec non-goal) and the third copy of the template list disappears.
- **Verify:** `python scripts/code-verify.py --check app/qml/ProjectEditor/Views/GroupTemplateMenu.qml`;
  grep that `GroupsView.qml` and `GroupFolderView.qml` are unchanged.
- **Deps:** T16
- [x] done

### T17b — Register the new QML files with the build

- **Files:** `app/CMakeLists.txt`
- **Does:** Add `qml/Commands/ProjectEditorMenuBindings.qml` and `qml/Widgets/CommandMenu.qml` to
  `QML_SOURCES`, in alphabetical position. The list is explicit, not globbed, so an unregistered
  QML file resolves to nothing at runtime and the Project Editor window fails to load.
- **Verify:** every `qml/**/*.qml` on disk appears in `QML_SOURCES` and vice versa.
- **Deps:** T17
- [x] done

### T18 — Document the menu surface

- **Files:** `doc/claude/architecture/commands-icons.md`, `CLAUDE.md`
- **Does:** Add a "Context menus" subsection to the command doc — the menu manifest, the
  `editor-menu/<kind>` surface convention, `include` fragments, `dynamic` roles, the target-on-
  bindings pattern, and the recipe for adding one entry. Extend the CLAUDE.md commands-icons
  subsystem row so a future session knows menus are registry-driven too.
- **Verify:** `python scripts/code-verify.py --check` on the Markdown; the recipe matches what T7/T8
  actually shipped.
- **Deps:** T17
- [x] done

### T19 — Maintainer verification pass

- **Files:** none (observation), `doc/claude/specs/0063-context-menu-registry/spec.md` (check boxes)
- **Does:** Walk AC1–AC11 with the app running against a project that exercises every kind, on both
  a commercial and a GPL build, and check the boxes in `spec.md`. AC3/AC4 compare the saved project
  JSON after menu-driven creation against the same object created from the diagram or a view
  toolbar, cross-checked live through `tests/utils/api_client.py` on `localhost:7777`.
- **Verify:** All eleven acceptance criteria checked, with AC9 walked against `inventory.md`.
- **Deps:** T18

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `python scripts/registry-verify.py` and `python scripts/generate-command-strings.py --check`
      pass, including the new menu checks.
- [x] `qt-cpp-review` run on the C++ diff (`ProjectEditor.h`, `ProjectEditorTree.cpp`,
      `CommandRegistry.h/.cpp`); findings addressed or noted.
- [x] Hotpath untouched — no `--benchmark-hotpath` run required for this change (plan's hotpath
      section states none; confirm the diff contains no frame-path file).
- [x] No `pytest` target applies; maintainer verification is the AC list in `spec.md`.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — the plan's file table is the lane; the view
      toolbars, dialogs, and project format stay untouched.
- [ ] `spec.md` status set to `done` (after T19).

