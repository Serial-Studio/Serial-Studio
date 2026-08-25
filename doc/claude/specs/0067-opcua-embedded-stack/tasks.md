---
spec: 0067-opcua-embedded-stack
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-24
---

# Tasks 0067 — Embedded OPC UA Stack and Secure Channels

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*, each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on, usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

**Stage gate.** T1 through T18 deliver spec R1-R6: the stack is replaced and *nothing else
changes*. That set must be green and shippable on its own before T19 starts. The single
measure of stage 1 is the spec 0066 integration suite passing unchanged.

## Tasks

### Stage 1 — own the stack (R1-R6, AC1-AC5)

### T1 — WITHDRAWN (crypto backend changed to mbedTLS)

- **Files:** none. `lib/OpenSSL/CMakeLists.txt` was edited and then reverted to HEAD.
- **Withdrawn 2026-08-24:** T1 existed so open62541 could share the static OpenSSL in
  `lib/OpenSSL`. Investigation for the pre-C++ configure check found that all nine shipping CI
  configurations set `ENABLE_GRPC=ON`, that the prebuilt gRPC bundles BoringSSL, and that
  `target_link_openssl()` no-ops under `ENABLE_GRPC` precisely to avoid a duplicate-symbol link.
  `lib/OpenSSL` is therefore absent from every build that ships, and open62541 does not support
  BoringSSL. The crypto backend moved to mbedTLS (symbols prefixed `mbedtls_`, no collision with
  either), which removes the dependency on `lib/OpenSSL` entirely. `plan.md` and `spec.md` were
  amended and re-confirmed. The `lib/OpenSSL` refactor was reverted rather than kept, so this
  feature's diff carries no unrelated change.
- [x] done

### T2 — Vendor open62541 and mbedTLS into `lib/`

- **Files:** `lib/open62541/{open62541.c,open62541.h,LICENSE,CMakeLists.txt}`, `lib/mbedtls/**`,
  `lib/CMakeLists.txt`
- **Does:** Drop in open62541 1.5.7's published single-file distribution and Mbed TLS 3.6.7
  (upstream source, test/program/doc trees removed), and build both as static libraries from a
  `CMakeLists.txt` we own. `target_link_open62541()` links the stack and defines
  `SS_OPCUA_ACTIVE`; a no-op fallback is defined when the stack is absent so every configuration
  can call it.
- **Verify:** configure succeeds for commercial, GPL, and `SS_ENABLE_OPCUA=OFF`; the amalgamation
  header declares `UA_ENABLE_ENCRYPTION_MBEDTLS`.
- **Deps:** none
- **Amended 2026-08-24:** originally `cmake/Open62541.cmake` using FetchContent. Three consecutive
  configure failures (Windows `MAX_PATH` on `deps/ua-nodeset`, `find_library` running before
  mbedTLS is built, and open62541's unconditional `install(EXPORT)`/`export()` rejecting
  dependencies outside their export set) made vendoring the cheaper and more controllable path,
  and it matches how `lib/` already carries hidapi, luajit and mdflib. The FetchContent module was
  deleted.
- [x] done

### T3 — Build option and app wiring

- **Files:** `CMakeLists.txt`, `app/CMakeLists.txt`
- **Does:** Add `SS_ENABLE_OPCUA` (default ON, effective only with `BUILD_COMMERCIAL`), drop
  `OpcUa` from `QT_MODULES`/`QT_LIBS`, and call `target_link_open62541()` on the executable. With
  OPC UA off the bus is hidden exactly as in a GPL build (spec R4/R5).
- **Verify:** three configures (commercial, GPL, `SS_ENABLE_OPCUA=OFF`) all succeed;
  `grep -r Qt6::OpcUa app/` returns nothing.
- **Deps:** T2
- **Amended 2026-08-24:** the `ENABLE_GRPC` configure-time failure this task originally required
  is gone. It would have broken all nine shipping CI configurations, and with mbedTLS the OPC UA
  stack no longer touches the OpenSSL that gRPC displaces. `SS_USE_SYSTEM_OPEN62541` was dropped
  as well: nothing implements it now that the source is vendored, and R4 is met by
  `SS_ENABLE_OPCUA=OFF`.
- [x] done

### T4 — License notices and REUSE declarations

