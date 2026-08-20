---
spec: 0026-project-navigation
title: Project navigation overhaul — editor history & workspace switching
status: done          # closed 2026-08-20
created: 2026-07-21
author: Alex Spataru
---

# Spec 0026 — Project navigation overhaul

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio's project editor already lets you move through the whole project — groups,
datasets, folders, workspaces, sources, output widgets, control scripts, tables — almost
like a file manager. But it is missing the two things that make a file manager feel like
one: **you cannot go back to where you just were**, and you cannot drive it with the tools
people already reach for. There is no navigation history of any kind. Once you click from a
dataset into a sibling group, the only way back is to re-find and re-click the original item
in the tree. A mouse with dedicated back/forward buttons (e.g. a Logitech MX Master) does
nothing in the editor, and the Backspace key — the universal "up/back" gesture — is unbound.
Folder operations (add folder, move to folder, reorder up/down) exist only in a right-click
context menu, so they are invisible until discovered.

On the dashboard side, "workspaces" (named groups of widgets) are the app's equivalent of
virtual desktops, but switching between them is abrupt and easy to lose your place in: the
canvas is torn down and rebuilt instantly with no visual continuity, so the user gets no
sense of direction or of where the new workspace sits relative to the old one. The current
switcher is a single drop-down combo; with more than a handful of workspaces it becomes slow
to scan, and there is no fast, spatial way to see them all and jump to one.

The result is a project- and dashboard-navigation experience that is *capable* but not
*fluid*. This spec makes navigation feel immediate, spatial, and intuitive — reachable by
mouse, keyboard, and eye — without stealing screen space or touching the data hotpath.

## Goals

- The project editor remembers where the user has been and lets them step **back** and
  **forward** through previously-selected items, like a browser or file manager.
- Back/forward is reachable three ways: **mouse side buttons**, **Alt+Left / Alt+Right**,
  and **Backspace** (back only), so it matches whatever muscle memory the user brings.
- Navigation and the most common folder operations are **visible and always available** in
  the UI, not hidden in a context menu — and they cost **no additional vertical space**.
- Switching dashboard workspaces has **visual continuity**: a short directional slide that
  shows which way you moved, so the user keeps their spatial bearings.
- The user can **see all workspaces at once and jump to any of them fast**, whether they
  have three workspaces or fifty.
- None of the above changes how data is parsed, or regresses the parse-rate gate.

## Non-Goals

- Not adding a new toolbar row or otherwise increasing the editor's vertical chrome. The
  new controls live inside the existing Project Structure pane caption (unused space today).
- Not changing what a "workspace" is, how workspaces are stored, created, renamed, deleted,
  or how per-workspace widget layouts are saved/restored.
- Not changing the destructive rebuild-on-switch behavior of the dashboard canvas; the slide
  is a presentation layer over the existing switch, not a re-architecture into live
  simultaneous workspace views.
- Not adding back/forward history to the **dashboard** (this spec's history feature is the
  project editor only; the dashboard gets the slide + the switcher).
- Not adding new folder *types* or new move semantics — the folder buttons trigger existing
  behavior, just from a more visible place.
- Not a Pro-gated feature; navigation is core UX available in all builds.

## Requirements

<!-- Numbered, testable, user-observable. Grouped by the three delivery phases. -->

### Phase 1 — Project editor navigation (delivered first)

1. **R1 — Back/forward history.** The editor maintains an ordered history of the items the
   user has visited (selected) in the project tree. "Back" re-selects the previously visited
   item and shows its editor pane; "Forward" re-selects the item stepped back from. Visiting
   a new item after stepping back discards the forward history from that point (standard
   browser semantics).
2. **R2 — History survives tree changes.** Back/forward continues to work correctly after the
   tree is rebuilt (e.g. after adding, deleting, moving, or renaming items). A history entry
   that no longer exists (its item was deleted) is skipped rather than causing an error or a
   blank pane.
3. **R3 — Mouse side buttons.** Pressing the mouse "back" button anywhere over the editor
   navigates back; the mouse "forward" button navigates forward.
4. **R4 — Keyboard.** `Alt+Left` navigates back and `Alt+Right` navigates forward from
   anywhere in the editor. `Backspace` navigates back **only** when the project tree (not a
   text field or other text-editing control) has keyboard focus, so it never interferes with
   typing.
5. **R5 — Visible controls, no added height.** Back and forward controls, plus the folder
   operations Add Folder, Move To Folder, Move Up, and Move Down, appear as buttons inside
   the **Project Structure pane's caption bar** (right-aligned, after the pane title),
   consuming no additional vertical space. The controls use the app's standard icon-button
   styling and theming.
6. **R6 — Correct enablement.** Back is disabled when there is nowhere to go back to; Forward
   is disabled when there is nowhere to go forward to. Each folder-operation button is enabled
   only when it applies to the current tree selection (e.g. Move Up/Down disabled when the
   selection cannot move; Add/Move To Folder disabled when the selection has no folder
   context), and acts on the **current selection** (not on a right-clicked item).
7. **R7 — Tooltips.** Every new control has a descriptive tooltip.

### Phase 2 — Dashboard workspace slide animation

8. **R8 — Directional slide.** Changing the active dashboard workspace plays a brief
   horizontal slide/fade transition. The direction reflects the move: advancing to a
   later-positioned workspace slides one way, returning to an earlier one slides the other.
9. **R9 — Non-destructive presentation.** The slide is purely visual continuity over the
   existing switch; the resulting workspace, its widgets, and its saved layout are exactly
   what they would be without the animation. The animation is short enough not to feel like a
   wait and does not block interaction once complete.
