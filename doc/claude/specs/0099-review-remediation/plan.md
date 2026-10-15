---
spec: 0099-review-remediation
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks) (approved 2026-10-14)
updated: 2026-10-14
---

# Plan 0099 — Remediation of the 2026-10-13 app-level review findings

> **Phase 2 of 4 — the HOW.** The design that satisfies [`spec.md`](./spec.md). The per-finding
> location, fix and verification live in [`findings.md`](./findings.md); this plan gives the
> per-theme design, the affected files, the hotpath/threading answer and the tradeoffs. Gate: do
> not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Each of the 47 findings gets the smallest change that restores an already-promised behavior; no
public contract moves. The work is grouped into the spec's seven themes, each a coherent,
independently reviewable change, and severity orders them: Theme B leads with the one critical
(the Modbus connect crash), then the high findings in A, C, D, E and F, then the low polish in G.
Every finding was re-confirmed against the code at HEAD by a per-theme read-only pass; that pass
corrected three review errors (recorded in `findings.md`): gs_usb reassembly (B f6) is not a live
bug and is **deferred**; the CSV-reopen bug (C f6) is CSV-only, because MDF4 and Sessions already
handle resume; and f13's line moved after spec 0098. The `findings.md` ledger is the tracking
artifact — every id ends `fixed` (with its verification) or `deferred` (with a reason), which is
AC10.

## Affected subsystems & files

Grouped by theme. Confirmed by the per-theme ground-truth pass; line numbers are in `findings.md`.
**GPL** marks a file that ships in the GPLv3 build (the fix must compile there); the rest are
`#ifdef BUILD_COMMERCIAL`. **P** marks a protected surface named in chat before it lands.

**Theme A — Pipeline and buffer correctness (GPL; hotpath/threading — main session)**

| File | Change |
|------|--------|
| `core/Core/CircularBuffer.h` | Free-running (unmasked) head/tail so full ≠ empty; mask only on access (A f2, f1). |
| `core/Pipeline/IO/FrameReader.cpp` | Align `discardPendingBytes` with what the ring overwrote (A f1, with f2). |
| `core/Pipeline/DataModel/FrameBuilder.cpp` | Span-lane writeback so republish sees current values (A f5); coalesce a dropped sync (A f7); clear the interned-key cache on the 1 Hz GC (A f4). |
| `core/Pipeline/DataModel/DataTable.{h,cpp}` | The interned-key cache clear entry point (A f4). |
| `core/Pipeline/IO/StreamWorker.cpp` | Guard `teardownEngines` against a marshal nested loop (A f3). |
| `core/Pipeline/DataModel/Scripting/FrameParser.cpp` | Marshal the GUI-thread entry points' engine/language access (A f6). |
| `core/Pipeline/DataModel/Scripting/ScriptApiCall.cpp` | Order the parked-flag set/clear around the blocking marshal (A f8). |
| `app/tests/tst_circular_buffer.cpp`, plus new ctest TUs | Regression tests for f2/f1, f5, f4, and the race fixes. |

**Theme B — Device and protocol drivers**

| File | Change |
|------|--------|
| `core/Devices/IO/Drivers/Modbus.cpp` **P-ish (crash)** | Null-guard the re-entrant `failDial` deref (B f1, critical). |
| `core/Devices/IO/Drivers/CANBus.cpp` | Route non-data frames as errors, not data (B f3, ingest hotpath). |
| `core/Devices/IO/Drivers/CANBus/SerialCanBackendBase.cpp` | Wire `readyRead` after the open-ack (B f4). |
| `core/Devices/IO/Drivers/S7.cpp`, `.../EthernetIp.cpp` | Slice the blocking dial waits to re-check the abort atomic (B f5). |
| `core/Protocols/CAN/GsUsbProtocol.h` | Reject degenerate bit-timing limits before the clamp (B f9). |
| `core/Devices/IO/Drivers/Iec104.cpp` | Ack an I-frame on drain, not only on the tick (B f7). |
| `core/Protocols/FileTransfer/ZMODEM.cpp` **GPL** | CRC-verify received headers (B f8); backpressure + ZEOF timing (B f2). |
| `core/Devices/IO/Drivers/Audio/PlaybackRing.h` | Pad underruns with a format-correct silence byte (B f10, render hotpath). |

**Theme C — Recording, export and session I/O**

| File | Change |
|------|--------|
| `core/Storage/CSV/Export.cpp` **GPL** | Cache the session structure and re-push it on resume (C f6, high); matches MDF4. |
| `core/Storage/CSV/Player.cpp` **GPL** | `std::max(0, …)` in the step path (C f12). |
| `core/Storage/Sessions/Export.cpp` | Clear the write-failure latch on the worker, not the GUI reset (C f11). |
| `core/Storage/Sessions/DatabaseWorker.cpp` | Escape CSV fields and report write errors (C f10). |
| `core/Storage/CSV/Export.cpp` / `SparseRowMerger.h` | Promote `escapeCsvField` to a shared helper (C f10). |