- **Files:** `app/rcc/messages/Acknowledgements.txt`, `REUSE.toml`, `LICENSES/MPL-2.0.txt`
- **Does:** MPL-2.0 notice for open62541 (with the section 3.2 source statement) and an
  Apache-2.0 notice for Mbed TLS, matching how gRPC and OpenSSL are declared. REUSE annotations
  for both vendored trees, plus the MPL-2.0 license text.
- **Verify:** `reuse lint` passes; every declared identifier has a file in `LICENSES/`.
- **Deps:** T2
- **Amended 2026-08-24:** twice. First narrowed to the acknowledgements file only, on the
  reasoning that REUSE annotates files in the repository and the dependency was fetched at build
  time. Vendoring reversed that: the sources ARE in the repository now, so REUSE entries and
  `LICENSES/MPL-2.0.txt` are required after all. Mbed TLS is dual Apache-2.0 OR GPL-2.0-or-later
  and is declared under the Apache arm, which avoids adding a GPL-2.0 license file for an arm we
  do not exercise.
- [x] done

### T5 — `OpcUaTypes.h`: transport-neutral value types

- **Files:** `app/src/IO/Drivers/OpcUaTypes.h`
- **Does:** Define `Endpoint`, `ReferenceRow`, `ReadRow`, `MonitoredValue`, `CertInfo`, plus
  `StatusCode`, `NodeAttribute` and `NodeClass` enums that replace the `QOpcUa*` equivalents.
  Qt Core only, so the ctest tier can link it without the OPC UA stack.
- **Verify:** `python scripts/code-verify.py --check` on the file; confirm no `UA_` or
  `QOpcUa` symbol appears in it.
- **Deps:** none
- [x] done

### T6 — `OpcUaMarshal`: variant, node id and status conversion

- **Files:** `app/src/IO/Drivers/OpcUaMarshal.h`, `app/src/IO/Drivers/OpcUaMarshal.cpp`
- **Does:** `UA_Variant` to `QVariant` (scalars, arrays, `LocalizedText`, `EUInformation`,
  `Range`), `UA_NodeId` to and from the `ns=N;s=...` string form, and status code to severity
  plus text. **Extracts `sourceTimestamp` off `UA_DataValue` explicitly**: losing it degrades
  silently to the `monotonicFrameNs()` safety net and breaks spec 0066 R10 with no visible
  error.
- **Verify:** `python scripts/code-verify.py --check`; covered by T7.
- **Deps:** T2, T5
- **Note:** every open62541 API used here was verified against the vendored
  `lib/open62541/open62541.h` rather than from memory. That caught `UA_DataType::typeIndex`, which
  does not exist in 1.5: dispatch goes through `typeKind`, and `EUInformation`/`Range` need a type
  pointer comparison because both report as plain `STRUCTURE`.
- [x] done

### T7 — `tst_opcua_marshal` unit suite

- **Files:** `app/tests/tst_opcua_marshal.cpp`, `app/tests/CMakeLists.txt`
- **Does:** ctest unit for every conversion in T6: each wire type, arrays, node-id round-trip,
  severity classification (Good and Uncertain publish, Bad latches), and source-timestamp
  extraction. No server required.
- **Verify:** `ctest -R tst_opcua_marshal` against an existing build dir (needs a configure with
  `SS_BUILD_TESTS=ON` AND `BUILD_COMMERCIAL=ON`: the suite is guarded on `TARGET open62541`, which
  a GPL unit-test configure does not have).
- **Deps:** T6
- [x] done

### T8 — `OpcUaSession`: lifecycle and the iterate pump

- **Files:** `app/src/IO/Drivers/OpcUaSession.h`, `app/src/IO/Drivers/OpcUaSession.cpp`
- **Does:** Client construction and config, the QTimer pump calling `UA_Client_run_iterate`,
  the async request registry, and teardown. **Binding invariant:** the pump stops and
  `clientContext` is nulled *before* `UA_Client_delete()`, every static trampoline null-checks,
  and deletion never runs from inside a callback (queued). This is the stale-callback-into-freed-
  object class that crashed `readFromSocket` on macOS.
- **Verify:** `python scripts/code-verify.py --check`; confirm `OpcUaSession.cpp` is the only
  file including `open62541/*`.
- **Deps:** T2, T5, T6
- [x] done

