---
spec: 0103-serial-bringup-and-fixed-frames
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-10-26
---

# Tasks 0103 — Serial Bring-Up Sequences, Line Control and Fixed-Length Frames

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
  Every C++ task also runs `python scripts/syntax-check.py <files>`; a SKIP means "unverified"
  and is reported as such. Every task with a user-facing string runs `term-verify.py`.
- **Deps** lists task IDs that must land first.
- **Build points** are the maintainer's; nothing here invokes cmake or a compiler:
  - **B1** after T9: the Fixed Length reader is complete, the ctest tier runs and
    `--benchmark-hotpath` measures AC11 at a clean point.
  - **B2** after T28: the sequence runner, the driver and their ctests build; hardware
    observation of a bring-up can start from the Setup Panel.
  - **B3** after T31: the handler is registered, so `SerialStudio --dump-api-schema
    app/rcc/api/api-schema.json` refreshes the snapshot, `sanitize-commit.py` regenerates the
    SDK, the proto ledger and the search index, and the three registry ctests run.
  - **B4** after T36: the editor dialog and the Setup Panel changes exist; the pytest files and
    the maintainer's observations (AC1, AC2, AC4, AC6, AC10) run against this build.
- Tasks marked **delegable** are mechanical sweeps the main session briefs to a cheaper
  executor (Sonnet; Haiku for one-line sweeps) and reviews; hotpath, driver, security-surface
  and design tasks stay in the main session.
- Four groups: Fixed Length (T1 to T18), the sequence model, runner and driver (T19 to T28),
  the API surface (T29 to T32), the UI and documentation (T33 to T42).

## Tasks

### Group 1 — Fixed Length framing

### T1 — Enum value and labels

- **Files:** `core/Core/SerialStudio.h`, `core/Core/EnumLabels.cpp`,
  `app/tests/tst_enum_labels.cpp`
- **Does:** Adds `FixedLength = 0x04` to `FrameDetection`; `frameDetectionSlug()` answers
  `fixed-length` and `frameDetectionLabel()` "Fixed Length"; the sweep test gains the fifth row
  and the out-of-range probe moves from 4 to 5.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tst_enum_labels` at B1.
- **Deps:** none
- [ ] done

### T2 — Source fields, keys and the settings reader

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`,
  `core/Core/DataModel/Frame.cpp`, `scripts/code_verify_rules.py`
- **Does:** `Keys::FrameLength`, `FrameHeaderMask`, `FrameHeaderValue`, `FrameBodyMask`,
  `FrameBodyValue`; five `int` fields on `Source` (default 0, the `alignas` static_assert kept
  true); `serialize()` writes them only when `frameDetection == FixedLength` or non-zero,
  `read()` uses `ss_jsr` defaults; `read_fixed_frame_settings(FixedFrameSettings&, obj)`
  beside `read_io_settings`, normalising `value &= mask`; the five literals join
  `_PROJECT_KEY_LITERALS`. A project from before this spec reads zeros and behaves as today.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T3's tests.
- **Deps:** T1
- [ ] done

### T3 — Serialization tests for the new fields

- **Files:** `app/tests/tst_frame_serialization.cpp`, `app/tests/tst_frame_json_legacy.cpp`
- **Does:** The Source round-trip carries the five fields and the FixedLength mode; absent keys
  read as zero; `read_fixed_frame_settings` masks the value and leaves a legacy object alone.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; both ctests at B1.
- **Deps:** T2
- [ ] done

### T4 — FrameConfig and the shared repair

- **Files:** `core/Core/IO/FrameConfig.h`, `core/Core/IO/FrameConfigBuilder.h`,
  `core/Core/IO/FrameConfigBuilder.cpp`
- **Does:** `FrameConfig` gains `fixedFrameLength` and the four rule bytes; `build()` copies
  them from the `Source` through `read_fixed_frame_settings`; `repairedDetection()` becomes a
  public function of the builder and downgrades a FixedLength config whose length is below 1 or
  above `kMaxFixedFrameLength` (half the reader buffer, a named constant) to NoDelimiters with
  one `qWarning`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; read back that the two existing
  repair branches are unchanged.
- **Deps:** T2
- [ ] done

### T5 — AppState uses the shared repair

- **Files:** `core/Pipeline/AppState.cpp`
- **Does:** `deriveFrameConfig()` copies the five fields for the source-0 path and replaces its
  inline repair copy with `FrameConfigBuilder::repairedDetection()`; the Quick Plot and
  Console paths are untouched. Binding invariant: `m_frameConfig` is re-derived on
  `frameDetectionChanged` and published on the bus; no new signal is added.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; read back the three mode paths.
- **Deps:** T4
- [ ] done

### T6 — Masked-byte scan on the circular buffer

