# Architecture — Remote Dashboard Mirror (spec 0040)

> Part of the architecture corpus ([index](../architecture.md)). Read this file in full before
> touching `core/Api/API/Mirror/`, `MirrorHandler`, `RemoteAttach.qml`, or anything that feeds
> `Dashboard::streamAvailable()`. The wire protocol's own spec lives in
> `doc/claude/specs/0040-remote-dashboard/wire-protocol.md`.

One Serial Studio instance (the *publisher*, attached to real hardware) streams its dashboard
to another instance (the *viewer*) over the existing API server socket (port 7777). Four TUs
in `core/Api/API/Mirror/`: `MirrorProtocol.h` (header-only codec, no sockets/UI/singletons),
`MirrorPublisher` (capture side), `MirrorClient` (viewer transport), `MirrorSession` (viewer
lifecycle); plus `API/Handlers/MirrorHandler` and `app/qml/Dialogs/RemoteAttach.qml`.

## Wire protocol (v1)

- **NDJSON on the API socket**; every push wraps in a top-level `"mirror"` key so key-sniffing
  legacy clients ignore it. Kinds: `structure`, `structureChunk`, `snapshot`, `heartbeat`.
  Commands: `mirror.getInfo` / `mirror.getStructure` (registry) and `mirror.subscribe` /
  `mirror.setRate` / `mirror.unsubscribe` (connection-scoped, dispatched before the registry).
- **Positional snapshots under a layout hash.** Structure sends the ordered
  `(sourceId, uniqueId)` list once per epoch; snapshots are bare number arrays. Validity rests
  on the FNV-1a-64 layout hash (seed `"mirror-v1\n"`): the viewer refuses a mismatched epoch
  (`MIRROR_LAYOUT_MISMATCH`), the publisher bumps the epoch instead of publishing a mismatched
  walk. **Any change to dataset ordering, `wireUniqueId`, or the seed is a wire break**:
  bump `kWireVersion` and regenerate `tests/fixtures/mirror/`.
- Strings and non-finite doubles ride sparse side maps (`strings`, `nonFinite` with
  `nan`/`inf`/`-inf`); the array slot holds `null`. Timestamps are relative ns per source
  (`monotonic-relative` + `originUnixMs`).
- Bandwidth: per-viewer rate 1-60 Hz on the wire (default 20) capped by the local UI tick; the
  in-app viewer always requests `kHzMax` and lets the publisher clamp (the RemoteAttach dialog
  has no rate field — rate is automatic); snapshots
  encoded once per distinct precision (`roundSignificant`, 0-17), not per viewer; 1 s
  heartbeat when idle; structures base64-then-chunked at 512 KB / max 64 parts
  (`MIRROR_STRUCTURE_TOO_LARGE` beyond).

## Publisher side — never on the frame path

- `MirrorPublisher` wakes on `UI::Dashboard::updated()` (display tick) and is wired **only
  while a viewer is subscribed**: first subscriber `activate()`, last `deactivate()`, with
  `SS_ASSERT_LOG(!m_updatedLink)` guarding the symmetry. An unwatched instance runs zero
  mirror code per tick.
- Snapshots read `dashboard.rawFrame()` on the UI thread, re-verify the walk against the
  epoch identity list, and bump the epoch on mismatch instead of publishing.
