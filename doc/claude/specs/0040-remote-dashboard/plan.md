---
spec: 0040-remote-dashboard
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
# Retro-flip 2026-07-25: campaign-level approval (spec 0030); phase gate not run.
updated: 2026-07-25
---

# Plan 0040 — Remote dashboard attach

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.
>
> **Dependency gate.** Everything from T9 onward is blocked on spec 0039 **M2**. T1-T8 are
> protocol, fixture, and measurement work with no dependency on 0039 and no app-code changes.
> The gate is restated inline in `tasks.md`, not just here.

## Approach (one paragraph)

The dashboard is mirrored, not the frame stream. A new **publisher** on the capture side wakes
on `UI::Dashboard::updated()` — a signal that is already coalesced to the UI tick, never the
parse rate — serializes the project structure once per structure epoch and a compact positional
value snapshot per mirror tick, and writes both to *subscribed* API connections through the
existing newline-delimited JSON transport on port 7777. A new **client** on the viewing side
loads the mirrored structure through the ordinary project-load path, then publishes each
arriving snapshot as a `TimestampedFramePtr` straight into `UI::Dashboard::hotpathRxFrame` —
the same entry the local `FrameBuilder` uses, so every push table, ring, alarm tracker, widget
model, and QML binding downstream is the code that already exists and cannot tell the
difference. The mirror is additive on the wire (a new push key alongside `frames` / `data` /
`event`, and a subscription that also opts the viewer *out* of the per-frame firehose), and
additive in the tree (four new TUs plus one handler); the only edit to a file on the capture
data path is a single new input to `Dashboard::streamAvailable()`'s cached flag, wired per the
dataflow rule.

## Transport decision (the roadmap's open question, answered)

The roadmap asks: *"is the existing gRPC transport the mirror channel, or a dedicated stream?"*
**Neither — it is the existing TCP/JSON API transport, with a new subscribed message kind.**

| Candidate | Verdict |
|-----------|---------|
| **Existing TCP/JSON API** (`app/src/API/Server.cpp`, port 7777) | **Chosen.** Ships enabled-by-a-switch in every build; already has framing, a token handshake for non-loopback peers, per-connection session identity, size/rate/connection caps, a worker thread for socket I/O, and a Python client the test suite already uses. Adding a message kind costs nothing on the wire and nothing at build time. |
| **gRPC** (`app/src/API/GRPC/`, port 8888) | **Rejected.** `ENABLE_GRPC` defaults **OFF** (`CMakeLists.txt:85`) and requires system gRPC + protobuf via `find_package(... REQUIRED)`; shipped binaries do not have it, so it cannot be *the* channel for a shipped feature. It is also bound with `grpc::InsecureServerCredentials()` and performs **no token check at all** — strictly worse than the TCP path it mirrors. Its `StreamFrames` server-streaming RPC is real and can carry the same payload later as a parity surface; that is a P2+ nicety, not v1. |
| **A dedicated mirror socket** | **Rejected.** A second listening port means a second bind decision, a second auth story, a second firewall rule, and a second thing to get wrong — for a channel whose measured cost (below) fits inside the existing one with three orders of magnitude to spare. |
| **MQTT publisher → broker → MQTT driver** | **Rejected.** The only existing way two instances share a live stream, but it moves raw frame bytes through a broker, is Pro-only on both ends, and mirrors data rather than a dashboard. |

### Bandwidth — why display cadence is the whole design

The two rates are not close, and the gap is what makes the feature tractable.

- **Capture rate**: the gated hotpath tiers run to 1.024 MHz; the product's headline is 256 kHz.
  The API server's existing behavior at that rate is to serialize *every* parsed frame
  (`ServerWorker::processItems`, `Server.cpp:300-324`) — a full `serialize(Frame)` emitting
  ~25 config keys per dataset — and broadcast it to every connected socket. At 256 kHz with a
  modest project this is tens to hundreds of MB/s; it exceeds the server's own
  `kMaxApiBytesPerWindow` (128 MB/s, `Server.cpp:52`) and would *disconnect the viewer* long
  before the network did. Subscribing a dashboard to the frame stream is not a slow design, it
  is a non-functional one.
