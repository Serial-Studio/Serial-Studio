---
spec: 0041-mqtt-mutual-tls
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-27
---

# Tasks 0041 — MQTT mutual TLS (client certificates) for AWS IoT Core

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — TlsIdentity helper TU

- **Files:** `app/src/MQTT/TlsIdentity.h`, `app/src/MQTT/TlsIdentity.cpp`
- **Does:** New `BUILD_COMMERCIAL` TU: `struct TlsIdentity { QSslCertificate certificate;
  QSslKey privateKey; }`, typed load error enum (MissingFile / Unreadable / NotPem /
  PassphraseRequired / PassphraseWrong) with a `tr()`-able message per value, a load
  function (cert path, key path, passphrase; combined-PEM fallback when key path empty),
  and `applyToSslConfiguration(QSslConfiguration&, const TlsIdentity&, const QByteArray&
  alpn)`. No GUI deps, no singletons — pure functions, ctest-linkable. Error strings never
  echo the passphrase.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/TlsIdentity.h
  app/src/MQTT/TlsIdentity.cpp`; read-back against the plan's error taxonomy.
- **Deps:** none
- [x] done

### T2 — Register TlsIdentity in the build

- **Files:** `app/CMakeLists.txt`
- **Does:** Add `TlsIdentity.{h,cpp}` to the commercial sources block, next to the other
  `MQTT/` entries. No other cmake change.
- **Verify:** Read-back; grep confirms both files listed exactly once.
- **Deps:** T1
- [x] done

### T3 — CredentialVault key-passphrase slot

- **Files:** `app/src/MQTT/CredentialVault.h`, `app/src/MQTT/CredentialVault.cpp`
- **Does:** Add `[[nodiscard]] QString keyPassphrase(const QString& host, quint16 port)
  const` and `void setKeyPassphrase(const QString& host, quint16 port, const QString&
  passphrase)` using a third encrypted `/keyPass` leaf under the existing sha1(host:port)
  key; empty passphrase removes the leaf (mirrors `/user`/`/pass` semantics). `Credentials`
  struct and existing methods untouched.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/CredentialVault.h
  app/src/MQTT/CredentialVault.cpp`; read-back for slot-removal symmetry.
- **Deps:** none
- [x] done

### T4 — ctest unit for TlsIdentity

- **Files:** `app/tests/tst_tls_identity.cpp`, `app/tests/CMakeLists.txt`
- **Does:** Unit tests with in-test generated/fixture PEM data: valid pair loads; combined
  PEM loads; missing file, non-PEM, encrypted-key-wrong-passphrase,
  encrypted-key-no-passphrase each return their distinct typed error;
  `applyToSslConfiguration` sets local certificate, private key, and ALPN on the config.
  Registered in the ctest tier like the sibling `tst_*` TUs.
- **Verify:** `python scripts/code-verify.py --check app/tests/tst_tls_identity.cpp`;
  maintainer runs `ctest` (AC5 local half).
- **Deps:** T1, T2
- [x] done

### T5 — Publisher state: BrokerConfig + properties + persistence