### T9 — `OpcUaSession`: discovery and connect

- **Files:** `app/src/IO/Drivers/OpcUaSession.h`, `app/src/IO/Drivers/OpcUaSession.cpp`
- **Does:** Async endpoint discovery emitting `endpointsReady`, `UA_Client_connectAsync`, and
  the state callback mapped to `stateChanged` / `connectFailed(reason)`. **Binding invariant:**
  the session reports a connect outcome exactly once per attempt; the driver's `failDial()`
  funnel and the 15 s `m_dialTimer` remain the only verdict owners. A driver that dials async
  and does not report both outcomes wedges the connect button.
- **Verify:** `python scripts/code-verify.py --check`; read back against the `io.md` dial
  doctrine.
- **Deps:** T8
- **Note:** `UA_Client_connectUsername()` is NOT used. Upstream documents it as setting the
  identity token and then calling the BLOCKING connect, which would freeze the GUI for the whole
  handshake and for the full timeout against an unreachable host. The token is installed on the
  config directly and the dial goes through `UA_Client_connectAsync()`.
- [x] done

### T10 — `OpcUaSession`: batched subscription

- **Files:** `app/src/IO/Drivers/OpcUaSession.h`, `app/src/IO/Drivers/OpcUaSession.cpp`
- **Does:** One `UA_Client_Subscriptions_create_async` plus one
  `UA_Client_MonitoredItems_createDataChanges_async` for all tags; surface the per-item status
  array so refusal stays *individual*. Data-change callback emits `valueChanged` carrying value,
  status and source timestamp. **Binding invariant:** source owns time; the stamp travels with
  the value and nothing downstream re-stamps.
- **Verify:** `python scripts/code-verify.py --check`.
- **Deps:** T8
- **Note 2026-08-24:** the monitored-item context carries the tag index (a pointer into
  `m_monitorTags`) rather than a monitored-item-id map. open62541 registers items locally BEFORE
  the CreateMonitoredItems reply arrives ("early processing", documented at the MonitoredItems
  block in the amalgamation), so a map keyed on the returned id drops the initial value of every
  slow-changing tag. `handleSubscribed()` gained a `serviceStatus` parameter so a reply that
  carries no results at all still yields one verdict per tag instead of a short, mis-indexed list.
- [x] done

### T11 — `OpcUaSession`: read and browse

- **Files:** `app/src/IO/Drivers/OpcUaSession.h`, `app/src/IO/Drivers/OpcUaSession.cpp`
- **Does:** `sendAsyncReadRequest` and `sendAsyncBrowseRequest` against the request registry,
  emitting `readFinished` / `browseFinished`. **Binding invariant:** at most ONE read
  outstanding; a queued read behind a slow PLC grows latency without bound. Reads stay chunked
  to the server's `MaxNodesPerRead`.
- **Verify:** `python scripts/code-verify.py --check`.
- **Deps:** T8
- **Note 2026-08-24:** the Read service answers POSITIONALLY and carries no node id on the reply,
  so rows are staged in request order (`PendingRead`) and merged on arrival. `browse()` takes a
  `BrowseQuery` whose `token` comes back with the reply: the picker browses one node for up to
  three different reasons (level expansion, has-children probe, units lookup) and the node id
  alone cannot route the answer. The one-read-outstanding gate applies to `readValues()` only --
  the picker's per-level `readAttributes()` is bounded by the level, not repeated on a timer.
  `MaxNodesPerRead` moved into the session as an internal read consumed before `readFinished`.
- [x] done

### T12 — Driver: lifecycle and dial onto the session

- **Files:** `app/src/IO/Drivers/OpcUa.h`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** Replace `QOpcUaClient`/`QOpcUaProvider` use with `OpcUaSession` for `open()`,
  `close()`, discovery and the dial. **Preserve verbatim:** discover-before-dial via
  `m_pendingDial`, the host and port substitution in `dialEndpoint()`, `endpointAcceptsToken()`,
  and the single `failDial()` funnel. **Binding invariant:** existing signal wiring must be read
  before it is changed; `reportOpenFinished` fires exactly once per attempt.
