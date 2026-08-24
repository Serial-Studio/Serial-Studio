---
spec: 0067-opcua-embedded-stack
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-24
amended: 2026-08-24 (crypto backend OpenSSL -> mbedTLS; see Tradeoffs)
---

# Plan 0067 — Embedded OPC UA Stack and Secure Channels

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Vendor open62541's published single-file distribution and mbedTLS 3.6 into `lib/`, build both as
static libraries the repository owns, and wrap them in one
purpose-built facade, `IO::Drivers::OpcUaSession`, that exposes only the six operations the
driver performs: discover endpoints, connect, subscribe, read attributes, browse, and
close. The facade owns the `UA_Client`, drives it from a QTimer calling
`UA_Client_run_iterate()` on the driver's own (GUI) thread using only the `*Async` service
calls, and emits Qt signals carrying plain Qt/POD types so no `UA_*` symbol escapes it.
`OpcUa.cpp` and `OpcUaTagModel.cpp` are rewritten against that surface rather than against Qt
OPC UA's one-handle-per-node model; every behavioral rule spec 0066 established (discover
before dial, host substitution, severity-bit quality, individual monitored-item refusal, the
one-verdict funnel, lazy browse) is preserved verbatim because it lives in the driver, not in
the transport. Security policy, certificate and trust handling become configuration on the
facade, which is what makes stage 2 additive rather than another rewrite.

## Affected subsystems & files

| File | Change |
|------|--------|
| `lib/open62541/` | **New.** Vendored `open62541.c` / `open62541.h` (v1.5.7 release assets, unmodified) plus our `CMakeLists.txt` defining the static target and `target_link_open62541()`. |
| `lib/mbedtls/` | **New.** Vendored Mbed TLS 3.6.7, upstream source with the test/program/doc trees removed. |
| `lib/CMakeLists.txt` | Add the two subdirectories behind `BUILD_COMMERCIAL AND SS_ENABLE_OPCUA`, and a no-op `target_link_open62541()` for builds without the stack. |
| `REUSE.toml`, `LICENSES/MPL-2.0.txt` | Declare the vendored trees; the sources now live in the repository, so REUSE covers them. |
| `app/rcc/messages/Acknowledgements.txt` | MPL-2.0 and Apache-2.0 notices for the two bundled components. |
| `app/CMakeLists.txt` | Drop `OpcUa` from `QT_MODULES`/`QT_LIBS`; include `Open62541.cmake`; call `target_link_open62541()`; add new sources. |
| `CMakeLists.txt` | Declare the `SS_ENABLE_OPCUA` option (R4). |
| `app/src/IO/Drivers/OpcUaSession.h/.cpp` | **New.** The facade: client lifecycle, iterate pump, async request registry, endpoint discovery, subscription, batched read, browse. Only file that includes `open62541/*`. |
| `app/src/IO/Drivers/OpcUaTypes.h` | **New.** Transport-neutral value types replacing the Qt ones: `Endpoint`, `ReferenceRow`, `ReadRow`, `MonitoredValue`, `StatusCode`, `NodeAttribute`, `NodeClass`. Qt Core only. |
| `app/src/IO/Drivers/OpcUaMarshal.h/.cpp` | **New.** `UA_Variant` ⇄ `QVariant`, `UA_NodeId` ⇄ `QString`, status-code → severity/text. The ctest seam. |
| `app/src/IO/Drivers/OpcUaSecurity.h/.cpp` | **New (stage 2).** Client cert/key generation and load, trust store, server-cert verification callback. |
| `app/src/IO/Drivers/OpcUa.h/.cpp` | Rewire against `OpcUaSession`; drop all `QOpcUa*` includes. Stage 2 adds policy/mode/identity config + trust-prompt signal. |
| `app/src/IO/Drivers/OpcUaTagModel.h/.cpp` | Rewire browse + batched attribute read against `OpcUaSession`. |
| `app/src/IO/Drivers/OpcUaWire.h` | **Unchanged** (Qt Core only, already transport-neutral). |
| `app/src/API/Handlers/OpcUaHandler.h/.cpp` | Stage 2: policy/mode/identity/cert/trust commands. |
| `app/qml/.../Drivers/OpcUa.qml` | Stage 2: policy + mode pickers, cert panel, deprecation labels. |
| `app/qml/.../Dialogs/OpcUaTrustDialog.qml` | **New (stage 2).** Server-certificate trust prompt. |
| `app/tests/tst_opcua_marshal.cpp` + `app/tests/CMakeLists.txt` | **New.** ctest unit for marshalling + status mapping, no server needed. |
| `examples/OPC UA PLC Simulator/opcua_plc_simulator.py` | Stage 2: secure endpoints, cert generation, X.509 identity, `--security` flags. |
| `tests/integration/test_opcua_driver.py` | Extend for AC6–AC15. |
| `doc/claude/architecture/io.md` | Rewrite the OPC UA bullet: it currently states "policy None only: the shipped backend has no encryption". |
| `doc/help/…` | Secure setup, cert export, trust (AC16). |
| `REUSE.toml` / `LICENSES/` | open62541 is MPL-2.0; add the license file and declaration. |

