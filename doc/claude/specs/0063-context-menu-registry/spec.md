---
spec: 0063-context-menu-registry
title: Registry-Driven Context Menus for the Project Editor
status: done          # closed 2026-08-20
created: 2026-08-18
author: Alex Spataru
---

# Spec 0063 — Registry-Driven Context Menus for the Project Editor

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Project Editor has two navigation surfaces over the same project: the **Project Structure
tree** and the **overview flow diagram**. Only the diagram can create anything. Right-clicking a
diagram block offers eleven context-sensitive menus (add group from eleven templates, add dataset
from eight, add output from six, add action / device / table, edit parser, rename, duplicate,
delete). Right-clicking the tree offers one menu that can rename, hide, duplicate, delete, reorder
and file into folders — but cannot create a single object. A user who works in the tree, which is
the surface the editor opens on and the one that shows folders, workspaces and shared tables, must
leave for the diagram or hunt for the right per-view toolbar button to add anything.

Worse, several tree rows are right-click dead zones: the project root, the `Frame Parser` child,
and the four section headers (`Dashboard Widgets`, `Shared Memory`, `Dataset Values`,
`Workspaces`) swallow the click and show nothing, as does the empty area below the tree. Those are
exactly the rows a user aims at to add a group, a folder, a table or a workspace.

Behind both surfaces the menu content is hardcoded three times over. "Add Group" exists as eleven
inline menu entries in the diagram, again as a shared template menu used by two folder views, and
again as toolbar buttons in the groups view; "Add Dataset" and "Add Output" exist twice each. Every
new widget type means editing several QML files, and the copies already differ in wording and
ordering. The rest of the app solved this in spec 0028: a command is declared once as data
(title, icon, tooltip, shortcut, Pro flag, translation) and bound once per context to behavior,
and toolbars, the start menu and the palette are rendered from layout manifests. Context menus
never joined that system, so they are the last hand-written command surface in the product and the
only one where adding an entry is a QML edit rather than a data edit.

## Goals

- Every actionable row in the Project Structure tree has a context menu whose entries are
  relevant to that row, including the rows that are dead zones today, plus the empty background.
- A user can create every project object from the tree: data source, group (all templates),
  dataset (all types), output widget, action, shared table, register, workspace, and folders in
  all three folder families — filed into the container they right-clicked.
- Menu content for both the tree and the overview diagram is declared as data (manifests), not as
  QML: adding, removing, reordering or retitling an entry is a manifest edit plus, for genuinely
  new behavior, one binding entry.
- The two surfaces cannot drift: a widget template or dataset type added once appears in both,
  with identical wording, icons and Pro gating.
- Menu entries obey the same registry guarantees as the rest of the app: registry-resolved icons,
  translated titles, Pro filtering on GPL builds, and script-verifiable manifests.

## Non-Goals

- No change to what the underlying operations do. Menus invoke the project mutators that already
  exist; their behavior, prompts, confirmations and undo semantics are unchanged.
- No conversion of the per-view toolbars (groups, workspaces, tables and folder views keep their
  buttons for now). Only the tree menus and the diagram menus are registry-driven in this spec.
- No new palette entries and no new keyboard shortcuts. Target-scoped commands are menu-only.
- No dialog hoisting. Entries whose dialog currently lives inside a view (alarm bands, frequency
  markers, action icon picker, add-widget-to-workspace) navigate to that view instead of opening
  the dialog; opening them in place is a follow-up.
- No drag-and-drop, no inline rename in the tree, no change to selection or navigation behavior.
- No change to the project file format, and no new persisted state.

## Requirements

1. **R1 — Per-row menus.** Right-clicking a tree row opens a menu specific to that row's kind.
   Covered kinds: project root, data source, frame parser, dashboard-widgets section, group
   folder, group, dataset, output widget, output panel group, action, shared-memory section,
   dataset-values node, table folder, shared table, workspaces section, workspace folder,
   workspace, control loop, and MQTT publisher.
2. **R2 — Live dead zones.** The rows that ignore right-clicks today (project root, frame parser,
   the four section headers) open their menu. Right-clicking empty tree space below the last row
   opens the project-root menu. The blank spacer row remains inert.
3. **R3 — Creation from the tree.** Each menu offers the creation entries that belong to its row:
   the project root offers source, group, action, shared table and workspace, plus the
   project-wide entries (control loop, MQTT publisher, expand and collapse all, seed aliases), and
   no file commands; a source offers group, dataset, output and action scoped to that source; a
   group offers dataset and output; a folder offers its own object type and a sub-folder inside
   itself; each section header offers its object type and a top-level folder.
4. **R4 — Target scoping.** A creation or edit entry acts on the row it was invoked from, not on
   the previously selected item. Adding a dataset from a group's menu puts it in that group;
   adding a group from a folder's menu files it into that folder; adding a group from a source's
   menu attaches it to that source. The new object becomes the selected row.
5. **R5 — Declarative content.** Which entries a menu contains, their order, grouping separators,
   submenus, titles, icons, tooltips and Pro flags are read from manifest data. Adding an entry to
   an existing menu, or reordering one, requires no change to any menu-rendering QML.
6. **R6 — One catalog, two surfaces.** The tree menus and the overview diagram menus resolve their
   entries from the same command catalog. A template or dataset type declared once appears on both
   surfaces with the same title, icon and gating.
7. **R7 — Dynamic sections.** Menus support entries whose content is computed at open time and
   cannot be spelled out in a manifest: the cascading "Move to Folder" folder tree, and the
   multi-select labels that count the selection ("Delete Selected (3)"). These appear at
   manifest-declared positions inside the menu.
8. **R8 — Selection collapse.** Right-clicking a row outside the current multi-selection drops the
   selection to that row; right-clicking inside the selection preserves it. This is today's
   behavior and it does not change.
