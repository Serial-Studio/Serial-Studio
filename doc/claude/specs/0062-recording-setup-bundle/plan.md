---
spec: 0062-recording-setup-bundle
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-17 (unattended; maintainer re-reviews)
updated: 2026-08-17
---

# Plan 0062 — Recording setup bundle

## Approach (one paragraph)

View state lives in `UI::Dashboard` (GUI thread, never in the project document): a per-widget
`QJsonObject` store + a global one, `saveWidgetViewState(widgetId, key, value)` /
`widgetViewState(widgetId)` / `viewStateJson()` / `setViewStateJson()` / `clearViewState()`,
emitting `viewStateChanged` only on real changes. `Plot.qml` / `MultiPlot.qml` push cursors,
zoom/pan, crosshair mode and pause through a 500 ms coalescing timer and read them back in
`Component.onCompleted`, so a dashboard rebuilt after a project restore applies the state
without any explicit ordering. `Sessions::Export` snapshots `viewStateJson()` beside the project
snapshot (same mutex), debounces pushes 1.5 s, and the `ExportWorker` writes
`sessions.view_state` at session start (`insertSession`), on each push (`storeViewState`, queued)
and in `finalizeSession`. `PlayerLoaderWorker` reads `view_state` into the payload;
`Sessions::Player` captures the pre-session view state with the pre-session project, applies the
bundle after `restoreProjectFromJson`, restores the captured one in `restorePreSessionState`, and
posts one Notification-Center warning when the embedded project differs from the project on disk
(recording's project wins, as before). Schema: `view_state TEXT` joins the nullable
`migrateSessionsTable` column list; `kUserVersion` 3 -> 4.

## Decisions taken for the open questions

| Question | Decision |
|----------|----------|
| Theme in the bundle | not recorded, not applied |
| Snapshot cadence | 500 ms QML coalesce + 1.5 s worker debounce; start and end always |
| "Keep mine" choice | notice only (no modal: API-driven playback must never block); recording's project always wins |
| Divergence notice home | Notification Center (`Sessions` channel) |
| Workspace / external windows | not bundled in this slice (needs a composition-root wire from Taskbar); noted |

## Hotpath & threading impact

None. GUI-thread JSON writes at interaction rate; worker-thread SQL only.

## Test & verification plan

- Spec AC1 (cursors/zoom/pause), AC3 (pre-0062 archive), AC4 (stop restores) in the running app;
  AC2 as a notice rather than a choice.
- `tst_sessions_legacy_archive` still passes (nullable column, legacy read path untouched).
