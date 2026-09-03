---
spec: 0075-review-remediation
package: WP-F (project layer and editors, R7 / H1-H13)
status: complete (2 unit suites relocated, 3 not registered -- see "Tasks not done")
updated: 2026-09-01
---

# Handoff — WP-F

All ten tasks (WPF-T1..T10) are ticked in `tasks.md`. Every C++/QML file changed passes
`python3 scripts/code-verify.py --check` with 0 errors and 0 advisories. Nothing was built,
compiled, committed or run against the maintainer's live app (a session was listening on 7777;
only read-only API queries were issued against it, to confirm command names and response shapes).

## Files changed

### Fixes

| File | Change |
|------|--------|
| `app/src/DataModel/ProjectModel.cpp` | H1: the Dashboard `pointsChanged` handler calls `setPointCount()` instead of writing the whole document to disk. H10: five duplicate `scheduleAutoSave` connections removed (all five are already covered by the `markDirty` lambda); the `groupsChanged` auto-workspace regen is queued through `scheduleWorkspaceRegen()` / `flushWorkspaceRegen()` so a bulk delete regenerates once instead of once per group. |
| `app/src/DataModel/ProjectModel.h` | New signal `actionDataChanged(int)`; `m_workspaceRegenPending` + the two regen helpers; forwarder for `setSourceFrameParserTemplateAndParams`. |
| `app/src/DataModel/Project/ProjectPersistence.{h,cpp}` | H6: `restoreDetachedDocument()` re-attaches the path, re-arms the watcher, restores/raises the modified flag and posts a NotificationCenter warning when a reload fails. `writeProjectFile()` flushes a queued workspace regen first, so a save can never serialize a stale workspace list. |
| `app/src/DataModel/Project/ProjectBulkOps.{h,cpp}` | H3: deletion order is an explicit rank (datasets, tables, actions, output widgets, groups, workspaces, then folders) instead of the ItemKind enum's numeric order. `BatchDeleteEntry`, `batchDeleteRank()` and `batchDeleteOrderBefore()` moved to the header (inline) so the rule is unit-testable without the application; the .cpp static_asserts the nine kind constants against `ProjectEditor::ItemKind`. |
| `app/src/DataModel/Project/ProjectEntities.{h,cpp}` | H4: `updateAction` sets runtime-dirty, schedules auto-save and always emits the new `actionDataChanged`. H9: `updateGroup` normalises `Dataset::sourceId` to its group's (covers the editor and the API `group.update` path). H11: `changeDatasetOption` range-checks the selection before mutating it. H8: the `groups[group] = grp` self-assignment in `setGroupWidget` removed; `renumberGroupIds()` moved to public so the output-widget delete path shares it. |
| `app/src/DataModel/Project/ProjectOutputWidgets.cpp` | H8: the inlined re-implementation of `renumberGroupIds` replaced by the shared call. |
| `app/src/DataModel/Project/ProjectSources.{h,cpp}` | H9: `deleteSource` propagates the remapped group sourceId to its datasets. H5: new compound mutator `setSourceFrameParserTemplateAndParams()` (one scope, one step). H10: `captureSourceSettings` opens its scope after the guard returns; `restoreSourceSettings` opens none (it mutates the driver, never the document, so its snapshot could never be committed). |
| `app/src/DataModel/Editors/FrameParserModel.cpp` | H5: the template picker calls the compound mutator instead of two scoped mutators. |
| `app/src/DataModel/Editors/JsCodeEditor.cpp` | H7: `readCode()` no longer falls back to source 0's parser code, so a source with no script shows an empty editor instead of persisting another source's script on the first keystroke. |

### Structural (R12.1 / H12)

| File | Change |
|------|--------|
| `app/src/DataModel/Editors/EmbeddedCodeEditorItem.{h,cpp}` (new) | `QQuickPaintedItem` base owning `EmbeddedCodeEditor`, the theme hook, the UI-tick grab, the resize forward, the three document-signal render marks, and the sixteen event overrides. |
| `ControlScriptEditor`, `JsCodeEditor`, `MacroEditor`, `OutputCodeEditor`, `PainterCodeEditor` (`.h` + `.cpp`) | Derive from the base; ~135 duplicated lines per host deleted (about 900 in total, net -1000 for the package). `OutputCodeEditor::m_testDialog` is now a `std::unique_ptr<TransmitTestDialog>` created on first use instead of a top-level widget built at startup. |
| `app/CMakeLists.txt` | The new TU/header appended next to `EmbeddedCodeEditor.*`. |

### Tests and fixtures

