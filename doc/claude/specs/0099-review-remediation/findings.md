---
spec: 0099-review-remediation
phase: findings
updated: 2026-10-14
---

# Findings 0099 — the review ledger

> Every id from the 2026-10-13 review outside the consent class (spec 0098). Status is
> `open` until a fix lands, then `fixed` (with its verification) or `deferred` (with a
> reason). File:line is where the review found it, at db2debec4; re-confirm at HEAD before
> fixing. One row per id.


## Theme A — Pipeline and buffer correctness

### pipeline-core:f5 (high) — status: deferred (benchmark-gated)
- **At:** `core/Pipeline/DataModel/FrameBuilder.cpp:1784`
- **Defect:** Span lane never writes parsed values back to the per-source template frames, yet republishFrames() stages those templates, pushing stale or initial values for every parsed dataset to the dashboard
- **Fix (DEFERRED to a maintainer-run, benchmark-gated task — re-confirmed 2026-10-14):** the span fast lane writes values only into the pooled slot, never into the source template that `republishFrames` iterates (the list/decode lane DOES update the template via `applyDatasetValues`; the asymmetry is the bug). A code recon this pass confirmed there is NO off-hotpath option: there is no per-source "last parsed values" buffer to read (pool slots recycle by use_count and are not indexed by source), so BOTH candidate fixes land on the 256 kHz-gated per-frame path — (a) a second structured value-apply into the template on the span lane, or (b) adding+writing a per-source last-values buffer per frame. Either must be measured against `--benchmark-hotpath` (the datasets+publish stage is 70-80% of per-frame time), which cannot be done in this environment. Correctness bug, narrow trigger (republish — dashboard.tick / reprocess / stream refresh — of a span-lane source with a computed or table-driven dataset).
- **Verify (when landed):** a ctest where dashboard.tick republishes the just-parsed values, not the template; `--benchmark-hotpath` within gate decides (a) vs (b).

### pipeline-core:f4 (high) — status: fixed
- **At:** `core/Pipeline/DataModel/DataTable.cpp:267`
- **Defect:** Interned-key lookup cache is keyed by raw Lua string addresses without holding the strings, so a garbage-collected name whose address is reused resolves to another register's slot
- **Fix:** Clear the interned-key cache whenever the engine runs its 1 Hz GC (and on reload): LuaJIT frees a GCstr only during a sweep, so clearing on GC means no freed address can survive to be reused. Cache read stays per-call; the clear is on the 1 Hz path, not per-frame.
- **Verify:** ctest: a built-name lookup across a forced GC resolves correctly; not per-frame, so no hotpath cost.

### pipeline-core:f3 (high) — status: fixed
- **At:** `core/Pipeline/IO/StreamWorker.cpp:458`
- **Defect:** teardownEngines() can run inside the nested event loop of a stream transform's table-API marshal, closing the lua_State or deleting the QJSEngine under the running script
- **Fix:** Do not run teardownEngines inside a table-API marshal nested loop: guard teardown behind an in-flight/marshal-depth check so the lua_State / QJSEngine is destroyed only when no marshal is on the worker stack.
- **Verify:** sanitizer-tier ctest driving a stream transform that marshals during stop(); clean under TSan.

### pipeline-core:f1 (medium) — status: fixed
- **At:** `core/Pipeline/IO/FrameReader.cpp:143`
- **Defect:** Overflow path drops only `overflow` bytes from m_pendingChunks, but the ring now reads as empty: per-chunk timing desyncs by the full ring capacity and grows without bound
- **Fix:** Align discardPendingBytes with what the ring overwrote (folded into the f2 free-running-index change), so per-chunk timing stays aligned after an overflow.
- **Verify:** ctest alongside f2; a fed overrun keeps the pending-chunk offsets consistent.

