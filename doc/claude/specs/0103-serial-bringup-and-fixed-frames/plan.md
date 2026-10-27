---
spec: 0103-serial-bringup-and-fixed-frames
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-26
---

# Plan 0103 — Serial Bring-Up Sequences, Line Control and Fixed-Length Frames

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The UART driver becomes an asynchronous opener when its source carries a Connection Sequence:
`open()` opens the port as today, then hands the open `QSerialPort` to a step runner built on
the existing `Async::` task tree (one `SequentialGroup` of step tasks under a `TimeoutTask`
budget, timers from an injected `AsyncClock`), reports `isConnecting()` and `isOpen() == false`
while it runs, swallows every byte into the runner instead of `dataReceived`, and calls
`reportOpenFinished` exactly once at the end, which is the edge the pipeline, the sinks, the
auto-execute actions and the Control Loop already key on. The sequence is a label-less driver
property (the Modbus `registerGroups` precedent), so it travels in the source's `connection`
block, is restored by `applyConnectionSettings`, and is edited by one QML dialog hosted from
both the Setup Panel and the Project Editor. Hold RTS, BREAK, a live baud change and
modem-line readback are plain driver methods exposed as `io.uart.*` commands with the capability,
sealed and assistant-tier classifications every command carries. Fixed Length is a fifth
`FrameDetection` value with its own `FrameReader` extractor that reuses the pooled-slot,
checksum and timestamp machinery of the three delimiter readers, plus one wrap-aware masked-byte
scan on `CircularBuffer`; the mode's five settings ride the `Source` struct through the
existing `FrameConfig` and `PipelineSpec` builders.

## Affected subsystems & files

### A. Sequence model (Core, every build)

| File | Change |
|------|--------|
| `core/Core/IO/ConnectionSequence.{h,cpp}` | **New.** `IO::SequenceStep` (kind enum: Break, Wait, SetLine, Send, Expect, Discard, SetBaud, SetFlowControl; fields `ms`, `line`, `high`, `data`, `hex`, `eol`, `checksum`, `patterns`, `timeoutMs`, `optional`, `rate`, `flowMode`), `IO::ConnectionSequence` (`steps`, `budgetMs` default 10000), `toJson()` / `fromJson()` (accepts `QJsonObject` and the `QVariantMap` shape `applyConnectionSettings` hands back), `validate()` returning the first error with its step index, `ExpectPattern` parsing (`"5D ?? 7F"`, `??` = any byte) and `matchIn(buffer)` returning the end offset of the first match. Qt-Core only, no I/O. |
| `core/Core/CMakeLists.txt` | Add the pair to the Core source list beside `IO/FrameConfigBuilder.cpp`. |

### B. UART driver (Devices)

| File | Change |
|------|--------|
| `core/Devices/IO/Drivers/UART/SequencePort.h` | **New.** `IO::Drivers::ISequencePort`: `write`, `clearInput`, `setBreak`, `setBaud`, `setLine`, `setFlowControl`, `lineStates`; the seam the runner drives and a test fakes. |
| `core/Devices/IO/Drivers/UART/SequenceRunner.{h,cpp}` | **New.** Builds the task tree from an `IO::ConnectionSequence` over an `ISequencePort` and an `Async::AsyncClock`; `feed(bytes)` from the owner; `start()` / `cancel()`; `finished(bool ok, QString reason, int stepIndex)`; `stepStarted(int index, QString text)` and `bytesSent(QByteArray)` for the console. Step tasks: `BreakTask`, `DelayTask`, `ExpectTask` (bounded 4 KiB receive buffer, consumes through the match), the rest as `Async::InvokeTask`. |
| `core/Devices/IO/Drivers/UART.{h,cpp}` | `m_rtsEnabled` + `rtsEnabled` Q_PROPERTY / setter (`UartDriver/rts`), applied after DTR in `open()` only when flow control is not RTS/CTS; `sendBreak(int ms)`, `setLiveBaudRate(qint32)` (port rate, read buffer and the Windows native buffer; never `m_baudRate` or settings), `lineStates()`; `m_sequence`, `m_runner`, `m_sequenceRunning`; `isConnecting()` override; `isOpen()` returns false while a connect-time sequence runs; private `portOpen()` for the internal checks (`handleError`, `onReadyRead`, `close`); `onReadyRead` routes to the runner while it runs and drops bytes while a BREAK is held; `write()` returns 0 while a sequence runs; `close()` cancels the runner first (no verdict); `runSequence()` for the on-demand re-run (frames gated, connected state kept); the `connectionSequence` label-less property and setter; the port adapter implementing `ISequencePort` as a private class in `UART.cpp`. |
| `core/Devices/IO/Drivers/UART/UartPolicy.h` | Two pure predicates beside the existing ones: `rtsSettable(flowControl)` and `sequenceVerdict(stepOutcome)` text shaping, so the ctest tier pins them without a port. |
| `core/Devices/CMakeLists.txt` | Add `IO/Drivers/UART/SequenceRunner.cpp` and the two headers to the explicit lists. |
| `core/Core/IO/HAL_Driver.h` | Protected `publishSentData(const QByteArray&)` emitting the already-declared, never-consumed `dataSent`. |
| `core/Devices/IO/DeviceManager.{h,cpp}` | Connect `dataSent` to a new `rawDataSent(deviceId, data)` beside `rawDataReceived`. |
| `core/Devices/IO/ConnectionManager.cpp`, `ConnectionManager/DeviceIoRouter.{h,cpp}` | Wire `rawDataSent` (Direct, GUI) to a router method that fans out to `m_taps[i]->onSentBytes`, so driver-internal sends echo in the console like routed writes. |
| `core/Core/Bus/Messages.h` | New topic `Core::Bus::LinkSequenceNote{int sourceId; QString text;}` for the step lines. |