- **Display rate**: `UI::Dashboard` already coalesces everything to `Misc::TimerEvents::uiTimeout`
  (`Dashboard.cpp:243-248`) — 60 Hz default, user-configurable 1-240. A human reading a gauge
  needs nothing above that, and a remote viewer needs less.
- **Mirror rate**: negotiated, **default 20 Hz**, clamped `[1, 60]`. A positional numeric
  snapshot costs roughly 10-12 bytes per dataset on the wire. 200 datasets ≈ 2.4 KB/tick ≈
  **~48 KB/s**; 1,000 datasets ≈ 12 KB/tick ≈ **~240 KB/s**. Both sit far inside
  `kMaxApiMessageBytes` (1 MB — the message cap only binds past ~80,000 datasets) and inside
  the byte-rate cap with three orders of magnitude of headroom.
- **The property that matters (R11)**: mirror cost is `O(datasets × mirrorHz)` and contains no
  term for the device rate. A 10 Hz sensor and a 256 kHz audio source cost the viewer the same.
  AC6 measures this rather than asserting it.

### Not receiving the firehose

`processItems` writes to every socket unconditionally; there is no subscription mechanism on
the TCP API today (the MCP `resources/subscribe` handler stores a URI list nothing ever reads —
`MCPHandler.cpp:421-455`). A mirror viewer therefore must be able to turn the frame broadcast
*off for itself*:

- `ConnectionState` (`Server.h:149-159`) gains `streamFrames` (**default `true`** — existing
  clients see no change), `mirrorSubscribed`, and `mirrorHz`.
- `ServerWorker` gains a parallel per-socket flag, set by a queued `setSocketStreamFrames(socket,
  sessionId, bool)` (same session-id-tagged pattern as `writeToSocket`, `Server.cpp:262-278`),
  and `processItems` skips sockets whose flag is false.
- **The serialization itself is not skipped in v1.** Whether any client wants frames feeds
  `FrameBuilder::refreshAnyAsyncSink()`, which is a *cached hotpath flag*
  (`FrameBuilder.cpp:583-600`, counts the API server when `enabled() && clientCount() > 0`).
  Making that accounting subscription-aware is a real optimization and a real cached-flag
  change; it is deliberately **out of v1** so that attaching a viewer costs the capture exactly
  what connecting any API client costs today — a known, already-shipped cost, not a new one.
  Recorded as a P2 follow-up.

## Affected subsystems & files

