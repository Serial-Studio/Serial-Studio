---
spec: 0057-cascading-envelope-rings
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-16 (unattended; maintainer re-reviews)
updated: 2026-08-16
---

# Plan 0057 — Cascading envelope rings for plot history

> **Phase 2 of 4 — the HOW.** Gate auto-approved for the unattended overnight run of
> 2026-08-16. The maintainer's morning review is the real gate; everything below is
> reversible by leaving the working tree uncommitted.

## Approach (one paragraph)

Add `DSP::EnvelopeRing` to `app/src/DSP.h`: a struct that *contains* today's `TimeRing` as
level 0 (unchanged behaviour, unchanged sizing) plus a `std::vector<EnvelopeLevel>` of coarse
levels, each a `FixedQueue<EnvelopeCell>` of time-ordered extreme pairs whose cells span `16^k`
level-0 grid cells. `append(t, v)` runs level 0's own sanitisation, detects "this sample opens a
new level-0 cell" with a one-line predicate added to `TimeRing`, folds the just-completed level-0
cell (level 0's `acc*` accumulators, which are exactly that cell's extremes) into every coarser
level's open cell, then calls `appendDecimated` as before. Cell identity is the integer level-0
cell index (`floor(t / interval)`) shifted right by `4k`, so grids nest exactly and never depend
on floating-point boundary rounding. `Dashboard` swaps its two ring maps to `EnvelopeRing`; the
sweep engines keep plain `TimeRing`s. `Plot`/`MultiPlot` call a new `downsampleTimeWindow`
overload that takes the whole ring, selects the coarsest level whose cell span is at or under one
pixel of time and that still covers the requested span, and feeds the *same* per-column
accumulation with either level 0's slots or the coarse level's `2 * cells` extreme points, rebased
to level 0's true newest sample. Level 0's grow-on-saturation path stays: with the render cost
decoupled from level-0 density it is now purely a memory-sizing policy, and changing memory policy
unattended is the wrong call.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DSP.h` | `TimeRing::opensCell()` predicate (used by `appendDecimated`); new `EnvelopeCell`, `EnvelopeLevel`, `EnvelopeRing`; `dsTimeWindowCore()` extracted from `downsampleTimeWindow`; new `downsampleTimeWindow(const EnvelopeRing&, ...)` overload |
| `app/src/UI/Dashboard.h` | `m_plotTimeRings` / `m_multiplotTimeRings` / snapshot-restore / `growTimeRing` / getters typed `EnvelopeRing` |
| `app/src/UI/Dashboard.cpp` | `makeHistoryRing` returns `EnvelopeRing`; every `appendDecimated` on a history ring becomes `append`; snapshot/restore/replay/grow/clear read `level0` |
| `app/src/UI/Widgets/Plot.cpp` | time-axis draw calls the ring overload |
| `app/src/UI/Widgets/MultiPlot.cpp` | same |
| `app/src/API/Handlers/DashboardHandler.cpp` | `tailFrames` reads `ring.level0.time/value` |
| `app/tests/tst_envelope_ring.cpp` | new ctest suite (AC1-AC4) |
| `app/tests/CMakeLists.txt` | register `tst_envelope_ring` |
| `doc/claude/architecture/dashboard.md` | TimeRing paragraph gains the pyramid (docs-only, same lane) |

## Architecture & data flow

Unchanged upstream of `Dashboard`: blocks arrive on the display tick (`applyBlock`), the frame
lane feeds `updateLineSeries` / `feedMultiRings` per irregular sample, the stream lane feeds
`applyBlockColumn` per uniform-grid column, replay seek feeds `fillSeekPlot*`, and every one of
those calls `ring.append(t, v)` where it called `ring.appendDecimated(t, v)`. Reads:
`Plot::updateData` / `MultiPlot::updateData` (60 Hz draw) call
`DSP::downsampleTimeWindow(ring, xLo, xHi, w, h, out, ws)`; the API's `dashboard.tailFrames`
keeps reading level 0. All of it is GUI thread, single writer, no mutex — the same discipline as
today's `TimeRing`.

Level bookkeeping inside `EnvelopeRing`:

- `level0` — the existing `TimeRing` (slots, `interval`, `nextEmit`, `acc*`, `cellSlots`).
- `levels[k-1]` — level k: `FixedQueue<EnvelopeCell> cells` (capacity
  `ceil((C0/2) / 16^k) + 1`, built while that is >= 3, at most 9 coarse levels), `shift = 4k`,
  `openIndex` (level-k index of the open cell, the ring's back).
- `openCell` / `openCellValid` — the level-0 index of the currently open level-0 cell.
- Fold: `for each level: idx = openCell >> shift; idx == openIndex ? merge min/max into back :
  push new cell`. Cells are stored time-ordered `{t0, v0, t1, v1}` so a coarse level reads as a
  monotonic `(time, value)` sequence of length `2 * cells` — which is exactly the shape the
  existing column accumulator consumes.
- Selection: `pixelSec = span / pixels`; walk levels while `interval * 16^k <= pixelSec`, the
  level is non-empty, and (if the level has wrapped) its front cell starts no later than the
  oldest time the reader needs; the last qualifying level wins, level 0 by default.
- Rebuild (only from `resizeCapacity`, i.e. the display-tick growth path or a time-range
  change): levels re-sized from the new C0 and refilled by folding level 0's retained slots as
  one-sample cells. O(C0 * K), rare by construction.

## Hotpath & threading impact

- **Touches the hotpath?** No pipeline-thread code changes. `Dashboard` ingest (GUI thread,
  display tick) gains one predicate + one predicted-not-taken branch per appended sample and a
  bounded (<= 9 iterations) fold on the rare level-0 cell completion. No allocation on append
  (levels are sized at construction / rebuild). No `Frame` copy. `--benchmark-hotpath` gated
  tiers are unaffected by construction (they run no dashboard); the ungated `lua+dashboard`
  readout may move within noise.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; the ring consumes the display-clock time it consumed
  before.

## Data model & persistence

None. Rings are runtime-only. Snapshot/restore across layout rebuilds keeps its contract:
same level-0 capacity + interval moves the whole pyramid, otherwise the kept level-0 slots
replay through `append` (which rebuilds the coarse levels as a side effect).

## API / SDK surface

None. `dashboard.tailFrames` keeps returning level-0 samples.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the pyramid lives | (a) inside `TimeRing`; (b) new struct containing a `TimeRing`; (c) parallel maps in Dashboard | (b): level 0 stays byte-for-byte the current ring (sweep engines keep using it unchanged), the pyramid is opt-in per use site, and one type owns the invariants. |
| Fold topology | (a) completed level-k cell folds into level k+1 only; (b) completed level-0 cell folds into every coarser open cell | (b): (a) makes level k lag by up to a level-(k-1) span (seconds at level 5), so the newest data would be missing from a coarse render; (b) keeps every level current to within one level-0 cell at the cost of a bounded <= 9-way loop per level-0 completion. Same content, same amortised O(1). |
| Cell identity | (a) floating cell start times; (b) integer level-0 cell index >> 4k | (b): exact nesting, exact wraparound consistency, no boundary rounding — and testable with integer arithmetic. |
| Coarse level sizing | (a) `C0 / 16^k` (covers a sparse level 0 completely, 1.13x); (b) `(C0/2) / 16^k + 1` (covers the saturated grid, 1.067x) | (b): the visible axis is `T` while the ring window is `1.25 T`; a sparse level 0 makes coarse levels span the window in wall time anyway (cells are created only when data arrives), and the reader's coverage check falls to a finer level whenever a coarse one falls short. |
| `growTimeRing` / `resizeCapacity` | (a) delete; (b) keep for level 0 only; (c) replace with a fixed byte budget per curve | (b): the render-cost reason for guessing is gone, the memory reason is not, and changing memory policy unattended is out of lane. (c) recorded as the follow-up (spec.md open questions). |
| Newest-time rebase for coarse reads | (a) rebase to the coarse level's own last time; (b) rebase to level 0's newest sample | (b): the axis' zero is "now"; a coarse level's last extreme lags the newest sample by up to one level-0 cell (sub-pixel by construction of the selection rule), so (a) would shift the trace right by that lag. |

## Risks & mitigations

- **Coarse level lags at the right edge** by less than one level-0 cell (< 1/16 px by the
  selection rule). Accepted; documented in the header.
- **Snapshot copies share storage** (`FixedQueue` copies alias their array — the same property
  the current code relies on). `EnvelopeRing` is a plain aggregate of such queues, so copy/move
  semantics are identical to today's `TimeRing`; the restore path checks `level0.time.raw()`.
- **`bulkLoadPlotWindow` normalises times to end at 0**, so cell indices go negative; `>>` on a
  negative `std::int64_t` is arithmetic (floor) in C++20, which is the nesting we want.
- **A ring at the empty-static fallback** (`kEmpty`, capacity 1) builds no coarse levels and
  reads as level 0 — same as today.
- **Silent-breakage class**: none of the cached hotpath flags or push tables change; the ring is
  addressed by widget index at use time exactly as before.

## Test & verification plan

- **Unit (maintainer builds, cannot be run tonight):** `app/tests/tst_envelope_ring.cpp` — AC1
  brute-force min/max per level on a ramp with a saturated grid; AC2 level selection for a
  seconds-per-pixel sweep, narrow-window fallback, coverage fallback; AC3 wraparound
  consistency at every level; AC4 NaN/inf rejection. Registered in `app/tests/CMakeLists.txt`.
- **Integration (maintainer):** none new; `tests/integration/` dashboard tests exercise
  `dashboard.tailFrames`, whose output is unchanged.
- **Hotpath:** `--benchmark-hotpath` — gated tiers must be unchanged; report the
  `lua+dashboard` line.
- **Static:** `python3 scripts/code-verify.py --check` on every touched file.
- **Behavioural (maintainer):** spec AC5.
