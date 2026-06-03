# Common Mistakes — Silent Breakage

> Extracted from CLAUDE.md. On-demand lookup table.

These are gotchas the linter can't always catch. Rule-driven mistakes are listed once in
the style sections (CLAUDE.md "Code Style") — don't restate.

| Mistake | Fix |
|---------|-----|
| `QMetaObject::invokeMethod(...)` forwarding received data without capturing time | Capture `SteadyClock::now()` in the callback, pass to `publishReceivedData(data, timestamp)`. |
| Export/report worker calling `steady_clock::now()` to stamp a frame | Use `monotonicFrameNs(frame->timestamp, baseline)` on the shared `TimestampedFramePtr`. |
| `buildTreeModel()` inside an item-change handler | Defer with `QTimer::singleShot(0, ...)`. |
| Mutexes in FrameReader / CircularBuffer | Recreate via `resetFrameReader()` / `reconfigure()`. SPSC is the contract. |
| `Qt::QueuedConnection` on the frame hotpath when both ends are on main | `Qt::DirectConnection`. Queued = queue-full drops at 10+ kHz. |
| `QMap::operator[]` on `m_sourceFrames[sourceId]` | `.find()` and validate — `operator[]` silently inserts. |
| Mutating `ProjectModel` on every keystroke | Update the tree item in-place via `m_*Items`. |
| `createDriver()` for UI config | `ConnectionManager::instance().uart()` etc. |
| Force-rebuilding `buildSourceModel` on selection | Guard with `m_awaitingContextRebuild`. |
| Live driver with empty device list | Call `refreshSerialDevices()` / `refreshSerialPorts()` in `open()` if empty. |
| BLE `selectDevice(index)` with placeholder compensation in `setDriverProperty` | `setDriverProperty` is raw; `selectDevice` subtracts 1. |
| Querying the **live** driver for `configurationOk()` | Check the **UI** driver — live may not be synced yet. |
| Setter without guard return | `if (m_foo == foo) return;` before assign + emit. |
| Looking for session DB code under `app/src/SQLite/` | It's `app/src/Sessions/` — `namespace Sessions` for DatabaseManager, `namespace SQLite` for Export/Player. |
| Mixing workspace IDs with group IDs | Workspace IDs are ≥ 1000; `Taskbar::deleteWorkspace()` branches on that. |
| Stamping `current_writer_version()` on every live `Frame::serialize` | Only project saves (`ProjectModel::serializeToJson`) carry version metadata. Live frames keep `schemaVersion = 0`. |
| `Per-dataset transformCode` with a leftover placeholder | `DatasetTransformEditor::onApply` discards code that doesn't define `transform()`. |
| `std::make_shared<DataModel::TimestampedFrame>(...)` directly on the FrameBuilder hotpath | Use `acquireFrame(src)` / `acquireFrame(src, ts)`. Direct `make_shared` bypasses the slot pool and brings back per-frame heap allocs. |
| Treating `CapturedData::data` as a smart pointer (`*data->data`, `data->data->size()`, `if (!data->data)`) | It's a `QByteArray` now. Use `data->data`, `data->data.size()`, `data->data.isEmpty()`. The shared_ptr indirection was removed because `QByteArray` is already COW with atomic refcount. |
| Driving `setInterrupted(true)` from a `QTimer` on the thread that runs `QJSValue::call()` | The event loop is blocked during the call, so the timer never fires — the original watchdog no-op. Arm a `DataModel::JsWatchdog`; only `JsWatchdogThread` flips the flag (off-thread). `code-verify.py:js-interrupt-off-thread` blocks `setInterrupted(true)` outside `JsWatchdogThread.cpp`. |
| Running heavy work synchronously inside a `QFileDialog::fileSelected` slot (open another dialog, parse a file, mutate model) | On macOS `fileSelected` fires from inside `QFileDialog::done()`, which runs from an NSSavePanel KVO callback (`ViewBridge`/`NSRemoteViewMarshal`). Re-entering Qt synchronously can leave the `WA_DeleteOnClose` dialog deleted under the panel and crash on return. **Always wrap the body in `QMetaObject::invokeMethod(this, [...] { ... }, Qt::QueuedConnection)`** so `done()` unwinds first. Same applies to slots calling `dialog->deleteLater()` — defer the *work*, not just the deletion. |
| Bundled scope creep — slipping an unrelated bug-fix, "small cleanup", rename, or import-sort into the same diff as the user's actual ask | Name it in chat first ("noticed X — want it in this pass?"). Every unrelated file you touch costs the reviewer an audit pass, and "all the changes were individually correct" doesn't restore the trust the surprise diff cost. The user can always say yes; they can't say no after the fact. |
| Injecting a second `-platform` after `--headless`/`--benchmark` already injected `offscreen` (Windows `adjustArgumentsForFreeType` appends `-platform windows:fontengine=freetype`) | Qt takes the **last** `-platform`, so the appended one silently defeats `offscreen` and the "headless" run uses the real `windows` GUI platform — which blocks on a session-less CI runner (looks like a hang). Skip the freetype override when a `-platform` is already present. Windows-only: Linux/macOS never call `adjustArgumentsForFreeType`. |
| Running the `--benchmark-hotpath` (or any CLI) path of the GUI-subsystem (`WIN32_EXECUTABLE TRUE`) Windows exe and expecting the shell to wait + capture stdout | A `/SUBSYSTEM:WINDOWS` binary **detaches from the launching console**: `cmd`/PowerShell don't wait for it, its stdout is unwired, and the `AttachConsole`+`CONOUT$` fallback writes to the console *screen buffer* that GitHub's pipe-based log capture never reads → CI hangs with no output and no exit code (`Start-Process -Wait` and plain `bash` both fail differently). For CI, benchmark a throwaway `editbin /SUBSYSTEM:CONSOLE` **copy** of the exe — a console-subsystem image stays attached, so the shell waits, stdout pipes through, and the exit code propagates. Leave the shipped exe `/SUBSYSTEM:WINDOWS` so it never flashes a console for end users. Background: <https://www.devever.net/~hl/win32con> |
