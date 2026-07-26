---
spec: 0040-remote-dashboard
phase: pre-gate artifact (T1, T6)
status: draft
updated: 2026-07-25
wireVersion: 1
---

# Mirror wire protocol v1

The normative contract for the spec-0040 remote dashboard mirror. Both implementations —
`app/src/API/Mirror/MirrorProtocol.h` (T7, post-gate) and `tests/utils/mirror_client.py`
(T3, landed) — are written against this document, and the conformance tests in
`tests/unit/test_mirror_protocol.py` exercise it against recorded fixtures with no app
running.

Everything here is decided by [`plan.md`](./plan.md) except where a section says otherwise;
sections marked **[correction]** record a place where measurement or code ground-truth
contradicted the plan, and state what this document does instead.

## 1. Transport and framing

The mirror rides the existing TCP/JSON API server on port 7777
(`app/src/API/Server.cpp`). No new listening socket, no new build dependency.

- Framing is newline-delimited JSON: one compact UTF-8 JSON object per line, `\n`
  terminated. Identical to every other message on this socket, so
  `tests/utils/api_client.py`'s reader works unchanged.
- Client requests are ordinary API commands:
  `{"type":"command","id":"<uuid>","command":"<name>","params":{...}}`.
- Server responses are ordinary API responses: `{"type":"response","id":...,"success":...}`.
- Server pushes carry a single new top-level key, `mirror`. See §9 for why that is
  invisible to existing clients.

The server's existing limits are the budget, unchanged: 1 MB per message, 4 MB inbound
buffer, 200 messages/s, 128 MB/s, 32 connections
(`Server.cpp:45-52`). §8 measures the mirror against them.

## 2. Wire version and forward compatibility

`wireVersion` is the integer `1`.

- A client sends `wireVersion` in `mirror.subscribe`. The server rejects a mismatch with
  `MIRROR_VERSION_MISMATCH` rather than serving a payload the client will misread.
- Every `structure` push repeats `wireVersion`. A client that decodes a structure whose
  version it does not speak reports the mismatch to the user and renders nothing; it never
  half-decodes.
- **Unknown object fields are ignored** at every level, by both sides.
- **Unknown `kind` values are dropped** silently, and so are unknown top-level push keys.
- The version is bumped only for a change that breaks one of those two rules. Adding a
  field, adding a `kind`, or adding a command does not bump it.

## 3. Connect sequence (normative)

**[correction]** `tasks.md` T1 lists the order as
`handshake → mirror.getInfo → mirror.subscribe{frames:false} → mirror.getStructure`, while
`plan.md`'s risk register requires that "the subscribe request is the *first* message after
the handshake". Those conflict, and the risk register is right: the server writes
`{"frames":[...]}` to every socket unconditionally from the first flush
(`ServerWorker::processItems`, `Server.cpp:300-324`), so any request that precedes
`mirror.subscribe` is a window in which a 256 kHz capture floods the viewer. The normative
order is therefore **subscribe first**, and `mirror.subscribe`'s response carries the whole
`mirror.getInfo` payload so nothing is lost by dropping the pre-flight probe.

```
1. TCP connect to host:port
2. -> {"type":"command","command":"mirror.subscribe",
       "params":{"wireVersion":1,"hz":20,"frames":false}}
3. <- {"type":"response","success":true,"result":{ ...info... }}
4. -> {"type":"command","command":"mirror.getStructure"}
5. <- {"type":"response","success":true,"result":{ ...structure payload... }}
6. <- {"mirror":{"kind":"snapshot",...}}   (repeating, at the negotiated rate)
```

### 3.1 Authentication is lazy, not unconditional

`Server::acceptConnection` sets
`state.authenticated = !(m_externalConnections && !peer.isLoopback())`
(`Server.cpp:1323`). A loopback peer, and *any* peer while external connections are off, is
pre-authenticated and never sees the handshake; sending it an unsolicited
`{"type":"auth",...}` line yields an error response, because `auth` is not a valid
`MessageType` on an already-authenticated connection.

