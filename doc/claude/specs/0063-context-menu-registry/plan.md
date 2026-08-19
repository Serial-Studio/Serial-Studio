---
spec: 0063-context-menu-registry
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-18
---

# Plan 0063 — Registry-Driven Context Menus for the Project Editor

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Context menus become the menu twin of the existing ribbon: one new **menu manifest**
(`app/rcc/commands/layouts/editor-menus.json`) declares a named menu per tree/diagram node kind,
built from `command` / `submenu` / `separator` / `include` / `dynamic` nodes; `UI::CommandRegistry`
loads it into the same `m_layouts` store under `editor-menu/<kind>` surface keys, reusing the
existing build-tier filter and node enrichment; a new generic `Widgets/CommandMenu.qml` walks that
tree and materializes a `QtQuick.Controls.Menu` on open; behavior comes from a new
`Commands/ProjectEditorMenuBindings.qml`, which is simultaneously the **target holder** — the
right-clicked node's kind and ids are properties on the bindings instance, so every `enabled` /
`visible` / `checked` / title expression stays an ordinary declarative QML binding, exactly as
`FlowDiagram`'s `menuController` works today. The tree and the flow diagram each own one
bindings + `CommandModel` + `CommandMenu` trio; they share the manifest and the command catalog,
which is what makes the two surfaces incapable of drifting (R6). Chosen over a kind-tagged flat
catalog (cannot express per-kind order/separators) and over shared QML fragments (still a QML edit
per entry, so R5 fails).

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/ProjectEditor.h` | Append six `ItemKind` values: `KindProjectRoot`, `KindFrameParser`, `KindGroupsRoot`, `KindTablesRoot`, `KindSystemDatasets`, `KindWorkspacesRoot`. Appended, never renumbered (the role is read by QML by name). |
| `app/src/DataModel/Project/ProjectEditorTree.cpp` | Stamp `TreeItemKind` (+ `TreeItemId` where meaningful) on the six currently-unkinded nodes: project `root`, `parserItem` (id = its `sourceId`), `groupsRoot`, `tablesRoot`, `sysDsItem`, `wsRoot`. Spacer row stays unkinded. |
| `app/src/UI/CommandRegistry.h` / `.cpp` | New `loadMenus(path)`: parses a multi-menu manifest, resolves `include` nodes against sibling menus (cycle-guarded), runs the existing `filterLayoutNodes`, and inserts each menu into `m_layouts` as `editor-menu/<name>`. `layout()` / `commandNode()` / `containerNode()` unchanged. One `loadMenus` line in the ctor. |
| `app/rcc/commands/projecteditor.json` | New `editor.menu.*` command declarations (title, tooltip, icon, `kind`, `category: "project"`, `contexts: []`). No `pro` flag — see Tradeoffs. |
| `app/rcc/commands/layouts/editor-menus.json` | **New.** The nineteen menus plus the three reusable `add-group` / `add-dataset` / `add-output` fragment menus. |
| `app/rcc/rcc.qrc` | One `<file>` entry for the new layout manifest. |
| `app/qml/Widgets/CommandMenu.qml` | **New.** Generic layout-driven menu renderer: builds items on `popup()`, resolves behavior through a `CommandModel`, honors toggles, skips unbound/invisible entries, delegates `dynamic` roles to host-supplied handlers. |
| `app/qml/Commands/ProjectEditorMenuBindings.qml` | **New.** Target properties (kind, ids, path, source, folder, widget type, self-enabled, selection counts) + one `cmd…` entry per `editor.menu.*` id + `map`. Host hooks (`signal painterCodeRequested(int groupId)` and friends) for the few dialogs that live in a view. |
| `app/qml/Commands/CommandModel.qml` | `join()` honors an optional binding-supplied `title` override, so bulk entries can read "Delete Selected (3)". Everything else unchanged. |
| `app/qml/ProjectEditor/Sections/ProjectStructure.qml` | Replace `sharedContextMenu` (and its `visible:`-toggle blocks) with the bindings + model + `CommandMenu` trio; delegate right-click fills the target and calls `popup(surfaceFor(kind))`; add a background right-click handler below the rows; keep `rebuildMoveMenu`'s folder-tree logic, now exposed as the `move-to-folder` dynamic role handler. |
| `app/qml/ProjectEditor/Views/FlowDiagram.qml` | Delete the eleven inline `Menu` blocks and the ~40 shared `Action` objects; `menuController` keeps node-key/pin state and forwards the node into the bindings target; `openForNode`/`openForBackground` call `CommandMenu.popup(surface)`. |
| `app/qml/ProjectEditor/Views/GroupTemplateMenu.qml` | Body reimplemented as a `CommandMenu` over the `add-group` fragment surface, keeping its `parentFolderId` API so `GroupsView` / `GroupFolderView` toolbars are untouched (spec non-goal). |
| `app/CMakeLists.txt` | Register the two new QML files in `QML_SOURCES` (amended during implementation: the list is explicit, not globbed, so an unregistered .qml fails to resolve at runtime and takes the whole editor window down). |
| `scripts/registry-verify.py` | Add the menu manifest to the layout checks; new checks: every menu name maps to a known surface key, every `include` target exists, every command id referenced by a menu is present in `ProjectEditorMenuBindings.qml`'s `map`. |

`scripts/generate-command-strings.py` **does** need one edit (amended during implementation): it
globs `commands/*.json` and `commands/layouts/*.json`, but walks only the `sections`/`items`/
`pinnedEnd` keys — the menu manifest nests its nodes under `menus`, so submenu titles ("Add Group",
"Visualizations", "Move to Folder") would ship untranslated. The collector gains a `menus` walk.

## Architecture & data flow

**Load (startup, GUI thread).** `UI::CommandRegistry`'s ctor gains
`loadMenus(":/commands/layouts/editor-menus.json")`. Parsing is: read `menus[]`, resolve every
`include` node by splicing the referenced menu's items (depth-limited, cycle-guarded, warning +
skip on a bad reference, matching the file's existing "warn and skip, never fatal" policy), then
run each menu through `filterLayoutNodes` so `pro`/`gplOnly` nodes drop by build tier, and store
under `editor-menu/<name>`. Query stays `layout(surface)`, so `layoutNodes` / `commandNode`
enrichment (translated titles, node-level overrides, icon ids, `role`) is inherited untouched.

**Open (right-click).** Tree delegate → `treeView.ctx*` assignment becomes a single
`menuBindings.setTarget({kind, id, parentId, path, sourceId, groupId, widget, selfEnabled})`,
followed by `contextMenu.popup(surfaceForKind(kind))`. `CommandMenu` pulls
`Cpp_UI_CommandRegistry.layout(surface)`, clears its previously built items, and materializes the
tree: `command` nodes become `MenuItem`s bound to `model.binding(id)`; `submenu` nodes become child
`Menu`s created via `Component.createObject` + `addMenu()` (the mechanism `populateMoveMenu`
already uses); `separator` nodes become `MenuSeparator`; `dynamic` nodes call the handler the host
registered for that `role`. Built items are destroyed on close, so no stale node survives a tree
rebuild.

**Run.** `MenuItem.onTriggered` calls `behavior.run()`. Bindings read their own target properties
and call the existing `Cpp_JSON_ProjectModel` / `Cpp_JSON_ProjectEditor` slots — selecting the
target entity first where a mutator works off "current" state (`selectGroup` before `addDataset`),
which is the `selectTargetGroup()` pattern `FlowDiagram` already ships. The diagram keeps its
`setSuppressViewChange` wrapper so a right-click add does not yank the view; the tree does not use
it, because there selection *is* navigation and the pending-selection reveal
(`consumePendingSelection`) should land on the new object (R4).

**Selection semantics.** R8 is today's behavior and stays in the delegate: a right-click on an
unselected row collapses the selection to it, a right-click inside the selection preserves it. The
existing `deletableSelectionCount` / `selectableSelectionCount` / `enableableSelectionCount`
counters move onto the bindings object, where they drive both `visible` (creation entries hidden
when the count is > 1) and the count-bearing titles.

**Toggles.** Hide/Show and the eight dataset visualization entries (R14) are declared
`kind: "toggle"` with `titleChecked` / `iconChecked`; the binding's `checked` reads the target
(`selfEnabled`, or the dataset's option bit), and `run()` calls `setItemsEnabled` /
`changeDatasetOption`. `CommandModel.join` already swaps title and icon on `checked`.

## Hotpath & threading impact

- **Touches the hotpath?** No. This is Project Editor UI only: manifests loaded once at startup,
  menus built on user right-click. Nothing in `FrameReader` / `CircularBuffer` / `FrameBuilder` /
  `Dashboard` draw / the span fast lane is read or written, and no per-frame work is added.
- **New cross-thread signal/slot?** No. Everything runs on the GUI thread: registry load in the
  singleton ctor, menu construction and command execution in QML.
- **New input to a cached hotpath flag?** No. The commands invoke existing project mutators, whose
  existing signals already feed whatever caches they feed today; no new flag and no new producer.
- **Timestamp ownership** — unaffected; no frame or sample path is touched.

## Data model & persistence

No project-file change, no `Frame.h` `Keys::` addition, no schema or writer-version bump, no
Sessions DB change. The new `ItemKind` values live only in the editor's in-memory tree model role
(`TreeItemKind`), which is never serialized — they are appended rather than inserted purely to keep
diffs and any in-flight QML comparisons honest. Tree expansion state persists by display path and
is unaffected. The menu manifest is a build resource, not user state.

## API / SDK surface

None. No new API handler, no `EnumLabels` slug, no generated-SDK change, no `apiCall` reach. Every
operation a menu triggers is already reachable from the API through its existing project command.

## QML / UI

`Widgets/CommandMenu.qml` is the one genuinely new component. Shape mirrors `CommandToolbar.qml`:

```
CommandMenu {
  model: aCommandModel          // CommandModel over the menu bindings
  dynamicHandlers: ({ "move-to-folder": fillMoveToFolder })
  function popup(surface)       // pulls layout(surface), builds, opens
}
```

Rules it must hold: items are created on open and destroyed on close (never cached across a tree
rebuild); an entry whose binding is missing or `visible === false` is not created at all (R9,
"absent rather than disabled"); a submenu whose children all resolve invisible is not created
either, so no empty cascades appear; consecutive separators and leading/trailing separators are
suppressed after visibility filtering, which removes the hand-maintained separator-visibility
expressions that `ProjectStructure.qml` carries today; icons resolve only through
`Cpp_Misc_IconRegistry.iconById(ref, 16)` at menu size.

The three fragment menus (`add-group`, `add-dataset`, `add-output`) are authored once and pulled in
by `include`, so a new widget template is one manifest entry visible on every surface that includes
the fragment — the tree, the diagram, and `GroupTemplateMenu`'s toolbar popup (AC5).

Dialogs owned by a view (painter code in `FlowDiagram`, and the deferred alarm-bands / frequency-
markers / icon-picker / add-widget set) are reached through host-hook signals on the bindings
object rather than by the bindings reaching into a view; where the spec defers them, the entry
navigates via `Cpp_JSON_ProjectEditor.select*` instead.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Menu content source | (a) one multi-menu manifest, (b) one layout file per menu (19 files + 19 `loadLayout` lines + 19 qrc entries), (c) kind-tagged flat catalog | **(a)** — one authored file to review and edit, ~20 lines of loader C++, and the surface-key convention keeps the existing `layout()` query untouched. |
| Fragment reuse | (a) `include` node resolved at load, (b) repeat item lists per menu | **(a)** — the whole point is that a new dataset type is declared once (AC5); (b) reintroduces the duplication this spec exists to kill, just inside one file. |
| Target delivery | (a) target properties on the bindings object, (b) `run(target)` argument, (c) shared ambient target singleton | **(a)** — keeps `enabled`/`visible`/`checked`/title as declarative bindings (an argument only reaches `run`), needs no `CommandModel` change, and matches the proven `menuController` shape. No QML singletons exist in this repo, ruling out (c). |
| Instance sharing | (a) one bindings+model+menu trio per surface (tree, diagram), (b) one shared instance for the editor window | **(a)** — no cross-surface stale target, no wiring through `ProjectEditor.qml`; the objects are cheap QtObjects. |
| Pro gating | (a) binding-level `enabled: Cpp_CommercialBuild`, (b) manifest `pro: true` | **(a)** — the registry *drops* `pro` commands from the catalog on GPL builds, but the spec (R9) wants Pro entries visible and disabled, which is also today's diagram behavior. `pro` on a layout node stays available for entries that should vanish entirely. |
| Bulk entry labels | (a) binding `title` override honored by `join`, (b) a manifest-side plural scheme | **(a)** — one four-line change in `CommandModel.join`, and it generalizes to any state-dependent label; the counts already use `.arg()` (never `%n` with `.arg()`, per `common-mistakes.md`). |
| Toolbar conversion | (a) leave view toolbars alone, reimplement `GroupTemplateMenu` over the registry, (b) convert the toolbars too | **(a)** — spec non-goal; reimplementing just `GroupTemplateMenu` removes the third copy of the template list without touching eight view files. |

## Risks & mitigations

- **Stale target across a tree rebuild.** Any creation triggers `buildTreeModel()`, invalidating
  captured ids. Mitigation: items are built at open and destroyed at close; the target is cleared
  on close and on `treeModelChanged`; `run()` completes before the rebuild lands. Never call
  `buildTreeModel()` from inside a menu handler synchronously — the existing deferral rule
  (`common-mistakes.md`, "ProjectModel & Project Files") applies unchanged since we only call
  existing slots.
- **New `ItemKind` values leaking into bulk operations.** `selectedTreeItems()` now packages the
  section roots too. Every QML bulk call already filters by kind, but
  `deleteSelectedItems` / `duplicateSelectedItems` / `moveSelectedItemsToFolder` /
  `setItemsEnabled` must be read during implementation to confirm an unknown kind is a no-op, not a
  default-branch mutation. Section-root rows must also be excluded from the counters that drive the
  bulk entries.
- **Empty or double separators.** Moving separator logic from hand-written `visible:` expressions
  into the renderer is a behavior swap; the filtering rule above (suppress leading, trailing and
  consecutive separators post-visibility) must be implemented in the renderer, not per menu.
- **Regression surface.** Two menu systems collapse into one; R11 is the guard. The task list will
  carry the enumerated pre-change entry inventory (tree menu, and each of the eleven diagram menus)
  so AC9 is checked row by row rather than by eyeball.
- **Missing binding = silently dropped entry.** `CommandModel` drops unbound ids by design, so a
  typo in a manifest id yields a quietly shorter menu. Mitigated by the new binding-coverage check
  in `registry-verify.py` (R13/AC8).
- **GPL build divergence.** Pro entries are visible-but-disabled by binding, so a GPL build must be
  spot-checked (AC7); the existing commercial-guard scan over `Commands/*.qml` still applies to the
  new bindings file, which must reference no `Cpp_Licensing_` / `Cpp_Sessions_` / `Cpp_MQTT_`
  symbol (it needs none — all selectors live on `Cpp_JSON_ProjectEditor`).
- **Translation drift.** New titles must flow through `generate-command-strings.py`; `--check` in
  the commit pipeline catches a missed regeneration.

## Test & verification plan

- **Unit (runnable here):** none — `tests/scripts/` covers JS frame parsers only; this change has
  no JS parser or hotpath surface.
- **Static (runnable here):** `python scripts/registry-verify.py` (extended: menu manifest node
  checks, `include` resolution, binding coverage) → **AC8**;
  `python scripts/generate-command-strings.py --check` → **AC8/AC10**;
  `python scripts/code-verify.py --check` on every touched file → **AC8**.
- **Maintainer, in-app** (project exercising all kinds: two sources, folders in all three families,
  an output panel, a shared table with registers, a workspace, an MQTT publisher on a commercial
  build):
  - **AC1** right-click each of the nineteen kinds; **AC2** the six former dead zones plus tree
    background open, spacer row stays inert.
  - **AC3/AC4** create one of each object from the tree, then diff the saved project JSON against
    the same object created from the diagram / view toolbar; confirm parenting for source-scoped,
    folder-scoped and group-scoped adds regardless of prior selection. Cross-check live state
    through `tests/utils/api_client.py` against a running app on `localhost:7777`.
  - **AC5** add one new dataset type as a manifest + binding diff only, and observe it on both
    surfaces.
  - **AC6** three-row selection shows no creation entries and a count-bearing delete.
  - **AC7** GPL build: Pro entries visible and disabled; commercial build: enabled.
  - **AC9** walk the recorded pre-change entry inventory row by row.
  - **AC10** toggle a dataset's FFT from the menu; confirm the dataset view and the saved project
    agree, and that the check state reflects the dataset on reopen.
  - **AC11** switch language; menu text updates on next open with no restart.
- **Hotpath:** not applicable — nothing on the frame path is touched, so `--benchmark-hotpath` is
  not a gate for this change (CI still runs it).
- **Before handoff:** `qt-cpp-review` over the C++ diff (registry loader, tree stamping), then
  `python scripts/sanitize-commit.py`.
