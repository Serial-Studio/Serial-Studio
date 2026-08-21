---
spec: 0064-export-replay-fidelity
phase: tasks
status: in-progress   # draft -> approved -> in-progress -> done
updated: 2026-08-20
---

# Tasks 0064 — Export and Replay Fidelity

> **Phase 3 of 4.** Ordered, individually-verifiable steps against the approved
> [`plan.md`](./plan.md). Written after Task 0 proved the root cause, so the checklist reflects
> what the evidence actually demanded rather than the pre-investigation design.

## Task 0 — Prove the root cause against the running app ✅

- [x] **T0.1** Reproduce with all three sources: 943 MB CSV, 1.45M rows, 4 of 636 columns.
- [x] **T0.2** Eliminate block-pool exhaustion — `notePoolExhausted()` never fired.
- [x] **T0.3** Eliminate sink-queue saturation and the sparse merger — interval mode bypasses the
      merger and forward-fills from every ingested block; still zero CAN columns.
- [x] **T0.4** Eliminate stream-source masking — `stream.getSources` reports {1,2};
      `config.sourceId = deviceId` is correct, so source 0 is never in `m_streamSourceIds`.
- [x] **T0.5** Eliminate GUI-thread starvation by the code editors — dropping the UI rate to 5 Hz
      changed nothing.
- [x] **T0.6** Establish the boundary: a CAN-only copy of the project records 631 of 632 columns.
- [x] **T0.7** **Proof.** With audio streaming at 1.77M samples/10 s the dashboard's CAN values
      update live while the CSV holds 4 columns. Only a `feedExports == false` publish does that,
      so the sink-masked lane is doing all the republishing and the export lane is being skipped.
- [x] **T0.8** Independent reproduction with no hardware: a project whose parser returns no
      datasets, plus `dashboard.reprocess` standing in for the stream lane's masked refresh —
      export lane publishes zero times, CSV never created.

## Task 1 — Separate the two republish lanes ✅

- [x] **T1.1** Add `DataModel::RepublishGate` (`app/src/DataModel/RepublishGate.h`), header-only:
      `needed()` / `noteChanged()` / `notePublished()` / `notePublishedTemplate()` / `clear()`.
- [x] **T1.2** Migrate `FrameBuilder` onto it, replacing the shared `m_republishedSourceIds` set.
- [x] **T1.3** Extract `republishOneFrame()` so the per-source loop and the combined-frame
      fallback share one sequence and cannot drift.
- [x] **T1.4** `emitRepublishedFrame` reports what actually reached a sink, comparing
      `m_blockNumbers` before/after instead of probing `m_openBlocks`.
- [x] **T1.5** `python scripts/code-verify.py --check` clean on all touched files.
- [ ] **T1.6** **Build and confirm the fix flips the failing test.** BLOCKED — build commands are
      denied by `.claude/settings.json`. Maintainer action; see handoff below.

## Task 2 — CSV timestamp contract ✅

- [x] **T2.1** Export origin = minimum `t0` across the whole first batch, not `items.front()->t0`.
- [x] **T2.2** Clamp every emitted offset to `>= 0` in the sparse lane and the interval lane.
- [x] **T2.3** Player accepts any finite numeric first cell, negatives included, so recordings
      already on disk stop prompting for a time column.

## Task 3 — The 60 Hz hidden-editor grab ✅

Separate defect found while measuring; separable from the rest of this spec if you want it in its
own commit.

- [x] **T3.1** `renderable()` = `isVisible() && window() && window()->isVisible()` —
      `QQuickItem::isVisible()` stays true inside a *closed* window, and the Project Editor's
      Loader is `active: !runtimeMode`, so its last-selected view kept grabbing forever.
- [x] **T3.2** Dirty-flag gate matching `MacroEditor`: every event handler calls `scheduleRender()`
      and the UI tick does at most one grab, focused editors excepted so the caret still blinks.
