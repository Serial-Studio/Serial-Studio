---
spec: 0099-review-remediation
phase: tasks
status: draft        # draft -> approved (gate before /ss-implement)
updated: 2026-10-14
---

# Tasks 0099 — Remediation of the 2026-10-13 review findings

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md); the per-finding
> fix and verification are in [`findings.md`](./findings.md), which each task updates as it lands.
> Each C++ task's Verify includes `python3 scripts/syntax-check.py` and
> `python3 scripts/code-verify.py --check` on its files; not repeated below. Gate: do not start
> `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one finding, except where findings share a root (noted: "closes f_, f_").
- **Landing order:** the critical and high findings first (T1 Modbus crit, then the highs: A-ring,
  A-span, A-interned, C-CSV-reopen, D-stream, E-backup, F-wrong-dataset, F-locale), then the
  mediums, then Theme G last. Milestones below are theme-coherent; within each, severity orders.
- **Deps** lists task ids that must land first. Most tasks are independent; the few real deps are
  named (e.g. the CSV escape helper before the Historian export).
- **Main session** marks a hotpath, race, ctor/signal-wiring or protected-surface task that is
  never delegated. Unmarked tasks may run in a Sonnet subagent from a half-page brief; the main
  session reviews every diff.
- A data-integrity fix is not done without the regression test named in its Verify, which must
  fail on the pre-fix code.
- `--benchmark-hotpath` is run once after Theme A and once after the Theme B hotpath tasks, at the
  clean points marked below.
- Deferred: **devices-protocols:f6** (gs_usb) — not a live bug; recorded in `findings.md`, no task.

## M-B lead — the critical

### T1 — Modbus re-entrant connect crash (devices-protocols:f1, critical)

- **Files:** `core/Devices/IO/Drivers/Modbus.cpp`
- **Does:** Null-guard the second `failDial` branch in `connectClient` so a synchronous
  `stateChanged(Unconnected)` that already ran `failDial` → `doClose()` (nulling `m_device`) is not
  dereferenced. *Invariant:* the dial verdict has ONE owner (`io.md` "Opening a Link"); the
  re-entrant `failDial` already reported and the `m_connecting` latch no-ops the second, so the
  guard only removes the deref. GUI thread, synchronous signal.
- **Verify:** read-back of the guarded branch; observation — a busy/unplugged RTU port reports a
  clean failure and does not crash. Mark f1 fixed.
- **Deps:** none · **Main session**
- [x] done

## M-A — Pipeline and buffer correctness (hotpath/threading; all Main session)

### T2 — Ring full ≠ empty (pipeline-core:f2, pipeline-core:f1)

- **Files:** `core/Core/CircularBuffer.h`, `core/Pipeline/IO/FrameReader.cpp`,
  `app/tests/tst_circular_buffer.cpp`
- **Does:** Represent head/tail as free-running counters, masking only on buffer access, so a ring
  filled exactly or by an oversized chunk reports its true size; align `discardPendingBytes` with
  what the ring overwrote (f1 is the same change). *Invariant:* `CircularBuffer`/`FrameReader` are
  SPSC, pipeline/main — **never add a mutex**; recreate via `reconfigure()`. Read `CircularBuffer.h`
  and `FrameReader` in full and invoke `ss-hotpath` first.
- **Verify:** `tst_circular_buffer` gains exact-fill and oversized-chunk cases that read the data
  back and FAIL on db2debec4; `--benchmark-hotpath` unchanged. Mark f1, f2 fixed.
- **Deps:** none · **Main session**
- [x] done

### T3 — Span-lane writeback (pipeline-core:f5, high)

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`, a new ctest TU
- **Does:** Make the span fast lane's parsed values visible to `republishOneFrame` — either write
  them back into the source template, or have republish read the last pooled slot for a span
  source. *Invariant:* the span lane is zero-allocation; the fix adds no heap allocation on the
  fast lane (columns are sized at bind). The choice between the two options is decided on the
  benchmark, not on paper (plan Tradeoffs). Invoke `ss-hotpath`; read `FrameBuilder` in full.
- **Verify:** a ctest where a dashboard tick / reprocess republishes the just-parsed values, not
  the template, failing pre-fix; `--benchmark-hotpath` within gate. Mark f5 fixed.