| File | Change |
|------|--------|
| `doc/claude/specs/0040-remote-dashboard/wire-protocol.md` | **New (P0).** The normative wire contract: message kinds, field names, epoch rules, handshake, versioning, error cases. The single source of truth both implementations are written against. |
| `tests/utils/mirror_client.py` | **New (P0).** Python mirror client (auth handshake, subscribe, structure + snapshot decode, reconnect). Sibling to `api_client.py`, reusing its NDJSON framing. |
| `tests/fixtures/mirror/` | **New (P0).** Recorded structure + snapshot streams for a small and a wide project, used by the codec tests and the bandwidth measurement. |
| `app/src/API/Mirror/MirrorProtocol.h` | **New.** Pure encode/decode: wire version constant, message-kind tags, `encodeStructure`/`encodeSnapshot`/`decode*`, epoch and layout-hash helpers. No Qt UI, no singletons — unit-testable on spec 0032's target. |
| `app/src/API/Mirror/MirrorPublisher.{h,cpp}` | **New.** Capture side. Subscribes to `Dashboard::updated`, `widgetCountChanged`, `dataReset`; owns the epoch counter, the rate divider per subscriber, and the snapshot buffer; hands encoded payloads to `API::Server` for delivery. Does nothing with zero subscribers. |
| `app/src/API/Mirror/MirrorClient.{h,cpp}` | **New.** Viewer side, transport half. Owns its `QTcpSocket`, performs the token handshake, issues `mirror.getInfo` / `mirror.getStructure` / `mirror.subscribe`, decodes push payloads, tracks staleness with a watchdog, and reconnects with backoff. |
| `app/src/API/Mirror/MirrorSession.{h,cpp}` | **New.** Viewer side, session half. Attach/detach lifecycle: snapshot and restore the local session state, load the mirrored structure, build the frame template, publish each snapshot as a `TimestampedFramePtr`, expose `attached` / `endpoint` / `live` / `stale` to QML. This is the class that takes the 0039 `SessionContext`. |
| `app/src/API/Handlers/MirrorHandler.{h,cpp}` | **New.** Stateless registry commands: `mirror.getInfo`, `mirror.getStructure`. Registered in `CommandHandler::initializeHandlers()` (`CommandHandler.cpp:224-272`). |
| `app/src/API/Server.{h,cpp}` | `ConnectionState` gains `streamFrames` / `mirrorSubscribed` / `mirrorHz`; `handleJsonMessage` (`Server.cpp:916-966`) gains a mirror branch **before** the generic command dispatch (the same shape as the existing MCP branch, because the stateful commands need the socket); `ServerWorker` gains the per-socket `streamFrames` flag and honors it in `processItems`; a `sendMirrorPayload(sessionId, bytes)` path reusing `writeToSocket`. |
| `app/src/API/CommandHandler.cpp` | One line registering `MirrorHandler`. |
| `app/src/UI/Dashboard.{h,cpp}` | **The only capture-path file edited.** `streamAvailable()` (`Dashboard.cpp:391-411`) gains a mirror-attached input; `connectStreamAvailableInputs()` (`Dashboard.cpp:423-440`) wires `MirrorSession::attachedChanged` into `updateStreamAvailable()` with `Qt::DirectConnection`. No change to `hotpathRxFrame`. |
| `app/src/Misc/CLI.{h,cpp}` | New flags for R16: `--api-external` (non-interactive equivalent of the modal "Allow External API Connections" confirmation) and `--api-token <hex>` (set/pin the token so a headless box can be provisioned). Applied in `applyProjectAndAutoConnect` alongside the existing `--api-server` handling (`CLI.cpp:428-431`). |
| `app/src/Misc/ModuleManager.cpp` | Construct `MirrorPublisher` and `MirrorSession` in `setupCrossModuleConnections()` **after** `instantiateCoreModules()` returns; register `Cpp_API_Mirror` context property next to `Cpp_API_Server` (`ModuleManager.cpp:757`). `instantiateCoreModules()` (`:620-656`) is **not** edited. |
| `app/CMakeLists.txt` | New sources + headers. Gated on `BUILD_COMMERCIAL` only if the Pro question below is answered "yes". |
| `app/qml/Dialogs/RemoteAttach.qml` | **New.** Host / port / token, remembered endpoints, connect / disconnect, live status and error text. |
| `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`, `app/qml/MainWindow/Panes/Setup.qml` | Remote/stale indicator, following the existing `Cpp_API_Server.enabled` + `clientCount` indicator pattern already in both files. |
| `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`, `app/rcc/icons/commands/{16,24,32,48}/` | One command (`app.remoteAttach`) + one binding + one tiered icon, per the spec-0028 recipe. `scripts/registry-verify.py` gates it. |
| `doc/help/` + `help.json` | Trust-model and usage page (R15, AC9). |
| `doc/claude/architecture/remote.md` + a row in `doc/claude/architecture.md` | The subsystem doc: wire contract summary, the mirror seam, what is and is not mirrored, the cached-flag input. |
| `CLAUDE.md` | Two lines pointing at the new architecture doc under the existing structure. |
| `tests/integration/test_remote_mirror.py` | **New.** Maintainer-run: attach, structure epoch, live values, detach, reattach, two viewers, staleness, auth refusal. |

## Architecture & data flow