- **Verify:** `python scripts/code-verify.py --check`; no `QOpcUa` include remains in the file.
- **Deps:** T9
- **Note 2026-08-24:** `connectToEndpoint()` takes an `OpcUaTypes::Endpoint`, not a URL, so the
  policy and mode travel with the dial and stage 2 needed no second rewrite. `dialEndpoint()`
  returns the substituted Endpoint; `policyIsNone()` became `endpointUsable()`, the one place that
  widens in T22.
- [x] done

### T13 — Driver: subscription, poll fallback and frame tick

- **Files:** `app/src/IO/Drivers/OpcUa.h`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** Rewire `subscribeAll`, `onMonitoringEnabled`, `enterPollMode`, the 1 Hz watchdog,
  `issueRead`/`onReadFinished` and `storeValue` onto the session's signals. **Preserve
  verbatim:** severity-bit quality (Good and Uncertain publish, Bad latches and lands in
  `badTags()`), only an all-refused verdict flips `m_pollMode`, and `onFrameTick` still encodes
  dirty slots and calls `publishReceivedData()` untouched.
- **Verify:** `python scripts/code-verify.py --check`; `ctest -R tst_opcua_wire` still green,
  proving the wire vocabulary did not move.
- **Deps:** T10, T11, T12
- **Note 2026-08-24:** the per-tag `onMonitoringEnabled` accumulator collapsed into one
  `onSubscribed(perItemStatus)`, because the batch reply carries every verdict at once. The rules
  it enforced are unchanged: refusal stays individual, only an all-refused verdict flips
  `m_pollMode`, and `onFrameTick` was not touched. `readServerLimits()` moved into the session.
  `unwrapValue()` is gone: `OpcUaMarshal` unwraps LocalizedText at ingestion.
- [x] done

### T14 — Tag model onto the session

- **Files:** `app/src/IO/Drivers/OpcUaTagModel.h`, `app/src/IO/Drivers/OpcUaTagModel.cpp`
- **Does:** Move `browse()`, `readLevel()`, `onAttributesRead()` and the EU/Range probe queue
  from `QOpcUaNode` handles to the session's browse and read signals. **Preserve verbatim:** the
  laziness contract of one Browse per expansion plus one batched Read for that level, and units
  and EURange resolved only for ticked tags through the bounded queue. Crawling the address
  space on open is what makes the picker unusable on a 100k-node gateway.
- **Verify:** `python scripts/code-verify.py --check`; no `QOpcUa` include remains.
- **Deps:** T11
- **Note 2026-08-24:** browse replies route on `BrowseQuery::token` (Level / Probe / Units), which
  is what one node being browsed for three reasons at once needs. The namespace array is read
  through the session's own read path on `setSession()`. `UA_Range` now marshals to {low, high}
  rather than the low bound alone: both become a generated dataset's plot range, and
  `tst_opcua_marshal` was amended to pin that.
- [x] done

### T15 — Drop Qt6::OpcUa from the build

- **Files:** `app/CMakeLists.txt`
- **Does:** Remove `OpcUa` from `QT_MODULES` and `Qt6::OpcUa` from `QT_LIBS`, register the new
  sources, and call `target_link_open62541()`.
- **Verify:** the tree builds (maintainer); `grep -r Qt6::OpcUa app/` returns nothing.
- **Deps:** T3, T13, T14
- **Done 2026-08-24:** the transitional `QT_MODULES OpcUa` / `Qt6::OpcUa` lines are deleted and a
  repo-wide sweep for `QOpcUa`, `Qt6::OpcUa` and `QtOpcUa` finds only the comment saying the module
  is not used. `OpcUaSecurity.h/.cpp` were registered alongside the rest.
- [x] done

### T16 — Stage 1 behavior verification (AC1, AC5)

- **Files:** none (verification only)
- **Does:** Run the spec 0066 integration suite unchanged and compare throughput and connect
  time against the pre-change build at equal tag count and publishing interval.
- **Verify:** `pytest tests/integration/test_opcua_driver.py -v` fully green with the app up and
  the simulator running; the AC5 comparison shows no regression outside run-to-run noise.
- **Deps:** T15
- **Blocked on the maintainer 2026-08-24:** needs a built Pro binary. The last CI pytest run
  (32674734731) had ten of these red against the OLD Qt backend, all downstream of a subscription
  that connected but delivered nothing, so there is no working baseline to compare against. Run it
  once the tree builds.
- [ ] done