## Architecture & data flow

`OpcUaSession` is a `QObject` living on the driver's thread (the GUI thread — drivers are never
`moveToThread`'d; only `FrameReader`, `FrameParser`/`FrameBuilder` and `StreamWorker` are). It
owns:

- a `UA_Client*` plus its `UA_ClientConfig`,
- a `QTimer` (`m_pump`) whose timeout calls `UA_Client_run_iterate(client, 0)`,
- a `QHash<quint32, Request>` mapping open62541 async request ids to the pending operation.

open62541's C callbacks are `static` trampolines that recover the session from the
`clientContext` pointer and immediately `Q_EMIT` — no work is done inside a callback beyond
translating into Qt types.

Control flow, per spec 0066's doctrine which this plan preserves:

```
open()  -> session.discoverEndpoints()      [no endpoint selected yet]
             -> endpointsReady(QList<Endpoint>, StatusCode)
             -> continuePendingDial() -> session.connect(dialEndpoint(), identity)
open()  -> session.connect(...)             [endpoint already selected]
             -> stateChanged(Connected)  -> reportOpenFinished(true) -> subscribeAll()
             -> stateChanged(Disconnected) / connectFailed(reason) -> failDial(...) -> once
```

`subscribeAll()` becomes one `UA_Client_Subscriptions_create_async()` followed by one
`UA_Client_MonitoredItems_createDataChanges_async()` carrying every tag. This is a *better* fit
than today's per-node `enableMonitoring()`: the batch reply carries a per-item status, which is
exactly the "every refused item is refused individually, only an all-refused verdict flips
`m_pollMode`" rule, and it collapses N round-trips into one. Data changes arrive through
`UA_Client_DataChangeNotificationCallback` → `valueChanged(int tag, MonitoredValue)`, which
lands in the existing `storeValue()` unchanged.

The frame path is untouched: `m_frameTimer` still encodes dirty slots into an `OpcUaWire` frame
and calls `publishReceivedData()` on the driver thread, stamped with the earliest source
timestamp through the existing steady-clock + server-offset mapping.

Browse and the batched attribute read in `OpcUaTagModel` move from `QOpcUaNode::browseChildren`
/ `readNodeAttributes` to `UA_Client_sendAsyncBrowseRequest` / `UA_Client_sendAsyncReadRequest`.
The model's laziness contract (one Browse per expansion + one batched Read for that level,
bounded EU/Range probe queue) is a property of the model and survives as-is.

**Stage 2** adds to `UA_ClientConfig`: `securityPolicyUri`, `securityMode`, the client
certificate/key `UA_ByteString`s, and `certificateVerification` pointing at our callback. That
callback is the only new place a session can block on a user decision, and it does not: an
unknown certificate fails the handshake with a distinct reason, the driver emits
`serverCertificateUntrusted(CertInfo)`, and the user's accept re-dials. This keeps the
one-verdict contract intact — the rejected dial reports failure exactly once, and the retry is a
new attempt with its own verdict.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** The change stops at `publishReceivedData()`, which is where
  the driver already hands off. `FrameReader`, `CircularBuffer`, `FrameBuilder`, the span fast
  lane and the Dashboard draw path are untouched, as is `OpcUaWire.h` and the `opcua` native
  template that decodes it. OPC UA frame rates are bounded by the publishing interval
  (`kMinIntervalMs` 10 ms), orders of magnitude below the 256 kHz gate.
- **New cross-thread signal/slot?** **No.** `OpcUaSession` is created by, parented to, and
  affine to the driver that owns it. Every signal it emits is a same-thread direct connection.
  open62541's C callbacks are invoked synchronously from inside `UA_Client_run_iterate()`, which
  we call from a QTimer on that same thread — so there is no foreign thread in the design at
  all, and `UA_MULTITHREADING` stays at its single-threaded default.
- **New input to a cached hotpath flag?** **No.** No change to `m_operationMode`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, or Dashboard `m_streamAvailable`.
  The driver is not stream-capable and does not publish `SampleBlock`s.
- **Timestamp ownership.** Unchanged and load-bearing: the source timestamp still arrives on the
  data-change notification, is mapped once through `toSteady()` using the per-connect
  steady-clock offset plus the server-to-local offset sampled at connect, and nothing downstream
  re-stamps. The marshalling layer must carry `sourceTimestamp` out of `UA_DataValue`
  explicitly — dropping it would silently fall back to `monotonicFrameNs()` and quietly break
  spec 0066 R10 / AC5, so `tst_opcua_marshal` asserts it.