9. **R9 — Selection-aware entries.** With one row selected, a menu shows its creation and
   single-item entries. With several rows selected, creation entries are absent and the bulk
   entries act on the whole selection, as they do today. Entries that do not apply to the target
   are absent rather than shown disabled — except Pro entries on a GPL build, which stay visible
   and disabled, matching the diagram's current behavior.
10. **R10 — Registry parity.** Menu entries take their icons from the icon registry, their titles
    and tooltips from the translated command catalog, and disappear from GPL builds when flagged
    Pro. Language switching updates open-next-time menu text without a restart.
11. **R11 — No regression of existing entries.** Every entry the tree menu offers today (expand
    and collapse all, move up and down, rename, hide and show, duplicate, delete, seed aliases,
    new folder, new sub-folder, move to folder) remains available on the same rows, with the same
    multi-select behavior. Every entry the diagram menus offer today remains available on the same
    blocks.
12. **R12 — Gaps closed.** Rows whose operations exist in the project model but are unreachable
    from the tree gain them: duplicate and delete for a data source, add-register and CSV
    import/export for a shared table, per-source frame-parser editing, control-loop and MQTT
    entry points.
13. **R13 — Verifiable manifests.** The repo's registry verification script validates the new
    manifests the same way it validates toolbar layouts: every referenced command exists, every
    icon resolves, and every command a menu references is bound in the context that renders it.
14. **R14 — Dataset visualization toggles.** A dataset's menu carries a submenu of checkable
    visualization entries (plot, FFT, waterfall, bar or level, gauge, compass, meter, LED). Each
    reflects whether that visualization is currently enabled on the dataset, and toggling one has
    the same effect as the matching button in the dataset view.

## Acceptance Criteria

- [ ] **AC1** — In a running app with a project loaded, right-clicking each of the nineteen row
      kinds in R1 opens a menu, and no row kind opens the wrong menu. Maintainer observation with
      a project that exercises all kinds (multi-source, folders in all three families, an output
      panel, a shared table, a workspace).
- [ ] **AC2** — Right-clicking the project root, the frame parser child, each of the four section
      headers, and the empty area below the tree opens a menu; right-clicking the blank spacer row
      opens nothing. Maintainer observation.
- [ ] **AC3** — Creating one of each object type from the tree produces the same result as creating
      it from the diagram or the corresponding view toolbar: same defaults, same title, same
      parent. Verified against the project file written after each creation.
- [ ] **AC4** — Target scoping holds: with source B and folder F present, adding a group from
      source B's menu yields a group attached to source B; adding a group from folder F's menu
      yields a group whose parent folder is F; adding a dataset from group G's menu yields a
      dataset in G, regardless of which row was selected beforehand. Verified in the saved project
      file and via the API server's project queries.
- [ ] **AC5** — Adding one entry to an existing menu (a new dataset type is the worked example)
      requires editing only manifest data plus one binding entry, and it appears in both the tree
      menu and the diagram menu. Demonstrated as a diff review at handoff.
- [ ] **AC6** — Selecting three deletable rows and right-clicking shows no creation entries and a
      delete entry labeled with the count 3; deleting removes all three. Maintainer observation.
- [ ] **AC7** — On a GPL build, Pro-flagged entries are visible and disabled, and no menu evaluates
      a Pro symbol; on a commercial build the same entries are enabled. Verified by building the
      GPL variant and by the registry script's commercial-guard scan.
- [ ] **AC8** — `scripts/registry-verify.py`, `scripts/generate-command-strings.py --check` and
      `scripts/code-verify.py --check` pass with the new manifests and menus in place.
- [ ] **AC9** — Every existing tree and diagram entry from R11 is still present and still works,
      checked row by row against the pre-change menus. Maintainer observation, with the enumeration
      recorded in the task list.
- [ ] **AC10** — A dataset with plot enabled and FFT disabled shows the visualization submenu with
      plot checked and FFT unchecked; toggling FFT from the menu enables it in the dataset view and
      in the saved project file, and toggling it back removes it. Maintainer observation plus the
      saved project file.
- [ ] **AC11** — Switching the application language updates menu entry text on the next open, with
      no restart and no untranslated entry. Maintainer observation.

## Constraints & Invariants

- Menus invoke existing project mutators only. No new mutation logic may live in menu code, so
  undo mementos (spec 0031) keep staging and committing exactly as they do now.
- Any creation rebuilds the project tree model, which invalidates the row a menu was opened from.
  A menu must never act on identifiers captured before a rebuild.
- Icons resolve only through the icon registry (spec 0028); no hardcoded resource paths.
- Titles and tooltips must be reachable by the translation extraction the command catalog already
  uses; dynamic per-state strings stay translatable at the binding.
- GPL builds must not evaluate Pro symbols in menu code, per the existing commercial-guard rule.
- This is Project Editor UI only: it must not touch the frame pipeline, the dashboard, or anything
  on the hotpath, and must not add per-frame work of any kind.
- The editor is usable with no project loaded and in every operation mode; menus must degrade to
  the entries that make sense rather than misbehaving.
- No new third-party dependency, and no new persisted user state.

## Open Questions

None — the three questions raised at draft were resolved at approval:

- **Project-root menu scope.** Creation and project-wide entries only; the file commands (new,
  open, save, import protobuf, restore backup) stay on the toolbar, where they already carry icons
  and shortcuts. Folded into R3.
- **Dataset visualization toggles.** In scope, as a checkable submenu — the command catalog already
  models toggles, so no dialog hoisting is implied. Folded into R14 and AC10.
- **Right-click outside the selection.** Keeps today's behavior: collapse to the clicked row, and
  preserve the selection when the click lands inside it. Folded into R8.
