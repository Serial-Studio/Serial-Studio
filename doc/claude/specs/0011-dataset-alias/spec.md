---
spec: 0011-dataset-alias
title: Dataset aliases for script and API dataset lookup
status: done          # closed 2026-08-20
created: 2026-07-15
author: Alex Spataru
---

# Spec 0011 — Dataset aliases for script and API dataset lookup

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Scripts read another dataset's value with `datasetGetRaw(uniqueId)` /
`datasetGetFinal(uniqueId)` (JS and Lua), and API commands that target a dataset take the
same numeric uniqueId. In a small project that is tolerable; in a real deployment with
hundreds of datasets (the driving case has ~800), a transform full of
`datasetGetRaw(128)` calls is unreadable and unreviewable — nothing in the script says
*which* channel 128 is, and the only way to find out is to open the project editor and
hunt. The number is also machine-assigned: it encodes source/group/position, so
restructuring the project can hand a dataset a different id and silently repoint every
script that referenced the old one.

A human-chosen, per-dataset alias — `datasetGetRaw("ATAM1-CH1")` — makes the script
self-documenting and keeps working across id renumbering, because the alias is a property
the user owns rather than a coordinate the system derives.

## Goals

- A user can give any dataset a short, stable, human-readable alias in the project editor,
  and that alias persists with the project file.
- Scripts (JS and Lua, everywhere `datasetGetRaw` / `datasetGetFinal` are available) can
  pass the alias string instead of the numeric uniqueId and get the same value back.
- API commands that identify a target dataset by uniqueId accept the alias string as an
  equivalent identifier.
- A project with hundreds of existing datasets can be alias-enabled in one action: a bulk
  "seed aliases from titles" fill, not 800 manual edits.
- A wrong alias fails loudly enough to debug: the lookup returns the same "not found"
  result as an unknown uniqueId, plus a console warning naming the unresolved alias.

## Non-Goals

- Aliases do **not** replace uniqueId anywhere. Numeric lookup keeps working unchanged;
  project JSON cross-references (x-axis source, waterfall Y source, workspace widget refs)
  stay numeric.
- No aliases for groups, actions, tables, or registers — datasets only.
- No display/UI role: the alias is an identifier, not a second title. Dashboards, widgets,
  exports, and CSV headers keep using the title.
- No retroactive script rewriting: existing scripts that use numeric ids are untouched.
- No alias support in Built-In (Native) frame-parser descriptors.

## Requirements

1. **R1 — Alias field.** Every dataset has an optional `alias` property, editable in the
   project editor and persisted in the `.ssproj` JSON. Empty means "no alias" and is the
   default; existing project files load unchanged.
2. **R2 — Uniqueness at edit time.** The editor refuses to commit an alias that is already
   assigned to another dataset in the project (case-sensitive comparison, surrounding
   whitespace trimmed before validation). Empty aliases never collide.
3. **R3 — String lookup in scripts.** `datasetGetRaw` and `datasetGetFinal` accept a string
   argument in both JS and Lua, in every script context where those functions are
   available today. A string is always treated as an alias — never coerced to a number
   — and a number is always treated as a uniqueId, so `"128"` and `128` are distinct
   lookups.
4. **R4 — Unresolved alias behavior.** A string that matches no alias returns the same
   result as an unknown uniqueId today (JS `null` / Lua `nil`) and emits a one-time console
   warning naming the alias, mirroring the existing unknown-uniqueId warning.
5. **R5 — Rename follows the dataset.** The alias is bound to the dataset, not its
   position: reordering groups, moving the dataset, or any operation that renumbers ids
   leaves alias lookups resolving to the same dataset.
6. **R6 — Live rename.** Changing a dataset's alias in the editor takes effect the next
   time scripts run against the rebuilt project (same lifecycle as changing a title or
   transform) — no application restart.
7. **R7 — API acceptance.** Every API server command that identifies a target dataset by
   its numeric uniqueId also accepts the alias string in that parameter, with the same
   numeric-vs-string discrimination as R3. Unknown aliases produce the command's normal
   "dataset not found" error, naming the alias.
8. **R8 — Bulk seed.** The project editor offers a single action that fills every *empty*
   alias from the dataset's title, sanitized to a stable identifier form and de-duplicated
   deterministically (e.g. numeric suffixes) so the result always satisfies R2. Datasets
   that already have an alias are untouched.
9. **R9 — Documentation.** The SDK reference, scripting help pages, and the in-app
   assistant corpus document the alias parameter form wherever `datasetGetRaw` /
   `datasetGetFinal` and the affected API commands are described.

## Acceptance Criteria

- [ ] **AC1** — A `tests/scripts/` JS unit (or equivalent transform-path check) shows
  `datasetGetRaw("<alias>")` and `datasetGetRaw(<uid>)` returning the same value for the
  same dataset, and `datasetGetFinal` likewise, in both JS and Lua.
- [ ] **AC2** — In the running app: assign an alias in the editor, reference it from a
  transform, observe the value; rename a *different* dataset's alias to the same string
  and observe the editor reject it (R2).
- [ ] **AC3** — With an alias in use, restructure the project so the dataset's uniqueId
  changes; the alias lookup still resolves to the same dataset (R5).
- [ ] **AC4** — `datasetGetRaw("no-such-alias")` returns JS `null` / Lua `nil` and logs
  one warning naming the alias; repeating the call does not spam the log (R4).
- [ ] **AC5** — A `pytest` integration test drives an API command that targets a dataset,
  once with the numeric uniqueId and once with the alias, and gets equivalent results;
  an unknown alias yields the "dataset not found" error naming the alias (R7).
- [ ] **AC6** — Bulk seed on a project with duplicate titles yields all-unique, non-empty
  aliases for previously-empty fields and leaves pre-existing aliases untouched (R8);
  verifiable in the editor and the saved `.ssproj`.
- [ ] **AC7** — `--benchmark-hotpath` still clears all nine gates after the change (alias
  plumbing must be free when unused and cheap when used).
- [ ] **AC8** — Loading a pre-alias `.ssproj` and re-saving it round-trips without data
  loss or schema complaints (R1).

## Constraints & Invariants

- **Hotpath:** `datasetGetRaw`/`datasetGetFinal` run inside transforms at frame rate. The
  numeric-id path must not get slower, and alias resolution must not allocate per call in
  steady state — the 256 kHz benchmark gate is the arbiter (AC7).
- **Identity model:** uniqueId remains the persisted stable identity; the alias is an
  additional user-owned name layered on top, never a replacement key in stored
  cross-references.
- **Schema compatibility:** older Serial Studio builds opening a project that contains
  aliases must at worst ignore the field; projects without aliases load bit-identical to
  today.
- **Safety-by-default for the API surface:** alias acceptance must not weaken any
  existing validation — an alias resolves to a uniqueId first, then the command runs
  exactly as if that uniqueId had been passed.
- No new dependencies; works in both GPL and Commercial builds (aliases are not Pro-gated).

## Open Questions

- Sanitization rules for bulk seed (R8): keep titles nearly verbatim (trim + collapse
  whitespace) or normalize harder (e.g. spaces to `-`)? Deciding at plan time is fine;
  the invariant is only "deterministic, unique, non-empty".
- Should the editor *warn* (not block) when a manually-typed alias is all digits, since a
  numeric string can never be looked up as an alias-typed number per R3? Cheap guard
  against a confusing footgun; default is to add the warning unless vetoed.
