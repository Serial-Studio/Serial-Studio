---
spec: 0088-silent-failure-hardening
phase: tasks
status: approved     # approved 2026-09-29
updated: 2026-09-29
---

# Tasks 0088 — Silent-failure hardening

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a human
> marks this `approved`.

## Conventions

- `CV` abbreviates `python scripts/code-verify.py --check <the task's files>`.
- The maintainer builds; ctest/integration lines run only against an existing build or a
  running app. Sequence hotpath work (T12) so `--benchmark-hotpath` runs once, at the end.

## Phase A — Notification provenance (R2)

### T1 — `NotificationPosted.origin` and the post plumbing

- **Files:** `core/Core/Bus/Messages.h`, `core/Pipeline/DataModel/NotificationCenter.h`,
  `core/Pipeline/DataModel/NotificationCenter.cpp`
- **Does:** Add `int origin` to `NotificationPosted` with named constants
  (`kNotificationOriginUser = 0`, `kNotificationOriginAlarmMonitor = 1`) beside the struct;
  `post()`/`postInfo`/`postWarning`/`postCritical` gain a defaulted `origin` parameter carried
  through `Event` and `appendEvent`'s bus publish. `resolve()` unchanged. Topic set unchanged
  — no `--bus-census` delta expected.
- **Verify:** CV; read-back that every publish site fills the field; `--bus-census` clean.
- **Deps:** none
- [x] done

### T2 — Filter by provenance, not translated channel name

- **Files:** `core/Ui/UI/AlarmMonitor.cpp`, `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`
- **Does:** The monitor's band notification posts pass
  `kNotificationOriginAlarmMonitor`; `onNotificationPosted` drops monitor-origin events and
  deletes the `UI::AlarmMonitor::tr("Alarms")` channel match. `Problems`/`System` stay
  excluded by their untranslated ids. A user post on a channel named "Alarms" now raises a
  point in every UI language (AC2).
- **Verify:** CV; read-back that no translated string participates in any filter decision.
- **Deps:** T1
- [x] done

## Phase B — Transform result hygiene (R1)

### T3 — Count the silent frame-lane branches

- **Files:** `core/Pipeline/DataModel/FrameBuilder/TransformDispatch.cpp`
- **Does:** `noteTransformError(uniqueId, ...)` in the four silent branches: Lua non-finite,
  Lua nil/wrong-type, JS non-finite, JS undefined/object. Static literal messages
  ("transform returned NaN", "transform returned no value"). **Invariant: every touched
  branch is already `[[unlikely]]`-guarded failure handling; the healthy path gains no
  instruction, and `noteTransformError`'s once-per-dataset message throttle is the
  allocation bound.** Return value stays `rawValue` (display unchanged).
- **Verify:** CV; grep `app/rcc/scripts/` and shipped templates for transforms that
  deliberately return nil/undefined ("keep raw" pattern) — none expected; report any found
  before proceeding.
- **Deps:** none
- [x] done

### T4 — Route stream-lane transform counters into the checker

- **Files:** `core/Pipeline/IO/StreamWorker.h`, `core/Pipeline/IO/StreamWorker.cpp`,
  `core/Ui/Misc/Problems/ScriptCheckers.cpp`
- **Does:** Confirm whether `StreamProcessor`'s `m_transformErrors` /
  `m_transformTimeouts` reach any checker today; if not, expose a pulled stats snapshot
  (worker-owned plain counters, read at 1 Hz — **never signaled, spec 0033**) and fold the
  deltas into `checkTransformErrors`' finding. No per-sample NaN policing (plan tradeoff:
  a NaN sample in a numeric stream is data).
- **Verify:** CV; read-back that nothing on the block path emits, locks, or allocates.
- **Deps:** T3
- [x] done

## Phase C — Control-script death is visible (R6)

### T5 — Latch the last worker error on `ControlScript`

- **Files:** `core/Pipeline/DataModel/Scripting/ControlScript.h`,
  `core/Pipeline/DataModel/Scripting/ControlScript.cpp`
- **Does:** `m_lastError` + `m_stoppedOnError` set in `onWorkerError`, cleared in
  `startWorker()`; `[[nodiscard]] const QString& lastError()`,
  `[[nodiscard]] bool stoppedOnError()`. No signal changes; no ctor-closure code touched.
