---
spec: 0075-review-remediation
phase: plan
status: approved   # draft -> approved (gate before /ss-tasks)
updated: 2026-09-01
---

# Plan 0075 — Remediate the 2026-09-01 full source review

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Evidence ids (A1..M12) refer to [`findings.md`](./findings.md).
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Eleven work packages on eleven branches, each owning a disjoint file set, merged in
dependency order: CI integrity and test harnesses first (WP0), then eight defect packages
that can run in parallel because they own different directories (WP-A hotpath and sinks,
WP-B script deadlines, WP-C IO core and general drivers, WP-D industrial drivers and sinks,
WP-E dashboard and QML, WP-F project layer and editors, WP-G API surface, WP-H assistant,
extensions, CLI and licensing), then two debt packages on top of fixed code (WP-I one
implementation per concern, WP-J documentation truth). Every defect fix lands with the test
that fails on the pre-fix tree, in the same branch. Files two packages both need
(`FrameBuilder.cpp`, `StreamWorker.cpp`, `ConnectionManager.cpp`, `Dashboard.cpp`, `ci.yml`,
the code-editor hosts) have exactly one owning package; the other package delivers a helper or
a patch through the coordinator. Execution model per the maintainer's decision: one agent per
package, a written brief per package naming invariants, owned files and tests; the
coordinator integrates shared files and reviews each branch before merge.

## Affected subsystems & files

Grouped by package. "Owner" means the only package that edits the file in this spec. New
files marked (new). Paths confirmed by the seam investigation of 2026-09-01; line anchors in
`findings.md`.

### WP0 — CI integrity, tooling, harnesses (R11, R14 scaffolding)

| File | Change |
|------|--------|
| `.github/workflows/ci.yml` | `upload` gets `if: default branch or tag` + `concurrency: release-<ref>`; `test` gains `needs` on the build jobs and downloads build artifacts instead of the Release; `upload` needs `[test, lint]`; `REQUIRE_TESTS_TO_PUBLISH` defaults `true`; every `uses:` pinned to a 40-hex SHA with a version comment; secrets moved from `run:` text to step `env:`; `permissions:` on every job; training activation `\|\| true` removed; FORTIFY compile-line assert step on Linux; ctest configure/build/run added to macOS and Windows jobs (`build/unit-ci` pattern from L278-301); new `sanitize` job (ASan+UBSan ctest + fuzz smoke + `--benchmark-hotpath --min-fps 1`, and a TSan ctest leg); benchmark step wrapped in a two-attempt runner script; `qmllint` step in `lint`. |
| `cmake/Hardening.cmake` | Per-target block: emit `-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=<n>` as one ordered `target_compile_options` pair (no `target_compile_definitions`), level 3 when GCC >= 12 and glibc >= 2.34 else 2; directory-level pair removed for targets that opt in. |
| `cmake/Sanitizers.cmake` | Add `ENABLE_FUZZERS` (clang `-fsanitize=fuzzer-no-link`) and expose ASan/UBSan/TSan as documented options the CI job sets. |
| `app/tests/CMakeLists.txt` | `ss_add_fuzz_target(name SOURCES LIBS)` helper (libFuzzer main, a `QTest`-driven corpus replay fallback when fuzzers are off so the target still builds and runs under ctest); `ss_add_unit_test` errors instead of silently returning on a missing source. |
| `app/tests/fuzz/` (new) | Fuzz entry points (see WP-D, WP-A, WP-G, WP-H rows) plus `corpus/` seeds. |
| `app/tests/support/FakeDriver.{h,cpp}` (new) | `HAL_Driver` test double with scripted `open()` outcomes (sync ok/fail, async ok/fail after N ms, drop after open) for the verdict-matrix and pause tests. |
| `app/tests/support/FakeProvider.{h,cpp}` (new) | `AI::Provider`/`Reply` double replaying a scripted stream (text, tool call, error, budget breach). |
| `app/tests/support/FakeTransport.{h,cpp}` (new) | `QNetworkAccessManager` replacement returning canned `QNetworkReply`s (for extension installer and provider reply tests). |
| `scripts/tests/test_code_verify.py` (new), `scripts/tests/fixtures/<rule>/` (new) | One positive and one negative fixture per rule kind in `code-verify.py` and `code_verify_rules.py`; run by `lint` job and by `sanitize-commit.py`. |
| `scripts/code-verify.py` | Default action becomes `--check`; `--fix` explicit only; new `cxx-duplicate-window` advisory (10 normalized lines, >40 shared windows per file pair) with `scripts/dup-census.json` baseline and `--dup-census --check/--accept` ratchet; `tu-cutter.py` references dropped from advisory text. |
| `scripts/code_verify_rules.py` | Rule for `disconnect(x, nullptr, y, nullptr)` wildcard slot form (error), covering the three sites the linter missed (C9, E8). |
| `scripts/claim-verify.py`, `scripts/doc-anchors.json` | New anchor kind `ordered` (multi-capture regex over one source file compared in order with a doc list) used to pin `instantiateCoreModules()` against `startup.md`. |
| `scripts/tu-cutter.py` | Deleted. `doc/claude/scripts.md` row removed. |
| `tests/requirements.txt`, `tests/requirements.lock` (new) | Lock generated with hashes; CI installs `--require-hashes -r tests/requirements.lock`; `reuse` pinned. |
| `tests/pytest.ini` | Register the `dos` marker CI already deselects; document xfail policy in `tests/README.md`. |
| `tests/security/test_access_control.py`, `tests/security/test_unknown_input_hardening.py` | Three by-design xfails deleted; the stress-crash xfail references finding I1/M9 and flips to a real assertion in WP-G. |
| `app/CMakeLists.txt` | Remove the five duplicate entries (L373/L683, L630/L640, L1569-1574/L1610-1615, L1576/L1757, L2092/L2188); `qt_add_qml_module` gains a `qmllint` target with a checked-in baseline (`app/qml/qmllint-baseline.json`). |
| `CMakeLists.txt` | `find_program` for `ar`/`ranlib`; `URL_HASH` or commit-SHA `GIT_TAG` on every FetchContent (zlib, expat, mimalloc). |
| `lib/VERSIONS.json` (new) | Machine-readable upstream version/commit per vendored tree; `claim-verify` pins io.md/scripting.md version claims to it. `lib/luajit/CMakeLists.txt` comment corrected; `lib/QSimpleUpdater/{CLAUDE.md,sanitize-commit.sh,tests/,tutorial/,etc/}` removed. |
| `app/src/SelfTest/SelfTest.cpp`, `app/src/SelfTest/QmlInstantiationSuite.cpp` (new) | `qml` suite: builds a `QQmlEngine` with stub context objects for every registered `Cpp_*` name and instantiates every file under `qrc:/serial-studio.com/`, failing on `ReferenceError`; runs after module init through a new CLI hook (the existing suites stay pre-root). |

### WP-A — Acquisition hotpath and sinks (R1, R3.1, R3.3, R3.5, R3.10; A1-A5, A11-A12, B1-B10, B14-B18)