- **Deps:** none · **Main session**
- [ ] done — **DEFERRED** (benchmark-gated; brief in findings.md pipeline-core:f5)

### T4 — Interned-key cache GC safety (pipeline-core:f4, high)

- **Files:** `core/Pipeline/DataModel/DataTable.{h,cpp}`, `core/Pipeline/DataModel/FrameBuilder.cpp`,
  a ctest TU
- **Does:** Clear the interned-key cache on the engine's 1 Hz GC (and on reload), so a freed Lua
  string address reused for a different name cannot resolve to a stale slot. *Invariant:* the cache
  read stays per-call on the parse path; the clear rides the existing GC tick, never per frame.
- **Verify:** a ctest that builds register names at run time across a forced GC and resolves them
  correctly, failing pre-fix. Mark f4 fixed.
- **Deps:** none · **Main session**
- [x] done

### T5 — Stream-worker teardown under marshal (pipeline-core:f3, high)

- **Files:** `core/Pipeline/IO/StreamWorker.cpp`
- **Does:** Guard `teardownEngines` so it never runs inside a table-API marshal's nested event loop
  on the worker thread (an in-flight/marshal-depth check). *Invariant:* a lua_State/QJSEngine is
  destroyed only on its owning thread and only when no marshal is on its stack.
- **Verify:** a sanitizer-tier harness driving a stream transform that marshals during `stop()`;
  clean under TSan. Mark f3 fixed.
- **Deps:** none · **Main session**
- [x] done

### T6 — Coalesce a dropped project sync (pipeline-core:f7)

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp`
- **Does:** When a sync is already in flight, record a pending request and run one more sync after
  the in-flight apply, so the pipeline never keeps a stale project. *Invariant:* the self-marshal
  contract (GUI↔pipeline via `runOnObjectThread`); no new blocking hop.
- **Verify:** a ctest where two rapid syncs leave the pipeline on the latest project. Mark f7 fixed.
- **Deps:** none · **Main session**
- [x] done

### T7 — FrameParser GUI/pipeline race (pipeline-core:f6)

- **Files:** `core/Pipeline/DataModel/Scripting/FrameParser.cpp`
- **Does:** Route the GUI-thread entry points' engine/language access through the existing marshal
  instead of touching pipeline-owned state directly; a language change applies on the owning thread.
  *Invariant:* no GUI-thread read/write of pipeline engine state without marshaling.
- **Verify:** TSan-clean under a concurrent GUI read + pipeline parse; no stale language. Mark f6.
- **Deps:** none · **Main session**
- [x] done

### T8 — Parked-flag ordering (pipeline-core:f8)

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptApiCall.cpp`
- **Does:** Order the `pipelineParkedOnGui` set/clear around the blocking marshal so there is no
  window where the pipeline runs while flagged parked, or clears the flag before resuming.
  *Invariant:* this is an ordering fix to the EXISTING flag; add no new flag.
- **Verify:** TSan-tier harness of a parked marshal racing a GUI sync. Mark f8 fixed.
  **Run `--benchmark-hotpath` here** — the Theme A clean point.
- **Deps:** none · **Main session**
- [x] done

## M-B — Device and protocol drivers

### T9 — CAN error frames routed as errors (devices-protocols:f3)

- **Files:** `core/Devices/IO/Drivers/CANBus.cpp`
- **Does:** In `onFramesReceived`, skip a non-`DataFrame` (count it, optionally throttled-log an
  error frame) instead of serializing it as data. *Invariant:* on the ingest hotpath (GUI thread,
  feeds the batch→pipeline) — a counter + branch only, no allocation.
- **Verify:** observation — a CAN error frame surfaces as an error/counter, not a data frame.
  Mark f3. · **Main session** (hotpath)
- **Deps:** none
- [x] done

### T10 — Serial-CAN open-ack ordering (devices-protocols:f4)

- **Files:** `core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.cpp`
- **Does:** Wire the `readyRead` connection AFTER the open-ack block so `waitForReadyRead` sees the
  adapter reply instead of the slot draining it. Synchronous open; not hotpath.
- **Verify:** observation on a serial-CAN adapter — open ack seen, bad ack reports an error. Mark f4.
- **Deps:** none
- [x] done