- **Files:** `app/src/MQTT/Publisher.h`, `app/src/MQTT/Publisher.cpp`
- **Does:** `BrokerConfig` gains `clientCertificate`, `clientPrivateKey`, `alpnProtocol`.
  `Publisher` gains members + Q_PROPERTYs + guard-return setters for
  `clientCertificatePath`, `privateKeyPath`, `keyPassphrase`, `alpnEnabled`,
  `alpnProtocol` (setters load/validate via `TlsIdentity`, reject-and-keep-prior on error,
  then `markConfigChanged()`); passphrase round-trips through the vault
  (`reloadCredentialsFromVault` / `persistCredentialsToVault` extended); `snapshotConfig()`
  copies the parsed identity + ALPN; `toJson()` / `applyProjectConfig()` add
  `clientCertPath`, `privateKeyPath`, `alpnEnabled`, `alpnProtocol` with defaulted reads
  (absent keys = feature off, AC6) — **passphrase never enters toJson()**; restore order:
  vault passphrase loaded before key parse. New `kKey*` constants. No in-header member
  init; Christmas-tree header blocks.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/Publisher.h
  app/src/MQTT/Publisher.cpp`; read-back: grep `toJson` body for passphrase (must be
  absent).
- **Deps:** T1, T3
- [x] done

### T6 — Publisher worker: apply identity + reconnect trigger + handshake hint

- **Files:** `app/src/MQTT/Publisher.cpp`
- **Does:** `applyBrokerConfig()` applies `setLocalCertificate` / `setPrivateKey` /
  `setAllowedNextProtocols` to `m_sslConfiguration` (skip entirely when identity empty —
  today's behavior byte-for-byte, R7/AC6) and **adds the three new fields to the
  `brokerChanged` comparison** (binding invariant: a missed field means cert changes
  silently never reconnect); `describeMqttError`/error path appends the
  client-certificate hint to `TransportInvalid` when an identity is configured. Worker
  thread unchanged otherwise — no new signals, connection-boundary only.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/Publisher.cpp`;
  read-back: every new `BrokerConfig` field appears in the `brokerChanged` expression.
- **Deps:** T5
- [x] done

### T7 — Publisher file pickers

- **Files:** `app/src/MQTT/Publisher.h`, `app/src/MQTT/Publisher.cpp`
- **Does:** `selectClientCertificate()` / `selectPrivateKey()` slots: file-mode
  `QFileDialog` (`*.pem *.crt *.cer *.key`), `WA_DeleteOnClose`, and the **queued-invoke
  body** in `fileSelected` (macOS reentrancy — same pattern as `addCaCertificates()`);
  on pick, call the T5 path setters; validation failure shows the typed error via
  `Misc::Utilities::showMessageBox`.
- **Verify:** `python scripts/code-verify.py --check app/src/MQTT/Publisher.h
  app/src/MQTT/Publisher.cpp`; read-back: dialog callback body is inside
  `QMetaObject::invokeMethod(..., Qt::QueuedConnection)`.
- **Deps:** T5
- [x] done

### T8 — Driver state: properties + persistence + apply

- **Files:** `app/src/IO/Drivers/MQTT.h`, `app/src/IO/Drivers/MQTT.cpp`
- **Does:** Members + Q_PROPERTYs + guard-return setters for the same five settings
  (NOTIFY `sslConfigurationChanged`); setters load/validate via `TlsIdentity`, apply into
  the member `m_sslConfiguration`, persist under `MqttInputDriver/` (`clientCertPath`,
  `privateKeyPath`, `alpnEnabled`, `alpnProtocol`), passphrase via vault, then
  `scheduleReconnectIfActive()`; `loadPersistedSettings()` restores (vault passphrase
  before key parse; defaulted reads, AC6); TLS-failure hint in `onErrorChanged()` when an
  identity is configured. Driver stays main-thread; no flow/retry changes (spec-0034:
  retry policy owns backoff, drivers never do).
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/MQTT.h
  app/src/IO/Drivers/MQTT.cpp`; read-back: no new timers/threads/retry state.
- **Deps:** T1, T3
- [x] done

### T9 — Driver pickers + driverProperties rows

- **Files:** `app/src/IO/Drivers/MQTT.cpp`, `app/src/IO/Drivers/MQTT.h`
- **Does:** `selectClientCertificate()` / `selectPrivateKey()` picker slots (queued-invoke
  body, as T7); `appendMqttSslProperties()` adds Text rows for cert/key paths, CheckBox for
  ALPN, Text for ALPN protocol (SSL-gated like the existing rows), Password row for the
  passphrase; `setDriverProperty()` handles the five new keys.
- **Verify:** `python scripts/code-verify.py --check app/src/IO/Drivers/MQTT.h
  app/src/IO/Drivers/MQTT.cpp`; read-back: property keys match setter names exactly.
- **Deps:** T8
- [x] done

### T10 — Publisher form model rows

- **Files:** `app/src/DataModel/Project/ProjectEditorItemIds.h`,
  `app/src/DataModel/Project/ProjectEditorMqtt.cpp`
- **Does:** Append `kMqttPublisher_ClientCertPath`, `_PrivateKeyPath`, `_KeyPassphrase`,
  `_AlpnEnabled`, `_AlpnProtocol` item ids; `buildMqttSslSection()` adds the five rows
  (TextField, TextField, PasswordField, CheckBox, TextField visible only when ALPN on;
  `Active` follows the section convention); `onMqttPublisherItemChanged()` cases — ALPN
  toggle rebuilds the model (same pattern as `kMqttPublisher_SslEnabled`). Existing
  signal wiring in the file untouched.
- **Verify:** `python scripts/code-verify.py --check
  app/src/DataModel/Project/ProjectEditorItemIds.h
  app/src/DataModel/Project/ProjectEditorMqtt.cpp`.
- **Deps:** T5
- [x] done

### T11 — Publisher toolbar buttons (QML)

- **Files:** `app/qml/ProjectEditor/Views/MqttPublisherView.qml`
- **Does:** Two `Widgets.ToolbarButton`s next to "Load CA Certs" — "Client Certificate…"
  and "Private Key…" — calling the T7 slots; enabled/tooltip gating mirrors the CA-certs
  button (`enabled && sslEnabled`). Icons via `Cpp_Misc_IconRegistry.icon(...)` only
  (never a hardcoded path); reuse existing editor-category icons.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/ProjectEditor/Views/MqttPublisherView.qml`; `python scripts/registry-verify.py`
  if any icon id is new.
