---
spec: 0041-mqtt-mutual-tls
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-27
---

# Plan 0041 — MQTT mutual TLS (client certificates) for AWS IoT Core

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Add a shared `MQTT::TlsIdentity` helper (new TU, `BUILD_COMMERCIAL`) that loads and
validates a PEM client certificate + private key (+ optional passphrase) into
`QSslCertificate`/`QSslKey` and applies them — plus an optional ALPN protocol — to a
`QSslConfiguration`. Both MQTT surfaces consume it with the **parse-at-pick** model: files
are read and validated on the main thread at selection/restore time, errors surface
immediately, and only already-parsed (implicitly shared, cheap-to-copy) objects travel to
the connection code. The publisher carries them in `BrokerConfig` to the worker exactly like
`caCertificates` today; the subscriber driver stores them straight into its main-thread
`m_sslConfiguration`. Paths + ALPN persist next to the existing TLS settings (project JSON
for the publisher, `MqttInputDriver/` QSettings for the driver); the passphrase persists
only in `CredentialVault`, which grows a third encrypted slot. Nothing on any per-frame
path changes.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/MQTT/TlsIdentity.h` | **New.** `struct TlsIdentity { QSslCertificate certificate; QSslKey privateKey; }` + load/validate free functions returning a typed error (missing file / unreadable / not PEM / passphrase required / passphrase wrong) and `applyToSslConfiguration(QSslConfiguration&, const TlsIdentity&, const QByteArray& alpn)`. |
| `app/src/MQTT/TlsIdentity.cpp` | **New.** Implementation; combined-PEM support (key found in cert file); no Qt GUI deps so the ctest tier can link it. |
| `app/src/MQTT/CredentialVault.h` | Add `keyPassphrase(host, port)` / `setKeyPassphrase(host, port, passphrase)` (third encrypted `/keyPass` slot; `Credentials` struct untouched). |
| `app/src/MQTT/CredentialVault.cpp` | Implement the new slot; empty passphrase removes it (mirrors user/pass semantics). |
| `app/src/MQTT/Publisher.h` | `BrokerConfig`: add `QSslCertificate clientCertificate`, `QSslKey clientPrivateKey`, `QByteArray alpnProtocol`. `Publisher`: Q_PROPERTYs + members + setters for `clientCertificatePath`, `privateKeyPath`, `keyPassphrase`, `alpnEnabled`, `alpnProtocol`; picker slots `selectClientCertificate()` / `selectPrivateKey()`; new `kKey*` persistence constants. |
| `app/src/MQTT/Publisher.cpp` | Setters (guard-return + `markConfigChanged()`), pickers (QFileDialog with the queued-invoke pattern), reload/validate via `TlsIdentity` on set/restore, vault round-trip for the passphrase, `snapshotConfig()` / `toJson()` / `applyProjectConfig()` extensions, worker `applyBrokerConfig()` applies identity + ALPN to `m_sslConfiguration` and adds the new fields to the `brokerChanged` comparison, handshake-failure hint when a client identity is configured. |
| `app/src/IO/Drivers/MQTT.h` | Q_PROPERTYs + members + setters for the same five settings; picker slots; identity applied to the existing `m_sslConfiguration`. |
| `app/src/IO/Drivers/MQTT.cpp` | Setters + QSettings persistence (`clientCertPath`, `privateKeyPath`, `alpnEnabled`, `alpnProtocol` under `MqttInputDriver/`), vault passphrase, restore in `loadPersistedSettings()`, `appendMqttSslProperties()` + `setDriverProperty()` rows, TLS-failure hint in `onErrorChanged()`. |
| `app/src/DataModel/Project/ProjectEditorItemIds.h` | Append `kMqttPublisher_ClientCertPath`, `_PrivateKeyPath`, `_KeyPassphrase`, `_AlpnEnabled`, `_AlpnProtocol`. |
| `app/src/DataModel/Project/ProjectEditorMqtt.cpp` | `buildMqttSslSection()`: five new rows (TextField x2, PasswordField, CheckBox, TextField shown only when ALPN on); `onMqttPublisherItemChanged()` cases (ALPN toggle rebuilds the model like `SslEnabled` does). |
| `app/qml/ProjectEditor/Views/MqttPublisherView.qml` | Two toolbar buttons next to "Load CA Certs": "Client Certificate…" / "Private Key…" → `Cpp_MQTT_Publisher.selectClientCertificate()` / `selectPrivateKey()` (path rows in the form stay hand-editable). |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml` | Under the TLS block: cert path + key path fields each with a Browse button, passphrase field (`echoMode: Password`), ALPN checkbox + protocol field (visible when checked, default `x-amzn-mqtt-ca`), all gated on `sslEnabled`. |
| `app/CMakeLists.txt` | Register `TlsIdentity.{h,cpp}` in the commercial sources block. |
| `app/tests/` (one new ctest TU) | Unit tests for `TlsIdentity` load/validate paths (AC5's local half). |

## Architecture & data flow

**Publisher (worker-threaded, export side — `architecture/export.md`):** UI/API set path
or passphrase → `Publisher` setter loads + validates via `TlsIdentity` (main thread),
stores parsed members, `markConfigChanged()` → debounced `syncToWorker()` →
`snapshotConfig()` copies certificate/key/ALPN into `BrokerConfig` → queued
`applyBrokerConfig()` on the worker sets `QSslConfiguration::setLocalCertificate` /
`setPrivateKey` / `setAllowedNextProtocols`, and the enlarged `brokerChanged` comparison
triggers the existing reconnect task tree. `runTestConnection()` inherits the identity for
free — the throwaway tester already dials with `m_sslConfiguration`.

**Subscriber driver (main-thread, spec-0034 flows — `architecture/io.md`):** setters
load/validate and write straight into the member `m_sslConfiguration`, persist, then
`scheduleReconnectIfActive()`; `dialBroker()` already passes `m_sslConfiguration` to
`connectToHostEncrypted()`. No new flow steps, no retry changes — `RetryPolicy` untouched.

**Validation errors** surface at set time via the existing per-surface reporting
(`Misc::Utilities::showMessageBox` from the pickers; the setter rejects and keeps the prior
identity). A handshake failure with an identity configured appends a one-line hint
("verify the client certificate matches the private key and is activated on the broker")
to the existing `TransportInvalid` description on both surfaces.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** `hotpathTxFrame` / `hotpathTxRawBytes` /
  `processItems` and the driver's `onMessageReceived` → `publishReceivedData` path are
  untouched. All new work runs at configuration/connection boundaries.
- **New cross-thread signal/slot?** No new signals. The existing queued
  `applyBrokerConfig(BrokerConfig)` carries three more implicitly-shared members
  (`QSslCertificate`/`QSslKey`/`QByteArray` — shallow copies, connection-time only).
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; no stamping code touched.

## Data model & persistence

- **Publisher (project JSON via `toJson()`/`applyProjectConfig()`):** new keys
  `clientCertPath`, `privateKeyPath`, `alpnEnabled`, `alpnProtocol`. Absent keys default to
  off/empty — old projects load unchanged (AC6). No `Frame.h` keys, no schema version bump
  (the MQTT block is free-form config, not frame schema). Passphrase **never** enters the
  project JSON: on load, `applyProjectConfig` pulls it from the vault (keyed host:port,
  same as username/password today).
- **Driver (QSettings `MqttInputDriver/`):** `clientCertPath`, `privateKeyPath`,
  `alpnEnabled`, `alpnProtocol`; passphrase in the vault.
- **Vault:** third slot `<sha1(host:port)>/keyPass`, SimpleCrypt-encrypted like `/user` and
  `/pass`. Missing slot decrypts to empty = no passphrase.
- Restore order in both surfaces: passphrase loaded from the vault before the key file is
  parsed (the key parse needs it).

## API / SDK surface

None in this spec. The publisher's new properties ride the existing
`ProjectModel::setMqttPublisher` JSON blob (already reachable over the API as an opaque
config object); the driver's new rows appear automatically in `driverProperties()` /
`setDriverProperty()`, which is the existing CLI/API surface for driver config. No new
handlers, no generated-registry or proto changes (dataset property registry is not
involved). All new C++ stays behind `BUILD_COMMERCIAL` (both surfaces already are;
`TlsIdentity` gets the same guard).

## QML / UI

- **Driver pane** (`SetupPanes/Drivers/MQTT.qml`, hand-written): follows the existing
  label + `Widgets.BoundField` grid; Browse buttons call the new picker slots; everything
  new is `visible:`/`enabled:`-gated on `Cpp_IO_Mqtt.sslEnabled` (R7). No ComboBox → no
  restore-race guard needed.
- **Publisher** (`ProjectEditorMqtt.cpp` form model): new rows in the SSL section, `Active`
  role follows the section's convention; ALPN checkbox toggles a model rebuild (same
  pattern as `kMqttPublisher_SslEnabled`). Path fields are editable text (typing a path
  works headless); the toolbar pickers are convenience.