### C. API surface (Api)

| File | Change |
|------|--------|
| `core/Api/API/Handlers/UARTHandler.{h,cpp}` | New commands: `io.uart.setRtsEnabled` (UI driver, like DTR), `io.uart.sendBreak` (`durationMs`, optional `sourceId`), `io.uart.setLiveBaudRate` (`baudRate`, optional `sourceId`), `io.uart.getLineStates` (optional `sourceId`), `io.uart.runSequence` (optional `sourceId`), `io.uart.getSequence` / `io.uart.setSequence` (UI driver). Live-targeting resolves `connectionManager.driver(sourceId)` with a checked cast and refuses a non-UART or closed device with `ExecutionError`. `getConfig` gains `rtsEnabled`, `liveBaudRate`, `sequenceStepCount`. |
| `core/Api/API/CommandCapabilities.cpp` | `sendBreak`, `setLiveBaudRate`, `runSequence` in `kCapable` as `WritesDevice`; `setSequence` as `{NoCapability, WritesDevice}` with an `argumentsAddCapabilities` branch (a sequence containing a Send step writes on connect, the `project.action.update` precedent); `setRtsEnabled`, `getLineStates`, `getSequence` in `kPlain`, sorted. |
| `core/Api/API/SealedPolicy.cpp` | Live-control commands in `kAllowed` (sorted); `getSequence` / `setSequence` in `kDenied` (project content). |
| `core/Api/API/Handlers/SourceHandler.cpp`, `ProjectParserCommands.cpp`, `ProjectDryRunCommands.cpp` | Frame-detection descriptions name the fifth mode; the two `<= 3` range checks become `<= FixedLength`; the new frame settings are accepted on `source.update` and `project.frameParser.*`, validated (length ≥ 1, mask/value 0..255), echoed by the getters, and copied into the dry-run `PipelineSpec`. |
| `app/rcc/ai/command_safety.json` | `getLineStates`, `getSequence` in `safe`; `sendBreak`, `setLiveBaudRate`, `setRtsEnabled`, `runSequence`, `setSequence` in `deviceGated`. |
| `app/rcc/api/api-schema.json` | Regenerated by the maintainer's build with `--dump-api-schema` (the registry ctests sweep the snapshot, not the live registry). |
| `app/rcc/api/SerialStudio.js`, `SerialStudio.lua`, `sdk-symbols.json`, `proto-fields.json`, `doc/grpc/serialstudio-typed.proto`, `app/rcc/ai/search_index.json` | Regenerated by the generators `sanitize-commit.py` runs; proto numbers append only. |

### D. Fixed Length framing (Core, Pipeline, Ui, Api)

