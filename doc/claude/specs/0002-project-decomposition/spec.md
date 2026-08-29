---
spec: 0002-project-decomposition
title: God-class decomposition (ProjectModel / ProjectEditor / ProjectHandler)
status: done          # draft -> approved -> in-progress -> done | shelved
created: 2026-07-06
author: Alex Spataru
---

# Spec 0002 - God-class decomposition (ProjectModel / ProjectEditor / ProjectHandler)

> **Phase 1 of 4 - the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Three translation units carry the entire project subsystem and each has grown into a god
class:

- `ProjectModel.cpp` - 8297 lines, ~12 distinct responsibility clusters (persistence,
  loading + legacy migration, source CRUD, entity CRUD, workspace synthesis, folder CRUD,
  tables/registers, selection state, disk-watch guard, autosave, diagram invokables,
  status/lock).
- `ProjectEditor.cpp` - 6677 lines, ~8 clusters (external wiring, tree model, Mqtt form,
  entity forms, commit handlers, multi-selection, selection mirrors, summaries).
- `ProjectHandler.cpp` - 6534 lines, N command-family clusters (file/snapshot, entities,
  parser/painter/dry-run, batch), plus ~60 file-scope static helpers.

Each file fuses many responsibilities into one compile unit. That inflates incremental
rebuild cost (any edit recompiles the whole unit), makes the code hard to navigate and
review, and lets unrelated concerns share file-scope state and static helpers with no
enforced boundary. The classes themselves are large public facades that QML and the API
layer bind against; the size problem is the *file*, not (yet) the *interface*. The first,
cheapest, zero-risk win is to split the physical translation units without changing a single
line of behavior - then, later and separately, extract genuine collaborators behind the
unchanged facade.

## Goals

- **Physical TU decomposition tonight (stages S1-S3).** Split each of the three `.cpp`
  files into per-responsibility translation units under a new `Project/` subdirectory, moving
  whole function bodies verbatim. Zero behavior change, zero interface change. Each stage, on
  its own, is a valid, buildable morning state.
- **Behavioral collaborator extraction later (stages S4-S5, spec-only tonight).** Once the
  files are split, carve out real collaborator objects (disk-watch guard, autosave
  controller, workspace synthesizer, UI-state store, combobox catalog, tree controller,
  per-entity form controllers, multi-selection controller) that own their state behind the
  same facade. This is design-captured here but *not* implemented in the overnight run.
- **Every public facade slot, property, signal, and Q_ENUM survives byte-identical** so that
  QML and the API layer require no edits.
- **Rebuild-cost and navigability improve** measurably: editing one concern recompiles one
  small TU, not the whole god file.

## Non-Goals

- **No facade API change.** The two facade headers (`ProjectModel.h`, `ProjectEditor.h`)
  and the handler registration surface stay byte-compatible. QML binds ~60 distinct
  `Cpp_JSON_ProjectModel.*` slot names and references `ProjectEditor.*` Q_ENUMs 150+ times;
  none of those bindings may break. This split does not add, rename, remove, or re-signature
  any public member.
- **No re-architecture of registration.** `registerCommands()` stays as-is; the API
  command table is already data-driven and the bulk is irreducible doc-string/schema
  literals. Do not rewrite it as a table.
- **No behavior change of any kind in S1-S3.** No bug fixes, no cleanups, no reordering of
  side effects, no "while I'm here" edits. Pure moves only.
- **No collaborator implementation tonight.** S4/S5 are specified and staged but deferred;
  the overnight run stops at S3.
- **No new dependencies, no new Q_OBJECT** in S1-S3 (the sole exception is moving the
  existing `CustomModel` class to its own header in S2b, which already has `Q_OBJECT`).

## Safety argument (why a byte-identical split is possible)

Two properties of the codebase make a verbatim TU split provably behavior-neutral:

- **moc-neutrality.** None of the three `.cpp` files includes its own `moc_*.cpp`; moc
  output is compiled through automoc's `mocs_compilation.cpp`. The `Q_OBJECT` macro, the
  meta-object surface, all `Q_PROPERTY`/`Q_INVOKABLE`/`signals:`/`slots:` declarations live
  in the *header*, which is not touched. Member-function *definitions* may live in any
  translation unit. Therefore moving whole function bodies from one `.cpp` into sibling
  `.cpp` files under the same class is invisible to moc: the class, its `Q_OBJECT`, and its
  meta-object are unchanged. S1-S3 TUs contain no `Q_OBJECT` of their own and need no moc
  work.
- **Unqualified-lookup trick (zero call-site edits).** Members are defined out-of-line as
  `DataModel::ProjectModel::foo(...)` (and `API::Handlers::...` for the handler). Inside such
  a definition, unqualified name lookup searches the enclosing namespaces. So a shared free
  helper declared in `namespace DataModel` (or `namespace API::Handlers`) in a shared header
  is found from every out-of-line member body *without qualifying a single call site*. This
  lets file-scope `static` helpers that must cross TUs be promoted to `inline` free
  functions in a shared header (templates keep their `template`; non-templates gain `inline`)
  with no edits at the call sites. Every promoted name was audited to be unique repo-wide, so
  promotion cannot introduce a collision. Helpers used by only one TU stay `static` in that
  TU.

The shared-helper header pattern follows the existing precedent
(`app/src/DataModel/Importers/ImporterCommon.h`): inline free functions with `@brief`
comments in a named namespace, never an anonymous namespace (the linter's
`cxx-anonymous-namespace` rule).

## Requirements

1. **R1 - Definition symmetry.** For each split class, the total count of out-of-line member
   definitions after the split equals the count before. No definition is lost, duplicated, or
   added.
2. **R2 - Byte-identical facade.** `ProjectModel.h` and `ProjectEditor.h` are unchanged (the
   sole header change is S2/S2b: moving the private item-id enum block and the `CustomModel`
   class out to new headers, which is itself a pure move with unchanged declarations). Every
   public slot name, property, signal, and Q_ENUM the QML/API layer binds against survives.
3. **R3 - QML contract preserved.** The ~60 `Cpp_JSON_ProjectModel.*` slot names and the
   150+ `ProjectEditor.*` enum references continue to resolve to the same symbols with the
   same behavior.
4. **R4 - Pure move.** The concatenation of every moved hunk equals the concatenation of
   every deleted hunk, modulo the `static` -> `inline` promotion on the audited shared-header
   helpers. No function body is altered.
5. **R5 - Build integrity.** Each new `.cpp` is registered exactly once in the CMake
   `SOURCES` list and each new `Q_OBJECT` header (S2b `CustomModel.h`) exactly once in
   `HEADERS`; `BUILD_COMMERCIAL` regions stay inline behind `#ifdef` and remain balanced per
   TU.
6. **R6 - Cannot-move invariants intact.** Every item on the CANNOT-MOVE list (below) stays
   in its facade TU or keeps its connection topology unchanged.
7. **R7 - Stage independence.** S1, S2, S3 are mutually independent; each stage that lands is
   a self-consistent buildable state. The overnight run may complete any prefix (S1, or
   S1+S2, or S1+S2+S3) and leave a coherent tree.
8. **R8 - Collaborator extraction (S4/S5) is specified, not implemented tonight.** Each S4/S5
   sub-stage names its owned state, its interface, and its signal-chaining strategy so that
   the facade's existing NOTIFY signal names are re-emitted with zero QML changes.

## Acceptance Criteria

- [x] **AC1 (R1, R4)** - For each class, `grep -cE "DataModel::ProjectModel::"` (and the
  editor / handler equivalents) summed across all new TUs equals the original count; each
  header-declared member is defined in exactly one family file. A pure-move diff check shows
  the concatenated moved hunks equal the deleted hunks modulo `static`->`inline` on the
  promoted shared helpers.
