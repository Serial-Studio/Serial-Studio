---
spec: 0063-context-menu-registry
phase: inventory
updated: 2026-08-18
---

# Pre-change menu inventory (T1)

Baseline for **AC9** — every entry listed here must still be reachable after the migration.
Captured from `ProjectStructure.qml` and `FlowDiagram.qml` before any menu was deleted.

## Tree — `ProjectStructure.qml` `sharedContextMenu`

One menu for every kind, entries toggled by `visible:`. 13 `MenuItem`s + 1 dynamic submenu.

| # | Title | Icon | Shown when | Calls |
|---|-------|------|-----------|-------|
| 1 | Expand All | `editor/expand` | always | `ProjectEditor.expandAllTreeItems()` |
| 2 | Collapse All | `editor/compress` | always | `ProjectEditor.collapseTreeToOverview()` |
| 3 | Move Up | `editor/move-up` | `ctxItemOrderable` | `moveContextItemBy(-1)` → `moveGroup` / `moveDataset` / `moveAction` / `moveOutputWidget` / `moveWorkspace` / `move*FolderInParent` |
| 4 | Move Down | `editor/move-down` | `ctxItemOrderable` | `moveContextItemBy(+1)`, same fan-out |
| 5 | Rename | `editor/rename` | `ctxRenameable` | `promptRenameGroup` / `promptRenameDataset` / `promptRenameAction` / `promptRenameSource` / `promptRenameWorkspace` / `promptRenameTable` / `promptRename*Folder` |
| 6 | Hide / Show, or Hide/Show Selected (n) | `editor/hide`, `editor/show` | `enableableSelectionCount > 0` | `setItemsEnabled(items, !ctxItemEnabled)` |
| 7 | Duplicate, or Duplicate Selected (n) | `editor/duplicate` | `selectableSelectionCount > 0` | `duplicateSelectedItems(items)` |
| 8 | Delete, or Delete Selected (n) | `editor/delete` | `deletableSelectionCount > 0` | `confirmDeleteWorkspace` / `confirmDeleteWorkspaceFolder` / `confirmDeleteGroupFolder` / `confirmDeleteTableFolder` / `confirmDeleteTable` for a single special row, else `confirmDeleteSelectedItems(items)` |
| 9 | Seed Aliases from Titles | `editor/rename` | always | `seedDatasetAliases()` |
| 10 | New Folder | `editor/add-folder` | `ctxSupportsFolders` | `promptAddGroupFolder(-1)` / `promptAddTableFolder(-1)` / `promptAddWorkspaceFolder(-1)` |
| 11 | New Sub-Folder | `editor/add-folder` | `ctxIsFolder` | same family, `parentFolderId = ctxItemId` |
| 12 | Move to Folder ▸ | `editor/move-folder` | `ctxSupportsFolders` | cascading submenu, built per open by `rebuildMoveMenu()` |
| 12a | └ Top Level | `editor/top-level-folder` | inside the cascade | `moveCtxItemToFolder(-1)` |
| 12b | └ *folder* ▸ Move Here | `widgets/folder`, `editor/move-here` | one per folder, recursive | `moveCtxItemToFolder(folderId)` |

Kind predicates that drive the visibility: `ctxItemOrderable` (group, dataset, action, output
widget, workspace, all three folder kinds), `ctxIsFolder`, `ctxSupportsFolders` (folders +
group + workspace + user table), `ctxRenameable` (folders + group, dataset, action, source,
workspace, user table).

Keyboard paths in the same file that must keep working: `Alt+Up`/`Alt+Down` reorder,
`Delete` on the selection.

## Diagram — `FlowDiagram.qml`, eleven menus

Shared `Action` objects behind the "Add …" submenus (all wrapped in `menuController.locked()`,
which suppresses the view change, and preceded by `selectTargetGroup()` where a group is the
target):