The client therefore **sends its first request optimistically**. If the response is
`success:false` with an "Authentication required" message, it sends
`{"type":"auth","token":"<token>"}`, waits for `{"authenticated":true}`, and retries the
request. A client with no token configured surfaces the refusal rather than retrying.

`kMaxAuthAttempts` is per connection; three failures close the socket.

### 3.2 Reconnect

On socket loss the client reconnects with exponential backoff `1, 2, 4, 8, 16, 30` seconds
and repeats the full sequence from step 2, including `mirror.getStructure`. A reconnect
never assumes the previously held epoch is still current.

## 4. Requests

### 4.1 `mirror.subscribe` (connection-scoped)

Handled in `Server::handleJsonMessage` ahead of the generic registry dispatch, because it
mutates per-connection state and `CommandRegistry::execute` has no connection context (the
same reason the MCP branch sits there).

| Param | Type | Required | Meaning |
|-------|------|----------|---------|
| `wireVersion` | int | yes | Client's protocol version. Mismatch is refused. |
| `hz` | int | no (default 20) | Requested mirror cadence. Must be in `[1, 60]`. |
| `frames` | bool | no (default `false`) | Whether this connection keeps receiving `{"frames":...}`. |
| `precision` | int | no (default 0) | Significant digits for snapshot values; `0` = full round-trip. See §7.3. |

Result — this is also exactly what `mirror.getInfo` returns, plus the per-connection fields:

```json
{
  "wireVersion": 1,
  "appVersion": "3.3.0",
  "sessionId": "17",
  "epoch": 7,
  "layoutHash": "9a3f0c2e11b47d05",
  "operationMode": 0,
  "datasetCount": 59,
  "sourceIds": [0],
  "viewersAllowed": true,
  "maxViewers": 32,
  "viewers": 1,
  "structureParts": 1,
  "commands": ["mirror.subscribe", "mirror.setRate", "mirror.unsubscribe"],
  "hz": 20,
  "effectiveHz": 20,
  "frames": false,
  "precision": 0
}
```

`effectiveHz` is `min(hz, local UI tick rate)`: the publisher wakes on
`UI::Dashboard::updated()` and cannot emit faster than that signal fires. A client uses
`effectiveHz`, not `hz`, to size its staleness watchdog.

Errors: `MIRROR_VERSION_MISMATCH`, `MIRROR_RATE_OUT_OF_RANGE`, `MIRROR_VIEWER_LIMIT`.
An unauthenticated connection cannot subscribe.

### 4.2 `mirror.setRate` (connection-scoped)

Params `{"hz": int}`, result `{"hz": int, "effectiveHz": int}`. Out-of-range is rejected
with `MIRROR_RATE_OUT_OF_RANGE`, never silently clamped: a silent clamp hides a
misconfigured viewer. Requires an active subscription (`MIRROR_NOT_SUBSCRIBED` otherwise).

### 4.3 `mirror.unsubscribe` (connection-scoped)

No params. Result `{"mirrorSubscribed": false}`. It stops the mirror and **does not**
re-enable the per-frame stream: `frames` is only ever changed by `mirror.subscribe`, so
unsubscribing can never accidentally reopen the firehose on a slow reader.

Disconnect clears mirror state alongside the existing MCP session cleanup in
`Server::onSocketDisconnected`.

### 4.4 `mirror.getInfo` (stateless, registry)

No params. Returns the §4.1 result minus `hz` / `effectiveHz` / `frames` / `precision`.
Discoverable through `api.getCommands`, MCP, and the generated SDK. It is a pre-flight
probe for tooling; the attach sequence does not use it, and a client that calls it before
subscribing accepts the firehose window described in §3.

### 4.5 `mirror.getStructure` (stateless, registry)

| Param | Type | Required | Meaning |
|-------|------|----------|---------|
| `part` | int | no (default 0) | Zero-based part index when the structure is chunked (§6.2). |

Returns the structure payload of §6.1 for the *current* epoch, or one chunk of it. The
client always compares the returned `epoch` against what it holds; there is no way to
request a historical epoch, and no reason to — a stale structure is refetched, not
replayed.

## 5. Ordering, identity, and the layout hash

The snapshot format is positional. The structure message carries the ordered identity list
once; every snapshot after it is a bare array of numbers.

