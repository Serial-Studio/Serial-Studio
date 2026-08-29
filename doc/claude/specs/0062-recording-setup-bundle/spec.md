---
spec: 0062-recording-setup-bundle
title: Recording setup bundle
status: done          # closed 2026-08-20
created: 2026-08-16
author: Claude (overnight run, unattended; planned + implemented 2026-08-17 on maintainer request)
---

# Spec 0062 — Recording setup bundle

> **Phase 1 of 4 — the WHAT and the WHY.** Written 2026-08-16 as specify-only; on 2026-08-17 the
> maintainer asked for B to be planned and implemented — see `plan.md` for the decisions taken
> on the open questions and the deviations recorded there.

## Problem / Motivation

A Session Database recording already embeds the project JSON (`sessions.project_json`, plus
`project_metadata.project_json` for the live project), and `Sessions::Player` restores it on
playback (`restoreProjectFromJson`), so widget layout, workspaces, per-widget `widgetSettings`
(interpolation, area fill, sweep config, and since spec 0058 the ruler markers/zero) already
travel with the recording. What does **not** travel is the *view state* that made the samples
meaningful at the moment they were recorded and that lives outside the project document:
cursor positions, zoom/pan (the visible window per plot), which widgets were paused, which
workspace was on screen, external/pop-out windows, the plot time range when it comes from
QSettings (non-ProjectFile modes), the theme, and anything else that is session state rather
than project state. Playing back a session therefore opens the right dashboard but at the
default view, and the user re-finds what they were looking at. There is also no defined story
for "the project on disk changed after this recording was made": today the embedded copy
silently wins for the duration of playback and the pre-session project is restored afterwards
(`schedulePreSessionRestore`), which is right, but nothing tells the user that the two differ.

## Goals

- Bundle a `viewState` document with a recording: per-widget cursors, visible window (zoom/pan
  or the world window), paused flags, per-plot Y range override if the user set one in the
  dialog, active workspace, external windows, plot time range, and the theme id.
- Restore it on playback after the project restore, in the same order the app applies user
  actions, so the dashboard reopens *as it looked*.
- Snapshot cadence: at recording start, on every debounced `widgetSettingsChanged`, and at
  recording end (so the bundle reflects the last state, not the first).
- Explain divergence: when the embedded project differs from the project currently loaded,
  playback says so once and offers "use recording's project" (default) or "keep mine".

## Non-Goals

- Recording per-frame view changes as a timeline (no "replay my zooming").
- Changing what the project JSON contains or how `restoreLastProject` works.
- Bundling for CSV/MDF4 replays (they have no per-session container to carry it).

## Requirements

1. **R1** — `viewState` is one JSON document per session (`sessions.view_state TEXT`, schema
   bump), written by the DB worker off the GUI thread from a GUI-side snapshot.
2. **R2** — Contents (all optional, absent = default): `plotTimeRange`, `theme`, `workspace`,
   `externalWindows[]`, and per widget id: `cursors {ax, ay, bx, by, aVisible, bVisible}`,
   `view {xZoom, xPan, yZoom, yPan}` (or the world window), `paused`, `yRange {min, max}` when
   user-set.
3. **R3** — Snapshot triggers: recording start, `widgetSettingsChanged` (already debounced),
   cursor/zoom changes coalesced on a 1.5 s timer (mirroring the autosave debounce), and
   recording end.
4. **R4** — Playback order: restore project JSON (existing), reconfigure dashboard, then apply
   `viewState` once the widgets exist (after `widgetCountChanged`), never before.
5. **R5** — Divergence notice: compare embedded `project_json` with the live project's
   serialization (title + a content hash); on mismatch show one non-modal notice with the two
   choices; "keep mine" plays the samples through the current project (datasets matched by
   `uniqueId`, unmatched ignored).
6. **R6** — Everything degrades: a session without `view_state` plays exactly as today; a
   `viewState` referencing a widget id that no longer exists is skipped silently.
7. **R7** — Leaving playback restores the pre-session project and view exactly as today
   (`schedulePreSessionRestore`); the bundle never leaks into the live project.

## Acceptance Criteria

- [x] **AC1** — Record with two cursors on plot 1, zoomed 4x on plot 2, plot 3 paused, workspace
      "Bench" active; stop; play back: all four are as they were.
- [x] **AC2** — Edit the project (rename a dataset) after recording; play back: the notice
      appears once; "use recording's project" shows the old name, "keep mine" the new one.
- [x] **AC3** — A pre-0062 session file plays back unchanged (no notice, default view).
- [x] **AC4** — Stop playback: the live project and view are what they were before playback.
- [x] **AC5** — No GUI-thread DB access (existing rule); snapshot cost is not per frame.

## Constraints & Invariants

- Session DB rule set: worker-thread writes only, surrogate keys, no `INSERT OR IGNORE`,
  schema version bump with a migration that adds the nullable column.
- Composition root, `restoreLastProject`, `SessionContext` untouched: this composes with the
  existing pre-session-restore path, it does not replace it.
- View state is *not* project state: it must never call `setModified(true)` on the project.

## Relation to `restoreLastProject`

`restoreLastProject` reopens the last project path from QSettings at startup and re-applies
the persisted operation mode; playback already sidesteps it by swapping the embedded project
in and restoring the pre-session project on close. This spec adds a second, smaller document
beside the embedded project rather than folding view state into the project JSON (which would
make every zoom mark the project modified and would land in the `.ssproj` on save). Nothing in
the startup path changes.

## Open Questions for Alex

- Should the theme be part of the bundle at all? (Restoring a theme on playback is surprising;
  recording it for context and *not* applying it may be enough.)
- Snapshot cadence: is 1.5 s coalescing on cursor/zoom acceptable, or should view state be
  captured only at start/stop?
- "Keep mine" matching by `uniqueId` only, or also by title as a fallback?
- Where does the divergence notice live: Problem Center finding, or the notification centre?