| Action | Title | Icon | Call |
|--------|-------|------|------|
| `actAddGroupGeneric` | Dataset Container | `editor/group` | `addGroup("Dataset Container", NoGroupWidget, sourceId)` |
| `actAddGroupMultiPlot` | Multi-Plot | `widgets/multiplot` | `addGroup("Multiple Plot", MultiPlot, sourceId)` |
| `actAddGroupAccel` | Accelerometer | `widgets/accelerometer` | `addGroup("Accelerometer", Accelerometer, sourceId)` |
| `actAddGroupGyro` | Gyroscope | `widgets/gyroscope` | `addGroup("Gyroscope", Gyroscope, sourceId)` |
| `actAddGroupGps` | GPS Map | `widgets/gps` | `addGroup("GPS Map", GPS, sourceId)` |
| `actAddGroupPlot3D` | 3D Plot | `widgets/plot3d` | `addGroup("3D Plot", Plot3D, sourceId)` |
| `actAddGroupImage` | Image View | `widgets/image` | `addGroup("Image View", ImageView, sourceId)` |
| `actAddGroupPainter` | Painter Widget | `editor/add-painter` | `addGroup("Painter Widget", Painter, sourceId)` |
| `actAddGroupWebView` | Web View | `widgets/webview` | `addGroup("Web View", WebView, sourceId)` |
| `actAddGroupDataGrid` | Data Grid | `widgets/datagrid` | `addGroup("Data Grid", DataGrid, sourceId)` |
| `actAddGroupBarPanel` | Bar Panel | `widgets/barpanel` | `addGroup("Bar Panel", BarPanel, sourceId)` |
| `actAddDsGeneric` | Generic | `editor/dataset` | `addDataset(DatasetGeneric, sourceId)` |
| `actAddDsPlot` | Plot | `widgets/plot` | `addDataset(DatasetPlot, sourceId)` |
| `actAddDsFFT` | FFT Plot | `widgets/fft` | `addDataset(DatasetFFT, sourceId)` |
| `actAddDsGauge` | Gauge | `widgets/gauge` | `addDataset(DatasetGauge, sourceId)` |
| `actAddDsBar` | Level Indicator | `editor/add-bar` | `addDataset(DatasetBar, sourceId)` |
| `actAddDsCompass` | Compass | `widgets/compass` | `addDataset(DatasetCompass, sourceId)` |
| `actAddDsMeter` | Meter | `widgets/meter` | `addDataset(DatasetMeter, sourceId)` |
| `actAddDsLED` | LED Indicator | `widgets/led` | `addDataset(DatasetLED, sourceId)` |
| `actAddOutPanel` | Output Panel | `widgets/output-panel` | `addOutputPanel(sourceId)` |
| `actAddOutSlider` | Slider | `editor/output-slider` | `addOutputControl(OutputSlider, sourceId)` |
| `actAddOutToggle` | Toggle | `editor/output-toggle` | `addOutputControl(OutputToggle, sourceId)` |
| `actAddOutKnob` | Knob | `editor/output-knob` | `addOutputControl(OutputKnob, sourceId)` |
| `actAddOutText` | Text Field | `editor/output-textfield` | `addOutputControl(OutputTextField, sourceId)` |
| `actAddOutButton` | Button | `editor/output-button` | `addOutputControl(OutputButton, sourceId)` |

The three submenus carry `enabled: Cpp_CommercialBuild` on **Add Output** only; group and
dataset submenus are ungated (individual Pro widget types are handled downstream).

### `backgroundMenu`

Add Group ▸ (11) · Add Dataset ▸ (8) · Add Output ▸ (6, Pro-gated) · — · Add Action
(`addAction`) · Add Data Source (`addSource`) · Add Data Table (`promptAddTable`).

### `sourceMenu`

Add Group ▸ · Add Dataset ▸ · Add Output ▸ · Add Action · — · Rename… (`promptRenameSource`) ·
Duplicate (`duplicateSource`) · — · Delete… (`deleteSource(id, true)`).

### `frameparserMenu`

Add Group ▸ · Add Dataset ▸ · Add Output ▸ · — · Edit Frame Parser…
(`ProjectEditor.selectFrameParser(sourceId)`).

### `groupMenu`

Add Dataset ▸ · Add Output ▸ · Edit Painter Code… (enabled when `widget === "painter"` and
`Cpp_CommercialBuild`; selects the group then opens `painterCodeDialog`) · — · Rename…
(`promptRenameGroup`) · Move Up / Move Down (`moveGroup ±1`, enabled by `canMoveUp`/`canMoveDown`) ·
Duplicate (`duplicateGroup`) · — · Delete… (`deleteGroup(id, true)`).

### `datasetMenu`

Edit Transform Code… (`ProjectEditor.openTransformEditorFor(groupId, datasetId)`) · — · Rename…
(`promptRenameDataset`) · Move Up / Move Down (`moveDataset ±1`) · Duplicate (`duplicateDataset`) ·
— · Delete… (`deleteDataset(groupId, datasetId, true)`).

### `outputMenu`

Move Up / Move Down (`moveOutputWidget ±1`) · Duplicate (`duplicateOutputWidget`) · — · Delete…
(`deleteOutputWidget(groupId, widgetId, true)`).

### `outputPanelMenu`

Add Output ▸ (5 — no Output Panel entry) · — · Rename… (`promptRenameGroup`) · Duplicate
(`duplicateGroup`) · — · Delete… (`deleteGroup(id, true)`).

### `actionMenu`

Rename… (`promptRenameAction`) · Move Up / Move Down (`moveAction ±1`) · Duplicate
(`duplicateAction`) · — · Delete… (`deleteAction(id, true)`).

### `tableMenu`

Rename… (`promptRenameTable(tableName)`) · — · Delete… (`confirmDeleteTable(tableName)`).

### `transformMenu`

Edit Code… (`openTransformEditorFor(groupId, datasetId)`).

### `controlScriptMenu`

Edit Control Loop… (`ProjectEditor.selectControlScript()`).

## Reconciliation

`grep -c "MenuItem {"`: `ProjectStructure.qml` = 13, `FlowDiagram.qml` = 130,
`GroupTemplateMenu.qml` = 11.

`FlowDiagram`'s 130 = the per-menu totals above (32 + 34 + 30 + 24 + 8 + 5 + 11 + 6 + 3 + 1 + 1 =
155 rows listed, of which 25 are separators/submenu headers), i.e. every literal `MenuItem` line,
counting the shared "Add …" entries once per menu that repeats them. That repetition — the same
25 actions re-listed across five menus — is what the `include` fragments replace.

`GroupTemplateMenu.qml`'s 11 entries are a third copy of the group-template list (used by the
`GroupsView` and `GroupFolderView` toolbars); after T17 they resolve from the same `add-group`
fragment.
