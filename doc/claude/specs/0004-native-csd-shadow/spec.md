---
spec: 0004-native-csd-shadow
title: Native OS/WM window shadows for CSD windows
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-07
author: Alex Spataru
---

# Spec 0004 — Native OS/WM window shadows for CSD windows

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

On the platforms where Serial Studio draws its own window chrome (Windows 10 and Linux —
Windows 11 and macOS already use native decorations), the app also paints its own drop
shadow. To do that, every CSD window is created larger than its visible content: an
invisible shadow band surrounds the window on all sides, and everything that deals with
window geometry has to compensate for it.

That compensation leaks, and the maintainer hits four recurring defect classes in daily
use:

1. Dialogs sometimes compute the wrong window size, because their size math must add the
   invisible margins back in and doesn't always get it right.
2. Windows cannot be moved flush against screen edges — the invisible shadow band hits the
   edge first, leaving a visible gap.
3. Resize handles activate over the shadow band instead of on the visible window border,
   so the cursor changes and drags start in what looks like empty space.
4. Combobox/dropdown popups can extend into or beyond the invisible band, where they are
   clipped or not clickable.

Beyond the defects, the painted shadow costs render time on every frame the chrome
repaints, and the geometry-compensation code is a standing source of bugs. Compositors
already know how to draw window shadows natively; delegating to them removes the entire
defect class instead of patching each symptom.

## Goals

- A CSD window's geometry equals its visible content — no invisible margins on any side,
  on any platform.
- Where the OS/compositor can draw a shadow around our frameless window, it does, and the
  shadow looks and behaves like every other native window's (active/inactive states,
  stacking, snapping).
- Where it cannot, the window degrades to a clean thin-border look with no shadow.
- The custom shadow painting and its geometry compensation are removed, not kept as a
  fallback.
- The existing custom CSD titlebar (colors, min/max/close controls, drag/double-click
  behavior) is unchanged.

## Non-Goals

- No change to Windows 11 or macOS window decoration behavior (both already native).
- No server-side decorations on Wayland: the custom CSD titlebar stays; Wayland windows
  simply have no shadow (plus the fallback border). Compositor-drawn full decorations are
  out of scope.
- No new user-facing setting: the existing "CSD shadow" toggle is removed, not
  generalized. Shadow presence becomes a platform decision.
- No redesign of the CSD titlebar visuals or window controls.
- No attempt to replicate the exact look of the current painted shadow; native shadows
  look like the platform, not like the old chrome.

## Requirements

1. **R1 — True-size windows.** On every platform, the window geometry reported to and used
   by the system (position, size, minimum size) matches the visible window exactly.
2. **R2 — Dialog sizing.** Dialogs open at the size their content requests — no dialog
   opens with extra empty bands or clipped content attributable to decoration margins.
3. **R3 — Edge travel.** A CSD window can be moved and snapped so its visible border sits
   flush against any screen edge, with no gap.
4. **R4 — Resize hit zones.** Resize cursors and drags activate on (and only on) the
   visible window border region; there is no active zone outside the visible window.
5. **R5 — Popup integrity.** Combobox and menu popups render fully visible and fully
   clickable; no popup region falls into a dead zone.
6. **R6 — Native shadow where supported.** On platforms/WMs with a mechanism for
   compositor-drawn shadows around undecorated windows (at minimum: Windows 10), the
   window shows an OS-drawn shadow. Detection is automatic; no user configuration.
7. **R7 — Graceful degradation.** On systems with no such mechanism (Wayland, and X11
   WMs/desktops without one), the window shows a thin contrasting border and no shadow.
8. **R8 — Setting removal.** The "CSD shadow" toggle disappears from Settings; a stale
   stored preference is ignored harmlessly.
9. **R9 — No regression elsewhere.** Windows 11 and macOS windows, and the non-CSD
   (native-decoration) mode, look and behave exactly as before.

## Acceptance Criteria

- [x] **AC1** — On Windows 10 and Linux, open every dialog reachable from the main window
  (Settings, About, Donate, Project Editor dialogs, CSV player, examples browser): each
  opens sized to its content with no blank margin bands and nothing clipped (R1, R2).
- [x] **AC2** — Drag the main window and a dialog against all four screen edges on
  Windows 10 and Linux: the visible border touches the edge; OS snap/aero-snap gestures
  still work (R3).
- [x] **AC3** — Hover every edge and corner of a CSD window: the resize cursor appears
  exactly at the visible border and resizing works from it; the cursor does not change
  outside the visible window (R4).
- [x] **AC4** — Open long comboboxes near the bottom/right window border (e.g. driver
  pickers in Setup, theme picker in Settings): every item renders and is clickable (R5).
- [x] **AC5** — On Windows 10, the window has a DWM-drawn shadow visually consistent with
  native apps; on KDE/KWin X11 the WM draws the shadow if the mechanism ships, otherwise
  the thin-border fallback appears (R6, R7).
- [x] **AC6** — On GNOME Wayland and KDE Wayland, windows show the thin-border fallback,
  move/resize/snap correctly, and popups behave (R5, R7).
- [x] **AC7** — Settings no longer shows the shadow toggle; launching with an existing
  `settings.ini` containing the old key neither crashes nor changes behavior (R8).
- [x] **AC8** — Windows 11 and macOS smoke pass: decoration behavior, caption colors, and
  quit/minimize/maximize identical to current release (R9).
- [x] **AC9** — `pytest tests/integration/ -v` passes against the running app on at least
  one CSD platform, confirming no API/window-manager interaction regressions.

## Constraints & Invariants

- No new third-party dependency; platform detection must use what Qt and the OS already
  provide.
- The custom CSD titlebar remains the drag/window-controls surface on Windows 10 and
  Linux — its height, colors, and behavior are unchanged.
- Runtime platform/WM detection must be safe on unusual environments (bare X11 WMs, SSH
  X-forwarding, VMs): unknown environments take the degradation path, never a crash or an
  invisible window.
- Existing windows created at runtime (external widget windows, dialogs) get the same
  treatment as the main window — no second geometry model.
- Startup must not regress: detection happens without blocking window presentation.
- Persisted window geometry from previous versions (saved with shadow-inflated sizes)
  must restore to something sane, not grow/shrink by the old margin on every launch.

## Open Questions

- None — fallback look (thin border, no shadow), settings-toggle removal, and the
  no-shadow Wayland stance were decided with the maintainer on 2026-07-07.
