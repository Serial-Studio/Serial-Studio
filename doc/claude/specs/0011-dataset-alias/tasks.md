---
spec: 0011-dataset-alias
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-15
---

# Tasks 0011 — Dataset aliases for script and API dataset lookup

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.

## Tasks

### T1 — Dataset model field + persistence

- **Files:** `app/src/DataModel/Frame.h`, `app/src/DataModel/Frame.cpp`
- **Does:** Adds `QString alias;` to `struct Dataset` (QString block, :438 area),
  `Keys::Alias("alias")`, write-when-non-empty serialization following the `color` pattern
  (:1133), and `d.alias = ss_jsr(obj, Keys::Alias, "").toString().simplified()` in
  `read(Dataset&, ...)`. Invariant: pre-alias projects must round-trip byte-identical —
  never write an empty alias key.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/Frame.h
  app/src/DataModel/Frame.cpp`; read-back that serialize/read are symmetric and the
  `static_assert` on Dataset alignment still holds.
- **Deps:** none
- [x] done

### T2 — Store alias index, QString accessors, register-name mirror

- **Files:** `app/src/DataModel/DataTable.h`, `app/src/DataModel/DataTable.cpp`
- **Does:** In `initialize()`'s dataset loop, for non-empty aliases: insert into new
  `QHash<QString, QPair<int,int>> m_aliasIndex` and mirror `("__datasets__", "raw:<alias>")`
  / `("final:<alias>")` into `m_index` at the existing slots. Adds
  `getDatasetRawByAlias(const QString&)` / `getDatasetFinalByAlias(const QString&)` (resolve
  → `captureRead(slot)` → storage read), `noteMissingAlias` one-shot warning, and clearing
  of the new members in `clear()`. **Binding invariants:** cold path only; probe `m_index`
  with `constFind` before mirror-insert (QHash::insert overwrites — collision must skip +
  warn once, never replace a uid register); do NOT extend `m_tableRegNames` (snapshot() /
  exports unchanged); duplicate aliases in a hand-edited file: first wins + warn; all
  lookups `constFind`, never `operator[]`.
- **Verify:** `python scripts/code-verify.py --check app/src/DataModel/DataTable.h
  app/src/DataModel/DataTable.cpp`; read-back against the invariants above.
- **Deps:** T1
- [x] done

### T3 — Lua alias path: interned cache + closure type-switch

- **Files:** `app/src/DataModel/DataTable.h`, `app/src/DataModel/DataTable.cpp`,
  `app/src/DataModel/FrameBuilder.cpp`
- **Does:** Adds interned-pointer alias accessors (`const char*` forms of the ByAlias
  getters) backed by a small fixed cache mirroring `m_internedKeyCache` (entry: alias ptr →
  raw/final slots or -1), cleared in `clearLookupCache()`. `luaDatasetGetRaw` /
  `luaDatasetGetFinal` (:2642/:2668) switch on `lua_type(L, 1)`: `LUA_TSTRING` → alias
  accessor, else existing `luaL_checkinteger` path unchanged. **Binding invariants
  (hotpath):** must use `lua_type()`, never `lua_isnumber()` (numeric strings are aliases
  per spec R3); zero steady-state allocation on the cached path; the numeric path stays
  byte-identical; cache invalidation must ride the existing `clearLookupCache()` call sites
  (Lua state close, store clear) — no new lifecycle hooks.
- **Verify:** `python scripts/code-verify.py --check` on the three files; read-back: trace
  one hit and one miss through the cache; confirm `m_tableStore.clearLookupCache()` at
  FrameBuilder.cpp:2253 covers the new cache.
- **Deps:** T2
- [x] done

### T4 — JS bridge alias path (QJSValue parameter)

- **Files:** `app/src/DataModel/DataTable.h`, `app/src/DataModel/DataTable.cpp`
- **Does:** Changes `TableApiBridge::datasetGetRaw/Final` from `int` to
  `const QJSValue&`: `isString()` → QString alias accessor; number → `toInt()` → existing
  uid path; anything else → empty QVariant. **Binding invariants:** never coerce a JS
  string to a number (`"128"` ≠ `128`); `prelude.js` pass-through (`__ss.datasetGetRaw(uid)`)
  must keep working unchanged for numeric callers.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back of the type
  branch against R3.
- **Deps:** T2
- [x] done

### T5 — Shared API dataset selector helper

- **Files:** `app/src/API/Handlers/ProjectApiSupport.h`
- **Does:** Adds a shared resolver taking the raw `QJsonValue` selector: number → uniqueId;
  string → trimmed alias scanned against `ProjectModel::groups()`; returns the resolved
  dataset (group + dataset refs or uid) or an error message that names the unresolved alias
  ("Dataset not found for alias 'X'") / uid, matching the existing inline error shape.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Handlers/ProjectApiSupport.h`;
  header-only read-back (Christmas-tree order, `[[nodiscard]]`).