## Data model & persistence

No `Frame.h` `Keys::` additions and no schema version bump for stage 1 — the driver's project
shape, its `SourceConn` block, and the `opcua` template's `schema` param are unchanged.

Stage 2 adds to the driver's `driverProperties()` (and therefore to `SourceConn` and QSettings):
`securityPolicy` (string URI), `securityMode` (int), `identityType` (int), and
`userCertificatePath` / `userKeyPath` (strings). These follow the existing pattern, so project
round-trip is inherited. **Nothing secret is added**: the client key, the user key and any
passphrase live outside the project file, and the existing `DriverProperty::Password` type is
what keeps them out. Older projects lack these keys and fall back to policy None + anonymous,
which is exactly today's behavior.

The trust store and the generated client certificate live in the app's writable config location,
not in the project — per the spec's open question, this plan assumes **per-installation**.

## API / SDK surface

Stage 1: none. `io.opcua.*` keeps its current shape; `getStatus`, `getConfig`, `listEndpoints`,
`listTags`, browse and `generateProject` are unchanged in name and payload.

Stage 2 adds, all behind `BUILD_COMMERCIAL` in `OpcUaHandler`: `io.opcua.setSecurityPolicy`,
`io.opcua.setSecurityMode`, `io.opcua.setIdentityType`, `io.opcua.setUserCertificate`,
`io.opcua.getCertificate`, `io.opcua.exportCertificate`, `io.opcua.listTrusted`,
`io.opcua.trustServer`, `io.opcua.revokeTrust`. `listEndpoints` gains `policy`, `mode`,
`deprecated` and `securityLevel` per row (it already carries `policy` and `selectable`).
`EnumLabels.cpp` gains slugs for the policy and identity-token enums.

## QML / UI

`OpcUa.qml` gains a security group: policy ComboBox, mode ComboBox, identity-token selector, and
a certificate panel (subject / fingerprint / validity, Export, Replace). Deprecated policies
render with a warning label per R8. The existing endpoint list stops greying out secure rows.

`OpcUaTrustDialog.qml` is a new modal fed by `serverCertificateUntrusted(CertInfo)`. It must be
opened **queued**, never synchronously from inside the driver's error path — `io.md` is explicit
that a synchronous modal inside an `open()` or error stack spins a nested event loop mid-emission
(the same class as the macOS file-dialog reentrancy in `common-mistakes.md`).

