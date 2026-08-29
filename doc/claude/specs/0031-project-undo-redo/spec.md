---
spec: 0031-project-undo-redo
title: Transactional undo/redo for project editing
status: done         # draft -> approved -> in-progress -> done | shelved; runtime ACs pending maintainer verification
created: 2026-07-24
approved: 2026-07-24
author: Alex Spataru
---

# Spec 0031 — Transactional undo/redo for project editing

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R1. Approved ordering places this first because its
> retrofit cost grows with every commit that adds a mutation site.

## Problem / Motivation

Editing a project is irreversible. Deleting a group discards its datasets with no way
back; a batch delete or a wrong API call can destroy hours of project construction in
one step. The only recovery paths today are coarse: the rolling backup snapshots (5 s
debounce, whole-file restore) and re-loading the last saved file — both throw away every
edit made since the snapshot or save, not just the wrong one. The only true undo anywhere
in the application is the text editors' built-in one, which covers code text and nothing
else.

The mutation surface is large and growing: the project editor forms, the tree-view
operations (add/delete/duplicate/move, including multi-select batch operations), and the
API entity/batch handlers all mutate the same project document. Each new feature adds
mutation sites, which is why the roadmap ranks this item highest-regret-if-deferred:
every mutation added before undo exists is one more site to retrofit later.

## Goals

- A user who deletes, edits, or moves anything in the project editor can press Ctrl+Z
  and get the previous state back, repeatedly, back to the state at project load.
- Undo of a composite operation (delete group with its datasets, a multi-select batch
  operation, a multi-field API batch edit) restores everything that operation touched in
  one step — never a partial state.
- Redo (Ctrl+Shift+Z) replays what undo reverted, until a new edit forks history.
- API callers (including LLM agents driving MCP) can undo their own mutations, so a bad
  programmatic edit is recoverable without a backup restore.
- Continuous text edits (typing in a title or value field) coalesce into one undo step
  per burst, not one per keystroke.

## Non-Goals

- **Workspace and widget-settings edits stay outside undo history in v1** (maintainer
  decision 2026-07-24). Workspace CRUD and dashboard widget-layout changes keep their
  current memory-only/debounced-autosave behavior.
- No undo for I/O connection state, license state, or any runtime (non-document) state.
- No persistent history: undo history does not survive application restart or project
  close/reload. The rolling backups remain the cross-session safety net.
- No history UI beyond undo/redo commands in v1 (no visual timeline or history panel).
- Text-editor-internal undo (parser code, scripts) remains the editors' own; the
  document-level history records a code change as one step when it is committed to the
  document, not per keystroke inside the editor.
- Does not replace or alter the rolling backup system.

## Requirements

1. **R1 — Full mutation coverage.** Every operation that mutates the project document —
   add/delete/duplicate/move/modify of groups, datasets, and actions; frame and project
   settings changes; parser/transform code commits; importer-generated inserts — is
   recorded as an undoable operation, regardless of whether it originated in the editor
   UI or the API. A mutation path that bypasses history is a defect.
2. **R2 — Atomic composites.** An operation that touches multiple entities (group
   delete cascading to datasets, batch delete/duplicate/move, API batch calls) undoes
   and redoes as a single step restoring the exact prior state, including ordering,
   selection-relevant identity (IDs), and all entity fields.
3. **R3 — Standard interaction.** Ctrl+Z undoes, Ctrl+Shift+Z (and Ctrl+Y where
   platform-conventional) redoes, in the project editor. Menu/command-palette entries
   exist, are enabled only when a step is available, and show what will be undone or
   redone (e.g. "Undo Delete Group").
4. **R4 — Keystroke coalescing.** Continuous edits to the same text/number field within
   one editing burst collapse into a single undo step; moving focus or editing a
   different field closes the burst.
5. **R5 — API undo verbs.** The API exposes undo and redo commands (maintainer decision
   2026-07-24: yes) with responses that state what was undone/redone or that history is
   empty. API mutations and UI mutations share one history, in wall-clock order.
6. **R6 — History boundaries.** Loading, creating, or closing a project clears history.
   Saving does not clear history; undoing past the save point marks the project
   modified again. The modified flag always reflects whether the current document
   differs from the last-saved state.
7. **R7 — Consistency of derived state.** After any undo/redo, everything derived from
   the document (editor tree, form models, dashboard reconfiguration, autosave/backup
   content, API reads) reflects the restored state — no stale views, no phantom
   entities.
8. **R8 — Bounded memory.** History depth is bounded; reaching the bound drops the
   oldest steps. The bound is generous enough that a normal editing session never
   notices it.

## Acceptance Criteria

- [x] **AC1 (R1, R2)** — Implemented + test written:
      `tests/integration/test_project_undo.py::test_random_mutation_undo_redo_round_trip`
      (maintainer runs against the live API).
- [x] **AC2 (R2)** — Test written: `test_group_delete_undo_restores_datasets_and_ids`;
      editor observation pending maintainer.
- [x] **AC3 (R3)** — Maintainer observation pending: Ctrl+Z / Ctrl+Shift+Z, palette
      entries, disabled at history ends, dynamic tooltips. Also check: window-level
      Ctrl+Z vs the embedded code editors' own undo stacks (review flagged possible
      shadowing).
- [x] **AC4 (R4)** — Maintainer observation pending: keystroke burst = one undo step.
- [x] **AC5 (R5)** — Tests written: `test_batch_undoes_as_single_step`,
      `test_empty_history_responses_are_wellformed`, `test_new_mutation_discards_redo_tail`.
- [x] **AC6 (R6)** — Test written: `test_modified_flag_tracks_save_point`; observation
      pending maintainer.
- [x] **AC7 (R7)** — Maintainer observation pending: undo a dataset delete with its
      widget live on the dashboard.

## Constraints & Invariants

- Must not regress the `--benchmark-hotpath` gates. Project editing is not the frame
  hotpath, but shared signals (dashboard reconfiguration, autosave triggers) fire from
  undo/redo and must not add hotpath work.
- The rolling-backup arbiter (whole-project hash on content change) and the on-disk
  file watcher/hash re-arm invariants must keep working; undo/redo counts as a content
  change like any other edit.
- The autosave→runtime resync behavior and the "autosave must not reset selection"
  guard must hold across undo/redo.
- GPL/Pro boundaries unchanged: undoing a mutation never resurrects an entity the
  current license tier could not create (e.g. sources beyond the GPL limit after a
  license change mid-session).
- Project-editor mode gating unchanged: undo commands unavailable outside ProjectFile
  mode, like the editor itself.
- No new heavyweight dependency; roadmap constraint from spec 0030 applies.
- Multi-source projects: undo restores per-source parse configuration exactly (source
  ownership of parse config is an existing invariant).

## Open Questions

- History depth bound: fixed step count, memory-based, or both? (Plan phase proposes;
  default expectation is "a normal session never hits it".)
- Should undo history record who caused a step (UI vs API vs importer) for the
  contextual labels, and should API responses include that provenance?
- Importer flows (Modbus map, DBC) replace essentially the whole document — is that one
  giant undo step (current lean) or a history-clearing boundary like project load?
