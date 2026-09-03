---
spec: 0075-review-remediation
package: WP-D (industrial drivers, Sparkplug, Influx)
tasks: WPD-T1 .. WPD-T9
status: complete except tst_ethernetip_worker (blocked, reason below)
---

# WP-D handoff

Findings closed: E1, E2, E3, E6, E11, E12, E14/K6, E15, E16, and part of M5. **E7 is NOT closed
here** — it belongs to WP-A's `LineProtocol.h` row and is still unfixed in this tree (see
"Invariants found", item 8). E13 is unassigned by any package (see "Patches", item 4). Every task
box in `tasks.md` "WP-D" is ticked; WPD-T7 is ticked 3-of-4 with the blocked suite named inline.

## Files changed

### Drivers and sinks

| Path | What changed |
|------|--------------|
| `app/src/IO/Drivers/S7/S7Pdu.cpp` | `decodeValue` returns invalid on an empty payload instead of `SS_ASSERT(!raw.isEmpty())`, because the length is the controller's (E1). |
| `app/src/IO/Drivers/S7.cpp` | `applyResult` counts a success item that does not decode as an item error and no longer credits it as a read. |
| `app/src/IO/Drivers/MQTT/SparkplugSession.{h,cpp}` | `reset()` keeps the slot table and clears only birth state, buffered traffic, values and counters; new `slotsJson()` / `restoreSlots()` plus the private `buildRestoredTable()` / `readSlotEntry()` (E2). `setGroupFilter` doc corrected: a slot key carries its own group, so a filter change cannot collide. |
| `app/src/IO/Drivers/MQTT/MQTTSparkplug.cpp` | New `appendSparkplugProperties()` (moved out of `MQTT::driverProperties`, adds the `sparkplugSlots` row while the lane is on) and `restoreSparkplugSlots()`; file-local `sparkplugJsonArray()`. |
| `app/src/IO/Drivers/MQTT.{h,cpp}` | `driverProperties()` calls the new helper (also brings the function back under the 100-line cap); `applySparkplugProperty` consumes `Keys::SparkplugSlots`. MQTT.cpp is now **1499 lines**, under the 1500 cap it used to exceed. |
| `app/src/DataModel/FrameKeys.h` | `Keys::SparkplugSlots` plus the four entry keys (`SparkplugNode/Group/Device/Metric`). |
| `app/src/MQTT/SparkplugPublisher.cpp` | `birthMessages()` resets `m_seq` to 0, so a mid-session rebirth NBIRTH carries seq 0 (E6). |
| `app/src/MQTT/PublisherWorker.cpp` | `processData()` re-declares a registry that grew since the birth, so a structure edit on an idle source still rebirths (E15, second half). |
| `app/src/IO/Drivers/Modbus.cpp` | `advanceAfterFailedPoll()` publishes the `[unit, fc, 0]` placeholder and steps the cursor; BOTH failure exits of `onReadReady` use it; `buildRtuFrame` takes the responding unit id and appends the CRC; `pollInterval` property min 10 -> 50 to match the QML validator (E3, E12). Now **1492 lines**. |
| `app/src/IO/Drivers/Modbus/ModbusRtuCodec.{h,cpp}` (new) | `functionCodeForType()` and `appendCrc()` (CRC-16/Modbus). Qt-Core-only, so the framing is unit-testable without a device; also what keeps Modbus.cpp under the TU cap. |
| `app/src/IO/Drivers/Modbus/ModbusRegisterGroups.cpp` | Type-aware request cap: `maxCountForType()` gives FC01/FC02 their own 2000-bit ceiling (E12). **Out-of-list file; see "Deviations from the file list".** |
| `app/src/IO/Drivers/Modbus/ModbusProjectGenerator.{h,cpp}` | Generated Lua skips a zero byte-count frame without decoding (still advancing the cycle) and resynchronises the group cursor on the (function code, byte count) pair when exactly one group matches; new private `buildGroupTable()` / `buildResyncHelper()`. |
| `app/src/IO/Drivers/OpcUaSession.{h,cpp}` | `verifyServerCertificate` reads TRUST first, so an accepted self-signed certificate dialed by IP is no longer refused for a hostname mismatch (E11); `Identity::allowPlaintextPassword` replaces the unconditional `allowNonePolicyPassword = true`. |
| `app/src/IO/Drivers/OpcUaSecurity.{h,cpp}` | `plaintextPasswordAllowed()` / `setPlaintextPasswordAllowed()`: a per-installation acknowledgement beside the trust store, default OFF. |
| `app/src/IO/Drivers/OpcUa.{h,cpp}` | `write()` returns -1 like the sibling read-only drivers; `allowPlaintextPassword` Q_PROPERTY + slot forwarding to `OpcUaSecurity`; `identity()` fills the new field. Not a driver property; see "Invariants found", item 3. |
| `app/src/IO/Drivers/Iec104.{h,cpp}`, `Iec104/Asdu.h` | Slot identity is `(ioa, typeId)` via the new `Iec104Proto::slotKey()`; `m_slotForIoa` -> `m_slotForKey` (`QHash<quint64,int>`); a report's live `kind` overwrites the restored one (E16). |
| `app/src/InfluxDB/Export.{h,cpp}` | One counted error per failed reply: `onSslErrors` records the reason, `failureMessage()` reports it once from `onReplyFinished`; `sampleEpochOffset()` re-samples the wall-clock offset at bootstrap, on an endpoint move and when the sink re-opens (E15). |
| `app/src/MQTT/CredentialVault.{h,cpp}`, `Publisher.cpp`, `app/src/AI/KeyVault.h`, `AI/Assistant.cpp`, `app/qml/AI/{AssistantPanel,KeyManagerDialog}.qml`, `app/qml/ProjectEditor/Views/InfluxSinkView.qml` | No user-facing string or docstring calls the credential store "encrypted" any more; it says "obfuscated in this machine's settings" (E14, K6). |
| `app/CMakeLists.txt` | Two contiguous entries for `ModbusRtuCodec.{h,cpp}`. |