### pipeline-core:f2 (medium) — status: fixed
- **At:** `core/Core/CircularBuffer.h:231`
- **Defect:** Ring treats full and empty the same: an append that fills the ring exactly, or any chunk at least as big as the ring, leaves size()==0 and silently drops every retained byte
- **Fix (landed — DEVIATION from the plan's option a):** the plan proposed free-running unmasked head/tail. I instead reserve one slot in `append()` (`usable = m_capacity - 1`) so full never aliases empty. Reason: a free-running-index rewrite spans ~10 accessors I cannot run tst_circular_buffer / the hotpath benchmark against here; reserve-one is confined to `append()`, rippleless (`freeSpace()` has no external callers, `capacity()` is only used for mlock sizing), and SPSC/no-lock. f1 is closed by the same change (FrameReader already consumes overflowCount). Arithmetic hand-verified; the maintainer runs ctest.
- **Verify:** ctest:tst_circular_buffer — updated `overflowingAppendCountsTheLostBytes` (overflow 2, size 7, read "34567ab") and `oversizedAppendKeepsTheTailOfTheChunk` (overflow 13, size 7), plus new regression `exactlyFullChunkIsNotDroppedAsEmpty` (append 8 to an 8-ring → size 7, overflow 1, read "BCDEFGH"; fails on db2debec4). Maintainer-run.

### pipeline-core:f7 (medium) — status: fixed
- **At:** `core/Pipeline/DataModel/FrameBuilder.cpp:787`
- **Defect:** syncFromProjectModel() drops, not defers, a sync requested while another is in flight, so the pipeline keeps a stale project
- **Fix:** Coalesce a dropped sync: when a sync is already in flight, record a pending request and re-run one more sync after the in-flight apply completes, so the pipeline never keeps a stale project.
- **Verify:** ctest: two rapid syncs leave the pipeline on the latest project.

### pipeline-core:f6 (medium) — status: fixed
- **At:** `core/Pipeline/DataModel/Scripting/FrameParser.cpp:831`
- **Defect:** GUI-thread FrameParser entry points read and mutate pipeline-owned engine and language state without marshaling: a data race, plus a stale language decision
- **Fix:** Route the GUI-thread FrameParser entry points through the existing marshal instead of reading/mutating pipeline-owned engine and language state directly; a language change is applied on the owning thread.
- **Verify:** ctest / TSan: concurrent GUI read + pipeline parse shows no race and no stale language.

### pipeline-core:f8 (medium) — status: fixed
- **At:** `core/Pipeline/DataModel/Scripting/ScriptApiCall.cpp:340`
- **Defect:** The 'parked' flag is cleared by the pipeline only after it resumes, so GUI-side marshals briefly run inline against a running pipeline and a deferred project snapshot can be stranded
- **Fix:** Order the parked-flag set/clear around the BlockingQueuedConnection so there is no window where the pipeline runs while flagged parked (or clears the flag before it has resumed); a deferred sync waits correctly.
- **Verify:** TSan-tier ctest exercising a parked marshal racing a GUI sync.


## Theme B — Device and protocol drivers

### devices-protocols:f1 (critical) — status: fixed
- **At:** `core/Devices/IO/Drivers/Modbus.cpp:461`
- **Defect:** Modbus connectClient dereferences m_device after a re-entrant failDial() nulled it (crash when an RTU port cannot be opened)
- **Fix:** Guard the second branch of connectClient so a re-entrant failDial (which doClose() already nulled m_device through) is not dereferenced: `if (!m_device->connectDevice() && m_device) failDial(...)`. The re-entrant failDial already reported with the right error and the m_connecting latch makes it a no-op, so the guard only removes the deref. GUI thread, synchronous signal. Commercial-only (RTU path).
- **Verify:** ctest/observation: a busy or unplugged RTU port reports a clean connect failure and does not crash.

### devices-protocols:f4 (medium) — status: fixed
- **At:** `core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.cpp:212`
- **Defect:** Serial-CAN open-ack check never sees the adapter's reply: readyRead inside waitForReadyRead drains it first
- **Fix:** Wire the readyRead connection AFTER the open-ack block, so waitForReadyRead sees the adapter reply instead of the slot draining it first. Synchronous open; not hotpath. Commercial-only.
- **Verify:** observation on a serial-CAN adapter: open ack is seen and a bad ack reports an error.

### devices-protocols:f2 (medium) — status: deferred (needs a cross-layer drain API)
- **At:** `core/Protocols/FileTransfer/ZMODEM.cpp:489`
- **Defect:** ZMODEM sender queues the whole file at once with no backpressure; ZEOF timeout runs from queue time, so large files fail on slow links
- **Fix (DEFERRED — re-confirmed 2026-10-14):** pace the next data chunk on the device bytesWritten/bytesToWrite instead of the `singleShot(0)` fire-hose, and defer the ZEOF timeout until the sink has drained. A recon this pass confirmed the Protocol layer has NO backpressure feedback: `Protocol` exposes only an outbound `writeRequested(QByteArray)` signal, and `FileTransmission::onProtocolWriteRequested` is fire-and-forget into `ConnectionManager::writeData` — the driver's `bytesWritten`/`bytesToWrite` are never surfaced back. So the fix REQUIRES a new cross-layer drain API (ConnectionManager -> FileTransmission -> Protocol) plus gating `sendNextDataChunk`/the ZEOF timer on it: a multi-file wire-shape change with `tst_zmodem` regression risk, unverifiable without a throttled link. Out of proportion to a remediation pass and not landable blind.
- **Verify:** observation: a large file transfers over a throttled link without a spurious ZEOF timeout.

### devices-protocols:f3 (medium) — status: fixed
- **At:** `core/Devices/IO/Drivers/CANBus.cpp:846`
- **Defect:** CAN error frames are published as ordinary data frames on ID 0x000
- **Fix:** In onFramesReceived, skip any frame whose frameType() is not DataFrame (count it, optionally throttled-log an ErrorFrame) instead of serializing it as data under its error-mask id. On the ingest hotpath (GUI thread), so a counter + branch only.
- **Verify:** observation: a CAN error frame is surfaced as an error/counter, not a data frame.

### devices-protocols:f5 (medium) — status: fixed
- **At:** `core/Devices/IO/Drivers/S7.cpp:572`
- **Defect:** Cancelling an S7 / EtherNet/IP dial blocks the GUI thread until the worker's blocking dial times out
- **Fix (landed):** three waits now re-check the existing abort atomic (`PolledPlcWorkerBase::m_abort`, relaxed). S7 `readTpdu` `waitForReadyRead` and EtherNet/IP `plc_tag_create` are sliced (50 ms / 5 ms). The S7 TCP connect could NOT be sliced — Qt documents that `waitForConnected` aborts the connect on each timed-out call and does not accumulate time, so slicing would kill any connect slower than one slice (the powered-off-PLC cancel case). It now runs on a nested `QEventLoop` (`awaitConnectedSliced`): quit on connected/errorOccurred, a deadline QTimer, a `kS7WaitSliceMs` abort tick. The nested loop is REQUIRED — it services `doClose()`'s `BlockingQueuedConnection` shutdown() (which deletes `m_socket` mid-loop, so the socket is held by a QPointer and never touched after the loop if null), fixing the GUI freeze at its root. This is the Qt-documented pattern (its `waitForConnected` doc recommends the event loop + connected() signal).
- **Deliberately NOT sliced:** the EtherNet/IP steady-state `plc_tag_read` stays blocking — `pollTick` already checks the abort atomic between tags (bounded ~1.5 s), and polling every steady-state read would add latency to the hot poll path to cover a rare case.
- **Verify (MAINTAINER, needs a running app / reachable+unreachable PLC):** cancelling an S7 connect to a powered-off host returns promptly instead of freezing to the 5 s deadline; a normal connect and a genuine timeout still behave as before; no crash on cancel mid-connect (the QPointer/re-entrant-delete path). The event-loop re-entrancy cannot be exercised in this environment.

### devices-protocols:f6 (medium) — status: deferred (not a live bug)
- **At:** `core/Devices/IO/Drivers/CANBus/GsUsbCanBackend.cpp:801`
- **Defect:** gs_usb RX reassembly assumes a fixed frame size per mode; variable-length host frames (classic frames or echoes in FD mode) desync the stream
- **Fix:** DEFERRED — not a live bug at HEAD. The fixed RX stride is set once per session from the FD flag, and the one variable-length case, a trailing HW timestamp, needs GS_CAN_MODE_HW_TIMESTAMP, which this backend never enables; partial transfers are already handled by m_rxCarry. A recon this pass confirmed an echoId resync guard is viable (a record whose echoId is neither 0xffffffff nor a pending-TX echo is a mis-frame), BUT it would sit on the RX per-frame path and a timing race against `m_pendingTx` could misclassify a valid echo as desync and clear the carry, dropping real frames — speculative hardening with its own risk, untestable without hardware, for a condition that does not occur as configured. Left deferred rather than add that risk to a non-bug.
- **Verify:** n/a (deferred): no reproduction as configured.

### devices-protocols:f8 (low) — status: fixed
- **At:** `core/Protocols/FileTransfer/ZMODEM.cpp:259`
- **Defect:** ZMODEM accepts receiver headers without CRC verification (binary headers never checked; hex check skipped on malformed CRC digits)
- **Fix:** Make the ZMODEM hex path drop a header whose CRC field is malformed (not fall through), and add CRC verification to the two binary header paths (bin16/bin32), which currently call parseReceivedHeader with no CRC check at all. GPL build included.
- **Verify:** unit: a header with a bad CRC is rejected on the hex and both binary paths.

### devices-protocols:f7 (low) — status: fixed
- **At:** `core/Devices/IO/Drivers/Iec104.cpp:692`
- **Defect:** IEC 104 acknowledges received I-frames only on the 100 ms tick, capping a station at k APDUs per tick
- **Fix:** After the receive-drain loop, send a supervisory S-frame when an ack is due rather than only on the 100 ms tick (keep the tick as the t2 fallback), so a station is not capped at k APDUs per tick. GUI ingest path. Commercial-only.
- **Verify:** observation: a station sustains more than k APDUs across a 100 ms window.

### devices-protocols:f9 (low) — status: fixed
- **At:** `core/Protocols/CAN/GsUsbProtocol.h:262`
- **Defect:** solveBitTiming calls std::clamp with lo > hi when firmware reports tseg2Max < max(1, tseg2Min), which aborts under _GLIBCXX_ASSERTIONS
- **Fix:** In the early bit-timing validation, reject degenerate firmware limits (`tseg2Max < max(1,tseg2Min)` or `tseg1Max < tseg1Min`) before the clamp, so std::clamp is never called with lo>hi. Pure inline function. Commercial/tests.
- **Verify:** unit: a degenerate limit set returns false and does not abort under _GLIBCXX_ASSERTIONS.

### devices-protocols:f10 (low) — status: fixed
- **At:** `core/Devices/IO/Drivers/Audio/PlaybackRing.h:158`
- **Defect:** Playback underrun pads with byte 0, which is full-scale negative (not silence) for unsigned 8-bit output
- **Fix:** Store a silence byte from the playback format at device init (0x80 for unsigned 8-bit, else 0) and memset underruns with it instead of 0, so an underrun is true silence for every format. Called from the miniaudio render callback (hotpath), but a stored-byte memset is free. Commercial-only.
- **Verify:** observation: an underrun on an unsigned-8-bit output is silent, not full-scale.


## Theme C — Recording, export and session I/O

### api-storage:f6 (high) — status: fixed
- **At:** `core/Storage/CSV/Export.cpp:577`
- **Defect:** CSV recording never reopens after pause/resume: the template is cleared on close and never re-pushed
- **Fix:** CSV is the only recording path with this bug (MDF4 already re-pushes via refreshTemplateFrame on the live boundary; Sessions never clears its structure). Mirror MDF4: cache the sessionStructureReady frame in a CSV::Export member and add the live-boundary else-branch that re-pushes it via a queued setTemplateFrame. FIFO on the single CSV worker thread guarantees clear-then-repopulate-then-reopen.
- **Verify:** integration: a pause/resume on a CSV recording produces a file with the post-resume blocks.

### api-storage:f10 (medium) — status: fixed
- **At:** `core/Storage/Sessions/DatabaseWorker.cpp:881`
- **Defect:** Historian 'Export Session to CSV' writes headers and text values without CSV escaping and ignores write errors
- **Fix:** Promote the existing CSV escapeCsvField to a public helper and wrap every Historian CSV header label and cell; after the write loop, flush and check out.status()/file.error() and report csvExportFinished(false,...) on failure instead of always true. Worker thread; helper is pure. Commercial.
- **Verify:** integration: a session with delimiter/quote/newline characters round-trips through a CSV reader; a write error is reported.

### api-storage:f12 (medium) — status: fixed
- **At:** `core/Storage/CSV/Player.cpp:446`
- **Defect:** CSV player frame stepping uses std::max(1, ...): row 0 is never backfilled and stepping back to row 0 trips SS_ASSERT
- **Fix:** Change both std::max(1, ...) to std::max(0, ...) in the CSV player step path (the seek path already uses qMax(0,...)), so row 0 is backfilled and stepping back to it does not pass startFrame>endFrame. GUI thread; off-by-one. Release silently drops row 0, debug asserts.
- **Verify:** observation/ctest: the CSV player steps to and from row 0 without an assertion, and row 0 renders.

### api-storage:f11 (medium) — status: fixed
- **At:** `core/Storage/Sessions/Export.cpp:1486`
- **Defect:** The queued open notification resets the write-failure latch after the worker has already set it
- **Fix:** Clear the write-failure latch on the worker thread inside createDatabase (before the first transaction), and drop that reset from the GUI resetSessionHealthBaseline (keep only its GUI-owned link-loss baseline), so a queued open notification cannot clear a failure the worker already latched. Set and clear both on the worker thread. Commercial.
- **Verify:** observation: a forced write error during recording stays reported.


## Theme D — API server robustness

### api-storage:f5 (high) — status: fixed
- **At:** `core/Api/API/Server/ServerWorker.cpp:327`
- **Defect:** stream.subscribe is acked as soon as the write is queued, so a slow or stalled subscriber grows the socket buffer without limit
- **Fix:** Ack the stream lane on DRAIN, not enqueue: in writeStreamBlock emit streamWriteDone only when bytesToWrite()==0 (via QAbstractSocket::bytesWritten, or immediately if already drained), and add an underWriteCap guard like the mirror lane, so a non-reading subscriber is capped/dropped instead of growing the socket buffer without bound. Must still ack on disconnect/error/session-mismatch so the in-flight latch never wedges. ServerWorker thread. Dual-licensed (ships in GPL).
- **Verify:** integration: a subscriber that stops reading does not grow server memory without bound and is dropped/capped.

### api-storage:f8 (medium) — status: fixed
- **At:** `core/Api/API/Server/ClientReception.cpp:684`
- **Defect:** Newline-less JSON is judged complete by its last byte; a TCP split after an inner '}' drops the command and sends the tail to the device as raw bytes
- **Fix:** Replace the first+last-byte completeness heuristic with an incremental, string/escape-aware brace/bracket-depth counter kept per connection; parse only at depth 0, and never clear the buffer or fall to raw forwarding while the top-level structure is still open. Avoids the quadratic re-parse. GUI thread; counter beside the per-connection buffer.
- **Verify:** integration: a command split across two TCP writes is reassembled and never reaches the device as raw bytes.

### api-storage:f9 (medium) — status: fixed
- **At:** `core/Api/API/Server/ServerWorker.cpp:161`
- **Defect:** closeResources edits m_sockets while iterating it (abort() emits disconnected synchronously), so sockets can be skipped and orphaned
- **Fix:** Iterate a snapshot of the socket keys (or disconnect the disconnected signal first) before abort(), so the synchronous disconnected -> removeSocket does not mutate the hash mid-iteration. Worker thread.
- **Verify:** integration/ctest: tearing down many connections orphans no socket.

### api-storage:f14 (low) — status: fixed
- **At:** `core/Api/API/Server.cpp:287`
- **Defect:** Bulk connection teardown skips per-connection cleanup (MCP sessions, stream flag, stale mirror subscriber)
- **Fix:** Factor onSocketDisconnected body into teardownConnection(it) and call it for each entry in dropConnections before clear(), then refreshStreamSubscriberFlag() and one streamSubscribersChanged, so bulk teardown runs the same per-connection cleanup (MCP session, stream flag; mirror sub is already bulk-cleared only when linked) as a single disconnect. GUI thread. Ties to f9.
- **Verify:** integration: a bulk teardown leaves no leaked MCP session and the stream-subscriber flag is correct.

### api-storage:f13 (low) — status: fixed
- **At:** `core/Api/API/CommandRegistry.cpp:352`
- **Defect:** A stray dryRun:true on a command that ignores dryRun skips the checkpoint, autosave and FrameBuilder re-sync while the change still happens
- **Fix (landed — DEVIATION on mechanism):** compute `isDryRun = commandHonorsDryRun(name) && params.dryRun`, where the new private `commandHonorsDryRun` derives the flag from the command's own `inputSchema.properties.dryRun` rather than a hand-set descriptor field. Reason: the schema is where a command declares its params, so it cannot drift from the handler, and the fix stays inside `CommandRegistry.{h,cpp}` instead of touching ~15 handler registration files (each a chance to miss one and re-introduce the bug). SAFETY CONFIRMED: execute()'s isDryRun gates only the pre-checkpoint and post-autosave/re-sync; it is NOT passed to the handler, whose own `params["dryRun"]` preview logic is untouched, so a schema-conservative miss can at worst take a harmless spurious checkpoint, never cause a wrong mutation. GUI thread; control-flow only. Protected API surface.
- **Verify:** ctest/integration: a command that ignores dryRun, called with dryRun:true, still checkpoints and re-syncs. Maintainer-run.


## Theme E — App services

### ui-app:f2 (high) — status: fixed
- **At:** `core/Ui/Misc/BackupManager.cpp:244`
- **Defect:** Backup list is sorted oldest first (QDir::Time|Reversed), so recovery and checkpoint restore pick the wrong snapshot
- **Fix:** Sort the backup list newest-first by the ISO-timestamp filename (`QDir::Name | QDir::Reversed`, as enforceRetention already does), immune to mtime rewrites. Same change at the dedup seed (`seedDedupFromNewest`, a sibling of this bug, same root cause). Sequenced after spec 0098, which also edits BackupManager. Pure sort-flag change on a const method.
- **Verify:** ctest or observation: list() returns newest first; BackupRecovery preselects the latest; assistant.listCheckpoints returns the most recent N; dedup seeds off the newest.

### ui-app:f3 (medium) — status: fixed
- **At:** `core/Ui/Misc/ExtensionManager.cpp:574`
- **Defect:** Uninstall builds a recursive-delete path from the unvalidated catalog 'type' instead of the recorded installed type
- **Fix (landed):** Uninstall reads the RECORDED installed type (`installedInfo(id).type`, as removeAll does), never the unvalidated catalog field, so the recursive delete path cannot be steered by catalog content. Defense-in-depth added: the installer re-checks `isSafePathComponent` on id and type before `removeRecursively`, and `isSafePathComponent` is now hardened to also reject `""` and `"."` (previously only `..`/separators), so neither an empty nor a dot component can resolve the delete to the parent directory. GUI thread.
- **Verify:** observation / unit: uninstall removes only the recorded install dir; a catalog with a hostile type does not reach removeRecursively.

### ui-app:f6 (medium) — status: fixed
- **At:** `core/Ui/UI/Dashboard.cpp:1112`
- **Defect:** Display-tick reconfigure spins a nested event loop mid-teardown, and the marshal's result is thrown away
- **Fix:** Gate the mid-teardown blocking marshal in resetData on `notify`: the reconfigure caller (notify==false) re-runs configureActions with the real frame afterward, so the nested-loop marshal is pure waste and a reentrancy hazard there; standalone resets (notify==true) keep it. Removes a nested event loop from the display tick. Dashboard is GUI-only / hotpath-adjacent.
- **Verify:** observation: reconfigure no longer spins a nested loop on the tick; actions still configure correctly; no reentrancy under a live stream.

### ui-app:f4 (low) — status: fixed
- **At:** `core/Ui/Console/Export.cpp:158`
- **Defect:** Console log folder uses the raw project title (no sanitizing) and reads GUI-owned state from the export worker thread
- **Fix:** Snapshot the operation mode and the SANITIZED console-log subdir on the GUI thread (via WorkspaceManager::sanitizeName) and hand them to the export worker, so the worker reads no GUI-owned state and the folder name is safe. Commercial-only.
- **Verify:** TSan-clean; a project title with path characters yields a safe folder.

### ui-app:f5 (low) — status: fixed
- **At:** `core/Ui/AI/FileSandbox.cpp:137`
- **Defect:** Off-thread fs.read/fs.search read WorkspaceManager::m_path, which the GUI thread can reassign
- **Fix:** Snapshot the sandbox root on the GUI thread before the fs.read/fs.search worker runs (capture workspaceRoot() into the task), or cache it in FileSandbox under its existing mutex, so the worker never reads WorkspaceManager::m_path while the GUI reassigns it. Commercial-only.
- **Verify:** TSan-clean under a path reassign racing an fs tool.


## Theme F — Editor and dialog correctness (QML)

### qml:f1 (high) — status: fixed
- **At:** `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml:908`
- **Defect:** Non-modal Alarm Bands / Frequency Markers editors commit to whatever dataset is selected when Apply is clicked
- **Fix (landed):** the commit is keyed to the dataset the dialog was opened for, not the global selection. commitAlarmBands/commitFrequencyMarkers take (groupId, datasetId, ...) and the editor captures the open-time **stable uniqueId** (`ProjectEditor::m_{alarmBands,freqMarkers}DialogUniqueId`, set by EditorForms at open), which `resolveDialogDataset` maps to the dataset's CURRENT positional ids at Apply — so a sibling reorder/delete between open and Apply cannot retarget a neighbour, and a removed dataset commits nothing (review follow-up). The batch route fires only for the multi dialog (-1,-1); a non-selected single target restores the model selection after `updateDataset` re-seats it (so a later delete/duplicate still acts on what the editor shows). On-Apply focus flush (f8) and locale-aware decimal parse (f2) are in the same two editors.
- **Verify:** observation: open editor for dataset A, select B, Apply writes to A; delete a sibling above A while the editor is open, Apply still writes to A.

### qml:f2 (high) — status: fixed
- **At:** `app/qml/Dialogs/TriggerDialog.qml:236`
- **Defect:** Locale-aware DoubleValidator paired with C-locale parseFloat/Number truncates or drops decimal input in comma-decimal locales
- **Fix:** ROOT A (5 dialogs). Parse decimals the way they are validated: replace each C-locale parseFloat/Number(text) with Number.fromLocaleString(Qt.locale(), text) (the repo idiom at TableDelegate.qml:857), guarded for bad input. Covers TriggerDialog, AxisRangeDialog, AccelerometerConfigDialog, AlarmBandsEditor, FrequencyMarkersEditor. A one-line Utilities helper prevents recurrence.
- **Verify:** observation in a comma-decimal locale: a fractional value round-trips in each dialog.

### qml:f4 (medium) — status: fixed
- **At:** `app/qml/Dialogs/WorkspaceDialog.qml:266`
- **Defect:** Clicking a widget's label or icon kills the checkbox binding, so later workspace dialogs show stale checkmarks
- **Fix:** The icon/label MouseArea toggles the model the way the checkbox onClicked does (toggle map[wid], reassign checkedWidgets) instead of assigning _checkbox.checked, which destroyed the binding and left recycled delegates stale.
- **Verify:** observation: toggling via the label keeps later dialog checkmarks correct.

### qml:f6 (medium) — status: fixed
- **At:** `app/qml/ProjectEditor/Views/UserTableView.qml:443`
- **Defect:** Numeric variable default value commits NaN or a truncated number on every keystroke
- **Fix:** ROOT D. Move the numeric variable-default commit from onTextEdited to onEditingFinished and guard isNaN/empty, so a partial or empty entry never commits NaN or 0 per keystroke.
- **Verify:** observation: typing a partial number commits nothing until the field is complete.

### qml:f7 (medium) — status: fixed
- **At:** `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml:198`
- **Defect:** Annotation CSV export builds an invalid path on Windows and always fails
- **Fix:** ROOT E. Pass the URL to C++ and resolve with QUrl::toLocalFile (make the export invokable take a QUrl) instead of substring(7), so the annotation CSV path is valid on Windows.
- **Verify:** observation on Windows: annotation CSV export writes a file.

### qml:f3 (medium) — status: fixed
- **At:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml:590`
- **Defect:** Tree context-menu flavour comes from the current group, not the right-clicked row
- **Fix:** Compute the context-menu flavour from the right-clicked row id, not the global current group: add a per-id query (Q_INVOKABLE groupIsPainter(int)/groupIsOutputPanel(int)) or a tree role, read from itemId.
- **Verify:** observation: right-click a non-selected group row shows the menu for that row.

### qml:f5 (medium) — status: fixed
- **At:** `app/qml/DatabaseExplorer/SessionDetail.qml:401`
- **Defect:** Historian session notes being typed are discarded when another session is selected or the list refreshes
- **Fix:** Commit Historian session notes continuously while editing (onTextChanged guarded by activeFocus, Qt.callLater(flush)), or flush the outgoing notes in the list row onClicked before selectSession, so a selection change or refresh does not discard the edit.
- **Verify:** observation: type a note, click another session, the note is saved.

### qml:f8 (medium) — status: fixed
- **At:** `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml:565`
- **Defect:** Editor tables commit only on editingFinished, so the last field edit is lost when Apply is clicked on macOS
- **Fix:** ROOT B (2 editors). On Apply, steal focus first (forceActiveFocus on the Apply button) so the focused field fires editingFinished synchronously before collectBands/collectMarkers reads the model.
- **Verify:** observation on macOS: the last field edit is kept on Apply.


## Theme G — QML polish and platform

### qml:f10 (low) — status: fixed
- **At:** `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml:442`
- **Defect:** Reduce Motion preference ignored by several slide and scale animations
- **Fix:** ROOT C. In onActiveGroupIdChanged, when reduceMotion snap the translate/opacity to the final state and skip the slide restart. The slide is a Translate+opacity (compliant); it only needs the gate.
- **Verify:** observation: Reduce Motion removes the group-switch slide.

### qml:f11 (low) — status: fixed
- **At:** `app/qml/Widgets/Dashboard/Terminal.qml:891`
- **Defect:** Hex-mode validation overwrites the send field's themed text colour permanently
- **Fix:** Bind the send field text colour to a sendTextValid bool set in onTextChanged, and delete the imperative palette.text assignment, so hex-mode validation never permanently overwrites the themed colour.
- **Verify:** observation: after a hex keystroke the send field colour still tracks the theme.

### qml:f9 (low) — status: fixed
- **At:** `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml:440`
- **Defect:** Taskbar autohide and toolbar collapse animate laid-out heights, re-laying out the dashboard every frame, with no Reduce Motion gate
- **Fix:** ROOT C. Gate the taskbar-host height Behavior on !Cpp_Misc_GraphicsBackend.reduceMotion, and (proper fix) animate an inner Translate/clip rather than Layout.preferredHeight so the dashboard does not re-lay-out every frame.
- **Verify:** observation: Reduce Motion stops the autohide animation; no per-frame relayout.

### qml:f12 (low) — status: fixed
- **At:** `app/qml/ProjectEditor/Views/TableDelegate.qml:938`
- **Defect:** Clearing a float field silently commits 0
- **Fix:** Add an empty-string (trimmed) case to the float editor early-return at TableDelegate.qml:935 so clearing the field commits nothing rather than Number of empty being 0.
- **Verify:** observation: clearing a float field leaves it unset, not 0.

### qml:f13 (low) — status: fixed
- **At:** `app/qml/ProjectEditor/Views/UserTableView.qml:408`
- **Defect:** Variable rename commits per keystroke; a transient name collision rebuilds the list and drops the in-progress edit
- **Fix:** ROOT D. Move the variable-rename commit to onEditingFinished so a transient name collision does not rebuild the list and drop the in-progress edit mid-typing.
- **Verify:** observation: renaming through a transient collision keeps the edit.

### qml:f14 (low) — status: fixed
- **At:** `app/qml/Dialogs/ExtensionManager/GridPage.qml:224`
- **Defect:** Local-repository extension screenshots and icons use a malformed file URL on Windows
- **Fix (landed):** ROOT E. QML builds a well-formed URL prefix (`base.startsWith("/") ? "file://" : "file:///"`) so a Windows drive-letter path gets the three-slash form instead of a malformed `file://C:/...`. Applied to GridPage.qml:224 AND the identical sibling DetailPage.qml:235 (same ROOT E class, flagged during review, fixed here for parity).
- **Verify:** observation on Windows: local-repo screenshots/icons render.

### qml:f15 (low) — status: fixed
- **At:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml:291`
- **Defect:** Tag search hides rows by changing delegate implicitHeight, which TreeView does not re-measure
- **Fix:** Call _tree.forceLayout() when the tag-search text changes (minimal), or filter in a QSortFilterProxyModel (correct), so TreeView re-measures and hidden rows leave no blank gaps.
- **Verify:** observation: OPC UA tag search leaves no blank rows.