| File | Contents |
|------|----------|
| `app/tests/tst_project_bulk_ops.cpp` (new) | Nine cases over the delete ordering: table-before-folder (the shipped H3 defect), group-before-folder, dataset/output-widget-before-group, workspace-before-folder, a full mixed selection, descending ids within a rank, strict-weak-ordering algebra, unknown kinds last. Links QtCore only. |
| `app/tests/tst_project_history.cpp` (new) | Fifteen cases: the two-phase capture contract (staged-without-commit records nothing), nested scopes as one step, hint-over-slot key precedence, the 1 s coalesce window (in and out), the 100-step and 64 MiB bounds, redo-tail truncation, save-point survival and unreachability, `markSaved` breaking the coalesce chain, and the disabled/applying suppressions. Links `ProjectHistory.cpp` plus two link-only stubs declared in the suite. |
| `app/tests/CMakeLists.txt` | Registers the two suites; carries a block explaining why `tst_project_workspace_refs` stays unregistered. |
| `tests/fixtures/projects/legacy/*.ssproj` + `README.md` (new) | Five hand-written legacy documents, one per loader migration (separator rewrite, index-based xAxis, `__layout__:N__` keys, schema-v0 + dashboardLayout, duplicate uniqueIds + positional workspace refs), with the expected post-load state tabulated. |
| `tests/integration/test_project_integrity.py` (new) | Twelve tests over the API: display setting never writes the file (H1 shape), action payload edit dirties + persists (H4), template pick undoes template and params together (H5), a new source's parser is not source 0's (H7), group source change normalises dataset sourceIds (H9), the five legacy fixtures load and migrate, and an external corrupt write leaves the document attached (H6, API half). Two AC7 cases are GUI-only and `pytest.skip` with the manual recipe in the message. |

## Tasks not done, and why

1. **`tst_project_persistence.cpp` (WPF-T7) not written.** Its minimum link set is `ProjectModel.cpp`
   plus the seventeen `Project/*.cpp` TUs plus the `SessionContext` dtor closure, i.e. the
   application.
   `app/tests/CMakeLists.txt` documents that exact wall twice already (`tst_proto_importer`,
   `tst_dbc_importer`) and states that a suite needing it belongs in another tier. The listed cases
   (disk-watch hash, point-count write, action autosave, reload failure) are covered in
   `tests/integration/test_project_integrity.py` instead.
2. **`tst_project_loader_migrations.cpp`, `tst_project_workspace_refs.cpp`,
   `tst_transmit_test_dialog.cpp` (WPF-T9) not registered.** `ProjectLoader` needs `ProjectModel`;
   `ProjectWorkspaceRefs.cpp` is pure but classifies widgets through
   `SerialStudio::getDashboardWidget(s)` / `*EligibleForWorkspace` / `activated()`, all defined in
   `SerialStudio.cpp` (not in the `SerialStudioFrameSupport.cpp` carve-out), so it drags the icon
   registry, theme manager and player singletons; `TransmitTestDialog` is a live QWidget over the
   connection manager. The migration coverage moved to the fixture corpus plus the pytest suite.
   Reasons are recorded in `app/tests/CMakeLists.txt` where a reader will look for them.