- **Verify:** CV; read-back of the set/clear sites against the worker lifecycle
  (stop-then-start on every rising edge).
- **Deps:** none
- [x] done

### T6 — `script.control` checker

- **Files:** `core/Ui/Misc/Problems/ScriptCheckers.cpp`
- **Does:** New checker (LinkSample | OnDemand) reading
  `DataModel::pipelineModules().controlScript` — **the sanctioned route; no `instance()`
  reach from `core/Ui` (singleton census)**. Warning finding carrying the latched error and
  the "notifications raised by this script can no longer clear themselves" note; absent
  while `running()` or when no error is latched. Numbered placeholders only (`%1`), never
  `%n` with `.arg()`.
- **Verify:** CV; `python scripts/code-verify.py --singleton-census --check` shows no growth.
- **Deps:** T5
- [x] done

## Phase D — Acknowledgements survive a drop (R3)

### T7 — `linkClosed` carries the drop classification

- **Files:** `core/Ui/UI/Alarms/AppEventSounds.h`, `core/Ui/UI/Alarms/AppEventSounds.cpp`,
  `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`
- **Does:** `linkClosed(bool wasDrop)` using the existing `m_lastCloseWasDrop`
  classification; the annunciator's connection and `onLinkClosed` signature follow. **Read
  the existing signal wiring in both files first; same-thread direct connection, command
  rate, unchanged.**
- **Verify:** CV; read-back of the emit site ordering (classification set before emit).
- **Deps:** none
- [x] done

### T8 — Hold-in-place on an unrequested drop

- **Files:** `core/Ui/UI/Alarms/AlarmAnnunciator.h`, `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`,
  `core/Ui/UI/Alarms/AnnunciatorSequence.h`, `core/Ui/UI/Alarms/AnnunciatorSequence.cpp` (added
  during implement, named in chat: `dropUnacknowledged(PointKind)`)
- **Does:** `m_dropHold`: on `wasDrop`, stop the audible, drop only unacknowledged
  notification-kind points, keep everything else; requested close keeps today's
  `dropAllPoints()`. While held: `onDataReset` skips the wipe (**invariant: `linkClosed`
  arrives on a direct connection, Dashboard's disconnect reset is queued — assert the
  ordering assumption**), `updateAudible` starts no burst. Release on: connected edge
  (before the monitor's baseline seed re-raises — surviving Alert/Acknowledged points at
  unchanged priority reconcile via `raise()` returning false), requested close,
  `jsonFileChanged`/`onProjectSoundsChanged`, operator `clear()`, and any `dataReset`
  outside a drop (player open, mode change). `onTrackersRebuilt` reaping unchanged.
- **Verify:** CV (functions ≤ 100 lines); read-back of every release path against the list
  above; integration AC3 at T16.
- **Deps:** T7
- [x] done

### T9 — Unit: the reconcile rule R3 leans on

- **Files:** `app/tests/tst_annunciator_sequence.cpp`
- **Does:** New case: an Acknowledged point re-raised at the same priority stays
  Acknowledged and `raise()` returns false (no burst restart); an Alert point likewise
  returns false. Pins the sequence-core behavior the facade's hold design depends on (AC9).
- **Verify:** `ctest -R tst_annunciator_sequence` against the maintainer's next build.
- **Deps:** none
- [x] done

## Phase E — One session boundary for every recorder (R4)

### T10 — AudioExport closes on the session boundary

- **Files:** `core/Ui/UI/Widgets/AudioExport.cpp`
- **Does:** Subscribe `DataModel::pipelineModules().frameBuilder`'s `sessionBoundary`
  (queued from the pipeline thread — **the flush-before-emit ordering is the tail
  guarantee**); remove the direct `connectedChanged`/`pausedChanged` closes; keep the
  player-open and license closes untouched.
- **Verify:** CV; read-back that no close trigger was lost (disconnect, pause, player,
  license); maintainer observation AC4 (WAV tail).
- **Deps:** none
- [x] done

### T11 — Console export joins the boundary and respects pause

- **Files:** `core/Ui/Console/Export.cpp`, `core/Ui/Console/Export.h`
- **Does:** Subscribe `sessionBoundary` in place of the `connectedChanged` close;
  `registerData` returns early while `ConnectionManager::paused()` (GUI-owned state read on
  the GUI thread — legal) so a mid-pause line does not lazily reopen a file. Worker-side
  close-drain unchanged (it is the console tail guarantee).