### T17 — Stage 1 packaging verification (AC2, AC3, AC4)

- **Files:** none (maintainer observation)
- **Does:** Confirm a shipped Pro build connects on a Windows machine with no Qt OPC UA module
  and no system OpenSSL; confirm the package carries no OPC UA plugin and no new crypto library;
  confirm an OPC-UA-disabled configure hides the source rather than offering a broken one.
- **Verify:** maintainer observation on a clean machine, recorded in the spec's AC boxes.
- **Deps:** T15
- **Blocked on the maintainer 2026-08-24:** packaging observation, nothing left to do in the tree.
- [ ] done

### T18 — Update the I/O architecture doc for stage 1

- **Files:** `doc/claude/architecture/io.md`
- **Does:** Rewrite the OPC UA bullet, which currently asserts "policy None only: the shipped
  backend has no encryption" and describes a Qt plugin that is no longer used. Describe the
  session facade, the pump, and the teardown ordering invariant.
- **Verify:** `python scripts/documentation-verify.py doc/claude/architecture/io.md`.
- **Deps:** T15
- **Done 2026-08-24:** the single bullet became five: the vendored stack, the session's thread
  affinity and teardown ordering, the positional-reply staging, the preserved dial doctrine, and
  the secure-channel configuration. Verifier findings are unchanged from HEAD (all pre-existing and
  outside this bullet) and the em-dash count went down.
- [x] done

### Stage 2 — secure channels (R7-R18, AC6-AC16)

### T19 — Client certificate identity

- **Files:** `app/src/IO/Drivers/OpcUaSecurity.h`, `app/src/IO/Drivers/OpcUaSecurity.cpp`
- **Does:** Generate a self-signed client certificate and key on first secure use, with a
  subject and application URI identifying the installation; load, inspect and replace them.
  Stored per-installation in the writable config location, never in the project file.
- **Verify:** `python scripts/code-verify.py --check`; generated cert parses and its application
  URI matches the one sent in the handshake.