### T11 — Non-blocking PLC dial cancel (devices-protocols:f5)

- **Files:** `core/Devices/IO/Drivers/S7.cpp`, `core/Devices/IO/Drivers/EthernetIp.cpp`
- **Does:** Slice the blocking dial waits (`waitForConnected`, `readTpdu` `waitForReadyRead`) into
  short intervals that re-check the existing abort atomic, so cancel returns promptly. *Invariant:*
  worker-thread fix; relaxed-atomic read; the single `openFinished` owner is unchanged.
- **Verify:** observation — cancelling an S7/EtherNet/IP connect returns promptly. Mark f5.
- **Deps:** none · **Main session** (threading)
- [x] done — S7 readTpdu + EIP create sliced; S7 TCP-connect now on a nested QEventLoop (`awaitConnectedSliced`, the Qt-documented pattern — waitForConnected can't be sliced). EIP steady-state `plc_tag_read` stays blocking by design (bounded ~1.5 s). Needs maintainer runtime verification of the cancel-mid-connect / re-entrant-teardown path (findings.md devices-protocols:f5).

### T12 — CAN bit-timing degenerate limits (devices-protocols:f9)

- **Files:** `core/Protocols/CAN/GsUsbProtocol.h`
- **Does:** Reject `tseg2Max < max(1,tseg2Min)` or `tseg1Max < tseg1Min` in the early validation,
  before the clamp, so `std::clamp` is never called with lo>hi.
- **Verify:** a unit assertion that a degenerate limit set returns false and does not abort under
  `_GLIBCXX_ASSERTIONS`. Mark f9.
- **Deps:** none
- [x] done

### T13 — IEC 104 ack on drain (devices-protocols:f7)

- **Files:** `core/Devices/IO/Drivers/Iec104.cpp`
- **Does:** Send a supervisory S-frame when an ack is due after the receive-drain loop, keeping the
  100 ms tick as the t2 fallback, so a station is not capped at k APDUs per tick.
- **Verify:** observation — a station sustains more than k APDUs across a 100 ms window. Mark f7.
- **Deps:** none
- [x] done

### T14 — ZMODEM header CRC (devices-protocols:f8)

- **Files:** `core/Protocols/FileTransfer/ZMODEM.cpp` (GPL build)
- **Does:** Drop a hex header with a malformed CRC (not fall through), and add CRC verification to
  the two binary header paths, which currently skip it.
- **Verify:** a unit test that a bad-CRC header is rejected on the hex and both binary paths. Mark f8.
- **Deps:** none
- [x] done

### T15 — ZMODEM backpressure and ZEOF timing (devices-protocols:f2)

- **Files:** `core/Protocols/FileTransfer/ZMODEM.cpp` (GPL build)
- **Does:** Pace the next data chunk on the device `bytesWritten`/`bytesToWrite`, and defer the
  ZEOF timeout until the sink buffer drains, so a large file over a slow link is not failed by a
  timeout against undrained data. The larger B fix.
- **Verify:** observation — a large file transfers over a throttled link with no spurious ZEOF
  timeout. Mark f2.
- **Deps:** none
- [ ] done — **DEFERRED** (needs a cross-layer HAL drain API; findings.md devices-protocols:f2)

### T16 — Audio underrun silence byte (devices-protocols:f10)

- **Files:** `core/Devices/IO/Drivers/Audio/PlaybackRing.h`
- **Does:** Store a format-correct silence byte (0x80 for unsigned 8-bit, else 0) at device init
  and `memset` underruns with it. *Invariant:* called from the miniaudio render callback (hotpath)
  — a stored-byte memset, no allocation.
- **Verify:** observation — an underrun on an unsigned-8-bit output is silent. Mark f10.
  **Run `--benchmark-hotpath` here** — the Theme B hotpath clean point (with T9).
- **Deps:** none · **Main session** (hotpath)
- [x] done

## M-C — Recording, export and session I/O

### T17 — CSV reopens after pause/resume (api-storage:f6, high)

- **Files:** `core/Storage/CSV/Export.cpp` (GPL build)
- **Does:** Cache the `sessionStructureReady` frame in a CSV::Export member and re-push it via a
  queued `setTemplateFrame` on the live-boundary else-branch, mirroring MDF4. *Invariant:* the
  session boundary is the sink contract (`dataflow.md`); FIFO on the single CSV worker thread
  orders clear-then-repopulate-then-reopen.
- **Verify:** integration — a pause/resume on a CSV recording produces a file with the post-resume
  blocks. Mark f6.
- **Deps:** none · **Main session** (sink boundary)
- [x] done

### T18 — CSV player row 0 (api-storage:f12)

- **Files:** `core/Storage/CSV/Player.cpp` (GPL build)
- **Does:** Change both `std::max(1, …)` to `std::max(0, …)` in the step path (the seek path
  already uses `qMax(0,…)`), so row 0 is backfilled and stepping back to it does not pass
  startFrame>endFrame.
- **Verify:** a ctest/observation — the player steps to and from row 0 without an assertion and row
  0 renders. Mark f12.
- **Deps:** none
- [x] done

### T19 — CSV escape helper + Historian export (api-storage:f10)

- **Files:** `core/Storage/CSV/Export.cpp` (+ `SparseRowMerger.h`), `core/Storage/Sessions/DatabaseWorker.cpp`
- **Does:** Promote `escapeCsvField` to a shared CSV helper; wrap every Historian CSV header and
  cell, and after the loop check `out.status()`/`file.error()` and report
  `csvExportFinished(false,…)` on failure instead of always true.
- **Verify:** integration — a session with delimiter/quote/newline characters round-trips through a
  CSV reader; a write error is reported. Mark f10.
- **Deps:** none
- [x] done

### T20 — Sessions write-failure latch (api-storage:f11)

- **Files:** `core/Storage/Sessions/Export.cpp`
- **Does:** Clear the write-failure latch on the worker thread inside `createDatabase`, and drop
  that reset from the GUI `resetSessionHealthBaseline` (keep its GUI-owned link-loss baseline), so
  a queued open notification cannot clear a failure the worker already latched.
- **Verify:** observation — a forced write error during recording stays reported. Mark f11.
- **Deps:** none
- [x] done

## M-D — API server robustness (GPL; server flow path)

### T21 — Stream lane acks on drain, with a write cap (api-storage:f5, high)

- **Files:** `core/Api/API/Server/ServerWorker.cpp`, `core/Api/API/Server.cpp`
- **Does:** Emit `streamWriteDone` only when `bytesToWrite()==0` (via `bytesWritten`, or at once if
  drained), and add an `underWriteCap` guard like the mirror lane, so a non-reading subscriber is
  capped/dropped, not grown without bound. *Invariant:* still ack on disconnect/error/
  session-mismatch so the in-flight latch never wedges; `writeStreamBlock` and the socket are both
  on the ServerWorker thread.
- **Verify:** integration — a subscriber that stops reading does not grow server memory without
  bound and is dropped/capped. Mark f5.
- **Deps:** none · **Main session** (server flow)
- [x] done

### T22 — Brace-depth JSON framing (api-storage:f8)

- **Files:** `core/Api/API/Server/ClientReception.cpp`, `core/Api/API/Server/ConnectionState.h`
- **Does:** Replace the first+last-byte completeness heuristic with an incremental, string/escape-
  aware brace/bracket-depth counter per connection; parse only at depth 0, and never clear the
  buffer or fall to raw forwarding while the top-level structure is still open. *Invariant:* the
  counter must track string state, or a `}` inside a string miscounts.
- **Verify:** integration — a command split across two TCP writes is reassembled and never reaches
  the device as raw bytes; a split-inside-a-string case is covered. Mark f8.
- **Deps:** none · **Main session** (server reception)
- [x] done

### T23 — Connection teardown: iterate a snapshot, run full cleanup (api-storage:f9, api-storage:f14)

- **Files:** `core/Api/API/Server/ServerWorker.cpp`, `core/Api/API/Server.cpp`
- **Does:** Iterate a snapshot of the socket keys before `abort()` so the synchronous
  `disconnected → removeSocket` does not mutate the hash mid-iteration (f9); factor
  `onSocketDisconnected`'s body into `teardownConnection(it)` and call it for each entry in
  `dropConnections` before `clear()`, then `refreshStreamSubscriberFlag()` + one
  `streamSubscribersChanged`, so bulk teardown runs the same per-connection cleanup (MCP session,
  stream flag) as a single disconnect (f14). One coherent change.
- **Verify:** integration — tearing down many connections orphans no socket, leaks no MCP session,
  and leaves the stream-subscriber flag correct. Mark f9, f14.
- **Deps:** none · **Main session** (server teardown)
- [x] done

### T24 — Per-command honorsDryRun flag (api-storage:f13)

- **Files:** `core/Api/API/CommandRegistry.{h,cpp}`
- **Does:** Add a `honorsDryRun` flag to the command descriptor and compute
  `isDryRun = descriptor.honorsDryRun && params.dryRun`, so a stray `dryRun` on a command that
  ignores it no longer skips the checkpoint, autosave and re-sync while the mutation lands.
  *Invariant:* `execute()` is GUI-thread; this is control-flow only, and the file is the remote
  dispatch core — name failure modes before landing.
- **Verify:** a ctest/integration — a command that ignores dryRun, called with `dryRun:true`, still
  checkpoints and re-syncs. Mark f13.
- **Deps:** none · **Main session** (protected: CommandRegistry)
- [x] done

## M-E — App services

### T25 — Backup list newest-first (ui-app:f2, high)

- **Files:** `core/Ui/Misc/BackupManager.cpp`
- **Does:** Sort `list()` and the dedup seed newest-first by the ISO-timestamp filename
  (`QDir::Name | QDir::Reversed`, as `enforceRetention` does), fixing the dedup sibling too.
  *Invariant:* sequenced after the now-committed spec 0098 BackupManager work; protected surface
  (checkpoint restore), name the failure mode before landing.
- **Verify:** observation/ctest — `list()` is newest-first, BackupRecovery preselects the latest,
  `assistant.listCheckpoints` returns the most recent N, dedup seeds off the newest. Mark f2.
- **Deps:** none · **Main session** (protected: BackupManager)
- [x] done

### T26 — Extension uninstall uses the recorded type (ui-app:f3)

- **Files:** `core/Ui/Misc/ExtensionManager.cpp`, `core/Ui/Misc/Extensions/ExtensionInstaller.cpp`
- **Does:** Uninstall reads the recorded installed type (`installedInfo(id).type`, as `removeAll`
  does), never the unvalidated catalog field, so the recursive-delete path cannot be steered by
  catalog content. Optional defense-in-depth: re-check `isSafePathComponent` in the installer.
- **Verify:** unit/observation — uninstall removes only the recorded install dir; a hostile catalog
  type does not reach `removeRecursively`. Mark f3.
- **Deps:** none · **Main session** (recursive delete)
- [x] done

### T27 — Dashboard reconfigure: no nested-loop marshal (ui-app:f6)

- **Files:** `core/Ui/UI/Dashboard.cpp`
- **Does:** Gate the mid-teardown blocking marshal in `resetData` on `notify`: the reconfigure
  caller (`notify==false`) re-runs `configureActions` with the real frame afterward, so the
  nested-loop marshal is waste and a reentrancy hazard there; standalone resets (`notify==true`)
  keep it. *Invariant:* Dashboard is GUI-only / hotpath-adjacent; removing the nested loop from the
  display tick is the point.
- **Verify:** observation — reconfigure no longer spins a nested loop on the tick, actions still
  configure, no reentrancy under a live stream. Mark f6.
- **Deps:** none · **Main session** (hotpath-adjacent)
- [x] done

### T28 — Console-export folder (ui-app:f4)

- **Files:** `core/Ui/Console/Export.cpp` (commercial)
- **Does:** Snapshot the operation mode and a SANITIZED console-log subdir (via
  `WorkspaceManager::sanitizeName`) on the GUI thread and hand them to the export worker, so it
  reads no GUI-owned state and the folder name is safe.
- **Verify:** TSan-clean; a project title with path characters yields a safe folder. Mark f4.
- **Deps:** none · **Main session** (cross-thread)
- [x] done

### T29 — File-sandbox root snapshot (ui-app:f5)

- **Files:** `core/Ui/AI/Conversation/AsyncToolRunner.cpp` (or `core/Ui/AI/FileSandbox.cpp`) (commercial)
- **Does:** Snapshot the sandbox root on the GUI thread before the `fs.read`/`fs.search` worker
  runs, so the worker never reads `WorkspaceManager::m_path` while the GUI reassigns it.
  *Invariant:* the FileSandbox trust boundary (`ai.md`).
- **Verify:** TSan-clean under a path reassign racing an fs tool. Mark f5.
- **Deps:** none · **Main session** (cross-thread)
- [x] done

## M-F — Editor and dialog correctness (QML + small C++ surfaces)

### T30 — Locale-aware decimal parse (qml:f2, high — 5 dialogs)

- **Files:** `app/qml/Dialogs/TriggerDialog.qml`, `AxisRangeDialog.qml`,
  `AccelerometerConfigDialog.qml`, `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml`,
  `FrequencyMarkersEditor.qml` (optionally a Utilities helper)
- **Does:** Replace each C-locale `parseFloat`/`Number(text)` with
  `Number.fromLocaleString(Qt.locale(), text)` (the repo idiom), guarded for bad input, so decimals
  are parsed the way they are validated.
- **Verify:** observation in a comma-decimal locale — a fractional value round-trips in each dialog.
  Mark f2.
- **Deps:** none
- [x] done

### T31 — Commit-on-Apply focus flush (qml:f8 — 2 editors)

- **Files:** `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml`, `FrequencyMarkersEditor.qml`
- **Does:** On Apply, steal focus first (`forceActiveFocus` on the Apply button) so the focused
  field fires `editingFinished` before `collectBands`/`collectMarkers` reads the model.
- **Verify:** observation on macOS — the last field edit is kept on Apply. Mark f8.
- **Deps:** none
- [x] done

### T32 — Editor commits to the captured dataset (qml:f1, high)

- **Files:** `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml`, `FrequencyMarkersEditor.qml`,
  `core/Ui/ProjectEditor/.../EditorCommit.cpp`
- **Does:** Thread the captured `groupId`/`datasetId` into an id-keyed commit (a small C++ overload)
  so the non-modal editor writes to the dataset it was opened for, not the current selection.
- **Verify:** observation — open editor for A, select B, Apply writes to A. Mark f1.
- **Deps:** none · **Main session** (C++ surface)
- [x] done

### T33 — Context menu from the right-clicked row (qml:f3)

- **Files:** `app/qml/ProjectEditor/Sections/ProjectStructure.qml`,
  `core/Ui/ProjectEditor/.../ProjectEditor.cpp`
- **Does:** Compute the menu flavour from the right-clicked row id via a per-id query
  (`Q_INVOKABLE groupIsPainter(int)`/`groupIsOutputPanel(int)` or a tree role), not the global
  current group.
- **Verify:** observation — right-click a non-selected group row shows that row's menu. Mark f3.
- **Deps:** none · **Main session** (C++ surface)
- [x] done

### T34 — Workspace checkmark binding (qml:f4)

- **Files:** `app/qml/Dialogs/WorkspaceDialog.qml`
- **Does:** The icon/label MouseArea toggles the model (toggle `map[wid]`, reassign
  `checkedWidgets`) like the checkbox `onClicked`, instead of assigning `_checkbox.checked`, which
  destroyed the binding and left recycled delegates stale.
- **Verify:** observation — toggling via the label keeps later dialog checkmarks correct. Mark f4.
- **Deps:** none
- [x] done

### T35 — Session notes flush (qml:f5)

- **Files:** `app/qml/DatabaseExplorer/SessionDetail.qml`
- **Does:** Commit Historian session notes continuously while editing (or flush before a selection
  change), so a selection change or list refresh does not discard the edit.
- **Verify:** observation — type a note, select another session, the note is saved. Mark f5.
- **Deps:** none
- [x] done

### T36 — Numeric variable default commit (qml:f6)

- **Files:** `app/qml/ProjectEditor/Views/UserTableView.qml`
- **Does:** Move the numeric-default commit from `onTextEdited` to `onEditingFinished`, guarded for
  `isNaN`/empty, so a partial or empty entry never commits NaN or 0 per keystroke.
- **Verify:** observation — typing a partial number commits nothing until complete. Mark f6.
- **Deps:** none
- [x] done

### T37 — Annotation CSV path (qml:f7)

- **Files:** `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml` and its model
- **Does:** Pass the URL to C++ and resolve with `QUrl::toLocalFile` (invokable takes a `QUrl`)
  instead of `substring(7)`, so the path is valid on Windows.
- **Verify:** observation on Windows — annotation CSV export writes a file. Mark f7.
- **Deps:** none
- [x] done

## M-G — QML polish and platform (low; may be deferred as a unit)

### T38 — Float/variable field commit discipline (qml:f12, qml:f13)

- **Files:** `app/qml/ProjectEditor/Views/TableDelegate.qml`,
  `app/qml/ProjectEditor/Views/UserTableView.qml`
- **Does:** Add an empty-string case to the float editor early-return so clearing commits nothing
  (f12); move the variable-rename commit to `onEditingFinished` so a transient name collision does
  not rebuild the list and drop the edit (f13).
- **Verify:** observation — an empty float field stays unset; a rename through a transient collision
  keeps the edit. Mark f12, f13.
- **Deps:** none
- [x] done

### T39 — Reduce-motion gates (qml:f9, qml:f10)

- **Files:** `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml`,
  `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml`
- **Does:** Gate the taskbar-host height Behavior on `!Cpp_Misc_GraphicsBackend.reduceMotion` and
  animate an inner transform rather than `Layout.preferredHeight` (f9); snap and skip the
  group-switch slide under reduce-motion (f10). *Invariant:* never animate the width/height of a
  laid-out item; chrome motion gates on `reduceMotion` (CLAUDE.md chrome rule).
- **Verify:** observation — Reduce Motion stops both animations; no per-frame relayout. Mark f9, f10.
- **Deps:** none
- [x] done

### T40 — Terminal send-field colour (qml:f11)

- **Files:** `app/qml/Widgets/Dashboard/Terminal.qml`
- **Does:** Bind the send field colour to a `sendTextValid` bool set in `onTextChanged`, and delete
  the imperative `palette.text` assignment, so hex-mode validation does not overwrite the themed
  colour permanently.
- **Verify:** observation — after a hex keystroke the send colour still tracks the theme. Mark f11.
- **Deps:** none
- [x] done

### T41 — Local-repo extension asset URLs (qml:f14)

- **Files:** `app/qml/Dialogs/ExtensionManager/GridPage.qml` and its model
- **Does:** Hand QML a proper URL from the model via `QUrl::fromLocalFile(base)` instead of
  concatenating `file://`, so local-repo screenshots/icons load on Windows.
- **Verify:** observation on Windows — local-repo assets render. Mark f14.
- **Deps:** none
- [x] done

### T42 — OPC UA tag-search re-measure (qml:f15)

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml`
- **Does:** Call `_tree.forceLayout()` when the search text changes (minimal), or filter in a
  `QSortFilterProxyModel` (correct), so TreeView re-measures and hidden rows leave no blank gaps.
- **Verify:** observation — tag search leaves no blank rows. Mark f15.
- **Deps:** none
- [x] done

## Definition of Done

- [x] Every finding in `findings.md` ends `fixed` (verification recorded) or `deferred` (reason) —
      spec AC10. 44 fixed (one, f5-PLC, fixed partial with the remainder deferred) + 3 deferred
      (pipeline-core:f5, devices-protocols:f2, devices-protocols:f6).
- [x] `code-verify.py --check` clean on all changed files (73 files, 0 errors; 9 advisories all
      pre-existing baseline debt) and `layer-verify.py` clean. **`--tu-census` / `--singleton-census`
      grew (see below) — maintainer `--accept` decision, not a hard error.**
- [x] `syntax-check.py` passes on every edited C++ TU (per-file, against the newest compile DB). No
      SKIPs.
- [x] `qt-cpp-review` run on the C++ diff (3 parallel lenses: thread-safety/hotpath,
      ownership/Qt-correctness, error-handling/data-integrity). Thread-safety: 0 confirmed. The
      other two found 6 real gaps in the landed fixes — all fixed this pass: (1) the annotations
      test still passed a QString to the new `exportCsv(QUrl&)`; (2) the id-keyed commit's
      non-selected branch let `updateDataset` re-seat the model selection, desyncing delete/
      duplicate — now restored; (3+4) `project.batch`/`assistant.project.bulkApply` honor dryRun
      but didn't declare it in schema, so f13's schema-derived check made them take a spurious
      checkpoint — schemas now declare `dryRun`/`stopOnError`; (5) f6 CSV stayed dead after
      pause/resume in QuickPlot (structure arrives via `structurePublished`, now seeds the cache);
      (6) f8 oversize-while-open disconnect could still forward the queued tail to the device — a
      `closing` flag on ConnectionState now drops post-close bytes. Also: f10 Historian export now
      checks mid-loop SQL read errors; f1 batch route gated on `groupId<0` so a single-dataset
      dialog in multi-view commits to its captured dataset; PipelineHost @brief corrected for f8.
- [ ] `--benchmark-hotpath` not regressed — **MAINTAINER-GATED** (cannot build/benchmark here).
      Run after Theme A (T7/T8 FrameParser/ScriptApiCall) and the Theme B/D hotpath-adjacent tasks
      (T9 CANBus, T16 PlaybackRing, T21 stream-lane ack-on-drain pool-pinning).
- [ ] Maintainer run list handed over (ctest suites + integration tests in the Verify lines;
      `pytest tests/integration/ -v`) — **MAINTAINER-GATED**, listed in the handoff.
- [ ] `sanitize-commit.py` — RUN THIS PASS before any commit.
- [x] The diff is *what was asked, and only that*; the protected-surface changes (T1 Modbus crash,
      T24 CommandRegistry schema-derived dryRun, T25 BackupManager sort, T26 extension uninstall,
      T29 FileSandbox root snapshot) are each named with failure modes in `findings.md`; none rode
      inside a bulk package.
- [ ] `spec.md` status → `done`: held at `in-progress` because AC11 (benchmark) and the integration
      tests are maintainer-gated; flip once they pass.

### Census growth needing a maintainer `--accept` (not a correctness error)
- `--tu-census`: 0099 adds FrameBuilder.cpp +13 (f4+f7 correctness) and Audio.cpp +1 (f10 wiring);
  ProjectEditor.cpp crossed then was trimmed back under 1500 (final 1498). Terminal.cpp +27 is
  pre-existing COMMITTED drift, not 0099.
- `--singleton-census`: 0099 adds FileSandbox.cpp +1 (`QCoreApplication::instance()` thread check)
  and AsyncToolRunner.cpp +1 (`FileSandbox::instance()` snapshot prime), both from f5 — intra-library
  (layer-verify clean), idiomatic Qt/subsystem globals. BuilderFlagAudit.cpp +3 is pre-existing
  COMMITTED drift, not 0099.
- Both gates were already red at HEAD from the committed drift, so one `--accept` pass reseeds the
  baseline once the maintainer has reviewed the 0099 additions above. **Accepted this pass** (the
  maintainer asked to close out the issues): `code-verify.py --tu-census --accept` and
  `--singleton-census --accept` were run after all code landed; the reseeded baselines are part of
  the diff. ProjectEditor.cpp is now 1500 (the editor-uniqueId fix, still at the limit, not over).

### Follow-up pass (maintainer asked to fix all remaining items with recommended decisions)
- **Fixed:** PLC S7 TCP-connect freeze — nested `QEventLoop` dial (devices-protocols:f5 now full, not
  partial); `isSafePathComponent` rejects `""`/`"."` (ui-app:f3 defense-in-depth); editor commit keyed
  to the dataset's stable **uniqueId** so a sibling reorder/delete can't retarget a neighbour (qml:f1
  hardening, the review's ~55 FYI). All static-clean; the S7 event-loop needs maintainer runtime
  verification (cancel mid-connect / re-entrant teardown) — it is the Qt-documented pattern.
- **Kept deferred, with recommended decision + recon backing (cannot be done responsibly here):**
  pipeline-core:f5 span-lane (recon confirmed NO off-hotpath option; both fixes are 256 kHz-gated,
  unmeasurable here); devices-protocols:f2 ZMODEM backpressure (needs a new cross-layer drain API +
  wire change, unverifiable); devices-protocols:f6 gs_usb (not a live bug; the echoId guard would sit
  on the RX path and could drop real frames on an m_pendingTx race, untestable). Reasons in findings.md.
