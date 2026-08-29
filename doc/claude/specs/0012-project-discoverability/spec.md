---
spec: 0012-project-discoverability
title: LLM discoverability primitives for large projects
status: done         # draft -> approved -> in-progress -> done | shelved
                     # AC1-AC7 verified (integration 11/11 + static 8/8 + docs). AC8 static
                     # half verified; AC9 static half verified, live-transcript observation
                     # optional/maintainer.
created: 2026-07-16
author: Alex Spataru
---

# Spec 0012 — LLM discoverability primitives for large projects

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

On large projects (500+ datasets across 80+ groups), the in-app AI assistant cannot navigate
the project. There is no middle ground between two extremes:

- **List everything.** `project.dataset.list`, `project.group.list`, and
  `project.dataset.getExecutionOrder` return every item in full. On a 570-dataset project the
  serialized result overflows the per-provider **tool-result byte budget** (provider-dependent,
  default ~4 KB, clamped to 2–16 KB; `fs.*` gets a larger 48 KB budget but `project.*` does
  not). An oversized result is not gracefully shortened — it is replaced wholesale by a
  byte-prefix `preview` string with `truncated:true`, so the assistant receives a mangled
  fragment of the first group's JSON and *none* of the structure it asked for. There is no
  `offset`/`limit` on these commands to page the result under the budget.
- **Resolve one exact item.** `assistant.dataset.resolve`, `project.dataset.getByPath`, and
  `project.dataset.getByTitle` each require an **exact** title or path. To use them the
  assistant must already know the dataset's title — which is precisely what it was trying to
  discover.

This came from a concrete session on a 570-dataset / 87-group pressure-instrumentation project.
The assistant obtained a `groupId` (614, 659–663) from workspace widgets but had no cheap way to
find out which datasets live inside those groups, so it began **guessing dataset titles**
("Channel 1", "PT-1") and burning tool calls, and ultimately had to ask the human to read the
titles out by hand. A second, compounding confusion: it assumed `groupId` was a dense 0..N-1
array index, then hit IDs in the 600s on an 87-group project, because `groupId` is a sparse
per-project counter that is never reused or compacted after deletes/reorders — and nothing in
the assistant-facing docs warns of this. A third friction from the same session: the assistant
runtime ages older tool results in the transcript down to a `[result elided -- ask again if
needed]` stub, and that stub is indistinguishable from the byte-budget truncation notice — so
the assistant wasted round-trips re-issuing identical oversized calls, unable to tell "this
result was too big" apart from "this result was hidden because it aged out".

