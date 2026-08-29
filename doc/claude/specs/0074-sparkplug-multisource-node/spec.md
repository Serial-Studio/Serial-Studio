---
spec: 0074-sparkplug-multisource-node
title: Multi-Source Sparkplug B Edge Node (stable-alias registry)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-08-28
author: Alex Spataru
---

# Spec 0074 — Multi-Source Sparkplug B Edge Node

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Spec 0073 shipped outbound Sparkplug B publishing: Serial Studio acts as an Edge Node,
declaring every published dataset as a metric with a stable alias in NBIRTH/DBIRTH and
carrying only-changed metrics by alias in NDATA/DDATA. It works correctly for a
single-source project and across a project swap (load project A, then project B).

It is wrong for a project that has **more than one data source** and publishes them into a
single Sparkplug edge node. The outbound metric registry is whole-registry-cleared and
rebuilt on every per-source structure publish (`structurePublished(sourceId, frame)` fires
once per source; the publisher keeps one frame template per source in `m_templates`, but the
Sparkplug registry is rebuilt from a single source's export schema). So when source B
publishes its structure, it wipes source A's metrics — the node then births and publishes
only the most-recently-structured source, and A's data silently never reaches the broker.
This is the silent-data-loss failure class the rest of spec 0073's review pass eliminated;
it is documented as a known limitation in `PublisherSparkplug.cpp::registerSparkplugMetrics`
rather than fixed, because the correct fix is not mechanical.

The blocker is the alias model. A Sparkplug alias is a stable per-connection handle a host
learns once at birth and uses to resolve every later data message. The registry today
assigns each metric its alias positionally (`alias = m_metrics.size() + 1`), so removing one
source's metrics from the middle of the table renumbers every surviving alias — which, on a
live connection, silently repoints a host's learned aliases at the wrong metrics. A correct
multi-source registry therefore needs aliases that never move once assigned, per-source
scoping so one source's structure change touches only its own metrics, and a rebirth whenever
the metric set changes so hosts re-learn the (possibly extended) alias table. That is a
data-model change, not a config tweak.

Who feels it: any integrator running two acquisition sources (e.g. a serial sensor plus a
TCP feed) into one dashboard and publishing that dashboard to an MQTT/Sparkplug
infrastructure — Ignition, HiveMQ, AWS IoT — expects both sources' tags to appear under the
node. Today one silently vanishes.

## Goals

- A project with two or more sources, all publishing structure, registers the union of every
  source's metrics under the one edge node, and NBIRTH/DBIRTH declares all of them.
- A structure change on one source (a dataset added, removed, renamed, or retyped) updates
  only that source's metrics in the registry and never renumbers another source's aliases.
- Any change to the published metric set triggers exactly one rebirth so every host re-learns
  the current alias table before the next data message uses it.
- A metric's alias, once assigned within a connection, never changes and is never reused for
  a different metric while the connection lives.
- Single-source projects and project-swap behavior are byte-for-byte unchanged from spec
  0073 on the wire (same aliases, same births, same data) — this feature adds the multi-source
  case without disturbing the shipped common cases.

## Non-Goals

- **Multiple edge nodes from one Serial Studio instance.** This spec merges sources into ONE
  node; publishing each source as its own edge node is a separate feature.
- **Per-source Sparkplug *devices*.** Mapping each project source to a Sparkplug DBIRTH/DDATA
  device (rather than folding all sources into the node's own metric space) is a plausible
  alternative shape but is out of scope here; this spec extends the existing node/device model,
  it does not add a source-to-device mapping.
- **Inbound (subscriber) Sparkplug**, the decoder, and the birth-certificate state machine —
  untouched.
- **Alias persistence across reconnects.** Aliases are stable within a connection; a new
  connection may re-derive them (a reconnect re-births anyway).
- **Changing the single-source wire output.** If honoring "stable alias" would alter a
  single-source project's alias numbering versus 0073, the single-source path keeps its
  current numbering (see Constraints).

## Requirements

1. **R1** — With two or more sources enabled and publishing structure, an NBIRTH (and DBIRTH
   when a device id is set) declares a metric for every dataset of every source, each with a
   distinct stable alias.
2. **R2** — NDATA/DDATA after such a birth carries changed metrics from any source, each
   addressed by the alias declared at birth; a host resolves every one.
3. **R3** — A structure change on one source (add/remove/rename/retype a dataset) leaves every
   other source's aliases unchanged, and the metrics of the unaffected sources keep publishing
   without interruption.
4. **R4** — Any change to the registered metric set (a source added, removed, or restructured)
   causes exactly one rebirth; no data message is published under a not-yet-declared or
   renumbered alias.
5. **R5** — Within one connection, an alias assigned to a metric is never reassigned to a
   different metric, even after the original metric's dataset is removed (the alias may be
   retired but not reused).
6. **R6** — A single-source project produces the same births, aliases, and data messages on
   the wire as spec 0073 did — verified against a captured 0073 single-source exchange.
7. **R7** — A project swap (load A, then B, no restructure of B's own sources) drops A's
   metrics and publishes only B's, exactly as spec 0073 did.
8. **R8** — The total metric count across all sources is bounded by the existing
   `kMaxMetrics` cap; exceeding it drops the overflow with the existing counter, and the drop
   is user-visible (never silent).
9. **R9** — Every source's metrics are declared in the edge node's own NBIRTH metric space;
   no Sparkplug Device (DBIRTH/DDATA) is created per source. A device id, when configured,
   still behaves exactly as spec 0073 (one device for the whole node), unaffected by source
   count.
10. **R10** — An alias retired when its dataset is removed is not reused for the life of the
    connection; the alias counter resets only on a new connection (which re-births).
11. **R11** — A metric declared from a dataset whose title is unique across all sources uses
    that bare title; only when two sources hold datasets that would produce the same name does
    the node qualify the colliding names with a source prefix. A single-source project's names
    are therefore never qualified (protecting R6).

## Acceptance Criteria

- [ ] **AC1** (R1, R2, R5) — C++ unit tier (`tst_sparkplug_publisher`): a two-source registry
  fixture births metrics from both sources with distinct aliases; data from either source
  resolves; removing a dataset from source A and re-registering does not change source B's
  aliases nor reuse A's retired alias.
- [ ] **AC2** (R3, R4) — Unit: restructuring one source flips `needsRebirth`, the next birth
  re-declares the full current set, and the other source's aliases are byte-identical before
  and after.
- [ ] **AC3** (R6) — Unit: a single-source fixture produces births/aliases/data byte-identical
  to a stored spec-0073 expectation (regression guard on the common path).
- [ ] **AC4** (R7) — Unit: swap fixture (register source set A, then source set B with no
  overlap) births only B's metrics.
- [ ] **AC5** (R8) — Unit: registering past `kMaxMetrics` across two sources drops the overflow
  and increments the existing cap counter.
- [ ] **AC7** (R9) — Unit: with a device id configured, a two-source project still births one
  device carrying all sources' metrics, no extra DBIRTH per source.
- [ ] **AC8** (R11) — Unit: two sources with a shared dataset title birth two distinctly-named
  metrics (source-qualified); a unique title stays bare; a single-source fixture's names match
  the AC3 stored 0073 expectation exactly.
- [ ] **AC6** (R1-R4, in-app) — Maintainer observation: a real two-source project published to
  a Sparkplug-aware broker (Ignition / HiveMQ / MQTT Explorer with a Sparkplug decoder) shows
  both sources' tags under one node; restructuring one source re-births without disturbing the
  other; both continue updating.

## Constraints & Invariants

- **Alias stability is the core invariant** — an alias never moves or is reused within a
  connection (R5). This is what forbids the current positional scheme; the registry must
  assign aliases from a monotonic source, decoupled from storage position.
- **Single-source wire compatibility is non-negotiable** — the overwhelmingly common case must
  not regress. If a stable-alias scheme would renumber a single-source project's aliases versus
  0073, the single-source path retains 0033-era numbering (a birth re-declares aliases anyway,
  so a host is never confused — but AC3 pins the exact bytes to catch accidental drift).
- **Rebirth is the only correct response to a metric-set change** — never mutate a live alias
  table and keep publishing; re-declare then publish (R4). The existing
  `m_registryDirty`/`needsRebirth`/`commitBirth` machinery is the seam.
- **No new hotpath cost.** Registration and rebirth happen at structure-change rate (a
  project/source edit), never per block or per frame; the publish path stays as spec 0073 left
  it. Must not regress the 256 kHz gate.
- **Bounded** — the union registry is still capped at `kMaxMetrics`; overflow drops and counts,
  never grows unbounded (R8).
- **Pro feature, existing gating** — rides the already-Pro MQTT publisher; no new license
  surface.
- **Project-file compatible** — no schema change; the multi-source case is a runtime behavior,
  not a new persisted field.

## Resolved Decisions (maintainer, 2026-08-28)

- **Placement: node metric space.** All sources' metrics fold into the one edge node's own
  NBIRTH metric set — NOT per-source DBIRTH devices. This keeps the node/device model unchanged
  and the spec small. (See R9.)
- **Source key: `sourceId` on the registry entry.** Each metric carries the id of the source
  that owns it; a source's re-register removes only entries with that id and re-adds its current
  set. Minimal change to the existing flat registry rather than a per-source sub-table. (R3.)
- **Retired aliases: retire until reconnect.** An alias is never reused within a connection; a
  reconnect re-births and resets the alias counter. No free-list, no reclaim. (R5, R10.)
- **Name collisions: qualify only on collision.** A metric keeps its bare dataset title when it
  is unique across all sources, and is prefixed with a source qualifier only when two sources
  would otherwise collide — mirroring how the InfluxDB sink resolved the same duplicate-title
  case in spec 0073. This preserves single-source names unchanged (protecting R6). (R11.)

These four are settled; the plan implements them, it does not re-litigate them.

## Open Questions

- None blocking. Remaining detail (the exact source-qualifier format for a name collision, and
  whether the alias counter is 16-bit-wide headroom against `kMaxMetrics`) is a plan-phase
  mechanical choice, not a product decision.
