---
spec: 0085-native-publish-allocation-free
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-11
---

# Plan 0085 — Native publish path: allocation-free, raw only when it can differ

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Two contained changes inside the frame lane's staging sub-object plus one rule shared with the
builder. **Open-block bookkeeping** moves from `std::map<int, shared_ptr<PooledBlockSlot>>`
(one node freed and re-allocated per block) to a flat `std::vector<OpenEntry>` reserved once,
where an entry is never erased: `flush` resets its `shared_ptr` and `openBlockFor` refills it,
so steady state performs no heap operation; the per-source block number moves into the same
entry, retiring the second map. **Raw presence** becomes a per-dataset rule,
`dataset_carries_raw(dataset)` in `Frame.h` (true when the dataset has transform code or is a
computed dataset), applied in `BlockStager::bindToFrame` to set `column.hasRaw` and in the
three builder writers to skip the `rawValue`/`rawNumericValue` copies for datasets that do not
carry raw. Every block consumer already has a `hasRaw == false` path because the stream lane
and replay synthesis never set it; the one consumer that would change its output, MDF4's
"(raw)" channel, gains a fallback to the final value so the file is unchanged.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Core/DataModel/Frame.h` | `[[nodiscard]] SS_FORCE_INLINE bool dataset_carries_raw(const Dataset&) noexcept` next to the in-place assign helpers: `!transformCode.isEmpty() \|\| virtual_`. |
| `core/Pipeline/DataModel/FrameBuilder/BlockStager.h` | `struct OpenEntry { int sourceId; std::shared_ptr<PooledBlockSlot> slot; quint64 blockNumber; }`; `std::vector<OpenEntry> m_open` replaces both maps; `findOpen(sourceId)` helper. |
| `core/Pipeline/DataModel/FrameBuilder/BlockStager.cpp` | ctor reserves `m_open` (16); `openBlockFor` / `flush` / `flushAll` / `blockNumber` on the vector; `bindToFrame` sets `column.hasRaw = dataset_carries_raw(dataset)`. |
| `core/Pipeline/DataModel/FrameBuilder.cpp` | `applyDatasetValueSpan` (span lane), `applyDatasetValue` (list lane) and the Quick Plot writer skip the raw copies when `!dataset_carries_raw(dataset)`. |
| `core/Storage/MDF4/Export.cpp` | Raw channel write: when `!column.hasRaw`, write the final value/text into the "(raw)" channel (channel creation unchanged). |
| `app/tests/tst_frame_builder_staging.cpp` | New cases: raw presence follows the transform rule; twenty flushes per source allocate nothing after the first (TU-local `operator new` counter; the stub host pre-reserves its vectors). |
| `app/tests/tst_data_block.cpp` | Case pinning `apply_block_sample` and `clone_block_trimmed` with `hasRaw == false` (fallback to final, no raw vectors copied). |
| `doc/claude/architecture/dataflow.md` | "FrameBuilder's Lane Sub-objects": the open-entry vector and the raw rule. |
| `tests/scripts/test_cpp_regressions.py` | Docstring only (the allocation counter now exists in the benchmark, spec 0084). |

Unchanged on purpose: `core/Storage/Sessions/Export.cpp` (writes a NULL raw blob when
`hasRaw` is false, and `BlockReader.h` already returns the final value in that case),
`core/Core/DataModel/DataBlock.h` (`size_block_storage` and `apply_block_sample` already handle
`hasRaw == false`), `BlockPublisher` and `clone_block_trimmed`, the API wire (`apply_block_sample`
fallback), the dashboard.

## Architecture & data flow

**Open entries.** `openBlockFor(sourceId)`:

```
entry = findOpen(sourceId)                 linear scan over m_open (sources are few)
if entry && entry.slot:                    held block
    reusable ? return : flush(sourceId)    (generation / mask rule unchanged)
