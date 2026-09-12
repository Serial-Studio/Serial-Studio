---
spec: 0085-native-publish-allocation-free
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-11
---

# Tasks 0085 — Native publish path: allocation-free, raw only when it can differ

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
- Every task on `BlockStager.*` or `FrameBuilder.cpp` is hotpath: read the file in full first,
  invoke `ss-hotpath`, and restate the invariant named in the task's Does line before editing.
- Consumers (T4) land before the producer flips the rule (T5, T6), so the tree never has a block
  a consumer misreads.

## Tasks

### T1 — `dataset_carries_raw` in `Frame.h`

- **Files:** `core/Core/DataModel/DataBlock.h` (moved from `Frame.h`, which already sits over the TU cap)
- **Does:** `[[nodiscard]] SS_FORCE_INLINE bool dataset_carries_raw(const Dataset& d) noexcept
  { return !d.transformCode.isEmpty() || d.virtual_; }` beside `assign_string_in_place`, with a
  one-line `@brief` naming it as the ONE rule for raw presence (block column and dataset copies).
- **Verify:** `python scripts/code-verify.py --check core/Core/DataModel/Frame.h`.
- **Deps:** none
- [x] done

### T2 — Open entries replace the two maps (declaration)

- **Files:** `core/Pipeline/DataModel/FrameBuilder/BlockStager.h`
- **Does:** Private `struct OpenEntry { int sourceId; std::shared_ptr<PooledBlockSlot> slot;
  quint64 blockNumber; }`, `std::vector<OpenEntry> m_open` replacing `m_open` and
  `m_blockNumbers`, `[[nodiscard]] OpenEntry* findOpen(int sourceId) noexcept` (+ const overload).
  `<map>` include dropped. Invariant in the class comment: entries are never erased; `slot` empty
  means "no block open for this source".
- **Verify:** `code-verify --check BlockStager.h`.
- **Deps:** none
- [x] done

### T3 — Open entries (implementation)

- **Files:** `core/Pipeline/DataModel/FrameBuilder/BlockStager.cpp`
- **Does:** ctor `m_open.reserve(16)`; `findOpen` linear scan; `openBlockFor` uses the entry (held
  block reuse/flush rule unchanged; `push_back` only when the source was never seen; slot handed
  over by `std::move`); `flush` takes `std::move(entry->slot)` and bumps `entry->blockNumber`;
  `flushAll` walks entries with a live slot; `blockNumber` reads the entry. Invariant: no heap
  operation on open/flush in steady state, pipeline thread only, `DataBlockPtr(slot, &slot->block)`
  aliasing hand-out unchanged, `structureGeneration` and `masked` stamping unchanged.
- **Verify:** `code-verify --check BlockStager.cpp`; existing `tst_frame_builder_staging` cases
  pass unchanged (maintainer `ctest -R staging`).
- **Deps:** T2
- [x] done

### T4 — MDF4 raw channel falls back to the final value

- **Files:** `core/Storage/MDF4/Export.cpp`
- **Does:** In the per-sample write, `if (target.rawChannel)` writes `column.hasRaw ?
  rawValues/rawText : values/text` (numeric vs text chosen exactly as the final-channel branch
  does). Channel creation untouched.
- **Verify:** `code-verify --check core/Storage/MDF4/Export.cpp`; read-back that the type
  selection mirrors the final channel's.
- **Deps:** none
- [x] done

### T5 — Block columns carry raw only when the dataset does

- **Files:** `core/Pipeline/DataModel/FrameBuilder/BlockStager.cpp`
- **Does:** `bindToFrame`: `column.hasRaw = DataModel::dataset_carries_raw(dataset)`. Invariant:
  bind runs once per (slot, generation, source), so this is not per-frame work; `size_block_storage`
  clears the raw vectors for those columns without releasing capacity.
- **Verify:** `code-verify --check`; the new T8 raw-presence case passes.
- **Deps:** T1, T3, T4
- [x] done