- Both pickers use file-mode `QFileDialog` (`*.pem *.crt *.cer *.key`) with the
  **queued-invoke body** (macOS `fileSelected` reentrancy — `common-mistakes.md`).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| When to parse PEM | parse-at-pick / parse-at-dial / blob-in-project | **Parse-at-pick** — errors at selection time (R6), zero file I/O in the worker, secrets never serialized; snapshot copies are implicit-share cheap. |
| Code sharing | shared `TlsIdentity` TU / duplicate logic per surface | **Shared TU** — one validation path, one error taxonomy, unit-testable in the ctest tier without the app. |
| ALPN UI | preset ComboBox / checkbox + editable text | **Checkbox + text defaulting to `x-amzn-mqtt-ca`** — satisfies "AWS preset, editable" (R4) with one setting instead of a preset registry. |
| Cert/key mismatch detection | local public-key comparison / handshake + hint | **Handshake + hint** — Qt has no public-key-derivation API for `QSslKey`; a local check would mean direct OpenSSL calls (new dependency surface, against constraints). Spec R6/AC5 amended accordingly (2026-07-27). |
| Key file reuse | separate key file mandatory / combined-PEM fallback | **Combined-PEM fallback** — when the key path is empty, try the cert file for a key block; matches how some brokers ship identity bundles (R1). |

