# Serial Studio review rules

Repo-specific invariants expressed as review rules. These are the things `code-verify.py`
cannot mechanically check and that a generic Qt reviewer would miss. They take precedence over
the generic Qt checklist where they overlap. Full detail lives in `doc/claude/`,
`CLAUDE.md`, and the [[ss-hotpath]] / [[ss-verify]] skills — read those for the *why*; this is
the review-time *what*.

## Hotpath (blockers, not advisories)

Data flow: `Driver -> FrameReader::processData (main) -> DeviceManager::onReadyRead ->
ConnectionManager::onFrameReady -> FrameBuilder -> TimestampedFramePtr -> Dashboard / async
sinks`. A violation here causes silent frame drops, not a compile error.

- **SS-HOT-1**: No mutex in `FrameReader` or `CircularBuffer`. They are main-thread / SPSC.
  Reconfigure by recreating via `resetFrameReader()` / `reconfigure()`. A lock here is a
  blocker.
- **SS-HOT-2**: Hotpath signal hops are `Qt::DirectConnection`. A queued connection between
  two main-thread objects fills the 4096-slot queue at 10+ kHz and drops frames. Applies to
  `DeviceManager` ready-read, `ConnectionManager::onFrameReady`, and
  `sourceStructureChanged -> rebuildDevices` (queued there lets a stale `m_devices[0]` survive
  into Connect).
- **SS-HOT-3**: No allocation and no `Frame` copy on the dashboard path. The Dashboard frame
  comes from `FrameBuilder::acquireFrame()` (the slot pool), never a direct
  `make_shared<TimestampedFrame>`. The async-sink fan-out in `hotpathTxFrame` makes exactly
  one detached `make_shared` copy, gated on a sink being enabled — that is the intentional
  slow-export path and is not a finding.
- **SS-HOT-4**: Source owns time. Stamp at the driver boundary; never re-stamp in an
  export/report worker. `monotonicFrameNs(...)` is the safety net only.
- **SS-HOT-5**: JS frame-parser / transform calls go through `IScriptEngine::guardedCall()` —
  never a raw `parseFunction.call()`. `setInterrupted(true)` only in `JsWatchdogThread.cpp`.
- **SS-HOT-6**: 256 kHz is a CI gate (`--benchmark-hotpath`, `Benchmark::HotpathBenchmark`), not a
  slogan. Any change that adds work per frame on the gated Lua/JS path is a regression risk —
  flag it for benchmarking. Don't regress the parse hotpath.

## Threading (general)

- **SS-THR-1**: `FrameReader` is no longer threaded (commit beeda4c0); `DeviceManager` signals
  must be `DirectConnection`.
- **SS-THR-2**: All `buildTreeModel` / `groupsChanged` connections in ProjectEditor use
  `Qt::QueuedConnection` (deliberate — avoids re-entrant model rebuilds).
- **SS-THR-3**: Sessions/DB singletons hold `QSqlDatabase`/`QSqlQuery` via `std::optional` and
  `reset()` them in `onQuit` so destruction happens while `qApp` is still alive.

## Driver lifecycle

Canonical reference: `app/src/IO/Drivers/BluetoothLE.{h,cpp}`. Read it before judging any
driver.

- **SS-DRV-1**: `~USB()` (and any driver with a worker thread) joins the worker thread BEFORE
  tearing down the resource it uses (e.g. `libusb_hotplug_deregister_callback` / `libusb_exit`)
  — the reverse order races and crashes macOS on rapid bus switches.
- **SS-DRV-2**: `configurationOk` checks the UI driver; live drivers populate their device list
  in `open()`.
- **SS-DRV-3**: `rebuildDevices` must sync `m_busType`, persist, and emit when a
  `source[0].busType` change triggers a live-driver swap, or the `setBusType` fast-path leaves
  the wrong driver active.
- **SS-DRV-4**: QML ComboBox bound to a dynamic `Q_PROPERTY` model must guard
  `onCurrentIndexChanged` with `if (count <= 0) return` — otherwise it clobbers restored
  values during model rebuild.

## NASA Power of Ten (safety-critical)

Hotpath violations are blockers; elsewhere they are strong advisories.

- **SS-POT-1**: No allocation / `Frame` copy on the dashboard path (see SS-HOT-3).
- **SS-POT-2**: Fixed loop bounds and capped recursion. Frame extractors cap iterations at
  `kMaxFramesPerCall`; new loops on the data path must be bounded the same way.
- **SS-POT-3**: Assertion density >= 2 per function on non-trivial functions.
- **SS-POT-4**: `[[nodiscard]]` and a checked return at every system boundary.
- **SS-POT-5**: No `reinterpret_cast` / `dynamic_cast` on the hotpath.
- **SS-POT-6**: Zero warnings; SPDX header on every file.

## Style invariants (code-verify.py owns these — flag only what it misses)

`scripts/code-verify.py` is the authoritative style/structure/tone linter. Don't re-derive its
rules. The header-layout and signal rules below bite most in review when the linter is bypassed
or a region is suppressed:

- **SS-STY-1**: Header (.h) member order: `Q_OBJECT` -> `Q_PROPERTY` -> `signals:` ->
  ctor/deleted-copy -> `public:` (`instance()` first) -> `public slots:` -> `private slots:`
  -> `private:`. `[[nodiscard]]` on every non-void return. (This supersedes the generic Qt
  QOB-4 order.)
- **SS-STY-2**: Never `Q_INVOKABLE void` — use `public slots:`.
- **SS-STY-3**: No in-header member initialization — initialize in the constructor init list.
- **SS-STY-4**: `Q_EMIT` not `emit`; lowercase `signals:` / `public slots:`; never
  `SIGNAL()`/`SLOT()`.
- **SS-STY-5**: Never `disconnect(nullptr)` as the slot (disconnects ALL) — capture the
  `QMetaObject::Connection` and disconnect that handle.
- **SS-STY-6**: Comments label, don't narrate. `.h`: SPDX banner + one `/** @brief */` per
  type. `.cpp`: one-line `@brief` per function definition; **no comments inside the function
  body** (`cxx-inbody-comment`) — fold a load-bearing why into the `@brief`, or fence a genuine
  exception with `// code-verify off`. No inline EOL comments, no AI narration. ASCII-only; no
  em-dash via ` -- ` (rewrite the sentence).
- **SS-STY-7**: Control flow: max 3 nesting levels (guard clauses / early return); functions
  40-80 lines, hard limit 100.
- **SS-STY-8**: SPDX banner is the dual-license block
  (`SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial`).
  Commercial-only files (API handlers under `app/src/API/Handlers/`, Pro modules) use
  `LicenseRef-SerialStudio-Commercial` and are gated behind `#ifdef BUILD_COMMERCIAL`.

## Known landmines (verify they are not reintroduced)

These were real bugs; treat a re-sighting as a high-confidence finding:

- `FrameBuilder::parseProjectFrame` — `m_sourceFrames[sourceId]` via `operator[]` silently
  inserts a default-constructed entry. Use `.value()` / `find()` for reads.
- `DeviceManager::killFrameReader` — `clear()` before `wait()` was a use-after-free.
- ProjectEditor disconnect — `disconnect(nullptr)` disconnected ALL slots, not the one lambda.
- SourceHandler API — a bounds check assumed contiguous IDs after deletes.