- **Deps:** T1
- [x] done

### T6 — Wire selector into project.dataset.getByUniqueId + move

- **Files:** `app/src/API/Handlers/ProjectHandlerBatch.cpp`,
  `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** `datasetGetByUniqueId` (:204) and `datasetMove` (:363) accept string selectors
  via the T5 helper (replacing the inline uid loops); registration doc strings (:962 area,
  :1065 area) advertise the number-or-alias form. Behavior after resolution is unchanged
  (spec safety invariant: resolve first, then run exactly as if the uid was passed).
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that error
  paths name the alias when a string was passed.
- **Deps:** T5
- [x] done

### T7 — dashboard.tailFrames alias filter elements

- **Files:** `app/src/API/Handlers/DashboardHandler.cpp`
- **Does:** The `uniqueIds` filter array (:474) accepts string elements, resolved to uids
  via the T5 helper before building the filter set; unresolved strings are skipped silently
  (matching today's silent-skip filter semantics, not an error).
- **Verify:** `python scripts/code-verify.py --check app/src/API/Handlers/DashboardHandler.cpp`.
- **Deps:** T5
- [x] done

### T8 — Assistant surfaces accept aliases

- **Files:** `app/src/API/Handlers/AssistantHandler.cpp`, `app/src/AI/ToolDispatcher.cpp`
- **Does:** `datasetResolve` (:105) and `resolveAddTileDataset` (:508) treat a string
  `uniqueId` as an alias (resolve via T5 helper or forward the string through to the
  T6-updated command); ToolDispatcher `uniqueId` schema properties (:139, :292) become
  integer-or-string with description text, and `resolveDataset` (:773) /
  `scriptTargetDataset` (:1077) pass string selectors through instead of `.toInt()`-ing
  them. Invariant: no new assistant tool is created, so no dual-registration / safety-JSON
  work — only param shapes on existing tools change.
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back that
  `.toInt()` on a possibly-string selector is gone from the touched paths.
- **Deps:** T6
- [x] done

### T9 — project.dataset.update learns the alias field

- **Files:** `app/src/API/Handlers/ProjectHandler.cpp`
- **Does:** `applyDatasetTextAndToggleFields` (:1420) accepts `alias` via the
  `takeParam(..., consumed, Keys::Alias)` idiom with `.simplified()` normalization and a
  project-wide uniqueness check returning an error string (the `color` validity-check
  shape); the command's field-list doc string (:1175 area) names `alias`. **Binding
  invariant:** the param MUST enter the `consumed` set or `appendUnknownFieldsWarning`
  (:1685) silently drops it — the known trap from the plan.
- **Verify:** `python scripts/code-verify.py --check app/src/API/Handlers/ProjectHandler.cpp`;
  read-back that both accept and reject paths mark the param consumed.
- **Deps:** T5
- [x] done (whitelist lives in ProjectHandlerEntities.cpp, not ProjectHandler.cpp -- same
  ProjectHandler split TU; doc string updated in ProjectHandler.cpp)

### T10 — Editor alias row + commit case

- **Files:** `app/src/DataModel/Project/ProjectEditorItemIds.h`,
  `app/src/DataModel/Project/ProjectEditorForms.cpp`,
  `app/src/DataModel/Project/ProjectEditorCommit.cpp`
- **Does:** Adds `kDatasetView_Alias`, a `TextField` row in the dataset General Information
  form (beside units, :904 area), and the commit case in `onDatasetCommonItemChanged`
  (:553 block) applying `.simplified()`. Read the existing form/commit wiring in full
  before adding (signal-wiring rule); no tree rebuild needed (falls through to the
  `rebuildTree=false` + `syncDatasetItemCache` path).
- **Verify:** `python scripts/code-verify.py --check` on the three files; read-back that
  the row renders via the generic `TableDelegate` (no QML change needed).
- **Deps:** T1
- [x] done (alias row extracted to member helper addDatasetAliasRow to keep addGeneralSection
  under the 100-line cap; +1 declaration in ProjectEditor.h)

### T11 — Editor uniqueness guard, all-digit warning, multi-select exclusion

- **Files:** `app/src/DataModel/Project/ProjectEditorCommit.cpp`,
  `app/src/DataModel/Project/ProjectEditorMultiSelect.cpp`
- **Does:** In `onDatasetItemChanged` (~:836), before `pm.updateDataset`: reject an alias
  already assigned to another dataset (restore the previous value via the existing
  `syncDatasetItemCache` path and surface the reason using the editor's current validation
  idiom); warn once (don't block) on all-digit aliases (spec open question, resolved:
  warn). Excludes the alias row from the multi-selection aggregate form (a shared alias is
  never valid).
- **Verify:** `python scripts/code-verify.py --check` on both files; read-back the
  reject-restore path against the batch-apply flag interactions (:599/:612/:642/:771).
- **Deps:** T10
- [x] done (reject uses a message box + deferred buildDatasetModel snap-back; all-digit warns
  via an informational box)

### T12 — Bulk "seed aliases from titles" action

- **Files:** `app/src/DataModel/ProjectModel.h`,
  `app/src/DataModel/Project/ProjectModelCrud.cpp`,
  `app/qml/ProjectEditor/Sections/ProjectStructure.qml`
- **Does:** New `ProjectModel` public slot `seedDatasetAliases()`: fills every *empty*
  alias from `title.simplified()`, deterministic dedupe with `-2`/`-3`... suffixes against
  all existing + newly seeded aliases, never touches non-empty aliases; wrapped in the
  `setAutoSaveSuspended` / batch-apply / single `flushAutoSave()` idiom
  (ProjectEditorMultiSelect.cpp:278 pattern). Exposed as a context-menu entry on the
  project root in `ProjectStructure.qml`. Read the existing context-menu wiring in that QML
  file in full before adding the entry.
- **Verify:** `python scripts/code-verify.py --check` on all three; read-back: seeding is
  idempotent (second run is a no-op) and the result always satisfies R2 uniqueness.
- **Deps:** T11
- [x] done (mutates m_groups directly to avoid updateDataset's m_selectedDataset churn over 800
  datasets; entry added to the tree sharedContextMenu, always visible)

### T13 — SDK preludes + regenerated bundles

- **Files:** `app/rcc/api/prelude.js`, `app/rcc/api/prelude.lua` (hand-edited);
  `app/rcc/api/SerialStudio.js`, `app/rcc/api/SerialStudio.lua`,
  `app/rcc/api/sdk-symbols.json` (regenerated)
- **Does:** Updates the `datasetGetRaw`/`datasetGetFinal` doc comments to the
  `(uniqueIdOrAlias)` form (bodies are pass-throughs — unchanged); runs
  `python scripts/generate-sdk.py` to regenerate the three bundles. Note: `api-schema.json`
  is NOT regenerated here — that needs the maintainer's `--dump-api-schema` run after T6-T9
  land (named in the handoff).
- **Verify:** `git diff` on the generated files shows only the expected doc-comment
  propagation; `python scripts/code-verify.py --check` on the preludes.
- **Deps:** T4, T3
- [x] done (regenerated: only the datasetGetRaw/Final param-name doc propagated;
  sdk-symbols.json unchanged)

### T14 — Help-manual updates

- **Files:** `doc/help/SerialStudio-SDK.md`, `doc/help/Dataset-Transforms.md`,
  `doc/help/Data-Tables.md`, `doc/help/Identity-Model.md`, `doc/help/API-Reference.md`
- **Does:** Documents the alias parameter form at every audited mention (SDK data-tables
  table L112; Dataset-Transforms L137/L179/L226; Data-Tables system-table + transform-API +
  derived-quantities sections; Identity-Model gains the alias as a parallel user-owned
  identity in the ID table + rule of thumb; API-Reference entries for
  `getByUniqueId`/`move`/`tailFrames`/`dataset.update`). Mirrors the manual's voice; run
  through the ss-docs conventions.
- **Verify:** `python scripts/documentation-verify.py` clean on the touched pages.
- **Deps:** T13
- [x] done (all 5 pages pass documentation-verify.py, 0 findings; Identity-Model gained an
  alias subsection; API-Reference kept to prose only pending schema regen)

### T15 — Assistant corpus updates + search index

- **Files:** the 12 `app/rcc/ai/docs/*.md` + `app/rcc/ai/skills/*.md` files mentioning
  `datasetGetRaw/Final` (transform_js, transform_lua, painter_js, frame_parser_js,
  control_script_js, output_widget_js; skills: transforms, painter, project_basics,
  control_script, api_semantics, debugging); `app/rcc/ai/search_index.json` (regenerated)
- **Does:** Adds the alias form wherever the numeric form is taught (per the
  ai-corpus-drift memory: keep claims grounded in the shipped behavior — parser/transform
  scripts get the direct functions, control scripts get `tableGet("__datasets__",
  "raw:<alias>")` via the register mirror); rebuilds the index with
  `python app/rcc/ai/build_search_index.py`.
- **Verify:** `pytest tests/scripts/test_ai_assistant_static.py -v` (index freshness
  check); grep the corpus for a stale `datasetGetRaw(uniqueId)`-only claim.
- **Deps:** T14
- [x] done (verified at HEAD 2026-08-14: all 12 files mention `alias`; committed in
  5d215834d; `pytest tests/scripts/test_ai_assistant_static.py -v` 9/9 passed)

### T16 — JS unit coverage (runnable here)

- **Files:** `tests/scripts/conftest.py`, `tests/scripts/test_table_handles.py` (or a new
  `tests/scripts/test_dataset_alias.py` if cleaner)
- **Does:** Extends `TABLE_API_SHIM` with alias-aware `datasetGetRaw/Final` modeling the
  C++ semantics (string = alias only, number = uid only, unknown → `undefined`); adds
  cases: alias/uid equivalence (AC1 JS half), `"128"` vs `128` discrimination (R3),
  unknown-alias returns `undefined` (AC4 shape).
- **Verify:** `pytest tests/scripts/ -v` passes locally (Node.js only, no app).
- **Deps:** T4
- [x] done (new tests/scripts/test_dataset_alias.py, 7 cases; full scripts suite 236 passed)

### T17 — Integration test for the API surface (maintainer runs)

- **Files:** `tests/integration/test_dataset_alias.py` (new)
- **Does:** Drives a live app: `project.dataset.getByUniqueId` numeric-vs-alias equivalence
  + unknown-alias error naming the alias (AC5); `project.dataset.update` sets an alias,
  rejects a duplicate (R2 API half); `dashboard.tailFrames` alias filter. Follows
  tests/README.md fixtures/markers; not runnable by Claude (needs the app on :7777).
- **Verify:** file passes `python scripts/code-verify.py`-adjacent lint (black via
  sanitize); maintainer runs `pytest tests/integration/test_dataset_alias.py -v`.
- **Deps:** T9, T7, T6
- [x] done (5 tests, collects cleanly; maintainer runs live. tailFrames alias filter left to
  the T16 JS/uid discrimination coverage since API-Reference has no tailFrames prose entry)

### T18 — Self-review + handoff

- **Files:** none (review pass)
- **Does:** Re-reads the full diff against the plan's file table (lane check); runs the
  counterfactual check (which rule does this diff most risk violating — hotpath allocation
  in the Lua alias path — and the concrete evidence it doesn't); runs `qt-cpp-review` on
  the C++ diff; runs `python scripts/sanitize-commit.py`. Handoff note lists the
  maintainer-only steps: `--dump-api-schema` refresh, `--benchmark-hotpath` (AC7),
  integration pytest (AC5), in-app observations (AC2/AC3/AC4/AC6/AC8), `.ts` catalog
  regeneration.
- **Verify:** sanitize-commit exits clean; findings from qt-cpp-review addressed or noted.
- **Deps:** T1–T17
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met or explicitly handed to the maintainer
      (AC5/AC7 + in-app checks) and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath invariants named and verified per T3/T4; `--benchmark-hotpath` handed to the
      maintainer/CI (AC7) with no regression expected from the design.
- [x] `pytest tests/scripts/ -v` passes locally; integration targets listed for the
      maintainer.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (WidgetDelegate.qml's pre-existing modification stays untouched).
- [x] `spec.md` status set to `done`.