The framing for the fix (maintainer's words): *think of what a human does with a
10,000-page PDF manual — they use an index and a search function.* The assistant needs the same
class of navigation primitives over a project: a search, a cheap drill-in, and pageable
listings — plus accurate mental-model docs so it stops reasoning from false assumptions.

## Goals

- The assistant can find **any project entity** — dataset, group, action, source, workspace, data
  table — by a **partial** name it partially remembers or infers, without knowing an exact title
  or path.
- The assistant can inspect the contents of **one** group (its dataset titles/ids/units) without
  pulling the entire project.
- Every whole-project listing can be **paged** so a default call returns a usable, structured,
  in-budget result on a 570-dataset project instead of a truncated preview.
- The assistant can **search the API command catalog itself** by substring, the way a human uses
  the index of a manual, instead of only looking a command up by exact name.
- The assistant's documented mental model of identity (`groupId`, `datasetId`, `uniqueId`) matches
  runtime reality, so it stops treating sparse counters as dense indices.
- The assistant can always tell **why** a result is incomplete — too big (truncated), windowed
  (paged, with a `nextOffset` to follow), or aged out of the transcript — and each state names a
  different recovery action, so "re-issue the identical call and hope" is never the rational move.
- The new primitives are **discoverable**: the assistant can find and correctly call them through
  the mechanisms it already uses to learn about tools.

## Non-Goals

- **Not** fuzzy, ranked, semantic, or typo-tolerant search. v1 is plain case-insensitive
  substring matching over both `project.search` and `meta.search`. Relevance ranking and fuzzy
  matching are a possible later enhancement.
- **Not** a replacement for the exact resolvers. `getByPath` / `getByTitle` / `resolve` /
  `meta.describeCommand` stay as the precise-addressing path; search is the *discovery* step that
  feeds them.
- **Not** search over live dataset **values** or telemetry. This is structural/project-metadata
  and API-catalog search only.
- **Not** changing the hotpath, the data model, or any persisted project format.

## Requirements

1. **R1 — Project search.** A new read-only command `project.search` accepts a required
   case-insensitive substring `query` and returns matching entities of **every project entity
   type — datasets, groups, actions, sources, workspaces, and data tables** — as typed compact
   rows. For datasets the match is against title + alias + units; for the other types, against
   the entity's title/name. It supports optional filtering by result `type` (one or more types)
   and by `groupId` / `sourceId`, and supports `offset` / `limit` paging. An empty or
   whitespace-only `query` is an explicit error (with a hint naming the list commands for
   enumeration) — search never silently matches everything. Result order is **deterministic and
   documented** (grouped by type, project order within a type), so `offset` paging over an
   unchanged project is coherent — no gaps, no duplicates. It never returns full item objects.

2. **R2 — Compact, typed, in-budget rows.** Each search row is small and self-locating so the
   assistant can act on it or drill in without a second lookup. Every row carries a `type`, a
   stable identifier for that type, and a human title/name. At minimum:
   - dataset row: `type="dataset"`, `uniqueId`, `title`, `path` ("Group/Dataset"), `groupId`,
     `groupTitle`, `index`, `units`.
   - group row: `type="group"`, `groupId` (positional), `uniqueId` (stable), `title`,
     `datasetCount`.
   - action row: `type="action"`, an identifier, and `title`.
   - source row: `type="source"`, `sourceId`, `title`.
   - workspace row: `type="workspace"`, `workspaceId`, `title`.
   - data-table row: `type="table"`, an identifier, and `title`.
   When a dataset row matched on something other than its title (alias or units), the row says
   which field matched, so the assistant understands *why* "psia" returned it. The response also
   carries `matchCount` (total matches across the whole project, not just this page),
   `nextOffset` when more remain, a one-line `_summary`, and the current `projectEpoch` (the same
   monotonic counter `project.snapshot` reports), so a caller paging across several calls can
   detect that the project mutated between pages and restart instead of trusting a torn view.

3. **R3 — Single-group read.** A new read-only command `project.group.get` returns one group
   plus a compact dataset summary (title / uniqueId / index / units per dataset), without
   returning the whole project. It accepts **either** group id space — the positional `groupId`
   (dense, 0..N-1, shifts on reorder) **or** the group's stable `uniqueId` (sparse per-project
   counter; the value workspace widget refs carry under the key `groupId`) — as two explicit,
   mutually exclusive parameters. The response carries both ids. If no group matches, the error
   is explicit and its hint explains the two id spaces (and that a large id from a workspace ref
   is a `uniqueId`), pointing at `project.search` / `project.group.list` to find valid ids.

4. **R4 — Pageable listings.** `project.dataset.list`, `project.group.list`, and
   `project.dataset.getExecutionOrder` (all three were reported blowing the budget) accept
   optional `offset` / `limit`, mirroring the windowing already present on `project.snapshot`.
   The response windows the array but always reports the whole-project total count, a
   `nextOffset` when more remain, and the current `projectEpoch`. A default call (no paging args)
   returns a bounded, in-budget page rather than the whole project, and states in the response
   that it is a window (so the assistant never mistakes page one for the full set).

5. **R5 — In-budget by construction.** On a 570-dataset / 87-group project, a default-parameter
   call to every command this spec adds or extends **whose rows are individually bounded**
   (`project.search`, `meta.search`, `project.group.get`, `project.dataset.list`,
   `project.dataset.getExecutionOrder`) returns a **structured** result that fits within the
   tool-result byte budget — i.e. it is not replaced by the `truncated:true` byte-prefix
   preview. Paging through with the returned `nextOffset` reaches every item.
   **Exception (D6, implementation-discovered):** `project.group.list` rows embed each group's
   full nested `datasets` array, so a single row is *unbounded* — the command cannot be made
   in-budget by row-count paging (one dense group can exceed any budget). It gains
   `offset`/`limit` paging and a count-capping assistant default, but the *compact, guaranteed-
   in-budget* discovery paths are `project.search` and `project.group.get`, not
   `project.group.list`.

