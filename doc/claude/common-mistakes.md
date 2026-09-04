# Common Mistakes — Silent Breakage

> On-demand lookup table, grouped by subsystem so a targeted read is possible. These are
> gotchas the linter can't always catch. Rule-driven mistakes are listed once in the style
> sections (CLAUDE.md "Code Style") — don't restate. Four essay-length entries live in full
> in `doc/claude/architecture/`; their rows here are pointers.

## Hotpath & Frame Pool

| Mistake | Fix |
|---------|-----|
| Mutexes in FrameReader / CircularBuffer | Recreate via `resetFrameReader()` / `reconfigure()`. SPSC is the contract. |
| `Qt::QueuedConnection` on the frame hotpath when both ends are on main | `Qt::DirectConnection`. Queued = queue-full drops at 10+ kHz. |
| `QMap::operator[]` on `m_sourceFrames[sourceId]` | `.find()` and validate — `operator[]` silently inserts. |
| `std::make_shared<DataModel::TimestampedFrame>(...)` directly on the FrameBuilder hotpath | Use `acquireFrame(src)` / `acquireFrame(src, ts)`. Direct `make_shared` bypasses the slot pool and brings back per-frame heap allocs. |
| Calling `.toDouble()` on **any** receiver (`QString`, `QStringView`, `QByteArray`, `QVariant`, `QJsonValue`) | Use the matching `SerialStudio::toDouble(...)` overload (inline, fast_float-backed). `code-verify.py:qt-todouble-direct` blocks direct calls outside `SerialStudio.h`/`ThirdParty/`. Qt's string parser walks the full locale + double-conversion pipeline even for plainly non-numeric text (measured >2x parse-throughput loss); the `QVariant`/`QJsonValue` overloads additionally parse string-typed payloads (JSON `"5.5"` loads as 5.5 instead of silently 0) and fall back to Qt for non-string types. `Frame.h` can't include `SerialStudio.h` (circular) — that's why the number-parsing `read()`/`serialize()` family lives out-of-line in `Frame.cpp`. |
| Treating `CapturedData::data` as a smart pointer (`*data->data`, `data->data->size()`, `if (!data->data)`) | It's a `QByteArray` now. Use `data->data`, `data->data.size()`, `data->data.isEmpty()`. The shared_ptr indirection was removed because `QByteArray` is already COW with atomic refcount. |
| Adding a new input to `Dashboard::streamAvailable()`, the `FrameBuilder` any-async-sink poll, or `SerialStudio::isAnyPlayerOpen()` without wiring its change signal to the matching cache refresh (`updateStreamAvailable` / `BlockPublisher::refreshSinkFlag` / the player lambdas) | The hotpath reads **cached** flags, not the live getters. A missed signal leaves the cache stale: frames silently never reach the dashboard, or exports silently stop. Wire the new signal with `Qt::DirectConnection` (queued refreshes lag a full event-loop turn behind frames already flowing). Flag mechanics: [architecture/dataflow.md](architecture/dataflow.md) "Cached Hotpath Flags". |
| Polling a singleton getter per frame on the parse/publish path (`AppState::operationMode()`, consumer `enabled()`, `isAnyPlayerOpen()`, per-frame `std::map::find`) | Cache it as a member refreshed by the owning signal; the per-frame cost of guarded-static hops + map walks is what dragged the native parser gate down. See the cached members in `FrameBuilder` (`m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`) and `FrameParser` (`m_engine0Cache`). |
| Returning `QList<QStringList>` from a new native template when a single-row view split would do | Implement `INativeParser::parseSpans` (views into the frame bytes, `-1` when the contract can't hold: multi-row, latch state, content rewriting like quote-unescaping). The span lane is what lets Native parse without per-token allocations; the QList path is the fallback, not the default. |
| Writing `dataset.value` / slot strings via implicit-share assignment on the span lane | Use `assign_utf8_in_place` / `assign_string_in_place` (Frame.h). A share-assign re-links buffers, so the next in-place write detaches and re-allocates every frame — the zero-alloc steady state silently degrades back to per-frame mallocs. |
| Caching a raw pointer (or QByteArrayView) into an extracted frame's `data->data` past the synchronous parse | Extracted `CapturedData` objects are pool slots reused IN PLACE once every `CapturedDataPtr` drops (`FrameReader::claimCapturedSlot`). Holding the `CapturedDataPtr` is safe (keeps the slot pinned); a COW `QByteArray` copy is safe (the refill swaps buffers instead of reusing a shared one); a raw pointer/view is NOT — it reads the next frame's bytes. Off-main consumers never see pool slots (they get driver chunks or detached copies), keeping the count-1 probe exact. |
| Publishing a `DataBlock` to the dashboard without stamping `structureGeneration = m_framePoolGeneration` | The dashboard skips its per-block `compare_frames()` structural revalidation whenever the cached per-source generation matches, so a block that leaves the field at its default `0` (or a stale value) makes `Dashboard::applyBlock` either reconfigure every block or — worse — never reconfigure after a real layout change. Every `claimBlockSlot` / `trySpanLane` publish site (pool slot **and** heap fallback) must set it; the generation only advances via `invalidateFramePool()`, which already fires on every structural change (project sync, mode/connection change, Quick Plot channel-count change). Generation `0` is not a bug for a stream worker — it means an unversioned producer, and `applyBlock` gates those on the source template instead. |

## Timestamps

| Mistake | Fix |
|---------|-----|
| `QMetaObject::invokeMethod(...)` forwarding received data without capturing time | Capture `SteadyClock::now()` in the callback, pass to `publishReceivedData(data, timestamp)`. |
| Export/report worker calling `steady_clock::now()` to stamp a frame | Use `monotonicFrameNs(frame->timestamp, baseline)` on the shared `TimestampedFramePtr`. |

## Dashboard & Widgets

| Mistake | Fix |
|---------|-----|
| Mixing workspace IDs with group IDs | Workspace IDs are ≥ 1000; `Taskbar::deleteWorkspace()` branches on that. |
| Adding a widget that displays `Dataset::value` (the string) without registering it in `Dashboard::buildValuePushes` | Numeric datasets propagate the string only to observable targets (DataGrid-group copies + `m_lastFrame`, which `dashboard.getData` serializes); every other target keeps a stale string on purpose. Add the new widget's dataset copies to the `string_targets` set or its cells silently show old values. |
| Per-frame `getDatasetWidget` / `getGroupWidget` / `widgetCount` lookups inside a `Dashboard::update*Series` helper | Resolve pointers at configure time into a push table (`SeriesPush` / `GpsPush` / `Plot3DPush`, mirroring `LinePush`); the per-frame walk must be lookup-free. Rebuild the table in the matching `configure*`; `clearPushTables()` drops them on reset. |

## ProjectModel & Project Files

| Mistake | Fix |
|---------|-----|
| `buildTreeModel()` inside an item-change handler | Defer with `QTimer::singleShot(0, ...)`. |
| Mutating `ProjectModel` on every keystroke | Update the tree item in-place via `m_*Items`. |
| Force-rebuilding `buildSourceModel` on selection | Wait for the one-shot `contextsRebuilt` connection that `handleSourceBusTypeChange` opens; it disconnects itself before rebuilding. |
| Stamping `current_writer_version()` on every live `Frame::serialize` | Only project saves (`ProjectModel::serializeToJson`) carry version metadata. Live frames keep `schemaVersion = 0`. |
| New document-mutating `ProjectModel` slot without a `ProjectUndoScope` — or trusting `undo-scope-missing` to catch it | The lint only fires when the body literally calls `setModified(true)` in a `ProjectModel*.cpp` (Editor TUs excluded); a mutator that dirties state without `setModified(true)` is invisible to it **and** to history — capture commits on the first `setModified(true)`, so a staged snapshot without one is silently discarded. Open the scope; call `setModified(true)`. Spec 0031 detail: [architecture/project.md](architecture/project.md) "Undo History". |
| `setNextUndoHint()` not immediately followed by the intended model call | Hints are consumed (and cleared) by the **next** `enterScope()` unconditionally — an intervening scoped call eats the hint; a hint before a call that never opens a scope leaks into an unrelated later step's label/coalesce key. |
| Early return added between `setApplying(true)` and `setApplying(false)` in `applyHistorySnapshot()` — or emitting `jsonFileChanged` from the apply path | A stuck `applying` flag permanently silences capture and blocks undo/redo; `jsonFileChanged` on apply fires the BackupManager per-undo snapshot + editor path/selection resets. `emitProjectLoadedSignals(false)` is load-bearing (`false` is also the default — easy to regress via the no-arg overload elsewhere). |
| Hand-editing `core/Pipeline/DataModel/Generated/` or `core/Api/API/Generated/` TUs, `proto-fields.json`, or re-deriving dataset keys in a new file | One declaration: `app/rcc/properties/dataset.json` → `scripts/generate-property-registry.py`. `code-verify.py` blocks it (`api-generated-edited`, `proto-field-renumbered`, `registry-parallel-field-map`), but only after you've wasted the edit — go to the manifest first. |

## IO & Drivers

| Mistake | Fix |
|---------|-----|
| `DriverFactory::create()` for UI config | `ConnectionManager::instance().uart()` etc. |
| Live driver with empty device list | Call `refreshSerialDevices()` / `refreshSerialPorts()` in `open()` if empty. |
| BLE `selectDevice(index)` with placeholder compensation in `setDriverProperty` | `setDriverProperty` is raw; `selectDevice` subtracts 1. |
| Querying the **live** driver for `configurationOk()` | Check the **UI** driver — live may not be synced yet. |
<!-- claim-verify off -->
| Expecting `HAL_Driver` to orchestrate the open, or a shared supervisor to recover a dropped link | The flow layer was removed 2026-07-30. `DeviceManager::open()` calls `HAL_Driver::open(mode)` synchronously; there is no `supportsAsyncOpen`/`beginOpen`/`abortOpen`/`openFinished`/`linkDropped` hook and no per-device `TaskRunner`. Drop recovery is per-driver (UART's `m_pendingReconnect` off the 1 Hz tick, MQTT's `scheduleReconnectIfActive()`). `core/Core/Async/` survives for `MQTT::Publisher` and the spec-0035 probes only — retry/backoff there still comes from `RetryPolicy::initialConnect()` / `autoReconnect()`, never a local interval. |
<!-- claim-verify on -->
| Pushing a finding into `Misc::ProblemCenter` from the frame path, or making a checker async | Diagnostics are pulled: checkers return synchronously, the 1 Hz tick polls plain `quint64` counters (no signal/alloc/lock per frame), and a recreated `FrameReader` zeroes them — consumers work on deltas and treat a decrease as a reset (spec 0033). |
| Stopping a read thread that uses the `connect(&thread, &QThread::started, this, &readLoop, Qt::DirectConnection)` idiom with only `m_running = false` + `wait()` | `started` fires **before** `QThread::run()`, so when `readLoop` returns the thread falls into `exec()` and never finishes on its own — the `wait()` times out every close and the `terminate()` fallback fires (`TerminateThread` on Windows kills the thread mid-dispatcher and corrupts quit). Call `thread.quit()` before `wait()`; `quit()` is safe even before `exec()` starts (it sets `quitNow`, so `exec()` returns immediately). `HID::cleanupDevice()` is the reference; the gs_usb CANable backend shipped without it and crashed every quit. |

## Scripting & Parsers

| Mistake | Fix |
|---------|-----|
| `Per-dataset transformCode` with a leftover placeholder | `DatasetTransformEditor::onApply` discards code that doesn't define `transform()`. |
| Driving `setInterrupted(true)` from a `QTimer` on the thread that runs `QJSValue::call()` | The event loop is blocked during the call, so the timer never fires — the original watchdog no-op. Arm a `DataModel::JsWatchdog`; only `JsWatchdogThread` flips the flag (off-thread). `code-verify.py:js-interrupt-off-thread` blocks `setInterrupted(true)` outside `JsWatchdogThread.cpp`. |
| Touching the QML-embedded code editors (`JsCodeEditor` & siblings) without preserving the hidden-widget plumbing | Three invariants (position sync, `ShortcutOverride` forwarding, completer-popup rerouting) hold the embedded editor together — read [architecture/scripting.md](architecture/scripting.md) "Embedded Code Editors" in full before editing. |
| Writing string-based byte access (`string.byte`, `string.unpack`, `charCodeAt`, `split`) in a Lua/JS parser that runs under the **Binary decoder** — including the importer-generated Lua templates | Binary frames arrive as a 1-indexed byte table (Lua) / byte `Array` (JS), **not a string** — every frame errors out. Index directly, or convert once at the top of `parse()`. Full semantics + reference patterns in [architecture/scripting.md](architecture/scripting.md) "Binary-decoder scripts". Both importers shipped this bug in 2026-06. |

## Export & Sessions

| Mistake | Fix |
|---------|-----|
<!-- claim-verify off -->
| Looking for session DB code under the pre-spec-0076 `app/src/Sessions/` location | It's `core/Storage/Sessions/` — `namespace Sessions` for DatabaseManager, Export, and Player (all three). |
<!-- claim-verify on -->
| Calling `clear()` on a big flat container at teardown and assuming the memory came back | `QList/QVector::clear()` and `std::vector::clear()` both KEEP capacity. This only bites **flat POD arrays** (index vectors, columnar doubles, scratch buffers) and **object pools** — containers *of* containers free their payload on `clear()` because the elements are destructed. On a close/stop path, `squeeze()` the Qt flats, default-assign (`= {}`) the std ones, and release pool slot storage explicitly (`FrameBuilder::releaseReplayPoolStorage()` when the last player closes). Found 2026-08-19: a closed 4.4 GB CSV pinned a 113 MB row index, and replay-bound pools pinned up to the 192 MB block budget for the whole session (spec 0064). |

## Qt & QML UI

| Mistake | Fix |
|---------|-----|
| Setter without guard return | `if (m_foo == foo) return;` before assign + emit. |
| Editable field with a live `text:`/`value:` binding to the model value it commits into | The commit's dataChanged (or any normalization) echoes back mid-typing: cursor jumps, a cleared field refills, "-" gets swallowed. Sync from the model only while unfocused (`property var modelValue` + `onModelValueChanged: if (!activeFocus) text = ...`; `TableDelegate.qml` hex/int/float fields are the reference) and enforce numeric bounds by clamping at commit, never with a key-blocking validator. Found five times in 2026-07 (registry int/float/text fields, AutoIntField SpinBox, license key, AxisRangeDialog). |
| Running heavy work synchronously inside a `QFileDialog::fileSelected` slot (open another dialog, parse a file, mutate model) | On macOS `fileSelected` fires from inside `QFileDialog::done()`, which runs from an NSSavePanel KVO callback (`ViewBridge`/`NSRemoteViewMarshal`). Re-entering Qt synchronously can leave the `WA_DeleteOnClose` dialog deleted under the panel and crash on return. **Always wrap the body in `QMetaObject::invokeMethod(this, [...] { ... }, Qt::QueuedConnection)`** so `done()` unwinds first. Same applies to slots calling `dialog->deleteLater()` — defer the *work*, not just the deletion. |
| Binding the same QML `Shortcut` sequence twice in one window (e.g. view-level `StandardKey.Open` gated on `activeFocus` + an unconditional window-level one) | When both are enabled Qt fires `activatedAmbiguously()` — which nobody handles — so the key does **nothing**. Bind each sequence once per window, or make the `enabled:` gates mutually exclusive. |
| Combining `%n` with `.arg()` in a translated string — `tr("%n elements").arg(count)` / `qsTr("%n elements").arg(count)` | `%n` is Qt's plural marker consumed by the *count* overload (`tr("%n elements", nullptr, count)`), **not** `.arg()`. Passing it through `.arg()` leaves the literal `%n` in the UI (or mangles it) and breaks every `.ts`/`.qm` translation. When building a string with `.arg()`, always use numbered placeholders: `tr("%1 elements").arg(count)`, same for `qsTr()`. Use `.arg()` notation only — never mix in another substitution style. A common breakage when adding new features. |

## CI & Platform

| Mistake | Fix |
|---------|-----|
| Injecting a second `-platform` after `--headless`/`--benchmark` already injected `offscreen` (Windows `adjustArgumentsForFreeType` appends `-platform windows:fontengine=freetype`) | Qt takes the **last** `-platform`, so the appended one silently defeats `offscreen` and the "headless" run uses the real `windows` GUI platform — which blocks on a session-less CI runner (looks like a hang). Skip the freetype override when a `-platform` is already present. Windows-only: Linux/macOS never call `adjustArgumentsForFreeType`. |
| Running `--benchmark-hotpath` (or any CLI path) of the GUI-subsystem (`WIN32_EXECUTABLE TRUE`) Windows exe and expecting the shell to wait + capture stdout | A `/SUBSYSTEM:WINDOWS` binary detaches from the launching console — CI hangs with no output and no exit code. Benchmark a throwaway `editbin /SUBSYSTEM:CONSOLE` **copy** of the exe; ship the GUI-subsystem original. Full mechanics: [architecture/dataflow.md](architecture/dataflow.md) "Hotpath Benchmark", CI gotcha. |
| Registering the main thread with MMCSS before the Qt message handler is installed, or treating the `QThread::start` priority warning it triggers as a real failure | The warning is benign (the thread lands at its exact pre-MMCSS priority); register only via `Platform::AppPlatform::registerIngestThreadWithMmcss()` AFTER `qInstallMessageHandler`, and never start a QThread expecting to inherit the boosted band. Full contract: [architecture/startup.md](architecture/startup.md) "MMCSS coexistence contract". |
| Registering MMCSS from the composition root and calling the acquisition threads boosted, or guarding the call with a process-wide flag | The band is **per thread** and a `QThread` never inherits it, so the ingest threads register themselves: `IO::PipelineHost::registerIngestThread()` posts it onto the pipeline thread and each `IO::StreamWorker` onto its own. A process-wide guard is the bug — the first caller silences every later one; `AppPlatform`'s latch is `thread_local` and `mmcssRegisteredOnCurrentThread()` reads it (spec 0075 N2, pinned by `tst_mmcss_registration`). The GUI thread is deliberately left unregistered. |

## Startup & Session Context

| Mistake | Fix |
|---------|-----|
| Calling `SessionContext::current()` from a method body | Sanctioned sites only: the composition root, and a class's own `instance()` accessor passing the context into its ctor. The singleton census (`code-verify.py --singleton-census --check`) fails on any increase. Full contract: [architecture/startup.md](architecture/startup.md) "Session Context". |
| Changing the pinned instantiation order without touching `SessionContext::shutdown()` (or vice versa) | `shutdown()` releases in the exact reverse of `instantiateCoreModules()`; the two must move in lockstep, and `shutdown()` stays in `main.cpp` (qApp alive, QML engine dead) — never a destructor/atexit path. |

## Process & Trust

| Mistake | Fix |
|---------|-----|
| Bundled scope creep — slipping an unrelated bug-fix, "small cleanup", rename, or import-sort into the same diff as the user's actual ask | Name it in chat first ("noticed X — want it in this pass?"). Every unrelated file you touch costs the reviewer an audit pass, and "all the changes were individually correct" doesn't restore the trust the surprise diff cost. The user can always say yes; they can't say no after the fact. |
| Treating a subagent's report as ground truth — writing docs, edits, or follow-up agent prompts on top of its claims without checking | Spot-check before you build: open 2-3 of the cited files/symbols (or grep for them) yourself before propagating anything a subagent reported. Agent reports are leads, not facts — the 2026 AI-docs audits repeatedly traced shipped wrong claims back to unverified agent output. Same discipline for cached workflow results: an empty result is a finding to verify, not proof of absence. |

## Diagnosing a GUI Stall — Sample, Don't Theorize

**Sample the running app before theorizing about a GUI stall.** Freezes, stutters, ignored
window resize/close: get the stack, don't reason from the source. `pgrep -f Serial-Studio-Pro`
(skip Helper processes), then run `sample <pid> 12 -f out.txt` **in the background** and tell the
user to reproduce during that window -- a drag steals their keyboard, so a foreground sample
never lines up. Read the `com.apple.main-thread` tree first. Known signatures: a nested
`QEventLoop` appears as a re-entrant `-[NSApplication run]` under a timer callback (it swallows
in-flight OS resize steps); render-thread saturation appears as `QWaitCondition::wait` under
`QQuickWindow::event`, with `QSGRenderThread` busy in `renderSceneGraph`. Earned the hard way on
2026-08-13: three plausible fixes read out of the source were all wrong, one sample was decisive.

Related: `MacroEditor` grabs its offscreen widget only when dirty or focused
([scripting.md](architecture/scripting.md) "Embedded Code Editors"); the project-editor siblings
still grab per tick. Never give a main-window-embedded editor an unconditional per-tick `grab()`
— that cost 13% of the GUI thread all session (2026-08-17, found by sampling, not by reading).