**Theme D — API server robustness (GPL; **P** — server auth/flow path)**

| File | Change |
|------|--------|
| `core/Api/API/Server/ServerWorker.cpp`, `core/Api/API/Server.cpp` | Ack the stream lane on drain + write cap (D f5, high); snapshot-iterate teardown (D f9); factor `teardownConnection` for bulk drop (D f14). |
| `core/Api/API/Server/ClientReception.cpp`, `core/Api/API/Server/ConnectionState.h` | Brace-depth framing for newline-less JSON (D f8). |
| `core/Api/API/CommandRegistry.{h,cpp}` **P** | Per-command `honorsDryRun` flag (D f13). |

**Theme E — App services**

| File | Change |
|------|--------|
| `core/Ui/Misc/BackupManager.cpp` **P (adjacent to 0098)** | Newest-first sort for `list()` and the dedup seed (E f2, high). |
| `core/Ui/Misc/ExtensionManager.cpp`, `.../ExtensionInstaller.cpp` | Uninstall from the recorded install type, not the catalog field (E f3). |
| `core/Ui/UI/Dashboard.cpp` | Gate the mid-teardown marshal on `notify` (E f6, hotpath-adjacent). |
| `core/Ui/Console/Export.cpp` | Snapshot a sanitized folder on the GUI thread (E f4). |
| `core/Ui/AI/Conversation/AsyncToolRunner.cpp` or `core/Ui/AI/FileSandbox.cpp` | Snapshot the sandbox root before the worker (E f5). |

**Theme F — Editor and dialog correctness (QML), with small C++ surfaces**

| File | Change |
|------|--------|
| `app/qml/Dialogs/TriggerDialog.qml`, `AxisRangeDialog.qml`, `AccelerometerConfigDialog.qml`, `app/qml/ProjectEditor/Dialogs/{AlarmBandsEditor,FrequencyMarkersEditor}.qml` | Locale-aware decimal parse (F f2, high); commit-on-Apply focus flush (F f8). |
| `AlarmBandsEditor.qml` / `FrequencyMarkersEditor.qml` + an id-keyed commit in `core/Ui/ProjectEditor/.../EditorCommit.cpp` | Commit to the captured dataset id, not the selection (F f1, high). |
| `app/qml/ProjectEditor/Sections/ProjectStructure.qml` + a per-id query in `ProjectEditor.cpp` | Context menu from the right-clicked row (F f3). |
| `app/qml/Dialogs/WorkspaceDialog.qml` | Toggle the model, not the checkbox binding (F f4). |
| `app/qml/DatabaseExplorer/SessionDetail.qml` | Flush session notes before a selection change (F f5). |
| `app/qml/ProjectEditor/Views/UserTableView.qml` | Commit numeric default on editingFinished, guarded (F f6). |
| `app/qml/Widgets/Dashboard/ConsoleAnnotations.qml` + its model | `QUrl::toLocalFile` for the annotation CSV path (F f7). |

**Theme G — QML polish and platform (low; may defer as a unit)**