- [x] **T3.3** Applied to `ControlScriptEditor`, `JsCodeEditor`, `OutputCodeEditor`,
      `PainterCodeEditor`. Measured cost before the fix: 56% of the GUI thread with the editor
      not open (`renderWidget` 40%, `QLineNumberArea::paintEvent` 11%, `getFirstVisibleBlock` 5%).

## Task 4 — Regression locks ✅ (one blocked on a build)

- [x] **T4.1** `app/tests/tst_republish_lanes.cpp` — QtCore-only ctest over the lane rule; seven
      cases, the load-bearing one being that N masked refreshes never discharge the export lane.
- [x] **T4.2** Registered in `app/tests/CMakeLists.txt` with a QtCore-only link set.
- [ ] **T4.3** Run it under `ctest`. BLOCKED — needs `-DSS_BUILD_TESTS=ON`, which needs cmake.
- [x] **T4.4** `tests/integration/test_export_replay_fidelity.py` — five cases at the integration
      tier: table-fed coverage, interleaved masked refreshes, the two CSV timestamp contracts, and
      replay-never-re-records. **See T6.5 — the masked-refresh case is coverage, not a lock.** On
      the unfixed binary 4 of 5 pass; `test_replaying_a_csv_creates_no_new_recording` times out on
      the `resetData` stall that T6.3 fixes, so it should flip green once built.

## Task 5 — Documentation

- [x] **T5.1** `doc/claude/architecture/dataflow.md` — the two republish lanes and the sink-dirty
      rule.
- [x] **T5.2** `doc/claude/architecture/scripting.md` — the closed-window visibility trap on
      embedded code editors.
- [x] **T5.3** Spec `plan.md` amended: root cause replaced the open risk; the superseded
      Republish-Parity design and the dropped MDF4/CLI work are recorded, not silently abandoned.

## Task 6 — Player fidelity and stalls (2026-08-19) ✅

Measured on the running app before fixing, per symptom.

- [x] **T6.1** Players never seeded the dashboard on open. CSV measured at 0 widgets / 0 datasets
      on open, 315 / 631 after pressing play. `seedCurrentFrame()` added to CSV, MDF4 and Sessions,
      called once the replay column map is installed.
- [x] **T6.2** Replay published one pooled block per row instead of batching to
      `kFrameBlockSampleCap`. Per-row flush removed; the sink mask moved onto `DataBlock::masked`
      so a block flushed later by the display tick still bypasses the recording sinks (R8).
      `openBlockFor` refuses to reuse a slot whose mask differs, so no block mixes both kinds.
- [x] **T6.3** `Dashboard::resetData()` blocked the GUI on the pipeline thread — `sample` caught
      4425 of 8850 main-thread samples in one ~4.4 s call, and it runs on every player open and
      close. The action-template fetch is now asynchronous.
- [x] **T6.4** `stream.getSources` reported the build-time channel count; now reports the observed
      count from the last block, falling back to config until the first block lands.
- [x] **T6.5** Test honesty: the pytest masked-refresh case cannot force the pipeline-thread
      interleaving and passed on a clean app, so it was reframed as coverage. `tst_republish_lanes`
      is the deterministic lock.

## Task 7 — Session replay and player crash (2026-08-19) ✅

- [x] **T7.1** MDF4 seek-stress crash. Symbolized `injectRow + 1244` to a `QMap::constBegin()`
      pointer chase: `replayChannelsTyped` marshals blocking and pumps the GUI event loop, so a
      queued `close()` cleared `m_sourceChannelsByIndex`/`m_text` under the running loop. Fixed with
      a re-entrancy guard, iterating copies, and deferring `closeFile()` while injecting. Applied to
      all three players. **Verified: the exact seek+close sequence that crashed now runs clean.**
- [x] **T7.2** Seed ran before `Q_EMIT openChanged()`, which queues `Dashboard::resetData` — the
      seed published into state about to be cleared. Seeds are now queued after the emit.
