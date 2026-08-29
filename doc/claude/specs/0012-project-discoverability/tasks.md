---
spec: 0012-project-discoverability
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-16
---

# Tasks 0012 — LLM discoverability primitives for large projects

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — Shared windowing helper

- **Files:** `app/src/API/Handlers/ProjectApiSupport.h`
- **Does:** Adds `applyWindow(total, offset, limit)` → `{start, count, nextOffset?}` helper
  mirroring the `qBound` idiom from `buildSnapshotGroups` (`ProjectHandlerFile.cpp:394-407`),
  for reuse by the three paged lists, `projectSearch`, and `groupGet`. Header-only, inline,
  `[[nodiscard]]`, no behavior change anywhere yet.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Handlers/ProjectApiSupport.h`
- **Deps:** none
- [x] done

### T2 — Page `project.group.list` + `project.dataset.list`

- **Files:** `app/src/API/Handlers/ProjectHandlerBatch.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** `groupsList`/`datasetsList` read optional `offset`/`limit` via T1, emit
  `nextOffset`, whole-project totals, `attachProjectEpoch`, and a `window:{offset,count,total}`
  self-identification block (R4/R9); registration schemas gain the optional params + updated
  descriptions. **Binding invariant: wire back-compat — a call without `limit` returns the full
  list exactly as today (the bounded default lives in T10's assistant shim, not here).**
- **Verify:** code-verify on both files; read-back that the no-args path is structurally
  unchanged (same keys as before plus the new epoch/window fields).
- **Deps:** T1
- [x] done

### T3 — Page `project.dataset.getExecutionOrder`

- **Files:** `app/src/API/Handlers/ProjectHandlerBatch.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** Same treatment as T2 for `datasetGetExecutionOrder` (schema was `empty` — becomes
  optional offset/limit). Same back-compat invariant: no `limit` → full order.
- **Verify:** code-verify on both files.
- **Deps:** T1 (T2 for idiom consistency)
- [x] done

### T4 — `projectSearch` one-walk collector (new TU)

- **Files:** `app/src/API/Handlers/ProjectHandlerDiscovery.cpp` (new),
  `app/CMakeLists.txt` (add TU near line 262)
- **Does:** New per-concern TU with `ProjectHandler::projectSearch`: single pass in the fixed
  order sources → groups/datasets → actions → workspaces (`activeWorkspaces()`) → tables
  (`tables()`, identified by `name`); case-insensitive substring on title (+alias/units for
  datasets, with `matchedField` only on non-title hits); `type`/`groupId`/`sourceId` filters;
  empty/whitespace query → `InvalidParam` with list-command hint; T1 windowing (default 20,
  cap 100), `matchCount`, `nextOffset`, `_summary`, `attachProjectEpoch`, drill-in `_hint`
  (R7). **Binding invariants: read-only const accessors, deterministic walk order (paging
  coherence), compact rows only — never `buildDatasetObject`; `Keys::` constants for any key
  that exists in `Keys::`, no hardcoded duplicates.**
- **Verify:** code-verify on the new TU; read-back against R1/R2 field lists.
- **Deps:** T1
- [x] done

### T5 — `groupGet` dual-id-space read

- **Files:** `app/src/API/Handlers/ProjectHandlerDiscovery.cpp`
- **Does:** `ProjectHandler::groupGet`: mutually exclusive `groupId` (positional) XOR
  `uniqueId` (stable) selectors — both/neither/unknown → `InvalidParam` whose hint explains
  the two id spaces and that large workspace-ref "groupId" values are uniqueIds (R3/D5);
  response carries both ids + compact `datasetSummary` (title/uniqueId/index/units) windowed
  via T1 (default 50, cap 200).
- **Verify:** code-verify on the TU.
- **Deps:** T4 (shares TU scaffolding/helpers)
- [x] done

### T6 — Declare + register the discovery commands

- **Files:** `app/src/API/Handlers/ProjectHandler.h`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** Declares `projectSearch`/`groupGet` + `registerDiscoveryCommands()` in the header
  (Christmas-tree order, `[[nodiscard]]` n/a — handlers return `CommandResponse`); registers
  `project.search` / `project.group.get` with full `SchemaBuilder` schemas and descriptions
  that cross-reference the exact-resolvers. **Binding invariant: registration stays in
  `ProjectHandler.cpp` per the spec-0002 split — the new TU holds impls only.**
- **Verify:** code-verify on both; grep confirms both names registered exactly once.
- **Deps:** T4, T5
- [x] done

### T7 — Safety classification

- **Files:** `app/rcc/ai/command_safety.json`
- **Does:** Adds `project.search` and `project.group.get` to the `"safe"` tier (read-only,
  auto-execute — R7). **Binding invariant: every registered command sits in exactly one tier;
  unlisted falls through to Confirm (registration bug).** `meta.search` deliberately absent
  (meta tools never enter the safety registry).
- **Verify:** `python -c "import json;json.load(open('app/rcc/ai/command_safety.json'))"`;
  grep both names appear once, in `safe` only.
- **Deps:** T6
- [x] done

### T8 — `ToolDispatcher::searchCommands`

- **Files:** `app/src/AI/ToolDispatcher.h`, `app/src/AI/ToolDispatcher.cpp`
- **Does:** New `searchCommands(query, offset, limit)` beside `listCommands()`: merges
  `assistantToolDefs()` + `fsToolDefs()` + `metaToolRoster()` + non-Blocked
  `API::CommandRegistry::commands()`, case-insensitive substring on name+description,
  **sorts matches by name (the registry is a QHash — unsorted paging is incoherent)**,
  windows (default 25, cap 100), rows `{name, family, snippet≤120}` + total/`nextOffset`.
  Adds `meta.search` to `metaToolRoster()`. **Binding invariant: the roster ↔ makeMetaTool
  sync contract stated in the roster's doc comment — T9 must land before this ships.**
- **Verify:** code-verify on both files.
- **Deps:** none
- [x] done

### T9 — Wire `meta.search` into the conversation

- **Files:** `app/src/AI/Conversation.cpp`
- **Does:** `dispatchMetaTool` route → new `runMetaSearch` (calls
  `m_dispatcher->searchCommands`); `makeMetaTool` entry in `appendCommandMetaTools` whose
  description disambiguates `meta.search` (commands) vs `meta.searchDocs` (documentation) and
  points at `meta.describeCommand` for drill-in. **Binding invariant: meta dual-wiring —
  roster entry (T8) and makeMetaTool entry must both exist or `meta.listCommands` and the
  provider tool list diverge.**
- **Verify:** code-verify; grep `meta.search` appears in roster, dispatch, and makeMetaTool.
  (Routed inline in dispatchMetaTool per the meta.fetchHelp precedent -- avoids a
  Conversation.h edit outside the plan's file list.)
- **Deps:** T8
- [x] done

### T10 — Assistant default-`limit` shim

- **Files:** `app/src/AI/ToolDispatcher.cpp`
- **Does:** In `executeCommand`, after safety gating and before `apiRegistry.execute`, inject
  the default `limit` when the call is `project.dataset.list` (8), `project.group.list` (4),
  or `project.dataset.getExecutionOrder` (50) and `args` lacks `limit` (R4-as-scoped/R5).
  **Binding invariants: runs only after the safety gate (never shortcuts it); touches only
  these three names; never overwrites a caller-supplied `limit`; TCP/SDK callers are
  unaffected because they never pass through ToolDispatcher.**
- **Verify:** code-verify; read-back of the gate ordering.
- **Deps:** T2, T3
- [x] done

### T11 — R9 wording + never-elide additions

- **Files:** `app/src/AI/Conversation.cpp`
- **Does:** First grep `tests/` for pinned strings (`elided`, `exceeded the`); then (a)
  `makeTruncatedResult` notice names paging/filter recovery ("narrow with offset/limit/query;
  see meta.describeCommand"), (b) `elideAgedToolResult` stub states the original call
  *succeeded* and was collapsed to save transcript space — re-issue only if the data is needed
  again, (c) add `project.search`, `project.group.get`, `meta.search` to the
  `isDiscovery`/`isFsContent` exemption at `ageHistoryToolResults` (~1601-1615). **Binding
  invariant: never-elided must mean bounded — all three results are in-budget by construction
  (R5); do not add any unbounded-result tool.**
- **Verify:** code-verify; grep confirms the three names in the exemption; the three notices
  use distinct wording (AC9 static half lands in T15).
- **Deps:** T6, T9
- [x] done

### T12 — Identity docs: the two group id spaces

- **Files:** `doc/help/Identity-Model.md`
- **Does:** Adds the two-id-spaces explanation (positional `groupId` vs sparse persisted group
  `uniqueId`) and the explicit collision warning: workspace widget refs serialize the group
  `uniqueId` under the JSON key `groupId`; a large "groupId" from workspace data must go to
  `project.group.get{uniqueId}` — never to positional-id commands (R6/D5).
- **Verify:** `python scripts/documentation-verify.py`; claim-by-claim match against
  `Frame.h:465-466` and `WorkspacesHandler.cpp:133`.
- **Deps:** T6 (references the shipped command)
- [x] done

### T13 — API reference: new commands + paging params

- **Files:** `doc/help/API-Reference.md`
- **Does:** Documents `project.search` and `project.group.get` (params, row shapes, errors) in
  the project.* section and adds the new optional `offset`/`limit` params to the three list
  entries; updates any stated command counts.
- **Verify:** `python scripts/documentation-verify.py`; names/params match T6 registration
  strings exactly (AI-corpus-drift memory: exact-name discipline).
- **Deps:** T6
- [x] done

### T14 — AI skills: discovery flow + id-space warnings

- **Files:** `app/rcc/ai/skills/api_semantics.md`, `app/rcc/ai/skills/dashboard_layout.md`,
  `app/rcc/ai/skills/tool_discovery.md`
- **Does:** `api_semantics`: R6 warning + "discover with project.search, drill with group.get /
  getByUniqueId" flow; `dashboard_layout`: strengthen the existing `:330` warning with the
  `project.group.get{uniqueId}` escape hatch; `tool_discovery`: insert `meta.search` as the
  first step of the search → describe → execute ladder.
- **Verify:** `python scripts/documentation-verify.py`; command names grep-match registration.
- **Deps:** T6, T9
- [x] done

### T15 — Static-surface test additions (AC7/AC9)

- **Files:** `tests/scripts/test_ai_assistant_static.py`
- **Does:** Asserts `project.search`/`project.group.get` sit in the `safe` tier; the three
  never-elide names appear in `Conversation.cpp`'s exemption; the truncation/elision notices
  carry their distinct R9 wording; `meta.search` present in roster + makeMetaTool + dispatch
  (string-level, matching the file's existing style).
- **Verify:** `pytest tests/scripts/test_ai_assistant_static.py -v` (runnable without the
  app).
- **Deps:** T7, T11
- [x] done

### T16 — Integration tests (AC1–AC5, AC8)

- **Files:** `tests/integration/test_project_discovery.py` (new)
- **Does:** Fixture builds 87 groups / 570 datasets via `project.dataset.addMany` /
  `project.batch`; tests per the plan's mapping: `test_search_basic` (AC1),
  `test_search_filters_and_types` (AC2), `test_group_get_both_id_spaces` (AC3),
  `test_list_paging` (AC4, incl. identical-repeat-page determinism),
  `test_default_calls_fit_budget` (AC5 — serialized compact size < 3.8 KB with the shim's
  defaults passed explicitly, **the binding Q2 measurement**; if it fails, adjust T4/T10
  defaults, not the test), plus a `meta.search`-reachability note (AC8's runnable half lives
  in T15). Follows `tests/README.md` conventions (markers, `tests/utils/api_client.py`).
- **Verify:** I cannot run these (live app required) — verify by read-back against
  tests/README.md conventions; maintainer runs
  `pytest tests/integration/test_project_discovery.py -v`. (10 tests collect cleanly.)
- **Deps:** T2, T3, T6
- [x] done

### T17 — Pipeline + self-review

- **Files:** (generated) `app/rcc/api/*`, `app/rcc/ai/search_index.json`
- **Does:** `python scripts/code-verify.py --check` across every touched file;
  `documentation-verify.py`; `python scripts/sanitize-commit.py` (regenerates SDK + search
  index — expected churn, pre-declared in the plan); `qt-cpp-review` on the C++ diff;
  counterfactual self-review: diff is the plan's file table and only that
  (`WorkspacesHandler.cpp` explicitly untouched).
- **Verify:** clean pipeline output; diff read-back.
- **Deps:** T1–T16
- [x] done (qt-cpp-review ran: 6 agents, 5 confirmed findings — all fixed same-pass: dead
  local, wrong @brief, match-before-dedupe in meta.search, MissingParam convention,
  string-selector coercion; plus fs.search byte caps closing the review's D-301 under the
  original fs-fix mandate. sanitize-commit clean; static tests 8/8.)

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC5/AC9 live
      halves + maintainer observation flagged for the maintainer's run).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched (plan says none) — no benchmark delta expected; CI gate confirms.
- [x] `pytest tests/scripts/test_ai_assistant_static.py` passes locally;
      `tests/integration/test_project_discovery.py` handed to the maintainer with the app-up
      instructions.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep; `WorkspacesHandler.cpp` and
      all foreign files untouched.
- [x] `spec.md` status set to `done`.