**Order** is the order `UI::Dashboard::buildValuePushes()` walks
(`Dashboard.cpp:2337-2362`):

1. ascending `sourceId` — the iteration order of `QMap<int, Frame> m_sourceRawFrames`;
2. then group order within that source's frame;
3. then dataset order within the group.

**Identity** is the pair `[sourceId, uniqueId]`. `uniqueId` is the dataset's persisted
stable identity (`Frame.h:441`); when it is `-1` (unassigned), the legacy value
`dataset_unique_id(sourceId, groupId, datasetId)` = `sourceId*1000000 + groupId*10000 +
datasetId` is used (`Frame.h:1049-1054`). Positional `datasetId` alone is not identity: it
changes on reorder.

**Layout hash** is FNV-1a 64 over the seed bytes `mirror-v1\n` followed by
`"<sourceId>:<uniqueId>;"` in UTF-8 for every entry, in list order, rendered as 16
lowercase hex digits. A client recomputes it over the received `datasets` array and
**refuses to adopt a structure whose announced hash does not match** — that is the one
check that makes a positional format safe. Reference implementation:
`layout_hash()` in `tests/utils/mirror_client.py`.

## 6. Server pushes

Every push is `{"mirror": {...}}` with a `kind` discriminator.

### 6.1 `structure`

Sent as a push when the epoch changes, and returned as the `result` of
`mirror.getStructure` (identical payload, so one decoder serves both).

```json
{"mirror":{
  "kind": "structure",
  "wireVersion": 1,
  "epoch": 7,
  "layoutHash": "9a3f0c2e11b47d05",
  "sourceIds": [0, 1],
  "datasets": [[0, 10000], [0, 10001], [1, 1010000]],
  "operationMode": 0,
  "plotTimeRange": 10.0,
  "frozen": false,
  "clock": {"domain": "monotonic-relative", "originUnixMs": 1753459200123},
  "project": { "...the serialized project document, verbatim..." }
}}
```

| Field | Type | Meaning |
|-------|------|---------|
| `epoch` | int | Monotonically increasing from 1. §6.4. |
| `layoutHash` | string | 16 hex digits over `datasets`. §5. |
| `sourceIds` | int[] | Ascending; parallel to a snapshot's `tNs` array. |
| `datasets` | [int,int][] | Ordered `[sourceId, uniqueId]` list. Its length is the snapshot value count. |
| `operationMode` | int | 0 ProjectFile, 1 ConsoleOnly, 2 QuickPlot. |
| `plotTimeRange` | double | Remote's plot window, seconds. |
| `frozen` | bool | Remote's frozen flag. |
| `clock.domain` | string | Always `"monotonic-relative"` in v1. §7.1. |
| `clock.originUnixMs` | int64 | Wall-clock instant of `tNs == 0`, for display only. |
| `project` | object | The existing serialized project document (`project.exportJson` shape), unchanged. No new keys, no schema bump. |

### 6.2 `structureChunk` **[correction]**

`plan.md` estimated the 1 MB message cap binds "at roughly 80,000 datasets". That figure is
for the *snapshot*. Measured against a real project ([`bandwidth.md`](./bandwidth.md)) the
**structure** message costs ~374 bytes per dataset marginal plus ~6.6 KB fixed, so it
crosses 1 MB at **~2,784 datasets** — a project size this product reaches routinely via a
Modbus map or DBC import. Chunked delivery is therefore part of v1, not a contingency.

When the encoded structure line would exceed **512 KB**, the publisher base64-encodes the
UTF-8 bytes of the structure payload object and splits the base64 text into parts of at
most 512 KB:

```json
{"mirror":{"kind":"structureChunk","epoch":7,"part":0,"parts":3,"data":"<base64 slice>"}}
```

- Parts are contiguous, ordered, and all carry the same `epoch`.
- The client concatenates `data` for parts `0..parts-1`, base64-decodes, and parses the
  result as a §6.1 payload, then processes it exactly as if it had arrived whole.
- A part whose `epoch` differs from the accumulator's resets the accumulator.
- `parts` is capped at 64; beyond that the publisher answers `MIRROR_STRUCTURE_TOO_LARGE`
  and the viewer reports that the remote project is too large to mirror, rather than
  rendering a truncated dashboard.