10. **R10 — Applies to all switch paths.** The slide plays regardless of how the switch was
    triggered (switcher control, keyboard cycle/jump, search, start menu), and degrades
    gracefully (no broken visuals) when a frozen snapshot of the outgoing canvas is
    unavailable.

### Phase 3 — Virtual-desktop workspace switcher

11. **R11 — Overview switcher.** A switcher overlay presents all workspaces at once in a
    spatial, scannable layout, and lets the user jump to any of them. It is invoked from a
    dashboard control and from a keyboard shortcut.
12. **R12 — Scales with count.** The switcher remains usable whether there are few or many
    workspaces: it scrolls when they exceed the visible area and offers **type-to-filter** to
    narrow a large set quickly.
13. **R13 — Keyboard-drivable.** Within the switcher, arrow keys move the highlight between
    workspaces and Enter activates the highlighted one; Escape dismisses without changing the
    active workspace.
14. **R14 — Honors existing model.** The switcher reflects the real set of workspaces
    (including their folder grouping and names) and activating one produces exactly the same
    result as the existing switch path.

## Acceptance Criteria

<!-- Phase-1 logic is the part with a mechanical test path; the rest is in-app observation
     plus the parse-rate gate, since these are UI/QML behaviors the app must be running to
     show. -->

- [ ] **AC1 (R1)** — In the running editor: select item A, then B, then C; Back returns to
      B, Back again to A; Forward returns to B. From B, selecting a new item D makes Forward
      unavailable (C is discarded).
- [ ] **AC2 (R2)** — With history A→B→C, delete B, then press Back from C: navigation lands
      on a valid item (A) and never on a blank/errored pane; no crash.
- [ ] **AC3 (R3/R4)** — Mouse back/forward buttons navigate history; `Alt+Left`/`Alt+Right`
      navigate history; `Backspace` navigates back when the tree is focused but does **not**
      when a text field in the editor is focused (typing Backspace in a field deletes a
      character as normal).
- [ ] **AC4 (R5/R6/R7)** — The Project Structure caption shows back/forward + the four folder
      buttons with no increase in editor height; each button's enabled state tracks the
      current selection as specified; each shows a tooltip; folder buttons act on the current
      selection and produce the same result as the equivalent context-menu action.
- [ ] **AC5 (R8/R9/R10)** — Switching workspaces (via each trigger path: switcher, keyboard
      cycle, keyboard jump, search) plays a directional slide whose direction matches the
      index delta; the settled workspace matches the non-animated result; no visual artifact
      when a snapshot cannot be produced.
- [ ] **AC6 (R11–R14)** — The switcher overlay opens from its control and shortcut, shows all
      workspaces, scrolls and type-filters with a large workspace set, is fully keyboard
      navigable (arrows/Enter/Escape), and activating a workspace matches the existing switch
      result.
- [ ] **AC7 (all)** — `--benchmark-hotpath` still passes its gate; the feature adds no work
      to the per-frame parse/publish path (animation snapshot work occurs only on
      user-initiated workspace switches, never per frame).

## Constraints & Invariants

- **No hotpath impact.** Nothing in this feature runs on the data parse/publish path. The
  dashboard snapshot/slide work is triggered only by a user-initiated workspace switch (rare),
  never per frame. The `--benchmark-hotpath` gate must not regress.
- **No added vertical chrome in the editor.** The new editor controls must fit within the
  existing Project Structure pane caption; do not add a toolbar row or otherwise grow the
  editor's vertical layout.
- **History identity must be stable, not index-based.** The project tree is rebuilt (with a
  fresh selection model) on structural change, so navigation history cannot rely on transient
  row indices; it must resolve to items in a way that survives rebuilds and tolerates deleted
  items (R2).
- **Reuse existing selection and folder operations.** Back/forward re-selection and the
  folder buttons must funnel through the editor's existing item-selection and
  folder-operation entry points, so behavior stays identical to today's tree/context-menu
  paths — no parallel, divergent logic.
- **Dashboard switch semantics unchanged.** The workspace slide must not alter the outcome of
  a switch, must not fight the project reload/reconcile path, and must behave correctly for
  independent (external-window) dashboards that track their own active workspace.
- **Works in all modes/builds.** Editor navigation must work in both QuickPlot and ProjectFile
  project modes and is not gated behind a Pro/commercial build.
- **Follow repo UI conventions.** New icons registered individually in the resource file (no
  globbing); standard themed icon-button component; tooltips via the standard attached
  mechanism; user-facing strings translatable.

## Open Questions

- **OQ1 — Backspace scope precision.** R4 scopes Backspace-as-back to "tree focused, not a
  text field." Is tree-focus the exact desired trigger surface, or should Backspace also work
  when focus is on a non-text control within the right-hand editor pane (e.g. a combo/toggle)?
  Recommendation: start with tree-focus only (safest, zero typing conflict) and widen later
  if it feels too narrow.
- **OQ2 — Does history record the folder/workspace tree positions too, or only item
  selections?** The tree contains navigable containers (folders, workspaces) as well as leaf
  items. Recommendation: record every selection change the user makes in the tree (containers
  included), since each already drives a pane change — treat "visited" == "was the current
  selection."
- **OQ3 — Phase 3 invocation shortcut.** Which keyboard shortcut opens the virtual-desktop
  switcher? The dashboard already uses PgUp/PgDown (cycle) and Ctrl+1..9 (jump). Recommendation
  to be settled in `/ss-plan` against the existing shortcut map to avoid collisions.
- **OQ4 — Should the folder-operation buttons in the caption be the full set (Add Folder,
  Move To Folder, Move Up, Move Down) for every selection kind, or a kind-appropriate subset
  that changes with the selection?** Recommendation: show the full set always, enable/disable
  per selection kind (predictable button positions beat appearing/disappearing buttons).
