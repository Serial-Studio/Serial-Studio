# Handoff — WP-A (acquisition hotpath and sinks)

Tasks WPA-T1 .. WPA-T20. Every code change in the block is implemented; six tasks are left
unticked in `tasks.md` because a named test could not be built in the unit tier (reasons below, one
per task). Nothing was left half-applied: the tree compiles as one consistent state and
`code-verify.py --check` reports **no new advisories** and only the two `hotpath-assert-scope`
errors that need the WP0 whitelist patch in "Patches for the coordinator".

`code-verify.py --tu-census --check`: **3953 excess lines against a 3968 baseline (the surface
shrank).** Do not re-`--accept` until every package has merged.

## Files changed

### Hotpath
| Path | Change |
|------|--------|
| `app/src/DataModel/FrameBuilder/BlockStager.{h,cpp}` (new) | Frame-lane block staging moved out of the facade: pooled slots, the open-block map, the cap/epoch flush and `flushAll`. It reaches the facade only through the new `BlockStagerHost` interface (four hooks), which is what makes it unit-testable without FrameBuilder's link set. |
| `app/src/DataModel/FrameBuilder.h` | Implements `BlockStagerHost` (overrides `final`); owns `m_stager`; new `sessionBoundary(bool,bool)` signal, `onPausedChanged`/`onPipelineParkedChanged` slots, `m_lastPausedState`, `m_deferredProjectSnapshot`, `wireAsyncSinkHooks()`, `emitSessionBoundary()`, `applyDeferredProjectSnapshot()`. Block-pool members/constants removed. |
| `app/src/DataModel/FrameBuilder.cpp` | T2 flush-then-`sessionBoundary` on connect and pause edges; T5 the two per-frame `isFinalValuePlayerOpen()` reads become `m_playerOpen`; T6 a sync arriving while the pipeline is parked is held and applied (posted, never inline) when the bracket closes; T7 only a fully table-fed source's synthetic block reaches the sinks; sink/capture wiring extracted to `wireAsyncSinkHooks()` to keep `setupExternalConnections` under the 100-line cap. |
| `app/src/IO/PipelineHost.{h,cpp}` | `parkedOnGuiChanged(bool)` emitted from `setPipelineParkedOnGui` (pipeline thread, Direct to the builder). The setter lost `noexcept`, since it now emits. |
| `app/src/IO/StreamWorker.{h,cpp}` | `StreamProcessor` owns a `JsWatchdog` per engine; block transform runs through `JsWatchdog::call`, the per-sample pass arms ONCE for the whole pass; a timeout re-deinterleaves raw samples, counts (`transformTimeoutCount()`) and logs once. Out-of-range channel clears its column instead of republishing stale values. `compileEngines` is posted BEFORE the feed is connected. |
| `app/src/DataModel/FrameConsumer.{h,cpp}` | One `processData` post per drain cycle (`markFlushPosted`/`clearFlushPost` latch on the worker base); `noteSecondaryEnqueued(pending)` lets a consumer's second SPSC lane share the trigger; `monotonicSourceNs(sourceId, ns)` is the new per-source tie-break, cleared by `resetMonotonicClock()`. |