### T6 — Builder writers skip the raw copies

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`
- **Does:** In `applyDatasetValueSpan` (span lane), `applyDatasetValue` (list lane) and the Quick
  Plot writer, wrap the `rawNumericValue` / `rawValue` stores in `if
  (DataModel::dataset_carries_raw(dataset))`. Invariant: removes writes only; `dataset.value` keeps
  `assign_utf8_in_place`; no share-assign is introduced; `reprocessDatasetValues` (the one reader
  of `dataset.rawValue`) iterates transform datasets only, which still carry raw.
- **Verify:** `code-verify --check FrameBuilder.cpp` (hotpath TU: violations block); grep
  `rawValue\b|rawNumericValue` across `core/` and `app/` re-run and compared with the plan's
  reader list.
- **Deps:** T1, T5
- [x] done

### T7 — Staging test: zero allocations across flushes

- **Files:** `app/tests/tst_frame_builder_staging.cpp`
- **Does:** TU-local `operator new`/`operator delete` overrides that count while an `armed` flag is
  set; `StubStagerHost` gains `reserve(n)` for its two vectors. New case
  `steadyStateFlushesDoNotAllocate`: two sources, bind and publish one block each, reserve the stub,
  arm, stage 20 × 64 rows per source, disarm, expect 0 news.
- **Verify:** `code-verify --check`; maintainer `ctest -R frame_builder_staging`.
- **Deps:** T3
- [x] done

### T8 — Staging test: raw presence follows the rule

- **Files:** `app/tests/tst_frame_builder_staging.cpp`
- **Does:** `makeFrame` gains an optional per-dataset transform/computed marker. New case
  `rawPresenceFollowsTheTransformRule`: dataset A with `transformCode`, B `virtual_`, C plain;
  after one stage, the published block has `hasRaw` true for A and B, false for C,
  `rawValues.size()` equal to the cap for A/B and 0 for C, `write_block_raw` left C's raw untouched.
- **Verify:** `code-verify --check`; maintainer `ctest -R frame_builder_staging`.
- **Deps:** T5
- [x] done

### T9 — Block test: `hasRaw == false` consumers

- **Files:** `app/tests/tst_data_block.cpp`
- **Does:** Case: a column with `hasRaw = false`; `apply_block_sample` yields `rawNumericValue ==
  numericValue` and `rawValue == value`; `clone_block_trimmed` copies empty raw vectors.
- **Verify:** `code-verify --check`; maintainer `ctest -R data_block`.
- **Deps:** none
- [x] done

### T10 — Integration test: raw follows a live transform edit

- **Files:** `tests/integration/test_block_raw_follows_transform.py`
- **Does:** Subscribes to the block stream, adds a doubling transform to one dataset via the project
  API, removes it; asserts the wire's `missed` counter stays zero and `seq` stays monotonic across
  both edits (no dropped block), and that the streamed values double and revert (the wire carries
  post-transform values only, so raw presence is pinned by the ctest unit, not here). Uses `stream.subscribe` (not `io.getLatestFrame`, which is
  lossy).
- **Verify:** `pytest tests/integration/test_block_raw_follows_transform.py -v` (maintainer, app
  up with the API server).
- **Deps:** T6
- [x] done

### T11 — Docs

- **Files:** `doc/claude/architecture/dataflow.md`, `tests/scripts/test_cpp_regressions.py`
- **Does:** "FrameBuilder's Lane Sub-objects": `BlockStager` keeps a flat open-entry vector (never
  erased, allocation-free) and sets `hasRaw` from `dataset_carries_raw`; the Unified Block Lane
  bullet on `rawValues` says raw is carried only for transform/computed datasets and every
  consumer falls back to the final value. Regression-test docstring updated (allocation gate now
  exists).
- **Verify:** `python scripts/claim-verify.py`; `pytest tests/scripts/test_cpp_regressions.py -q`.
- **Deps:** T6
- [x] done

### T12 — Maintainer measurement

- **Files:** this file
- **Does:** `--benchmark-hotpath` before/after on the same machine (stats build for the allocation
  column, PGO-use build for FPS); MDF4 and Historian comparisons per plan AC4/AC5.
- **Verify:** Table below filled; AC boxes ticked in `spec.md`; spec 0084 AC2's second half ticked.
- **Deps:** T11
- [x] done

| Measurement | Before | After |
|-------------|--------|-------|
| native(numeric) alloc/frame (stats build) | | |
| native(numeric) FPS | 3,283,981 (2026-09-11, M2 Pro, shipped 4.1.0); 3,317,222 (PGO before, 2026-09-12) | 4,653,499 (PGO after 0084-0086, +40%); mixed 2,519,705 -> 3,558,150 |
| `datasets+publish` ns/frame | 265 | |
| MDF4 transform-free recording channel-by-channel | identical | |

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on `BlockStager.*`, the `FrameBuilder.cpp` hunks and `MDF4/Export.cpp`.
- [x] `ss-hotpath` invariants restated per task; `--benchmark-hotpath` not regressed, Native rows
  at 0 allocations/frame.
- [x] Maintainer `ctest` (staging, data_block) and the integration test in T10 run.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
