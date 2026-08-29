---
spec: 0024-fft-audio-export
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-07-20
---

# Tasks 0024 — Audio generation output for the FFT widget

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- Implementer preflight: read `ImageExport.h/.cpp`, `FrameConsumer.h`, and the touched
  `Dashboard.cpp` regions in full before T1; invoke `ss-hotpath` before T5.

## Tasks

### T1 — AudioExport header: types + class skeleton

- **Files:** `app/src/UI/Widgets/AudioExport.h` (new)
- **Does:** Commercial-SPDX header declaring `AudioExportItem {float value; quint32
  sessionKey;}`, `AudioSessionConfig` (rate, scale mode, center/halfRange, dataset/project
  titles), worker-side `AudioSession` state, `AudioExportWorker :
  FrameConsumerWorker<AudioExportItem>` and the `AudioExport : FrameConsumer<AudioExportItem>`
  singleton facade — mirroring `ImageExport.h`'s layout exactly (header-order rules, no
  in-header member init, `[[nodiscard]]` on every non-void return, `Q_INVOKABLE` only for the
  value-returning `audioPath`).
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/AudioExport.h`;
  read-back against `ImageExport.h` for structural parity.
- **Deps:** none
- [x] done

### T2 — Worker: WAV writer + drain loop + finalize

- **Files:** `app/src/UI/Widgets/AudioExport.cpp` (new, worker half)
- **Does:** `AudioExportWorker::processItems` appends float32 PCM per session QFile (header
  written with placeholder sizes at open, RIFF/data sizes patched on every flush batch so an
  abnormal end still yields an openable file); finalize patches the header, runs the
  deterministic peak-rescale pass (fallback mode only, peak × 10^(-1/20) headroom), deletes
  zero-sample files, and emits the audible companion (same data chunk, header rate = rate ×
  ceil(8000/rate), `-audible-<N>x.wav` suffix) when rate < 8000 Hz. No clock calls anywhere
  (sample position × 1/rate is the timeline); overflow drops are counted and warned at
  finalize. Fixed loop bounds + ≥2 assertions per function.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/AudioExport.cpp`;
  desk-check the WAV header field offsets against the RIFF spec in the task diff.
- **Deps:** T1
- [x] done

### T3 — Facade: session registry + auto-stop wiring

- **Files:** `app/src/UI/Widgets/AudioExport.cpp` (facade half)
- **Does:** Singleton facade: `(widgetType, index)`-packed session keys, `openSession` /
  `closeSession` / `closeAllSessions` (config and close marshaled to the worker via
  `QMetaObject::invokeMethod` queued hops — the `setSnapshotIntervalMs` precedent), inline
  `enqueueSample` (lock-free `try_enqueue`, queue config `{65536, 4096, 33}`), `audioPath`
  (WorkspaceManager "Audio Recordings" tree, `imagesPath` shape), and
  `setupExternalConnections`: `connectedChanged`/`pausedChanged`, all three player
  `openChanged`, and `LemonSqueezy::activatedChanged` (Pro dropped) → `closeAllSessions()`.
  **Binding invariant: sessions must disarm the Dashboard taps *before* replay frames reach
  `hotpathRxFrame` — a per-sample `isAnyPlayerOpen()` poll is banned by the cached-flag
  rule.**
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/AudioExport.cpp`;
  read-back: every auto-stop source in plan.md's list has a connection here.
- **Deps:** T1, T2
- [x] done — facade owns sessions; widgets own taps; `sessionsClosed()` broadcast is the
  disarm contract. Deviations noted: drop counter = worker short-writes (no queue-overflow
  hook in base class); titles unsanitized (exact ImageExport parity).

### T4 — Build + module registration

- **Files:** `app/CMakeLists.txt`, `app/src/Misc/ModuleManager.cpp`
- **Does:** Add `AudioExport.h`/`.cpp` to the commercial header/source lists (adjacent to the
  `ImageExport` entries, ~664/~764); include + instantiate the singleton, call
  `setupExternalConnections()`, and expose `Cpp_Audio_Export` inside the existing
  `BUILD_COMMERCIAL` block (~864-872). **Binding invariant: registration goes in
  `registerImageProvidersAndLoadQml`'s commercial block, NOT `instantiateCoreModules` — the
  pinned ctor order there must not change.**
- **Verify:** `python scripts/code-verify.py --check app/src/Misc/ModuleManager.cpp`; grep
  both files to confirm placement inside `if(BUILD_COMMERCIAL)` / `#ifdef BUILD_COMMERCIAL`.
- **Deps:** T1–T3
- [x] done

### T5 — Dashboard ingest tap (hotpath)