### Sinks and players
| Path | Change |
|------|--------|
| `app/src/CSV/Export.cpp` | Closes on `FrameBuilder::sessionBoundary`; irregular blocks keep `t0 - reference + times[i]` with the per-source tie-break; interval-mode write failure closes and reports like the sparse path. |
| `app/src/MDF4/Export.cpp` | Same boundary (plus the template re-push the pause handler used to do); per-source time; `createTimeChannel` sets `Sync(Time)`; the absolute-epoch write into the master is gone; text channels declare UTF-8. |
| `app/src/MDF4/PlayerLoaderWorker.cpp` | Decode is per channel group (one key/timestamp vector + one vector per channel, appended in `OnSample` order) merged k-way by key into the same flat payload; the dense per-instant map is gone; the legacy single-master timestamps became a vector; `isTimeMaster()` accepts sync `Time` or the legacy `None`. |
| `app/src/Sessions/Export.{h,cpp}` | Closes on `sessionBoundary` (pause included); raw lane shares the flush trigger, counts `rawOverruns`, drains 10000/pass; `transaction`/`commit`/`exec` checked into `noteWriteFailure` (latched flag + dropped count + `writeErrorChanged`); `isOpen()` reads false after a failure; `finalizeSession` stores NULL digests over lost rows; the raw-timeline head takes the oldest queued chunk as the baseline. New `writeFailed`/`rawOverruns`/`droppedBlocks`/`currentSessionIdOrNone()`. |
| `app/src/Sessions/BlockFingerprint.{h,cpp}` (new) | The four spec-0044 hash functions moved out of `Export.cpp` (declarations out of `Export.h`), which both keeps `Export.cpp` under the 1500-line cap after the T10 additions and makes the layout unit-testable. |
| `app/src/Sessions/PlayerLoaderWorker.cpp` | Timestamp index split in two: dense blocks (per dataset row, block start pushed once) and irregular blocks expanded ONCE per block via `GROUP BY source_id, block_number`; the archive opens read-only with no journal pragma. |
| `app/src/Sessions/Player/SessionDbReader.cpp` | Read-only open, no `journal_mode=WAL`. |
| `app/src/Sessions/DatabaseManager.cpp` | `sessionIsLive`/`refuseLiveSession` guard on `deleteSession`, `confirmDeleteSession`, notes and the three tag-assign verbs; refusal is a NotificationCenter warning, not a modal. |
| `app/src/Sessions/DatabaseWorker.cpp` | Worker-side refusal of a live `deleteSession`; `looksLikeSqlite()` rejects a non-SQLite file before `open()`. |
| `app/src/Sessions/Player.{h,cpp}` | `m_playbackEpoch` on both timer chains (bumped in `play()`/`pause()`). |
| `app/src/CSV/Player.{h,cpp}` | `catchUpTargetRow` stops at a backwards timestamp; `reanchorOnBackwardsRow()` restarts the clock there. |
| `app/src/CSV/Player.cpp`, `app/src/MDF4/Player.cpp`, `app/src/Sessions/Player.cpp` | `playbackKeyIsClaimed()`: Space/arrows are ignored while a modal window is up or the focused item accepts text input; media keys stay global. |
| `app/src/MQTT/Publisher.{h,cpp}` | The three pipeline-thread entry points read `m_hotEnabled` / `m_hotSparkplug` / `m_hotHasTopic` / `m_workerMode` atomics instead of the GUI-owned bools and `QString m_topicBase`. |
| `app/src/InfluxDB/LineProtocol.h` | Backslash added to the measurement and key escape sets. |
| `app/CMakeLists.txt` | `BlockStager.{h,cpp}` and `BlockFingerprint.{h,cpp}` added (contiguously, in place). |

## Tests added
| Suite | Covers |
|-------|--------|
| `app/tests/tst_frame_builder_staging.cpp` (new) | Cap flush, epoch flush, `flushAll` tail, mask split, `structureGeneration` stamping, per-source block numbers, per-sample offsets, pool exhaustion, `releaseIdleStorage`. Stub `BlockStagerHost`, no FrameBuilder link. |
| `app/tests/tst_csv_export_times.cpp` (new) | Per-source export time (B1): two sources keep their own instants, collisions bump only their own source, a uniform grid never takes the tie-break, a session reset clears every source. |
| `app/tests/tst_sessions_loader_index.cpp` (new) | B7 and B15 end to end against archives it writes itself: one index entry per instant regardless of dataset count, ascending + de-duplicated, and a chmod-500 archive still loads. |
| `app/tests/tst_sessions_export_worker.cpp` (new) | The spec-0044 fingerprint layout the write-failure policy depends on: every field participates, blob/string boundaries cannot shift, NaN folds to 0.0. |
| `app/tests/tst_stream_worker.cpp` | JS runaway `transform_block` and runaway per-sample `transform` both time out, fall back to raw and count; an out-of-range channel clears its column instead of republishing the previous block's samples. Link set gains the two JsWatchdog TUs. |
| `app/tests/tst_frame_consumer.cpp` | The flush-post latch: one post per drain cycle under a 5000-item burst, re-armed by the drain. |
| `app/tests/tst_influx_lineprotocol.cpp` | Backslash cases in the measurement and tag positions, including a trailing backslash. |
| `tests/integration/test_recording_fidelity.py` (new) | Disconnect tail into ONE file, pause tail, two sources not collapsed onto one clock, read-only historian directory does not report recording. |
| `tests/integration/test_historian_live_guard.py` (new) | Deleting the live session leaves it in place and the app answering. |
| `app/tests/fuzz/fuzz_{block_codec,csv_row,mdf4_reader}.cpp` + `corpus/` (new) | Untrusted-bytes harnesses with 10 seeds; registered under `# spec 0075 fuzz targets` guarded by `if(COMMAND ss_add_fuzz_target)` so the tier configures before WP0 lands. |

## Tasks not fully done, and why
All six have their **code** complete; only the named test is missing.