| File | Change |
|------|--------|
| `app/qml/ProjectEditor/Views/TableDelegate.qml` | Empty float field commits nothing (G f12). |
| `app/qml/ProjectEditor/Views/UserTableView.qml` | Variable rename on editingFinished (G f13). |
| `app/qml/MainWindow/Panes/Dashboard/DashboardLayout.qml` | Reduce-motion gate; animate a transform, not laid-out height (G f9). |
| `app/qml/MainWindow/Panes/Dashboard/DashboardCanvas.qml` | Reduce-motion gate on the group-switch slide (G f10). |
| `app/qml/Widgets/Dashboard/Terminal.qml` | Bind the send colour to a validity bool (G f11). |
| `app/qml/Dialogs/ExtensionManager/GridPage.qml` + its model | `QUrl::fromLocalFile` for local-repo assets (G f14). |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/OpcUaTagBrowser.qml` | `forceLayout()` on search, or a proxy filter (G f15). |

## Architecture & data flow

The themes are independent; the cross-cutting shapes:

- **Theme A sits on the acquisition path.** The ring (f2/f1) is the SPSC buffer between the
  driver and `FrameReader`; the fix changes how head/tail are represented, not the lock model.
  The span-lane writeback (f5) is in `FrameBuilder`'s zero-allocation fast lane. The interned
  cache (f4) is read per table call; its clear rides the existing 1 Hz GC. The three races (f3,
  f6, f8) are about the GUI↔pipeline marshal boundary (`runOnObjectThread` / the parked flag) and
  the stream-worker teardown order; each fix keeps the existing marshal contract.
- **Theme B** is the driver connect/dial path (`io.md` "Opening a Link": one `openFinished`/
  `failDial` owner) plus the wire codecs; f3 and f10 touch per-frame/render paths.
- **Themes C/D** are the recording-sink boundary (`dataflow.md` "The Session Boundary") and the
  socket server's framing/flow/teardown; the recording fix (C f6) mirrors the MDF4 pattern.
- **Themes E/F/G** are GUI-thread services and QML; F f1/f3 add a small id-keyed C++ query so the
  QML commits by captured id instead of the global selection.

## Hotpath & threading impact

**REQUIRED answer.** This is the section that gates Theme A.

- **Touches the hotpath?** **Yes, Theme A, and two spots in B/E.** The rest do not.
  - **Ring (A f2/f1)** — `CircularBuffer` is SPSC, main/pipeline. The fix moves to free-running
    indices and masks on access; **no mutex is added** (the contract forbids it). The read/write
    cost is unchanged (an extra subtract, no branch on the hot read). Guarded by
    `--benchmark-hotpath` and `tst_circular_buffer`.
  - **Span lane (A f5)** — the fix must stay allocation-free on the fast lane. Two options in
    Tradeoffs; the chosen one adds at most a bounded copy already sized at bind, never a heap
    allocation, and is benchmarked. **This is the one finding whose fix could move the 256 kHz
    number**; the plan requires the before/after benchmark as its gate.
  - **Interned cache (A f4)** — read per table-API call; the fix only adds a cache *clear* on the
    1 Hz GC tick, nothing per frame.
  - **CAN frame routing (B f3)** and **audio underrun (B f10)** are per-frame/per-render but add a
    branch and a stored-byte `memset`; no allocation.
- **New cross-thread signal/slot?** Only within existing contracts: D f5 connects
  `QAbstractSocket::bytesWritten` on the ServerWorker thread (same thread as the socket); A's race
  fixes use the existing marshal, add no `BlockingQueuedConnection` from the GUI into the pipeline.
- **New input to a cached hotpath flag?** No. A f8 fixes the *ordering* of the existing
  `pipelineParkedOnGui` flag; it adds no new flag.
- **Timestamp ownership.** Untouched. No fix re-stamps; the source still stamps at the driver
  boundary.

## Data model & persistence

- No `Frame.h` key, schema, writer-version or project-JSON change.
- No new `QSettings` key. (E f2 changes a sort order, not a stored value.)
- No migration: every fix reads and writes the same formats as today. The Historian CSV export
  (C f10) changes its *output* escaping, which only makes malformed output correct.

## API / SDK surface

- No new API command, no changed command signature, no `api-schema.json` regeneration.
- D f13 adds an internal per-command descriptor flag (`honorsDryRun`); it is not part of the wire
  surface. D f5/f8/f9/f14 are server-internal behavior. The dry-run commands keep their replies.

## QML / UI

- Themes F and G are QML plus three small C++ query/commit surfaces (F f1 id-keyed commit, F f3
  per-id group query, F f7/G f14 `QUrl` helpers on existing models). No new component, model or
  context property. The reduce-motion gates (G f9/f10) use the existing
  `Cpp_Misc_GraphicsBackend.reduceMotion`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Ring full ≠ empty (A f2) | (a) free-running indices, mask on access; (b) keep one slot empty; (c) a separate count/full flag | **(a)** — exact, no capacity lost, single-producer/consumer-safe with plain atomics; (b) wastes a slot and still needs care at wrap; (c) adds a third atomic to keep in sync. |
| Span-lane writeback (A f5) | (a) write parsed cells back into the source template each frame; (b) republish reads the last pooled slot for a span source | **(b)** if the benchmark shows (a) regresses the gate; **(a)** if it does not. Decide on the `--benchmark-hotpath` number, not on paper. |
| Interned-cache correctness (A f4) | (a) clear on GC; (b) hold a ref to each cached Lua string; (c) stop caching by raw pointer | **(a)** — LuaJIT frees a GCstr only on a sweep, so clearing on the 1 Hz GC makes a reused address impossible; (b) pins strings and complicates teardown; (c) loses the hot-path win the cache exists for. |
| CSV reopen (C f6) | (a) mirror MDF4 (cache + re-push on boundary); (b) invalidate the pool on pause so `structurePublished` re-fires | **(a)** — local to CSV, matches a proven sibling; (b) perturbs the shared pool-generation contract for every sink. |
| dry-run bookkeeping (D f13) | (a) per-command `honorsDryRun` flag; (b) drive autosave/re-sync off the mutation epoch and drop the `!isDryRun` gate | **(a)** — also fixes the skipped checkpoint, and stops a genuine dry run from snapshotting; (b) fixes only the re-sync. |
| Backup sort (E f2) | (a) `QDir::Name | Reversed` (ISO-filename, newest-first); (b) drop `Reversed` (mtime) | **(a)** — matches `enforceRetention`, immune to mtime rewrites. |
| Locale decimal parse (F f2) | (a) `Number.fromLocaleString(Qt.locale(), …)` everywhere; (b) force `locale: "C"` on each validator | **(a)** — the repo's own idiom, and it lets comma-decimal users type a comma; (b) is smaller but blocks their input. A one-line Utilities helper curbs recurrence. |
| Theme G scope | include in 0099 / defer as a unit | **Last milestone, defer-able** — the ledger records the outcome; A–F do not depend on it. |
| gs_usb (B f6) | fix / defer | **Defer** — confirmed not a live bug at HEAD (HW-timestamp mode never enabled, splits handled); recorded with its reasoning. |

## Risks & mitigations

- **Regressing the 256 kHz gate (A f5, f2; B f3, f10).** Each hotpath fix is measured before and after; f5's two options are decided on that number. If (a) regresses, (b) is the fallback.
- **A ring rewrite is subtle (A f2).** It lands with `tst_circular_buffer` cases for exact-fill
  and oversized chunks that fail on the old code, and the FrameReader desync (f1) is tested
  alongside, because they are one change.
- **The race fixes (A f3/f6/f8, E f6) are the silent-breakage class** `common-mistakes.md`
  warns about: queued-vs-direct, nested event loops, mutate-while-iterate. Each is verified under
  the sanitizer tier where a unit harness can drive it, and `ss-hotpath` is invoked before the
  FrameBuilder/FrameReader/CircularBuffer edits per CLAUDE.md.
- **Server flow-control (D f5) must still ack on disconnect/error/session-mismatch** or it wedges
  the in-flight latch; the fix preserves that guarantee and a test drives a stalled subscriber.
- **The brace-depth framing (D f8) must be string/escape-aware** or a `}` inside a JSON string
  miscounts; the parser tracks string state, and a fuzz/split test covers it.
- **Protected surfaces** (D f13 CommandRegistry, E f2 BackupManager) are named in chat with their
  failure modes before they land and never ride inside a bulk package; E f2 is sequenced after the
  now-committed 0098 BackupManager work.
- **Three-review-error risk** is already retired: the ground-truth pass caught f6-gsusb (defer),
  f6-CSV (CSV-only) and the f13 line shift; `findings.md` records them.

## Test & verification plan

Mapped to the spec's acceptance criteria; `findings.md` holds the per-id verification.

- **Unit / ctest (you build; I may run against an existing build dir):**
  - **AC1/AC2 (Theme A):** `tst_circular_buffer` exact-fill + oversized (f2/f1); a span-lane
    republish test (f5); an interned-cache GC-reuse test (f4); sanitizer-tier harnesses for the
    teardown/sync/parked races (f3/f5/f7/f8).
  - **AC3 (Theme B):** Modbus re-entrant-failure (f1), bit-timing degenerate limits (f9), and the
    ZMODEM header-CRC (f8) have unit coverage; connect-path and PLC-cancel (f1/f5) are observations.
  - **AC5 (Theme C):** CSV player row-0 step (f12) ctest; the CSV escaping (f10) round-trips.
- **Integration (you run a headless or GUI app with the API server on):**
  - **AC5:** CSV pause/resume produces post-resume blocks (f6); a forced write error stays
    reported (f11).
  - **AC6 (Theme D):** a stalled stream subscriber is capped/dropped and memory stays bounded
    (f5); a split command is reassembled, not sent raw (f8); bulk teardown orphans no socket and
    runs full cleanup (f9/f14).
- **Maintainer observation (scripted where possible):** the connect/PLC-cancel findings (B f1/f5);
  the editor and dialog findings (AC8: F f1/f2/f8/f5/f4/f6/f3); the QML polish (AC9: Theme G) or
  its recorded deferral.
- **Sanitizer tier:** the cross-thread fixes (A f3/f6/f8, E f4/f5, C f11) run clean under TSan.
- **Hotpath:** `--benchmark-hotpath` before and after Theme A and the two B hotpath fixes (AC11).
- **Static, per theme:** `syntax-check.py` on every edited TU; `code-verify.py --check`,
  `--tu-census`, `--singleton-census`, `layer-verify.py`; `qt-cpp-review` on each theme's C++ diff
  before handoff; `sanitize-commit.py` before any commit.
- **AC10:** `findings.md` ends with every id `fixed` (verification recorded) or `deferred`
  (reason recorded).
