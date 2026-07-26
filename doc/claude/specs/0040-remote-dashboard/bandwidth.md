---
spec: 0040-remote-dashboard
phase: pre-gate artifact (T5)
status: measured (fixtures) / pending (live AC6 leg)
updated: 2026-07-25
---

# Mirror bandwidth and message size

The evidence for **AC6** (R11) and the input to the chunked-structure decision. Every number
below is computed from the recorded fixtures by a script, not estimated in prose. Nothing
here required a running app.

## Method

```bash
python3 tests/fixtures/mirror/generate_fixtures.py       # rebuild fixtures from examples/
python3 tests/fixtures/mirror/measure_bandwidth.py --markdown
```

`generate_fixtures.py` encodes real checked-in example projects through the same codec the
client decodes with (`tests/utils/mirror_client.py`), so the byte shapes are the shapes the
socket would carry. `measure_bandwidth.py` re-encodes each fixture's snapshots at a given
value precision, measures the encoded NDJSON line length including the newline, and
multiplies the mean by the mirror rate.

Values are synthetic but shaped like telemetry: a deterministic random walk inside each
dataset's configured widget range. That matters, because the dominant term in a snapshot is
the decimal length of a double, not the field names.

**The device rate does not appear anywhere in the calculation.** That is the point. Mirror
cost is `O(datasets x effectiveHz)`; there is no term for how fast the device is producing
data. The live leg below confirms the property on a real pair; the arithmetic is why it is
expected to hold.

| Fixture | Source project | Datasets | Groups | Sources |
|---------|----------------|----------|--------|---------|
| `small` | `examples/LorenzAttractor/LorenzAttractor.ssproj` | 6 | 2 | `[0]` |
| `wide` | `examples/System Monitor/system-monitor.ssproj` | 59 | 8 | `[0]` |
| `multisource` | `examples/Dual Drone Telemetry/Dual Drone Telemetry.ssproj` | 48 | 16 | `[0, 1]` |

`multisource` is not required by `tasks.md` T2, which asks for a small and a widest fixture.
It was added because it is the only checked-in project with two sources, and the positional
value format's ordering rule is defined over `(sourceId, uniqueId)` — an untested ordering
rule is the kind of thing that ships wrong.

## Snapshot cost

Mean encoded snapshot line, and the resulting mirror byte rate at 1 Hz, the 20 Hz default,
and the 60 Hz ceiling.

### small — 6 datasets

| precision | B/dataset | mean snapshot | 1 Hz | 20 Hz | 60 Hz |
|-----------|-----------|---------------|------|-------|-------|
| full | 31.9 | 192 B | 192 B/s | 3.7 KB/s | 11.2 KB/s |
| 6 sig | 22.0 | 132 B | 132 B/s | 2.6 KB/s | 7.7 KB/s |
| 4 sig | 20.1 | 120 B | 120 B/s | 2.4 KB/s | 7.1 KB/s |

### wide — 59 datasets

| precision | B/dataset | mean snapshot | 1 Hz | 20 Hz | 60 Hz |
|-----------|-----------|---------------|------|-------|-------|
| full | 19.2 | 1,136 B | 1.1 KB/s | 22.2 KB/s | 66.5 KB/s |
| 6 sig | 9.3 | 546 B | 546 B/s | 10.7 KB/s | 32.0 KB/s |
| 4 sig | 7.3 | 432 B | 432 B/s | 8.4 KB/s | 25.3 KB/s |

### multisource — 48 datasets, 2 sources

| precision | B/dataset | mean snapshot | 1 Hz | 20 Hz | 60 Hz |
|-----------|-----------|---------------|------|-------|-------|
| full | 20.2 | 969 B | 969 B/s | 18.9 KB/s | 56.8 KB/s |
| 6 sig | 9.9 | 476 B | 476 B/s | 9.3 KB/s | 27.9 KB/s |
| 4 sig | 8.0 | 383 B | 383 B/s | 7.5 KB/s | 22.4 KB/s |

### Reading these

- **`plan.md` estimated "roughly 10-12 bytes per dataset on the wire". At full precision the
  measured figure is 19-32.** The shortest round-trip representation of an arbitrary double
  runs to 17 significant digits, and the per-dataset fixed cost (comma plus null slots) is
  small next to it. The plan's estimate holds only at ~6 significant digits.
- The plan's headline projections shift accordingly: 200 datasets at 20 Hz is ~77 KB/s at
  full precision rather than ~48 KB/s; 1,000 datasets at 20 Hz is ~384 KB/s rather than
  ~240 KB/s. **Both are still far inside every cap**, so the conclusion the plan drew from
  its estimate survives its estimate being wrong.
- Precision is therefore worth having as a subscribe parameter but is not worth making the
  default: halving 22 KB/s buys nothing, and losing plot fidelity by default is the worse
  failure. `wire-protocol.md` §7.3 records that decision.