slot = claimSlot(sourceId)                 pool probe, unchanged
if !entry: m_open.push_back({sourceId, {}, 0})   first sight of a source only
entry.slot = std::move(slot)               shared_ptr move: no control block, no node
```

`flush(sourceId)`: `auto slot = std::move(entry.slot)` (leaves the entry, keeps the source's
block number), then the existing publish through the aliasing `DataBlockPtr(slot, &slot->block)`.
`flushAll` walks the vector. `blockNumber(sourceId)` reads the entry. `releaseIdleStorage` is
unchanged (it walks the pool, not the open set).

**Raw rule.** `bindToFrame` runs once per (slot, generation, source) and sets per column
`hasText = true` (unchanged, spec 0055 D6) and `hasRaw = dataset_carries_raw(dataset)`.
`size_block_storage` then clears `rawValues`/`rawText` for those columns (capacity kept, no
allocation on a later rebind). `stage` is untouched: `write_block_raw` already returns early on
`!hasRaw`. The builder's three writers wrap their two raw stores in the same predicate, so a
dataset without raw never touches `dataset.rawValue`; nothing reads that field for such a
dataset (`reprocessDatasetValues` reads it only for datasets with transform code, the Painter
bridge and the API template read the dashboard-side copy that `apply_block_sample` fills from
the block with the fallback).

**Re-derivation on edit (R4).** A transform edit arrives as a project snapshot;
`applyProjectSnapshot` calls `invalidateFramePool()`, which bumps `m_framePoolGeneration`, the
reference `BlockStager` holds. The next `openBlockFor` sees `slot->generation != m_generation`,
flushes the held block first and rebinds, so no block mixes layouts. No new signal needed.

**MDF4.** `writeBlockSample` (the `column.hasRaw && target.rawChannel` branch) becomes
`if (target.rawChannel)` with the value source chosen by `hasRaw`. Channel creation stays
per dataset, so channel set, names and types are identical to today.

**Historian.** Unchanged: a transform-free dataset's `blocks` row now stores NULL in the raw
blob columns instead of a copy of the final blob; `BlockReader` already maps NULL to the final
value. The spec's AC5 is amended to "reads back identical" (the row is smaller, the readout is
the same). The Verifier hashes what is stored, on both write and verify, so archive integrity
is unaffected.

## Hotpath & threading impact

- **Touches the hotpath?** Yes: `BlockStager` (publish path) and `applyDatasetValueSpan` (span
  lane). Rules preserved:
  - pipeline-thread only, no mutex or atomic added; `m_open` is touched only from `stage`,
    `flush`, `flushAll`, all pipeline-thread by contract;
  - no allocation in steady state: vector reserved once, entries never erased, `shared_ptr`
    moves only; the first block of a never-seen source may `push_back` once (spec R1 allows it);
  - in-place string writes stay in place: skipping `assign_string_in_place(dataset.rawValue, ...)`
    removes a write, it never share-assigns; `dataset.value` is untouched;
  - `structureGeneration` stamping, the `use_count()==1` probe, the mask rule and the epoch
    flush are unchanged;
  - column order stays export-schema order; consumers index positionally.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No. The raw rule is evaluated at bind time from the
  frame snapshot, not per frame.
- **Timestamp ownership** — unchanged.
- **Benchmark plan.** Before/after `--benchmark-hotpath` on the same machine: Native numeric
  and mixed FPS, the `datasets+publish` stage figure, and (spec 0084) `native(numeric)`
  allocations per frame going from ≈0.03 to 0.

## Data model & persistence

No `Keys::` change, no schema bump. Historian `blocks` rows for transform-free datasets carry
NULL raw blobs, which the reader already accepts (spec-0055 files with NULL raw exist today for
stream-lane sources). MDF4 files are byte-identical for transform-free projects.

## API / SDK surface

None. The API frame is built through `apply_block_sample`, which already substitutes the final
value when `hasRaw` is false; the mirror fixtures do not carry raw values (verified at tasks time
by grepping `tests/fixtures/mirror/` for `rawValue`).

## QML / UI

None. Painter's `rawValue`/`rawText` getters read the dashboard's dataset copy, which
`apply_block_sample` fills with the fallback.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Open-block container | (a) flat vector, entries kept; (b) `std::map` with `extract`/re-insert node reuse; (c) `std::unordered_map` with reserve | **(a)**. Zero steady-state heap work, trivially auditable, and `flushAll` becomes a linear walk. (b) keeps a tree and its per-lookup pointer chase. (c) still allocates on `emplace`. |
| Raw rule condition | transform code only vs transform code or computed dataset | **Both** (spec open question, recommended answer). A computed dataset's final value is script-produced, so the parse-vs-produced classification keeps its meaning. |
| MDF4 raw channel for datasets without raw | (a) keep channel, write final; (b) stop creating the channel | **(a)**. File identical to today (spec R3, AC4); (b) changes the channel set of every recording. |
| Historian raw blob for datasets without raw | (a) NULL, reader falls back; (b) keep writing a copy | **(a)**. Reader already handles NULL; smaller rows; spec AC5 reworded to "reads back". |
| Where the raw rule lives | `Frame.h` inline vs a `BlockStager` private | **`Frame.h`**. `BlockStager` and `FrameBuilder` both need it; one definition or the two drift. |
| Zero-allocation proof in ctest | TU-local `operator new` counter vs mimalloc stats | **`operator new` counter**. The unit tier does not link mimalloc; `std::vector`/`std::map` nodes go through `operator new`, so the counter sees exactly the churn being removed. |

## Risks & mitigations

- **A reader of `dataset.rawValue` this plan missed.** Grep found four: `BlockStager::stage`
  (guarded by `hasRaw`), `reprocessDatasetValues` (transform datasets only), Painter bridge
  and API template (dashboard-side copies). Tasks phase re-greps `rawValue\b|rawNumericValue`
  across `core/` and `app/` before the writers change.
- **Stub host allocates in the zero-allocation test.** `StubStagerHost::publishStagedBlock`
  pushes into a vector; the test pre-reserves it (and the announce vector) before arming the
  counter, and the counter is armed only around the twenty flushes.
- **First-source `push_back`.** Counted once, before steady state; the test's baseline block
  absorbs it.
- **Silent-breakage classes exposed** (`common-mistakes.md`): the
  "share-assign re-links buffers" row (we remove a write, never add a share-assign); the
  "`structureGeneration` stamping" row (untouched code); "per-frame `std::map::find`" row
  (this change removes one).
- **`refreshBudget` formula.** It budgets two doubles and two strings per sample; with raw
  absent the real footprint is smaller. Left conservative on purpose; noted, not changed.
- **MDF4 file size (review note, 2026-09-12).** A "(raw)" channel is still created for every
  dataset, so a plain dataset now writes its final value into both channels. File size and
  the per-sample cost are what they were before the spec; gating channel creation on the raw
  rule would change the channel set and is deliberately out of scope (spec AC4).
- **Zero-allocation proof scope (review note, 2026-09-12).** A TU-local `operator new` sees
  std container traffic only; QString buffers go through `malloc`. The staging test therefore
  pins the string side by asserting every published block's text buffers are the same two
  recycled slots' buffers, and releases published blocks so the slots actually recycle.

## Test & verification plan

- **Unit (maintainer builds, `ctest` against the build dir):**
  - AC3: `tst_frame_builder_staging` — `rawPresenceFollowsTheTransformRule` (two sources, one
    dataset with transform code, one computed, others plain: `hasRaw` true for the first two
    only; `rawValues.size()` is the cap for those and 0 for the rest) and
    `steadyStateFlushesDoNotAllocate` (stage 20 × 64 rows per source with the `operator new`
    counter armed after the first block; expect 0).
  - `tst_data_block` — `hasRaw == false` fallback in `apply_block_sample` and
    `clone_block_trimmed` copies no raw vectors.
- **Integration (maintainer runs, app up with API server):**
  - AC4: `pytest tests/integration/test_export_replay_fidelity.py` and the CSV/MDF4 export
    tests unchanged; plus a manual MDF4 recording of a transform-free example project,
    compared channel-by-channel with a pre-change recording (channel list, types, sample
    values).
  - AC5: Historian session with one transformed and one plain dataset; read back through
    the `sessions.*` API, raw equals final for the plain dataset, raw ≠ final for the
    transformed one.
  - AC6: new `tests/integration/test_block_raw_follows_transform.py`: subscribe to the block
    stream, add a transform via `project.*`, remove it, assert block numbers stay continuous
    and the raw field appears/disappears with the transform.
- **Hotpath:** `--benchmark-hotpath` before/after (AC1, AC2): Native rows at zero
  allocations per frame (spec 0084 column), `datasets+publish` down by the target recorded
  in tasks after the first measurement (spec says ≥ 8%).
- **Static:** `python scripts/code-verify.py --check` on every touched file (hotpath TUs:
  violations block); `qt-cpp-review` on `BlockStager.*` and the `FrameBuilder.cpp` hunks;
  `python scripts/sanitize-commit.py` before commit.