- **Deps:** T7
- [x] done

### T12 — Driver setup pane fields (QML)

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml`
- **Does:** Under the TLS block, all `visible:`/`enabled:`-gated on
  `Cpp_IO_Mqtt.sslEnabled` (R7): cert path + key path `Widgets.BoundField`s each with a
  Browse button (T9 slots), passphrase field with `echoMode: TextInput.Password`, ALPN
  CheckBox + protocol field (visible when checked, placeholder `x-amzn-mqtt-ca`). Follow
  the existing label+field grid and Connections-sync patterns; `qsTr()` with `%1`
  placeholders only.
- **Verify:** `python scripts/code-verify.py --check
  app/qml/MainWindow/Panes/SetupPanes/Drivers/MQTT.qml`.
- **Deps:** T9
- [x] done

### T13 — Self-review + static gates

- **Files:** all touched files (read-only pass)
- **Does:** Re-read the full diff against the plan's file table: in-lane, no scope creep,
  driver/publisher `TlsIdentity` parity, counterfactual check (most-at-risk rule:
  `brokerChanged` completeness + passphrase-on-disk; name the evidence). Run the repo
  linter over every touched file and `qt-cpp-review` on the C++ diff.
- **Verify:** `python scripts/code-verify.py --check <all touched>` clean of new errors;
  `qt-cpp-review` findings addressed or noted in chat.
- **Deps:** T1–T12
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met or handed to the maintainer with its
      concrete check (AC1/AC2 AWS run, AC3 mosquitto, AC4 restart+grep, AC5 ctest +
      handshake, AC6 pytest regression).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (6-agent pass 2026-07-27); all confirmed findings
      fixed; deferred items noted in chat (cert-chain support, secret-scanner fixture noise,
      BUILD_COMMERCIAL self-define in the test target).
- [x] Hotpath untouched (plan says none) — no `--benchmark-hotpath` delta expected; CI gate
      confirms.
- [x] Relevant `pytest` tests identified for the maintainer (existing
      `tests/integration/` MQTT suite for AC6).
- [x] `python scripts/sanitize-commit.py` ran at commit time; the spec landed as commit
      b1929bdf. Only the maintainer ACs above remain open.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [x] `spec.md` status set to `done`.