- Base64 is used rather than raw slicing because it makes every slice boundary safe
  regardless of where a multi-byte UTF-8 sequence falls. The 33% inflation is paid once per
  epoch on a human-paced event.
- `mirror.getInfo` / `mirror.subscribe` report `structureParts` so a client knows before
  asking.

### 6.3 `snapshot`

```json
{"mirror":{
  "kind": "snapshot",
  "epoch": 7,
  "seq": 1234,
  "tNs": [1523400000, 1523390000],
  "values": [12.5, null, 3.25, null],
  "strings": {"1": "OK"},
  "nonFinite": {"3": "nan"}
}}
```

| Field | Type | Meaning |
|-------|------|---------|
| `epoch` | int | Must equal the client's held epoch. §6.4. |
| `seq` | uint64 | Increments by 1 per emitted snapshot, per publisher (not per subscriber). A gap means a viewer's rate divider skipped, not that data was lost. |
| `tNs` | int64[] | One entry per `sourceIds` entry, in the same order. §7.1. |
| `values` | array | Length equals the structure's `datasets` length, always. |
| `strings` | object | Optional sparse map, decimal index → string. Omitted when empty. |
| `nonFinite` | object | Optional sparse map, decimal index → `"nan"` / `"inf"` / `"-inf"`. Omitted when empty. |

A `null` in `values` means "look in `strings`, then `nonFinite`; if neither has this index,
the dataset has no current value". `null` is never a legal *numeric* value.

### 6.4 Epoch semantics

The epoch is the entire consistency story. There are no partial structure updates, no
diffs, and no ordering assumptions between the two message kinds.

- The publisher increments the epoch on `Dashboard::widgetCountChanged` and
  `Dashboard::dataReset`, i.e. whenever the ordered dataset list or any structural
  attribute could have changed.
- On increment the publisher pushes the new `structure` to every subscriber **before** any
  snapshot carrying the new epoch.
- A client **drops** any snapshot whose `epoch` is not the one it holds, and issues
  `mirror.getStructure`. At most one structure request is in flight at a time.
- A client also drops a snapshot whose `values` length disagrees with the held structure,
  and treats it identically. That is a belt-and-braces check: it can only fire if a
  publisher violates the epoch rule.
- There is no code path anywhere that applies values against a structure they do not match.

### 6.5 `heartbeat` **[correction]**

Not in `plan.md`, and required by R8/R9. The publisher wakes on `Dashboard::updated()`,
which only fires when data arrives. Without a heartbeat, "the remote capture is idle" and
"the link is dead" produce byte-identical silence, and the viewer must call both stale — so
an idle-but-healthy monitoring session reads as a failure.

When no snapshot has been emitted for **1 second**, the publisher emits:

```json
{"mirror":{"kind":"heartbeat","epoch":7,"seq":1234}}
```

`seq` repeats the last emitted snapshot's sequence, so the client can confirm it missed
nothing. The client's two states are therefore distinguishable:

| Condition | `stale` | `live` | UI reads |
|-----------|---------|--------|----------|
| Snapshots arriving | false | true | "Live" |
| Only heartbeats arriving | false | false | "Connected, no data" |
| Nothing within the watchdog | true | false | "Disconnected / stale" |

## 7. Time and values

### 7.1 Clock domain **[correction]**

`plan.md` says "the snapshot carries the remote's frame timestamp" and "the viewer passes
the received value into the `TimestampedFrame` it publishes". As written that is not
implementable: `TimestampedFrame::timestamp` is a `std::chrono::steady_clock::time_point`
(`Frame.h:1601-1605`) — a process-local monotonic clock whose epoch is arbitrary and
differs between machines and between runs on the same machine. Transporting its raw value
produces a meaningless number on the viewer.

The wire therefore carries a **relative** time, which is what the dashboard actually
consumes: `Dashboard` rebases each source against the first frame it saw
(`clk.origin`) and only ever uses `frame->timestamp - clk.origin`
(`Dashboard.cpp:1749-1756`).

