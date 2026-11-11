---
spec: 0119-resync-decision-seam
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-11-10
---

# Plan 0119 — Testable re-sync decisions for the frame builder and the dashboard

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Two small objects take over the three decisions, and the god classes keep only the effects. A
`DataModel::SessionAnnouncer` sub-object decides which layouts a session announces and whether the
combined frame may stand in; it calls back through a `SessionAnnouncerHost` interface that
`FrameBuilder` implements, the shape `BlockStager` already uses. A header-only
`UI::RebuildRetryLedger` holds the per-(source, generation) retry and quarantine state, the shape
`RepublishGate` already uses. One ctest suite drives both with a recording fake host and no
singleton. One integration test drives the real trigger end to end. The design was reviewed by a
Fable advisor, whose four amendments to the first sketch are folded in below.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/FrameBuilder/SessionAnnouncer.h` | New: `SessionAnnouncerHost` and `SessionAnnouncer`. |
| `core/Pipeline/DataModel/FrameBuilder/SessionAnnouncer.cpp` | New: the decisions. |
| `core/Pipeline/CMakeLists.txt` | Register both, beside `BlockStager` (source list at line 155, plus the header list). |
| `core/Pipeline/DataModel/FrameBuilder.h` | Inherit the host; declare `m_announcer` directly after `m_stager`; rename one method, drop one. |
| `core/Pipeline/DataModel/FrameBuilder.cpp` | Delegate to the announcer (detail below). |
| `core/Ui/UI/Dashboard/RebuildRetryLedger.h` | New, header-only: the retry and quarantine state. |
| `core/Ui/CMakeLists.txt` | List the header beside `PlotMemoryLedger.h` (line 223). |
| `core/Ui/UI/Dashboard.h` | Replace `m_quarantinedSources` and `m_retriedSources` with `m_rebuildLedger`. |
| `core/Ui/UI/Dashboard.cpp` | `handleMissingDataset` switches on the verdict; `resetData` clears the ledger in the `if (notify)` branch. |
| `app/tests/tst_resync_decisions.cpp` | New ctest suite. |
| `app/tests/CMakeLists.txt` | Register it; donor entry `tst_quickplot_format_state` (line 408). |
| `tests/integration/test_live_resync.py` | New integration test (AC8). |
| `scripts/doc-anchors.json` | New anchor `rebuild-ledger-hard-reset-only` (AC9). |
| `scripts/tu-census.json` | Re-baselined downward only (AC7). |
| `doc/claude/architecture/dashboard.md` | Lines 39-40: one rebuild per (source, generation), cleared only by a hard reset. |
| `doc/claude/directory-map.md` | Lines 140 and 222: name the two new objects. |
| `doc/claude/TODO.md` | Delete the "live re-sync of a multi-source session" item (lines 32-35). |

## Architecture & data flow

**`SessionAnnouncerHost`** follows `BlockStagerHost`: deleted copy and move, out-of-line virtual
destructor in the `.cpp`.

```cpp
virtual void announceSourceTemplate(const DataModel::Source& src) = 0;
virtual void announceProjectFrame()                               = 0;
virtual void forgetPublishedStructures()                          = 0;
```

**`SessionAnnouncer`** binds the host, `const Frame&` and `const QSet<int>&`. Its constructor only
stores the references and reads nothing.

| Method | Decision |
|--------|----------|
| `void announceConnect()` | Multi-source: `announceSource` for each source. Otherwise, if every group is an image widget: `announceProjectFrame()`. Otherwise nothing. |
| `void announceResync(bool connected, SerialStudio::OperationMode mode)` | Returns, forgetting nothing, unless connected, in project mode, with groups and a title. Then forget, `announceConnect()`, and `announceSource` for each stream-lane id. |
| `void announceSource(int sourceId)` | Finds the source, skips one with no group, calls the host. |
| `[[nodiscard]] bool publishesOwnFrame(int sourceId) const noexcept` | False for a stream-lane source. |
| `[[nodiscard]] bool combinedFrameStandsIn(bool anyOwnFrame) const noexcept` | `!anyOwnFrame`, not multi-source, has groups and a title. |

A private `isMultiSource()` backs both `announceConnect` and `combinedFrameStandsIn`, so "per-source
layouts" and "no combined stand-in" cannot disagree. The mode is a parameter because
`m_operationMode` is a `Cached<>`, which a test cannot construct without a checker.

**`FrameBuilder` changes:**

- `publishSessionTemplates` is deleted; `onConnectedChanged` calls `m_announcer.announceConnect()`.
- `publishSourceTemplateFrame` is renamed to the `announceSourceTemplate` override. Its group-less
  early return stays as a guard, though the announcer now filters first.
- `announceProjectFrame` is a new one-line override calling
  `ensureStructurePublished(m_frame.sourceId, m_frame)`.
- `forgetPublishedStructures` (existing public slot) becomes the host override, marked `final`.
- `republishSessionTemplates` keeps its thread marshal plus one call to `announceResync`.
- `publishSourceTemplate(int)` keeps its two guards and delegates to `announceSource`, so the
  source lookup has one implementation. Its caller (`StreamWorkerPool.cpp:191`) is unchanged.
- `republishFrames` uses `publishesOwnFrame` and `combinedFrameStandsIn(any_source)`. The predicate
  keeps the groups and title terms from today's line 1276, which spec R2 does not spell out.

**`UI::RebuildRetryLedger`** holds one `QHash<int, Entry{quint64 generation; bool quarantined;}>`,
which makes "quarantined but never retried" unrepresentable.
`[[nodiscard]] Verdict onMismatch(int sourceId, quint64 generation)` returns `Rebuild`,
`Quarantine` or `Drop`; `void clear() noexcept` forgets all. `Dashboard` keeps the `qWarning` on
`Quarantine` and the generation lookup.

**Hard resets.** The ledger clears only in `resetData`'s `if (notify)` branch. The session callers
are the four spec R3 names. `BenchmarkRunner.cpp:621` also calls `resetData()`, outside any session.

## Hotpath & threading impact

- **Touches the hotpath?** Yes, the files, but not the per-frame path. `FrameReader`,
  `CircularBuffer`, the span lane, `BlockStager` and `DashboardIngest` are untouched. The edited
  functions run at connect-edge rate, project-edit rate, mismatch rate, reset rate, and
  `republishFrames`' display-tick (only when stream values changed) or command rate. On that last
  path the two predicates are `noexcept`, allocate nothing and take no lock. `FrameBuilder` and
  `Dashboard` were read in full; `ss-hotpath` was loaded. `--benchmark-hotpath` is the check (AC5).
- **New cross-thread signal/slot?** No. The marshal in `republishSessionTemplates` stays in the
  facade; the announcer runs entirely on the builder thread.
- **New input to a cached hotpath flag?** No. `m_operationMode` is read and passed by value.
- **Timestamp ownership** — unchanged; nothing here stamps.

## Data model & persistence

None.

## API / SDK surface

None. The integration test uses existing commands.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Seam shape | A: free verdict functions. B: sub-object with a host. C: link the real classes into a ctest with stubs. D: a pure function returning a step list the builder interprets. E: the dashboard pulls layouts after its reset. | **B.** A cannot pin forget-before-announce. C drags in Lua, JS and the project model. D's interpreter adds lines to the god TU. E removes the trigger wiring but breaks behaviour-preserving. |
| Group-less and stream filters | In the host, or in the announcer | **Announcer.** A fake host never runs the host's code, so a filter left there is unpinned. |
| Ledger storage | Two maps, or one map with a state | **One map.** Same outcomes while a generation only moves forward, and one fewer invalid state. A generation returning to an earlier value, which only a mirror peer can produce, earns a fresh rebuild as R3 says, where two maps could drop. |
| Ledger form | `.h`/`.cpp` pair, or header-only | **Header-only**, as `RepublishGate`; the test links nothing for it. |
| Duplicate stream-source template rebuild | Keep, or dedupe | **Keep.** Spec non-goal; the test asserts "at least once", never an exact count. |
| Suites | One, or one per library | **One**, `tst_resync_decisions`: `SessionAnnouncer.cpp`, Qt6::Core, SerialStudio::Core. |
| AC9 enforcement | Review only, or an anchor | **Anchor plus review.** The anchor catches the clear leaving the `if (notify)` branch. It cannot prove absence, so a second clear elsewhere is still review's to catch (`grep -c "m_rebuildLedger.clear"` equals 1). |

## Risks & mitigations

- **Behaviour drift from moving the filter and the lookup.** Compare the fake-host log with a hand
  trace of today's code; AC8 covers the real path.
- **Announcer bound before its members exist.** Declared after `m_stager`, which follows `m_frame`
  and `m_streamSourceIds`; its constructor only binds.
- **New translation units never compiled.** `scripts/syntax-check.py` with a borrowed
  `BlockStager.cpp` compile entry. A SKIP means "it compiles" is unverified, and the handoff says so.
- **God TU grows.** Estimated about -30 lines in `FrameBuilder.cpp` and -3 in `Dashboard.cpp`;
  `--tu-census --check` gives the exact figure. The census already carries an unrelated 5-line
  shrink, which the re-baseline absorbs and must not be credited to this change.
- **AC8 is flaky or passes vacuously in a suite that now gates.** Paused streams, a poll on the new
  title arriving, deadlines only. A flake goes on `_FLAKY_TESTS`; the step is never softened.
- **AC8 may not reproduce the pre-fix failure.** The sequence is derived from reading the code and
  has not been run against a pre-fix binary. AC4 carries the revert proof. AC8 pins the trigger and
  the re-announcement only: once the re-announcement rebuilds the per-source frames, a refresh cannot
  reach the combined-frame decision, so only the ctest pins that.
- **A confirm prompt on the edit.** `update_dataset` has precedent in `test_aural_alerts.py:618`;
  confirm on the first run.

## Test & verification plan

**`tst_resync_decisions` (ctest; the maintainer builds, CI runs it, including ASan and TSan):**

| AC | Cases |
|----|-------|
| AC1 | Multi-source connect announces one layout per source with groups; a group-less source gets none. |
| AC1 | Image-only single source announces the project frame; a mixed single source announces nothing. |
| AC1 | Re-sync log starts with `forget`, then the connect set. |
| AC1 | Re-sync announces each stream source at least once, including on a single-source project. |
| AC1 | Disconnected, QuickPlot, no groups, no title: the log stays empty, `forget` included. |
| AC2 | Two sources: false for both `anyOwnFrame` values. One source: true only when `anyOwnFrame` is false. |
| AC2 | `publishesOwnFrame` is false for a stream id. No groups or no title: false. |
| AC3 | A rebuilds; B rebuilds and quarantines; A quarantines; a quarantined pair drops; a new generation rebuilds; `clear()` re-arms both. |

**AC4 (the maintainer reverts each, rebuilds only this suite, records the failure).** The pre-fix
lines disappear in the extraction, so `tasks.md` names these one-line edits to the new code:

1. Stand-in: drop the `!isMultiSource()` term from `combinedFrameStandsIn`.
2. Re-announce: make `announceResync` return before the forget.
3. Retry: key the ledger on a constant `0`, the old single flag.

**AC8, `tests/integration/test_live_resync.py`** (commercial build; `_is_commercial` probe, then
`pytest.skip`). Its own light two-source project, both sources near 20 Hz, on ports 9040 and 9041:

1. Start both streams; poll until both counters move.
2. Stop both streams, the only deterministic way to act before either source's next frame. Poll
   until values are quiet; record `get_widget_count()`.
3. `update_dataset(group, 0, title="...")`: a real value change guarantees the mutation epoch moves.
4. `command("dashboard.reprocess")`. From the GUI it queues and answers `published: false`, so its
   response is not asserted.
5. Poll up to 10 s until the count equals the pre-edit value and the new title appears in
   `get_dashboard_data()`, which proves the re-sync happened with both sources silent.
6. Resume source 1 only; hold 2 s; the count stays constant.
7. Resume source 0; poll until both counters advance; hold 2 s.

**AC5:** the `--benchmark-hotpath` CI gate on every leg, plus the maintainer's local before and
after. **AC6:** the pytest legs. **AC7:** `python3 scripts/code-verify.py --tu-census --check`, then
`--accept` downward. **AC9:** the anchor through `claim-verify.py`, plus the grep above.

**Static (the assistant runs):** `scripts/syntax-check.py` on every C++ file edited,
`scripts/code-verify.py --check` on the touched files, `scripts/claim-verify.py`,
`pytest scripts/tests/ -q`, and `qt-cpp-review` before handoff.

Nothing is committed unless the maintainer asks; the change then lands as one commit with the spec.