## Risks & mitigations

- **macOS file-dialog reentrancy** (shipped crash class): both pickers wrap their work in
  `QMetaObject::invokeMethod(..., Qt::QueuedConnection)` — same as `addCaCertificates()`.
- **`brokerChanged` comparison drift**: forgetting the new `BrokerConfig` fields would make
  cert changes silently not reconnect. The comparison edit is an explicit task, and
  `QSslKey::operator==`/`QSslCertificate::operator==` make it a value compare.
- **Passphrase leak into project JSON**: `toJson()` never serializes it; verified by AC4's
  on-disk grep and a `TlsIdentity`-side review that no error string echoes the passphrase.
- **Editable-field echo loop** (`common-mistakes.md` QML row): driver pane path fields use
  `Widgets.BoundField` (already unfocused-sync safe), not raw `text:` bindings.
- **Old-project compatibility** (AC6): all new JSON/QSettings reads use defaulted
  `value(...)` lookups; both-empty-paths short-circuits to today's behavior — no
  `setLocalCertificate`/`setPrivateKey` call at all.
- **Driver/publisher divergence**: both surfaces call the same `TlsIdentity` functions;
  review checklist item pins parity.
- **Trust-contract lane**: file list above is the lane; anything discovered outside it gets
  named in chat before touching.

## Test & verification plan

- **Unit (ctest tier, maintainer runs `ctest`):** new `TlsIdentity` TU — valid pair loads;
  combined PEM loads; missing file / non-PEM / encrypted-key-wrong-passphrase /
  encrypted-key-no-passphrase return their distinct typed errors (AC5 local half);
  `applyToSslConfiguration` sets local cert, private key, ALPN.
- **Integration (maintainer, live):**
  - AC1/AC2 — real AWS IoT thing, port 8883 then ALPN + 443, frames visible in the AWS
    MQTT test client (publisher) and dashboard fed from an AWS-side publish (driver).
  - AC3 — local mosquitto `require_certificate true`: connect with pair succeeds, without
    fails with the TLS-auth message, on both surfaces.
  - AC4 — configure, restart, reconnect; grep settings + project file on disk for the
    passphrase (must be absent; vault entry encrypted).
  - AC6 — existing MQTT pytest integration (`tests/integration/`) still green with the new
    fields untouched.
- **Hotpath:** not touched; no `--benchmark-hotpath` change expected (CI runs it anyway).
- **Static:** `python scripts/code-verify.py --check` on every touched file; `qt-cpp-review`
  before handoff; `python scripts/sanitize-commit.py` before commit.
