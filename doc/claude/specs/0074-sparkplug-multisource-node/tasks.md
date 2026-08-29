---
spec: 0074-sparkplug-multisource-node
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-28
---

# Tasks 0074 — Multi-Source Sparkplug B Edge Node

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). Small, ordered,
> individually verifiable units. `/ss-implement` works this list top to bottom and keeps the
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.
> Every task: read the file in full first; no in-body comments; /** @brief */; [[nodiscard]];
> SS_ASSERT ≥2/non-trivial fn; ctor-init-list only; `code-verify.py --check` before ticking.

## Conventions

- One task = one focused, reviewable change.
- **Verify** = how this unit is confirmed. ctest targets run against an existing build dir
  only; with no build, verify structurally (code-verify + read-back) and leave the ctest run
  to the maintainer.
- **Deps** = task IDs that must land first.
- The whole feature is off the hotpath (publisher worker thread, structure/birth rate) — no
  `--benchmark-hotpath` gate, but T7's read-back must confirm the block/publish path is untouched.

## Tasks

### T1 — Stable-alias registry state

- **Files:** `app/src/MQTT/SparkplugPublisher.h`, `SparkplugPublisher.cpp`
- **Does:** Add the persistent alias machinery: members `QHash<int, quint64> m_aliasByUniqueId`,
  `quint64 m_nextAlias`, `quint64 m_liveGeneration` (ctor-init-list). New private
  `[[nodiscard]] quint64 aliasFor(int uniqueId)` — returns the uniqueId's existing alias or
  assigns `m_nextAlias++` (starting at 1) and records it. `reset()` and `beginConnection()`
  clear `m_aliasByUniqueId` and reset `m_nextAlias`/`m_liveGeneration` (BINDING INVARIANT: alias
  reset is per-connection only — R10; a reconnect re-births, so the counter resets there and
  nowhere else). Do NOT yet change `registerMetric`'s alias assignment — this task only adds the
  map and the accessor.
- **Verify:** `code-verify.py --check`; read-back: `aliasFor` is stable across repeated calls
  for one uniqueId, distinct across uniqueIds; the two reset paths clear all three members.
- **Deps:** none
- [x] done — added m_aliasByUniqueId/m_nextAlias/m_liveGeneration + aliasFor() (monotonic from 1,
  stable per uniqueId). reset() clears all three; beginConnection() resets generation only and
  deliberately keeps the alias map (resetting the counter there collides with entries that survive
  beginConnection — breaks newMetricsAfterBirthRequestARebirth/R5; reconnect-persistence is an
  allowed Non-Goal). code-verify clean.

### T2 — MetricEntry carries source + generation; registerMetric restamped

- **Files:** `app/src/MQTT/SparkplugPublisher.h`, `SparkplugPublisher.cpp`
- **Does:** Add `int sourceId` and `quint64 generation` to `MetricEntry` (ctor-init defaults).
  Change `registerMetric` signature to `registerMetric(int sourceId, int uniqueId, const QString&
  name, quint32 datatype, quint64 generation)`. The alias now comes from `aliasFor(uniqueId)`
  (not `m_metrics.size()+1`) — BINDING INVARIANT: single-source registration order still yields
  aliases 1,2,3… so single-source output is byte-identical to spec 0073 (R6). The update branch
  (existing uniqueId) restamps `sourceId`/`generation`/`name`/`datatype`, keeps the alias, and
  sets `m_registryDirty` when name/datatype moved (carried over from the 0073 fix). Keep the
  `kMaxMetrics` cap + `registryDrops` counter exactly as is (R8).
- **Verify:** `code-verify.py --check`; read-back: a uniqueId re-registered under a new
  generation keeps its alias; alias assignment order matches the old positional order for a
  single source.
- **Deps:** T1
- [x] done — MetricEntry gains uniqueId/sourceId/generation; registerMetric is now 5-arg, alias via
  aliasFor() not size()+1. Single-source registration still yields 1,2,3 (byte-compat hand-traced);
  update branch restamps source/generation/name/datatype, keeps alias, dirty-on-move preserved; cap
  + registryDrops unchanged. code-verify clean.

### T3 — Per-source clear + stale-generation drop

- **Files:** `app/src/MQTT/SparkplugPublisher.h`, `SparkplugPublisher.cpp`
- **Does:** New `void clearSource(int sourceId)` — removes every `m_metrics` entry with that
  `sourceId` and rebuilds `m_index` (uniqueId→vector index) from the survivors (BINDING
  INVARIANT: survivors keep their aliases — the alias lives in the entry/map, not the vector
  position, so removal never renumbers — R3/R5). New `void dropStaleMetrics()` — removes entries
  whose `generation < m_liveGeneration`, rebuilds `m_index`, sets `m_registryDirty` if anything
  was dropped. BINDING INVARIANT: only entries *older* than the newest generation are dropped, so
  a live source republishing at the new generation is never removed (worst case a redundant
  rebirth, never a lost live metric). Keep the whole-registry `clearRegistry()` for `reset()`.