- Delivery: `payloadReady(socket, sessionId, payload)` → `Server::sendMirrorPayload`, which
  re-validates the sessionId and `mirrorSubscribed` before writing (a reused socket pointer
  can't hit the wrong client).
- `mirror.subscribe` opts the connection out of the per-frame firehose;
  **`ConnectionState::streamFrames` defaults `true`** (the legacy-client guarantee) and only
  `mirror.subscribe` may flip it — unsubscribe must not reopen the firehose. Viewer ceiling:
  `API/MaxViewers` QSetting (0 = refuse viewers, API otherwise usable).

## Viewer side — `MirrorSession`

- Injects frames via `dashboard.hotpathRxFrame(...)` from pre-built per-source
  `TimestampedFramePtr` templates (values assigned in place, timestamp rewritten against a
  local anchor). It **never** reaches the sink fan-out — a viewer's export
  sinks never see mirrored data.
- Mirrored frames carry `structureGeneration = kMirrorGenerationBase + epoch`
  (base `1ULL << 48`) so they can't alias a local frame-pool generation.
- **`Dashboard::streamAvailable()` gains `API::MirrorSession::mirroring()` as a leading
  `[[unlikely]]` disjunct** — a plain module-static `bool s_mirroring`, because
  `streamAvailable()` is reached from Dashboard's ctor inside the pinned order and must not
  construct `MirrorSession` (built after `restoreLastProject()`). The flag is written
  **before** `attachedChanged` fires (DirectConnection into
  `Dashboard::updateStreamAvailable`, like every other stream input); reversing that order
  drops the first snapshot.
- A mirrored project loads with an **empty source path** — that is what stops the debounced
  autosave from overwriting the user's own project with the remote layout.
- Attach is exclusive with a local stream (`canAttach` requires no local
  `streamAvailable()`); concurrent local+remote needs spec 0039 M3. Detach must restore the
  captured `LocalState` on every exit path, including fatal transport failure.
- `MirrorPublisher` and `MirrorSession` take `SessionContext&` by injection (spec 0039
  pilots) and are constructed in `ModuleManager` after the pinned order; QML sees
  `Cpp_API_Mirror` (registered in the core, non-commercial block).

## Gating & access

- **No commercial gate today** — spec 0040 leaves publisher/viewer gating an open question;
  both ship in GPL and commercial builds. Access control is transport-level: API auth token +
  loopback binding, `--api-external` / `--api-token <hex>` for headless opt-in. The
  RemoteAttach dialog remembers tokens per endpoint (`Mirror/EndpointTokens` QSettings map,
  plaintext by design — same protection as the publisher's own token); `forgetEndpoint()`
  drops the token with the endpoint, and an attach with an empty token clears any stored one.
- Command surface: `app.remoteAttach` in `app/rcc/commands/app.json` (contexts app +
  dashboard), icons `commands/{16,24,32}/remote-attach.svg`. While attached, the toolbar's
  right-pinned Connect button hides and a "Remote Dashboard" button (same `app.remoteAttach`
  entry) takes its place; per-widget disconnected overlays are suppressed via
  `Cpp_API_Mirror.attached` in `WidgetDelegate.qml`. Detach pops the UI back to the console
  view (`MainWindow.qml` watches `attachedChanged`, mirroring the `onDataReset` handler)
  since the restored local session has no live stream.

## Tests

Codec: `tests/unit/test_mirror_protocol.py` (pure Python, no app) + reference client
`tests/utils/mirror_client.py` + fixtures `tests/fixtures/mirror/` (regenerate via
`generate_fixtures.py` on any wire change). Live: `tests/integration/test_remote_mirror.py`
(maintainer-run, `SS_MIRROR_HOST/PORT/TOKEN`), bandwidth probe
`tests/manual/mirror_bandwidth_live.py`.

## Spec 0055 — the viewer publishes blocks

The publisher is unchanged: it still reads `Dashboard::rawFrame()` on the display tick, so it
carries one value per dataset per tick exactly as before, and the wire format is untouched
(`kWireVersion` stays 1 — nothing about the payload changed, so bumping it would refuse peers for
no reason).

The **viewer** did change, because `Dashboard::hotpathRxFrame` no longer exists. `MirrorSession`
now hands the dashboard a `StructureSnapshot` per mirrored source when it adopts an epoch's layout,
and wraps each snapshot as a **single-sample** `DataBlock` through `Dashboard::applyBlock`. One
sample is exactly what arrived: padding it out to the remote's real rate would invent measurements
the viewer never received.

**Known gap against spec 0055 R4.** The mirror therefore keeps its pre-0055 fidelity — a viewer
watching a 48 kHz source still sees a tick-rate trace, not the signal. Closing that means carrying
sample runs in the snapshot payload, which is a redesign of `encodeSnapshot`, the chunker,
`MirrorClient`'s decode, the viewer's reconstruction and every checked-in fixture. It is deferred
deliberately rather than half-done; the MQTT and gRPC halves of R4 *are* met.