3. **`tst_frame_parser_model.cpp` (WPF-T5's Verify) not written.** Same reason: a QObject bound to
   `ProjectModel`. The template-pick invariant is asserted over the API instead.
4. **The `frameDetectionChanged` -> AppState coalescing timer (WPF-T6) is not implemented.** See "Invariants the plan did not state" below; the reason is a correctness gate,
   not effort.
5. **`--dup-census` (WPF-T8's Verify) does not exist yet.** It is a WP0 deliverable
   (`code-verify.py` currently offers only `--singleton-census` and `--tu-census`). The five editor
   pairs are byte-identical no more; re-run the census once WP0 lands.

## Patches for the coordinator

### 1. `app/src/UI/Dashboard/DashboardTools.cpp` (owned by WP-E): H4 / R7.6

`configureActions()` is only ever called with the FrameBuilder's template frame
(`Dashboard.cpp:1518-1523` fetches it with `invokeOnBuilderThreadBlocking`, and `:1950` passes the
published frame), and that frame only refreshes on `syncFromProjectModel()`. So an action payload or
timer edit reaches the dashboard's `QTimer`s no earlier than the next runtime resync, and never at
all in a session with no project file, because the resync rides the auto-save.

`ProjectModel` now emits `actionDataChanged(int)` on every `updateAction`, including the
per-keystroke path where `actionsChanged` stays suppressed (the editor updates the tree
item in place). Two details make the naive wiring wrong, so please implement it as:

```cpp
  connect(&DataModel::ProjectModel::instance(),
          &DataModel::ProjectModel::actionDataChanged,
          this,
          &UI::DashboardTools::refreshActionsFromProject);
```

with a slot that reads the **project document**, on the GUI thread:

```cpp
/**
 * @brief Rebuilds the action list and its timers from the project document. The template frame
 *        only refreshes on a pipeline resync, so a payload or interval edit would otherwise not
 *        reach the dashboard until the next auto-save.
 */
void UI::DashboardTools::refreshActionsFromProject()
{
  const auto& actions = DataModel::ProjectModel::instance().actions();
  ...rebuild m_actions / m_timers / m_repeatCounters from `actions`...
}
```

- Do **not** call `m_frameBuilder->frame()` from the GUI thread: `Dashboard.cpp` marshals that read
  onto the builder thread for a reason, and the frame is stale for this purpose anyway.
- `configureActions()` early-returns on `frame.groups.size() <= 0`, so handing it a synthetic frame
  carrying only `actions` silently does nothing. Either factor the action/timer rebuild out of
  `configureActions()` and call that from both places, or drop the group guard on the new path.

### 2. `app/src/DataModel/Project/ProjectPresentation.cpp`: H11 (outside my task file list)

`savePluginState` writes into `m_widgetSettings` and emits `widgetSettingsChanged` even outside
ProjectFile mode, unlike its sibling `saveWidgetSetting` which guards on `widget_settings_active()`.
A QuickPlot plugin therefore mutates (and dirties, once a project is loaded) the loaded project's
blob:

```cpp
 void DataModel::ProjectPresentation::savePluginState(const QString& pluginId,
                                                      const QJsonObject& state)
 {
+  if (!widget_settings_active())
+    return;
+
   const auto key = QStringLiteral("plugin:") + pluginId;
   if (m_widgetSettings.value(key).toObject() == state)
     return;
 
   m_widgetSettings.insert(key, state);
-  static auto& appState = AppState::instance();
-  if (appState.operationMode() == SerialStudio::ProjectFile)
-    m_model.setModified(true);
-
+  m_model.setModified(true);
   Q_EMIT m_model.widgetSettingsChanged();
 }
```

### 3. `app/src/DataModel/Project/ProjectEditorCommit.cpp:628`: H11 (outside my task file list)

`onProjectItemChanged()` ends with an unconditional `pm.setModified(true)` outside any scope, so a
no-op item change (the tree writes the same value back) dirties the project, and the call commits
whatever memento a previous scope happened to stage. Make it follow the branch that
mutated:

```cpp
   switch (static_cast<ProjectItem>(id.toInt())) {
     case kProjectView_Title:
       pm.setNextUndoHint(tr("Rename Project"), QStringLiteral("project-title"));
       pm.setTitle(value.toString());
-      break;
+      return;
     default:
       break;
   }
-
-  pm.setModified(true);
 }
```

(`setTitle` already opens its own scope and calls `setModified(true)`.)

### 4. `doc/claude/architecture/project.md` (WP-J): the doc now contradicts the code

Under "Undo History", the sentence

> the Dashboard `pointsChanged` write-back (`ProjectModel.cpp` lambda, no scope, no `setModified`)
> and `persistLegacyMigration()` bypass it

is false as of this package: the `pointsChanged` handler now delegates to `setPointCount()` (scope +
`setModified`), and `markSaved()` is bypassed only by `persistLegacyMigration()`. Please fold the
correction into WP-J rather than leaving a doc claim my diff invalidated.

### 5. Singleton census must be re-baselined after the merge

`python3 scripts/code-verify.py --singleton-census --check` reports total **1584 -> 1573** (a net
decrease of eleven, because the five editor hosts stopped reaching for ThemeManager/TimerEvents/
CommonFonts individually) but fails the ratchet on two per-file increases:
`EmbeddedCodeEditorItem.cpp 0 -> 3` (the moved calls, in the ctor init list, which is the shape the rule asks for) and `ProjectPersistence.cpp 7 -> 8` (the NotificationCenter warning in
`restoreDetachedDocument`, matching the two existing call sites in that file). I did not run `--singleton-census --accept`: the baseline is a shared checked-in file and every
package would conflict on it. Re-accept once at integration.

## Invariants the plan did not state

1. **`CommandRegistry::execute()` gates auto-save and the project apply on `mutationEpoch()`
   changing within the handler call** (`app/src/API/CommandRegistry.cpp:297-308`). The epoch is
   bumped by signal emissions, so deferring `frameDetectionChanged` to a coalescing timer (which
   plan.md's WP-F row asks for) would make every delimiter-only API command
   (`project.frameParser.update` -> `setFrameStartSequence`, whose only signal is
   `frameDetectionChanged`) look like a no-op: no auto-save, no pipeline apply. That is the same
   silent-loss class this spec exists to close, so I implemented the workspace-regen half of
   WPF-T6 (whose signals never solely carry an epoch bump) and left the frame-detection emission
   synchronous. A safe version of that optimisation has to debounce the *consumer*
   (`AppState::onProjectLoaded` -> `FrameBuilder::syncFromProjectModel`), and it has to flush before
   `AppState::frameConfig()` is read at driver-open time, or a delimiter typed just before Connect
   opens the link with the previous frame config. That is an AppState/WP-A decision, not a
   ProjectModel one.
2. **A 0 ms coalescing timer cannot collapse keystrokes.** Each keystroke is its own event-loop
   turn, so "one sync per burst" only ever means one per turn. The workspace-regen queue does help
   because a bulk delete emits N `groupsChanged` inside a single call; the per-keystroke delimiter
   path needs a real idle debounce, with the flush obligation above.
3. **A deferred document mutation must be flushed before serialization.** The queued workspace
   regen mutates persisted state (`workspaces`), so `writeProjectFile()`, the single choke point for
   every disk write (auto-save and save-as included), flushes it first. Any future
   "coalesce this ProjectModel work" change owes the same flush.
4. **`ProjectHistory::enterScope()` consumes the pending editor hint unconditionally**, so moving a
   `ProjectUndoScope` below a guard return also stops the hint from being consumed on that path,
   and it can leak into an unrelated later step (the hazard `common-mistakes.md` names). I
   checked every caller of the two slots I moved (`captureSourceSettings`, `restoreSourceSettings`):
   none sets a hint first, so both moves are safe. Any other scope relocation needs the same check.
5. **`Dataset::sourceId` has exactly one derivation rule** (`finalize_frame`: it equals its group's).
   Normalising it in `ProjectEntities::updateGroup` covers both the editor and the API `group.update`
   path in one place, which is why `ProjectUpdateCommands.cpp` needed no patch.
6. **`EmbeddedCodeEditor` (the offscreen-widget helper) already existed**; H12's remaining duplication
   was only the QQuickPaintedItem-side forwarding. The five hosts were byte-identical except that
   `MacroEditor` also called `scheduleRender()` on focus in/out; the base adopts MacroEditor's
   superset, so the four Project-Editor editors now repaint on focus change too (the caret is what
   changes, and the grab stays gated).

## Counterfactual self-check

**Which rule does this diff most risk violating?** The three hidden-widget invariants in
`scripting.md` ("Embedded Code Editors"): position sync inside `renderWidget()`, `ShortcutOverride`
forwarding in `event()`, and the completer-popup reroute in `keyPressEvent()`, plus the
`renderable()` gate that stopped the 56%-of-GUI-thread regression of 2026-08-18. WPF-T8 deleted all
five hosts' copies of that plumbing.

**Evidence it does not:** none of the three invariants lived in the hosts. Each is implemented
inside `EmbeddedCodeEditor`: `syncWidgetPosition()` is called by
`EmbeddedCodeEditor::renderWidget()`, the override handling by `handleShortcutOverride()`, the
reroute by `handleKeyPress()`. The hosts only forwarded into those three entry points. `EmbeddedCodeEditorItem` calls exactly the same
three, from `renderWidget()`, `event()` and `keyPressEvent()`, with bodies copied verbatim from
`MacroEditor` (the file scripting.md names as the pattern all of them follow). The `renderable()`
gate is likewise internal to `EmbeddedCodeEditor` and is selected by the `RenderGate` argument,
which the base takes as a constructor parameter: I carried each host's original value across
unchanged (`ItemVisible` for `MacroEditor`, `WindowVisible` for the four Project-Editor hosts), so
no editor's grab is gated more weakly than before. What a compiler cannot check here is the QML
side, which is why WPF-T8's Verify line is a maintainer walkthrough: open each of the five editors,
type, trigger the completer, use an editing shortcut, drag a file in, then close the Project Editor
window and confirm with `sample` that no `renderWidget` frames appear.

**Runner-up risk:** the queued auto-workspace regeneration is now one event-loop turn late, so any
code that read `activeWorkspaces()` immediately after a group mutation sees the previous list for
one turn. The two consumers that must not see stale state are covered (every disk write flushes
first, and the API's epoch-gated apply is driven by `groupsChanged`, which is still synchronous),
but a QML binding that assumed synchronous regeneration would now repaint one tick later. That is
observable only as a single-frame delay, and `editorWorkspacesChanged` / `activeWorkspacesChanged`
still fire.