```
CAPTURE SIDE (headless or windowed)                VIEWING SIDE (GUI)
─────────────────────────────────────              ──────────────────────────────────────
Driver → FrameReader → FrameBuilder                MirrorClient (own QTcpSocket)
  │  (unchanged, per-frame, untouched)               │ handshake → getInfo → getStructure
  ▼                                                  │ subscribe{hz, frames:false}
UI::Dashboard::hotpathRxFrame                        ▼
  │ push tables, rings, m_lastFrame                MirrorSession
  │                                                  │ structure epoch → ProjectModel load
  ▼  uiTimeout coalesce (60 Hz default)              │ builds Frame template once per epoch
Dashboard::updated()  ────────────────┐              │ per snapshot: assign values positionally
                                      │              ▼
MirrorPublisher (rate-divided, 20 Hz) │        UI::Dashboard::hotpathRxFrame
  │ epoch? → encodeStructure          │              │  (the SAME entry FrameBuilder uses)
  │ else   → encodeSnapshot           │              ▼
  ▼                                   │        push tables → rings → updated()
API::Server → ServerWorker::writeToSocket             │
  │  (worker thread, session-id tagged)                ▼
  ▼                                              widget models → QML (unchanged)
  TCP 7777, NDJSON  ═══════════════════════════════════╝
```

**Why `hotpathRxFrame` is the injection point.** Every widget model captures
`UI::Dashboard::instance()` by reference in its constructor init list (`Bar.cpp:47`,
`Plot.cpp:38`, `DataGrid.cpp:136`, ...) and listens to `Dashboard::updated`; the helper macros
`GET_GROUP` / `GET_DATASET` / `VALIDATE_WIDGET` / `FMT_VAL` at the bottom of `Dashboard.h` call
`UI::Dashboard::instance()` directly. QML reaches the same object through `Cpp_UI_Dashboard`.
There is exactly one place where "what the dashboard shows" is decided, and feeding *that* makes
the QML-cannot-tell requirement true by construction. A mirrored-item-model layer with an object
broker — the pattern the roadmap's research note records — would instead require a parallel
proxy per widget type, tracking every model role, with fidelity as an ongoing liability. That
option is recorded as rejected below.

**Structure epochs.** The publisher increments an epoch on `widgetCountChanged` / `dataReset`
and stamps every snapshot with it. A snapshot whose epoch the client does not hold is dropped
and triggers a `mirror.getStructure` re-fetch. This is the whole consistency story: no partial
structure updates, no diffing, no ordering assumptions across two message kinds.

**Value snapshots are positional.** Values are emitted per source in row-major group/dataset
order — the identical order `Dashboard::buildValuePushes` walks — so the wire carries an array
of numbers, not a map of ids. The structure message carries the ordered `(sourceId, uniqueId)`
list plus a layout hash; a mismatched hash is treated exactly like an unknown epoch. Non-numeric
datasets carry their string in a sparse side-map keyed by position, because most projects have
few of them and paying per-dataset string keys on every tick is the thing the positional format
exists to avoid.

**Timestamps.** The snapshot carries the remote's frame timestamp. The viewer does **not**
re-stamp: it passes the received value into the `TimestampedFrame` it publishes, and the
dashboard's display clock rebases for the time axis exactly as it does for a local source. This
is the "source owns time" rule applied across a process boundary; the plan's failure mode if
ignored is a plot whose X axis is the viewer's clock drift.

**Detach.** `MirrorSession` snapshots the local session state on attach (project document,
operation mode, plot time range, frozen flag) and restores it on detach; the remote is told
nothing beyond the socket closing, so the capture is unaffected (R4/R5). v1 refuses to attach
while the local session has a device or player open — the single-Dashboard design cannot hold
both, and pretending otherwise is how a user loses a capture.

**Staleness.** `MirrorClient` runs a watchdog at `3 × (1/mirrorHz)` bounded to `[500 ms, 3 s]`.
On expiry it sets `stale`, which the QML indicator surfaces and which stops the dashboard from
presenting last values as current (R9/AC4). Socket loss triggers exponential reconnect
(1 s → 30 s cap) with the structure re-fetched on every successful reconnect.