- [x] **T7.3** `resetData` cleared the dashboard while FrameBuilder still held per-source
      "structure published" marks, and a replay never bumps the pool generation that would clear
      them, so the seeded block arrived for a layout the dashboard no longer had.
      `FrameBuilder::forgetPublishedStructures()` is now called from `resetData`. This is also what
      gates dashboard *visibility*: MainWindow only calls `showDashboardNow()` when
      `Cpp_UI_Dashboard.available` flips true, which needs `totalWidgetCount() > 0`.
- [x] **T7.4** Session/MDF4 seeds force every mapped source. At the first instant of a mixed-rate
      recording only the dense sources have data (session 12: 2 blocks of 207,058 cover `t0_ns`),
      so an activity-gated seed would build 2 of 635 widgets.
- [x] **T7.5** `sessions.replay` returned `replaying: true` even with no database open, where
      `replaySelectedSession()` had silently returned. It now reports the failure.

## Task 8 — Reset and minimal reapply (2026-08-19 evening)

The accumulated working tree caused a live-dashboard regression: values sat at 0 and spiked as the
dashboard rebuilt continuously (`drainStructureSnapshots` hot on the GUI thread, displayDrops ~25/s
vs ~2/s). Mechanism: `Dashboard::resetData(false)` is called from inside the reconfigure path, so
`forgetPublishedStructures()` inside `resetData` looped reconfigure -> forget -> structure republish
-> reconfigure at block rate. The maintainer ordered a reset to HEAD; the tree was reverse-patched
(backup: scratchpad/session_full.patch) and ONLY the verified subset reapplied:

- [x] KEPT: RepublishGate migration in FrameBuilder (export fix, verified 635/635 live).
- [x] KEPT: CSV export origin (min t0 + >=0 clamps) and player negative-elapsed accept.
- [x] KEPT: the four editor render gates (56% GUI thread, measured).
- [x] KEPT: stream.getSources observed channels; sessions.replay failure reporting.
- [x] KEPT: docs (CLAUDE.md rule, dataflow.md lanes section, scripting.md editor trap),
      tst_republish_lanes + registration, the pytest suite.
- [x] DROPPED (each needs its own design pass): player seed-on-open, DataBlock::masked + replay
      batching, injectRow re-entrancy guards + close deferral, Dashboard::resetData async
      configureActions, forgetPublishedStructures, MDF4/Sessions force-all-sources seeds. The
      dropped work is preserved verbatim in the scratchpad patch and this file's Tasks 6-7 record
      the evidence behind each.

Consequences of the drop, stated plainly: players show an empty dashboard until play is pressed
(pre-existing behavior); session replay still does not populate (pre-existing, needs the structure
invalidation designed properly — NOT via resetData, which the reconfigure path re-enters); the
replay speed ceiling stays (~25.6k rows/s); the seek-settle crash hazard in MDF4 (nested event loop
during blocking replay marshal) remains latent as it was at HEAD.

## Task 9 — Player replay, designed properly this time (2026-08-19 night)

- [x] **T9.1** Root cause of every "player shows no/partial dashboard" symptom: FrameBuilder's
      per-source published-structure marks survive `Dashboard::resetData(true)`, so replay stages
      blocks for a layout the dashboard no longer holds and `structureIsCurrent()` refuses to
      resend. CSV replay showed ONLY audio widgets because stream-worker teardown republishes
      templates for the stream sources -- the frame-lane sources never come back.
- [x] **T9.2** Fix: `FrameBuilder::forgetPublishedStructures()` called from `resetData` **only
      under `notify == true`**. The reconfigure path re-enters `resetData(false)`; forgetting there
      is the 2026-08-19 reconfigure loop. The notify gate is the difference between the fix and
      the regression.
