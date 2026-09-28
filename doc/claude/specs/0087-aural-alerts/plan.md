---
spec: 0087-aural-alerts
phase: plan
status: approved     # approved 2026-09-27
updated: 2026-09-27
---

# Plan 0087 — Aural alerts

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Two new pieces, one per layer. In `core/Devices`, `IO::SoundPlayer` owns a miniaudio playback
device (its own `ma_context`, separate from the Audio driver's), a `SoundBank` of pre-decoded
float buffers built by a small first-party RIFF/PCM `WavDecoder` plus `ma_data_converter`, and
a real-time callback that drains a fixed SPSC command ring and mixes two lanes (alarm, event)
with no allocation, lock or file I/O. In `core/Ui`, `UI::AlarmAnnunciator` is a root-owned
QObject facade (not a singleton) composed of `AnnunciatorSequence` (the pure ISA-18.1 point
table, arbitration and sequence A/M/R transitions, ctest-able), `SoundTheme` (the QSettings
slot table, project overrides, bank loading, device choice) and `AppEventSounds` (the
connection, recording, replay and error-dialog hooks). It consumes band transitions from
`UI::AlarmMonitor` (one new signal) and notification points from the existing
`Core::Bus::NotificationPosted` topic, drives the master annunciator in the taskbar through
Q_PROPERTYs, and is reached by QML as `Cpp_UI_Alarms`, by the palette through five registry
commands, and by the API through a new `alarms.*` handler. Project overrides ride on one new
`AlarmBand.sound` field and one new project-level `sounds` object mirroring the `mqttPublisher`
blob pattern. The vendored miniaudio switches stay exactly as they are.

## Affected subsystems & files

Every path below was confirmed by grep on 2026-09-27.

| File | Change |
|------|--------|
| `core/Devices/IO/Audio/WavDecoder.h/.cpp` | **New.** RIFF/WAVE parser: PCM 8/16/24/32-bit integer and 32-bit float, mono/stereo, 8 to 96 kHz, 10 s cap. Output: interleaved float, source rate and channel count. Rejects anything else with a reason string. No Qt beyond `QByteArray`/`QString`. |
| `core/Devices/IO/Audio/SoundBank.h/.cpp` | **New.** Fixed slot table (one per theme slot, plus per-project override slots) of device-rate stereo float buffers. `load(slot, path)` decodes and converts on the GUI thread; retired buffers go to a graveyard freed only after the callback generation counter passes the retirement mark. |
| `core/Devices/IO/Audio/SoundPlayer.h/.cpp` | **New.** `ma_context` + playback `ma_device` (10 ms period, 3 periods, float32 stereo at the device's native rate), output-device enumeration by stable id, SPSC command ring (32 entries: start/stop lane, slot, gain), two-lane mixer callback, fixed 12 dB event-lane duck while the alarm lane is active, `health()` (running, lost-device flag from the stop notification), master gain atomic. |
| `core/Devices/CMakeLists.txt` | Add the three source pairs to the Devices target. **Found during implement (2026-09-27):** `ThirdParty/miniaudio.{h,cpp}`, `ss_apply_miniaudio_definitions`, the vendored TU's warning suppressions, the unity skip and the ALSA link all sat inside `if(BUILD_COMMERCIAL)`; the spec's every-build requirement moves that block out of the gate (the Audio driver sources stay Pro). The define set itself is unchanged. |
| `core/Devices/IO/ConnectionManager.h/.cpp` | Add `[[nodiscard]] bool lastCloseRequested() const noexcept` backed by a flag set in `disconnectDevice()` before the close and cleared in `connectDevice()`; lets the annunciator tell Disconnected from Link Lost on `connectedChanged`. |
| `core/Core/Bus/Messages.h` | **New topic** `AppEventRaised { int kind; }` with `kAppEvent*` ordinals (ErrorDialogShown, ExportFinished). Published by `Misc::Utilities` (Ui) and the Historian report/CSV export (Storage); subscribed by `AppEventSounds`. |
| `scripts/bus-census.json` | `--accept` once for the new topic (publisher + subscriber both land in this spec). |
| `core/Core/DataModel/FrameKeys.h` | New keys: `Sound("sound")` on a band; project-level `Sounds("sounds")`, `Sequence("sequence")`, `Channels("channels")`, `SoundWarning("warning")`, `SoundCaution("caution")`, `SoundAdvisory("advisory")`. |
| `core/Core/DataModel/Frame.h` | `AlarmBand` gains `QString sound;` (after `label`, keeps the alignas contract); `serialize(const AlarmBand&)` writes it only when non-empty. |
| `core/Core/DataModel/Frame.cpp` | `read(AlarmBand&)` reads `Keys::Sound` (default empty, `.simplified()`). |
| `core/Pipeline/DataModel/ProjectModel.h/.cpp` | `Q_PROPERTY(QJsonObject sounds ...)`, `sounds()`, `setSounds()` with `ProjectUndoScope` + `setModified(true)` + `soundsChanged()`, reset in `newJsonFile()` next to the MQTT/Influx blobs, `emitSinkConfigResets` gains the third flag. |
| `core/Pipeline/DataModel/Project/ProjectLoader.cpp` | `loadSinkConfigs` also reads `Keys::Sounds` (absent = empty object). |
| `core/Pipeline/DataModel/Project/ProjectPersistence.cpp` | Writes `Keys::Sounds` when non-empty. |
| `core/Ui/UI/AlarmMonitor.h/.cpp` | New signal `bandTransition(int uniqueId, int severity, const QString& title, const QString& label, double value)` emitted on every band index change (including exit to none, severity -1) after `initialized`; the existing 3 s notification cooldown is untouched and applies to notifications only. `rebuildTrackers` emits `trackersRebuilt()` so the annunciator can drop band points that no longer exist. |
| `core/Ui/UI/Alarms/AnnunciatorSequence.h/.cpp` | **New.** Pure class, no Qt signals: point table keyed by `{kind, id}` (band uniqueId or channel+title hash), states Normal/Alert/Acknowledged/ReturnToNormal, sequence A/M/R transitions per ISA-18.1, `acknowledge()`, `silence()`, `reset()`, `raise(point, priority)`, `clear(point)`, `soundingPriority()` arbitration, `unacknowledgedCount()`, `highestActivePriority()`, `ringbackPending()`. Silence sets a per-point `silenced` flag cleared by any re-raise or priority rise (reflash). **Found during implement (2026-09-28):** the enum grew the ISA-18.1 option-4 variants `A4`/`M4`/`R4` (`baseSequence()`, `locksIn()`); `settle()` keeps an unacknowledged point in Alert only under a plain sequence, so under the default A-4 the audible stops when the condition clears. Codes `A-4`.. parse in `SoundTheme::sequenceFromLetter`, the editor validates through it. `clear()` slot / `alarms.clear` drops the whole table for the panel's Clear button. |
| `core/Ui/UI/Alarms/SoundTheme.h/.cpp` | **New.** QSettings-backed slot table (`AlarmSounds/...`): enabled, muted, volume, sequence, output device id, per-slot file + enabled, per-priority repeat interval clamped to the IEC ranges. Resolves the effective file for a point: band override → channel map → theme slot → bundled `qrc:/sounds/<slot>.wav`. Loads the bank, validates picks through `WavDecoder`, tracks unresolved paths for the Problem Center checker. |
| `core/Ui/UI/Alarms/AppEventSounds.h/.cpp` | **New.** Subscribes to `ConnectionStateChanged` (Connected / Reconnected / Disconnected / Link Lost via `lastCloseRequested()`), CSV/MDF4/Historian `openChanged` edges (Recording Started/Stopped), `AppEventRaised` (Error Dialog Shown, Export Finished), and exposes `playEvent(name)` for QML (Button Pressed, Toggle Changed). |
| `core/Ui/UI/Alarms/AlarmAnnunciator.h/.cpp` | **New.** The facade: owns the three sub-objects and the `IO::SoundPlayer`; Q_PROPERTYs `highestPriority`, `unacknowledgedCount`, `sounding`, `soundingPriority`, `alerting` (any point in Alert, drives the flash), `muted`, `enabled`, `volume`, `sequence`, `outputDevices`, `outputDeviceIndex`, `points` (QVariantList for the context menu / API); slots `acknowledge()`, `silence()`, `reset()`, `test()`, `setMuted()`, `playEvent()`; repeat scheduling on one re-armed `QTimer`; 1 Hz health poll off `Misc::TimerEvents::timeout1Hz`; ProblemCenter checker `alarms.sounds` (LinkSample \| ProjectChanged) reporting lost device and unresolved files. |
| `core/Ui/CMakeLists.txt` | Add the four Alarms pairs and the handler. **Found in review:** also `ss_apply_miniaudio_definitions(SerialStudioUi)`, because `AlarmAnnunciator.h` embeds `SoundPlayer` (miniaudio structs) by value; `app/CMakeLists.txt` gets the same call for the executable. |
| `core/Ui/ApiHandlers/AlarmsHandler.h/.cpp` | **New**, unconditional (not `BUILD_COMMERCIAL`). Verbs: `alarms.state`, `alarms.acknowledge`, `alarms.silence`, `alarms.reset`, `alarms.test`, `alarms.setMuted`, `alarms.getProjectSounds`, `alarms.setProjectSounds`. Holds a pointer bound at registration, never an `instance()`. |
| `core/Ui/ApiHandlers/UiHandlers.h/.cpp` | `registerAll(UI::AlarmAnnunciator&)`. |
| `core/Ui/Misc/Utilities.cpp` | `showMessageBox` / `postMessageBox` publish `AppEventRaised{ErrorDialogShown}` when the icon is Warning or Critical, through `Core::services().bus`. |
| `core/Storage/Sessions/DatabaseManager.cpp` | Publish `AppEventRaised{ExportFinished}` on the Historian exporter's `csvFinished` / `pdfFinished` success edges (the only one-shot exports; CSV/MDF4/Console/Image sinks are recordings). **Found during implement:** both flows already terminate in `SessionExporter` signals the manager wires, so `HtmlReport.cpp` stays untouched. Pro TU; the topic is Core and `Utilities.cpp` keeps a GPL publisher. |
| `core/Ui/Misc/ContextRegistry.cpp` | Add `Cpp_UI_Alarms` to `buildObjectNames()`. |
| `core/Ui/Misc/Problems/` | No new file: the checker lambda lives in `AlarmAnnunciator`. |
| `app/src/Misc/ModuleManager.h/.cpp` | `std::unique_ptr<UI::AlarmAnnunciator> m_alarms` constructed at the end of `instantiateCoreModules()` after `Dashboard` (takes `Dashboard&`, `NotificationCenter&`, `ConnectionManager&`, `ProblemCenter&`, the bus); `setupExternalConnections()` in both `setupCrossModuleConnections()` (after `AlarmMonitor`) and `setupHeadlessSessionConnections()`; `registerApiHandlers()` passes it to `UI::ApiHandlers::registerAll`; `registry.add("Cpp_UI_Alarms", m_alarms.get())`; released before `SessionContext::shutdown()` alongside the other root-owned objects. |
| `app/src/main.cpp`, `app/src/Misc/CLI.cpp` | **Found at first run:** `onQuit()` runs `stopFrameConsumerWorkers()` while QML is alive, so destroying the annunciator there nulls every `Cpp_UI_Alarms` binding. The worker stop now only stops audio; each root calls `ModuleManager::releaseAnnunciator()` right before `SessionContext::shutdown()`, once the engine is gone. |
| `app/qml/Widgets/MiniWindow.qml`, `app/qml/Widgets/Dashboard/DashboardToolButton.qml` | **Maintainer request at first run:** Button Pressed also covers the mini-window chrome (menu, minimize, maximize/restore, close) and the dashboard tool buttons. |
| `app/rcc/commands/app.json` | Five commands: `alarms.acknowledge` (Ctrl+Shift+A), `alarms.silence` (Ctrl+Shift+H), `alarms.reset` (Ctrl+Shift+R), `alarms.test` (Ctrl+Shift+T), `alarms.mute` (toggle, Ctrl+Shift+U); `contexts: ["app", "dashboard"]`, `category: "tools"`, `shortcutWindows: ["main"]`. |
| `app/qml/Commands/AppCommandBindings.qml` | Five `cmd…` entries calling `Cpp_UI_Alarms`; the dashboard palette model already includes the app set, so one binding set covers both palettes. |
| `app/rcc/icons/notifications/{16,24,32}/` | `annunciator.svg`, `acknowledge.svg`, `silence.svg`, `mute.svg`, `test.svg` (reuse `commands/reset`). |
| `app/rcc/sounds/*.wav` | **New assets** (14): `warning`, `caution`, `advisory`, `ringback`, `connected`, `disconnected`, `link-lost`, `reconnected`, `export-finished`, `recording-started`, `recording-stopped`, `error`, `button`, `toggle`. 48 kHz, 16-bit, mono, one burst each. |
| `app/rcc/rcc.qrc` | `sounds/` and icon entries. |
| `REUSE.toml` | Add `app/rcc/sounds/**` to the first-party asset list. |
| `app/CMakeLists.txt` | **Found during implement:** the QML module lists every `.qml` file, so the three new pages are registered there (no other change). |
| `app/qml/Dialogs/Settings.qml` | Tab "Sounds" after Export (index 7 in GPL builds, Notifications stays last and Pro-only), page instance, `implicitHeight` entry, reset block. |
| `app/qml/Dialogs/Settings/SettingsSoundsPage.qml` | **New.** Sections: Output (enable, volume, output device Combo, mute), Alarms (sequence Combo; rows Warning/Caution/Advisory/Ringback: file field + browse + play, interval SpinBox where applicable), Events (per-event enable + file + play), Test button, Reset to bundled. File fields follow the `modelValue`/unfocused-sync pattern. |
| `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml` | **New.** Tray item: priority-coloured pill with count, fast flash (450 ms half-period, same as LED blink) while `alerting`, slow flash (1000 ms) while ringback pending, steady when `reduceMotion`; click = acknowledge; right-click menu Silence / Reset / Test / Mute; muted glyph. **Found during implement (2026-09-28):** reshaped on review: an `IconButton` bell (icon carries the colour, not the background) that is always in the tray; left click opens a `Popup` panel (points newest first, click navigates to the widget, header with Acknowledge / Silence / Reset / Clear / Test / Mute as compact `PanelButton`s), right click acknowledges. Icons live in the flat `buttons/` set, so the `alarms.*` manifest commands carry no icon. |
| `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml` | Instantiate `MasterAnnunciator` beside `_thinningBadge` in the tray row; visible when `Cpp_UI_Alarms.highestPriority >= 0 || Cpp_UI_Alarms.muted`. |
| `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml` | New `bandSound` role and a Sound column (browse button + clear); `collectBands()` emits `sound`. |
| `core/Ui/ProjectEditor/EditorForms.cpp` | `bandsToVariantList` and `alarmBandsEqual` include `sound`; `openChannelSoundsEditor()` emits the current project map. |
| `core/Ui/ProjectEditor/EditorCommit.cpp` | `parseAlarmBandList` reads `sound`; new `commitChannelSounds(QVariantList)` and `setProjectSoundSequence(QString)` write through `ProjectModel::setSounds`. |
| `core/Ui/ProjectEditor/ProjectEditor.h` | Signal `openChannelSoundsEditor(QVariantList)`; slots `openChannelSoundsEditor()`, `commitChannelSounds()`, `setProjectSoundSequence()`; Q_PROPERTY `projectSoundSequence`. |
| `app/qml/ProjectEditor/Dialogs/ChannelSoundsEditor.qml` | **New.** Rows: channel name, Warning / Caution / Advisory file pickers; Apply → `commitChannelSounds`. Same launcher-signal grammar as `AlarmBandsEditor`. |
| `app/qml/ProjectEditor/Views/ProjectView.qml` | "Alarm Sounds" section: sequence Combo (Default / A / M / R) and "Edit Channel Sounds…" button. |
| `app/qml/ProjectEditor/ProjectEditor.qml` | Instantiate the dialog and connect `onOpenChannelSoundsEditor`. |
| `app/qml/Widgets/ToolbarButton.qml`, `IconButton.qml`, `BigButton.qml`, `TaskbarButton.qml`, `MenuButton.qml` | `Cpp_UI_Alarms.playEvent("button")` on `clicked`. |
| `app/qml/Widgets/Dashboard/Output/DashboardButton.qml`, `DashboardToggle.qml` | `playEvent("button")` / `playEvent("toggle")`. |
| `app/tests/tst_annunciator_sequence.cpp`, `app/tests/tst_wav_decoder.cpp`, `app/tests/CMakeLists.txt` | **New** ctest suites (see Test plan). |
| `tests/unit/test_alarm_sounds.py` | **New** pure-Python asset check (AC3). |
| `tests/integration/test_aural_alerts.py` | **New** API-driven suite (AC1, AC2, AC4, AC5, AC9, AC11). |
| `doc/help/Aural-Alerts.md`, `doc/help/help.json` | **New page** (section Configuration) + index entry. |
| `doc/help/Notifications.md`, `doc/help/Widget-Reference.md` | Link the new page; document `sound` on a band. |
| `doc/claude/architecture/dashboard.md`, `doc/claude/directory-map.md`, `CLAUDE.md` | Alarm-band section gains the annunciator paragraph; the two new directories; one line in Project Overview. |
| `scripts/generate-command-strings.py` output (`core/Ui/UI/CommandStrings.cpp`) | Regenerated by sanitize, never hand-edited. |

## Architecture & data flow

```
Dashboard::updated (display tick, GUI thread)
  └─ AlarmMonitor::evaluateAlarms ──bandTransition(id, sev, …)──▶ AlarmAnnunciator
NotificationCenter::appendEvent ──bus NotificationPosted──▶ AlarmAnnunciator
                                                             │  (Warning/Critical → raise point,
                                                             │   "Resolved: X" Info → clear point,
                                                             │   other Info → Advisory one-shot)
AlarmAnnunciator
  ├─ AnnunciatorSequence: point table, A/M/R transitions, arbitration
  ├─ SoundTheme: effective file per point, bank slots, settings
  ├─ AppEventSounds: bus + facade signals → event lane
  └─ IO::SoundPlayer ──SPSC command ring──▶ ma_device callback (audio thread)
                                              └─ mixes alarm lane + ducked event lane
```

- **Band points.** `bandTransition` fires on every band index change once the tracker is
  initialized. Severity ≥ 2 raises the point at Warning (3) or Caution (2); severity < 2 or -1
  clears it. `trackersRebuilt` (project reload, `dataReset`) clears every band point; connection
  loss clears all points (R19) through `ConnectionStateChanged`.
- **Notification points.** Keyed by `(channel, title)`. `Critical` raises Warning, `Warning`
  raises Caution. An Info whose title starts with `Resolved: ` clears the point with the
  remaining title (the shape `NotificationCenter::resolve` produces). Any other Info plays one
  Advisory burst on the alarm lane only if the lane is idle (R6) and creates no point.
- **Arbitration and repetition.** After every table change the facade asks the sequence for the
  sounding priority (highest priority among points in Alert and not silenced; ringback only when
  none). A change of sounding priority, or a re-raise at the sounding priority, restarts that
  slot's burst now; the single `QTimer` is re-armed to `lastBurstStart + interval(priority)`.
  Advisory never repeats.
- **Actions.** `acknowledge()` → sequence acknowledges every Alert point, stops the alarm lane,
  cancels the timer. `silence()` → marks Alert points silenced, stops the lane; a new raise or a
  priority rise clears the mark and re-sounds. `reset()` → Return-to-normal points go Normal
  (A: no-op). `test()` → plays Advisory, Caution, Warning, Ringback in that order on the event
  lane with a 1.5 s spacing and drives a `testing` property the annunciator UI renders through
  each state; no point is created.
- **Threading.** Everything above the ring runs on the GUI thread. The callback reads the ring
  and the bank's slot pointers only; the bank never frees a buffer the callback might still be
  reading (graveyard + generation counter). `SoundPlayer` never emits signals from the callback;
  the stop notification sets an atomic the 1 Hz poll reads.
- **Device selection.** `SoundTheme` stores the miniaudio device id (as bytes) plus the display
  name. On start, `SoundPlayer` resolves the id; missing → default device + lost flag →
  checker finding "Alarm sound device '<name>' not found, playing on the system default".
  The 1 Hz poll re-enumerates only while the lost flag is set and rebinds when the id returns.
- **Headless.** `--headless` and the benchmark roots construct the annunciator but
  `setupExternalConnections()` skips `SoundPlayer::start()` when `ModuleManager::m_headless`
  is set or the device init fails; the state machine and API stay live.
- **Reduce Motion.** `MasterAnnunciator.qml` reads `Cpp_Misc_GraphicsBackend.reduceMotion`;
  flash animations gate on it and fall back to a steady two-colour state.

## Hotpath & threading impact

- **Touches the hotpath?** No. Nothing on `FrameReader` / `CircularBuffer` / `FrameBuilder` /
  Dashboard ingest / the span lane. The only per-tick work is the existing
  `AlarmMonitor::evaluateAlarms` on `Dashboard::updated`, which gains one signal emission per
  band *transition* (not per frame, not per tick). `--benchmark-hotpath` runs unchanged as
  AC10; no code in this plan is in a benchmark TU.
- **New cross-thread signal/slot?** No Qt cross-thread connection. The one cross-thread
  boundary is the miniaudio callback, crossed by an SPSC ring of POD commands and atomics
  (master gain, lost flag, generation counter). `NotificationPosted` subscription is
  `Qt::AutoConnection` on the GUI-affine facade, same as `NotificationCenter`'s own bus
  subscription; posts already marshal to the GUI thread inside the center.
- **New input to a cached hotpath flag?** None. `m_operationMode`, `m_anyAsyncSink`,
  `m_captureLatestFrame`, `m_streamAvailable`, `m_changeDriven` are untouched.
- **Timestamp ownership.** Not involved; the annunciator stamps its own burst starts with the
  monotonic clock for the API only.

## Data model & persistence

- **Band:** `AlarmBand.sound` (`Keys::Sound`, JSON `"sound"`), optional string. Absent or empty
  = use priority default. Serialized only when non-empty, so untouched projects are
  byte-identical. Flows through the existing `readAlarmBands`/`writeAlarmBands` hooks; no
  change to `app/rcc/properties/dataset.json` or any generated artifact.
- **Project:** `Keys::Sounds` (`"sounds"`), opaque object owned by the annunciator's theme
  parser, stored like `mqttPublisher`:

  ```json
  "sounds": {
    "sequence": "M",
    "channels": {
      "Engine": { "warning": "sounds/horn.wav", "caution": "", "advisory": "" }
    }
  }
  ```

  `sequence` ∈ `"A" | "M" | "R"`, absent = app preference. A relative path resolves against the
  project file's directory; an absolute path is used as-is. Absent key = empty object = no
  override; older readers ignore it. No schema version bump: additive optional keys.
- **Preferences (QSettings):** `AlarmSounds/enabled` (true), `/muted` (false), `/volume`
  (80), `/sequence` ("A"), `/outputDeviceId`, `/outputDeviceName`, `/slot/<name>/file` (empty =
  bundled), `/slot/<name>/enabled`, `/interval/warning` (5000), `/interval/caution` (10000).
- **Sanitized inputs:** file picks and API paths go through `WavDecoder::probe()` before they
  are stored; a rejected file leaves the slot unchanged and returns the reason (R10).

## API / SDK surface

New handler `API::Handlers::AlarmsHandler`, registered from `UI::ApiHandlers::registerAll`
(unconditional; the notification log stays Pro upstream). Verbs and shapes:

| Verb | Params | Reply |
|------|--------|-------|
| `alarms.state` | – | `{ sequence, muted, enabled, highestPriority (-1/0/1/2 → none/Advisory/Caution/Warning), unacknowledgedCount, alerting, sounding: { priority, slot, file, burstStartedMs, burstCount } \| null, ringbackPending, points: [{ kind: "band"\|"notification", id, title, channel, priority, state: "alert"\|"acknowledged"\|"returnToNormal", silenced, sinceMs }] }` |
| `alarms.acknowledge` / `alarms.silence` / `alarms.reset` / `alarms.test` | – | `{ ok, unacknowledgedCount }` |
| `alarms.setMuted` | `muted: bool` | `{ muted }` |
| `alarms.getProjectSounds` | – | the project `sounds` object |
| `alarms.setProjectSounds` | `sounds: object` | `{ ok, rejected: [{ path, reason }] }` — validates every path, stores the object, reports what will fall back |

`project.exportJson` / `project.loadJson` carry the `sounds` key and the band `sound` field
through the existing serializers, which is the round-trip AC11 checks. `generate-sdk.py`
picks the verbs up from the registry during sanitize. Priority ordinals are exposed once in
`EnumLabels.cpp` as slugs `warning`/`caution`/`advisory` so scripts never see magic numbers.

## QML / UI

- **`Cpp_UI_Alarms`** is the single context property (facade). Properties listed above; the
  `outputDevices` list is refreshed on demand when the Sounds page opens (no polling).
- **Sounds page** mirrors `SettingsNotificationsPage.qml` layout grammar (section labels,
  GridLayout, Switch rows). File rows: `LineField` with the `modelValue`/unfocused-sync
  pattern, a browse button (`FileDialog`, `*.wav`), a play button, a clear button. A rejected
  pick shows the reason inline under the row.
- **Master annunciator** is an `IconButton` bell in `Taskbar.qml`'s tray row, always visible.
  Colours: Warning → `colors["alarm"]`, Caution → the theme's warning band colour, Advisory →
  info, idle → `taskbar_text`. It does not appear in the pinned-button row (not a pin, not
  reorderable). The panel is a plain `Popup` here, not a registry `CommandMenu`, because this
  is dashboard chrome, not a Project Editor surface; its buttons call the same slots as the
  five bindings plus `clear()`.
- **Palette / shortcuts** come from the five `app.json` commands; each id is bound exactly once
  (main window `Instantiator`), avoiding the ambiguous-shortcut trap.
- **Project Editor:** `AlarmBandsEditor` gets one more column; `ProjectView` gets a two-row
  section; `ChannelSoundsEditor` is a new dialog in the `AlarmBandsEditor` shape (launcher
  signal → dialog → commit slot).
- **Chrome motion:** the pill's flash is an `opacity` animation gated on `reduceMotion`; no
  width/height animation, nothing inside `Widgets/Dashboard/`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Audio backend | (a) Qt Multimedia `QSoundEffect`; (b) miniaudio `ma_engine` with decoding/engine switches turned on; (c) own two-lane mixer on a raw `ma_device` | **(c).** (a) is not linked into `core/`, has no device pick and misses R11's 30 ms. (b) is the least code but pulls the resource manager's worker threads and the node graph into a process already running an RT capture callback, re-inits (and drops every loaded sound) on device change, and grows both Pipeline and Devices archives; the shared define function exists precisely to keep that out. (c) is the Audio driver's own idiom (`PlaybackRing`), keeps the vendored switches untouched, and a PCM WAV parser is ~120 lines with a ctest. |
| Where the player lives | Ui (needs miniaudio defines on a third target) vs Devices (owns the header) | **Devices**, `IO/Audio/`. Ui already links Devices; the player has no upward dependency. |
| Annunciator ownership | New `instance()` singleton (census re-seed) vs root-owned `unique_ptr` like `SimdSettings` | **Root-owned.** Zero census growth, injected references, one construction site; handler and QML get a pointer at registration. |
| Where band transitions come from | Rewrite `AlarmMonitor` into the annunciator vs one new signal on `AlarmMonitor` | **One signal.** `AlarmMonitor` keeps its notification contract and cooldown; the annunciator is additive (spec constraint "existing behavior unchanged"). |
| Notification points source | Subscribe to `NotificationCenter::notificationPosted` (Pipeline signal) vs bus `NotificationPosted` | **Bus topic.** Same payload, already published post-dedup, no new Ui→Pipeline signal wiring. |
| Advisory semantics | Standing point that needs acknowledge vs one-shot burst | **One-shot, no point** (spec R16 amended 2026-09-27). A standing Info point would inflate the unacknowledged count with chatter. |
| Project-level override editing | New tree node kind like `KindMqttPublisher` (model + view + tree plumbing) vs a section in `ProjectView` + a dialog | **Section + dialog.** Two rows of chrome and one dialog in the proven `AlarmBandsEditor` shape; no tree-kind, no `CustomModel`. |
| Channel map granularity | Per channel per priority vs one file per channel | **Per priority** (maintainer choice, spec). |
| Device loss | Stop and warn vs fall back + warn + auto-rebind | **Fall back + warn + rebind** (maintainer choice). Rebind polling only while lost, at 1 Hz. |
| Keeping the device open | Open per burst vs keep running while enabled | **Keep running** while `enabled && !muted`; idle callback cost is negligible and it is what makes the 30 ms bound trivial. Muting or disabling stops the device. |
| Bundled sound authoring | Synthesize in-app at runtime vs ship WAVs | **Ship WAVs** (spec). Alex renders in Ableton per the R8 table; a throwaway scratchpad Python synth produces IEC-shaped placeholders so the build never lacks assets and AC3 can run before the final render lands. |
| UI feedback coverage | Every `Button`/`Switch` in the app vs the shared components | **Shared components only** (`ToolbarButton`, `IconButton`, `BigButton`, `TaskbarButton`, `MenuButton`, the two output widgets). Bare Controls in dialogs stay silent; this is a theme, not an accessibility contract. |
| Shortcuts | From the audit: Ctrl+Shift+A / H / R / T / U | Nothing in `app.json`, `dashboard.json`, `projecteditor.json` or the hand-written `Shortcut` blocks claims them; Ctrl+Shift+M (minimize), Ctrl+Shift+F/L/W (taken) and Alt chords (AltGr) avoided. Mute's "U" is weak but every mnemonic key is taken. |

## Risks & mitigations

- **Allocation or blocking in the audio callback.** The callback touches only the ring, slot
  pointers and the output buffer. `tst_wav_decoder` cannot test that; the review checklist
  and `qt-cpp-review` do. Bank retirement uses the generation counter so a swapped buffer is
  never freed under the callback.
- **Static-cache census growth.** No `static auto& x = X::instance()` anywhere in the new code;
  the facade holds injected references. `code-verify.py --singleton-census --check` gates it.
- **Bus census.** A new topic must have both a publisher and a subscriber; both land here,
  then `--bus-census --accept`.
- **Layering.** Ui → Devices include (`IO/Audio/SoundPlayer.h`) is allowed; Devices includes
  only Core. `layer-verify.py` gates it.
- **`undo-scope-missing`.** `ProjectModel::setSounds` opens `ProjectUndoScope` and calls
  `setModified(true)` exactly like `setMqttPublisher`.
- **Commercial guard in `Commands/*.qml`.** The five bindings reference only `Cpp_UI_Alarms`;
  no `Cpp_Licensing_`/`Cpp_Sessions_`/`Cpp_MQTT_` lines.
- **Ambiguous shortcuts.** Each sequence bound once, through the manifest only.
- **Editable field echo.** File path fields use the unfocused-sync pattern.
- **Startup sounds (R18).** `SoundTheme` loads the bank and `SoundPlayer` starts inside
  `setupExternalConnections()`, but the sequence table starts empty and `AlarmMonitor` only
  emits transitions after a tracker's first sample; `restoreLastProject()` therefore cannot
  produce a burst. The Connected event is suppressed for the first `connectedChanged` seen
  before `restoreLastProject()` returns (an ephemeral session may auto-connect).
- **Pro TU publishing a Core topic.** The Historian TUs are compiled only under
  `BUILD_COMMERCIAL`; the topic keeps a GPL publisher (`Utilities.cpp`), so the bus census is
  satisfied in both builds. Recording Started/Stopped come from the CSV, MDF4, Console and
  Historian sinks' `openChanged` edges (Image export closes per group and is left out).
- **qrc growth.** 14 WAVs at 48 kHz/16-bit mono: bursts ≤ 2.5 s, events ≤ 0.6 s, roughly 1.2 MB
  total. Acceptable; checked in the asset test.
- **Headless CI.** `SoundPlayer::start()` failing (no device on a runner) is a logged
  no-op; every API test runs against the state machine only.
- **TU length.** Four Ui TUs and three Devices TUs, each well under 1500 lines; the facade
  delegates rather than accretes.
- **Missing translation of command strings.** `generate-command-strings.py` runs in sanitize;
  the five titles/tooltips reach `CommandStrings.cpp` automatically.

## Test & verification plan

- **Unit, ctest (maintainer builds, I run `ctest` against the build dir):**
  - `tst_annunciator_sequence`: sequence A/M/R transition tables (AC1 logic), acknowledge vs
    silence vs reflash (AC2 logic), arbitration order Warning > Caution > Advisory > Ringback
    (R6), repeat interval clamping (R7), reset semantics per sequence (R4).
  - `tst_wav_decoder`: synthetic WAVs for every accepted format, rejection of non-PCM, >10 s,
    truncated headers (R10); conversion output length matches rate × channels.
- **Unit, pure Python (I can run):** `tests/unit/test_alarm_sounds.py` — reads every
  `app/rcc/sounds/*.wav` with `wave` + `numpy`: format 48 kHz/16-bit/mono, duration caps,
  envelope pulse counts 10/3/1/2 for warning/caution/advisory/ringback, FFT fundamental in
  150 to 1000 Hz for the four alarm files, no pulse-burst structure in event files (AC3, R8, R9).
- **Integration (maintainer launches the app with the API server; I run):**
  `tests/integration/test_aural_alerts.py`:
  - AC1: `notifications.post` Critical → `alarms.state` shows one Warning point in `alert`;
    `alarms.acknowledge` → `acknowledged`; `notifications.resolve` → gone (A) / `returnToNormal`
    (M, R via `alarms.setProjectSounds{sequence}`); `alarms.reset` → gone. Band path: load a
    project with a Critical band, stream a value into it through the device simulator.
  - AC2: Caution then Warning; `sounding.priority` Warning; acknowledge only the Warning
    point through a resolve+repost dance; `sounding.priority` Caution; `alarms.silence`;
    new Caution post → `sounding` non-null again.
  - AC4: `sounding.burstStartedMs` advances by the configured interval; Advisory
    `burstCount == 1` after 3 × interval.
  - AC5: `alarms.setProjectSounds` with a `.txt` path → `rejected` non-empty and stored map
    unchanged; a missing `.wav` path → `problems.list` carries `alarms.sounds` and the point
    still alerts.
  - AC9: after `project.load` of an example with bands, `alarms.state.points == []` and
    `sounding == null`; disconnect during an alert under sequence R → points empty,
    `ringbackPending == false`.
  - AC11: channel map round-trip through `project.exportJson` and reload; `sounding.file`
    reports the mapped file for the mapped channel.
- **Maintainer observations:** AC6 (loopback onset < one display frame; figure recorded
  here), AC7 (annunciator behaviour, Reduce Motion, Mute persistence, Sounds page, band
  override), AC8 (event lane ducks under a burst; Button Pressed silent by default), AC12
  (USB device pick, unplug, replug).
- **Hotpath:** `--benchmark-hotpath` at the current floor (AC10); no hotpath TU is touched.
- **Static:** `python scripts/code-verify.py --check` on every touched file;
  `scripts/registry-verify.py` (icons, commands, bindings); `scripts/layer-verify.py`;
  `scripts/generate-command-strings.py --check`; `qt-cpp-review` before handoff;
  `python scripts/sanitize-commit.py` before commit; `reuse lint`.