The plaintext-credentials warning banner becomes conditional on the negotiated mode (R15).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Abstraction shape | **Shim** (clone Qt's OPC UA API), **Session** (narrow purpose-built facade), **Direct** (call open62541 inline) | **Session**. The driver uses six operations; shaping the facade to them keeps `UA_*` out of the driver, matches open62541's request/callback model instead of emulating Qt's node-handle model, and creates the ctest seam. Shim means owning a clone of an API we don't control forever. |
| Threading | GUI-thread iterate pump vs. a worker thread owning the client | **GUI-thread pump with `*Async` calls only.** Preserves the existing driver architecture exactly, adds no cross-thread hazard, and OPC UA rates (≤100 Hz) make the cost negligible. A worker would force slot cache + frame encoding onto it too (per-value queued emissions are forbidden), that is, a second rewrite for no measured gain. Revisit only if profiling shows GUI cost. |
| Crypto backend | mbedTLS (new, static) vs. the OpenSSL in `lib/OpenSSL` vs. gRPC's BoringSSL | **mbedTLS** (amended 2026-08-24). Every shipping build sets `ENABLE_GRPC`, and the prebuilt gRPC bundles BoringSSL; `target_link_openssl()` no-ops there to avoid a duplicate-symbol link, so `lib/OpenSSL` is absent from exactly the configurations that ship. open62541 does not support BoringSSL. mbedTLS prefixes every symbol `mbedtls_`, so it collides with neither and behaves identically with `ENABLE_GRPC` on or off. |
| Acquisition | FetchContent from git vs. vendoring the release into `lib/` | **Vendored** (amended 2026-08-24). FetchContent cost three configure failures in a row: a submodule whose paths exceed Windows `MAX_PATH`, a `find_library` that runs before the archive exists, and unconditional `install(EXPORT)`/`export()` rules rejecting any dependency outside their export set. The published single-file distribution has none of that, is already generated with `UA_ENABLE_ENCRYPTION_MBEDTLS`, and matches how `lib/` already carries hidapi, luajit and mdflib. |
| mbedTLS version | 4.2.0 (latest) vs. 3.6.7 (LTS) | **3.6.7**. open62541 1.5.7's crypto plugin guards its API use with `MBEDTLS_VERSION_NUMBER` checks that stop at 3.x; 4.x is a breaking PSA-only release it does not handle. |
| Subscription creation | Per-node (today's shape) vs. one batched `createDataChanges` | **Batched**. One round-trip instead of N, and the per-item status array *is* the individual-refusal rule spec 0066 requires. |
| `ENABLE_GRPC` builds | Fail configure, share gRPC's BoringSSL, or carry an independent crypto stack | **No interaction at all** (amended 2026-08-24). The original plan failed the configure, which would have broken all nine shipping CI configurations, every one of which sets `ENABLE_GRPC=ON`. With mbedTLS the question dissolves: OPC UA no longer depends on the OpenSSL that gRPC displaces. |
| Trust store scope | Per-installation vs. per-project | **Per-installation**. This matches UaExpert and operator expectation; a per-project store would make a shared `.ssproj` carry its author's trust decisions. Flagged as a spec open question. |

## Risks & mitigations

- **The one-verdict contract is the bug class this design is most exposed to.** `io.md`: a
  driver that dials async and does not report both outcomes wedges the connect button. Stage 2
  adds several new failure points (policy rejected, cert rejected, trust declined, bad identity
  token). Mitigation: every one routes through the existing `failDial()` funnel, the 15 s
  `m_dialTimer` stays as the last-resort deadline, and AC14 asserts a distinct reason reported
  exactly once for each.
- **A stale `UA_Client` callback firing into a freed session.** Same family as the macOS
  CFSocket ABA crash `io.md` warns about. Mitigation: the session nulls its `clientContext` and
  stops the pump *before* `UA_Client_delete()`, and every trampoline null-checks; teardown never
  runs from inside a callback (deletion is queued).
- **Silently losing the source timestamp** during `UA_DataValue` marshalling, which degrades to
  the `monotonicFrameNs()` safety net and breaks spec 0066 R10 without any visible error.
  Mitigation: `tst_opcua_marshal` asserts it; `getStatus["unstamped"]` already counts it and
  `test_source_timestamps_in_csv` asserts it is zero.
- **Losing behavior that lives in the transport, not the driver**, during the rewrite —
  host substitution in `dialEndpoint()`, `endpointAcceptsToken()`, severity-bit quality, the
  bounded single-outstanding-read rule. Mitigation: stage 1 is defined by the spec 0066 suite
  passing unchanged (AC1), which covers each of these.
- **Static OpenSSL symbol collisions** with anything else in the link, and a size increase in
  the shipped binary. Mitigation: both are observable at build time; the same static OpenSSL is
  already linked today.
- **Build time and offline builds.** Another FetchContent dependency. Mitigation: the
  `SS_USE_SYSTEM_OPEN62541` / `SS_ENABLE_OPCUA` options (R4), mirroring `SS_USE_MIMALLOC=OFF`.

## Test & verification plan

- **Unit (I can run):** `tst_opcua_marshal` via `ctest` against an existing build dir —
  `UA_Variant` ⇄ `QVariant` for every wire type plus arrays, `UA_NodeId` ⇄ string round-trip,
  status-code severity classification (Good/Uncertain publish, Bad latches), and
  `sourceTimestamp` extraction. `tst_opcua_wire` must keep passing untouched, which proves the
  wire vocabulary did not move. No server required for either.
- **Integration (maintainer runs the app; I drive it):** `pytest tests/integration/test_opcua_driver.py`.
  AC1 is the whole existing suite green. New cases cover AC6 (each policy × mode), AC7
  (secure-only server), AC8 (endpoint auto-selection + deprecated never auto-picked), AC9 (cert
  generated once, reused, exportable), AC10 (trust prompt gates the session, persists), AC11
  (expired / hostname-mismatch reasons), AC12 (warning present/absent by mode), AC13 (X.509
  identity), AC14 (one distinct verdict per failure mode), AC15 (project round-trip, no secrets
  on disk). Requires the app up with the API server and the extended simulator.
- **Hotpath:** not applicable — the hotpath is untouched. `--benchmark-hotpath` is still the
  regression guard the maintainer runs before release, and AC5's throughput comparison is a
  driver-level measurement (tag count × interval vs. `framesPublished`), not a hotpath gate.
- **Packaging (AC2, AC3, AC4):** maintainer observation on a clean Windows machine with no Qt
  OPC UA module and no system OpenSSL; plus an inspection of the shipped package for absent
  plugin/crypto files; plus one configure run with OPC UA disabled asserting the source is
  hidden rather than broken.
- **Static:** `python scripts/code-verify.py --check` on every touched file; `qt-cpp-review`
  before handoff; `reuse lint` for the new MPL-2.0 component; `python scripts/sanitize-commit.py`
  before commit.