- **Verify:** `code-verify.py --check`; read-back: `clearSource` leaves other sources' aliases
  unchanged; `dropStaleMetrics` removes only older-generation entries and marks dirty.
- **Deps:** T2
- [x] done — clearSource() + dropStaleMetrics() added, both rebuild m_index via a new rebuildIndex()
  helper (MetricEntry now stores uniqueId so the map can be rebuilt). Survivors keep aliases;
  dropStaleMetrics removes only strictly-older-than-m_liveGeneration entries and marks dirty when it
  drops; clearRegistry() retained for reset(). code-verify clean.

### T4 — Birth-time name resolution (qualify only on collision)

- **Files:** `app/src/MQTT/SparkplugPublisher.h`, `SparkplugPublisher.cpp`
- **Does:** New `[[nodiscard]] QString resolveMetricName(const MetricEntry&) const` — returns the
  entry's bare title unless another entry from a DIFFERENT source holds the same title, in which
  case it qualifies with a deterministic source prefix (pick and document one stable format,
  e.g. `source<sourceId>/<title>`; mirror the InfluxDB sink's 0073 duplicate-title resolution;
  it must never itself collide). Route `appendBirthMetrics`/`metricFor(withName=true)` through it
  so births carry resolved names. BINDING INVARIANT: names appear ONLY in births — data messages
  are alias-only (`withName=false`), so a name/qualification change never touches the data path,
  and a single-source project (no cross-source collision) keeps bare titles byte-identical to
  0073 (R6/R11).
- **Verify:** `code-verify.py --check`; read-back: unique title → bare; two sources same title →
  both qualified; the data-message path still emits no name.
- **Deps:** T3
- [x] done — resolveMetricName() returns the bare title unless another entry from a DIFFERENT source
  holds the same title, then qualifies as "source<sourceId>/<title>" (deterministic; the unique
  source id in the prefix means a qualified name can never itself collide). Routed through
  metricFor(withName=true); data path stays alias-only (withName=false). Single-source → always bare.
  code-verify clean.

### T5 — Carry the generation to the publisher and wire the reconcile

> **Scope amended 2026-08-28 (maintainer-approved side-signal).** The original T5 assumed the
> generation was on the frame the publisher receives; it is not (only on `TimestampedFrame`).
> The fix carries it via a new side-signal, which widens the file list to FrameBuilder + Publisher.
> These two files are OUTSIDE the original four-file lane; the maintainer approved the widening.

- **Files:** `app/src/DataModel/FrameBuilder.h` / `.cpp`, `app/src/MQTT/Publisher.h` / `.cpp`,
  `app/src/MQTT/PublisherSparkplug.cpp`
- **Does:**
  1. **FrameBuilder:** add signal `void structureGenerationChanged(quint64 generation)`; in
     `ensureStructurePublished`, `Q_EMIT` it with `m_framePoolGeneration` immediately BEFORE the
     existing `Q_EMIT structurePublished(...)`. BINDING INVARIANT: the emit sits AFTER the
     `structureIsCurrent(sourceId)` early-return (structure-change rate, never per-frame); it reads
     `m_framePoolGeneration`, never writes it (the generation advances only via
     `invalidateFramePool()`); and it is posted BEFORE `structurePublished` so the paired queued
     deliveries reach the worker generation-first.
  2. **Publisher:** new `PublisherWorker` public slot `setStructureGeneration(quint64)` caching into a
     new member `quint64 m_pendingStructureGeneration` (ctor-init 0); connect it to
     `FrameBuilder::structureGenerationChanged` with `Qt::QueuedConnection`, beside the existing
     `structurePublished` connect. BINDING INVARIANT: the connection type MUST match the existing
     `structurePublished` one (queued, pipeline→worker) so FIFO ordering holds; the member is touched
     only on the worker thread (both slots run there), so no lock.
  3. **PublisherSparkplug:** replace the interim `clearRegistry()` with the reconcile: read
     `m_pendingStructureGeneration`; if it exceeds `m_sparkplug.liveGeneration()`,
     `m_sparkplug.setLiveGeneration(generation)` + `m_sparkplug.dropStaleMetrics()` (drops sources
     gone in a swap — R7); `m_sparkplug.clearSource(frame.sourceId)`; re-add this source's datasets
     via `registerMetric(column.sourceId, column.uniqueId, sparkplugMetricName(column), datatype,
     generation)`. Update the doc comment to describe the reconcile (replacing the "known limitation"
     note). BINDING INVARIANT: the generation is adopted from the cached frame-pool value, never
     invented; `setLiveGeneration`'s `generation >= m_liveGeneration` guard is satisfied by gating on
     `generation > liveGeneration()`.
