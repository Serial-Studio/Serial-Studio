---
spec: 0064-export-replay-fidelity
phase: plan
status: approved      # draft -> approved (gate before /ss-tasks)
updated: 2026-08-18
---

# Plan 0064 — Export and Replay Fidelity

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The synthetic refresh that `dashboardTick()` drives is the only publication path a table-fed
virtual dataset has, and spec 0055 gave it its own staging/flush sequence separate from the one a
parsed frame takes. That separate sequence is where the data is being lost. Rather than repair the
one dropped step, this plan **collapses the two paths into one**: `emitRepublishedFrame` stops
open-coding a flush/stage/flush dance and calls the same entry a parsed frame calls, so a
republished block and a parsed block are the same object built the same way and cannot diverge
again. On top of that, a **per-source published-sample counter** joins the pulled diagnostics
(specs 0033/0035) so a source that publishes structure but never a sample is reported at capture
time instead of discovered in an empty recording. The four independently-proven defects — the
inverted staged check, the CSV export reference timestamp, the CSV player's negative-elapsed
rejection, and the replay channel-order assumptions — are fixed alongside. Coverage lands in three
tiers, with the full round trip hosted in a **new headless CLI verification mode** that reuses the
spec-0044 verifier bootstrap, because `FrameBuilder`'s link set is far past what the ctest tier
admits and `--selftest` runs before the composition root exists.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/FrameBuilder.cpp` | Collapse `emitRepublishedFrame` onto the parsed-frame stage/flush entry; fix the inverted `staged` probe; add per-source published-sample accounting; extend `republishFrames` source-frame seeding so a stream source's presence cannot suppress seeding of a non-stream source. |
| `app/src/DataModel/FrameBuilder.h` | Declare the new accounting members (plain `quint64` maps) and the pulled getters. No new signals. |
| `app/src/CSV/Export.cpp` | Reference timestamp = minimum `t0` across the whole first batch, not `items.front()->t0`; latch it so it never moves after the first row is written; clamp a late-arriving earlier sample to the monotonic floor rather than emitting a negative elapsed. |
| `app/src/CSV/Export.h` | Member for the latched reference/floor state. |
| `app/src/CSV/Player.cpp` | Accept any finite numeric first cell as a numeric elapsed column, including negatives; keep the prompt for genuinely non-numeric first cells. Extract the predicate into a header so the ctest tier can exercise it without the player. |
| `app/src/CSV/ReplayTimestampMode.h` | **New.** Header-only, allocation-free predicate + unit-scale resolution lifted out of `Player.cpp`, so timestamp-mode detection is unit-testable with a QtCore-only link set. |
| `app/src/MDF4/Player.cpp` | `buildReplayLayout()` currently enumerates project datasets in tree order while the MDF4 writer enumerates channel groups in project group order and the CSV writer sorts by uniqueId; make the mapping resolve by identity rather than position, and drop the `widget == "image"` skip that the writer does not mirror. |
| `app/src/Sessions/Player.cpp` | Same identity-over-position audit for `alignColumnsToProject()` / `buildMultiSourceMapping()`; no change expected beyond what the round-trip test proves. |
| `app/src/Sessions/ReportData.cpp` | No behavioral change expected — the report gap is downstream of R1/R2. Listed because AC8 asserts against it and a defect found there lands here. |
| `app/src/Misc/CLI.cpp` | Register `--verify-export-replay <project>` and route it to the new round-trip runner, following `runSessionVerification()` exactly. |
| `app/src/Misc/CLI.h` | Option members + `runExportReplayVerification()` declaration. |
| `app/src/Sessions/RoundTrip.cpp` / `.h` | **New.** The round-trip runner: pinned composition root, synthetic multi-source project with table-fed virtual datasets, record to temp CSV/MDF4/session, replay each back, diff, JSON verdict on stdout. |
| `app/tests/tst_replay_timestamp_mode.cpp` | **New.** ctest suite over `ReplayTimestampMode.h`: negative, zero, positive, non-numeric, unit-suffixed headers. |
| `app/tests/tst_export_schema_parity.cpp` | **New.** ctest suite asserting the CSV column order, the MDF4 channel order and the replay layout order agree for a project whose dataset tree order differs from its uniqueId order. |
| `app/tests/tst_csv_sparse_writer.cpp` | Extend with the reference-timestamp property: multi-source batches never produce a decreasing or negative elapsed column. |
| `app/tests/CMakeLists.txt` | Register the two new suites with their link sets. |
| `tests/integration/test_block_lane_sinks.py` | Extend: record with table-fed virtual datasets, stop, reopen each recording, assert the populated dataset set matches and that replay created no new recording. |
| `tests/integration/test_session_report.py` | Extend or create: every dataset the report lists carries plot data and statistics. |
| `doc/claude/architecture/dataflow.md` | Document the single republish entry and the new published-sample diagnostic. |
| `doc/claude/architecture/export.md` | Document the latched CSV reference timestamp and the identity-based replay mapping. |
| `CLAUDE.md` | One line under Threading & Hotpath: republished frames take the same staging entry as parsed frames. |

## Architecture & data flow

Nothing moves between threads. Everything below already runs on `IO::PipelineHost`'s processing
thread; `dashboardTick()` self-marshals from the GUI (control script) via `invokeOnBuilderThread`
and that stays exactly as it is.

Today the synthetic refresh takes its own sequence:

```
dashboardTick()  (pipeline thread)
  republishFrames(feedExports = true)
    per source frame: reprocessDatasetValues()   transform-only pass, change-driven skips
    emitRepublishedFrame()
      flushBlock(sourceId)                        close any real captured block, unmasked
      stageFrameValues(sourceId, frame, now())    <-- open-coded, diverges from the parse path
      staged = m_openBlocks.find(sourceId) != end()
      flushBlock(sourceId)