- [x] **AC2 (R2, R3)** - `ProjectModel.h` / `ProjectEditor.h` public surface is unchanged
  (diff limited to the extracted `ItemIds` / `CustomModel` declarations in S2). All ~60
  facade slot names and 150+ enum references still resolve. QML files are untouched.
- [x] **AC3 (R5)** - Each new `.cpp` appears exactly once in `app/CMakeLists.txt` `SOURCES`;
  `CustomModel.h` appears exactly once in `HEADERS`; `#ifdef BUILD_COMMERCIAL` open/close
  count balances per TU.
- [x] **AC4 (static/ODR closure)** - Every promoted shared helper is defined exactly once
  (inline in the shared header); every remaining `static`/`template` helper resolves within
  its own TU or the shared header; no `detail::` type (`ThreeAxisLayout`, `RefAnchor`) is
  duplicated across TUs.
- [x] **AC5 (build + run, maintainer)** - The Pro and non-Pro builds compile with no new
  warnings, and the running app exhibits identical project-editor, persistence, and
  API-handler behavior (the maintainer builds and exercises it; this is the only check the
  agent cannot run).
- [x] **AC6 (S4/S5)** - Deferred. Each collaborator sub-stage has written acceptance criteria
  in `tasks.md`; none is checked until that sub-stage is implemented in a later run.

## Constraints & Invariants

**CANNOT MOVE (verbatim from the validated design - these stay in their facade TU or keep
their topology unchanged):**

1. Both facade headers byte-compatible (all Q_PROPERTY/Q_INVOKABLE/signals/slots).
2. ProjectEditor Q_ENUMs (CurrentView/EditorWidget/CustomRoles/ItemKind) - QML enum home.
3. ProjectModel ctor wiring :358-468 (fenced ordering invariant).
4. autoSave()/syncRuntime()/m_runtimeDirty (race area).
5. setupExternalConnections() (cross-singleton wiring).
6. m_selected* mirrors in both classes (cyclic PM<->PE callbacks :5472,:6003,:6258,:6492,
   :6717).
7. SaveBlocker (private enum).
8. wireProjectModelRebuilds() connect topology (Queued contracts + title-edit rule; bodies
   may change TU, not shape).
9. onDataset*/onGroup* handlers movable only with ItemIds header.

**Further invariants the split must preserve:**

- The ctor at `ProjectModel.cpp:358-468` wires `bumpEpoch`, `markDirty` ->
  `scheduleAutoSave`, and calls `newJsonFile()` *before* the `groupsChanged` ->
  auto-workspace-regen lambda, behind a reviewed `// code-verify off` fence (:445-450). The
  ordering is load-bearing and must not be reordered or moved.
- `groupsChanged` -> `buildTreeModel` is a `QueuedConnection` (`ProjectEditor.cpp:285-289`);
  the connection type must not change.
- Watcher invariant: `writeProjectFile()` / `loadFromJsonDocument()` / `newJsonFile()` must
  re-arm via `watchProjectFile()` (`ProjectModel.cpp:7735`). Splitting these across TUs must
  keep the re-arm call intact.
- The `detail::ThreeAxisLayout` and `detail::RefAnchor` types must live in exactly one TU
  (Crud) to avoid an ODR violation.
- No `tr(` in any shared header (would drag in translation context where none belongs).

## Open Questions

- **ProjectSerializer/Loader collaborator (S4 tail).** Extracting the ~1,100-line
  serialize/load pair likely needs a `ProjectDocument` aggregate to carry state across the
  boundary. A `friend class` is the cheaper interim. Decide the aggregate-vs-friend question
  before scheduling that sub-stage - it does not block S1-S3 or S4a-e.
- **S3 stop-rule outcome.** ProjectHandler has ~60 file-scope statics; 14 are verified
  cross-family and move to `ProjectApiSupport`. If any *other* static turns out to have call
  sites spanning two families, its family is left unsplit (partial split is acceptable). The
  exact set of families that end up split is only known after the closure check runs.