- **Files:** `core/Core/CircularBuffer.h`, `app/tests/tst_circular_buffer.cpp`
- **Does:** `findMaskedByte(mask, value, from)`: a wrap-aware linear scan over the existing
  storage returning the first offset at or after `from` whose byte satisfies
  `(b & mask) == value`, or -1. Binding invariants: SPSC header on the pipeline thread, no
  allocation, no lock, reads through the same head/tail loads as `byteScanWrap`. The unit test
  covers a hit before, across and after the wrap point, `from` past the end, and an empty buffer.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tst_circular_buffer` at B1.
- **Deps:** none
- [ ] done

### T7 — FrameReader: the Fixed Length extractor

- **Files:** `core/Pipeline/IO/FrameReader.h`, `core/Pipeline/IO/FrameReader.cpp`
- **Does:** `setFixedLength(int)`, `setByteRules(hm, hv, bm, bv)`, the five members, and
  `readFixedLengthFrames()` wired as `case FixedLength` in the ProjectFile switch, following
  the plan's algorithm (header scan, body-rule check, pooled slot, checksum over the first
  `N - m_checksumLength` bytes, consume 1 on a checksum error when a header rule exists, else
  N). Binding invariants, named for the edit: this runs inside `processData` on the pipeline
  thread, SPSC with no mutex; frames fill pooled slots through `claimCapturedSlot` and
  `peekRangeInto` with no temporaries; the loop is capped by `kMaxFramesPerCall`; per-frame
  asserts are `SS_ASSERT_HOTPATH` only where they restate a guard, the scan result from device
  bytes keeps `SS_ASSERT` with recovery; stamping and counting go through `enqueueCaptured`;
  no new signal hop; the NoDelimiters fast lane and the three delimiter readers are untouched.
- **Verify:** `code-verify.py --check` (hotpath rules are blockers), `syntax-check.py`; T9.
- **Deps:** T1, T6
- [ ] done

### T8 — Reader configurators carry the new settings

- **Files:** `core/Pipeline/IO/PipelineHost.cpp`,
  `core/Pipeline/DataModel/Scripting/FrameParserPipeline.h`,
  `core/Pipeline/DataModel/Scripting/FrameParserPipeline.cpp`
- **Does:** `attach()` applies the two new setters from `FrameConfig`; `PipelineSpec` gains the
  five fields and `configureAndFeed` applies them. Read `core/Storage/Sessions/Verifier.cpp`
  lines 604 to 616 and confirm it builds through `FrameConfigBuilder::build` and needs no edit
  (record the finding in the done note).
- **Verify:** `code-verify.py --check`, `syntax-check.py`; grep that every
  `setFrameDetectionMode(` call site also sets the fixed-length settings.
- **Deps:** T4, T7
- [ ] done

### T9 — Reader unit tests (AC9, AC10 repair)

- **Files:** `app/tests/tst_frame_reader_modes.cpp`
- **Does:** Fixed Length cases on the existing fixture: plain cutting at the length; a frame
  split across two chunks; a header rule skipping leading garbage; a body-rule violation
  dropping to the next header; checksum accepted, rejected and incomplete; length 1; the
  timestamp from the chunk holding the last byte; `framesExtracted` and `checksumErrorCount`
  deltas; and a `FrameConfigBuilder` case where length 0 repairs to NoDelimiters.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tst_frame_reader_modes` at B1.
- **Deps:** T7, T8
- [ ] done

**Build point B1** — ctest tier for T1 to T9; `--benchmark-hotpath --min-fps 256000` (AC11),
then once more with a Fixed Length project loaded to read the parse tiers.

### T10 — Project Editor option lists and item ids

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/ProjectEditor.cpp`,
  `core/Ui/ProjectEditor/ProjectEditorItemIds.h`
- **Does:** "Fixed Length" / `FixedLength` appended to `m_frameDetectionMethods` and
  `m_frameDetectionMethodsValues` (the combo keeps its own order and maps through the value
  list, never an index); new `SourceItem` ids `kSourceView_FrameLength`,
  `kSourceView_FrameHeaderMask`, `kSourceView_FrameHeaderValue`, `kSourceView_FrameBodyMask`,
  `kSourceView_FrameBodyValue` and `kSourceView_ConnectionSequence` at the end of the enum.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`.
- **Deps:** T1
- [ ] done

### T11 — Source form rows and their commits

- **Files:** `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** `buildSourceFrameDetectionRows` hides the start, end and hex-delimiter rows for
  FixedLength and adds Frame Length (IntField, min 1, max `kMaxFixedFrameLength`), Header Rule
  mask and value, Body Rule mask and value (HexText, one byte) shown only for FixedLength;
  `EditorCommit` routes the five ids to `handleSourceFixedFrameChange`, which validates,
  masks the value, calls `m_model.updateSource` and rebuilds the form when the signature
  changes. Binding invariant: a mutator opens its `ProjectUndoScope` and commits with
  `setModified(true)`, as the existing frame-start handler does.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`; read back the
  `showStart` / `showEnd` logic for the four old modes.
- **Deps:** T10
- [ ] done

### T12 — Frame-parser test model and dialog

- **Files:** `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp`,
  `app/qml/Dialogs/FrameParserTest.qml`
- **Does:** `kDetectionValues` and `detectionOptions()` gain the mode (appended, dialog order);
  `inputContainsDelimiters()` returns `input.size() >= frameLength` for FixedLength instead of
  reaching the `needs_start || needs_end` assert; `dryRun()` copies the five fields into the
  `PipelineSpec`. The QML's hard-coded indices already yield "no delimiter needed" for index
  4; confirm by reading lines 65 to 68 and 262 to 331 and note it in the done line.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`.
- **Deps:** T8, T10
- [ ] done

### T13 — API handlers accept the mode and its settings

- **Files:** `core/Api/API/Handlers/SourceHandler.cpp`,
  `core/Api/API/Handlers/ProjectParserCommands.cpp`,
  `core/Api/API/Handlers/ProjectDryRunCommands.cpp`
- **Does:** Descriptions name five modes; the two `<= 3` range checks become
  `<= SerialStudio::FixedLength`; `source.update`, `project.frameParser.update` and the dry run
  accept `frameLength`, `frameHeaderMask`, `frameHeaderValue`, `frameBodyMask`,
  `frameBodyValue` (validated: length 1..`kMaxFixedFrameLength`, bytes 0..255, else
  `InvalidParam`); the getters echo them; the dry run copies them into its spec. No new
  command, so no classification table changes here.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tests/integration/test_api_surfaces.py`
  at B3.
- **Deps:** T2, T8
- [ ] done

### T14 — Assistant surfaces (delegable)

- **Files:** `core/Ui/AI/Tools/ToolSchemas.cpp`, `core/Ui/AI/Tools/ToolScriptTools.cpp`,
  `core/Ui/ApiHandlers/AssistantHandler.cpp`, `core/Ui/AI/ContextBuilder.cpp`
- **Does:** The frame-detection schema text names `4=FixedLength`; the five keys join the
  pass-through whitelist and copy block in `ToolScriptTools` and the strip list in
  `AssistantHandler`; the prompt text in `ContextBuilder` mentions the mode where it lists the
  others. Mechanical; the executor's brief is this task text plus the four line ranges from
  the plan's reconnaissance.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`.
- **Deps:** T13
- [ ] done

### T15 — Python test helpers (delegable)

- **Files:** `tests/utils/api_client.py`, `tests/utils/data_generator.py`
- **Does:** `configure_frame_parser` and the project generator accept the five fields as
  optional keyword arguments; the two docstrings list mode 4.
- **Verify:** `python -m pyflakes` or an import of both modules; the existing
  `tests/scripts/` suite still passes.
- **Deps:** T13
- [ ] done

### T16 — Integration test: fixed-length frames over TCP

- **Files:** `tests/integration/test_fixed_length_frames.py`, `tests/README.md`
- **Does:** New file (markers `integration`, `network`, `project`): a TCP server in the test
  streams 5-byte frames with a header bit set in the first byte and clear in the others;
  asserts dataset values through `dashboard.getData`, resync after one injected byte, Quick Plot
  ignoring the mode, and the zero-length repair reported in the parser config. One README row.
- **Verify:** `pytest tests/integration/test_fixed_length_frames.py -v` at B3 (maintainer).
- **Deps:** T13, T15
- [ ] done

### T17 — Manual pages for Fixed Length (delegable)

- **Files:** `doc/help/Data-Flow.md`, `doc/help/Operation-Modes.md`, `doc/help/Concepts.md`,
  `doc/help/Glossary.md`, `doc/help/Project-Editor.md`, `doc/help/API-Reference.md`
- **Does:** Five modes everywhere the four are listed; Fixed Length rows explaining the length,
  the Header Rule and the Body Rule with one worked example; the three `0-3` mentions in the
  API reference become `0-4` with the new parameters named. Split the brief per file; prose
  says "Device" and "Control Loop".
- **Verify:** `documentation-verify.py` and `term-verify.py` on the six files.
- **Deps:** T13
- [ ] done

### T18 — Assistant skills and architecture notes for Fixed Length (delegable)

- **Files:** `app/rcc/ai/skills/project_basics.md`, `app/rcc/ai/skills/frame_parsers.md`,
  `app/rcc/ai/skills/debugging.md`, `app/rcc/ai/skills/behavioral.md`,
  `doc/claude/architecture/dataflow.md`
- **Does:** The mode tables gain a Fixed Length row; the FrameReader row in dataflow.md names
  the fifth extractor and the masked-byte scan. `search_index.json` is regenerated by
  `sanitize-commit.py` at B3, not by hand.
- **Verify:** `documentation-verify.py`, `term-verify.py`, `claim-verify.py` on dataflow.md.
- **Deps:** T7
- [ ] done

### Group 2 — Sequence model, runner and driver

### T19 — Connection Sequence model

- **Files:** `core/Core/IO/ConnectionSequence.h`, `core/Core/IO/ConnectionSequence.cpp`,
  `core/Core/CMakeLists.txt`
- **Does:** `IO::SequenceStep` (kind enum and fields per the plan), `IO::ConnectionSequence`
  (`steps`, `budgetMs` default 10000), `toJson()`, `fromJson()` accepting a `QJsonObject` or
  the `QVariantMap` shape `applyConnectionSettings` hands back, `validate()` returning the first
  error with its step index, `ExpectPattern` parsing (`"5D ?? 7F"`, `??` any byte) and
  `matchIn(buffer)` returning the end offset of the first match or -1; `stepText(index)` for
  the console lines. Qt-Core only, no I/O. Registered in the Core source list.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T20.
- **Deps:** none
- [ ] done

### T20 — Model unit test

- **Files:** `app/tests/tst_connection_sequence.cpp`, `app/tests/CMakeLists.txt`
- **Does:** JSON round-trip for every step kind, `QVariantMap` restore, validation errors with
  the step index (negative duration, empty send, bad hex, pattern longer than the buffer,
  budget below the sum of fixed waits), pattern parsing and wildcard matching, match-end
  offsets. Registered with `ss_add_unit_test(tst_connection_sequence SOURCES ... ${SS_CORE_SRC}/Core/IO/ConnectionSequence.cpp LIBS Qt6::Core SerialStudio::Core)`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; the ctest at B2.
- **Deps:** T19
- [ ] done

### T21 — Port seam and the sequence runner

- **Files:** `core/Devices/IO/Drivers/UART/SequencePort.h`,
  `core/Devices/IO/Drivers/UART/SequenceRunner.h`,
  `core/Devices/IO/Drivers/UART/SequenceRunner.cpp`, `core/Devices/CMakeLists.txt`
- **Does:** `ISequencePort` (write, clearInput, setBreak, setBaud, setLine, setFlowControl,
  lineStates); `SequenceRunner(const ConnectionSequence&, ISequencePort&, Async::AsyncClock&)`
  building `TimeoutTask(budget) { SequentialGroup { steps } }` with `BreakTask`, `DelayTask`,
  `ExpectTask` (4 KiB bounded buffer fed by `feed(bytes)`, consumes through the match, honours
  `optional`) and `Async::InvokeTask` for the rest; `start()`, `cancel()`, `feed()`;
  signals `finished(ok, reason, stepIndex)`, `stepStarted(index, text)`, `bytesSent(bytes)`.
  Binding invariants: thread-affine on the creating thread, one `finished` per run, cancel is
  silent, the budget is the engine's `TimeoutTask`, no retry policy of its own.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; T22.
- **Deps:** T19
- [ ] done

### T22 — Runner unit test with a fake port and a virtual clock

- **Files:** `app/tests/tst_sequence_runner.cpp`, `app/tests/CMakeLists.txt`
- **Does:** `FakePort` recording calls and scripting replies; the `VirtualClock` from
  `tst_async_engine`. Cases: a six-step success path with the verdict only after the last
  step; expect timeout, write refusal and port error each fail with the right step index and a
  non-empty reason quoting what was received; an optional expect continues; the budget
  expires; `cancel()` mid-step reports nothing; `cancel()` after finish is a no-op; BREAK holds
  exactly the requested virtual time and releases; bytes fed during BREAK are dropped.
  Registered with the runner, `Core/Async/TaskTree.cpp`, `RetryPolicy.cpp`,
  `Qt6::SerialPort` and `SerialStudio::Core`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; the ctest at B2.
- **Deps:** T21
- [ ] done

### T23 — Sent-bytes echo path

- **Files:** `core/Core/IO/HAL_Driver.h`, `core/Devices/IO/DeviceManager.h`,
  `core/Devices/IO/DeviceManager.cpp`
- **Does:** Protected `publishSentData(const QByteArray&)` emitting the declared, never-consumed
  `dataSent`; `DeviceManager` connects it beside `consoleDataReceived` and re-emits
  `rawDataSent(deviceId, data)`. Binding invariant: GUI-thread, handshake-rate traffic; never
  on the frame path.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `layer-verify.py` (Core header
  unchanged in its includes).
- **Deps:** none
- [ ] done

### T24 — Router fan-out for driver-internal sends

- **Files:** `core/Devices/IO/ConnectionManager/DeviceIoRouter.h`,
  `core/Devices/IO/ConnectionManager/DeviceIoRouter.cpp`, `core/Devices/IO/ConnectionManager.cpp`
- **Does:** `DeviceIoRouter::onRawDataSent(deviceId, data)` calls `m_taps[i]->onSentBytes` the
  way `writeToDevice` does; `ConnectionManager::wireDevice` connects `rawDataSent` to it with
  the same Direct GUI-thread connection `rawDataReceived` uses. Binding invariant: a Direct
  hop between two GUI objects at command rate, no queue, no per-frame emission.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; read back `wireDevice` so the
  disconnect path mirrors the connect.
- **Deps:** T23
- [ ] done

### T25 — Step notes on the bus and in the console

- **Files:** `core/Core/Bus/Messages.h`, `core/Ui/Console/Handler.h`,
  `core/Ui/Console/Handler.cpp`
- **Does:** `Core::Bus::LinkSequenceNote{int sourceId; QString text;}`; the console handler
  subscribes in its ctor (a `Core::Bus::Subscription` member beside `m_terminalModeWatch`) and
  appends `"[Connection Sequence] " + text` through the per-device buffer the data uses, in the
  system style, so the per-source filter applies. Binding invariant: bus traffic at step rate,
  never on the frame path.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`.
- **Deps:** none
- [ ] done

### T26 — UART line control: Hold RTS, BREAK, live baud, line states

- **Files:** `core/Devices/IO/Drivers/UART.h`, `core/Devices/IO/Drivers/UART.cpp`
- **Does:** `m_rtsEnabled` with `rtsEnabled` Q_PROPERTY, setter persisting `UartDriver/rts`,
  applied in `open()` after DTR only when flow control is not RTS/CTS, and a `rts` CheckBox
  property with `showWhen("flowControlIndex", {0, 2})`; `sendBreak(int ms)` (refused when no
  port or a BREAK is held; `setBreakEnabled(true)`, a clock timer, `setBreakEnabled(false)`,
  `clear(Input)`; `m_breakHeld` drops bytes in `onReadyRead`); `setLiveBaudRate(qint32)`
  (port rate, `setReadBufferSize(idealSerialBufferSize)`, the Windows native buffer; never
  `m_baudRate` or settings; emits `liveBaudRateChanged`); `lineStates()` returning
  `{cts, dsr, dcd, ri, rts, dtr}` from `pinoutSignals()`. The clock is an `Async::SystemClock`
  member so T28 shares it. Binding invariants: driver stays GUI-affine, nothing blocks, no
  persisting setter is called from these paths.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`; TU length stays
  under the ratchet (`code-verify.py --tu-census`).
- **Deps:** T19
- [ ] done

### T27 — Pure policy predicates and their test

- **Files:** `core/Devices/IO/Drivers/UART/UartPolicy.h`, `app/tests/tst_uart_policy.cpp`
- **Does:** `rtsSettable(QSerialPort::FlowControl)` (false under `HardwareControl`) and
  `sequenceFailureText(kind, index, reason, received)` shaping the one-line verdict the driver
  logs and reports; both pinned without a port.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; `tst_uart_policy` at B2.
- **Deps:** none
- [ ] done

### T28 — UART driver: the connect-time sequence and the re-run

- **Files:** `core/Devices/IO/Drivers/UART.h`, `core/Devices/IO/Drivers/UART.cpp`
- **Does:** The `connectionSequence` label-less property (value `QJsonObject`, setter accepting
  `QJsonObject` and `QVariantMap`, persisted as JSON text under `UartDriver/connectionSequence`
  for the UI instance), `connectionSequenceJson` Q_PROPERTY and `validateConnectionSequence`
  slot for QML (both on the driver, so `Cpp_IO_Serial` exposes them and
  `app/src/Misc/ModuleManager.cpp` needs no new context property); a private
  `QSerialPortSequencePort` adapter; `open()` starts the runner after
  the port opens when the sequence is non-empty and returns true; `isConnecting()` returns the
  running flag, `isOpen()` returns false while a connect-time run is active, and a private
  `portOpen()` replaces `isOpen()` in `handleError`, `onReadyRead`, `close()` and the
  reconnect poll; `onReadyRead` feeds the runner and `publishConsoleData` while running;
  `write()` returns 0 while running; `close()` cancels the runner first; `finished` →
  `reportOpenFinished(true)` then `Q_EMIT configurationChanged()`, or `logDriverError` +
  `reportOpenFinished(false, reason)`; `stepStarted` publishes `LinkSequenceNote` through
  `messageBus()`; `bytesSent` → `publishSentData`; the persisting setters apply to the port
  only when no run is active; `runSequence()` on an open port sets `m_sequenceGating` (routes
  bytes, refuses writes, keeps `isOpen()` true) and reports through a note plus a
  `NotificationRaised` warning on failure. Binding invariants, named for the edit: one verdict
  per open through the latch; `isOpen()` is the connected state and `portOpen()` the port;
  `close()` may re-enter from the failure verdict; no byte reaches `dataReceived` before the
  verdict; nothing blocks the GUI thread.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`,
  `--tu-census`; read back every former `isOpen()` call site in the file.
- **Deps:** T21, T23, T25, T26, T27
- [ ] done

**Build point B2** — the tree builds; `tst_connection_sequence`, `tst_sequence_runner`,
`tst_uart_policy` run; the maintainer can configure a sequence through the API (T29 not yet
registered, so through `project.source.setProperty`) and watch a bring-up in the console.

### Group 3 — API surface

### T29 — UART handler commands

- **Files:** `core/Api/API/Handlers/UARTHandler.h`, `core/Api/API/Handlers/UARTHandler.cpp`
- **Does:** `io.uart.setRtsEnabled` (UI driver, the DTR pattern), `io.uart.sendBreak`
  (`durationMs` 1..10000, optional `sourceId`), `io.uart.setLiveBaudRate` (`baudRate`,
  optional `sourceId`), `io.uart.getLineStates` (optional `sourceId`), `io.uart.runSequence`
  (optional `sourceId`, returns `{started}`), `io.uart.getSequence` and `io.uart.setSequence`
  (UI driver; `setSequence` validates through the model and answers `InvalidParam` with the step
  index). Live targeting: `connectionManager.driver(sourceId)` with a checked cast, refusing a
  non-UART or closed device with `ExecutionError`; a handler comment records this as the
  `io.*` convention for live-control commands. `getConfig` gains `rtsEnabled`, `liveBaudRate`,
  `sequenceStepCount`. Schemas through `API::makeSchema` / `rangeProp`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; function-length rule (split the
  registration as `registerLineSettings` does).
- **Deps:** T28
- [ ] done

### T30 — Classifications and the assistant tiers

- **Files:** `core/Api/API/CommandCapabilities.cpp`, `core/Api/API/SealedPolicy.cpp`,
  `app/rcc/ai/command_safety.json`, `app/tests/tst_command_capability_registry.cpp`
- **Does:** `sendBreak`, `setLiveBaudRate`, `runSequence` in `kCapable` as `WritesDevice`;
  `setSequence` as `{NoCapability, WritesDevice}` with an `argumentsAddCapabilities` branch
  (any Send step present) and its `argumentRefinements_data` row; `setRtsEnabled`,
  `getLineStates`, `getSequence` in `kPlain`; live-control commands in `kAllowed`,
  `getSequence` / `setSequence` in `kDenied`; `getLineStates` and `getSequence` in `safe`,
  the other five in `deviceGated`. Tables stay sorted. Decision-visible security surface:
  the classification of each name is stated in chat before the edit lands. With T29 and the
  B3 regenerations this is AC8: every new command classified, in the reference, refused to the
  assistant with device control off, reachable from a Control Loop.
- **Verify:** `code-verify.py --check`, `syntax-check.py`; the three registry ctests at B3.
- **Deps:** T29
- [ ] done

**Build point B3** — build; `SerialStudio --dump-api-schema app/rcc/api/api-schema.json`;
`python scripts/sanitize-commit.py` regenerates `SerialStudio.js`, `SerialStudio.lua`,
`sdk-symbols.json`, `proto-fields.json`, `doc/grpc/serialstudio-typed.proto` and
`search_index.json`;
`tst_command_capability_registry`, `tst_sealed_policy_registry`, `tst_assistant_batch_tiers`;
`pytest tests/integration/test_api_surfaces.py tests/scripts/test_proto_ledger_static.py`.

### T31 — Integration tests for the sequence and the line control

- **Files:** `tests/integration/test_uart_sequence.py`, `tests/integration/test_api_drivers.py`,
  `tests/README.md` (`tests/pytest.ini` read only: the `uart` and `integration` markers exist,
  no new marker)
- **Does:** New file (markers `uart`, `integration`; skips on Windows or without `socat`): the
  test owns the far pty and plays the device. Cases: connected only after the device answers
  (AC5, with a frame counter of zero during the handshake); frames flow after (AC4); no answer
  fails within the budget and the console names the expect step (AC4); the handshake bytes are
  absent from a CSV export of the session (AC6); a live baud change keeps the connection and
  leaves the saved rate alone (AC3); a project with a sequence round-trips through
  `project.source.setProperty` and a save/load (AC7). `test_api_drivers.py` adds
  `setRtsEnabled` and the new `getConfig` fields (AC1's API half). README rows for both.
- **Verify:** `pytest tests/integration/test_uart_sequence.py -v` at B4 (maintainer).
- **Deps:** T29, T30
- [ ] done

### T32 — API reference and Control Loop manual pages (delegable)

- **Files:** `doc/help/API-Reference.md`, `doc/help/Control-Loop.md`,
  `app/rcc/ai/skills/tool_discovery.md`
- **Does:** Seven command sections in the existing `#### 🟢` format with parameters and a
  `test_api.py send` example each, plus the `sourceId` convention note; a Control Loop example
  calling `io.uart.sendBreak` and `io.uart.runSequence` under the 2 s watchdog note; the UART
  group line in the discovery skill.
- **Verify:** `documentation-verify.py`, `term-verify.py`.
- **Deps:** T29
- [ ] done

### Group 4 — UI and documentation

### T33 — Setup Panel: Hold RTS and the sequence button

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml`
- **Does:** A "Hold RTS" `CheckBox` (`setup.uart.rtsEnabled`) after the DTR row, bound to
  `Cpp_IO_Serial.rtsEnabled`, disabled with a tooltip while `flowControlIndex === 1`; a
  full-width `Widgets.PushButton` "Connection Sequence…" (`setup.uart.connectionSequence`)
  with a "%1 steps" label (numbered placeholder, never `%n` with `.arg`), opening the T34
  dialog bound to `Cpp_IO_Serial`. Follows the Modbus register-groups layout.
- **Verify:** `code-verify.py --check` (QML rules, including the object-name rules of spec
  0101), `term-verify.py`; visual check at B4.
- **Deps:** T28, T34
- [ ] done

### T34 — The Connection Sequence editor dialog

- **Files:** `app/qml/ProjectEditor/Dialogs/ConnectionSequenceEditor.qml`, `app/CMakeLists.txt`
- **Does:** One `SmartDialog` for both hosts, modelled on `AlarmBandsEditor.qml`: a `ListView`
  of steps with a per-kind `Loader` (duration; line and state; data, hex, EOL and checksum;
  patterns, timeout and optional; baud; flow mode), add, remove and move buttons, the budget
  field, a validation banner fed by the host's `validateConnectionSequence(json)`, Apply and
  Cancel. Payload in and out is the sequence JSON string. Binding invariants: `PopupEnter` /
  `PopupExit` transitions; editable fields sync from the model only while unfocused and clamp
  at commit (the `TableDelegate` rule); every control carries an `objectName`. Registered in
  `QML_SOURCES`.
- **Verify:** `code-verify.py --check`, `term-verify.py`; `qt-qml-review` on the file; visual
  check at B4.
- **Deps:** T28
- [ ] done

### T35 — Project Editor host for the dialog

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/ProjectEditor.cpp`,
  `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** `openConnectionSequenceEditorForSelection()` and `commitConnectionSequence(json)`
  following the alarm-bands pair; `buildSourceModel` adds a Button row "Connection Sequence…
  (%1 steps)" for UART sources only, using `kSourceView_ConnectionSequence`; `EditorCommit`
  routes the click sentinel to the open call and the commit to
  `drv->setDriverProperty("connectionSequence", json)` then `captureSourceSettings`. Binding
  invariant: the commit opens its `ProjectUndoScope` and calls `setModified(true)`.
- **Verify:** `code-verify.py --check`, `syntax-check.py`, `term-verify.py`.
- **Deps:** T10, T28, T34
- [ ] done

### T36 — Reduce-motion and object-name sweep of the new QML (delegable)

- **Files:** `app/qml/ProjectEditor/Dialogs/ConnectionSequenceEditor.qml`,
  `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml`
- **Does:** Confirms every animation gates on `Cpp_Misc_GraphicsBackend.reduceMotion`, every
  interactive control has a literal `objectName`, and no `%n` string meets `.arg()`; fixes
  what the sweep finds.
- **Verify:** `code-verify.py --check` on both files (the two QML rules of spec 0101).
- **Deps:** T33, T34
- [ ] done

**Build point B4** — build; `pytest tests/integration/test_uart_sequence.py
tests/integration/test_fixed_length_frames.py tests/integration/test_api_drivers.py -v`;
maintainer observations: AC1 (RTS on a line LED or a second port's CTS), AC2 (BREAK on a logic
analyser, native and USB adapters, each platform), AC4 and AC6 on the test-cell adapter from
the button, the API, the CLI and after a power cycle, AC10 in the frame-parser test dialog.

### T37 — UART manual page

- **Files:** `doc/help/Drivers-UART.md`
- **Does:** Hold RTS in the settings table (with the RTS/CTS caveat); a "Connection Sequence"
  section: when a device needs one, the step kinds, expect patterns and wildcards, the budget,
  what a failure looks like in the console, the re-run; the `io.uart.*` list gains the seven
  commands and the `sourceId` note. Prose says "Device" and "Control Loop".
- **Verify:** `documentation-verify.py`, `term-verify.py`.
- **Deps:** T29
- [ ] done

### T38 — Architecture notes and the mistakes row

- **Files:** `doc/claude/architecture/io.md`, `doc/claude/common-mistakes.md`, `CLAUDE.md`
- **Does:** "Opening a Link" gains the UART sequence bullet (an async opener only when a
  sequence is set; `isOpen()` false until the verdict; bytes withheld from `dataReceived`;
  cancel in `close()`; the on-demand re-run keeps the connected state) and the Sub-objects
  section notes the `rawDataSent` tap; an IO row in common-mistakes names the `isOpen()` versus
  `portOpen()` trap; one clause in the CLAUDE.md "Driver opens are synchronous" bullet names
  UART as an async opener when a sequence is set.
- **Verify:** `claim-verify.py` on the three files (every path and symbol must resolve now
  that the code exists), `documentation-verify.py`; `claim-verify.py --context-budget --check`
  for the CLAUDE.md growth.
- **Deps:** T28
- [ ] done

### T39 — Hotpath re-read against the rules

- **Files:** `core/Pipeline/IO/FrameReader.cpp`, `core/Core/CircularBuffer.h` (read only)
- **Does:** Re-reads the two diffs against the `ss-hotpath` hard rules (SPSC, no allocation,
  capped loop, assert scope, stamping) and the counterfactual: which rule the diff most risks
  and what in the code says it does not. Fixes anything found; otherwise records the check.
- **Verify:** `code-verify.py --check` on both; the B1 benchmark numbers quoted in the done
  note.
- **Deps:** T9
- [ ] done

### T40 — C++ review pass

- **Files:** every C++ file in the plan's table (read only)
- **Does:** Runs `qt-cpp-review` on the whole C++ diff; addresses or records each finding;
  re-runs `syntax-check.py` on anything it changes.
- **Verify:** The review report; `code-verify.py --check` clean.
- **Deps:** T30, T35
- [ ] done

### T41 — Documentation and terminology sweep

- **Files:** every Markdown and JSON file touched (read only)
- **Does:** Runs `documentation-verify.py`, `term-verify.py` (all surfaces) and `claim-verify.py`
  (the AI tier); fixes findings in the new text only. This closes AC12 together with T17,
  T18, T32, T37 and T38.
- **Verify:** The three tools report nothing new against their baselines.
- **Deps:** T17, T18, T32, T37, T38
- [ ] done

### T42 — Sanitize and close

- **Files:** `doc/claude/specs/0103-serial-bringup-and-fixed-frames/spec.md`, this file
- **Does:** `python scripts/sanitize-commit.py` (regenerations, `--check-snapshot`,
  `registry-verify.py`); confirms the diff is what the plan listed and nothing else; ticks the
  acceptance criteria met in `spec.md` and sets its status to `done`; records in the done notes
  which maintainer observations remain open.
- **Verify:** `git status` shows only files in the plan's table plus the regenerated outputs;
  `sanitize-commit.py` exits 0.
- **Deps:** every task above
- [ ] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there; the ones that
  need hardware (AC1 line LED, AC2 logic analyser, AC4 and AC6 on the adapter) are recorded as
  maintainer observations with their outcome.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors);
  `--tu-census` and `--singleton-census` do not grow past their baselines.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file without a SKIP, or the
  SKIP is reported.
- [ ] `qt-cpp-review` run on the C++ diff and `qt-qml-review` on the new dialog; findings
  addressed or noted.
- [ ] `--benchmark-hotpath --min-fps 256000` not regressed (B1), and the Fixed Length run's
  parse tiers recorded.
- [ ] The three registry ctests, `tst_frame_reader_modes`, `tst_circular_buffer`,
  `tst_connection_sequence`, `tst_sequence_runner`, `tst_uart_policy`, `tst_enum_labels`,
  `tst_frame_serialization` and `tst_frame_json_legacy` pass.
- [ ] The pytest files of T16 and T31 and the surfaces tests pass for the maintainer.
- [ ] `python scripts/sanitize-commit.py` run; the regenerated SDK, ledger, proto and search
  index are in the diff; working tree clean of lint debt.
- [ ] `claim-verify.py`, `documentation-verify.py` and `term-verify.py` report nothing new.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched; the
  QSettings pollution and the `read_io_settings` signature stay as named adjacent items.
- [ ] `spec.md` status set to `done`.