- `tNs[i]` is nanoseconds since the publisher's origin for `sourceIds[i]`, as an int64.
  Non-decreasing within an epoch. The origin is the publisher's own frame-clock origin for
  that source; it resets only when the epoch resets it.
- `clock.originUnixMs` in the structure gives the wall-clock instant of `tNs == 0`, for
  display and log correlation only. It is never used for axis math.
- The viewer reconstructs a local `steady_clock::time_point` as
  `attachAnchorLocal + (tNs - tNsAtAttach)`. Deltas are faithful, so plot geometry is
  faithful; the absolute value is derived from a local reference, which is exactly what
  "source owns time ... where a display axis needs a local reference, it is derived, not
  substituted" means across a process boundary.
- The viewer never substitutes its own clock for a delta and never re-stamps.

### 7.2 Multi-source time

`tNs` is an array, one entry per `sourceIds` entry, because `Dashboard` maintains an
independent clock origin per source. A single scalar timestamp would be wrong the moment a
project has two sources, and the cost of the array is a few bytes per tick.

### 7.3 Value precision

`values` are JSON numbers. By default (`precision: 0`) the publisher emits the shortest
round-trip representation of the double, which is what a plot needs and what
[`bandwidth.md`](./bandwidth.md) measures as the worst case: ~19-32 bytes per dataset per
tick.

A client may request `precision: N` (significant digits, `1..17`) in `mirror.subscribe`.
At `N = 6` the measured cost roughly halves with no visible difference on a gauge. v1
defaults to full precision because losing plot fidelity by default is the worse failure;
the parameter exists so a bandwidth-constrained viewer can trade it deliberately.

## 8. Size budget

Measured, not estimated — see [`bandwidth.md`](./bandwidth.md) for method and full tables.

| Quantity | Measured | Cap | Headroom |
|----------|----------|-----|----------|
| Snapshot, 59 datasets, full precision | 1,136 B | 1 MB/message | ~920x |
| Mirror rate, 59 datasets @ 60 Hz | 66.5 KB/s | 128 MB/s byte rate | ~1,970x |
| Mirror rate, 59 datasets @ 20 Hz (default) | 22.2 KB/s | 128 MB/s | ~5,900x |
| Structure, marginal | 374 B/dataset | 1 MB/message | binds at ~2,784 datasets |

The property R11 asks for is algebraic, not empirical: mirror cost is
`O(datasets x effectiveHz)` and contains no term for the device rate. AC6 confirms it on a
live pair; the fixture math is why it is expected to hold.

Message-rate cap: the mirror consumes zero of the client's 200 msg/s inbound budget (pushes
are outbound). A viewer's own request traffic is a handful of messages per attach.

## 9. Compatibility with existing API clients (T6)

Every server→client push in `app/src/API/Server.cpp` today, and why a new top-level
`mirror` key cannot be confused for any of them:

| Push | Site | Discriminator | Effect of a `mirror` key |
|------|------|---------------|--------------------------|
| `{"frames":[{"data":{...}},...]}` | `ServerWorker::processItems`, `Server.cpp:307-317` | presence of `frames` | none — a `mirror` push has no `frames` key |
| `{"data":"<base64>"}` | `ServerWorker::writeRawData`, `Server.cpp:216-219` | presence of `data` | none — `mirror` payloads nest `data` only inside `structureChunk`, never at top level |
| `{"event":"<name>"}` | `ServerWorker::broadcastEvent` via `Server::broadcastLifecycleEvent`, `Server.cpp:764-769` | presence of `event` | none |
| `{"type":"response","id":...,"success":...}` | `CommandResponse::toJsonBytes` via `sendResponseToSocket` | `type == "response"` | none — a `mirror` push carries no `type` |
| MCP JSON-RPC responses | `MCPHandler::processMessage`, `Server.cpp:926-934` | `jsonrpc == "2.0"` | none |

None of the four non-MCP pushes carries a `type` field, so every existing client already
distinguishes them by key sniffing. A client that key-sniffs sees an object with no key it
recognizes and ignores it; `tests/utils/api_client.py::_recv_message` returns it to the
caller, and `command()` skips any object that is not the response it is waiting for
(`api_client.py:159`). No existing shape changes meaning.