**Multi-viewer.** The publisher builds one snapshot per tick and fans it out; per-subscriber
state is a rate divider and a last-sent epoch. N viewers cost N socket writes, not N snapshots.
The existing `kMaxApiClients` (32) is the ceiling, and a new `maxViewers` setting (default: all
allowed while the API server is on) can lower it.

## Hotpath & threading impact

- **Touches the hotpath?** **One input, named up front.** No per-frame code is edited: not
  `FrameReader`, not `CircularBuffer`, not `FrameBuilder`, not `Dashboard::hotpathRxFrame`, not
  the span fast lane. The single edit on the capture path is
  `UI::Dashboard::streamAvailable()` / `connectStreamAvailableInputs()` gaining a
  mirror-attached input, on the **viewing** side's behalf — without it an attached dashboard
  early-returns at `Dashboard.cpp:1735` and shows nothing. Per the cached-flag rule the change
  signal (`MirrorSession::attachedChanged`) is wired into `updateStreamAvailable()` with
  `Qt::DirectConnection`; a queued refresh would lag a full event-loop turn behind snapshots
  already arriving. `--benchmark-hotpath` is a required gate for this spec because of this one
  edit, even though the edit adds no per-frame work.
- **New cross-thread signal/slot?** Publisher → socket write reuses the existing
  `QMetaObject::invokeMethod(worker, "writeToSocket", Qt::QueuedConnection, ...)` path with the
  session id carried and verified — the pattern that exists specifically because socket pointers
  are reused by the allocator (`Server.cpp:1348-1353`, the macOS ABA regression). No new thread,
  no new queue. `MirrorClient`'s socket lives on the main thread: at 20 Hz there is nothing to
  offload, and a worker thread would buy a lifetime problem for no throughput.
- **New input to a cached hotpath flag?** Yes, exactly one: `Dashboard::m_streamAvailable`, as
  above. `m_anyAsyncSink` is **not** given a new input — subscription-aware async-sink accounting
  is explicitly deferred (see Transport decision). `m_captureLatestFrame`, `m_changeDriven`,
  `m_operationMode`, and `m_playerOpen` gain nothing.
- **Timestamp ownership** — the capture side stamps at its driver boundary as today; the mirror
  carries that value; the viewer never re-stamps. Export and report workers on the viewing side
  are not fed by the mirror at all (see below), so `monotonicFrameNs` is not involved.

**Deliberately not fed on the viewing side:** the mirror publishes into `Dashboard` only. It does
**not** go through `FrameBuilder::hotpathTxFrame`, so a viewer's CSV/MDF4/Sessions/MQTT/API sinks
never see mirrored frames. Recording a remote session locally is a P2 question with its own
correctness story (gaps, epochs, clock domains); v1 must not half-answer it by accident.

## Data model & persistence

- **No `Frame.h` `Keys::` additions and no project-JSON change.** The structure payload is the
  existing serialized project document plus a small envelope (wire version, epoch, layout hash,
  ordered dataset list, operation mode, plot time range, frozen flag). Nothing new is persisted
  in a `.ssproj`.
- **No schema or writer version bump.** The mirror is a transport, not a format.
- **New QSettings keys** (viewer side): remembered endpoints list, last endpoint, mirror rate
  preference. (Capture side): optional `API/MaxViewers`. The existing `API/Enabled`,
  `API/ExternalConnections`, `API/AuthToken` keys are reused unchanged; `--api-external` and
  `--api-token` write the latter two, which is precisely what makes a headless box provisionable
  (R16) without the modal confirmation at `Server.cpp:497-514` that an offscreen process cannot
  answer.
- **Wire versioning.** `MirrorProtocol.h` carries an integer wire version; `mirror.getInfo`
  returns it; a client refusing an incompatible version says so in the dialog rather than
  half-rendering. Forward compatibility rule: unknown fields are ignored, unknown message kinds
  are dropped, and the version is bumped only for changes that break either rule.