```

After this change the middle three lines become one call to the same helper `parseFrame` uses to
append a row and let the cap/epoch rules decide the flush, with an explicit immediate flush after
it because a synthetic refresh must reach the display within the tick that asked for it. The
`staged` verdict is captured from the staging call's own return rather than probed from
`m_openBlocks` after the flush already erased the entry — which is why it is unconditionally false
today, and why `m_republishedSourceIds` never fills and the documented "publish only on change"
suppression never engages (R7).

`republishFrames`'s source-frame seeding is the second half of the parity work. `dashboardTick()`
seeds `m_sourceFrames` from `m_frame.groups` **only when the map is entirely empty**. With dense
stream sources present, the map is never empty, so seeding is skipped and a non-stream source that
has not yet been seeded elsewhere is never iterated. Seeding becomes per-source and unconditional:
every `sourceId` present in `m_frame.groups` gets a source frame, whether or not some other source
already created one.

**This seeding gap is a genuine latent defect but it does not by itself explain the BADAQ capture.**
`parseProjectFrame(int sourceId, ...)` calls `ensureSourceFrame(sourceId)` unconditionally, so a
source whose frames parse at all is already seeded — and the BADAQ capture's 4516 recorded table
snapshots prove its Lua parser ran. Static reading has therefore **not** identified the step that
drops the block, only eliminated candidates: the async-sink flag is correctly wired and shared
across sources (and the stream sources' blocks do reach the sinks through it); pool starvation is
unlikely because the 192 MB byte budget yields all 64 slots for a 635-dataset project; the group
`sourceId` default is 0, so the CAN source's frame is populated with all 107 of its groups; and the
inverted `staged` probe cannot suppress the publish because the publish has already happened inside
`flushBlock` by the time it is read. Closing this requires the running app, which is what Task 0
exists for.

The published-sample accounting is a `std::unordered_map<int, quint64>` incremented inside the
single publish tail — one integer add per block, no allocation, no signal — read on the existing
1 Hz diagnostics poll alongside the FrameReader/FrameBuilder counters. A source whose structure has
been published but whose sample count is still zero after a configurable number of polls raises the
same one-shot `NotificationCenter` warning shape that `notePoolExhausted()` already uses. This is
strictly a pulled counter: nothing signals per frame (specs 0033/0035).

Replay mapping moves from positional to identity-based. The MDF4 writer emits one channel group per
project group in project group order, and the loader strips master and `" (raw)"` channels, so its
flattened channel order is project tree order — while `buildExportSchema` sorts CSV columns by
`uniqueId`. Two writers, two orders, and `MDF4::Player::buildReplayLayout()` reconstructs one of
them from `ProjectModel::groups()` by position while additionally skipping `widget == "image"`
groups that the writer does not skip. For the BADAQ project the tree order and the uniqueId order
diverge at index 583 of 635, so any file whose reader and writer disagree silently misassigns the
tail. The fix is to resolve each channel to its dataset by recorded identity and to remove the
asymmetric skip, so neither writer's ordering choice is load-bearing.

## Hotpath & threading impact

- **Touches the hotpath?** **Yes.** `stageFrameValues`, `flushBlock`, `publishBlock` and the block
  slot pool are the spec-0055 publish path. Preserved as follows: the parity change *removes* a
  code path rather than adding one, so the parsed-frame lane executes strictly fewer branches, not
  more; no allocation is introduced anywhere on the publish path — the new accounting is an integer
  increment into a map sized once per structural change, not per block; the pooled-slot discipline
  (`claimBlockSlot`, `use_count() == 1`, aliasing `shared_ptr`) is untouched; the single
  `clone_block_trimmed` for async sinks stays the only copy and stays gated on `m_anyAsyncSink`. No
  in-pipeline connection changes, so the `Qt::DirectConnection` rule is not engaged.
  `--benchmark-hotpath` runs before and after; every tier must clear at its default rate on the
  PGO-optimized binary (AC10).
- **New cross-thread signal/slot?** **No.** No new connection of any kind. `dashboardTick()` keeps
  its existing GUI-to-pipeline `invokeOnBuilderThread` marshal, and no
  `BlockingQueuedConnection` is added from the GUI into the pipeline.
- **New input to a cached hotpath flag?** **No.** `m_anyAsyncSink`, `m_operationMode`,
  `m_playerOpen`, `m_captureLatestFrame`, `m_changeDriven` and `Dashboard::m_streamAvailable` all
  keep their current inputs and refresh wiring. The published-sample counter is deliberately *not*
  a cached flag read by the hotpath — it is written by the hotpath and read by the 1 Hz poll, which
  is the opposite direction and carries none of the cache-staleness hazard.
- **Timestamp ownership.** Unchanged and reaffirmed. Sources stamp at the driver boundary; the CSV
  export fix corrects **which sample the recording is measured from**, not when a sample is
  stamped. No export or report worker re-stamps; `monotonicFrameNs(...)` stays the same-nanosecond
  collision safety net it is today and does not become the source of truth.

## Data model & persistence

- **No schema change.** `Frame.h` gains no `Keys::` entry. The session database schema and
  `DatabaseManager::kCaptureFormatVersion` (2) are unchanged, so existing session files open
  unmodified and `sessionUsesBlocks()` keeps discriminating per session rather than by
  `PRAGMA user_version` (R9).
- **No on-disk format change** to CSV or MDF4. Column and channel sets, labels, the `" (raw)"`
  companion channels and the per-group master time channels all stay exactly as they are — the
  MDF4 raw duplication is an explicit spec non-goal.
- **Migration story:** none required. Recordings already on disk keep their negative first elapsed
  value; the player-side fix is what makes them replayable, which is why the negative-tolerance
  change must ship even though the exporter will stop producing negatives (R3 covers both).
- Recordings already written cannot be repaired and still contain only the dense-stream datasets.
  That is stated in the spec and is not addressed here.

## API / SDK surface

- **One new CLI verb, no new API handler.** `--verify-export-replay <project>` joins
  `--verify-session` and `--regress-session` as a headless, child-process-friendly verification
  mode. It is registered in `CLI::process()` ahead of the composition root check exactly where the
  other two are, and prints a JSON verdict on stdout with a binary exit code, matching the
  established contract.
- No `app/src/API/Handlers/` addition, no `CommandHandler::initializeHandlers()` change, no
  `EnumLabels.cpp` slug, no generated SDK regeneration, no gRPC field number.
- The new mode is not gated behind `BUILD_COMMERCIAL` itself, but its session leg is — session
  recording is Pro, so the session round-trip leg compiles out and reports as skipped in a GPL
  build rather than failing.

## QML / UI

None. The only user-visible surface is the existing Notification Center warning shape reused for
the published-but-silent-source diagnostic, which needs no new component.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Republish publication path | (a) Point-repair the one dropped step; (b) collapse onto the parsed-frame entry; (c) revert to the pre-0055 `hotpathTxFrame(acquireFrame(...))` shape | **(b)** — a second publication path that can silently differ from the first is the defect class, not just this instance; (c) reintroduces the payload duplication spec 0055 removed. |
| Negative elapsed | (a) Fix the exporter only; (b) tolerate it in the player only; (c) both | **(c)** — (a) leaves every recording already on disk unreplayable, (b) leaves new files with a meaningless time origin. R3 and R4 are deliberately separate requirements for this reason. |
| Late earlier-arriving sample after the reference is latched | (a) Rewrite the file; (b) emit a negative elapsed; (c) clamp to the monotonic floor | **(c)** — rewriting a streaming file is not an option and a negative value is what broke replay; clamping is bounded, visible in the drop counters, and preserves ordering. |
| Replay channel mapping | (a) Make every writer sort identically; (b) resolve by recorded identity | **(b)** — (a) is a format change and leaves existing files mismatched; identity resolution makes both orderings correct forever. |
| Round-trip test host | (a) ctest suite linking `FrameBuilder`; (b) `--selftest` suite; (c) new headless CLI mode on the verifier bootstrap | **(c)** — (a) blows past the link-set budget the ctest tier is explicitly bounded by; (b) runs *before* the composition root, so it may not touch an application singleton. (c) reuses a bootstrap already proven by specs 0044/0047. |
| Silent-source diagnostic | (a) Pulled counter only; (b) counter + one-shot warning; (c) per-block signal | **(b)** — (c) violates the pulled-diagnostics rule outright; (a) alone would have left this bug just as invisible as it was. |
| Report gap (R10) | (a) Fix in the report reader; (b) let it fall out of R1/R2 and lock it with a test | **(b)** — the report is a faithful reader of what was recorded; changing it would paper over the real hole. |

## Risks & mitigations

- **The exact drop step is unproven, and this is the plan's single largest open risk.** Static
  reading eliminated every candidate it could reach (see "Architecture & data flow" above) without
  landing on one that survives scrutiny. The parity rewrite is designed to be correct regardless of
  which step turns out to be at fault, but it is not a substitute for observing the failure.
  **Mitigation:** Task 0 of the implementation is instrumentation against the running app with the
  real BADAQ project — a per-source block counter logged at the publish tail plus the existing
  "Block pool exhausted" warning check — and no production line is edited until the drop point is
  observed. This is the repo's ground-truth-over-on-paper-reasoning rule applied literally: three
  plausible mechanisms have already been eliminated by reading, and the fourth will be too.
  **If Task 0 shows the parity rewrite does not address the observed cause, the plan comes back
  here for amendment rather than being widened during implementation.**
- **Hotpath regression.** The publish path is the 256 kHz gate. **Mitigation:** the change is
  net-subtractive on the parsed-frame lane; `--benchmark-hotpath` before and after is a hard gate
  (AC10), and `datasets+publish` is 70-80% of per-frame time so a regression would be immediately
  visible rather than marginal.
- **Change-driven transform interaction.** `reprocessDatasetValues` skips a transform whose read
  slots have not changed, and returns `changed == false` for a fully skipped pass. Once
  `m_republishedSourceIds` starts filling correctly (R7), a source whose values genuinely have not
  changed will stop republishing — which is correct, but it is a behavior change relative to
  today's accidental republish-every-tick. **Mitigation:** the round-trip test covers a project
  with `changeDrivenTransforms` enabled and asserts recorded sample counts, so an over-eager
  suppression shows up as missing samples rather than as a subtle gap.
- **Silent breakage classes this is exposed to** (`common-mistakes.md`): timestamp capture before
  queueing (the CSV reference change is adjacent to it — the fix must not move where a stamp is
  taken); queued-vs-direct on the hotpath (no connection changes, asserted in review);
  `operator[]` insertion on the new accounting map (use `find`/`try_emplace`, never
  `operator[]` in the publish tail).
- **`ProjectUndoScope` / lint:** none of the touched files are `ProjectModel` mutators, so the
  `undo-scope-missing` lint is not engaged.
- **Link-set creep in the ctest tier.** Both new suites are QtCore-only by construction — one over
  a new header-only predicate, one over `ExportSchema.h` plus a synthetic project structure.
  Neither links `FrameBuilder`. If either grows a heavier dependency during implementation, that is
  the signal to move the coverage into the CLI round-trip mode instead of growing the suite.

## Test & verification plan

**Unit — ctest tier (maintainer builds; runnable against an existing build dir):**

- `tst_replay_timestamp_mode` — **AC2.** Negative, zero and positive first cells all resolve to a
  numeric elapsed column with no prompt; a genuinely non-numeric first cell still falls through to
  the date-time/interval path; bracketed and `_suffix` unit headers still resolve their scale.
- `tst_export_schema_parity` — **AC3.** A synthetic project whose dataset tree order differs from
  its uniqueId order (the BADAQ shape) produces a CSV column order, an MDF4 channel order and a
  replay layout that all resolve the same dataset to the same values. Fails today.
- `tst_csv_sparse_writer` (extended) — **AC2.** Multi-source batches whose blocks arrive out of t0
  order still yield a non-negative, non-decreasing elapsed column.

**Round trip — new headless CLI mode (maintainer builds; CI-runnable, no GUI, no device):**

- `--verify-export-replay <project>` — **AC1, AC4, AC6, AC7, AC8.** Builds the pinned composition
  root, loads a synthetic multi-source project carrying table-fed virtual datasets alongside dense
  stream datasets, drives frames through the untouched pipeline, records to a temp CSV, MDF4 and
  session, then replays each back and diffs. Asserts: every dataset appears in every recording with
  the expected sample count (AC1); the session recording holds blocks for every dataset and replays
  them (AC4); a synthetic refresh reports itself published and a no-change refresh is suppressed
  (AC6); the frozen fixtures under `tests/fixtures/sessions/` still open and replay (AC7); a report
  generated from the recorded session carries plot data for every dataset it lists (AC8). JSON
  verdict on stdout, binary exit code, following `runSessionVerification()`.

**Integration — pytest (needs the app up with the API server on `localhost:7777`):**

- `tests/integration/test_block_lane_sinks.py` (extended) — **AC5.** Load a project with
  script/table-fed datasets, record, stop, reopen each recording, assert the dashboard's populated
  dataset set matches the live capture, and assert no new recording file or session row appeared
  during replay (R8).
- `tests/integration/test_session_report.py` — **AC8** cross-check against a live-recorded session.

**Hotpath:**

- `--benchmark-hotpath` on the PGO-optimized binary, before and after — **AC10.** All nine tiers at
  their default rates. `ci.yml` already runs this per push/PR as a hard gate.

**Maintainer observation:**

- **AC9.** Real BADAQ project: record a short capture, then replay the CSV, the MDF4 and the
  session recording, and generate the session report. Each replay opens with no time-column prompt
  and a populated dashboard; the report plots the APS500 and CAN groups.

**Static:**

- `python scripts/code-verify.py --check` over every touched file.
- `qt-cpp-review` before handoff — the change is on the hotpath and in a threading-sensitive
  module, which is exactly its remit.
- `python scripts/sanitize-commit.py` before the commit.