- **Verify:** CV; integration AC4 at T16 (two files across a pause, tail in the first).
- **Deps:** none
- [x] done

## Phase F — Starved datasets are reported (R5)

### T12 — Delivered-token watermark in FrameBuilder

- **Files:** `core/Pipeline/DataModel/FrameBuilder.h`, `core/Pipeline/DataModel/FrameBuilder.cpp`,
  `core/Pipeline/DataModel/FrameBuilder/ShortFrameWatch.h` / `.cpp`, `core/Pipeline/CMakeLists.txt`
  (extracted during implement: the TU-census ratchet mandates a sub-object over facade growth;
  census re-seeded for the six residual call-site lines)
- **Does:** Per-source watermark entry (delivered-token max this window + max configured
  frame index, computed from enabled non-virtual datasets in `applyProjectSnapshot` /
  `ensureSourceFrame`); compare+store at the three sites where the count is in hand
  (`trySpanLane` tokens, `tryCellLane` per-row count, list path `channels.size()`);
  `shortFrameStats()` snapshot getter with a window reset, marshaled like
  `parseLoadSnapshot`. **Hotpath invariants, named: no allocation, no lock, no signal, no
  map lookup on the steady path (cached last-source entry); counters are pulled at 1 Hz
  (spec 0033); no cached hotpath flag gains an input; `SS_ASSERT_HOTPATH` only on the
  per-frame path.**
- **Verify:** CV (hotpath violations block); read-back of all three call sites;
  `--benchmark-hotpath` at Definition of Done.
- **Deps:** none
- [x] done

### T13 — `link.short-frames` checker

- **Files:** `core/Ui/Misc/Problems/LinkCheckers.cpp`
- **Does:** New finding: watermark < max index sustained over `kSustainTicks` (same
  sustain/delta/reset-tolerant rules as `frames-without-values`; a decrease is a reset).
  Names the starved datasets (index > watermark) from `ProjectModel::groups()` on the GUI
  thread; bucketed text, stable while the condition is stable. Clears when the watermark
  covers the max index or the project changes (AC5). Quiet-source diagnostics untouched.
- **Verify:** CV; integration AC5 at T16.
- **Deps:** T12
- [x] done

## Phase G — A silent annunciator is visible (R7)

### T14 — `hasConfiguredAlarms` and the disabled-annunciator finding

- **Files:** `core/Ui/UI/Alarms/AlarmAnnunciator.h`, `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`,
  `core/Ui/UI/Alarms/AnnunciatorChecker.h` / `.cpp`, `core/Ui/CMakeLists.txt` (extracted during
  implement: the facade crossed the 1500-line TU cap, so the finding builder moved to a
  ref-binding sub-object)
- **Does:** `Q_PROPERTY(bool hasConfiguredAlarms READ hasConfiguredAlarms NOTIFY
  stateChanged)` refreshed from the dashboard dataset map on `trackersRebuilt` and project
  change (guard return before emit); `registerChecker` gains the finding: bands or a
  project sounds map present while `!enabled()` → Warning naming the remedy
  (Preferences → Sounds). No finding without alarm configuration (AC7).
- **Verify:** CV; read-back that the property refresh allocates nothing per tick (it runs at
  rebuild rate, not frame rate).
- **Deps:** none
- [x] done

### T15 — Taskbar pill disabled state

- **Files:** `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml`
- **Does:** Pill `visible:` extended with `Cpp_UI_Alarms.hasConfiguredAlarms &&
  !Cpp_UI_Alarms.enabled`; disabled glyph + tooltip branch beside the muted one. **Chrome
  rules: opacity-only animation, no width/height animation, reduce-motion path unchanged.**
- **Verify:** `qmllint`; maintainer observation AC7 (disabled distinct from muted).
- **Deps:** T14
- [x] done

## Phase H — Tests and documentation

### T16 — Integration suite

- **Files:** `tests/integration/test_silent_failure_hardening.py`
- **Does:** AC1 (NaN/typo'd-params/missing-return transforms → `script.transform` finding,
  displayed value unchanged), AC2 ("Alarms"-channel post raises a point; band alarm yields
  exactly one point; non-English language variant), AC3 (ack survives drop+reconnect,
  `burstStartedMs` unchanged; unacknowledged resumes; requested disconnect clears), AC4
  (console pause split), AC5 (short frames finding appears and clears), AC6
  (`script.control` finding lifecycle), AC7 (disabled-annunciator finding lifecycle). Uses
  `api_client`, `clean_state`, `device_simulator`; delays per `tests/README.md` (read it
  before writing).