| File | Change |
|------|--------|
| `core/Core/SerialStudio.h` | `FixedLength = 0x04` in `FrameDetection`. |
| `core/Core/EnumLabels.cpp` | `fixed-length` slug and "Fixed Length" label. |
| `core/Core/IO/FrameConfig.h` | `int fixedFrameLength = 0; quint8 headerMask, headerValue, bodyMask, bodyValue = 0;`. |
| `core/Core/IO/FrameConfigBuilder.{h,cpp}` | Copy the five fields from the `Source`; `repairedDetection()` downgrades a FixedLength config whose length is < 1 or > half the reader buffer to NoDelimiters and normalises `value &= mask`; the function becomes public so `AppState` stops duplicating it. |
| `core/Core/DataModel/Frame.h`, `Frame.cpp`, `FrameKeys.h` | `Source` gains `frameLength`, `frameHeaderMask`, `frameHeaderValue`, `frameBodyMask`, `frameBodyValue` (ints, default 0); keys `Keys::FrameLength`, `Keys::FrameHeaderMask`, `Keys::FrameHeaderValue`, `Keys::FrameBodyMask`, `Keys::FrameBodyValue`; written only when `frameDetection == FixedLength` or non-zero, read with `ss_jsr` defaults; `read_fixed_frame_settings(FixedFrameSettings&, obj)` beside `read_io_settings`. No `kSchemaVersion` bump. |
| `scripts/code_verify_rules.py` | The five literals join `_PROJECT_KEY_LITERALS`. |
| `core/Core/CircularBuffer.h` | `findMaskedByte(mask, value, from)`: wrap-aware linear scan returning the first offset whose byte satisfies `(b & mask) == value`, no allocation. |
| `core/Pipeline/IO/FrameReader.{h,cpp}` | `setFixedLength(int)`, `setByteRules(hm, hv, bm, bv)`, members, `readFixedLengthFrames()`, the `case FixedLength` in the ProjectFile switch. |
| `core/Pipeline/IO/PipelineHost.cpp` | `attach()` applies the two new setters from `FrameConfig`. |
| `core/Pipeline/AppState.cpp` | `deriveFrameConfig` copies the fields and calls the shared `repairedDetection()` instead of its inline copy. |
| `core/Pipeline/DataModel/Scripting/FrameParserPipeline.{h,cpp}` | `PipelineSpec` gains the fields; `configureAndFeed` applies them. |
| `core/Storage/Sessions/Verifier.cpp` | Nothing to edit if it builds through `FrameConfigBuilder::build`; confirm during tasks. |
| `core/Ui/ProjectEditor/ProjectEditor.{h,cpp}` | Append "Fixed Length" / `FixedLength` to `m_frameDetectionMethods` / `Values`. |
| `core/Ui/ProjectEditor/EditorForms.cpp` | In `buildSourceFrameDetectionRows`: hide the start, end and hex-delimiter rows for FixedLength; add Frame Length (IntField, min 1), Header Rule mask/value and Body Rule mask/value (HexText, 1 byte) rows shown only for FixedLength. In `buildSourceModel`: a Button row "Connection Sequence…" with the step count, UART sources only. |
| `core/Ui/ProjectEditor/ProjectEditorItemIds.h`, `EditorCommit.cpp` | New `SourceItem` ids and their commit handlers (`handleSourceFixedFrameChange`, `handleSourceConnectionSequenceButton`). |
| `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp` | `kDetectionValues` and `detectionOptions()` gain the mode; `inputContainsDelimiters()` returns `input.size() >= length` for FixedLength instead of tripping the assert; `dryRun()` copies the fields into the spec. |
| `app/qml/Dialogs/FrameParserTest.qml` | Verify index 4 needs neither delimiter (it does by construction); no new inputs, the dialog tests the source's configured settings. |
| `core/Ui/AI/Tools/ToolSchemas.cpp`, `ToolScriptTools.cpp`, `core/Ui/ApiHandlers/AssistantHandler.cpp`, `core/Ui/AI/ContextBuilder.cpp` | Mode text names five values; the new keys join the pass-through whitelist and the strip list. |

### E. UI: Setup Panel and the sequence editor (Ui, app)