- The small fixture's high per-dataset figure is an artifact of a 6-dataset project: the
  snapshot envelope (`kind`, `epoch`, `seq`, `tNs`) is fixed cost amortized over very few
  values.

### Headroom against the server's own limits

| Quantity | Worst measured | Cap (`Server.cpp:45-52`) | Headroom |
|----------|----------------|--------------------------|----------|
| Snapshot message | 1,136 B (`wide`, full) | `kMaxApiMessageBytes` 1 MB | ~920x |
| Mirror byte rate | 66.5 KB/s (`wide`, 60 Hz, full) | `kMaxApiBytesPerWindow` 128 MB/s | ~1,970x |
| Mirror byte rate at the 20 Hz default | 22.2 KB/s | 128 MB/s | ~5,900x |
| Inbound messages | ~3 per attach | `kMaxApiMessagesPerWindow` 200/s | not binding |

`kMaxApiBufferBytes` (4 MB) governs the server's *inbound* buffer and is not touched by
outbound pushes.

For contrast, the thing the mirror exists to avoid: `ServerWorker::processItems` serializes
every parsed frame and broadcasts it. At the product's headline 256 kHz with a 59-dataset
project the frame stream is on the order of hundreds of MB/s — past
`kMaxApiBytesPerWindow`, so the server would disconnect the viewer before the network
noticed. That is why `mirror.subscribe {"frames": false}` is the first message on the wire.

## Structure cost — the finding that changes the plan

`plan.md` (Risks) says: *"1 MB binds at roughly 80,000 datasets, but the structure message is
much larger per dataset (~25 config keys). Mitigation: the wide-project fixture is part of P0
precisely to measure this."* Measured:

| Fixture | Structure message | Per dataset (average) |
|---------|-------------------|-----------------------|
| `small` | 11,983 B | 1,997 B |
| `wide` | 28,869 B | 489 B |
| `multisource` | 113,549 B | 2,366 B |

An average is the wrong statistic here — structure size is a fixed part (control script,
widget settings, per-source parser code, MQTT publisher block) plus a per-dataset part. To
separate them, `measure_bandwidth.py` replicates the widest project's groups and measures
the real encoded size:

| Datasets | Structure bytes |
|----------|-----------------|
| 59 | 28,678 |
| 118 | 50,718 |
| 236 | 94,880 |
| 472 | 183,204 |
| 944 | 359,900 |

Linear fit: **374 B per dataset marginal, 6,597 B fixed.**

| Threshold | Dataset count |
|-----------|---------------|
| `kMaxApiMessageBytes` (1 MB) | **~2,784** |
| `kMaxApiBufferBytes` (4 MB) | ~11,189 |

### Decision: chunked structure delivery is in v1

~2,784 datasets is not a hypothetical ceiling. A Modbus map import or a DBC import produces
thousands of signals routinely, and the multisource fixture already shows that a project with
image groups and painter code carries ~2.4 KB per dataset — which would cross 1 MB at ~440.
A viewer that silently fails to attach to a large project, or attaches to a truncated one, is
precisely the failure class this spec exists to avoid.

`wire-protocol.md` §6.2 therefore specifies `structureChunk` as part of wire version 1:
base64 the structure payload, split at 512 KB, reassemble by part index, cap at 64 parts and
answer `MIRROR_STRUCTURE_TOO_LARGE` beyond it. `tests/fixtures/mirror/edge/structure-chunked.ndjson`
exercises the path, including out-of-order arrival.

This adds scope to T7 (`MirrorProtocol.h`), T10 (`MirrorHandler`), T11 (`MirrorPublisher`),
and T14 (`MirrorClient`) that `tasks.md` does not currently carry. That is the pre-gate phase
working as intended.

## Live measurement (AC6) — maintainer-run

Not yet run: it needs the mirror implemented (T7-T13), which is behind the 0039 M2 gate.

```bash
# machine A (the capture)
serial-studio --headless --api-server --api-external project.ssproj

# machine B (the viewer)
python3 tests/manual/mirror_bandwidth_live.py \
    --host <machine-A> --token <hex> --mirror-hz 20 --low-hz 10 --high-hz 2000
```

The harness drives a real device through `DeviceSimulator` on a producer thread, measures the
mirror socket for a fixed window at each device rate, and prints a PASS/FAIL against AC6's
"within the same order of magnitude" bar. It refuses to run unless the high rate is at least
two orders of magnitude above the low one.

Record the printed table here when it runs:

| device Hz | mirror Hz | snapshots/s | mean snapshot | mirror B/s |
|-----------|-----------|-------------|---------------|------------|
| _pending_ | | | | |
| _pending_ | | | | |

**Expected result**, from the arithmetic above: the two rows differ by well under 2x, because
neither term in `O(datasets x effectiveHz)` changed between them. A material difference would
mean the publisher is doing per-frame work, which would be a finding against the plan's
central claim rather than a tuning problem.