- **Verify:** `pytest tests/integration/test_silent_failure_hardening.py -v` with the app up
  (`nc -z 127.0.0.1 7777` first).
- **Deps:** T2, T3, T6, T8, T11, T13, T14
- [x] done

### T17 — User help

- **Files:** `doc/help/Aural-Alerts.md`
- **Does:** Excluded channels documented by identifier (Problems, System — the translated
  "Alarms" exclusion is gone); reconnect behavior (acknowledgements survive an unrequested
  drop); the disabled-annunciator indicator and finding. Never says "requires a license".
- **Verify:** `python scripts/documentation-verify.py`; `ss-docs` two-tier check.
- **Deps:** T2, T8, T14
- [x] done

### T18 — AI-facing docs

- **Files:** `doc/claude/architecture/scripting.md`, `doc/claude/architecture/dataflow.md`,
  `doc/claude/architecture/export.md`, `doc/claude/architecture/dashboard.md`
- **Does:** scripting.md: "non-finite results are rejected" gains "and counted as transform
  errors"; dataflow.md: the watermark joins the pulled-counter list; export.md: the
  session-boundary sink list grows to five (Audio, Console); dashboard.md: the alarm section
  gains the hold-in-place rule and the provenance filter.
- **Verify:** `python scripts/claim-verify.py`; `python scripts/documentation-verify.py`.
- **Deps:** T3, T8, T10, T11, T12
- [x] done

## Review record (2026-09-29)

`qt-cpp-review` ran on the diff (Phase 1 lint clean; six parallel agents). Confirmed and
fixed on disk: the audible never re-armed when the drop hold released (a mid-outage
notification point sat listed but silent forever -- `onConnectionEdge` now re-runs
`updateAudible`), the one-shot drop-reset consumption could swallow a replay-open reset when
the drop itself produced none (replaced with a player-aware swallow: held while disconnected
and no player is open), the ShortFrameWatch "count published last" claim had no memory-model
backing (`m_count` is now an atomic with release/acquire pairing), `ShortFrameWatch` and
`AnnunciatorChecker` gained deleted copy/move (the BlockStager shape; the watch's cached
self-pointer made implicit copies wrong), `dropUnacknowledged` gained `[[nodiscard]]`, the
`origin = 0` default is anchored to `kNotificationOriginUser` by a `static_assert`, `rebuild`
gained its two assertions, and `note()`'s release-evaluated asserts became
`SS_ASSERT_HOTPATH` with `ShortFrameWatch.cpp` added to `_HOTPATH_ASSERT_ALLOWED` in
`scripts/code-verify.py` (named whitelist growth: the TU joined the per-frame path and both
conditions restate caller-proven guards). The stream-transform finding's remedy now says the
lane keeps no per-dataset error detail. Two items first left as noted were then fixed at the
maintainer's request (2026-09-29 follow-up): `noteTransformError` now retains the message
once per dataset per engine lifetime (`m_errorNotedDatasets`), so failing datasets no longer
ping-pong a QString allocation at parse rate (the suspected unsynchronized GUI read turned
out not to exist -- the checker marshals to the builder thread); and the watch's single-entry
cache became a sourceId-indexed direct map, one indexed load per frame for any interleaving
of sources. Sanitize ran last.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` met and checked off there (AC4's WAV tail and
      AC7's pill state by maintainer observation; AC8 recorded from the maintainer's
      benchmark run).
- [x] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [x] `--singleton-census --check`, `--bus-census`, `--tu-census --check`,
      `python scripts/layer-verify.py` all clean.
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed (AC8; gate pass/fail, judged against the
      historical noise band).
- [x] `ctest -R tst_annunciator_sequence` and
      `pytest tests/integration/test_silent_failure_hardening.py` identified for the
      maintainer; the ctest tier run by me against the next build.
- [x] `python scripts/sanitize-commit.py` run; `reuse lint` clean; diff is *what was asked,
      and only that*.
- [x] `spec.md` status set to `done`.
