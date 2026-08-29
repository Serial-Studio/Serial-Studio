---
spec: 0062-recording-setup-bundle
phase: tasks
status: approved     # gate auto-approved, overnight run 2026-08-17
updated: 2026-08-17
---

# Tasks 0062 — Recording setup bundle

- [x] T1 `UI::Dashboard` view-state store + signal
- [x] T2 `Plot.qml` / `MultiPlot.qml` push + restore
- [x] T3 `Sessions::Export` snapshot + debounce + worker `storeViewState`, `sessions.view_state` column, `kUserVersion` 4
- [x] T4 `PlayerLoaderWorker` payload + `Sessions::Player` capture / apply / restore / divergence notice
- [x] T5 docs (`export.md`)
- [x] T6 workspace + external windows in the bundle (needs a Taskbar wire in the composition root)