### Tests

| Path | What it pins |
|------|--------------|
| `app/tests/tst_s7comm_pdu.cpp` | `aZeroLengthSuccessItemDecodesToNothing` drives the exact PDU from finding E1 (aborts a debug build pre-fix). |
| `app/tests/tst_sparkplug_session.cpp` | `resetClearsSession` became `resetKeepsSlotsAndClearsSession` (it pinned the buggy contract); new `reconnectWithReversedBirthsKeepsIndices`, `slotTableRoundTripsThroughJson`, `aMisplacedRestoreEntryIsRefusedWhole`. |
| `app/tests/tst_sparkplug_publisher.cpp` | The rebirth case now pins `seq == 0` (it pinned 1) and that data resumes at 1 afterwards. |
| `app/tests/tst_modbus_generation.cpp` | `parserIdentifiesGroupsByCodeAndSize`, `parserSkipsTheFailedPollPlaceholder`. |
| `app/tests/tst_modbus_register_groups.cpp` (new) | Both request caps, duplicate/empty refusal, restore ordering, out-of-range drop, truncation, JSON shape, function codes, golden CRC. |
| `app/tests/tst_opcua_security.cpp` (new) | Identity reuse, SAN hostname matching, no wildcard against an IP literal, **trust independent of the hostname**, revoke/return, PEM->DER, plaintext-password default. |
| `app/tests/tst_opcua_frame_assembler.cpp` (new) | Per-array-element slots, delta-only encoding, array fan-out, Bad status keeping the last good value, **frame splitting with a rotating cursor**, monotonic stamps, one type-mismatch warning per slot. |
| `app/tests/tst_opcua_subscriptions.cpp` (new) | Individual vs all-refused monitored items, notifications reaching the cache, what a reset forgets and keeps, interval adoption. Driven through a stub host holding an unopened session. |
| `app/tests/tst_iec104_slots.cpp` (new) | One address under two type ids is two slots; keys never collide across the 24-bit address range; the type id decides the value class. |
| `app/tests/fuzz/fuzz_{s7_pdu,isotsap,iec104_apci,iec104_asdu,sparkplug_payload,opcua_wire}.cpp` (new) | One `LLVMFuzzerTestOneInput` per untrusted-bytes codec, with 20 seed files under `app/tests/fuzz/corpus/<name>/` (the E1 trigger PDU is one of them). |
| `app/tests/CMakeLists.txt` | Four new `ss_add_unit_test` blocks and the six `ss_add_fuzz_target` calls under `# spec 0075 fuzz targets`, guarded by `if(COMMAND ss_add_fuzz_target)` so the file configures before WP0 lands. |
| `tests/scripts/test_cpp_regressions.py` | New sections R20 (Modbus placeholder / CRC / caps), R21 (OPC UA trust order, plaintext grant, write -1), R22 (IEC 104 slot key, Influx single count, no "encrypted" credential-store claim anywhere under `app/qml`, `app/src/MQTT`, `app/src/AI`). **These run and pass: `pytest tests/scripts/ -q` -> 311 passed.** |
| `tests/integration/test_modbus_groups.py` (new) | Stub Modbus TCP server with an injected dropped reply; asserts every published frame is a valid RTU frame and that a dropped poll never puts two group-B frames back to back. |
| `tests/integration/test_sparkplug_host.py` (new) | Own mosquitto instance the test cycles; asserts every wire index survives the broker cycle when the nodes re-birth in reverse order, and that a clean birth raises no hardening counter. |