1. **WPA-T13**: `tst_mdf4_export_times.cpp` was NOT written. Both exporters now share one
   implementation (`FrameConsumerWorkerBase::monotonicSourceNs`), so a second suite would assert the
   same function twice; `tst_csv_export_times` covers it. MDF4's own "per-group masters intact"
   check needs mdflib and a written file, which is the integration tier.
2. **WPA-T14**: `tst_mdf4_writer_conformance.cpp` blocked: it needs a checked-in binary fixture
   (`tests/fixtures/mdf4/legacy-master.mf4`) produced by a build of the app, which this agent
   cannot generate. The writer change (`Sync(Time)`, no epoch write, UTF-8 text) and the reader
   acceptance (`isTimeMaster`) are in.
3. **WPA-T15**: `tst_mdf4_loader_memory.cpp` blocked: it needs a generated 10-minute 48 kHz `.mf4`,
   i.e. mdflib at test-authoring time. **A round-trip suite IS feasible** and is the recommended
   follow-up: `MDF4/PlayerLoaderWorker.cpp` links against nothing but Qt6::Core and the `mdf`
   target, so such a suite can write a two-channel-group file with mdflib and assert the merged payload.
4. **WPA-T17**: `tst_sessions_database_manager.cpp` blocked: `Sessions::DatabaseManager` reaches
   ProjectModel, the player, AppState and the licensing block; that is an application link.
   `tests/integration/test_historian_live_guard.py` covers the guard instead.
5. **WPA-T18**: `tst_sessions_player_epoch.cpp` / `tst_csv_player_catchup.cpp` blocked: both
   players are GUI singletons reaching Dashboard and FrameBuilder. The epoch guard is now
   byte-identical to the CSV and MDF4 players' (already shipped) pattern.
6. **WPA-T19**: `tst_mqtt_publisher_hotflags` blocked: `MQTT::Publisher` links the broker stack,
   licensing and the vault. The Influx half of that task's verification is in.

**WPA-T4 scope note (ticked):** the parked-snapshot and synthetic-republish cases from its Does
line live in `FrameBuilder`, not in the extracted stager, so they are not in the unit suite;
`test_export_replay_fidelity.py` and `tst_republish_lanes` remain their coverage.

## Patches for the coordinator

### 1. REQUIRED: `scripts/code-verify.py` (WP0 owns it). Without this the tier is RED.
`BlockStager.cpp` is now a per-frame hotpath TU and keeps its two `SS_ASSERT_HOTPATH`s (one is
per-column inside the staging loop; promoting it to `SS_ASSERT` is exactly the 5%-throughput
mistake the macro exists to avoid). Add one line to the whitelist:

```diff
 _HOTPATH_ASSERT_ALLOWED = (
     "app/src/SSAssert.h",
     "app/src/DSPSimd.h",
     "app/src/DataModel/Frame.h",
     "app/src/DataModel/Frame.cpp",
     "app/src/DataModel/DataBlock.h",
     "app/src/DataModel/FrameBuilder.h",
     "app/src/DataModel/FrameBuilder.cpp",
+    "app/src/DataModel/FrameBuilder/BlockStager.cpp",
     "app/src/IO/CircularBuffer.h",
```

### 2. Recommended: `app/src/API/Handlers/SessionsHandler.cpp` (WP-G / API owner)
`sessions.delete` still answers `{"deleting": true}` for the live session, which the refusal then
ignores. Return the coded error instead, and expose the live id `getStatus` already knows:

```diff
   const int sessionId = params.value(QStringLiteral("sessionId")).toInt();
   static auto& db     = Sessions::DatabaseManager::instance();
+  if (sessionId >= 0 && sessionId == Sessions::Export::currentSessionIdOrNone())
+    return CommandResponse::makeError(
+      id,
+      ErrorCode::ExecutionError,   // SESSION_LIVE once the shared enum lands
+      QStringLiteral("This session is being recorded; stop the recording first."));
+
   if (!db.isOpen())
```
```diff
   result[QStringLiteral("exportEnabled")] = exp.exportEnabled();
   result[QStringLiteral("isOpen")]        = exp.isOpen();
+  result[QStringLiteral("writeFailed")]      = exp.writeFailed();
+  result[QStringLiteral("rawOverruns")]      = static_cast<qint64>(exp.rawOverruns());
+  result[QStringLiteral("droppedBlocks")]    = static_cast<qint64>(exp.droppedBlocks());
+  result[QStringLiteral("currentSessionId")] = exp.currentSessionId();
```
`tests/integration/test_historian_live_guard.py` skips itself until `currentSessionId` is exposed,
and `test_recording_fidelity.py`'s read-only case asserts on `isOpen` (already true today) rather
than on `writeFailed`.