## API / SDK surface

Additive; no existing command, response, or push envelope changes meaning.

- **Registry commands** (stateless, discoverable, also reachable from MCP and the SDK):
  `mirror.getInfo` — wire version, app version, session id, current epoch, layout hash,
  operation mode, dataset count, viewers-allowed; `mirror.getStructure` — the full structure
  payload for the current epoch.
- **Connection-scoped control**, handled in `Server::handleJsonMessage` *before* generic
  dispatch, because `CommandRegistry::execute` has no connection context and the MCP branch
  already establishes this precedent for per-session state: `mirror.subscribe {hz, frames}`,
  `mirror.setRate {hz}`, `mirror.unsubscribe`. Their existence is advertised by `mirror.getInfo`
  so a client is not required to guess; the split (stateless in the registry, stateful at the
  socket) is a real seam in the current design and is documented rather than papered over.
- **New server push envelope**: `{"mirror":{...}}`. Existing clients distinguish pushes by key
  presence (`frames` / `data` / `event` — none of them carry a `type` field), so a new top-level
  key is invisible to them.
- `EnumLabels.cpp` is untouched; no new enum reaches the API.
- Commercial gating: if remote attach is Pro (open question), the *attach* commands and the
  `MirrorSession`/dialog compile behind `BUILD_COMMERCIAL`; `MirrorPublisher` and the
  `mirror.*` server commands stay ungated so a GPL headless capture can still be watched.

## QML / UI

- `RemoteAttach.qml` — host, port, token, remembered endpoints, connect/disconnect, and an
  error line that distinguishes refused / unauthorized / version-mismatch / unreachable. Follows
  the existing dialog grammar; no ComboBox restore-race surface beyond the endpoint list, which
  guards `onCurrentIndexChanged` with the standard `if (count <= 0) return`.
- Status: the attached endpoint and the live/stale state render in the two places that already
  carry the API-server indicator (`Setup.qml`, `DashboardLayout.qml`), reusing that component's
  shape so it reads as one system.
- Command surface: one `app.remoteAttach` entry in `app/rcc/commands/app.json`, one binding in
  `AppCommandBindings.qml`, one tiered icon; `scripts/registry-verify.py` gates layout, tiers,
  qrc sync, and the commercial guard.
- Widgets that v1 cannot drive (`FFTPlot`, `Waterfall`, `Plot3D`) render an explicit
  unavailable-over-remote state (R10/AC5) driven by a single `MirrorSession.attached` binding —
  not an empty widget, and not a zeroed one.

## Session context dependency — what 0039 actually has to deliver

This is the load-bearing dependency and the honest version is more precise than "R7 depends on
R4".

**Why not today.** A GUI process has exactly one `UI::Dashboard` and one
`DataModel::ProjectModel`, both Meyers singletons, both captured by reference in the constructor
init lists of every widget model and reached by macro from `Dashboard.h`. "Attach" in the naive
form means loading the remote project over the local one and never being able to put it back —
which is a demo, not the feature, and is exactly the shortcut this plan is written to prevent.

**What v1 needs: 0039 M2.** M2 is where the context stops forwarding and starts *owning* the
session subsystems. What this plan consumes from it: the ability to snapshot and restore the
session's project document and dashboard-facing state as one unit, so attach is reversible and
detach genuinely returns the user to where they were. v1 keeps **one** dashboard object and
swaps its *feed*; that is expressible at M2.

**What v1 does not need: 0039 M3.** M3 (plurality — a second constructible context, scoped
`current()`) is what would let a GUI hold its own live capture *and* a remote view at the same
time. v1 explicitly refuses to attach while the local session is capturing, so it does not need
M3. Concurrent local + remote, and more than one simultaneous remote view, are P2/P3 and should
be specced against M3 when it exists.