- [x] **T9.3** Close-while-playing: the close click is processed inside the replay marshal's
      nested event loop and tears player state down under live cell views. Reapplied the verified
      guard set: `m_injecting` re-entrancy latch in each player's inject, iterate-copies in
      MDF4/Sessions, and `closeFile()` re-queuing itself while injecting.
- [x] **T9.4** Deliberately NOT reapplied: seeds-on-open (dashboard appears once playback flows;
      an on-open seed needs a design that survives the queued reset), replay batching
      (DataBlock::masked), async configureActions.

## Task 10 — Replay throughput and block batching (2026-08-19 night, measured)

- [x] **T10.1** Timing probes split the 9 ms inject: builder publish work 4 us, GUI round trip
      8,105 us. The generic event-loop marshal's wake (queued QEventLoop::quit) lands on the next
      UI tick. The three replay lanes were switched to plain Qt::BlockingQueuedConnection — the
      design dataflow.md already documented as "the ONE exception". Result: 10 us per inject,
      replay at ~67k rows/s, full real-time pacing.
- [x] **T10.2** That exposed the next stage: 67k single-sample blocks/s into the 32-slot dashboard
      ring drained at ~1.9k/s = ~97% random drops; the sparse sources' rare blocks almost never
      survived (CAN gauges frozen while audio moved). Fix: publishReplayValues no longer flushes
      per row — replay batches to the same kFrameBlockSampleCap/epoch rules as the live lane
      (~1.1k blocks/s, fits the ring), with the sink mask carried on DataBlock::masked so a block
      flushed later by the display tick can never leak into a recording sink (R8). openBlockFor
      refuses to reuse a slot whose mask differs, so no block mixes masked and unmasked samples.

## Task 11 — Memory release on player close + instrumentation removal (2026-08-19) ✅

- [x] **T11.1** `FrameBuilder::releaseReplayPoolStorage()`: when the last player closes
      (`onPlayerOpenChanged`, open -> closed transition), every idle block-pool and frame-pool
      slot returns its storage (a replay binds slots to the full 635-column layout and cycles
      frame slots; pool storage otherwise persists for the whole session, up to the 192 MB block
      budget plus populated frame slots). Busy slots are skipped; the next claim rebinds.
- [x] **T11.2** Players shrink instead of clear on close: CSV squeezes the row index
      (offsets+seconds+bits, 17 B/row -- 113 MB on a 4.4 GB file), MDF4 default-assigns its
      columnar std::vectors (clear() keeps capacity) and squeezes the Qt containers, Sessions
      default-assigns the timeline and stream blob buffers.
- [x] **T11.3** All [SS0064] instrumentation and timing probes removed; repo-wide grep clean;
      full lint sweep 0 errors 0 advisories.

## Verified working by the maintainer (2026-08-19)

Live dashboard steady; CSV, MDF4 and session replay all populate every widget with smooth values
at real-time pacing; replay never re-records; recordings carry all 635 datasets; session PDF
report generation works.

## Blocked / not done

- **No build, no benchmark.** `cmake`, `ninja`, `make` and the compilers are in the `deny` list in
  `.claude/settings.json`. Every C++ change here is lint-clean and hand-reviewed but
  **uncompiled**. AC10 (`--benchmark-hotpath`) is unrun.
- **MDF4/Sessions replay channel mapping** — dropped. The tree-order vs uniqueId-order mismatch is
  real on paper (BADAQ diverges at index 583 of 635) but was never shown to misassign data once
  the true cause was found. Worth its own spec rather than a speculative rewrite here.
- **`--verify-export-replay` CLI round-trip mode** — dropped; it cannot be written or verified
  without a build.
- **Recording file size** — an explicit spec non-goal, and still the largest practical problem:
  ~93k rows/s x 636 columns is ~60 MB/s. `CSVExportInterval` is the existing lever. Needs its own
  spec.