| File | Change |
|------|--------|
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml` | "Hold RTS" `CheckBox` (`setup.uart.rtsEnabled`) after the DTR row, disabled with a tooltip while flow control is RTS/CTS; a full-width `Widgets.PushButton` "Connection Sequence…" (`setup.uart.connectionSequence`) with a "N steps" status label, the Modbus groups layout. |
| `app/qml/ProjectEditor/Dialogs/ConnectionSequenceEditor.qml` | **New.** One editor for both hosts, modelled on `AlarmBandsEditor.qml`: a step list with add, remove, move up/down, a kind combo and per-kind fields (duration, line + state, data + hex + EOL + checksum, patterns + timeout + optional, baud, flow mode), a budget field, inline validation text from the C++ `validate()`, Apply/Cancel. Payload in and out is the sequence JSON. |
| `core/Ui/ProjectEditor/ProjectEditor.{h,cpp}` | `openConnectionSequenceEditorForSelection()` / `commitConnectionSequence(json)` following the alarm-bands pair; commit goes `drv->setDriverProperty("connectionSequence", json)` then `captureSourceSettings`. |
| `app/src/Misc/ModuleManager.cpp` or the UART driver | `Cpp_IO_Serial.connectionSequenceJson` / `setConnectionSequenceJson` and `validateConnectionSequence(json)` for the Setup Panel host (Q_PROPERTY + slot on the driver; no new context property). |
| `app/CMakeLists.txt` | Register the new QML file in `QML_SOURCES`. |
| `core/Ui/Console/Handler.{h,cpp}` | Subscribe to `LinkSequenceNote`; append the text as a system-styled line through the same per-device buffer the data uses, so the per-source filter applies. |

### F. Tests

| File | Change |
|------|--------|
| `app/tests/tst_connection_sequence.cpp` | **New** ctest (Core only): JSON round-trip, `QVariantMap` restore, validation errors with step index, pattern parsing, wildcard matching, match-end offsets. |
| `app/tests/tst_sequence_runner.cpp` | **New** ctest: `FakePort` + `VirtualClock` (the `tst_async_engine` clock): a six-step success path; expect timeout, write refusal and port error each fail with the right step index and reason; an optional expect continues; the budget expires; `cancel()` mid-step reports nothing; BREAK holds exactly the requested virtual time; bytes during BREAK are dropped. |
| `app/tests/tst_uart_policy.cpp` | The two new pure predicates. |
| `app/tests/tst_frame_reader_modes.cpp` | Fixed Length cases: plain cutting; a frame split across chunks; header rule skips garbage; body-rule violation resyncs to the next header; checksum accepted, rejected, incomplete; length 1; timestamps from the chunk holding the last byte; NoDelimiters repair at configuration. |
| `app/tests/tst_enum_labels.cpp` | Sweep gains the fifth value; out-of-range becomes 5. |
| `app/tests/tst_frame_serialization.cpp`, `tst_frame_json_legacy.cpp` | Source round-trip with the five fields; absent keys read as zero. |
| `app/tests/tst_command_capability_registry.cpp` | One `argumentRefinements_data` row for `io.uart.setSequence`. |
| `app/tests/CMakeLists.txt` | Register the two new tests (`tst_connection_sequence`: Core + `IO/ConnectionSequence.cpp`; `tst_sequence_runner`: adds the runner, `Core/Async/TaskTree.cpp`, `RetryPolicy.cpp`, `Qt6::SerialPort`). |
| `tests/integration/test_uart_sequence.py` | **New**, markers `uart`, `integration`; skips on Windows or without `socat`. The test plays the device on the other pty: verifies connected only after it answers (AC5), frames flow after (AC4), no answer fails within the budget with the step named (AC4), the handshake bytes are absent from the CSV export (AC6), a live baud change keeps the connection (AC3), and a project with a sequence round-trips through the API (AC7). |
| `tests/integration/test_fixed_length_frames.py` | **New**: a TCP source streaming 5-byte frames with a header bit; asserts dataset values, resync after an injected byte, Quick Plot ignoring the mode, and the zero-length repair. |
| `tests/utils/api_client.py`, `tests/utils/data_generator.py` | `configure_frame_parser` and the generator accept the five fields; docstrings name mode 4. |
| `tests/README.md`, `tests/pytest.ini` | Rows for the two files (no new marker). |

### G. Documentation

| File | Change |
|------|--------|
| `doc/help/Drivers-UART.md` | Hold RTS in the settings table; the "Connection Sequence" section (step kinds, expect patterns, budget, failure text, console marker, re-run); the `io.uart.*` list (line 90) gains the new commands. |
| `doc/help/Data-Flow.md`, `Operation-Modes.md`, `Concepts.md`, `Glossary.md`, `Project-Editor.md` | Five modes; Fixed Length rows with the header and body rules. |
| `doc/help/API-Reference.md` | Sections for the seven commands; the three `0-3` mentions become `0-4`. |
| `doc/help/Control-Loop.md` | One example of `io.uart.sendBreak` and `io.uart.runSequence` under the 2 s watchdog note. |
| `app/rcc/ai/skills/project_basics.md`, `frame_parsers.md`, `debugging.md`, `behavioral.md`, `tool_discovery.md` | Mode tables and the UART command group; `search_index.json` regenerated. |
| `doc/claude/architecture/io.md` | "Opening a Link" gains the UART sequence bullet (isOpen false while sequencing, verdict once, bytes withheld, cancel in close); the Sub-objects section notes the `rawDataSent` tap. |
| `doc/claude/architecture/dataflow.md` | FrameReader row names the fifth extractor and the masked-byte scan. |
| `doc/claude/common-mistakes.md` | IO row: `UART::isOpen()` is the connected state, `portOpen()` the port; gating on the wrong one swallows errors or leaks handshake bytes. |
| `CLAUDE.md` | One clause in the "Driver opens are synchronous" bullet naming UART as an async opener when a sequence is set. |

## Architecture & data flow

### Connect with a sequence

```
ConnectionManager::openDevice(id)              GUI thread
  armOpenReport(); DeviceManager::open() → UART::open(mode)
    port created and opened as today (saved baud, DTR, RTS when applicable)
    if sequence empty: return true                      (today's synchronous path, no report)
    else: m_runner = SequenceRunner(sequence, portAdapter, m_clock)
          m_sequenceRunning = true  → isConnecting() true, isOpen() false
          runner.start(); return true
  ConnectionManager sees ok && isConnecting() → notePendingDial, wait cursor released,
  notifyConnectedStateChanged() publishes connecting

QSerialPort::readyRead → UART::onReadyRead
  portOpen() && m_breakHeld        → drop
  portOpen() && m_sequenceRunning  → runner.feed(bytes); publishConsoleData(bytes)   (console only)
  portOpen()                       → publishReceivedData(bytes)                        (today)

SequenceRunner (task tree on the GUI thread, AsyncClock timers)
  TimeoutTask(budget) { SequentialGroup { step tasks } }
  Send   → port.write(bytes); Q_EMIT bytesSent → UART::publishSentData → dataSent
           → DeviceManager::rawDataSent → DeviceIoRouter → taps->onSentBytes (console echo)
  Expect → buffer bytes from feed(); match any pattern; consume through the match; fail on timeout
  Break  → port.setBreak(true); clock timer; port.setBreak(false); port.clearInput()
  each step → Q_EMIT stepStarted → UART publishes Core::Bus::LinkSequenceNote → Console line
  finished(ok, reason, step)
    ok   → m_sequenceRunning = false; reportOpenFinished(true); Q_EMIT configurationChanged()
    fail → m_sequenceRunning = false; logDriverError(title, reason); reportOpenFinished(false, reason)
           ConnectionManager::onDriverOpenFinished → disconnectDevice → UART::close() (re-entrant, runner already finished)

connectedChanged (once, after the verdict)
  FrameBuilder::onConnectedChanged → session boundary, auto-execute actions, template frames
  ControlScript::onConnectedChanged → setup()
```

`close()` runs `m_runner->cancel()` first; the tree's cancel is silent by design (`TaskRunner`
disconnects before cancelling), so a user disconnect mid-sequence produces no verdict, which is
what `disconnectDevice` expects after it disarmed the report. A fatal port error mid-sequence
takes today's path: `handleError` (gated on `portOpen()`, not `isOpen()`) calls
`disconnectDevice(this)`, which closes and cancels; with auto-reconnect on, the 1 s poll reopens
and the sequence runs again, which is R7's reconnect case.

### Re-run on demand (R11)

`UART::runSequence()` on an open, connected port keeps `isOpen()` true (no session boundary,
no sink close, no Control Loop restart) and sets a `m_sequenceGating` flag that routes bytes to
the runner and refuses writes exactly as the connect-time run does. The outcome is a
`LinkSequenceNote` plus, on failure, a `NotificationRaised` warning through `messageBus()`. The
API command returns `{started: true}` immediately; the result is observable in the console and
the notification center.

### Fixed Length extraction

```
FrameReader::processData (pipeline thread) → appendChunk → case FixedLength: readFixedLengthFrames()
  loop ≤ kMaxFramesPerCall:
    if size < N: break
    if headerMask: i = m_circularBuffer.findMaskedByte(headerMask, headerValue, 0)
                   if i < 0: consumeBytes(size); break       (nothing can start a frame)
                   if i > 0: consumeBytes(i)
                   if size < N: break
    if bodyMask:   v = first offset in [1, N) with (b & bodyMask) != bodyValue (operator[] scan)
                   if v found: consumeBytes(v); continue       (the violator may be the next header)
    cd = claimCapturedSlot(ptr); peekRangeInto(0, N - m_checksumLength, cd->data)
    r = checksum(cd->data, N - m_checksumLength)
    FrameOk        → enqueueCaptured(ptr, cd, N)              (stamps, counts, consumes N)
    ChecksumError  → consumeBytes(headerMask ? 1 : N)         (resync by scan, or stay aligned)
```

Timestamps come from `frameTimestamp(N)` through `enqueueCaptured`, so a frame is stamped from
the chunk holding its last byte like every other mode. Diagnostics use the existing counters.
Quick Plot never reaches the switch (it calls `readEndDelimitedFrames()` first), satisfying R16.

### Settings flow

`Source` fields → `FrameConfigBuilder::build` (project connect, Verifier, stream config) and
`AppState::deriveFrameConfig` (source 0 / Quick Plot) → `FrameConfig` → `PipelineHost::attach`
→ `FrameReader` setters. The Project Editor's dry run builds `PipelineSpec` from the same
`Source` fields. The sequence travels separately: `driverProperties()` → `driverSettings()` →
`sources[].connection.connectionSequence` → `applyConnectionSettings` → `setDriverProperty`.

## Hotpath & threading impact

- **Touches the hotpath?** Yes, `FrameReader` only. `readFixedLengthFrames()` is a fourth
  extractor with the same shape as `readStartDelimitedFrames()`: pipeline thread inside
  `processData`, SPSC with no lock, pooled `CapturedData` slots through `claimCapturedSlot`,
  `peekRangeInto` into the slot's `QByteArray` (no temporaries), the `kMaxFramesPerCall` cap,
  `SS_ASSERT_HOTPATH` on conditions that restate guards, and `SS_ASSERT` with recovery on the
  one condition derived from device bytes (the header scan result). `findMaskedByte` is a
  wrap-aware linear scan over the existing storage with no allocation. The three delimiter
  readers and the NoDelimiters fast lane are untouched, so the data-pipeline benchmark tier
  (EndDelimiterOnly workload) cannot move; the maintainer also runs the benchmark once with a
  Fixed Length project loaded to confirm the parse tiers hold with the new mode selected.
- **New cross-thread signal/slot?** No per-frame hop. `dataSent → DeviceManager::rawDataSent →
  DeviceIoRouter` is a GUI-thread Direct connection at handshake rate, mirroring
  `rawDataReceived`. `LinkSequenceNote` is a bus message at step rate. The sequence runner,
  its timers and the port all live on the GUI thread with the driver; nothing blocks and no
  event loop is spun.
- **New input to a cached hotpath flag?** No. `isConnecting()` already feeds
  `ConnectionStateChanged`, which the existing caches consume; the sequence only changes when
  that edge fires.
- **Timestamp ownership.** Fixed Length frames are stamped by `enqueueCaptured` from the chunk
  timeline; the driver keeps stamping at `publishReceivedData`. Nothing downstream re-stamps.

## Data model & persistence

- **`Source`** (`core/Core/DataModel/Frame.h`): five `int` fields with defaults 0, keys in
  `FrameKeys.h`, serialized only when `frameDetection == FixedLength` or non-zero, read with
  `ss_jsr` defaults. A project saved before this spec reads zeros and never selects mode 4, so
  nothing changes for it. A project saved with mode 4 and opened by an older build hits the
  existing `repairedDetection()`, which maps an unknown detection to NoDelimiters through its
  empty-delimiter branch; the file is not refused. No `kSchemaVersion` bump: a bump shows every
  old project the "defaults applied" notice for no gain.
- **Connection Sequence**: `sources[].connection.connectionSequence` =
  `{ "budgetMs": 10000, "steps": [ {"kind":"break","ms":55}, {"kind":"wait","ms":50},
  {"kind":"discard"}, {"kind":"send","data":"15 15","hex":true,"eol":"","checksum":""},
  {"kind":"expect","patterns":["55","5D 5?"],"timeoutMs":100,"optional":false},
  {"kind":"baud","rate":115200}, {"kind":"flow","mode":"rtscts"},
  {"kind":"line","line":"rts","high":true} ] }`. Absent = empty sequence. The UI driver also
  persists it as JSON text under `UartDriver/connectionSequence`, like its other settings, so
  Quick Plot and Console sessions keep it across restarts.
- **Hold RTS**: `connection.rts` (bool) and `UartDriver/rts`, default false. `dtr` keeps its
  default of true.
- **Sealed packages**: `stripSealedContent` already clears every source's `connection` on the
  copies that leave the process (mirror, API, MQTT, historian); the sealed process itself loads
  and runs the sequence. No change.
- **No dataset-registry change**: there is no source property registry, so no generated TU is
  touched. `_PROJECT_KEY_LITERALS` in `scripts/code_verify_rules.py` gains the five keys so the
  hardcoded-literal lint covers them.

## API / SDK surface

- Seven `io.uart.*` commands registered in `UARTHandler::registerCommands()` with
  `API::makeSchema` / `rangeProp` schemas; `sourceId` optional and defaulting to 0 on the four
  live-control commands (a new convention for `io.*`, recorded in the API reference and the
  handler comment). `getConfig` reports `rtsEnabled`, `liveBaudRate` and `sequenceStepCount`.
- Project sources set their sequence through the existing `project.source.setProperty(sourceId,
  "connectionSequence", json)`; `io.uart.setSequence` edits the UI driver (single-source and
  Setup Panel sessions).
- Frame settings: `source.update`, `project.frameParser.update` and the dry-run accept
  `frameLength`, `frameHeaderMask`, `frameHeaderValue`, `frameBodyMask`, `frameBodyValue`;
  range checks on `frameDetection` move from `3` to `SerialStudio::FixedLength`.
- Classifications: capability table, sealed table and assistant tier for every new name (the
  three registry ctests fail otherwise). SDK wrappers (`io.uart.sendBreak(durationMs, {sourceId})`
  and so on) come from regenerating the schema snapshot and running `generate-sdk.py`; the
  Control Loop reaches them through `apiCall` with no prelude edit.
- Every build: no `BUILD_COMMERCIAL` guard anywhere in this spec.

## QML / UI

- **Setup Panel** (`Drivers/UART.qml`): Hold RTS checkbox bound to `Cpp_IO_Serial.rtsEnabled`
  with `enabled: Cpp_IO_Serial.flowControlIndex !== 1` and the "not applicable under RTS/CTS"
  tooltip; the "Connection Sequence…" button plus a "N steps" label spanning both columns, as
  Modbus does for its register groups; both carry `setup.uart.*` object names (spec 0101).
- **Editor dialog** (`ProjectEditor/Dialogs/ConnectionSequenceEditor.qml`): `SmartDialog`
  with `PopupEnter`/`PopupExit`, a `ListView` of steps with a per-kind `Loader`, add/remove/move
  buttons, the budget field, a validation banner fed by `validateConnectionSequence(json)`, and
  Apply/Cancel. Payload in and out as a JSON object string. Fields follow the `TableDelegate`
  rule for editable text: sync from the model only while unfocused, clamp at commit.
- **Project Editor source form**: the Button row opens the same dialog for the selected UART
  source; the frame-detection rows gain Frame Length and the two rule pairs, visible only for
  Fixed Length, and hide the start, end and hex-delimiter rows in that mode. The combo keeps its
  own order with "Fixed Length" appended, mapped through `m_frameDetectionMethodsValues`.
- **Console**: `LinkSequenceNote` lines render in the system style with the
  `[Connection Sequence]` prefix; handshake RX bytes arrive through `onConsoleBytes` and TX
  echoes through `onSentBytes`, so they look like device traffic and the note lines mark the
  steps between them.
- **Terminology** (spec 0100): "Connection Sequence", "Hold RTS", "Send BREAK", "Fixed Length",
  "Header Rule", "Body Rule" everywhere; prose says "Control Loop" and "Device", never "control
  script" or "data source".

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the sequence runs | (a) in the UART driver over its open port; (b) a generic pre-data hook in the connection manager; (c) Control Loop `setup()` with new primitives | **(a)**. The spec scopes sequences to UART; (b) rebuilds the orchestration layer io.md removed in 2026-07 for a generality the spec declares a non-goal; (c) cannot keep handshake bytes out of the pipeline or delay the connected edge. |
| Step runner implementation | (a) `Async::` task tree (`SequentialGroup` + `TimeoutTask` + `InvokeTask`, three small step tasks); (b) a bespoke `QTimer` state machine | **(a)**. Cancel-exactly-once, step identity in `StepError`, the budget as a `TimeoutTask` and a virtual clock for the ctest come for free from an engine already unit-tested; (b) re-implements all four. A sequence is a lifecycle boundary, which is what the engine is for. |
| Connected state during the sequence | (a) `isOpen()` false until the verdict (the Network pattern); (b) `isOpen()` true and a separate "gating" flag | **(a)** for the connect-time run, because `isConnected()` is `isOpen()` and every consumer keys on that edge; **(b)** for the on-demand re-run, where a false `isOpen()` would close recordings and restart the Control Loop. Internals use `portOpen()`. |
| Writes during the sequence | (a) refuse (`write()` returns 0); (b) queue like Network's dial | **(a)**. A queued write released at the end of a handshake could corrupt the device's first data frame; a refused one is visible in the API reply. |
| Console marking | (a) RX via `publishConsoleData`, TX via the unused `dataSent` signal through the router's tap, step lines as a bus note; (b) a new annotation-layer entry point; (c) `qInfo` per step | **(a)**. Reuses two existing channels and one new bus topic; (b) needs TX offsets the RX-only annotation model cannot carry; (c) ignores the per-source console filter and routes through the debug toggle. |
| Sequence storage | (a) label-less driver property inside `connection`; (b) a new `Source` field with its own apply path | **(a)**. Persistence, restore, the editor capture and the UI-to-live sync all exist for driver properties (Modbus `registerGroups`); (b) adds a second apply path to `buildDeviceForSource` and the UI sync for no benefit, since sealed exits strip both the same way. |
| Fixed Length configuration | (a) five `Source` fields and `FrameConfig` fields; (b) encode length and rules into the `frameStart` string | **(a)**. Typed fields validate at configuration and read cleanly in the API; (b) overloads a delimiter string with a second grammar. |
| Checksum error in Fixed Length | consume 1 byte or consume N | Consume 1 when a header rule exists (the scan resyncs), N otherwise (the stream is assumed aligned). |
| Live targeting of `io.uart.*` | (a) optional `sourceId` on the four live-control commands, default 0; (b) UI driver only | **(a)**. BREAK, baud, line states and re-run act on an open port, which in a multi-source project is a per-source live driver; the UI-config instance has no port. Getters and setters of settings stay on the UI driver like today. |
| Capability of `setSequence` | plain; `WritesDevice` always; argument-dependent | **Argument-dependent** (`{NoCapability, WritesDevice}` when a Send step is present), the `project.action.update` precedent for stored writes that fire on connect. |
| BREAK an adapter cannot time | fail the step; warn and continue | **Warn and continue** (the spec's recommendation): `setBreakEnabled` reports success or failure, not duration; a failure fails the step, a success is trusted. |
| Setup-Panel sequence editing | inline list in the pane; a button to the shared dialog | **Button to the shared dialog**: the pane is a two-column form, and one editor keeps the two hosts identical. |

## Risks & mitigations

- **`isOpen()` false while the port is open** is the one new invariant that can bite: every
  internal check that means "the port exists" must read `portOpen()`. `handleError` would
  otherwise swallow a mid-sequence unplug and `onReadyRead` would leak handshake bytes.
  Mitigation: one private predicate, a `common-mistakes.md` row, and the runner ctest's
  port-error case; `tst_uart_policy` keeps the error classification pure.
- **Re-entrant `close()`** from `onDriverOpenFinished(false)`. The runner has already
  finished when the verdict is reported, and `cancel()` on a finished tree is a no-op; the
  ctest covers cancel-after-finish.
- **UI-to-live sync mid-sequence** (single-source projects mirror every Setup Panel edit onto
  live driver 0). A baud change applied by `setBaudRate` during the handshake would corrupt
  it. Mitigation: the setters apply to the port only when no sequence is running; the pending
  value takes effect on the next open, which is what they already do for a closed port.
- **QSettings pollution**: per-source live instances already write the shared `UartDriver/*`
  keys from their setters. The sequence steps use the port adapter, never the persisting
  setters, so a project's baud step cannot rewrite the Setup Panel's default. `rts` follows
  the existing pattern, imperfect as it is, because fixing the pollution is out of lane; it is
  named here only.
- **Registry ctests and the schema snapshot**: `tablesNameOnlyRealCommands` fails if a table
  names a command absent from `api-schema.json`, and `everySchemaCommandIsClassifiedOnce` fails
  the other way. The tables, the tier file and the snapshot land in one commit; the maintainer
  regenerates the snapshot from the built binary before running ctest.
- **Two combobox orders versus the enum order**: both editors map through value lists, never
  indices; "Fixed Length" is appended last in both, and `FrameParserTest.qml`'s hard-coded
  indices already yield "no delimiter needed" for index 4. The tasks include a check of that
  dialog with the new mode.
- **BREAK fidelity by platform**: Qt uses `SetCommBreak`/`ClearCommBreak` on Windows and
  `TIOCSBRK`/`TIOCCBRK` on POSIX; some USB adapters emit a fixed pulse. AC2 is measured on the
  maintainer's adapters; the step trusts a successful `setBreakEnabled` and fails only on a
  reported failure. Bytes received while BREAK is held are dropped in `onReadyRead`.
- **`setRequestToSend` under hardware flow control** fails with `UnsupportedOperationError`
  (Qt documents it). The driver never calls it in that mode; the checkbox is disabled and the
  property row carries a `visibleWhen` rule.
- **Translation-unit length**: `UART.cpp` is 1152 lines against the 1500 cap and the growth
  ratchet. The runner, the port seam and the step model are separate TUs; the driver gains the
  glue only.
- **Silent-breakage classes exposed** (common-mistakes.md): a queued hop on the frame path
  (none added), `QMap::operator[]` on source maps (not touched), timestamp re-stamping (not
  touched), a stale cached flag (no new input), the editable-field echo trap in the new dialog
  (follow `TableDelegate`), `%n` with `.arg()` in the new strings (use numbered placeholders).

## Test & verification plan

- **Unit (you can run):** none in `tests/scripts/`; the new logic is C++.
- **ctest (maintainer builds, you may run against the build dir):**
  - `tst_connection_sequence` (new): AC7's model half, pattern grammar, validation.
  - `tst_sequence_runner` (new): AC4's logic half with a fake port and a virtual clock, AC2's
    discard rule, AC5's ordering (no verdict before the last step).
  - `tst_frame_reader_modes`: AC9 in full, AC10's repair.
  - `tst_enum_labels`, `tst_frame_serialization`, `tst_frame_json_legacy`: the enum and the
    five keys.
  - `tst_command_capability_registry`, `tst_sealed_policy_registry`, `tst_assistant_batch_tiers`:
    AC8's classifications.
  - `tst_uart_policy`: the two new predicates.
- **Integration (maintainer runs, app up with the API server):**
  - `tests/integration/test_uart_sequence.py` with a `socat` pty pair: AC3, AC4 (positive and
    negative), AC5, AC6, AC7; skips on Windows.
  - `tests/integration/test_fixed_length_frames.py` over a TCP source: AC10, part of AC9.
  - `tests/integration/test_api_drivers.py`: `io.uart.getConfig` carries the new fields;
    `io.uart.setRtsEnabled` round-trips (AC1's API half).
  - `tests/integration/test_api_surfaces.py` and `tests/scripts/test_proto_ledger_static.py`
    pass after the regenerations (AC8).
- **Maintainer observations on hardware:** AC1 (RTS on a line LED or a second port's CTS),
  AC2 (BREAK duration on a logic analyser, native and USB adapters, each platform), AC4 and
  AC6 on the test-cell adapter (connect from the button, the API, the CLI and after a power
  cycle; console shows the marked handshake, the CSV export does not), AC10 in the
  frame-parser test dialog.
- **Hotpath:** `--benchmark-hotpath --min-fps 256000` unchanged (AC11), plus one run with a
  Fixed Length project loaded to read the parse tiers.
- **Static:** `python scripts/code-verify.py --check <files>` per task; `scripts/syntax-check.py`
  on every edited C++ file; `qt-cpp-review` on the C++ diff; `term-verify.py`,
  `documentation-verify.py`, `claim-verify.py` on the docs; `python scripts/sanitize-commit.py`
  before commit (runs `generate-sdk.py`, the property registry, the search index and
  `registry-verify.py`).