- **Files:** `app/src/UI/Dashboard.h`, `app/src/UI/Dashboard.cpp`
- **Does:** `SeriesPush` gains `bool record; quint32 sessionKey;` under `#ifdef
  BUILD_COMMERCIAL`; `updateFftSeries` (~2469) and `updateWaterfallSeries` (~2686) branch on
  `p.record` next to the existing `p.buf->push(*p.value)` and call
  `AudioExport::enqueueSample`; `configureFftSeries`/`configureWaterfallSeries` initialize
  taps off on every rebuild; add commercial-guarded `setFftAudioTap` / `setWaterfallAudioTap`
  setters (main-thread, plain writes). **Binding invariants: invoke `ss-hotpath` and read the
  touched Dashboard regions in full first; the disarmed cost must be one bool test on the
  already-walked struct — no allocation, no lookup, no signal hop, no Frame copy; taps cleared
  unconditionally by every push-table rebuild so no stale index can fire; the parse lane and
  existing cached flags stay untouched.**
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Dashboard.h
  app/src/UI/Dashboard.cpp` (hotpath violations are blockers); re-read the diff against the
  ss-hotpath rules; note in chat that `--benchmark-hotpath` is the user-run gate (taps-off
  state is what CI measures).
- **Deps:** T1, T3 (enqueue + session key contract)
- [x] done — push↔widget index 1:1 alignment verified in both configure loops; singleton
  cached outside the loops; benchmark remains a user-run gate.

### T6 — FFTPlot recording property + lifecycle

- **Files:** `app/src/UI/Widgets/FFTPlot.h`, `app/src/UI/Widgets/FFTPlot.cpp`
- **Does:** Commercial-guarded `Q_PROPERTY(bool audioRecordingEnabled ...)` + setter/getter:
  enable is runtime-gated (`FeatureTier`, sweep-setter parity), opens the session with the
  ctor-resolved `m_samplingRate`/`m_scaleIsValid`/`m_center`/`m_halfRange` + dataset/project
  titles, then arms the Dashboard tap; disable and the dtor reverse both (session close +
  disarm). **Binding invariants: moc-visible `#ifdef BUILD_COMMERCIAL` (Dashboard.h member
  shape) so GPL builds compile without the property; dtor close is the reconfigure-safety
  story — widgets are destroyed on dashboard rebuild, so no re-arm path may exist.**
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/FFTPlot.h
  app/src/UI/Widgets/FFTPlot.cpp`; read-back: setter guard-returns, property absent outside
  `BUILD_COMMERCIAL`.
- **Deps:** T3, T5
- [x] done — Pro gate = `SerialStudio::proWidgetsEnabled()`; dtor moved out-of-line;
  singleton captured as `m_audioExport` member ref (arch-singleton-instance rule).

### T7 — Waterfall recording property + lifecycle

- **Files:** `app/src/UI/Widgets/Waterfall.h`, `app/src/UI/Widgets/Waterfall.cpp`
- **Does:** Same property/lifecycle as T6 on the (already commercial-only) Waterfall class,
  keyed as the Waterfall widget kind so an FFT + Waterfall on the same dataset record
  independent sessions (spec AC6). No `#ifdef` needed inside these files beyond what they
  already have.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Waterfall.h
  app/src/UI/Widgets/Waterfall.cpp`; read-back against the T6 diff for parity.
- **Deps:** T6
- [x] done — rate/scale read from Waterfall's existing ctor-resolved members.

### T8 — QML toolbar buttons

- **Files:** `app/qml/Widgets/Dashboard/FFTPlot.qml`, `app/qml/Widgets/Dashboard/Waterfall.qml`
- **Does:** Two `DashboardToolButton`s per widget: record toggle bound to
  `root.model.audioRecordingEnabled` (camcorder-icon parity with `ImageView.qml:166-184`) and
  open-folder calling `Cpp_Misc_Utilities.revealFile(Cpp_Audio_Export.audioPath(...))`; both
  `visible: Cpp_CommercialBuild` (the `Plot.qml:89` gating pattern). QML comment style is the
  `//` sandwich; no per-widget toolbar-hiding logic (WidgetToolbar owns the policy).
- **Verify:** `python scripts/code-verify.py --check` on both QML files; read-back: no
  binding evaluates `Cpp_Audio_Export` when `Cpp_CommercialBuild` is false.
- **Deps:** T6, T7
- [x] done — icons are the maintainer-supplied `audio-file.svg` (record) + `sound-folder.svg`
  (open folder), registered in `rcc.qrc`. Folder button resolves live via
  `model.recordingsFolder()` (see T9 review fix), not the removed `datasetTitle` property.

### T9 — Whole-feature verification pass

- **Files:** none (review pass)
- **Does:** Re-read the full diff against plan.md's file table (lane check: those files and
  only those), run the static pipeline, and state the counterfactual check in chat: which
  rule does this diff most risk violating (expected: the ingest-tap hotpath rules) and the
  concrete evidence it doesn't. Hand the maintainer the AC checklist (spec AC1–AC10) with the
  two build-side gates called out: `--benchmark-hotpath` unchanged and a
  `BUILD_COMMERCIAL=OFF` compile.
- **Verify:** `python scripts/code-verify.py --check` clean on all touched files;
  `qt-cpp-review` run on the C++ diff; `python scripts/sanitize-commit.py` run.
- **Deps:** T1–T8
- [x] done — `qt-cpp-review` run (6 agents, Opus on hotpath/ownership/error). Hotpath pass:
  no blockers (SPSC intact, zero-alloc ingest, cached-tap discipline correct). Fixed:
  path-collision/AC6 (kind+index in filename), stale folder button (live `recordingsFolder()`),
  silent open-failure (`sessionOpenFailed`->`sessionClosed(key)`->widget disarm), disk-full
  partial-write desync (`bytesOnDisk`), unchecked finalize writes, quit-drain, +nits. All 11
  files code-verify clean; new files clang-formatted. Out-of-lane (flagged, NOT touched):
  `FrameConsumer::stopWorker` not clearing `m_consumerEnabled` (shared base class);
  title->path sanitization (matches ImageExport precedent). Counterfactual: highest-risk rule
  = ingest hotpath no-alloc — evidence: 8-byte POD `try_enqueue` on a preallocated SPSC queue,
  disarmed path is one `[[unlikely]]` bool test; benchmark is the user-run gate.

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1–AC7 are
      maintainer in-app observations; AC8 benchmark; AC9 replay; AC10 GPL build).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (user-run; taps-off is
      the CI state).
- [x] No `pytest` surface (per plan.md) — maintainer AC list stands in.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (the Waterfall ring-size hole stays out of lane).
- [x] `spec.md` status set to `done`.