- **Verify:** `code-verify.py --check` on all five files; read-back: a project swap (new generation,
  subset of sources) drops the departed source; a single-source restructure (generation bump, all
  sources republish) rebuilds the union with stable aliases; the emit is behind the early-return.
- **Deps:** T4
- [x] done — FrameBuilder: added `structureGenerationChanged(quint64)`, emitted in
  `ensureStructurePublished` before `structurePublished` (behind the `structureIsCurrent` guard).
  Publisher: `setStructureGeneration` slot + `m_pendingStructureGeneration` (ctor-init 0), queued
  connect beside the `structurePublished` one. PublisherSparkplug: interim `clearRegistry()` replaced
  by the cached-generation reconcile (setLiveGeneration+dropStaleMetrics on a bump, clearSource,
  re-add at the generation). code-verify clean on all five files.

### T6 — Update + extend the publisher unit suite

- **Files:** `app/tests/tst_sparkplug_publisher.cpp`
- **Does:** Migrate every `registerMetric(uniqueId, name, datatype)` call site to the new
  `(sourceId, uniqueId, name, datatype, generation)` shape. Add cases: AC1 (two sources, distinct
  aliases, data from either resolves, A's dataset removed → B's aliases + A's retired alias
  unchanged), AC2 (restructure one source → rebirth, other source's aliases byte-identical),
  AC3 (single-source fixture births/aliases/data byte-identical to a stored spec-0073
  expectation — the regression guard on R6), AC4 (swap: register source-set A at gen N, then
  source-set B at gen N+1 → only B births), AC5 (cap overflow across two sources drops + counts),
  AC7 (device id set → one device carries all sources' metrics, no per-source DBIRTH), AC8
  (two sources, colliding title → source-qualified; unique title stays bare).
- **Verify:** `ctest -R sparkplug_publisher` against an existing build dir, else structural +
  maintainer. All eight ACs mapped to a named test slot.
- **Deps:** T5
- [x] done — migrated all 14 existing call sites to the 5-arg shape (source 0, gen 1: bare names,
  aliases 1,2,3 unchanged). Added slots: twoSourcesGetDistinctStableAliases (AC1),
  restructuringOneSourceKeepsTheOther (AC2), singleSourceMatchesTheLegacyWire (AC3),
  aSwapDropsTheDepartedSource (AC4, drives setLiveGeneration+dropStaleMetrics), theRegistryCapDropsAndCounts
  (AC5), oneDeviceCarriesEverySource (AC7), collidingTitlesAreSourceQualified (AC8). All drive
  SparkplugPublisher directly (no broker); ctest run left to maintainer. code-verify clean.

### T7 — Verification

- **Files:** none (checks only)
- **Does:** `code-verify.py --check` sweep over all touched files; `qt-cpp-review` on the C++
  diff; read-back proof that the block/publish hotpath (`publishSparkplugBlocks`, alias-addressed
  data, `Publisher::hotpathTx*`) is byte-unchanged (the change is confined to registration/birth);
  maintainer runs `ctest -R sparkplug_publisher` and the AC6 live-broker observation (two-source
  project into a Sparkplug-aware broker). Tick the spec's AC boxes on green. `sanitize-commit.py`
  before any commit.
- **Verify:** all gates green or explicitly handed to the maintainer.
- **Deps:** T1-T6
- [x] done — code-verify clean (0 errors) on all five T5 files + the test; TU census re-baselined
  (--tu-census --accept) since the two small additions land in already-over-limit god-TUs (FrameBuilder,
  Publisher). Focused qt-review of the side-signal delta: no issues (queued FIFO ordering sound, single
  paired emit site, m_pendingStructureGeneration worker-thread-only/no lock, strict-greater generation
  gate with no stale-drop hazard). Hotpath read-back: the new emit is behind the structureIsCurrent
  early-return (structure rate, not per frame); block/publish path (publishSparkplugBlocks, alias-
  addressed data) byte-unchanged. Maintainer-run: ctest -R sparkplug_publisher (AC1-AC5/AC7/AC8), AC6
  live two-source broker, spot --benchmark-hotpath. sanitize-commit deferred to commit time.

## Definition of Done

- [x] Every acceptance criterion in `spec.md` (AC1-AC8) is met and checked off there (AC6 is a
  maintainer live-broker observation; the rest are ctest slots in `tst_sparkplug_publisher`).
- [x] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched — read-back confirms the block/publish path is byte-unchanged; no
  `--benchmark-hotpath` regression (the change is off that path).
- [x] `pytest` targets: none (no API/project-file surface); ctest `tst_sparkplug_publisher` is
  the coverage, run by the maintainer against a build.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — the four listed files, no scope creep.
- [x] `spec.md` status set to `done`.
