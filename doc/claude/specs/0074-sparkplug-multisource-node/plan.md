---
spec: 0074-sparkplug-multisource-node
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-28
---

# Plan 0074 — Multi-Source Sparkplug B Edge Node

> **Phase 2 of 4 — the HOW.** Satisfies every requirement in [`spec.md`](./spec.md). Grounded
> in the real `SparkplugPublisher` registry, the `PublisherWorker` structure wiring, and the
> `FrameBuilder` generation stamp — not from memory.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The registry's positional alias (`alias = m_metrics.size() + 1`) is the whole problem: it
renumbers survivors when any entry is removed. Replace it with a **persistent
uniqueId→alias map** that assigns each metric a monotonic alias once per connection and never
moves or reuses it (R5, R10). Give each `MetricEntry` a `sourceId` and the frame-pool
`structureGeneration`, so a source's structure republish clears and re-adds only its own
metrics (R3), and a stale source left behind by a project swap is dropped at birth time
because its entries still carry an older generation than any live source republished at the
new one (R7). **Plan-correction (2026-08-28):** the generation is NOT on the inner
`DataModel::Frame` the worker receives (it lives only on `TimestampedFrame`), so a small
side-signal `structureGenerationChanged(quint64)` — emitted from `ensureStructurePublished`
right before `structurePublished`, at structure-change rate — carries it to the publisher,
which caches it and pairs it with the very next `setTemplateFrame`. One new signal on one
existing emit site, no project-reset hook, and the seven `structurePublished` consumers stay
untouched. Metric **names** stay bare and are qualified with a source
prefix only when two sources collide, resolved at birth-build time (names appear only in
births; data is alias-addressed), which keeps single-source output byte-identical to spec
0073 (R6, R11). Everything happens at structure-change rate on the publisher worker thread;
the block/publish hotpath is untouched.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/MQTT/SparkplugPublisher.h` | `MetricEntry` gains `int sourceId` and `quint64 generation`. New members: `QHash<int, quint64> m_aliasByUniqueId` (persistent alias assignments), `quint64 m_nextAlias`, `quint64 m_liveGeneration`. `registerMetric` signature gains a leading `int sourceId` and a `quint64 generation`. New private `[[nodiscard]] quint64 aliasFor(int uniqueId)` and `void dropStaleMetrics()` / `void clearSource(int sourceId)`. `birthMessages`/`appendBirthMetrics` resolve display names with a new `resolveMetricName(const MetricEntry&) const`. |
| `app/src/MQTT/SparkplugPublisher.cpp` | Alias assignment moves from positional to `aliasFor()` (map lookup, assign `m_nextAlias++` on first sight). `registerMetric` stamps `sourceId`/`generation`, updates in place by uniqueId. `clearSource` removes one source's entries and rebuilds `m_index`. `dropStaleMetrics` removes entries whose `generation < m_liveGeneration` and rebuilds `m_index`, marking dirty. `reset()`/`beginConnection` clear `m_aliasByUniqueId` and reset `m_nextAlias`/`m_liveGeneration` (per-connection alias reset, R10). Name collision resolved in `resolveMetricName` (bare title unless another source holds the same title). |
| `app/src/DataModel/FrameBuilder.h` / `.cpp` | **(Plan-correction, 2026-08-28.)** New signal `structureGenerationChanged(quint64 generation)`, emitted in `ensureStructurePublished` immediately before `structurePublished`, at structure-change rate only (behind the same `structureIsCurrent` early-return). This is the side-signal that carries `m_framePoolGeneration` to the publisher; the inner `DataModel::Frame` does NOT carry the generation (it lives only on `TimestampedFrame`, Frame.h:1522), so the reconcile cannot read it off the frame. No change to the `structurePublished` signal itself, so its seven other consumers are untouched. |
| `app/src/MQTT/PublisherSparkplug.cpp` | `registerSparkplugMetrics(frame)`: read the worker's cached generation (`m_pendingStructureGeneration`); if it exceeds `m_sparkplug.liveGeneration()`, `setLiveGeneration()` + `dropStaleMetrics()` (drops sources gone in a swap); `clearSource(frame.sourceId)`; re-add this source's datasets with `(frame.sourceId, uniqueId, name, datatype, generation)`. Replaces the interim whole-registry `clearRegistry()`. |
| `app/src/MQTT/Publisher.h` / `.cpp` | New `PublisherWorker` slot `setStructureGeneration(quint64)` caching into a new member `quint64 m_pendingStructureGeneration` (ctor-init 0), connected to `FrameBuilder::structureGenerationChanged` with `Qt::QueuedConnection` beside the existing `structurePublished` connect. Both are queued from the pipeline thread to the same worker object, so Qt delivers them FIFO — the generation is cached before `setTemplateFrame` runs the reconcile. Worker-thread-local; no lock. |
| `app/tests/tst_sparkplug_publisher.cpp` | Update the `registerMetric(uniqueId, …)` call sites to the new `(sourceId, uniqueId, …)` shape; add the multi-source, per-source-restructure, stable-alias, swap, cap-overflow, and name-collision cases (AC1-AC5, AC7-AC8); keep/adapt the single-source cases as the AC3/AC6 byte-compat guard. |

Grep-confirmed: `structurePublished` is emitted only from `FrameBuilder.cpp:389` with the inner
`DataModel::Frame` (which carries `sourceId`, Frame.h:900, but NOT `structureGeneration` —
that field is on `TimestampedFrame`, Frame.h:1522); it has seven `setTemplateFrame` consumers
(MDF4, InfluxDB, Sessions, CSV, API Server, gRPC, MQTT), so the generation rides a NEW
side-signal rather than a widened `structurePublished`. `registerSparkplugMetrics` is called
only from `PublisherWorker::setTemplateFrame` (Publisher.cpp:341); `clearRegistry()` keeps its
`reset()` caller.

## Architecture & data flow

`FrameBuilder::invalidateFramePool()` bumps `m_framePoolGeneration` on every structural
change; each live source republishes structure at the new generation via
`structureGenerationChanged` then `structurePublished` (both queued to the worker thread, in
that order). The worker caches the generation from the first, then `setTemplateFrame` binds
the per-source frame template and calls `registerSparkplugMetrics(frame)`, which now:

1. reads the cached generation; a bump means a wholesale structural change, so it drops
   every registry entry still stamped at an older generation (`dropStaleMetrics`) — that is
   exactly the set belonging to sources that did not republish (a swap's departed sources);
2. clears only `frame.sourceId`'s entries (`clearSource`) and re-adds that source's current
   datasets, each stamped `(sourceId, generation)`, its alias fetched from the persistent
   `m_aliasByUniqueId` (assigned once, stable);
3. marks the registry dirty, so the existing `needsRebirth()` → `commitBirth()` path
   re-declares the current union on the next tick (R4).

Births walk the union and emit each metric's resolved name (bare, or source-qualified on
collision) plus its stable alias. Data messages are alias-only, so a name change or a
re-qualification never touches the data path — only the next birth. Aliases live for the
connection; `beginConnection` resets the map so a reconnect re-births cleanly (R10).

## Hotpath & threading impact

- **Touches the hotpath?** No. All of this runs in `registerSparkplugMetrics` /
  `birthMessages` / `dropStaleMetrics` on the **publisher worker thread**, at
  structure-change and birth rate — never per block, never per frame. The block publish path
  (`publishSparkplugBlocks`, alias-addressed data) is unchanged. The new
  `structureGenerationChanged` emit sits behind the same `structureIsCurrent` early-return that
  already gates `structurePublished`, so it fires at structure-change rate, never per frame.
  `--benchmark-hotpath` is not exercised by this change and must stay flat.
- **New cross-thread signal/slot?** One: `FrameBuilder::structureGenerationChanged(quint64)` →
  `PublisherWorker::setStructureGeneration`, `Qt::QueuedConnection`, pipeline→worker, at
  structure-change rate. It rides beside the existing `structurePublished` connect and is
  posted first, so Qt's FIFO queued delivery caches the generation before the paired
  `setTemplateFrame` reconcile. NOT a pipeline→pipeline hop, so the DirectConnection rule does
  not apply; NOT a per-frame emission.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; births/data stamp from the block/source time as in
  spec 0073.

## Data model & persistence

No project-file change, no `Keys::` addition, no schema bump — the multi-source behavior is
pure runtime state in `SparkplugPublisher`. `MetricEntry` grows two fields (`sourceId`,
`generation`) and the class grows one map plus two counters, all worker-thread-local. Nothing
persists; a reconnect rebuilds from the live templates.

## API / SDK surface

None. No new verb. `SparkplugPublisher` is not a QObject and has no API handler; it is driven
by `PublisherWorker`. The `registerMetric` signature change is internal (one caller, one test).

## QML / UI

None. The publisher's Sparkplug UI (spec 0073, the four MqttPublisherView fields) is
unchanged — multi-source is automatic when a project has multiple sources.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the worker learns a source is GONE (swap vs restructure) | (a) generation-reconcile via a `structureGenerationChanged(quint64)` side-signal; (b) widen `structurePublished` to carry the generation; (c) add `quint64 structureGeneration` to the inner `DataModel::Frame` and stamp it | **(a) side-signal** — the generation is NOT on the inner `DataModel::Frame` (it lives only on `TimestampedFrame`), so a bump must be carried explicitly; one new signal on the single `ensureStructurePublished` emit site, queued FIFO ahead of `structurePublished`, leaves the seven `structurePublished` consumers untouched and adds no per-frame cost. (b) ripples a signature change across all seven `setTemplateFrame` consumers; (c) requires stamping a new `Frame` field at multiple frame-finalization sites in the hotpath file, two of which hold the frame by `const&` — a wider, more error-prone hotpath touch for no wire gain. *(Corrected 2026-08-28: the original plan chose "(a) generation already on the frame", which was factually wrong — the field is on `TimestampedFrame`, not the inner `Frame`.)* |
| Alias stability mechanism | positional (today); persistent uniqueId→alias map; tombstoned vector slots | **Persistent map** — decouples alias from storage position so per-source removal never renumbers survivors, and a uniqueId keeps its alias across rebuilds within a connection (R5). Tombstoning the vector keeps dead slots and complicates the birth walk for no gain. |
| Where a name collision is resolved | at registration (store qualified name); at birth-build (resolve on emit) | **At birth-build** — names appear only in births; data is alias-addressed. Resolving on emit means the stored entry keeps its bare title, single-source output is untouched (R6), and adding a second source that collides just changes the next birth's names, which a rebirth already re-declares. |
| Name-qualifier format | `sourceN/title`; `<sourceTitle>/title`; `title (sourceN)` | **Plan-phase mechanical detail**, deferred to tasks — must be deterministic and stable, and must never collide again; mirror the InfluxDB sink's 0073 resolution. Not a product decision. |

## Risks & mitigations

- **Single-source wire drift (R6).** The highest risk: any change to alias numbering or metric
  names on the common path breaks existing deployments. Mitigation: AC3 pins a single-source
  fixture's births/aliases/data byte-for-byte against a stored spec-0073 expectation; the
  name resolver returns the bare title whenever no other source collides, and the alias map
  assigns 1,2,3… in registration order exactly as the positional scheme did for one source.
- **Stale-drop timing.** If `dropStaleMetrics` ran mid-burst (before all live sources
  republished), it could drop a live source momentarily. Mitigation: it drops only entries
  *older* than the newest generation seen; a live source republishing at the new generation is
  never older, and the drop feeds a rebirth, so the worst case is a redundant rebirth, never a
  lost live metric.
- **Alias-space growth (R10).** Retired aliases are not reused within a connection; over a
  pathological restructure-churn session the monotonic counter climbs. Mitigation: the counter
  is 64-bit local and resets every connection; `kMaxMetrics` still caps live entries; a
  reconnect is the reset. Documented, matches the resolved decision.
- **Silent-breakage class.** This is the same silent-empty-recording family as the rest of
  0073 — a metric that should publish but doesn't. Mitigation: every drop path
  (`dropStaleMetrics`, cap overflow) increments a pulled counter; AC5 asserts the cap counter,
  AC2 asserts the surviving-source aliases are unchanged.

## Test & verification plan

- **Unit (ctest, maintainer runs; you author):** `app/tests/tst_sparkplug_publisher.cpp` —
  AC1 (two-source births/aliases/data + stable alias after A's dataset removed), AC2
  (restructure one source → rebirth, other source's aliases byte-identical), AC3 (single-source
  byte-compat vs stored 0073 expectation), AC4 (swap registers only B), AC5 (cap overflow drops
  + counts), AC7 (device id: one device carries all sources), AC8 (colliding titles →
  source-qualified, unique title stays bare). All drive `SparkplugPublisher` directly, no broker.
- **Integration:** none required — no API verb, no project-file change.
- **Hotpath:** `--benchmark-hotpath` must stay flat (change is off the block/frame path); a
  spot benchmark run confirms no regression, but there is no new gate.
- **In-app (AC6):** maintainer publishes a real two-source project to a Sparkplug-aware broker
  (Ignition / HiveMQ / MQTT Explorer with a Sparkplug decoder): both sources' tags appear
  under one node; restructuring one re-births without disturbing the other; both keep updating.
- **Static:** `python scripts/code-verify.py --check` on the touched files; `qt-cpp-review`
  on the C++ diff before handoff; `python scripts/sanitize-commit.py` before commit.
