---
spec: 0088-silent-failure-hardening
phase: plan
status: approved     # approved 2026-09-29
updated: 2026-09-29
---

# Plan 0088 — Silent-failure hardening: alarm loop and script contracts

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Seven independent fixes that share one shape: a silent wrong behavior becomes either correct
behavior or a Problem Center finding, with zero change for correct projects. R1 counts the
transform result branches that today return the raw value without touching
`noteTransformError` (the `script.transform` checker then fires with no further work). R2
replaces the translated-channel-name exclusion in the annunciator with a provenance field on
the `NotificationPosted` bus message. R3 keeps the alarm point table *in place* across an
unrequested link drop (audible stopped, a hold flag suppressing restarts) instead of wiping
it — the monitor's baseline re-seed then reconciles state naturally on reconnect, because
`AnnunciatorSequence::raise()` already returns false for an existing Alert/Acknowledged point
at the same priority. R4 subscribes `AudioExport` and `Console::Export` to
`FrameBuilder::sessionBoundary`, the contract CSV/MDF4/Sessions already follow. R5 adds a
per-source delivered-token watermark on the parse path (plain compare+store), diffed at 1 Hz
against the max configured frame index by a new `link.short-frames` checker. R6 latches a
control script's last runtime error on `ControlScript` and reports it through a new
`script.control` checker via `pipelineModules().controlScript`. R7 adds a
"has configured alarms" property to the annunciator facade, a finding when alarms exist but
the master enable is off, and a distinct disabled state on the taskbar pill.

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/FrameBuilder/TransformDispatch.cpp` | R1: `noteTransformError` in the four silent branches — Lua non-finite (~193), Lua nil/wrong type (~205), JS non-finite (~283), JS undefined/object (~298). Static literal messages ("transform returned NaN", "transform returned no value"). |
| `core/Pipeline/IO/StreamWorker.cpp` / `.h` | R1: expose the existing `m_transformErrors` / `m_transformTimeouts` through a pulled stats getter if none reaches a checker today (verify at task time; compile errors and aborted blocks already count). |
| `core/Ui/Misc/Problems/ScriptCheckers.cpp` | R1: fold stream-worker transform counters into `checkTransformErrors` (pulled, delta-based). R6: new `script.control` checker. |
| `core/Core/Bus/Messages.h` | R2: `NotificationPosted` gains `int origin` (0 = user, 1 = alarm monitor); named constants beside the struct. Topic unchanged — no bus-census delta. |
| `core/Pipeline/DataModel/NotificationCenter.h` / `.cpp` | R2: `post()` family gains a defaulted `origin` parameter carried through `Event` → `appendEvent` → the bus publish. `resolve()` unchanged. |
| `core/Ui/UI/AlarmMonitor.cpp` | R2: posts its band notifications with the alarm-monitor origin. |
| `core/Ui/UI/Alarms/AlarmAnnunciator.h` / `.cpp` | R2: filter on `origin`, delete the `UI::AlarmMonitor::tr("Alarms")` match (Problems/System stay, untranslated). R3: hold-in-place logic (`m_dropHold`), gated `onDataReset`, release on reconnect/requested-close/project-change/player-open. R7: `hasConfiguredAlarms` Q_PROPERTY + disabled-annunciator finding in `registerChecker`. |
| `core/Ui/UI/Alarms/AppEventSounds.h` / `.cpp` | R3: `linkClosed` gains the drop classification (`linkClosed(bool wasDrop)`); the flag already exists as `m_lastCloseWasDrop`. |
| `core/Ui/UI/Alarms/AnnunciatorSequence.h` / `.cpp` | R3 (added during implement, named in chat): `dropUnacknowledged(PointKind)` — selective removal of Alert points of one kind, because `clearKind` removes acknowledged ones too and `clear()` obeys lock-in. |
| `core/Ui/UI/Widgets/AudioExport.cpp` | R4: close via `FrameBuilder::sessionBoundary` (queued, cross-thread); drop the direct `connectedChanged`/`pausedChanged` closes; keep the player-open and license closes. |
| `core/Ui/Console/Export.cpp` / `.h` | R4: subscribe `sessionBoundary`, drop the `connectedChanged` close; `registerData` skips (does not lazily reopen) while paused. |
| `core/Pipeline/DataModel/FrameBuilder.h` / `.cpp` | R5: per-source delivered-token watermark (compare+store at the three parse call sites where the token count is known: span lane, cell lane, list lane), max-configured-index cached per source frame at snapshot apply; `shortFrameStats()` pulled snapshot (marshal, same pattern as `checkTransformErrors`). |
| `core/Ui/Misc/Problems/LinkCheckers.cpp` | R5: `link.short-frames` finding — watermark < max index sustained over the sample window; names the starved datasets from ProjectModel groups (GUI thread); window resets per sample; delta rules per spec 0033. |
| `core/Pipeline/DataModel/Scripting/ControlScript.h` / `.cpp` | R6: latch `m_lastError` + `m_stoppedOnError` in `onWorkerError`, cleared in `startWorker()`; `[[nodiscard]]` getters. |
| `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml` | R7: pill visible also when `hasConfiguredAlarms && !enabled`; distinct disabled glyph/tooltip (muted styling already exists). |
| `app/tests/tst_annunciator_sequence.cpp` | AC9: acknowledged point re-raised at same priority stays Acknowledged with no restart (the reconcile rule R3 leans on). |
| `tests/integration/test_silent_failure_hardening.py` | AC1–AC7 end-to-end (new file). |
| `doc/help/Aural-Alerts.md` | R2 exclusions by identifier; R3 reconnect behavior; R7 disabled indicator. |
| `doc/claude/architecture/scripting.md`, `dashboard.md`, `dataflow.md`, `export.md` | Amend: non-finite results now counted; alarm hold-in-place; watermark counter; five boundary sinks. |

## Architecture & data flow

- **R1** — all inside the existing per-dataset transform call on the pipeline thread
  (`TransformDispatch::apply`); `noteTransformError` is the established `SS_COLD` throttled
  counter (message allocated once per failing dataset, counter per event). The
  `script.transform` checker already polls it at 1 Hz through
  `invokeOnBuilderThreadBlocking`. Stream lane: counters stay worker-owned plain ints,
  pulled — never signaled (spec 0033).
- **R2** — `NotificationCenter::appendEvent` already publishes the bus message; the new
  field rides along. `AlarmAnnunciator::onNotificationPosted` switches from channel-name
  matching to `event.origin`. `Problems`/`System` remain excluded by their untranslated
  `QStringLiteral` channel ids and get documented as reserved.
- **R3** — flow on an unrequested drop: `AppEventSounds::onConnectedChanged` (GUI, direct)
  classifies the close and emits `linkClosed(wasDrop)`. The annunciator: `wasDrop == false`
  keeps today's `dropAllPoints()`; `wasDrop == true` sets `m_dropHold`, calls
  `stopAudible()`, and drops only *unacknowledged notification-kind* points (resolved open
  question). While `m_dropHold` is set, `onDataReset` skips `dropAllPoints()` (the
  disconnect-driven `Dashboard::dataReset` arrives queued right after) and `updateAudible`
  starts no burst. Release points: ConnectionStateChanged connected (clears the flag, then
  the monitor's baseline seed emits `bandTransition` per tracked dataset — same band + same
  severity hits `raise()` on the surviving point, which returns false for Alert/Acknowledged
  at unchanged priority, so nothing restarts; a changed band takes the normal
  transition path), an operator-requested close, `jsonFileChanged` /
  `onProjectSoundsChanged`, operator `clear()`, and player open (`dataReset` outside a drop).
  `onTrackersRebuilt` skips its reap while the hold is set (amended during implement: the
  disconnect reset empties the dashboard dataset map, so an ungated reap would remove every
  held point); normal reaping resumes with the reconnect rebuild.
- **R4** — `sessionBoundary(connected, paused)` is emitted on the pipeline thread after
  `m_stager.flushAll()`; the queued connection into each GUI-side sink preserves the
  flush-before-close ordering for block consumers (AudioExport). Console export's data is
  the raw console path, not blocks; it gains the pause edge and keeps its worker-side
  close-drain for the tail. `registerData` checks `ConnectionManager::paused()` (GUI-owned,
  GUI thread — legal) so a line arriving mid-pause does not lazily reopen a file.
- **R5** — the watermark is written where the token count is already in hand:
  `trySpanLane` (`tokens`), `tryCellLane` (per-row `count`), and the list path
  (`channels.size()`). Storage: a small per-source entry beside the existing per-source
  state, resolved once per pass via a cached-last-source pointer (steady single-source case:
  one compare + one conditional store per frame). The max configured index per source is
  computed once in `applyProjectSnapshot`/`ensureSourceFrame` from enabled, non-virtual
  datasets. The checker pulls `{sourceId, watermark, maxIndex}` snapshots, requires the
  shortfall sustained over `kSustainTicks` like `frames-without-values`, then resolves
  dataset titles with `index > watermark` from `ProjectModel::groups()` on the GUI thread.
- **R6** — `ControlScript` is GUI-affine; the checker reads
  `pipelineModules().controlScript` (the sanctioned route — no `instance()` reach from
  `core/Ui`). Finding severity Warning; text carries the latched error and the
  notifications-cannot-clear warning; clears when `running()` is true again.
- **R7** — `hasConfiguredAlarms` is refreshed from the dashboard dataset map on
  `trackersRebuilt` (the annunciator already connects to it) and project change; the checker
  emits the finding when `hasConfiguredAlarms && !enabled()`; the QML pill binds the same
  property.

## Hotpath & threading impact

- **Touches the hotpath?** Yes — R1 and R5 touch `TransformDispatch`/`FrameBuilder` parse
  paths (read in full; `ss-hotpath` invoked). R1 adds counter calls only inside already
  `[[unlikely]]`-guarded failure branches; the healthy path is untouched. R5 adds one
  compare + conditional store of a plain `int` per parsed frame (cached per-source entry, no
  map lookup on the steady path, no allocation, no lock, no signal). Counters are polled at
  1 Hz per spec 0033; nothing on the frame path emits. `--benchmark-hotpath` must show no
  regression (AC8); `datasets+publish` is the stage to watch.
- **New cross-thread signal/slot?** Two subscriptions to the existing
  `FrameBuilder::sessionBoundary` signal (pipeline → GUI, auto/queued — the same shape
  CSV/MDF4/Sessions use). `AppEventSounds::linkClosed` gains a bool parameter;
  same-thread direct, unchanged rate.
- **New input to a cached hotpath flag?** None. The watermark and error counters are
  diagnostics, not gates; none feeds `m_operationMode` / `m_anyAsyncSink` /
  `m_captureLatestFrame` / `m_streamAvailable`.
- **Timestamp ownership** — untouched; no path stamps or re-stamps time.

## Data model & persistence

None. No `Keys::` additions, no project-JSON change, no schema bump. (The alarm opt-out that
would add keys is spec'd separately, by design.)

## API / SDK surface

No new verbs. Observable API deltas: `alarms.state` points reflect the R3 hold (points
persist across a drop; a `held`/unchanged `sinceMs` is visible), `problems.list` gains the
three new finding codes (`link.short-frames`, `script.control`, the disabled-annunciator
code under the existing `alarms.sounds` checker id), and `notifications.post` keeps its
schema (the origin is internal — remote posts are always user-origin). SDK regeneration not
required; `generate-sdk.py --check` run at sanitize to confirm.

## QML / UI

`MasterAnnunciator.qml` only: extend the pill's `visible:` with
`(Cpp_UI_Alarms.hasConfiguredAlarms && !Cpp_UI_Alarms.enabled)`, add the disabled glyph +
tooltip branch beside the muted one. No new components; opacity-only animation rules
unchanged; no width/height animation.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| R3 mechanism | (a) hold table in place + audible suppression; (b) snapshot-and-restore held map; (c) wipe but mute the reseed | (a) — the sequence core's `raise()` semantics already reconcile a surviving point, no map lifecycle to expire, and the panel keeps showing last-known state during the outage, which is annunciator-correct. (c) fails the approved spec. |
| R5 detection | (a) per-source delivered watermark per sample window; (b) per-dataset miss counters; (c) checker-side stale-value inference | (a) — (b) false-positives on heterogeneous multi-shape frames (each shape starves the other shape's indices every frame); (c) cannot distinguish starvation from legitimately constant data. |
| R2 mechanism | (a) origin field on `NotificationPosted`; (b) reserve an untranslated channel id for the monitor | (a) — provenance is the actual intent; (b) renames a user-visible channel and still leaves name-matching semantics. |
| R1 counting rate | (a) count per failing event (existing pcall-error behavior); (b) latch once per edge | (a) — consistency with the existing counter contract; the message throttle already prevents per-frame allocation. |
| Console pause behavior | (a) close on pause, no logging while paused; (b) close on pause but reopen on mid-pause data | (a) — matches what CSV/MDF4/Historian record during a pause (nothing); (b) would log data the other recorders drop, un-aligning the story the spec chose. |
| Stream-lane R1 scope | (a) wire existing error/timeout counters into the checker; (b) also flag NaN samples inside returned blocks | (a) — a NaN sample in a numeric stream is data, not a failed transform; the spec's clause covers the *result shape*, which the stream lane already polices. |

## Risks & mitigations

- **Hotpath regression (R5).** Mitigation: cached-entry compare+store only, no lookup on the
  steady path; benchmark gate AC8; `code-verify.py --check` (hotpath violations block).
- **R1 turns a formerly "legitimate" pattern into a finding** — a transform deliberately
  returning nil/undefined to mean "keep raw". Mitigation: grep shipped templates and
  `app/rcc/` scripts for that pattern before landing (task item); the finding is a Warning,
  the displayed value is unchanged, and the message names the exact dataset.
- **R3 stale points on a changed project across a drop** — a project edited *while*
  disconnected. Mitigation: `jsonFileChanged` clears the hold; `onTrackersRebuilt` reaps
  datasets that no longer exist or lost their bands (already shipped behavior).
- **R3 ordering between `linkClosed` and the queued `dataReset`** — the hold flag must be
  set before the reset arrives. `linkClosed` is emitted from a direct `connectedChanged`
  connection; `Dashboard::resetData` is queued (`connectSessionResets`), so the direct hop
  wins by construction. An assertion documents the assumption.
- **R4 changes AudioExport's close trigger** — a close signal missed would leave a WAV
  growing forever. Mitigation: the license/player closes remain; integration AC4 plus
  maintainer observation; the boundary fires on both connect and pause edges.
- **Silent-breakage classes exposed** (common-mistakes.md): queued-vs-direct on the frame
  path (nothing new on it), cached-flag misses (no flag touched), `%n`-with-`.arg()` in the
  new finding strings (use numbered placeholders), setter guard returns on the new
  properties.

## Test & verification plan

- **Unit (I can run once the maintainer builds):**
  `ctest -R tst_annunciator_sequence` — new case: acknowledged point re-raised at the same
  priority stays Acknowledged, returns false (no restart); existing A-4/lock-in suite
  guards R3's non-regression (AC9).
- **Integration (app up, API server on):** `tests/integration/test_silent_failure_hardening.py`
  - AC1: install NaN-returning JS + typo'd-params Lua transforms via the API, stream data,
    assert `problems.list` finding + unchanged displayed value; repeat for missing return.
  - AC2: `notifications.post` on channel `Alarms` → point in `alarms.state`; band alarm
    produces exactly one point; repeat under a non-English language (`misc.setLanguage` or
    settings round-trip).
  - AC3: raise → acknowledge → kill simulated link → reconnect → point still acknowledged,
    `burstStartedMs` unchanged; unacknowledged variant resumes; requested disconnect clears.
  - AC4: console export + pause/resume → two files, tail in the first.
  - AC5: 4-value frames vs 8-index project → finding names the four; widen → clears.
  - AC6: control script whose `loop()` throws → `script.control` finding; fix → clears.
  - AC7: project with bands + master enable off → finding; enable → clears.
- **Maintainer observation:** WAV tail after a disconnect (AC4); taskbar pill disabled state
  distinct from muted (AC7).
- **Hotpath:** `--benchmark-hotpath` full gate run (AC8), maintainer-run, compared against
  the historical band only (CI noise memory: gate pass/fail, not deltas).
- **Static:** `python scripts/code-verify.py --check` on every touched file;
  `python scripts/claim-verify.py` (doc amendments); `qt-cpp-review` on the C++ diff;
  `python scripts/sanitize-commit.py` before commit.