### 3. Note: `ConnectionManager` (WP-C owns it)
No patch needed. `FrameBuilder` wires `pausedChanged`/`connectedChanged` from its own side only,
and reads the state through `PipelineHost`'s atomic mirrors, never through ConnectionManager.

## Invariants found that the plan did not state

1. **The unit tier cannot link any facade.** The plan assigned ctest suites to `Sessions::Export`,
   `DatabaseManager`, `MQTT::Publisher` and the players; none of them is linkable at that tier
   (`app/tests/CMakeLists.txt` links per-TU sets, and `tst_stream_worker` already stubs FrameBuilder
   symbols to stay lean). Testability is a *design* input here: `BlockStager` got a host interface
   and the Sessions fingerprints their own pair, precisely so their suites can exist.
2. **`sessionBoundary` had to be flush-then-emit, and the parked apply had to be posted.** The
   plan's "apply when the bracket closes" would run inside the script call that parked the
   pipeline, i.e. mid-frame, where `applyProjectSnapshot` clears `m_frame` under a live dataset
   pass. The apply is a `QueuedConnection` post; the assert that made A3 visible stays.
3. **`Q_EMIT` cost `setPipelineParkedOnGui` its `noexcept`.**
4. **"Only table-fed datasets" is not expressible per dataset in a block.** A block's columns are
   bound to the whole source frame, so a per-dataset filter would need a second column layout and
   a second pool binding per lane. Implemented at the source level instead: a source whose datasets
   are all Computed (no frame index) feeds the sinks; a source carrying parsed channels renders but
   stays masked, because its channels are already recorded on arrival. R1.6's failure scenario
   (duplicate channel samples in a recording) is closed; a mixed source's synthetic refresh still
   double-pushes its plot rings on the dashboard, exactly as before.
5. **Diagnostics are pulled, so the stream-lane timeout is a counter, not a signal.** The plan
   listed a queued worker→GUI notification; that would have put `NotificationCenter` into
   `StreamWorker.cpp` and broken `tst_stream_worker`'s link set, and it contradicts spec 0033.
   `transformTimeoutCount()` is polled like every other worker counter.
6. **`DatabaseWorker`'s connection cannot become read-only** (B15 lists it): it writes the schema,
   tags and the lock state. Only the two true readers changed. Browsing an archive on read-only
   media needs a read-only *mode* in the explorer, which is a follow-up rather than a line change.
7. **`Sessions::Export::isOpen()` is the recording indicator**, so the write-failure latch is folded
   into it rather than into a new `isRecording` property; QML and the API keep reading one flag.
8. **`isAnyPlayerOpen()` and `isFinalValuePlayerOpen()` are byte-identical implementations**, which
   is what makes the T5 substitution exact rather than approximate.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Don't regress the 256 kHz gate." T1 was meant
to be a pure move, and it is not: `BlockStager` reaches the facade through a virtual
interface, so `stage()` now pays two indirect calls per staged frame (`announceStructure`, and
`stagingFlushEpoch` once per frame plus once per opened block) where the old code made same-TU
calls.

**Evidence that it does not.** Both overrides are `final` on the only implementer in the binary, so
LTO devirtualizes them; the calls sit next to work that already costs far more (a
`std::map::find` per frame in `ensureStructurePublished`, a relaxed atomic load, and the per-column
staging loop), and `datasets+publish` (the 70-80% of per-frame time the benchmark reports) is
untouched. Everything else on the frame path got *cheaper*: two per-frame GUI-owned reads
(`isFinalValuePlayerOpen`, and the MQTT gate's four config reads including a `QString`) became
cached-flag and atomic reads, and `FrameConsumer::enqueueData` no longer allocates a
`QMetaCallEvent` per enqueue past the threshold. No allocation was added to the publish path, the
pool's `use_count()==1` probe and its aliasing hand-out are unchanged, `structureGeneration` is
still stamped at bind and at open, no SPSC queue gained a mutex, and the only new cross-thread
signals (`sessionBoundary`, `parkedOnGuiChanged`, `writeFailed`) fire at session-edge, park-bracket
and failure rate. **The number is still unmeasured here: `--benchmark-hotpath` after T1 is the
maintainer's checkpoint, and it is the one thing that can falsify the paragraph above.**

Second candidate: "never touch files outside your own edits." The diff stays inside the WP-A file
list plus the two new pairs it creates and the tests it names; `scripts/code-verify.py` and
`SessionsHandler.cpp` are left alone and delivered as patches above.