| File | Change |
|------|--------|
| `app/src/DataModel/FrameBuilder.cpp/.h` | `onConnectedChanged` and a new `onPausedChanged` slot flush `m_openBlocks` before clearing state, then emit `sessionBoundary(bool connected, bool paused)` (queued to GUI); the two per-frame `isFinalValuePlayerOpen()` calls read `m_playerOpen`; `syncFromProjectModel` on the parked-inline path stores the snapshot in `m_deferredProjectSnapshot` and applies it when the park bracket closes (`PipelineHost::setPipelineParkedOnGui(false)` hook); synthetic republish limited to table-fed datasets (A11). |
| `app/src/DataModel/FrameBuilder/BlockStager.{h,cpp}` (new) | Staging + open-block map + flush logic moved out of the facade (also serves R12.8). |
| `app/src/IO/PipelineHost.{h,cpp}` | `parkedOnGuiChanged` hook so the builder can apply a deferred snapshot when the bracket closes. |
| `app/src/IO/StreamWorker.{h,cpp}` | `JsWatchdog` owned per worker engine, armed per block around `runJsBlockTransform` / per sample pass; timeout falls back to raw and counts (uses WP-B's helper); Lua Fast mode keeps no hook by design and is documented; out-of-range channel clears the column (A12); `compileEngines` before feed connect. |
| `app/src/DataModel/FrameConsumer.h/.cpp` | Threshold post coalesced through `std::atomic<bool> m_flushPosted` (worker clears at `processData` entry); second SPSC lane helper so the raw-bytes queue gets the same threshold trigger. |
| `app/src/CSV/Export.cpp/.h`, `app/src/CSV/SparseRowMerger.h` | Close on `FrameBuilder::sessionBoundary` instead of `connectedChanged`/`pausedChanged`; per-source `monotonicFrameNs` baselines (`std::unordered_map<int, qint64>`), irregular blocks use `t0 + times[i]`; interval-mode write failure closes and reports like the sparse path. |
| `app/src/MDF4/Export.cpp/.h` | Same boundary and per-source time change; `createTimeChannel` sets `Sync(Time)`; absolute-epoch write into the master removed (B10). |
| `app/src/MDF4/PlayerLoaderWorker.{h,cpp}` | Reader accepts `cn_sync_type` 0 from a Serial Studio writer as time (decision); decode cache rebuilt per channel group as time-major columnar arrays (per-group timestamp vector + per-channel value vector), merged lazily by the player; memory bound = samples x channels-in-group. |
| `app/src/Sessions/Export.cpp/.h` | Close on `sessionBoundary`; `transaction()`/`commit()`/`exec()` results checked: failure -> `m_writeFailed` atomic, `dropped` counter, `writeErrorChanged` queued to GUI, `isRecording` false, no `finalizeSession` fingerprint over lost rows; raw lane threshold flush + overrun counter exposed in `sessions.getStatus`; raw chunks before the first block keep their own capture ns (B18). |
| `app/src/Sessions/DatabaseManager.cpp`, `Sessions/DatabaseWorker.cpp` | `deleteSession`/edit verbs refuse `Sessions::Export::currentSessionId()` with `SESSION_LIVE`; `openDatabase` validates the SQLite header and surfaces failure; readers open `QSQLITE_OPEN_READONLY` and skip the WAL pragma (B15). |
| `app/src/Sessions/Player.cpp/.h` | `m_playbackEpoch` guard on both timer chains (copy of the CSV pattern). |
| `app/src/Sessions/PlayerLoaderWorker.cpp` | Timestamp index built once per distinct `(t0_ns, dt_ns, frames, times_blob)` row group via SQL `GROUP BY`, not per dataset row. |
| `app/src/CSV/Player.cpp` | `catchUpTargetRow` stops at a backwards timestamp and re-anchors (B9). |
| `app/src/MQTT/Publisher.{h,cpp}` | Hotpath reads only `std::atomic<bool> m_hotEnabled`, `std::atomic<int> m_hotMode`, `std::atomic<bool> m_hotSparkplug`; topic base is read on the worker from a `std::shared_ptr<const QString>` swapped atomically by the setter (A5). |
| `app/src/InfluxDB/LineProtocol.h` | Backslash added to the measurement/tag/field-key special sets (E7). |
| `app/src/Console/Handler.cpp` and the three player `eventFilter`s | Space/arrow filter scoped to the player's own view having focus (B19). |
| `app/tests/tst_frame_builder_staging.cpp` (new), `tst_stream_worker.cpp`, `tst_sessions_export_worker.cpp` (new), `tst_csv_export_times.cpp` (new), `tst_mdf4_export_times.cpp` (new), `tst_sessions_player_epoch.cpp` (new), `tst_mdf4_loader_memory.cpp` (new), `tst_frame_consumer.cpp`, `tst_influx_lineprotocol.cpp` | Tests per M1/M2 (table below). |
| `tests/integration/test_recording_fidelity.py` (new) | Two simulator sources through CSV/MDF4/Historian across connect, pause, resume, disconnect; disk-full via read-only session directory. |
| `app/tests/fuzz/fuzz_block_codec.cpp`, `fuzz_csv_row.cpp`, `fuzz_mdf4_reader.cpp` (new) | Untrusted-bytes fuzzers (R14.3). |

### WP-B — Script execution deadlines (R2; A1 helper, H2, I4, J3-adjacent)

| File | Change |
|------|--------|
| `app/src/DataModel/Scripting/LuaDeadlineHook.{h,cpp}` (new) | One helper: installs the `LUA_MASKCOUNT` hook with a `QDeadlineTimer`, `lua_error`s on expiry, reports `timedOut()`. Used by `TransformCompiler`, `StreamProcessor` (via WP-A), editors, `MacroRunner`, `PublisherScriptEditor`. |
| `app/src/DataModel/Scripting/ScriptDryRun.{h,cpp}` (new) | `runJsDryRun(code, prelude, budgetMs, callbacks)` and `runLuaDryRun(...)`: throwaway engine + `JsWatchdog` / `LuaDeadlineHook`, returns `{ok, timedOut, error}`. Lifted from `ControlScriptHandler::dryRun`. |
| `app/src/API/Handlers/ProjectDryRunCommands.cpp`, `ControlScriptHandler.cpp` | Both bare engines replaced by `ScriptDryRun`; `controlScript.dryRun` re-based on it. |
| `app/src/DataModel/Editors/ControlScriptEditor.cpp`, `DatasetTransformEditor.cpp`, `app/src/DataModel/Dialogs/TransmitTestDialog.cpp`, `app/src/DataModel/Scripting/MacroRunner.cpp`, `app/src/MQTT/PublisherScriptEditor.cpp` | Validate/Test/Apply and macro/publisher previews go through `ScriptDryRun`; Lua paths get `LuaDeadlineHook`. (WP-F owns the editor files for its own changes; WP-B lands first, WP-F rebases.) |
| `app/src/DataModel/FrameBuilder/TransformCompiler.cpp` | Uses `LuaDeadlineHook` instead of its inline hook (no behaviour change). |
| `app/tests/tst_script_dryrun.cpp` (new), `tst_lua_deadline_hook.cpp` (new) | `while(true)` in JS and Lua returns timeout within budget on every surface. |
| `tests/integration/test_script_deadlines.py` (new) | Submits a looping script to every API-reachable surface (parser, transform per lane, control script, output widget, painter, dry-runs, editor validate via `project.*` commands) and asserts a timeout error plus a live ping afterwards. |

### WP-C — IO core and general-purpose drivers (R4.1-R4.2, R5.1-R5.9, R3 pieces; C1-C3, C8, C11-C13, D1-D21, E4, E5-adjacent, E9)

| File | Change |
|------|--------|
| `app/src/IO/AsyncTcpDial.{h,cpp}` (new) | GUI-thread QObject: `QHostInfo::lookupHost` (async) -> optional QTimer-paced refusal probe on a throwaway socket (keeps the control-script bind window) -> one `connectToHost` on the caller's socket -> `finished(ok, reason)` exactly once; bounded by one deadline; `cancel()`. Used by Network TCP, Modbus TCP pre-probe, Iec104, MQTT (timer only), OPC UA (resolution only). |
| `app/src/IO/Drivers/Network/NetworkTcp.cpp`, `Network.{h,cpp}` | TCP dials through `AsyncTcpDial`; `isConnecting()` covers TCP; verdict via `succeedDial`/`failDial` like WS/HTTP; `driverProperties()` emits every transport's rows (D17); `readDatagram` return checked; HTTP body cap. |
| `app/src/IO/Drivers/Modbus.cpp` | Pre-probe via `AsyncTcpDial`; coil/discrete request cap (2000 bits); RTU frame builder emits CRC and the responding unit id; `pollInterval` bound matches the UI; on a reply error publish a zero-length placeholder frame `[slave, fc, 0]` so the generated parser's group cursor stays in step (E3, compat-preserving). |
| `app/src/IO/Drivers/Modbus/ModbusProjectGenerator.cpp` | Generated Lua skips zero-length frames without advancing decode, resyncs on function code + byte count. |
| `app/src/IO/Drivers/Iec104.cpp` | `dialStation` async through `AsyncTcpDial`; verdict through `openFinished`; `QHostInfo` sync call removed. |
| `app/src/IO/Drivers/MQTT.cpp` | Dial timer (15 s) -> `failDial`; internal re-dial `open()==false` -> `reportOpenFinished(false)` (E4); CA path directory refused; vault written once; NCMD rebirth gated on writable. |
| `app/src/IO/Drivers/OpcUaSession.cpp`, `OpcUa.cpp` | Host resolved with `QHostInfo` before `UA_Client_connectAsync`; numeric endpoint URL with the original hostname kept for the verifier; idle pump timer 10 ms -> adaptive (100 ms idle, 10 ms while a request is outstanding); process-global `setFilterRules` removed from the ctor. |
| `app/src/IO/Drivers/OpcUa/OpcUaEndpointSelection.cpp` | Deprecated policies score below "no endpoint" (E10). |
| `app/src/IO/Drivers/S7.cpp`, `EthernetIp.cpp` | `open()` starts the worker dial and returns with `isConnecting()`; the worker reports `openFinished` (async drivers under the spec-0050 latch); `waitForConnected` stays on the worker. (Base extraction is WP-I.) |
| `app/src/IO/Drivers/CANBus/SerialCanBackendBase.{h,cpp}` (new) | Owns `QSerialPort`, open prologue, `readyRead` + `errorOccurred` (ResourceError -> `setError(ReadError)` + `close()`), bounded `m_rxBuffer`, `close()`; Slcan/Seeed become protocol-only subclasses (`sendInit`, `parseLine`, `encodeFrame`); Slcan id parse `ok` fixed; open verdict reads the adapter's reply. |
| `app/src/IO/Drivers/CANBus.cpp` | `onStateChanged` Unconnected while open -> queued `disconnectDevice(this)` (one established-drop path, D10); per-frame emission batched by `logicalFramesHint` (D13). |
| `app/src/IO/Drivers/BluetoothLE.cpp` | `disconnected` while open -> the same drop path; discovery dedupe by address; `characteristicIndex` validated. |
| `app/src/IO/ConnectionManager.{h,cpp}` | `connectDevice(int, ResumePolicy)`: user paths resume, `connectDevice(HAL_Driver*)` (auto-reconnect) keeps pause (C2); cancel-during-dial never emits `sessionClosed` (C13); QuickPlot rebuild keeps device 0 alive across the swap (C8); `SS_ASSERT`s in the five entry points; `setBusType` defers destruction like `rebuildDevices` (C7). |
| `app/src/IO/DeviceManager.cpp` | Wildcard `disconnect` replaced by captured connections; pipeline-thread object teardown before join. |
| `app/src/IO/Drivers/UART.cpp` | Custom-path ports honour `ResourceError` (D5); `registerDevice` failure -> `logDriverError` + NotificationCenter, no modal (D4); persisted auto-select runs once per port-list change and `setPortIndex(0)` clears the key (D9); dead mutex removed; `errorOccurred` wired after open. |
| `app/src/IO/Drivers/USB.cpp` | `setTransferMode(Advanced)` without recorded consent -> refuse + `logDriverError` + ProblemCenter finding; the UI button path asks once and records consent (D4); `write()` moved to the pump thread (D14). |
| `app/src/IO/Drivers/Audio.cpp/.h`, `app/src/IO/Drivers/Audio/PlaybackRing.h` (new) | Capture-only sessions ignore output absence (`playback.channels > 0` gate, and "none selected" distinguished from "vanished"); playback = fixed SPSC byte ring drained at device rate, underrun zero-fill + atomic counter; input path stops allocating `QByteArray` per block (pre-sized pool) and counts drops; `applyConnectionSettings` goes through the setters (D7); `open()` failure reasons translated. |
| `app/src/IO/Drivers/HID.cpp`, `Process.cpp` | HID `open()` closes first; Process `doClose` and `refreshProcessList` non-blocking (signal + timer); stderr kept out of the frame stream; double `disconnectDevice` guarded. |
| `app/src/IO/Drivers/Audio/AudioContext.{h,cpp}`, `IO/Drivers/UsbContext.{h,cpp}`, `IO/Drivers/HidContext.{h,cpp}` (new) | Refcounted native contexts shared by the UI and live instances (D11); live instances `setPersistent(false)` via `DriverFactory` (D12). |
| `app/src/IO/FileTransmission/XMODEM.cpp`, `ZMODEM.cpp`, `FileTransmission.cpp`, `Protocol.h` | NAK/timeout set `SendingBlocks` before resend (C1); typed `protocolError()` signal replaces string sniffing (C3); ZMODEM seek failure closes state, ZRPOS cancels the pending chain (C11); blank lines sent (C12). |
| `app/src/Licensing/MachineID.cpp` | Persisted last-good id used without spawning; tool spawns only on first run, on a worker thread, with a 500 ms cap each (K11). |
| `app/src/Misc/Extensions/PluginRunner.cpp` | `waitForStarted` -> `started`/`errorOccurred` + timer; quit path bounded to 1 s total then terminate (K12). |
| `app/tests/tst_connect_fanout.cpp`, `tst_connection_verdicts.cpp` (new, FakeDriver), `tst_serial_can_backend.cpp` (new), `tst_playback_ring.cpp` (new), `tst_async_tcp_dial.cpp` (new), `tst_xymodem.cpp` (XFAILs flipped), `tst_file_transmission.cpp` (new), `tst_ui_driver_sync.cpp`, `tst_stream_config_builder.cpp` (new) | M3/M4 coverage. |
| `tests/integration/test_driver_drops.py` (new), `test_connection_verdicts.py`, `test_audio_loopback.py` | Simulator unplug per driver family; pause preserved across reconnect; audio playback tone through `write()`. |

### WP-D — Industrial drivers, Sparkplug, Influx (R5.10-R5.14, R3.6; E1-E3, E6-E8, E11-E16)

| File | Change |
|------|--------|
| `app/src/IO/Drivers/S7/S7Pdu.cpp` | `decodeValue` returns invalid on empty payload; `applyResult` counts it as an item error (E1). |
| `app/src/IO/Drivers/MQTT/SparkplugSession.{h,cpp}`, `MQTTSparkplug.cpp`, `MQTT.cpp` | `reset()` keeps `m_slots`/`m_slotIndex`, clears values and birth state only; `slotsJson()`/`restoreSlots()`; driver property `sparkplugSlots` persisted like Iec104 `points` (decision); project generator writes it. |
| `app/src/MQTT/SparkplugPublisher.cpp`, `PublisherWorker.cpp`, `app/tests/tst_sparkplug_publisher.cpp` | Rebirth resets `m_seq` to 0; test pins 0 (E6); rebirth also triggered from the connection edge, not only on data (E15). |
| `app/src/IO/Drivers/OpcUaSession.cpp`, `OpcUaSecurity.cpp`, `OpcUa.cpp` | `allowNonePolicyPassword` only when the user opted in; trust decision checked before hostname/expiry (E11); `write()` returns -1. |
| `app/src/IO/Drivers/Iec104.cpp` | Slot key includes type id; live `typeId` wins over restored `kind` (E16). |
| `app/src/InfluxDB/Export.cpp` | TLS failure counted once; wall-clock offset re-sampled on reconnect (E15). |
| `app/src/MQTT/CredentialVault.cpp` and UI strings | Wording "stored obfuscated in settings" (E14, K6). |
| `app/tests/fuzz/fuzz_s7_pdu.cpp`, `fuzz_isotsap.cpp`, `fuzz_iec104_apci.cpp`, `fuzz_iec104_asdu.cpp`, `fuzz_sparkplug_payload.cpp`, `fuzz_opcua_wire.cpp` (new) | R14.3 fuzzers with seeds from the existing table tests. |
| `app/tests/tst_sparkplug_session.cpp`, `tst_s7comm_pdu.cpp`, `tst_modbus_generation.cpp`, `tst_opcua_security.cpp` (new), `tst_opcua_frame_assembler.cpp` (new), `tst_opcua_subscriptions.cpp` (new), `tst_modbus_register_groups.cpp` (new), `tst_ethernetip_worker.cpp` (new, backend seam stub) | M5 coverage. |
| `tests/integration/test_modbus_groups.py` (new), `test_sparkplug_host.py` (new, broker-marked) | Injected timeout keeps group attribution; reconnect keeps slots. |

### WP-E — Dashboard, widgets, QML (R6, R3.4; F1-F20 defects, G1-G3, G10, K10)

| File | Change |
|------|--------|
| `app/src/UI/Dashboard/DashboardIngest.{h,cpp}` (new) | `applyBlock`/`applyBlockColumn`/`applyBlockValues`/`advancePlotClock`/`feed*Sweep`/`update*Series` move out of the facade (R12.8 for Dashboard); the uniform-grid branch resolves per-column `LinePush`/GPS/3D/MultiPush consumers by index in `StreamTargets` and feeds them per sample (line pushes) or per block (GPS, 3D, latest values) (F3). |
| `app/src/UI/Dashboard.{h,cpp}` | `setPoints` and `setPlotTimeRange` share `rebuildLineSeriesPreservingState()` (sweep config + run flags saved and restored) (F4); `QString::number` per column moved behind `stringTargets` (F9); `WidgetMapBuilder` reference buckets held by index (F10); dead `m_frameBuilder` static re-resolve removed. |
| `app/src/UI/Widgets/Waterfall.{h,cpp}`, `Waterfall/WaterfallOverlay.cpp` | 256-entry `QRgb` colormap LUT per map (F5); spectrogram split into fixed-height texture tiles (64 rows), only tiles containing new rows re-uploaded per tick (F1); overlay re-rasterized on axis/marker/theme change only (F7); `historySizeChanged` emitted on real change. |
| `app/src/UI/Widgets/Terminal.{h,cpp}`, `Terminal/TerminalBuffer.cpp` | `clear()` resets selection; `copy()` clamps to buffer rows; ANSI erase overrides rebase/invalidate selection through `applyLineDrop`; colour rows trimmed in lockstep with text regardless of `m_ansiColors`; CSI parameter list capped (F2, F17); per-paint `mid()`/`toString()` churn removed (F18). |
| `app/src/UI/Widgets/ExtensionData.{h,cpp}`, `DataGrid.cpp` | Rows rebuilt on `widgetCountChanged`/structure only; per-tick value update through cached `Dataset*` refs; shared `datasetWidgetsFor()` helper (F6). |
| `app/src/UI/Widgets/FFTPlot.cpp`, `MultiPlot.cpp` | `markerValuesChanged` emitted on change; dead `m_drawOrders` removed (F15). |
| `app/src/UI/WindowManager.cpp` | Clamped rect applied instead of gesture discard (F19). |
| `app/src/Misc/ThemeManager.{h,cpp}` | `colors` becomes a `QQmlPropertyMap` (per-key reads and per-key notify; bracket syntax in QML unchanged); `getColor` reads the map directly (G2). |
| `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml` | `refresh()` assigns `lanes` only when `labelledEnd`, window or class set changed (G1). |
| `app/qml/ProjectEditor/Views/TableDelegate.qml` | Theme-change repaint connection (G3). |
| `app/qml/Widgets/Dashboard/ValueFormat.js`, `Compass.qml` | One `formatValue` per sample (G10). |
| `app/src/Licensing/MonotonicClock.cpp`, `Trial.cpp` | `now()` writes `lastSeen` at most once per minute; `daysRemaining()`/`trialEnabled()` read a cached value refreshed on the 1 Hz tick (K10). |
| `app/tests/tst_dashboard_ingest.cpp` (new, headless Dashboard fixture), `tst_terminal_selection.cpp` (new), `tst_waterfall_tiles.cpp` (new), `tst_colormap_lut.cpp` (new), `tst_extension_data_rows.cpp` (new), `tst_theme_property_map.cpp` (new) | M6 coverage. |
| `tests/integration/test_dashboard_lanes.py` (new) | Audio/replay stream into a Samples-axis plot, multiplot, GPS; sweep survives `dashboard.setPlotPoints`. |

### WP-F — Project layer and editors (R7; H1-H13)

| File | Change |
|------|--------|
| `app/src/DataModel/ProjectModel.cpp/.h` | `pointsChanged` handler calls a new mutator `setPointCount(int)` (undo scope, `setModified`, `scheduleAutoSave`), no direct write (H1); duplicate `scheduleAutoSave` connections removed; `frameDetectionChanged` -> `AppState` sync coalesced through a 0 ms timer; `groupsChanged` -> auto-workspace regen coalesced (H10). |
| `app/src/DataModel/Project/ProjectPersistence.cpp` | Reload failure keeps `m_filePath` and previous content, restores `m_modified`, posts the reason (H6); `savePluginState` gated on ProjectFile mode (H11). |
| `app/src/DataModel/Project/ProjectBulkOps.cpp` | Explicit deletion order table (datasets, tables, actions, widgets, then groups, then folders) instead of enum numeric order (H3). |
| `app/src/DataModel/Project/ProjectEntities.cpp`, `ProjectOutputWidgets.cpp`, `ProjectSources.cpp` | `updateAction` dirties and schedules autosave, `DashboardTools::configureActions` re-reads on `actionsChanged` (H4); self-assignment removed; `renumberGroupIds` shared; `sourceId` normalisation on `deleteSource` and the API path (H9); `changeDatasetOption` range check first. |
| `app/src/DataModel/Project/ProjectSources.cpp`, `Editors/FrameParserModel.cpp`, `Editors/JsCodeEditor.cpp` | `setSourceFrameParserTemplateAndParams` compound mutator under one scope (H5); empty code seeds the language's default template, never source 0 (H7). |
| `app/src/DataModel/Editors/EmbeddedCodeEditorItem.{h,cpp}` (new) | `QQuickPaintedItem` base carrying the sixteen forwarding overrides; five hosts derive from it (H12, R12.1). `OutputCodeEditor` holds `TransmitTestDialog` by `unique_ptr`, created on first use. |
| `app/src/DataModel/Editors/DatasetTransformEditor.cpp` | Lua sandbox setup shared with `LuaScriptEngine` helper. |
| `app/tests/tst_project_history.cpp` (new), `tst_project_workspace_refs.cpp` (new), `tst_project_persistence.cpp` (new), `tst_project_loader_migrations.cpp` (new, fixtures under `tests/fixtures/projects/legacy/`), `tst_project_bulk_ops.cpp` (new), `tst_frame_parser_model.cpp` (new), `tst_transmit_test_dialog.cpp` (new) | M8 coverage. |
| `tests/integration/test_project_integrity.py` (new) | File hash unchanged after points change with unsaved edits; undo depth per mutation; bulk delete; corrupt external write; empty parser for new source. |

### WP-G — API surface (R8, R3.2, R3.7; I1-I14)

| File | Change |
|------|--------|
| `app/src/API/Server/ClientReception.{h,cpp}`, `ConnectionState.h` | `consumeBytes` copies one line out, dispatches, then re-resolves `ConnectionState*` through `ReceptionHost::stateFor(socket, sessionId)` (null -> stop) (I1); HTTP request-line sniff on the first bytes closes the connection; `handshakeSeen` flag gates raw forwarding until one valid JSON message (I2); byte accounting counted once (I12). |
| `app/src/API/Server.{h,cpp}` | `authorizeDeviceWrite` never spins a modal in the receive path: consent Unset -> write refused with `CONSENT_REQUIRED`, prompt posted queued, next write passes once recorded; two loopback listeners (v4 + v6) or dual-stack `Any` when external (I10); `API/Port` setting + `--api-port`; `Server.h` section order fixed (I14). |
| `app/src/API/PathPolicy.{h,cpp}`, `CommandRegistry.{h,cpp}` | `CommandDefinition` gains `pathParams` (names + `allowMissing`); the registry enforces the policy for every registered path param; handlers keep no ad hoc checks (I3, I7); the three bypassing commands register their params. |
| `app/src/API/Server/ServerWorker.cpp` | `writeToSocket` and `broadcastEvent` under the cap; exceeding disconnects with `WRITE_BACKLOG` counted (I6). |
| `app/src/API/GRPC/GRPCServer.cpp` | Marshal via an abortable `PendingCall` (mutex + condvar + `abandon()`) that `stopServer` drains before `Shutdown` (I5); peer parsed with `QHostAddress` (I8); `WriteRawData` cap; error codes from the shared enum (I9). |
| `app/src/API/Mirror/`, `scripts/registry-verify.py` | Lint: a change to `wireUniqueId`/dataset order without a `kWireVersion` bump fails (I13). |
| `app/tests/tst_client_reception.cpp` (new, host stub mutating the table mid-call), `tst_path_policy_registry.cpp` (new), `tst_server_worker_caps.cpp` (new), `tst_grpc_pending_call.cpp` (new) | M9 coverage. |
| `app/tests/fuzz/fuzz_api_json.cpp` (new) | Reception fuzzer. |
| `tests/security/test_http_on_api_socket.py` (new), `test_path_policy_all_commands.py` (new), `test_write_backlog.py` (new), `tests/integration/test_grpc_lifecycle.py` (new) | R14.7. |

### WP-H — Assistant, extensions, CLI, licensing, startup (R9, R10, R3.8, R3.9, R4.3-R4.4, R2.3; J1-J8, K1-K6, K9, K12-K14)

| File | Change |
|------|--------|
| `app/src/AI/Conversation.cpp`, `Assistant.cpp` | Autosave timer -> `BackupManager::snapshot("assistant")` checkpoint; disk write only via `project.save` (Confirm tier) (J2); `approveToolCall`/`denyToolCall` share the tri-condition resume guard (J4); `fs.*` tools dispatched to a worker (`QtConcurrent::run` + generation id) through the existing outstanding-tool-result path, cancellable (J3). |
| `app/src/AI/Providers/LocalProvider.cpp`, `Provider.h`, settings page | `ai/localContextWindow` setting feeds `capabilities().contextWindowTokens` (J1). |
| `app/src/AI/Providers/*Reply.cpp`, `Provider.cpp` | Redirects refused (`ManualRedirectPolicy` + abort), plain `http://` only to loopback; `finishOk`/`finishWithError`/budget/readyRead/finished lifted into `Reply` (J5, R12.1); parse-error policy unified (J6). |
| `app/src/AI/KeyVault.cpp` | `redact` returns `"***"` (J8). |
| `app/qml/Dialogs/Settings*.qml` (assistant page) | Toggle copy states checkpoint semantics; credential store wording (K6). |
| `app/src/Misc/Extensions/ExtensionInstaller.{h,cpp}`, `ExtensionCatalog.cpp`, `ExtensionManager.cpp` | Catalog v2: `files: [{path, sha256, size}]`; entries without digests refused; download to `<id>.staging`, verify all, swap atomically, previous kept until success (K3, K5); `http://` repos refused; `hasUpdate` via `QVersionNumber` (K12); install folder from local package type. |
| `app/rcc/extensions/schema/catalog.json` (new) | Catalog schema; `registry-verify.py` validates the bundled catalog. |
| `app/src/Misc/CLI.cpp` | `--reset` uses default `QSettings` with the crash-recovery preserve list (K2); `--activate`/`--deactivate` wait on `LemonSqueezy::requestFinished(ok, reason)` (K1); `--api-token-file` and `SS_API_TOKEN` (K14). |
| `app/src/Licensing/LemonSqueezy.{h,cpp}`, `OfflineLicense.cpp` | `requestFinished` emitted once per request on every path; deactivation clears the cache only on `deactivated == true`; offline activation routes through `notifyEntitlementMaybeChanged` (K9). |
| `app/src/Misc/CrashTracker.cpp` | Preserve-list reset factored into a shared function. |
| `app/src/main.cpp` | Both exits run the same teardown ladder (`shutdownSession()` local function) (K4). |
| `app/src/Licensing/Trial.cpp`, `LemonSqueezy.cpp`, `Misc/ExtensionManager.cpp` | Reply-handler modals posted queued (K13). |
| `app/tests/tst_sse_event_reader.cpp`, `tst_think_tag_splitter.cpp`, `tst_reply_state_machine.cpp`, `tst_file_sandbox.cpp`, `tst_redactor.cpp`, `tst_conversation_turn.cpp`, `tst_sentinel_probe.cpp`, `tst_extension_installer.cpp`, `tst_simplecrypt.cpp`, `tst_monotonic_clock.cpp`, `tst_commercial_token.cpp`, `tst_lemonsqueezy_rules.cpp`, `tst_trial_state.cpp`, `tst_cli_exits.cpp`, `tst_session_context_lifecycle.cpp` (all new) | M10/M11 coverage. |
| `app/tests/fuzz/fuzz_sse_reader.cpp` (new) | R14.3. |
| `tests/integration/test_assistant_autosave.py` (new), `test_extension_install.py` (new), `test_cli_licensing.py` (new, stub server) | AC9, AC10. |

### WP-I — One implementation per concern (R12; the remaining D, E8, F12-F14, G4-G8, H8, J7, K7-K8, L8-L9, L11, F13, F16, B11-B13, A6-A7, A14, C4, C14)

| File | Change |
|------|--------|
| `app/src/IO/Drivers/PolledPlcWorkerBase.{h,cpp}` (new) | Shared worker ctor, `publishDirtySlots`, atomic counters, `open`/`doClose`/`onLinkLost`/`statusJson`/`generateProject` skeleton; S7 and EthernetIp derive (E8). |
| `app/src/AI/Providers/OpenAICompatibleProvider.{h,cpp}` (new) | Table-driven endpoint/model/caps; DeepSeek, Groq, Mistral, OpenRouter become table rows (J5). |
| `app/src/DataModel/ReplayPlaybackEngine.{h,cpp}` (new) | Seek window, catch-up budget, epoch, anchor and timer chain composed by the three players (B11). |
| `app/src/DataModel/ExportStructure.{h,cpp}` (new) | `setTemplateFrame`/`applyPublishedStructure`/`sanitizeTitle`/session-dir logic shared by the three exporters. |
| `app/src/DataModel/FrameBuilder.cpp` + `FrameBuilder/BlockPublisher.{h,cpp}`, `ReplayIngest.{h,cpp}` (new) | Publish fan-out and replay lanes moved out; four dataset-apply tails and two `parseProjectFrame`s collapsed; vestigial frame slot pool, `kFramePoolBudgetBytes`, `frameChanged`, `m_channelScratch` removed (A6, A7, A14); facade under 1500 lines. |
| `app/src/IO/ConnectionManager.cpp` + `ConnectionManager/DriverUiForwarders.{h,cpp}` (new) | 150 forwarding lines moved; stale comments removed; under 1500 (C4, C14). |
| `app/src/AI/Conversation.cpp` + `Conversation/ToolTurnRunner.{h,cpp}` (new) | Tool batch + approval + autosave/checkpoint logic extracted; `ToolDispatcher` given state or deleted; `Assistant` single ownership (J7); under 1500. |
| `app/src/UI/Widgets/Plot3D/Plot3DNodes.{h,cpp}` | Becomes a real class owned by `Plot3D` (F13). |
| `app/src/UI/Widgets/PlotBase.{h,cpp}` (new) | `setInterpolationMode`, `clampToVisibleX`, `buildLogXScratch`, sweep setters, `pushSweepConfig` shared by Plot/MultiPlot/FFTPlot (F14). |
| `app/src/Misc/ModuleManager.cpp`, `Misc/ContextRegistry.{h,cpp}` (new) | One table of `{name, QObject*}` registered in a loop; `hostContextNames()` generated from it; four unused globals removed (G4). |
| `app/qml/Widgets/Dashboard/InstrumentBase.qml`, `SwipePages.qml`, `app/qml/Widgets/DialogEscape.qml`, `app/qml/ProjectEditor/Views/CodeEditorMenu.qml`, `app/qml/Dialogs/DriverTagPickerDialog.qml` (new) | Shared instrument chrome/tick math/page persistence, Escape handling, editor context menu, tag-picker family (G5, G8). |
| `app/qml/Dialogs/ExtensionManager/{Grid,Detail,Repos}Page.qml`, `app/qml/AI/AssistantPanel/*.qml`, `app/qml/DatabaseExplorer/ReportOptions/*.qml` + `app/src/Sessions/ReportOptionsModel.{h,cpp}` (new) | Three monoliths split at page boundaries; tree-model JS moved into a C++ proxy model (G6). |
| `app/src/API/Server.h`, `S7.cpp`, `EthernetIp.cpp`, `DeviceManager.cpp`, `MDF4/Export.cpp`, `MDF4/Player.cpp` | Rule violations closed (wildcard disconnects, header order, trial-parity wording) (R12.3). |
| `app/src/Sessions/Export.cpp` | ConsoleOnly branches removed (B13); stale replay-marshal comments removed (B12). |
| `doc/claude/architecture/startup.md` | Cached-reference idiom + single-session assumption documented; `singleton-census.json` unchanged (K7). |

### WP-J — Documentation truth (R13)

| File | Change |
|------|--------|
| `doc/claude/architecture/dataflow.md`, `io.md`, `export.md`, `dashboard.md`, `project.md`, `startup.md`, `scripting.md`, `code-style.md` | Every drifted statement from A6, B21, C5, E10, F8, G11, H13, K8 corrected; new mechanisms from this spec described (session boundary, AsyncTcpDial, catalog v2, ScriptDryRun, QQmlPropertyMap theme). |
| `doc/claude/architecture/ai.md` (new), `doc/claude/architecture.md`, `CLAUDE.md` sub-doc table | AI assistant architecture: tier model, checkpoint semantics, meta-tool seam, provider abstraction (J9). |
| `scripts/doc-anchors.json`, `scripts/claim-baseline.json` | Ordered anchor for `instantiateCoreModules()`; new pins for the constants this spec introduces. |
| `tests/README.md` | New tiers (fuzz, sanitizer, qmllint, QML selftest), xfail policy, per-file additions. |


### WP-K — Rendering and priority cost (R15; N1-N4, added 2026-09-02, runs after WP-I in the integration tree)

| File | Change |
|------|--------|
| `app/src/UI/Widgets/Waterfall.{h,cpp}`, `Waterfall/WaterfallTiles.{h,cpp}` (WP-E's), `Waterfall/WaterfallOverlay.cpp`, `Waterfall/WaterfallRingTexture.{h,cpp}` (new) | Replace the per-tick tile re-creation with one persistent `QSGTexture` per widget owning a `QRhiTexture` (public QRhi API, Qt >= 6.6): a row ring where the newest row lands at a moving write index uploaded as a sub-rect through the frame's `QRhiResourceUpdateBatch`, and the material applies the scroll as a UV offset; rebuild only in `rebuildHistoryImage`; skip `writeRow` when no samples arrived; overlay repaint only on axis/size/marker/theme change; `ItemVisibleHasChanged` to hidden releases image and textures. Fallback path when the RHI texture cannot be created: the existing tiles. |
| `app/src/Platform/AppPlatform.{h,cpp}`, `app/src/IO/PipelineHost.{h,cpp}`, `app/src/IO/StreamWorker.cpp`, `app/src/Misc/ModuleManager.cpp`, `app/src/Benchmark/HotpathBenchmark.cpp` | `registerIngestThreadWithMmcss()` becomes per-thread (thread-local guard); the GUI call in `initializeQmlInterface` is removed; `PipelineHost` registers from inside the pipeline thread via a queued call issued after the message handler is installed (the MMCSS coexistence contract in startup.md), `StreamWorker` threads likewise on start; the benchmark keeps its own call on the thread it drives. |
| `app/src/UI/Widgets/Bar.{h,cpp}`, `Gauge.{h,cpp}`, `Meter.{h,cpp}` (and their shared value-widget base if WP-I's InstrumentBase landed), `app/qml/Widgets/Dashboard/Bar.qml`, `Gauge.qml`, `Meter.qml` | `hasData` Q_PROPERTY (false until the first sample, cleared on `resetData`); `alarmSeverity` reports -1 while `!hasData`; QML binds every `Animation.Infinite` and colour animation to `model.alarmTriggered && model.hasData`. |
| `app/src/UI/Widgets/PlotCurve.cpp`, `GpuStroke.cpp`, `PlotAreaFill.cpp` | Grow-only geometry allocation at 1.5x headroom, reuse otherwise, draw the first N through the index count; verified against `QSGGeometry::allocate` shrink semantics in 6.11 by reading the Qt source before relying on it. |
| `app/tests/tst_waterfall_ring_texture.cpp`, `tst_value_widget_hasdata.cpp`, `tst_plot_curve_geometry.cpp` (new), `tst_mmcss_registration.cpp` (new, Windows-only body) | R15 coverage; the geometry test counts `allocate` calls through a seam on a stationary point count. |
| `doc/claude/architecture/dashboard.md`, `startup.md` (WP-J) | Waterfall ring texture, `hasData`, MMCSS now on the pipeline thread. |

Tradeoff added: ring texture through public QRhi (one upload of one row per tick, no copy, needs Qt >= 6.6 which the tree already requires) versus keeping WP-E's tiles (one 1 MiB texture allocation per dirty tile per tick); ring chosen because the finding measured the allocation, not only the bytes.

## Architecture & data flow

**Session boundary (WP-A).** Today sinks close on `ConnectionManager::connectedChanged`
(GUI, synchronous) while the builder's own `onConnectedChanged` runs queued on the pipeline
thread, so the sink is gone before the builder could flush. The plan inverts the order: the
builder is the only thing that knows when the last block is out. On its (queued) connected or
paused slot it flushes `m_openBlocks` through the normal `publishBlock` (one clone per sink,
SPSC enqueue), then emits `sessionBoundary(connected, paused)`; sinks connect to that signal
(auto-queued to GUI) and `closeFile()` there. Because the SPSC enqueue happened before the
signal was posted, and each sink's `close()` drains its queue on the worker before closing the
file, every staged sample lands in the file that was open when it was produced. Replay lanes are
unaffected (they never reach `publishBlock`'s sink fan-out).

**Per-source time in sinks (WP-A).** `DataBlock` already carries `t0`, `dt` or explicit
`times`. The CSV and MDF4 workers stop calling the worker-wide `monotonicFrameNs` for irregular
blocks; they compute `t0 - referenceTimestamp + times[i]` and apply the strictly-increasing guard
per `sourceId` (a small map keyed by source, reset with the session). MDF4 already has one master
per group; CSV's `SparseRowMerger` already merges across sources by time, so it now receives true
per-source instants.

**Script deadlines (WP-B).** One JS mechanism (`JsWatchdog` + `JsWatchdogThread`) and one Lua
mechanism (`LuaDeadlineHook`, the count hook with a `QDeadlineTimer`) behind two entry points:
`ScriptDryRun` for throwaway GUI-thread evaluations (editors, API dry-runs, macro and publisher
previews) and direct ownership for long-lived engines (`StreamProcessor` gets a `JsWatchdog` per
engine, armed once per block like the frame lane arms once per dataset pass). Lua Fast mode
remains hook-free by construction, as scripting.md documents; the stream lane inherits the same
rule and the doc says so.

**Async dial (WP-C).** `IO::AsyncTcpDial` is a GUI-thread QObject that sequences resolution
(`QHostInfo::lookupHost`), an optional paced refusal probe on a throwaway socket (the spec-0050
control-script bind window), and one `connectToHost` on the driver's own long-lived socket. It
reports `finished(ok, reason)` exactly once under one deadline and supports `cancel()`. Drivers
that adopt it become async under the existing `HAL_Driver` verdict latch (`isConnecting()` true,
`reportOpenFinished` on both outcomes), which `ConnectionManager` already handles. The doctrine
"never abort-and-redial a run-loop socket" is preserved: the driver socket dials once. S7 and
EtherNet/IP keep their worker-thread blocking calls but the GUI stops waiting on them.

**Dashboard uniform-grid lane (WP-E).** `StreamTargets` (indexes only, per the dashboard.md
staleness contract) gains `linePushIndexes`, `gpsPushIndex`, `plot3DPushIndex`, `multiSampleIndexes`
resolved in the same second pass as the time-ring indexes. `applyBlockColumn` feeds line-push
consumers per sample (bounded by the column's sample count and the raw ring capacity) and the
last sample into GPS/3D/latest-value targets; the irregular path is unchanged.

**Theme map (WP-E).** `ThemeManager::colors` becomes a `QQmlPropertyMap` built from the same
JSON. QML's `Cpp_ThemeManager.colors["text"]` resolves to a dynamic property read; a theme
switch calls `insert()` per key, so each binding re-evaluates on its own key's `valueChanged`
rather than on a whole-map conversion.

**API reception (WP-G).** The receive loop never holds a reference across a dispatch: it pops one
line into a local, dispatches, then asks the host for the state again by `(socket, sessionId)`.
Consent prompts leave the receive path entirely: an unset consent refuses the write with a coded
error and posts the prompt queued; the client retries after the user answers. Path policy moves
from four hard-coded names to a per-command declaration in `CommandDefinition`, enforced once in
`CommandRegistry::execute`.

**Extension install (WP-H).** Catalog v2 carries `sha256` and `size` per file. The installer
downloads every file into `<id>.staging`, verifies each digest as it lands, and only when the set
is complete renames `<id>` to `<id>.previous`, `<id>.staging` to `<id>`, then deletes `.previous`.
Any failure deletes `.staging` and leaves the installed version untouched. `installed.json` is
written last.

**CI (WP0).** `test` consumes build artifacts, so it runs before `upload`; `upload` needs
`[test, lint]` and is guarded to the default branch or a tag with a per-ref concurrency group.
A `sanitize` job builds `build/unit-ci` with ASan+UBSan (and a TSan leg) and runs ctest, the fuzz
targets' corpus replay, and the benchmark at `--min-fps 1`.

## Hotpath & threading impact

- **Touches the hotpath?** Yes: `FrameBuilder` (flush on session edge, cached `m_playerOpen`
  reads, deferred project snapshot, staging extraction), `StreamWorker` (watchdog arm per block),
  `FrameConsumer` (coalesced flush post), `Dashboard` ingest (uniform-grid consumers, ingest
  sub-object), `MQTT::Publisher::hotpathTxRawFrame` (atomic reads). Rules preserved: no
  allocation on the publish path (the boundary signal is per session edge; the coalesced flush
  removes an allocation; the new consumer indexes are resolved at configure time; the watchdog
  arm is an atomic store); SPSC and single producer unchanged (sinks still receive one
  `clone_block_trimmed` from the pipeline thread); in-pipeline hops stay Direct (the new
  `sessionBoundary` is pipeline -> GUI at edge rate, auto-queued). `--benchmark-hotpath` runs on
  every WP-A, WP-B, WP-E branch and in the sanitizer job; the ingest-cost readout
  (`HOTPATH_DASHBOARD_INGEST_COST`) is compared before/after WP-E.
- **New cross-thread signal/slot?** `FrameBuilder::sessionBoundary` (pipeline -> GUI, queued,
  edge rate); `StreamWorker` watchdog timeout notification (worker -> GUI, queued, once per
  timeout); `Sessions::ExportWorker::writeErrorChanged` (worker -> GUI, queued, once per failure);
  `AsyncTcpDial::finished` (same thread, GUI); `PendingCall` in gRPC (worker <-> GUI, condvar,
  abortable). No new per-frame emission anywhere.
- **New input to a cached hotpath flag?** No new inputs. Two existing reads that bypassed
  `m_playerOpen` are routed to it; `MQTT::Publisher` gains its own atomics mirrored by the GUI
  setters (write once per settings change). `m_anyAsyncSink` is untouched: sinks still close on a
  session edge, and `refreshAnyAsyncSink` already re-evaluates on their `enabledChanged`.
- **Timestamp ownership.** Unchanged at the driver boundary. WP-A removes two downstream
  re-stamps (CSV, MDF4 irregular blocks) and one fabricated ramp (Sessions raw head);
  `monotonicFrameNs` survives only as the per-source same-ns tie-breaker. The Audio playback ring
  and the CAN batching do not touch capture stamps.

## Data model & persistence

- **Project JSON:** `Keys::SparkplugSlots` (driver property `sparkplugSlots`, array of
  `{group, node, device, metric, index}`) written by the MQTT driver's `driverProperties()` and
  read by `setDriverProperty`; absent = derive on first birth (old projects keep working, slots
  become stable after the first save). `Keys::PointCount` is unchanged but now written only
  through the mutator path. No schema version bump: additive keys under an existing driver
  property object.
- **Extension catalog v2:** `schemaVersion: 2`, `files[]` entries `{path, sha256, size}`. v1
  catalogs are refused with a Problem Center finding naming the repo. `installed.json` gains
  `sha256` per file for repair checks.
- **MDF4:** master channel `cn_sync_type = Time`; reader accepts 0 from files whose header
  author is Serial Studio. Text channels declared UTF-8 (`StringUtf8`) where mdflib supports it.
- **Sessions DB:** no schema change. `sessions.getStatus` gains `rawOverruns`, `writeFailed`,
  `droppedBlocks`.
- **Settings:** `API/Port` (int, default 7777), `ai/localContextWindow` (int, default 8192),
  `IO_Serial_SelectedDevice` cleared on placeholder, `ExtensionRepositories` entries validated
  (https or file only), `licensing/lastSeen` written at most once per minute.
- **Modbus wire:** placeholder frame `[slave, fc, 0]` on a reply error; generated parsers from
  this version skip it; older generated parsers see a zero-length payload and decode nothing for
  that group (no worse than today).

## API / SDK surface

- New error codes: `CONSENT_REQUIRED`, `WRITE_BACKLOG`, `SESSION_LIVE`, `PATH_NOT_ALLOWED`
  (replacing the per-handler `InvalidParam` text), `SCRIPT_TIMEOUT` (dry-runs), `CATALOG_UNSIGNED`.
- `CommandDefinition.pathParams` (registry metadata, exported into `api-schema.json` as
  `x-pathParams` so `generate-property-registry.py --check-snapshot` pins it).
- New commands: none. Changed: `sessions.getStatus` fields above; `assistant.checkpoint` semantics
  documented (the assistant's autosave now creates one).
- CLI: `--api-port <n>`, `--api-token-file <path>`, `SS_API_TOKEN`; `--activate`/`--deactivate`
  exit codes become meaningful; `--selftest qml`.
- gRPC: unchanged proto; `WriteRawData` enforces `kMaxApiRawBytes`.

## QML / UI

- Preferences: API port field beside the enable toggle; assistant auto-approve toggle copy
  ("edits are checkpointed; the project file changes only when you save"); local model context
  window field; credential-store wording.
- USB Advanced transfer consent asked from the setup pane button, recorded in settings; the
  project/API path never prompts.
- Theme map change is transparent to QML syntax; the three monolith splits and the shared
  components (WP-I) keep every `objectName`/`Settings` category so persisted UI state survives.
- `qmllint` baseline checked in; new warnings fail `lint`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Sink close ordering | (a) sinks close on `connectedChanged` and the builder flushes first via a GUI-side blocking marshal; (b) sinks close on a builder-emitted boundary after the flush; (c) a close command enqueued through the consumer queue | (b): no GUI-to-pipeline blocking wait (forbidden), ordering by construction, one signal. |
| Per-source time in CSV | (a) per-source monotonic baselines in the worker; (b) `SparseRowMerger` re-derives from block metadata; (c) drop the tie-break entirely | (a): smallest change, keeps the same-ns tie-break the merger needs, per source. |
| Stream JS watchdog | (a) `JsWatchdog` per worker engine armed per block; (b) run stream JS on the pipeline thread; (c) forbid JS on stream sources | (a): matches the frame lane, no lane change, Fast-mode Lua parity documented. |
| TCP dial | (a) keep sync doctrine, move resolve to a thread and block on it; (b) async single-attempt dial on the driver socket with a resolver step (`AsyncTcpDial`); (c) reinstate the retry stack | (b): WS/HTTP already work this way, the latch handles it, control-script bind window preserved by the paced probe; (c) is the banned design. |
| S7/EIP dial | (a) keep BlockingQueued GUI wait; (b) async verdict from the worker | (b): the worker already dials; only the wait moves. |
| Modbus group identity | (a) new header byte with the group index (breaks old generated parsers); (b) zero-length placeholder frame on error (old parsers unaffected) | (b): compatibility; the placeholder costs one tiny frame per failed poll. |
| Sparkplug slots | persist in project vs derive at runtime | persist (maintainer decision); derive stays a follow-up if renames prove common. |
| Waterfall upload | (a) sub-rect texture upload via RHI (private-ish API); (b) fixed-height texture tiles, re-upload only dirty tiles; (c) halve resolution | (b): public API only, bounded upload per tick, no visual change. |
| Theme colours | (a) `QQmlPropertyMap`; (b) generated per-key `Q_PROPERTY`s; (c) leave as is | (a): zero QML churn, per-key notify; (b) needs a generator for 118 keys. |
| API loopback | reject HTTP-shaped pre-auth + raw gated on handshake vs token for all | reject + gate (maintainer decision). |
| Consent prompts in API | (a) queued prompt with refusal code; (b) keep modal but snapshot state | (a): removes the nested loop from the receive path entirely; (b) leaves the same hazard for any future modal. |
| gRPC marshal | (a) abortable `PendingCall`; (b) timeout on BlockingQueued (not supported by Qt) ; (c) run gRPC handlers on the GUI thread via queued + future | (a): mirrors `PipelineHost::MarshalCall::abandon()`, proven pattern in this tree. |
| Extension integrity | digests + atomic install vs signing | digests now (maintainer decision); signing later. |
| Assistant autosave | checkpoint vs disk | checkpoint (maintainer decision). |
| MachineID spawns | (a) background thread on every start; (b) persisted id, spawn only on first run | (b): no start-up latency and no EDR noise for the common case; licensing reads the same id it always did. |
| Benchmark retry | (a) same-runner second attempt after 30 s; (b) separate retry job on a fresh runner (`if: failure()` + artifact download); (c) none | **(b) as decided**, implemented as one `benchmark-retry-<os>` job per platform that downloads the built binary artifact; `upload` gates on `build || retry` via `if: always()` + result check. Costlier YAML; matches the decision. |
| Fortify level | 3 vs 2 | 3 on GCC >= 12 / glibc >= 2.34, else 2; asserted from `compile_commands.json` in CI (decision). |
| ctest on macOS/Windows | every push vs tags only | every push (decision); built in the existing jobs' `build/unit-ci`. |
| Duplication lint | new rule with ratchet vs review only | rule + ratchet (decision), advisory. |
| Facade size (R12.8) | (a) split the four named facades; (b) all nine over-cap TUs | (a): FrameBuilder, Dashboard, ConnectionManager, Conversation; the other five stay under the no-growth ratchet, listed as follow-up. |
| Players' shared logic | inheritance vs composed engine | composed `ReplayPlaybackEngine`: the three players differ in Q_PROPERTY surface and storage; composition keeps one class per file pair. |

## Risks & mitigations

- **Flush-on-edge changes recording row counts.** Every existing export test that counted rows
  across a disconnect could shift by one tick. Mitigation: `test_recording_fidelity.py` asserts
  equality with frames sent; existing tests updated in the same branch.
- **Async TCP changes the control-script `io.connect()` contract** ("open() returns with the
  link established"). Mitigation: `io.connect` waits on the verdict for sync-style callers (the
  fan-out already treats an in-flight dial as connected for toggle purposes); pytest
  `test_connection_verdicts.py` extended for the scripted case.
- **`QQmlPropertyMap` semantics.** A key missing from a theme returns `undefined` instead of an
  empty variant. Mitigation: map is seeded with every key from `default.json`; `tst_theme_property_map`
  asserts the key set; qmllint baseline catches typos.
- **Dashboard ingest extraction on the hotpath.** Mitigation: pure move first (benchmark
  equal), lane change second; `HOTPATH_DASHBOARD_INGEST_COST` compared per commit.
- **Catalog v2 breaks existing third-party repos.** Mitigation: Problem Center finding names the
  repo and the missing field; the bundled repo migrates in the same release.
- **CI reorder makes `upload` wait on the full pytest matrix** (~1 h). Accepted per decision;
  tagged releases already did.
- **Silent-breakage classes exposed** (common-mistakes.md): cached-flag inputs (none added, two
  reads fixed); `QMap::operator[]` inserts in new maps (per-source baselines use `find`/`try_emplace`);
  macOS file-dialog reentrancy untouched; `%n` in new translated strings avoided; setter guard
  returns in every new setter; `QThread` started-signal idiom for the Audio/USB contexts follows
  `HID::cleanupDevice`.
- **Agent execution.** Each package brief names its 3-5 binding invariants and owned files;
  a package that needs a foreign file sends a patch to the coordinator; the coordinator runs
  `code-verify --check`, `qt-cpp-review` and `sanitize-commit.py` per branch before merge.

## Test & verification plan

Per finding, the pinning test (ctest unless marked py = pytest, fz = fuzz target):

| Findings | Test |
|----------|------|
| A1 | `tst_stream_worker` JS runaway case; py `test_script_deadlines` |
| A2, B14 | `tst_frame_builder_staging` disconnect/pause tail; py `test_recording_fidelity` |
| A3 | `tst_frame_builder_staging` parked snapshot deferral |
| A4, A5 | `tst_frame_builder_staging` player-open cache; `tst_mqtt_publisher_hotflags` (new); sanitizer TSan leg |
| A11, A12 | `tst_frame_builder_staging` synthetic republish scope; `tst_stream_worker` out-of-range channel |
| B1 | `tst_csv_export_times`, `tst_mdf4_export_times` two-source staircase |
| B2, B3, B18 | `tst_sessions_export_worker` raw-lane throughput, commit failure, raw head stamps |
| B4 | `tst_sessions_database_manager` (new) live-session guard; py `test_historian_live_guard` (new) |
| B5 | `tst_sessions_player_epoch`; py `test_replay_timeline` extended to Historian and MDF4 |
| B6, B7 | `tst_mdf4_loader_memory`, `tst_sessions_loader_index` (new) with an address-space cap |
| B8 | `tst_frame_consumer` coalesced post count |
| B9, B10, B15-B17 | `tst_csv_player_catchup` (new), `tst_mdf4_writer_conformance` (new, reads back with mdflib), `tst_sessions_readonly_open` (new) |
| C1, C3, C11, C12 | `tst_xymodem` (XFAIL -> pass), `tst_file_transmission` |
| C2, C8, C13 | `tst_connection_verdicts` (FakeDriver) pause preservation, QuickPlot swap, cancel no `sessionClosed` |
| D1, D8, D19 | `tst_serial_can_backend` unplug, malformed id, buffer bound |
| D2, D3, D7, D20 | `tst_playback_ring`; py `test_audio_loopback` no-output capture + tone |
| D4, D5, D9 | `tst_uart_policy` (new), `tst_usb_transfer_consent` (new); py `test_driver_api_comprehensive` extended |
| D6, E5, E9 | `tst_async_tcp_dial` (blackholed resolver stub); py `test_connection_verdicts` GUI-responsive ping during dial |
| E1 | `tst_s7comm_pdu` zero-length item; fz `fuzz_s7_pdu` |
| E2 | `tst_sparkplug_session` reconnect keeps slots; py `test_sparkplug_host` |
| E3, E12 | `tst_modbus_generation` placeholder skip; py `test_modbus_groups` |
| E4 | `tst_mqtt_driver_verdict` (new, stub broker) |
| E6 | `tst_sparkplug_publisher` rebirth seq 0 |
| E7 | `tst_influx_lineprotocol` backslash cases |
| E10, E11, E16 | `tst_opcua_endpoint_selection`, `tst_opcua_security`, `tst_iec104_slots` (new) |
| F1, F5, F7 | `tst_waterfall_tiles`, `tst_colormap_lut`; benchmark `HOTPATH_DASHBOARD_INGEST_COST` |
| F2, F17 | `tst_terminal_selection`, `tst_terminal_buffer` |
| F3, F4 | `tst_dashboard_ingest`; py `test_dashboard_lanes` |
| F6 | `tst_extension_data_rows` |
| G1, G2, G3 | `tst_theme_property_map`; QML selftest; maintainer observation for G3 |
| H1-H7, H9 | `tst_project_persistence`, `tst_project_bulk_ops`, `tst_project_history`, `tst_frame_parser_model`; py `test_project_integrity` |
| I1 | `tst_client_reception` host stub mutating the table; py stress xfail flipped |
| I2 | py `test_http_on_api_socket` |
| I3, I7 | `tst_path_policy_registry`; py `test_path_policy_all_commands` |
| I4 | `tst_script_dryrun`; py `test_script_deadlines` |
| I5 | `tst_grpc_pending_call`; py `test_grpc_lifecycle` |
| I6, I8, I10, I12 | `tst_server_worker_caps`; py `test_write_backlog`, `test_api_ipv6` (new) |
| J1 | `tst_conversation_turn` local window budget |
| J2 | py `test_assistant_autosave` (hash unchanged until save) |
| J3, J4 | `tst_conversation_turn` (FakeProvider) async fs tool + approve-during-stream |
| J5, J6, J8 | `tst_reply_state_machine` (FakeTransport), `tst_redactor` |
| K1, K2, K14 | `tst_cli_exits`; py `test_cli_licensing` |
| K3, K5, K12 | `tst_extension_installer` (FakeTransport: partial, bad digest, http repo, downgrade); py `test_extension_install` |
| K4 | `tst_session_context_lifecycle` (headless bootstrap + early-exit ladder) |
| K9, K10, K11 | `tst_lemonsqueezy_rules`, `tst_monotonic_clock`, `tst_machine_id` (new) |
| L1-L7, L12, L14 | CI itself: FORTIFY assert step, branch guard (`act`-style dry run in `scripts/tests/test_ci_workflow.py` parsing the YAML for pins/guards/permissions), ctest on three OSes, sanitizer job green, benchmark retry job, pytest lock installed, xfail policy check in `test_ci_workflow.py` |
| L5 | `scripts/tests/test_code_verify.py` |
| M7 | `qmllint` in `lint`; `--selftest qml` in the Linux build job (both builds) |

- **Unit (agents can run):** `tests/scripts/` static tests, `scripts/tests/`, and the Node-free
  ctest suites against an existing build dir when the maintainer has built one.
- **Integration / security / perf (maintainer runs):** the py files above; `test_recording_fidelity`
  and `test_driver_drops` need the simulator only; `test_sparkplug_host` needs the broker marker;
  `test_audio_loopback` needs the audio marker.
- **Hotpath:** `--benchmark-hotpath` after WP-A, WP-B, WP-E and WP-I merges; the sanitizer job
  runs it at `--min-fps 1` on every push.
- **Static:** `code-verify.py --check` (new `--dup-census`, `--tu-census`, `--singleton-census`),
  `claim-verify.py` (ordered anchor), `registry-verify.py` (catalog schema, mirror version lint),
  `qt-cpp-review` per branch, `sanitize-commit.py` before every commit.

## Left Open (WP-J, 2026-09-02)

Written at the end of the spec, against the merged tree, so the ledger closes honestly rather
than by omission. Every entry below is either "done differently" or "not done, and here is why".

### Named ctest suites that were not written, and where the coverage went instead

The recurring reason is one wall, documented twice already in `app/tests/CMakeLists.txt`
(`tst_proto_importer`, `tst_dbc_importer`): the unit tier links per-TU source sets, and a class
whose constructor reaches `AppState`, `ProjectModel`, `ConnectionManager` or `UI::Dashboard`
drags the whole application into the link. Testability is a *design* input here, which is why
`BlockStager` got a host interface, `Sessions::BlockFingerprint`, `UartPolicy`, `ModbusRtuCodec`,
`ExportStructure`, `PolledPlcWorkerBase` and `Bands::reportedSeverity` exist as separable units at
all.

| Named suite | Package | Status |
|-------------|---------|--------|
| `tst_mdf4_export_times` | WPA-T13 | Not written: both exporters now share `FrameConsumerWorkerBase::monotonicSourceNs`, so it would assert the same function twice. `tst_csv_export_times` covers it. |
| `tst_mdf4_writer_conformance` | WPA-T14 | Blocked: needs a checked-in binary `.mf4` fixture that only a build can produce. The writer change (`Sync(Time)`, no epoch write, UTF-8 text) and the reader acceptance (`isTimeMaster`) are in. |
| `tst_mdf4_loader_memory` | WPA-T15 | Blocked: needs a generated ten-minute 48 kHz `.mf4`, i.e. mdflib at test-authoring time. **A round-trip suite IS feasible** and is the recommended follow-up: `MDF4/PlayerLoaderWorker.cpp` links nothing but `Qt6::Core` and the `mdf` target. |
| `tst_sessions_database_manager` | WPA-T17 | Blocked (application link). `tests/integration/test_historian_live_guard.py` covers the live-session guard. |
| `tst_sessions_player_epoch`, `tst_csv_player_catchup` | WPA-T18 | Superseded: the epoch and the catch-up gate moved into `DataModel::ReplayPlaybackEngine` and are covered by `tst_replay_playback_engine`. |
| `tst_mqtt_publisher_hotflags` | WPA-T19 | Blocked (broker stack + licensing + vault in the link). The Influx half of that task is covered by `tst_influx_lineprotocol`. |
| `tst_usb_transfer_consent`, `tst_file_transmission`, `tst_stream_config_builder`, `tst_mqtt_driver_verdict` | WP-C | Blocked (application link). Covered instead by `tst_xymodem`'s typed-error case, `tst_connection_verdicts`, `tst_uart_policy`, `tst_serial_can_backend`, and the USB/MQTT/Modbus pytest tier. |
| CAN batching case in `tst_can_reassembly` | WPC-T10 | Not written: the batching lives in `CANBus.cpp`, which the unit tier cannot link; left to the integration tier. |
| `tst_project_persistence`, `tst_project_loader_migrations`, `tst_project_workspace_refs`, `tst_frame_parser_model`, `tst_transmit_test_dialog` | WP-F | Not written or not registered (application link; `ProjectWorkspaceRefs` drags `SerialStudio.cpp`'s icon registry and theme manager). Covered by the five legacy `.ssproj` fixtures plus `tests/integration/test_project_integrity.py`. Reasons are recorded in `app/tests/CMakeLists.txt`, where a reader looks for them. |
| `tst_extension_data_rows` + the shared `datasetWidgets()` helper | WPE-T7 / F14 | Not done: `ExtensionRowsModel` still lives inside `ExtensionData.cpp` beside a `QQuickItem` bound to `UI::Dashboard`. Landing the test needs the file-pair split first. The F6 defect itself is fixed. |
| `tst_lemonsqueezy_rules`, `tst_session_context_lifecycle`, `tst_trial_state` | WP-H | Not written (application link; `Trial`'s constructor also issues a live network request). Pinned instead at source level by `tests/scripts/test_cpp_regressions.py` and end to end by `test_cli_licensing.py`. |
| `tst_ethernetip_worker` | WPD-T7 | Written, but against `PolledPlcWorkerBase` in WP-I: `kEipBackend` is a label, not an injectable seam. |
| `test_replay_timeline.py` | WP-I | Not written (needs a running app). |
| The hidden-waterfall release path | WPK-T2 | No ctest: it needs a live `QQuickItem` in a window. `tst_waterfall_ring_texture` covers everything below the item. |

### Work not done at all

- **WPC-T12 (second half), USB `write()` off the GUI thread.** The consent half is done and
  tested. Moving the 1 s synchronous bulk-OUT transfer off the GUI thread needs an async
  `libusb_fill_bulk_transfer` submit/callback path inside `UsbTransferPump`, with its own transfer
  allocation, completion signal and cancellation. Written blind it can only be validated by
  hardware. Model to follow: `submitControlTransfer`.
- **WPC-T14, shared native contexts.** Done: hidapi `hid_init`/`hid_exit` refcounted, HID `open()`
  closes first. Not done: shared libusb/miniaudio contexts (`UsbContext`, `AudioContext`) and the
  `setPersistent(false)` rollout across ~70 setters in seven drivers. Both are mechanical and both
  silently break device enumeration for every user if wrong; neither is verifiable without
  hardware.
- **WPI-T5, the FrameBuilder split, is partial.** `BlockPublisher`, `ReplayIngest` and the
  `parseProjectFrame` collapse took 286 lines; `FrameBuilder.cpp` is **2983** lines, still the
  worst TU in the tree. The two clusters left are the dataset-apply cluster (683 lines, and
  `applyDatasetValueSpan` is `SS_HOT` on the 1.024 MHz Native tier) and the transform dispatch
  (318 lines holding the `m_compileGuard` re-entry contract). Neither was authorised, and neither
  should be extracted blind: sharing a tail in the per-cell hot loop costs a call unless
  force-inlined, and the only check is `--benchmark-hotpath`.
- **`ConnectionManager.cpp` is 1589 lines** and the remaining 89 over the cap cannot come out:
  `QObject::sender()`, `Q_EMIT`, `connect(..., this, ...)` and the facade's own Q_INVOKABLE
  per-bus QML accessors all require the QObject, and moving the accessors to a second TU would
  create the headerless continuation "one class = one .h/.cpp pair" bans.
- **`Terminal.cpp` is 1988 lines** (grew ~50 in WP-E). Not in the R12.8 four; a follow-up.
- **Four Lua hook implementations still hand-roll `lua_sethook`** (`LuaScriptEngine`,
  `MacroRunner`, `MQTT/PublisherScript`, `IO::StreamWorker`) instead of `LuaDeadlineHook`. The
  mechanical part is easy, but `bind()` calls `lua_newuserdata` and must run inside each caller's
  protected bootstrap, and adopting it changes the timeout error string that other suites pin.
  Four engines on the per-frame parse path with no compiler available: deferred.
- **Twelve files still carry a wildcard `disconnect(x, nullptr, y, nullptr)`** (15 sites, down
  from 19 in 16 files), baselined per file by the new `qt-disconnect-wildcard` rule.
  `OpcUaSubscriptions::unbindSession` is one of them. R12.3 wants the baseline emptied.
- **`ScriptDryRun::runJsDryRun()` has no caller.** It is a public static on the class (not the
  free function `plan.md` describes), and every real dry-run path constructs a session instead.
  Either wire it or drop it.
- **`FramePoolPolicy::applyMemoryBudget()` has no production caller** either, now that
  `kFramePoolBudgetBytes` and `refreshFramePoolBudget` are gone (A6). Only `tst_frame_pool_policy`
  exercises it.
- **`SessionContext::sealed()` has no caller** outside its own definition (K8).
- **The qmllint baseline ships unseeded** (`"seeded": false`), so the gate reports and passes.

### Decisions taken during implementation that the plan did not anticipate

- **`Qt6::GuiPrivate` is now a link dependency of the app target** — the first private-Qt
  dependency in the tree. `<rhi/qrhi.h>` is under Qt's private include path and
  `WaterfallRingTexture` needs it. The plan called QRhi "public API, Qt >= 6.6"; it is
  semi-public, source-compatible within a Qt **minor series only**, so a Qt minor upgrade must
  re-check `WaterfallRingTexture.cpp`. What limits the blast radius is that the 64-row tile path
  remains a complete fallback, selected whenever `WaterfallRingTexture::supported()` is false
  (no QRhi, unsupported format, oversized texture, big-endian target). Reverting the private
  dependency costs the scanline upload and keeps the tile path's own improvement.
- **A hidden waterfall now loses its history.** `itemChange(ItemVisibleHasChanged, false)` frees
  the history image and requests a GPU teardown; becoming visible again rebuilds it at the floor
  colour and refills from live data. That is what R15.1's "a hidden widget releases its image and
  textures" costs, and it is visible when switching workspace tabs. The alternative (keep the
  image, release only the textures) is a one-line change if the maintainer prefers it.
- **An unchanged spectrum stops the waterfall scrolling.** The idle gate compares the smoothed
  row to the previous tick's; identical means no row is written and no frame is scheduled.
  Intended for a silent or disconnected source; a *perfectly* constant live signal also stops,
  which is indistinguishable once the history is uniform but visible during the first fill.
  Campbell mode is exempt.
- **Geometry uploads roughly 1.5x more bytes per frame** in exchange for zero per-frame
  malloc/free. N4 measured the allocation, not the bandwidth; the padded tail is degenerate
  triangles the rasterizer discards. Real trade, named here rather than buried.
- **The two `--benchmark-hotpath` consumer-path floors are gates**, not readouts: nine gated rows,
  not seven. `lua+exporters` and `lua+dashboard` are gated at 0.5x so a consumer-path collapse
  cannot ship silently.
- **`--dup-census` catches no C++ pair** at the agreed threshold. The census normalizes comments
  and whitespace but deliberately not identifiers, so it has zero false positives; S7 and
  EtherNet/IP shared 24 windows, below the 40 threshold, because the clones differ by identifier.
  WPI-T1 removed that pair by construction anyway. All 20 remaining pairs are QML.