**Therefore the gate is M2, and it is hard.** Tasks T9 onward do not start before M2 lands. The
pre-gate tasks (T1-T8) are deliberately chosen so that the protocol, the fixtures, the test
client, and the bandwidth evidence all exist and are reviewable while 0039 is in flight — if the
measurement invalidates the design, that is discovered before any app code exists.

**Soft dependencies.** Spec 0032 (C++ unit tier) is where `MirrorProtocol.h`'s codec tests want
to live; if it has not landed, they land as fixture-driven Python tests instead. Spec 0036
(property registry) would let the structure payload be generated rather than hand-serialized;
it is a simplification, not a gate.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Mirror channel | existing TCP/JSON API / gRPC / dedicated socket / MQTT | **Existing TCP/JSON API** — the only one present and enabled in shipped builds, and it already has auth, framing, caps, and a test client. gRPC is off by default and has no auth at all. |
| Mirror cadence | parse rate / local UI rate / negotiated rate | **Negotiated, default 20 Hz** — decouples wire cost from both the device rate (R11) and the viewer's 1-240 Hz UI setting, so a 240 Hz viewer cannot multiply bandwidth. |
| What is mirrored | frames / dataset values / item models | **Dataset values + structure** — values are what the dashboard displays; frames are the firehose; item models require a proxy per widget type with permanent fidelity risk. |
| Injection point | `FrameBuilder::replayChannels` / new `FrameBuilder` entry / `Dashboard::hotpathRxFrame` | **`Dashboard::hotpathRxFrame`** — mirrored values are already post-transform, so re-entering the parser would re-run transforms the remote already ran (and would require shipping the user's scripts to the viewer). It also leaves `FrameBuilder.cpp` completely untouched, which is worth a great deal. |
| Structure sync | full resend on epoch / incremental diff | **Full resend** — structure changes are rare and human-paced; a diff protocol is a consistency bug generator for no measurable gain. |
| Value encoding | keyed by uniqueId / positional array | **Positional**, with an ordered id list and a layout hash in the structure message — the format the size budget depends on, and the mismatch case is already handled by the epoch machinery. |
| Firehose opt-out | server-side flag per connection / client ignores it / make frames opt-in globally | **Per-connection flag defaulting to on** — the only option that does not break an existing client, and it costs one bool in `ConnectionState`. |
| Async-sink accounting | make it subscription-aware now / leave as-is | **Leave as-is** — it is a cached hotpath flag; changing it is a separate, reviewable change, and leaving it means attaching costs exactly what any API client costs today. |
| Session model | one dashboard, swapped feed / two concurrent contexts | **One dashboard, swapped feed** — needs 0039 M2 rather than M3, and does not require converting every widget model to context injection (far beyond M3's pilot scope). Concurrency is P2/P3. |
| Control channel | in v1 / deferred | **Deferred to P3** — the current credential is all-or-nothing and unencrypted; shipping remote control on it would be the wrong first move. |
| Transport security | add TLS now / LAN-trust v1 | **LAN-trust v1, documented plainly** — TLS is a transport-wide change affecting every existing client and the test suite; doing it inside a feature spec is how it gets done badly. P4, and named as a prerequisite for P3. |
| Headless opt-in | new CLI flags / reuse persisted settings only | **New flags** (`--api-external`, `--api-token`) — the external-connections toggle is behind a modal confirmation a windowless process cannot answer, so without them a headless-only machine literally cannot be made attachable. |
| Unsupported widgets | render empty / render an explicit state | **Explicit state** — an empty spectrum is indistinguishable from a broken one, and this is the failure class the product keeps paying for. |

## Risks & mitigations

- **0039 M2 slips or lands differently than assumed.** The whole implementation half is gated on
  it. Mitigation: pre-gate tasks produce standalone, reviewable value (protocol, fixtures,
  measurement); the plan states the M2 shape it needs in one paragraph so a mismatch is caught
  when 0039's plan is reviewed, not when T9 starts.
- **The firehose floods a viewer before it can opt out.** A client that connects and reads
  slowly gets `{"frames":[...]}` at the capture rate from the first flush. Mitigation: the
  subscribe request is the *first* message after the handshake, and the client sets
  `frames:false` in it; the wire contract makes that ordering normative and the Python client
  demonstrates it.
- **The cached stream-available flag is wired queued by accident.** The documented silent-breakage
  class: frames arrive, the flag lags a turn, the first snapshots vanish. Mitigation: the
  connection type is stated in the plan, checked at the task level, and named in the review.
- **Stale data presented as live.** The worst possible failure for a monitoring feature — an
  engineer reads a frozen gauge as a healthy one. Mitigation: the watchdog and the explicit stale
  state are a requirement (R9) with their own acceptance criterion (AC4), not a polish item.
- **Structure/value race after a remote project edit.** Mitigation: epoch on every snapshot;
  unknown epoch or layout-hash mismatch drops the snapshot and re-fetches structure. There is no
  code path that applies values against a structure it does not match.
- **Detach loses the user's local project.** Mitigation: attach refuses while the local session
  is capturing; local session state is snapshotted before load and restored on detach; a task
  verifies restore explicitly, including after an abnormal disconnect.
- **License divergence between the two sides.** The known 2026-07-09 failure class: state derived
  at load time from `proWidgetsEnabled()` bakes in fallback widgets. A mirrored structure loaded
  on a differently-licensed viewer is exactly that shape. Mitigation: the renderer's license
  governs (open question, recommended), unsupported widgets get the explicit unavailable state
  rather than a silent downgrade, and the structure load path is checked against
  `activatedChanged` re-derivation.
- **Wide projects exceed the message cap.** 1 MB binds at roughly 80,000 datasets, but the
  *structure* message is much larger per dataset (~25 config keys). Mitigation: the wide-project
  fixture is part of P0 precisely to measure this; if the structure message approaches the cap,
  the answer is chunked structure delivery, decided with a number in hand rather than a guess.
- **Two viewers, one rate divider.** A per-subscriber divider driven off a shared tick can drift
  or double-send. Mitigation: last-sent epoch and last-sent tick per subscriber; the multi-viewer
  test asserts consistency between viewers, not just liveness.
- **Scope creep into remote control.** Every review of this feature will suggest "and you could
  also trigger actions". Mitigation: the read-only boundary is a spec non-goal, P3 exists, and
  the connection carries no write capability the mirror added.

## Test & verification plan

- **Unit (agent can run):** none in `tests/scripts/` — there is no JS surface. `MirrorProtocol.h`
  codec round-trips run against the P0 fixtures: structure encode/decode, snapshot encode/decode,
  epoch mismatch handling, layout-hash mismatch, unknown-field tolerance, version mismatch.
  These run as pure Python against the fixtures pre-gate, and are ported to spec 0032's C++ unit
  target when it exists.
- **Bandwidth measurement (agent can run against fixtures, maintainer confirms live):** the
  AC6 number, recorded in this spec directory as `bandwidth.md` — bytes/second at a low device
  rate and at a rate two orders of magnitude higher, on the same project.
- **Integration (maintainer runs, needs two instances):** `tests/integration/test_remote_mirror.py`
  — attach and read live values; structure epoch after a remote project change; detach leaves the
  capture running (verified against the remote's own CSV output); reattach; two simultaneous
  viewers agree; severed link marks stale within bound and recovers; auth refused with a wrong
  token; attach refused when bound to loopback only. The existing `tests/integration/` suite is
  re-run as a smoke check that no existing API behavior changed.
- **Hotpath:** `--benchmark-hotpath` is **required** (the one cached-flag edit), run before and
  after, compared against the tier table; plus a diff review confirming `Dashboard.cpp` is the
  only capture-path file present and that its change is the named input.
- **Static:** `python scripts/code-verify.py --check` on every changed C++/QML file;
  `python scripts/registry-verify.py` after the command/icon task;
  `python scripts/documentation-verify.py` after the help page; `qt-cpp-review` on the C++ diff;
  `python scripts/sanitize-commit.py` before commit.
