---
spec: 0015-operator-mode-hardening
title: Operator mode hardening — read-only dashboard chrome + deployment search-bar option
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-17
author: Alex Spataru
---

# Spec 0015 — Operator mode hardening

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Operator deployments (shortcuts generated from the New Deployment dialog) are meant to hand
an operator a locked, view-only dashboard: the author designs the project, the operator
watches live data. Today three author-level controls leak through into operator mode:

1. **Dashboard freeze is operator-togglable.** Both the taskbar freeze button and the Start
   menu's "Freeze Dashboard" entry work in operator mode. Freeze is stored in the project
   itself (spec 0007), so an operator flipping it is silently overriding the author's
   authoring intent for the session — an author who ships a frozen kiosk layout can have an
   operator unfreeze it, drag widgets around, and change what the deployment shows.
2. **The current workspace is operator-editable.** The taskbar's edit-workspace button and
   the taskbar-button right-click "Remove from Workspace" action both function in operator
   mode for user workspaces. The Start menu side already suppresses workspace
   rename/hide/delete and "New Workspace…" in operator mode, so the current behavior is
   inconsistent as well as too permissive.
3. **The search bar is not a deployment choice.** Whether the taskbar search field shows in
   an operator deployment is inherited from whatever the machine's saved preference happens
   to be (the author-only Preferences dialog toggle). The deployment author has no way to
   decide "this kiosk has no search bar" when generating the shortcut, even though every
   comparable taskbar knob (mode, pinned buttons) is already a deployment option.

A fourth surface found while auditing sits on the same trust boundary and is in scope
(maintainer decision 2026-07-17): the per-widget caption menu ("Rename Widget…", "Freeze
Title" mode) also mutates the project and is not gated in operator mode.

## Goals

- An operator running a deployment cannot change whether the dashboard is frozen; the
  frozen state that ships in the project file is simply in effect.
- An operator cannot modify the composition or name of any workspace: no edit dialog, no
  removing widgets, no creating/renaming/hiding/deleting workspaces (the last three already
  hold — they must keep holding).
- The deployment author decides at shortcut-generation time whether the operator taskbar
  shows the search field, alongside the existing taskbar mode and pinned-button options.
- Operator-facing chrome stays consistent: controls the operator may not use are absent,
  not present-but-broken.

## Non-Goals

- No change to author-mode behavior: freeze toggles, workspace editing, and the Preferences
  search-bar setting all keep working exactly as today outside operator mode.
- No change to what freeze itself does (spec 0007 semantics, licensing gates, chrome
  hiding) — only to who may toggle it.
- No new persistence: operator sessions already run with non-persistent settings; this spec
  does not add any operator-writable stored state.
- Not a general operator-permissions system (no per-deployment allow-lists for individual
  controls beyond the search-bar option added here).
- Existing operator conveniences stay available: workspace *switching*, auto-layout toggle,
  full screen, external windows, and the pinned-button set chosen by the deployment author.
- Already-generated deployment shortcuts are not migrated or rewritten.

## Requirements

1. **R1** — In operator mode, no freeze toggle is reachable anywhere in the dashboard UI
   (taskbar, Start menu, search results, shortcuts); the dashboard honors the project
   file's frozen state as-is.
2. **R2** — In operator mode, the edit-workspace control is not reachable, and
   right-clicking a taskbar widget button offers no "Remove from Workspace" action.
3. **R3** — In operator mode, no other route creates, renames, hides, unhides, or deletes a
   workspace (the routes that already block this keep blocking it).
4. **R4** — The New Deployment dialog's taskbar options include a search-bar on/off choice,
   defaulting to on (today's effective default). The generated shortcut records the choice.
5. **R5** — A deployment launched with search off never shows the search field and its
   search keyboard shortcut does nothing; with search on, the field and shortcut work
   regardless of the host machine's saved author preference.
6. **R6** — The search-bar choice is meaningful only when the taskbar can appear (shown or
   auto-hide modes); a hidden-taskbar deployment shows no search field either way, and the
   dialog does not present the choice as if it applied.
7. **R7** — In operator mode, the per-widget caption menu offers no project-mutating
   entries: no "Rename Widget…" and no "Freeze Title" mode changes. Non-mutating entries
   (e.g. opening the widget in an external window) remain available.

## Acceptance Criteria

- [x] **AC1** — Maintainer check, operator deployment of a frozen project: no freeze
      control exists in taskbar or Start menu; layout is locked. Same deployment of an
      unfrozen project: layout is live and nothing offers to freeze it. Searching "freeze"
      in the taskbar search returns no freeze action.
- [x] **AC2** — Maintainer check, operator deployment with a user workspace active: the
      edit-workspace button is gone, right-click on a taskbar widget button shows no
      removal action, and the workspace switcher offers switching only (no "New
      Workspace…", no context menu). Author mode on the same machine still offers all of
      these.
- [x] **AC3** — Maintainer check, New Deployment dialog: search-bar option present under
      the taskbar options, default on, and presented as inapplicable when taskbar mode is
      "Hidden". Two generated deployments (search on / search off) behave per R5 on a
      machine whose author preference is the opposite of the deployment's setting.
- [x] **AC4** — Existing deployment shortcuts generated before this change still launch;
      their search bar shows (absence of the flag means "search on"), independent of the
      host machine's saved author preference.
- [x] **AC5** — Maintainer check, operator deployment: a widget's caption menu shows no
      "Rename Widget…" or "Freeze Title" entries but still offers "Open in External
      Window". Author mode shows the full menu.

## Constraints & Invariants

- The project file is the single source of truth for frozen state in operator mode;
  operator sessions must not write project or settings state (existing non-persistent
  operator settings behavior must not regress).
- Spec 0007 freeze invariants stay intact, including "unfreeze must always work" in author
  mode and the license gating direction.
- Operator mode exists only in Pro deployments; nothing here may change GPL-build author
  behavior.
- No new dependency; the per-deployment search choice must live in the shortcut itself, not
  in machine state (constraint on shape, not a design).
- Any new UI strings go through the normal translation flow.

## Open Questions

None — all resolved with the maintainer on 2026-07-17:

- **Q1 (caption menu scope)**: in scope; project-mutating caption-menu entries are blocked
  in operator mode (R7).
- **Q2 (old shortcuts)**: absence of the search-bar flag means "search on" — deterministic
  over inheriting machine state (AC4).
- **Q3 (presentation)**: operator-suppressed controls are hidden entirely, not shown
  disabled, matching how "New Workspace…" already disappears.