6. **R6 — Accurate identity docs.** *(Amended during planning: code ground truth shows
   `groupId` is positional/dense; the sparse counter is `uniqueId` — and the workspaces API
   serializes the group's `uniqueId` under the JSON key `groupId`, which is the naming collision
   that actually confused the reporting assistant.)* The assistant-facing documentation (the
   help manual's identity model and the AI skills that describe project identity or workspaces)
   must state the two group id spaces explicitly: the positional `groupId` (dense, renumbers on
   reorder — what `project.group.*` commands take) versus the group `uniqueId` (sparse persisted
   counter, can be far larger than the group count — what workspace widget refs carry, *labeled*
   `groupId`), and warn that a large "groupId" seen in workspace data is a `uniqueId` that must
   not be passed to positional-id commands (except `project.group.get`, which accepts both).

7. **R7 — Discoverable.** The new/extended commands are exposed through the same channels the
   assistant already uses to learn tools: they appear in the assistant's tool surface, are
   self-describing via `meta.describeCommand`, and carry a read-only safety classification so they
   run without a confirmation gate. `project.search` results include a hint pointing to the
   exact-resolve / `project.group.get` commands for drilling into a chosen match.

8. **R8 — API-catalog search.** A new read-only command `meta.search` accepts a case-insensitive
   substring `query` and returns matching **callable tools** — the index for the assistant's whole
   tool surface, not just one namespace: every command family the assistant can invoke
   (`project.*`, `assistant.*`, `fs.*`, `meta.*`, and the rest of the registered catalog) is
   searchable from this one place. It matches against each command's name and description and
   returns compact rows (command `name`, its namespace/family, and a short description snippet),
   with `offset` / `limit` paging, `matchCount`, and `nextOffset`. It complements the existing
   `meta.describeCommand` (exact lookup) and `meta.fetchHelp`: `meta.search` is the discovery
   step, `describeCommand` the drill-in. A row's `name` is directly usable as the
   `meta.describeCommand` argument.

9. **R9 — Incomplete results are self-explaining.** The three ways a result can be incomplete
   are worded so they cannot be confused, and each names its own recovery:
   - **Windowed** (this spec's paging): the response itself says it is a page and carries
     `nextOffset` — recovery is "call again with this offset".
   - **Truncated** (over the byte budget): the notice states the result was too large and names
     the paging/filter parameters of *that command* as the fix — recovery is "narrow the call",
     never "retry it verbatim".
   - **Aged out** (transcript elision): the elision stub states that the *original call
     succeeded* and was collapsed only to save transcript space — recovery is "re-issue only if
     the data is needed again". Additionally, the new discovery results (`project.search`,
     `meta.search`, `project.group.get`) join the never-elided set that `meta.describeCommand`
     already enjoys, since eliding a discovery result forces blind retry loops.

## Acceptance Criteria

<Integration tests run against a live app instance with the API server enabled; the maintainer
runs them. Where a check is a maintainer observation, it is stated as such.>

- [x] **AC1 (R1/R2)** — An integration test loads a project, calls `project.search` with a
      substring that matches several datasets in different groups, and asserts the result contains
      typed dataset rows with the R2 fields, a correct `matchCount`, and a `path` that
      `project.dataset.getByPath` then resolves successfully.
      *(test_project_discovery.py::test_search_basic + test_search_matched_field_provenance — pass.)*
- [x] **AC2 (R1)** — `project.search` with a `type` filter returns only rows of that type; with a
      `groupId` filter returns only datasets in that group; queries matching a group, source,
      workspace, and data-table title each return a row of the corresponding `type`.
      *(test_search_filters_and_types — pass.)*
- [x] **AC3 (R3)** — `project.group.get` returns the same group's compact dataset summary
      whether addressed by positional `groupId` or by stable `uniqueId` (both ids present in the
      response); passing both selectors, neither, or a non-existent id returns an explicit error
      whose hint explains the two id spaces.
      *(test_group_get_both_id_spaces + test_group_get_selector_errors — pass.)*
- [x] **AC4 (R4)** — `project.dataset.list` / `project.group.list` /
      `project.dataset.getExecutionOrder` with `limit=N` return exactly N items (or fewer at the
      tail), report the full-project total and `projectEpoch`, and return a `nextOffset` that
      pages to the remaining items with no gaps or duplicates; two identical paged calls on an
      unchanged project return identical rows (deterministic order).
      *(test_list_paging + test_unpaged_calls_remain_complete — pass.)*
- [x] **AC5 (R5)** — Against a fixture project with 500+ datasets, default-parameter calls to
      every bounded-row command produce results that are **not** `truncated` (verified by
      compact-size assertion); `project.group.list` is excluded per D6 (unbounded nested rows)
      and instead verified to page correctly.
      *(test_compact_defaults_fit_budget + test_group_list_is_paged_not_byte_bounded — pass.)*
- [x] **AC6 (R6)** — `scripts/documentation-verify.py` passes on the edited docs, and the edited
      identity docs contain the sparse-counter warning (maintainer review).
- [x] **AC7 (R7)** — `meta.describeCommand` returns a schema for each new command, and the AI
      static-surface test (`tests/scripts/test_ai_assistant_static.py`) passes with the new
      commands present in the catalog and classified read-only in the safety policy.
- [x] **AC8 (R8)** — `meta.search` with a substring that appears in several command names/
      descriptions returns compact rows whose `name` each resolve via `meta.describeCommand`;
      rows span more than one namespace when matches exist in several (e.g. a query hitting both
      `project.*` and `fs.*`); paging with `nextOffset` covers all matches; `matchCount` reflects
      the full match set.
- [x] **AC9 (R9)** — The three incomplete-result notices use distinct wording: the truncation
      notice names the command's paging/filter parameters, the elision stub states the original
      call succeeded and was collapsed to save transcript space, and a windowed response
      identifies itself as a page. Verified by the AI static-surface test where the strings are
      static, plus maintainer review of one live transcript on the large fixture project.

## Constraints & Invariants

- **Read-only.** Every command this spec adds or extends only reads state (project model or
  command catalog); none mutates, so none touches the epoch-gated mutation/autosave apply path.
  They must read live state (reflect the current project/catalog), not a cached snapshot that
  could go stale.
- **In-budget by construction** (R5) is the deciding constraint: correctness of the response shape
  is defined by fitting the byte budget at default paging, not by the caller picking a small slice.
- **Discoverability parity.** New `project.*` commands must be registered wherever existing
  `project.*` read commands are registered so they inherit the assistant tool surface, the
  `meta.describeCommand` schema path, and a safety tier — a command the assistant cannot see or is
  gated behind a confirmation defeats the purpose.
- **No hotpath impact**; no new third-party dependency; no persisted-format change.
- Consistent with existing paging: reuse the `offset` / `limit` / `nextOffset` / `projectEpoch`
  shape already established by `project.snapshot` rather than inventing a second convention.
- **Never-elided must mean bounded.** Joining the elision-exempt set (R9) is only safe because
  these results are compact by construction (R5); the exemption must not extend to any command
  whose result size is unbounded, or transcripts balloon.
- `meta.search` must be available everywhere `meta.describeCommand` is available — the discovery
  step and the drill-in step travel together.

## Resolved Decisions

- **D1 — Search field breadth (was Q1).** `project.search` matches `title` + alias + `units` for
  datasets, and title/name for every other type. (Broad recall.)
- **D2 — Entity coverage (was Q3).** `project.search` covers **all** project entity types:
  datasets, groups, actions, sources, workspaces, and data tables.
- **D3 — API-catalog search (was Q4).** `meta.search` is **in scope** for this spec (R8), not a
  follow-up. This spec delivers both the project index and the API index in one pass.
- **D4 — Incomplete-result disambiguation added (R9).** The reporting session's fourth pain point
  — truncation vs transcript elision being indistinguishable — is in scope: the three
  incomplete-result states get distinct wording and distinct recovery paths, and the new
  discovery commands join the existing never-elided set.
- **D6 — group.list excluded from the in-budget guarantee (implementation-phase, live-test
  discovered).** The integration run showed `project.dataset.list` at limit 8 = 5588 bytes and,
  on inspection, that a `project.group.list` row embeds the group's full nested `datasets`
  array (unbounded per row). R5 now guarantees in-budget-by-construction only for
  bounded-row commands; `project.group.list` is pageable but not byte-bounded. The same run
  found `project.group.list` never exposed the positional `groupId` (`serialize(Group)` omits
  it as positional) — now added to each row so `project.group.get{groupId}` is usable from a
  list result. Assistant shim defaults recalibrated against the measurement: dataset.list
  8→4, getExecutionOrder 50→20, group.list count-cap 5.
- **D5 — R3/R6 corrected against code ground truth (planning-phase amendment).** `Group.groupId`
  is positional/dense (`Frame.h`), the sparse identity is `Group.uniqueId`, and the workspaces
  API serializes `uniqueId` under the JSON key `groupId` (`WorkspacesHandler` refs). The spec's
  original "groupId is sparse" claim was the same misconception the reporting assistant had. R3
  now requires `project.group.get` to accept either id space; R6 now requires documenting the
  collision rather than a false sparseness claim. Re-confirmed at the plan gate.

## Open Questions

- **Q2 — Default and maximum `limit`.** Default `limit` ≈ 50 rows and a hard cap ≈ 200 (agreed in
  principle); exact numbers to be tuned in `/ss-plan` against real serialized sizes so a default
  page fits the ~4 KB budget with the compact row shape.
