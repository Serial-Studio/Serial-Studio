---
spec: 0117-dataset-dialog-commands
title: Palette and context-menu commands for the dataset dialogs
status: in-progress  # approved 2026-11-09 with OQ1 and OQ2 resolved
created: 2026-11-09
author: Alex Spataru
---

# Spec 0117 — Palette and context-menu commands for the dataset dialogs

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A dataset has three editors of its own: Alarm Bands, Value Labels and Frequency Markers. Each
opens only from a button on the dataset's toolbar, so the dataset has to be the one the
Project Editor is showing first. Someone working in the project tree or the flow diagram
right-clicks a dataset and finds Edit Transform Code there but none of the three. The editor's
command palette lists none of them either. In a project with dozens of datasets, adding bands
or labels to several means a navigate, click, edit, navigate loop for every one.

A context menu also cannot reuse the toolbar's "open for what is selected" behaviour. The
tree selects a right-clicked row only when it is not already part of the selection, so a
right-click inside a multi-selection keeps it. The flow diagram never changes what the editor
shows when its menu opens. In both cases "the selected dataset" is not the one the user
right-clicked, so a menu entry that opened the selection's dialog would edit the wrong dataset.

## Goals

- From the project tree and the flow diagram, a right-click on a dataset offers its Alarm Bands,
  Value Labels and Frequency Markers editors, and each edits that dataset.
- From the Project Editor's command palette, the same three editors open for the dataset the
  editor is showing.
- An entry appears only where its editor applies, by the same rule the toolbar button uses.

## Non-Goals

- No new editor and no change to what the three editors do or store.
- No bulk editing from the menu. Alarm Bands keeps the multi-selection editor it already has,
  reached from the multi-selection view's button; the menu entry edits one dataset.
- The dataset toolbar is unchanged: same buttons, same disabled state, same tooltips.
- No keyboard shortcuts and no "Remove …" entries for the three.
- No API or control-script change: the API already reads and writes bands, labels and
  markers.

## Requirements

1. **R1 — Context menu.** The dataset menu, in the tree and in the flow diagram, gains Edit
   Alarm Bands, Edit Value Labels and Edit Frequency Markers, placed beside Edit Transform Code.
   Each opens its editor for the dataset the menu was opened on.
2. **R2 — The target is the right-clicked dataset.** Opening an editor from the menu never
   depends on what the editor currently shows or selects. The flow diagram keeps showing what
   it showed; the tree keeps its existing right-click selection rule.
3. **R3 — Palette.** The Project Editor's command palette lists the same three commands. Each
   opens its editor for the dataset the editor is showing, and is not listed while the editor
   shows something other than a single dataset.
4. **R4 — One availability rule per editor.** An entry is listed only when its editor applies
   to the target dataset, by the same rule its toolbar button uses today: Alarm Bands where
   bands can be drawn (spec 0093), Frequency Markers where the dataset has an FFT plot or a
   waterfall, Value Labels on any dataset. The rule is evaluated for the target, which in the
   menu is the right-clicked dataset and need not be the selected one.
5. **R5 — Hidden, not dimmed.** Where an editor does not apply, the menu and the palette leave
   the entry out instead of showing it disabled, as the platform guidelines ask of contextual
   menus. The toolbar keeps its disabled button, whose tooltip explains the rule.
6. **R6 — Same commit and undo as the toolbar.** Applying an editor opened from the menu or the
   palette changes only the target dataset, as one undo step, exactly as the toolbar path does.
7. **R7 — Same locks.** The entries follow the Project Editor lock and the dataset's
   editability the way the toolbar buttons do. A locked editor or a read-only dataset opens
   nothing that could change it.
8. **R8 — Translatable and recognisable.** Titles and tooltips are translatable and reach the
   translation catalog. Each entry uses the icon its toolbar button uses. The work lands before
   the next translation update, which the TODO already holds back for it.

## Acceptance Criteria

- [x] **AC1 (R1, R8)** — The command registry check passes with the three new commands: every
  menu entry resolves to a command with a binding and an icon. The translation-strings
  generator lists the three titles and tooltips.
- [ ] **AC2 (R1, R2, R6)** — Maintainer observation in the tree: with dataset A open, right-click
  dataset B and choose Edit Alarm Bands. The editor shows B's bands, Apply changes only B, and one
  Undo restores B; A is untouched. The same holds for Edit Value Labels and Edit Frequency
  Markers.
- [ ] **AC3 (R1, R2)** — Maintainer observation in the flow diagram: with dataset A's view open,
  right-click dataset B's node and choose each entry. The editor edits B and the Project Editor
  still shows A afterwards.
- [ ] **AC4 (R4, R5)** — Maintainer observation, both hosts: a plot-only dataset's menu has no
  Edit Alarm Bands; a dataset with neither an FFT plot nor a waterfall has no Edit Frequency
  Markers; every dataset has Edit Value Labels. A Gauge dataset right-clicked while a plot-only
  dataset is selected does list Edit Alarm Bands.
- [ ] **AC5 (R3)** — Integration over the driving surface (spec 0101), or maintainer
  observation where it cannot open the palette: with a dataset selected, the palette lists the
  applicable entries and running one opens that dataset's editor. With a group selected, none
  is listed.
- [ ] **AC6 (R7)** — Maintainer observation: with the Project Editor locked, no entry opens an
  editor that can change the dataset, matching the toolbar.
- [x] **AC7 (R8)** — The documentation, terminology and claim checks pass. The Project Editor
  manual page names the new menu and palette entries.

## Constraints & Invariants

- **Deciding constraint: a command acts on the dataset it was invoked on.** The menu's target
  is the right-clicked row, carried with the command. It is never read back from the editor's
  selection, which in the tree's multi-selection and in the flow diagram is a different dataset.
- **One availability rule per editor.** The toolbar, the palette and the menu ask the same
  question for each editor. A second copy of the Alarm Bands rule is the orphan-alarm class spec
  0093 closed: an entry offered for a dataset whose bands can never be drawn.
- Commands come from the shared command catalog and its menu and palette surfaces, never from a
  hand-written menu.
- No effect on the acquisition pipeline, the dashboard or a running session; the three editors
  already edit the project only.
- Works the same in the GPL and the commercial build: none of the three editors is Pro-only today.

## Premise Check

- **Question:** Does a right-click already make the clicked dataset the editor's selection? If
  it always did, the existing "open for the selection" behaviour would serve the menu as is,
  and this spec would shrink to three menu lines.
- **Measured:** read on 2026-11-09 from the right-click handling of both menu hosts (the app was
  not running, so no live run). The tree selects the clicked row only when it is not already
  selected, and keeps a multi-selection intact when the click lands inside it. The flow diagram
  never changes the editor's selection when its menu opens. Edit Transform Code already works
  around exactly this by passing the clicked dataset's ids with the command.
- **Verdict:** proceed. In two of the three right-click cases the selection is not the clicked
  dataset, so each editor needs a way to open for a given dataset.

## Open Questions

Resolved by the maintainer on 2026-11-09:

- **OQ1 — Alarm Bands inside a multi-selection:** the menu entry edits the right-clicked dataset
  only, like Edit Transform Code. Bulk editing stays in the multi-selection view.
- **OQ2 — Titles:** the full names, "Edit Alarm Bands", "Edit Value Labels" and "Edit Frequency
  Markers", with no ellipsis, matching Edit Transform Code.