## Verification run

- `python3 scripts/code-verify.py --check <every changed/added C++/QML file>`: **0 errors**, 2
  advisories, both pre-existing `cxx-tu-too-long` on files I did not grow (`OpcUa.cpp` +1 line,
  `Publisher.cpp` unchanged).
- `python3 scripts/code-verify.py --tu-census --check`: **3966 excess (baseline 3968)**: the
  surface SHRANK. It asks for a re-baseline; `scripts/` belongs to WP0, so the coordinator should
  run `python scripts/code-verify.py --tu-census --accept` at integration.
- `--singleton-census --check`: unchanged (1584 / 1085).
- `claim-verify.py`: 0 errors, 2 pre-existing advisories in `scripting.md`.
- `pytest tests/scripts/ -q`: 311 passed (was 305; +6 new source-level regressions).
- ctest and the app were NOT run (per the brief).

## Deviations from the file list

Three files I touched are not named by a WPD task, all of them one-line-per-site and all inside the
concerns my brief assigns me ("Modbus frame emission + caps + RTU builder", "vault docstrings and
UI strings"):

1. `app/src/IO/Drivers/Modbus/ModbusRegisterGroups.cpp` — the 125-register cap that WPD-T4 asks me
   to make type-aware lives here, not in `Modbus.cpp`; a group past the cap is refused at `add()`
   time, so there is no other place the 2000-bit rule can be applied. No other package owns it.
2. `app/src/IO/Drivers/Modbus/ModbusRtuCodec.{h,cpp}` (new) — created so `Modbus.cpp` stays under
   the 1500-line cap it was ALREADY close to (1459 before, 1527 with the fix inline) and so the CRC
   is testable. Registered contiguously in `app/CMakeLists.txt`.
3. `app/src/AI/{KeyVault.h,Assistant.cpp}`, `app/qml/AI/*.qml`, `app/qml/ProjectEditor/Views/InfluxSinkView.qml`,
   `app/src/MQTT/Publisher.cpp` — WPD-T6's own Verify line greps `app/qml app/src/MQTT app/src/AI`
   for credential-store "encrypted" claims, which cannot pass without these. WP-H owns
   `app/qml/Dialogs/Settings*.qml` for the same wording; I did not touch those, so there is no
   overlap. Each edit is a single string or `@brief` line.

## Tasks not done, and why

**`tst_ethernetip_worker` (part of WPD-T7) is blocked.** The task assumes `kEipBackend` is an
injectable seam. It is not. It is a `static constexpr const char*` label, and the seam is a
set of FILE-LOCAL static functions (`createHandle`, `releaseHandles`, the read path) selected by
`#ifdef SS_EIP_ACTIVE` inside `EthernetIp.cpp`. `EipPollWorker` cannot be linked into a suite
without the whole driver TU, which resolves `AppState`, `ProjectModel` and `ConnectionManager`
singletons in its constructor. Turning the backend into an injectable interface is exactly
**WPI-T1 (`PolledPlcWorkerBase`)**; the suite should land there, against the base class, once the
seam exists. Nothing else in WP-D depends on it.

## Patches for the coordinator

### 1. `app/src/API/Handlers/ModbusHandler.cpp`: the API path still caps every type at 125

`addRegisterGroup` validates `count` against 125 regardless of register type, so the 2000-bit coil
read the driver now accepts is refused over the API and in a project applied through it. Same
fix, same reason as `ModbusRegisterGroups::add`:

```diff
@@ API::Handlers::ModbusHandler::addRegisterGroup
-  if (count < 1 || count > 125) {
+  const int maxCount = (type == 2 || type == 3) ? 2000 : 125;
+  if (count < 1 || count > maxCount) {
     return CommandResponse::makeError(
       id,
       ErrorCode::InvalidParam,
-      QStringLiteral("Invalid count: %1. Valid range: 1-125").arg(count));
+      QStringLiteral("Invalid count: %1. Valid range: 1-%2")
+        .arg(QString::number(count), QString::number(maxCount)));
   }
```

`tests/integration/test_modbus_groups.py` does not exercise a 2000-bit group, so this is not a
test blocker; `tst_modbus_register_groups` covers the driver-side rule.

### 2. `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUa.qml`: the plaintext-password opt-in has no control

`OpcUa::allowPlaintextPassword` is a settable Q_PROPERTY with no UI, so a user whose server needs
None-policy username login has no way to grant it and the login now fails closed. The natural home
is beside the existing `credentialsExposed` banner (around line 311):

```diff
     Label {
       Layout.columnSpan: 2
       ...
       text: qsTr("Credentials travel in the clear on this channel. Choose a policy other than None with Sign and Encrypt to protect them.")
     }
+
+    Item {}
+    Widgets.Checkbox {
+      Layout.columnSpan: 1
+      Layout.fillWidth: true
+      visible: Cpp_IO_OpcUa.credentialsExposed
+      enabled: app.ioEnabled
+      checked: Cpp_IO_OpcUa.allowPlaintextPassword
+      text: qsTr("Send the password anyway on this unencrypted channel")
+      onCheckedChanged: Cpp_IO_OpcUa.allowPlaintextPassword = checked
+    }
```

(The exact widget type should match the pane's other checkboxes; I did not edit QML outside my
file list.) Until this lands, the grant is reachable only through the QSettings key
`OpcUaDriver/allowPlaintextPassword`.

### 3. Wildcard disconnects left in place

`OpcUaSubscriptions::unbindSession` uses the banned `disconnect(session, nullptr, this, nullptr)`
form (`app/src/IO/Drivers/OpcUa/OpcUaSubscriptions.cpp`). It is on the list WP0-T13's new lint rule
will catch and WP-I fixes (C9/E8); I left it alone because it is outside my tasks. Flagging it so
the lint rule's baseline is not seeded with it as "accepted".

### 4. Finding E13 is unassigned

`MQTT.cpp` E13 (a directory handed to `addCaCertificates`, the vault written twice (momentarily
with an empty password) during `loadPersistedSettings`, and NCMD rebirths published while
`isWritable()` reports false) is cited by requirement R5.13 but no WP-D or WP-C task carries it.
The vault double-write is the one with teeth: `loadPersistedSettings` calls `setUsername(...)`
before `setPassword(...)`, and the first call removes the `/pass` key. It needs a single
`m_vault.setCredentials(host, port, user, pass)` after both members are set.

## Invariants I found that the plan did not state

1. **`Keys::` does not live in `Frame.h`.** Both `CLAUDE.md` and `tasks.md` say the namespace is at
   the top of `app/src/DataModel/Frame.h`; it is in `app/src/DataModel/FrameKeys.h`, which
   Frame.h includes. I edited FrameKeys.h. WP-J should correct the two doc statements.
2. **`keys-hardcoded-literal` does not read Frame.h.** The linter matches against a hand-curated
   `_PROJECT_KEY_LITERALS` set in `scripts/code_verify_rules.py`, so adding a `Keys::` constant does
   NOT make the raw literal an error. If the new Sparkplug keys should be enforced, WP0 has to add
   them to that set.
3. **A security acknowledgement must not be a driver property.** `applyConnectionSettings` replays
   every key of a project's `connection` object through `setDriverProperty`, so anything exposed
   there is granted by OPENING SOMEONE ELSE'S PROJECT. That is why `allowPlaintextPassword` lives in
   `OpcUaSecurity` (per-installation, beside the trust store) and is absent from
   `OpcUa::driverProperties()`. R5.5 states the same rule for consent generally; the plan's WP-D row
   did not say where the opt-in should live.
4. **`QJsonObject` iterates in sorted key order**, which is what makes the `sparkplugSlots` restore
   safe: `applyConnectionSettings` applies `sparkplugEnabled` < `sparkplugGroupId` < `sparkplugSlots`,
   so the table is restored after the group filter is set. The restore is also refused once the
   session holds slots of its own, so the UI instance can never clobber a live table.
5. **Keeping the slot table across `setGroupFilter` is safe and required.** A slot key already
   carries its group, so an old-group slot can never collide with a new one; clearing on a filter
   change would have re-opened E2 through the connect path, because
   `sparkplugStateChanged(connected)` sets the filter on every connect.
6. **`SS_ASSERT_LOG` aborts in debug builds too** (`SS_ASSERT_IS_FATAL()`), so it is not a "log
   only" macro. I removed the `SS_ASSERT_LOG(registerType <= 3)` I had first written into
   `ModbusRtu::functionCodeForType`: the register-type index arrives from a project file or the API.
   This is the same class as E1 and is worth stating in the ledger.
7. **The generator's `%` escape.** `ModbusProjectGenerator::buildFrameParser` cannot use
   `QString::arg` for the Lua modulo: `arg()` does not collapse `%%`, so the escape reaches the
   generated source verbatim. The existing code appends the number separately and my new
   zero-length branch does the same.
8. **E7 is still open and is NOT mine.** I checked: `app/src/InfluxDB/LineProtocol.h` still has
   `kMeasurementSpecials = ", "` and `kKeySpecials = ",= "`, neither of which contains a backslash,
   so a dataset titled `Temp\` still renders `Temp\=21.5` and 400s the whole batch. The file is
   assigned to **WP-A** ("Backslash added to the measurement/tag/field-key special sets (E7)") and
   no WP-D task lists it, so I left it alone. Confirming it is unfixed so the ledger is not closed
   on it by mistake — WPD-T6 covers only the Influx COUNTER and clock-offset halves of E15.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Source owns time", via the InfluxDB change: I
made `m_epochOffsetNs` re-sample when the sink re-opens, which changes the wall-clock mapping of
points written after a reconnect.

**Evidence it does not.** The offset is not a stamp. It is the steady-to-wall DISTANCE, and
`epochNs()` still returns `block.t0 + sample_offset_ns(block, index) + offset`. The sample's own
capture instant and the spacing the source produced are untouched; only the constant that projects
them onto the wall clock is refreshed, which is what makes an NTP step stop corrupting later points
instead of causing it. Nothing in the change re-stamps a frame, and no sink writes a time it
invented: `sampleEpochOffset()` is called at bootstrap, on an endpoint move and on the
unhealthy->healthy edge, never per block. The pins are `test_influx_counts_one_error_per_failed_write`
(which asserts `sampleEpochOffset()` appears in `onReplyFinished` and at least three times overall)
and the unchanged `tst_influx_lineprotocol`.

**Runner-up, the hotpath.** Nothing in WP-D touches `FrameBuilder`, `StreamWorker`,
`FrameConsumer`, `Dashboard`, `CircularBuffer`, `FrameReader` or `PipelineHost`. The two additions
on a per-message path are `SparkplugSession::reset()` (session-edge rate) and
`PublisherWorker::processData()`'s `needsRebirth()` check, which is a plain bool read on the
worker's own consumer tick, allocates nothing, and is on the publisher's queue-drain path, not the
acquisition publish path. `Modbus::advanceAfterFailedPoll` allocates one 5-byte frame per FAILED
poll (poll-interval rate, floor 50 ms), which is the same allocation class as the successful
`buildRtuFrame` beside it.