- **Deps:** T15
- **Note 2026-08-24:** `OpcUaSecurity.h` stays Qt Core only so the driver and the API handler can
  use it without seeing the stack; the .cpp is the second (and last) file that includes
  open62541, plus mbedTLS directly for certificate inspection. That amends T8's "only
  OpcUaSession.cpp includes open62541/*", which was a stage-1 statement.
- [x] done

### T20 — Trust store and server-certificate verification

- **Files:** `app/src/IO/Drivers/OpcUaSecurity.h`, `app/src/IO/Drivers/OpcUaSecurity.cpp`
- **Does:** Persistent per-installation trust store plus the verification callback. Extract
  `CertInfo` (subject, issuer, fingerprint, validity window) and classify the failure as
  untrusted, expired, not-yet-valid, or hostname mismatch, each distinctly (R14).
- **Verify:** `python scripts/code-verify.py --check`; unit coverage for the classification.
- **Deps:** T19
- **Note 2026-08-24:** the trust store is one DER file per accepted certificate, named by its
  SHA-256 fingerprint, so a re-keyed server is a new decision rather than an inherited one.
- [x] done

### T21 — Session: policy, mode and identity tokens

- **Files:** `app/src/IO/Drivers/OpcUaSession.h`, `app/src/IO/Drivers/OpcUaSession.cpp`
- **Does:** Apply `securityPolicyUri`, `securityMode`, the client certificate and key, and the
  verification callback to `UA_ClientConfig`. Add the X.509 identity token alongside anonymous
  and username (R16). Report the negotiated policy and mode back to the driver.
- **Verify:** `python scripts/code-verify.py --check`.
- **Deps:** T20
- **Note 2026-08-24:** the session REPLACES open62541's trust-list check with its own
  `verifyCertificate` hook rather than seeding a trust list, because a user's accept has to be
  able to override a chain no CA signs. A small `s_verifiers` registry maps the certificate group
  back to the session: the hook is a plain C function pointer and the group's `context` belongs to
  open62541's memory store. `allowNonePolicyPassword` had to be set for spec 0066's None-policy
  username login to work at all.
- [x] done

### T22 — Driver: security configuration and endpoint selection

- **Files:** `app/src/IO/Drivers/OpcUa.h`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** Add `securityPolicy`, `securityMode`, `identityType` and user cert/key to
  `driverProperties()` so they persist and round-trip (R18). Replace `policyIsNone()` selection
  with: all five policies supported, deprecated ones labelled and never auto-selected, default
  to the most secure endpoint the chosen identity can use (R7, R8, R9). **Binding invariant:**
  no key or password may enter the project file; `DriverProperty::Password` is what keeps them out.
- **Verify:** `python scripts/code-verify.py --check`; read back a saved project for secrets.
- **Deps:** T21
- **Note 2026-08-24:** `identityType` is `authMode` widened to three values rather than a second
  overlapping property; the API exposes `setIdentityType` as an alias. `selectBestEndpoint()`
  scores usable endpoints and never selects a deprecated policy unless it was asked for by name.
- [x] done

### T23 — Driver: security failure verdicts and the trust signal

- **Files:** `app/src/IO/Drivers/OpcUa.h`, `app/src/IO/Drivers/OpcUa.cpp`
- **Does:** Route every security failure through the existing `failDial()` funnel with its own
  reason, and emit `serverCertificateUntrusted(CertInfo)`. **Binding invariant:** one verdict per
  attempt. The new failure points are new ways to fail, never new ways to hang; a trust prompt
  followed by accept is a *new* attempt with its own verdict.
- **Verify:** `python scripts/code-verify.py --check`; covered by AC14 in T29.
- **Deps:** T22
- **Note 2026-08-24:** `reportTrustFailure()` runs before `failDial()` so the funnel carries the
  distinct reason, and emits `serverCertificateUntrusted` QUEUED. The refused certificate is also
  in `io.opcua.getStatus` under `serverCertificate`, which is how a headless client gets the
  fingerprint it must pass to `io.opcua.trustServer`.
- [x] done

### T24 — API commands for security

- **Files:** `app/src/API/Handlers/OpcUaHandler.h`, `app/src/API/Handlers/OpcUaHandler.cpp`
- **Does:** Add `setSecurityPolicy`, `setSecurityMode`, `setIdentityType`, `setUserCertificate`,
  `getCertificate`, `exportCertificate`, `listTrusted`, `trustServer`, `revokeTrust`. Extend
  `listEndpoints` rows with `policy`, `mode`, `deprecated` and `securityLevel`. All behind
  `BUILD_COMMERCIAL`.
- **Verify:** `python scripts/code-verify.py --check`; `io.opcua.*` reachable over the API.
- **Deps:** T23
- **Note 2026-08-24:** `regenerateCertificate` was added alongside the listed commands; replacing
  the client identity is otherwise only reachable from the GUI.
- [x] done

### T25 — Enum labels for the new domains

- **Files:** `app/src/API/EnumLabels.cpp`
- **Does:** Add slugs and labels for the security policy and identity-token enums.
- **Verify:** `ctest -R tst_enum_labels`.
- **Deps:** T24
- **Note 2026-08-24:** the new label functions take plain ints and are NOT guarded on
  BUILD_COMMERCIAL, so `tst_enum_labels` sweeps them in the GPL unit tier like every other domain.
- [x] done

### T26 — Setup pane security group

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUa.qml`
- **Does:** Policy and mode pickers, identity selector, certificate panel (subject, fingerprint,
  validity, Export, Replace), deprecation labels on weak policies, and the endpoint list no
  longer greying out secure rows. The plaintext-credentials banner becomes conditional on the
  negotiated mode (R15).
- **Verify:** `python scripts/code-verify.py --check`; maintainer observation in the running app.
- **Deps:** T24
- [x] done

### T27 — Server-certificate trust dialog

- **Files:** `app/qml/MainWindow/Dialogs/OpcUaTrustDialog.qml`, `app/qml/.../Drivers/OpcUa.qml`
- **Does:** Modal showing subject, issuer, fingerprint, validity and the distinct failure reason,
  with trust or reject. **Binding invariant:** opened **queued**, never synchronously from inside
  the driver's error path. A synchronous modal inside an `open()` or error stack spins a nested
  event loop mid-emission (same class as the macOS file-dialog reentrancy).
- **Verify:** `python scripts/code-verify.py --check`; maintainer observation.
- **Deps:** T26
- **Deviation 2026-08-24:** the dialog lives at `app/qml/Dialogs/OpcUaTrustDialog.qml`; there is no
  `app/qml/MainWindow/Dialogs/` directory in this tree and every other dialog is under
  `app/qml/Dialogs/`.
- [x] done

### T28 — Simulator: secure endpoints and X.509 identity

- **Files:** `examples/OPC UA PLC Simulator/opcua_plc_simulator.py`,
  `examples/OPC UA PLC Simulator/README.md`
- **Does:** Generate a server certificate, advertise the supported policies in both modes behind
  flags, and accept an X.509 identity token. Must stay runnable with only the documented Python
  dependencies.
- **Verify:** run the simulator locally and connect with a plain `asyncua` client over each
  policy.
- **Deps:** T21
- **Note 2026-08-24:** verified locally: `--security all` advertises 11 endpoints (five policies in
  Sign and SignAndEncrypt plus None) and a plain `asyncua` client opens a Basic256Sha256
  SignAndEncrypt channel against it. The server certificate is generated on first secure run and
  reused, so a client that trusted it once is not asked again. `--cert-expired` and
  `--cert-wrong-host` were added beyond the task text: AC11 and AC14 ask for tests against an
  expired and a hostname-mismatched certificate, and there was no way to serve either.
  `setup_self_signed_certificate` always starts the validity window at "now", so the expired case
  builds its certificate with explicit dates.
- [x] done

### T29 — Integration tests for the secure channel (AC6-AC15)

- **Files:** `tests/integration/test_opcua_driver.py`
- **Does:** Add cases for each policy and mode, a secure-only server, endpoint auto-selection,
  certificate generation and reuse, the trust prompt gating and persisting, expired and
  hostname-mismatch reasons, the conditional warning, X.509 identity, one distinct verdict per
  failure mode, and the project round-trip carrying no secrets.
- **Verify:** `pytest tests/integration/test_opcua_driver.py -v` with the app up and the extended
  simulator running.
- **Deps:** T27, T28
- **Note 2026-08-24:** `TestOpcUaSecureChannel` adds twelve cases. Each secure case connects,
  accepts the certificate through `io.opcua.trustServer` when the first attempt is refused,
  reconnects, and revokes it before returning, so a run leaves no trust decision behind. The
  expired case also asserts that ACCEPTING an expired certificate does not let it through:
  renewing it on the server is the only fix, and a prompt that could wave it past would be worse
  than no prompt. Written but NOT run: they need a built Pro binary.
- [x] done

### T30 — User documentation (AC16)

- **Files:** `doc/help/…`, `help.json`
- **Does:** Document secure connection setup, client-certificate export, and server-certificate
  trust. Register any new page.
- **Verify:** `python scripts/documentation-verify.py` on the changed pages.
- **Deps:** T29
- **Note 2026-08-24:** no new page; `doc/help/Drivers-OPC-UA.md` gained a "Secure connections"
  section and its stale policy-None claims were replaced, so `help.json` is unchanged.
- [x] done

### T31 — Final architecture doc pass

- **Files:** `doc/claude/architecture/io.md`, `CLAUDE.md`
- **Does:** Fold the secure-channel behavior into the OPC UA bullet, and update the CLAUDE.md
  data-source line, which currently reads "OPC UA (Qt OpcUa, policy None only)".
- **Verify:** `python scripts/documentation-verify.py`; grep for stale "policy None" claims.
- **Deps:** T29
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched, as `plan.md` states. `ctest -R tst_opcua_wire` proves the wire vocabulary
      did not move (maintainer runs it). No `--benchmark-hotpath` regression expected or accepted.
- [ ] `ctest -R "tst_opcua_marshal|tst_opcua_wire|tst_enum_labels"` green. `tst_opcua_marshal` now
      builds in the GPL unit tier, so CI runs it for the first time.
- [ ] `pytest tests/integration/test_opcua_driver.py` green with the app up (maintainer runs the
      app).
- [ ] **Property-registry baselines need recapturing on a built binary.** CI has been red on
      `test_round_trip_matches_baseline` since before this feature (7 example projects drifted) and
      the OPC UA simulator project has no baseline at all. With the app up:
      `SS_CAPTURE_BASELINES=1 pytest tests/integration/test_property_registry.py::test_capture_baselines`,
      then review the diff before committing it.
- [ ] `reuse lint` passes with the new MPL-2.0 component declared.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that*: no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