- **RESOLVED, not a bug:** `io.setPaused(false)` does restart the stream workers — three
  pause/resume cycles measured 0 samples while paused and ~145k on resume. The earlier report was
  a wrong generalisation from one observation.
- **App wedges under sustained API churn.** After many project loads and player opens,
  `io.disconnect` timed out at 95 s and every later test failed until a relaunch. The `resetData`
  stall (T6.3) is one contributor; whether it is the only one is unproven.
- **`stream.getSources` reports source 1 with `channels: 1` but two datasets bound to channel 0 and
  channel 1.** RESOLVED by T6.4 — the count was the build-time value, and the binding always
  bounds-checked against the block, so no dataset was ever dropped.

## Maintainer handoff — exact verification sequence

```bash
# 1. build (denied to the assistant)
cmake --build build/Qt_6_11_0_for_macOS-Release --parallel 10

# 2. the regression must now pass; it fails on the current binary
python3 -m pytest tests/integration/test_export_replay_fidelity.py -v      # app up, API on

# 3. the cheap lane lock
cmake -S . -B build/tests -DSS_BUILD_TESTS=ON && ctest --test-dir build/tests -R republish_lanes

# 4. hotpath gate (AC10)
.../Serial-Studio-Pro --headless --benchmark-hotpath --min-fps 256000

# 5. real BADAQ: record with all three sources, expect ~635 populated CSV columns
```

## Task 12 — Dense-lane session replay decimation (2026-08-20 amendment, R11)

Reported: audio Quick Plot session replays distorted with a dead FFT; the session data is
complete; CSV/MDF4 replays of equivalent captures are fine. Root cause and design in the spec.md
and plan.md amendments of the same date.

- [ ] **T12.1** `PlayerStreamBlockIndex` gains a `fromBlocks` tag so one index and one replay walk
      serve both the legacy `stream_blocks` table and spec-0055 dense `blocks` rows.
- [ ] **T12.2** `loadBlockTimestampIndex` indexes each dense row (`dt_ns != 0`) into
      `streamBlocks` (block_id/source/uid/t0/dt/frames), skipping rows whose `frames` is
      non-positive or above `kMaxBlockFrames`; timestamp index contribution unchanged (t0 only).
- [ ] **T12.3** Merged stream-block index stable-sorted by `(t0Ns, sourceId, rowId)` after both
      loads — the grouping walk assumes same-source contiguity at equal t0.
- [ ] **T12.4** `fetchStreamSamples` takes the index entry and fetches
      `blocks.values_blob` or `stream_blocks.samples` by tag; both decode via
      `unpackStreamSamples`; blob-length mismatch still fails loudly.
- [ ] **T12.5** `frameValuesFromBlocks` gains `AND dt_ns = 0` so a dense block's t0 sample is not
      double-published through the frame lane.
- [ ] **T12.6** `teardownLocalDb` resets the new prepared query.
- [ ] **T12.7** Lint: `python scripts/code-verify.py --check` clean on the four touched files.
- [ ] **T12.7b** Maintainer test round 1 (2026-08-20): dashboard never appeared during an audio
      Quick Plot session replay. Cause: `replayChannels` reaches `ensureStructurePublished`
      through the staging path, but `replayBlock` published values only — a dense-only replay
      never announces a structure, so the dashboard builds zero widgets and `available` never
      flips. Fix: `replayBlock` ensures the source's structure (ProjectFile mode, source frame
      from `m_frame`) before its first publish, same as a live source's first block; repeat cost
      is one `structureIsCurrent` probe.
- [ ] **T12.8** **AC11 (maintainer, needs a build):** audio Quick Plot session replays with an
      undistorted waveform and a live FFT; scrub + settle do not blank the audio plots.
- [ ] **T12.9** **AC12 (maintainer):** mixed frame-lane + dense-lane session replays both lanes;
      no new recording is created during replay (R8).
- [ ] **T12.10** **AC13 (maintainer):** a legacy `stream_blocks` recording still replays.