**`streamFrames` defaults to `true`.** The new per-connection flag is only ever set to
`false` by an explicit `mirror.subscribe {"frames": false}`. A client that never sends that
command keeps receiving the frame broadcast byte-for-byte as today. This is the whole
reason the opt-out is a per-connection flag rather than a global setting.

**Async-sink accounting is deliberately unchanged.** `FrameBuilder::refreshAnyAsyncSink()`
counts the API server whenever `enabled() && clientCount() > 0`, without asking whether any
client wants frames. Making it subscription-aware is a cached-hotpath-flag change and is
out of v1 on purpose (`plan.md`, "Not receiving the firehose"), so attaching a viewer costs
the capture exactly what connecting any API client costs today.

### 9.1 Regression checks for T24

The maintainer runs these against an unmodified `tests/utils/api_client.py` while a mirror
viewer is attached to the same instance:

1. `pytest tests/integration/ -v` passes with a viewer attached — no existing behavior
   changes when another connection is subscribed.
2. A plain `SerialStudioClient` connected alongside a subscribed viewer still receives
   `{"frames":[...]}` at the capture rate (the default `streamFrames: true`).
3. A plain client's `command()` round-trips correctly while `{"mirror":...}` pushes are
   interleaved on the *viewer's* socket and, in a second run, on its own socket after it
   subscribes.
4. `api.getCommands` lists `mirror.getInfo` and `mirror.getStructure` and does not list the
   connection-scoped commands (they are not registry entries); `mirror.getInfo`'s
   `commands` field is where a client discovers those.
5. MCP `tools/list` still resolves, and an MCP session on one socket is unaffected by a
   mirror subscription on another.
6. Disconnecting a subscribed viewer leaves the plain client's session and the capture
   untouched (`Server::onSocketDisconnected` session-id guard).

## 10. Error codes

Mirror failures use new string constants added to `API::ErrorCode`
(`app/src/API/CommandProtocol.h`) in T9/T10. Adding constants is additive; no existing code
changes meaning.

| Code | Raised when |
|------|-------------|
| `MIRROR_VERSION_MISMATCH` | `mirror.subscribe` carries a `wireVersion` the server does not speak |
| `MIRROR_RATE_OUT_OF_RANGE` | `hz` outside `[1, 60]` on subscribe or `setRate` |
| `MIRROR_NOT_SUBSCRIBED` | `setRate` / `unsubscribe` on a connection with no subscription |
| `MIRROR_VIEWER_LIMIT` | subscribe would exceed `API/MaxViewers` |
| `MIRROR_STRUCTURE_TOO_LARGE` | the structure needs more than 64 chunks |
| `EXECUTION_ERROR` "Authentication required" | existing code, unchanged; drives the lazy handshake in §3.1 |

Client-side failures that are not server errors — layout-hash mismatch, unknown epoch,
undecodable chunk sequence — are handled by dropping and refetching (§6.4), never by
rendering.

## 11. Conformance assets

| Asset | Role |
|-------|------|
| `tests/utils/mirror_client.py` | Reference codec + client. Drives a live socket or a recorded file. |
| `tests/fixtures/mirror/generate_fixtures.py` | Builds the fixtures from checked-in example projects. `--check` fails on drift. |
| `tests/fixtures/mirror/{small,wide,multisource}.ndjson` | Recorded structure + snapshot streams. |
| `tests/fixtures/mirror/edge/*.ndjson` | Epoch mismatch, hash mismatch, version mismatch, forward compatibility, length mismatch. |
| `tests/fixtures/mirror/manifest.json` | Per-fixture dataset counts, source ids, layout hashes, sizes. |
| `tests/fixtures/mirror/measure_bandwidth.py` | Produces every number in `bandwidth.md`. |
| `tests/unit/test_mirror_protocol.py` | Conformance tests. No app, no network. |
| `tests/manual/mirror_bandwidth_live.py` | AC6 harness against two live instances. Maintainer-run. |

When `MirrorProtocol.h` lands (T7), its field names and layout-hash function are checked
against this document and against the fixtures before anything else in P1 proceeds.
