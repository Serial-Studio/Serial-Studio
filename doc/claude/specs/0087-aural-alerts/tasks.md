---
spec: 0087-aural-alerts
phase: tasks
status: approved     # approved 2026-09-27
updated: 2026-09-27
---

# Tasks 0087 — Aural alerts

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
- `CV` below abbreviates `python scripts/code-verify.py --check <the task's files>`.
- The maintainer builds; a ctest or integration line in Verify runs only against an existing
  build or a running app. The pure-Python unit in T27 runs anywhere.

## Phase A — Data model and keys

### T1 — Keys and `AlarmBand.sound`

- **Files:** `core/Core/DataModel/FrameKeys.h`, `core/Core/DataModel/Frame.h`,
  `core/Core/DataModel/Frame.cpp`
- **Does:** Add `Keys::Sound`, `Keys::Sounds`, `Keys::Sequence`, `Keys::Channels`,
  `Keys::SoundWarning`, `Keys::SoundCaution`, `Keys::SoundAdvisory`. Add `QString sound;` to
  `AlarmBand` after `label` (keep the `alignas(8)` static_assert green); `serialize` writes
  it only when non-empty; `read` takes it `.simplified()`, default empty. Untouched projects
  stay byte-identical on save.
- **Verify:** CV; read back `serialize`/`read` symmetry; `ctest -R tst_bar_bands` still passes
  against the existing build (struct change only).
- **Deps:** none
- [x] done

### T2 — Project-level `sounds` object on `ProjectModel`

- **Files:** `core/Pipeline/DataModel/ProjectModel.h`, `core/Pipeline/DataModel/ProjectModel.cpp`
- **Does:** `Q_PROPERTY(QJsonObject sounds READ sounds WRITE setSounds NOTIFY soundsChanged)`,
  member `m_sounds`, `setSounds()` with guard return, `ProjectUndoScope` and `setModified(true)`
  exactly like `setMqttPublisher` (`undo-scope-missing` only sees the literal `setModified(true)`,
  so it must be there), reset in `newJsonFile()` beside the two sink blobs,
  `emitSinkConfigResets` gains a third flag.
- **Verify:** CV; grep confirms `setSounds` opens the scope; no ctor-closure code touched
  (`newJsonFile` only clears a member, no `instance()` reach).
- **Deps:** T1
- [x] done

### T3 — Load and persist `sounds`

- **Files:** `core/Pipeline/DataModel/Project/ProjectLoader.cpp`,
  `core/Pipeline/DataModel/Project/ProjectPersistence.cpp`
- **Does:** `loadSinkConfigs` reads `Keys::Sounds` (absent = empty object, emits
  `soundsChanged`); persistence inserts it only when non-empty.
- **Verify:** CV; read-back that an example project without the key serializes unchanged.
- **Deps:** T2
- [x] done

## Phase B — Audio engine (`core/Devices/IO/Audio/`)

### T4 — `WavDecoder`

- **Files:** `core/Devices/IO/Audio/WavDecoder.h`, `core/Devices/IO/Audio/WavDecoder.cpp`,
  `core/Devices/CMakeLists.txt`
- **Does:** RIFF/WAVE parser for PCM 8/16/24/32-bit integer and 32-bit float, 1 or 2 channels,
  8 to 96 kHz, 10 s cap; `probe(path, reason)` and `decode(path) -> {rate, channels, frames,
  std::vector<float>}`; every failure names its reason. Reuses the `AudioPcm` full-scale
  constants for normalization; no `reinterpret_cast` of unaligned bytes (use `qFromLittleEndian`
  on copies). Depends on Core and Qt Core only.
- **Verify:** CV; T25 ctest once built.
- **Deps:** none
- [x] done

### T5 — `SoundBank`

- **Files:** `core/Devices/IO/Audio/SoundBank.h`, `core/Devices/IO/Audio/SoundBank.cpp`,
  `core/Devices/CMakeLists.txt`
- **Does:** Fixed table of slots holding device-rate stereo float buffers; `load(slot, decoded,
  deviceRate)` converts through `ma_data_converter` on the caller's (GUI) thread; a replaced
  buffer moves to a graveyard tagged with the current callback generation and is freed by
  `collectGarbage(currentGeneration)` only once the generation has advanced. The callback side
  reads `const Sound*` by slot index through an atomic pointer array; never frees.
- **Verify:** CV; read-back of the retirement path; unit coverage in T25.
- **Deps:** T4
- [x] done

### T6 — `SoundPlayer` device and mixer

- **Files:** `core/Devices/IO/Audio/SoundPlayer.h`, `core/Devices/IO/Audio/SoundPlayer.cpp`,
  `core/Devices/CMakeLists.txt`
- **Does:** Own `ma_context` (separate from the Audio driver's), `enumerateOutputs()` (stable
  id + name), `start(deviceId)` / `stop()`, playback `ma_device` at float32 stereo, 10 ms
  period, 3 periods. SPSC command ring (32 POD entries: lane, slot, gain, op). Callback:
  drain ring, mix alarm lane and event lane, event lane at fixed -12 dB while the alarm lane
  is active, master gain atomic, increment generation counter, zero-fill on idle. **No
  allocation, lock, log or Qt call in the callback.** Stop notification sets an atomic lost
  flag; `health()` reports it; `rebindIfAvailable(deviceId)` re-enumerates on demand.
- **Verify:** CV; read-back that the callback body references only ring, bank pointers,
  atomics and the output buffer; `qt-cpp-review` thread-safety agent at handoff.
- **Deps:** T5
- [x] done

## Phase C — Annunciator (`core/Ui/UI/Alarms/`)

### T7 — `AnnunciatorSequence` (pure ISA-18.1 core)

- **Files:** `core/Ui/UI/Alarms/AnnunciatorSequence.h`, `core/Ui/UI/Alarms/AnnunciatorSequence.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Point table keyed `{kind, id}`; states Normal / Alert / Acknowledged /
  ReturnToNormal; sequences A, M, R per the ISA-18.1 tables; `raise`, `clear`, `clearAll`,
  `acknowledge`, `silence`, `reset`; per-point `silenced` cleared on re-raise or priority rise
  (reflash); `soundingPriority()` = highest priority among Alert-and-not-silenced points,
  ringback only when none and R has a ReturnToNormal point; `unacknowledgedCount`,
  `highestActivePriority`. No Qt signals, no allocation on the query paths, fixed loop bounds.
- **Verify:** CV; T25 ctest.
- **Deps:** none
- [x] done

### T8 — `SoundTheme` (settings, overrides, bank loading)

- **Files:** `core/Ui/UI/Alarms/SoundTheme.h`, `core/Ui/UI/Alarms/SoundTheme.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** QSettings-backed slot table under `AlarmSounds/` (enabled, muted, volume,
  sequence, output device id + name, per-slot file + enabled, per-priority interval clamped to
  the IEC ranges); slot catalog = 4 priorities/ringback + 10 events; `resolve(point)` = band
  override → project channel map → theme slot → bundled `:/sounds/<slot>.wav`; relative
  project paths resolve against the project directory; `setSlotFile` validates through
  `WavDecoder::probe` and refuses with the reason; loads the bank; tracks unresolved paths for
  the checker; `effectiveSequence()` = project override or preference.
- **Verify:** CV; read-back of the resolution order against spec R14.
- **Deps:** T3, T5
- [x] done

### T9 — `AlarmMonitor` transition signal

- **Files:** `core/Ui/UI/AlarmMonitor.h`, `core/Ui/UI/AlarmMonitor.cpp`
- **Does:** Emit `bandTransition(uniqueId, severity, title, label, value)` on every band index
  change after `initialized` (severity -1 on exit to no band), placed **before** the 3 s
  cooldown check so the annunciator sees every transition while notifications keep their
  cooldown; emit `trackersRebuilt()` at the end of `rebuildTrackers`. Signal wiring rule:
  same-thread Ui object, default (direct) connection; nothing per frame.
- **Verify:** CV; read-back that `processValue`'s notification path is unchanged.
- **Deps:** none
- [x] done

### T10 — `ConnectionManager::lastCloseRequested()`

- **Files:** `core/Devices/IO/ConnectionManager.h`, `core/Devices/IO/ConnectionManager.cpp`
- **Does:** Bool flag set at the top of the public `disconnectDevice()` and `toggleConnection()`
  close path, cleared in `connectDevice()`; `[[nodiscard]] bool lastCloseRequested() const
  noexcept`. Read existing signal wiring in the file first; no new signal, no change to
  `connectedChanged` timing.
- **Verify:** CV; read-back.
- **Deps:** none
- [x] done

### T11 — Bus topic `AppEventRaised`

- **Files:** `core/Core/Bus/Messages.h`, `core/Ui/Misc/Utilities.cpp`, `scripts/bus-census.json`
- **Does:** Declare `struct AppEventRaised final { int kind; }` with `kAppEventErrorDialogShown`
  and `kAppEventExportFinished`; `showMessageBox` / `postMessageBox` publish it through
  `Core::services().bus` when the icon is Warning or Critical. **Bus census requires a
  publisher and a subscriber per topic**; the subscriber lands in T12, so `--bus-census
  --accept` runs after T12, not here.
- **Verify:** CV; `python scripts/code-verify.py --bus-census` after T12 shows the topic
  balanced.
- **Deps:** none
- [x] done

### T12 — `AppEventSounds`

- **Files:** `core/Ui/UI/Alarms/AppEventSounds.h`, `core/Ui/UI/Alarms/AppEventSounds.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Subscribes `ConnectionStateChanged` (connected false→true: Connected, or
  Reconnected when the previous close was not requested; true→false: Disconnected when
  `lastCloseRequested()`, else Link Lost), `AppEventRaised`, and the `openChanged` edges of
  `CSV::Export`, `MDF4::Export`, `Console::Export` and (commercial) `Sessions::Export`
  reached through `API::handlerContext()` (Recording Started/Stopped); `playEvent(name)`
  for QML. Suppresses the first Connected seen before the facade's `armed` flag is set
  (R18). Publishes nothing per frame.
- **Verify:** CV; `--bus-census --accept` then `--bus-census` clean; read-back of the
  requested-vs-lost branch.
- **Deps:** T10, T11
- [x] done

### T13 — `AlarmAnnunciator` facade

- **Files:** `core/Ui/UI/Alarms/AlarmAnnunciator.h`, `core/Ui/UI/Alarms/AlarmAnnunciator.cpp`,
  `core/Ui/CMakeLists.txt`
- **Does:** Takes `Dashboard&`, `NotificationCenter&`, `ConnectionManager&`, `ProblemCenter&`,
  `TimerEvents&`, `MessageBus&` by reference (no `instance()` reach, no `static auto&`
  cache: the singleton census gates both). Owns the three sub-objects and `IO::SoundPlayer`.
  `setupExternalConnections()`: connect `AlarmMonitor::bandTransition`/`trackersRebuilt`,
  subscribe `NotificationPosted` (Warning/Critical raise; `Resolved: ` Info clears; other
  Info = Advisory one-shot when the alarm lane is idle), `Dashboard::dataReset` clears all,
  `ConnectionStateChanged` disconnect clears all without ringback, `timeout1Hz` health poll,
  register checker `alarms.sounds` (LinkSample | ProjectChanged). Q_PROPERTYs and slots per
  plan; single re-armed `QTimer` for repeats; `test()` sequence on the event lane with a
  `testing` property; `start()` of the player skipped when `headless` is passed or init fails.
- **Verify:** CV (TU under 1500 lines, functions under 100); read-back of every connection
  against the list above.
- **Deps:** T6, T7, T8, T9, T12
- [x] done

## Phase D — Composition root and API

### T14 — Root ownership and wiring

- **Files:** `app/src/Misc/ModuleManager.h`, `app/src/Misc/ModuleManager.cpp`,
  `core/Ui/Misc/ContextRegistry.cpp`
- **Does:** `std::unique_ptr<UI::AlarmAnnunciator> m_alarms` constructed at the end of
  `instantiateCoreModules()` after `Dashboard` and after `bindPipelineModuleSet`; **the pinned
  order is machine-checked by the `composition-root-order` anchor and this object is not a
  SessionContext slot, so it is constructed after the list, never inside it**;
  `setupExternalConnections()` after `AlarmMonitor` in `setupCrossModuleConnections()` and in
  `setupHeadlessSessionConnections()`; `registerApiHandlers()` passes `*m_alarms`;
  `registry.add("Cpp_UI_Alarms", m_alarms.get())` and the name in `buildObjectNames()`;
  released in `onQuit` before `SessionContext::shutdown()`.
- **Verify:** CV; `python scripts/claim-verify.py` (anchor unchanged);
  `python scripts/code-verify.py --singleton-census --check` shows no growth.
- **Deps:** T13
- [x] done

### T15 — `AlarmsHandler` API

- **Files:** `core/Ui/ApiHandlers/AlarmsHandler.h`, `core/Ui/ApiHandlers/AlarmsHandler.cpp`,
  `core/Ui/ApiHandlers/UiHandlers.h`, `core/Ui/ApiHandlers/UiHandlers.cpp`,
  `core/Ui/CMakeLists.txt`, `core/Core/EnumLabels.cpp`
- **Does:** Unconditional handler (not under `BUILD_COMMERCIAL`) with the eight verbs and
  schemas from the plan; `registerAll(UI::AlarmAnnunciator&)` binds a file-static pointer;
  `alarms.setProjectSounds` validates every path and writes `ProjectModel::setSounds` through
  `DataModel::pipelineModules().project`; priority slugs in `EnumLabels.cpp`.
- **Verify:** CV; T28 integration once the app runs; `python scripts/generate-sdk.py --check`
  after sanitize.
- **Deps:** T14
- [x] done

## Phase E — Commands, icons, taskbar

### T16 — Icons and command manifest

- **Files:** `app/rcc/icons/notifications/{16,24,32}/annunciator.svg`, `acknowledge.svg`,
  `silence.svg`, `mute.svg`, `test.svg`; `app/rcc/rcc.qrc`; `app/rcc/commands/app.json`
- **Does:** Five commands `alarms.acknowledge` (Ctrl+Shift+A), `alarms.silence` (Ctrl+Shift+H),
  `alarms.reset` (Ctrl+Shift+R, icon `commands/reset`), `alarms.test` (Ctrl+Shift+T),
  `alarms.mute` (toggle, Ctrl+Shift+U, `iconChecked`), `contexts: ["app","dashboard"]`,
  `category: "tools"`, `shortcutWindows: ["main"]`. Each sequence appears once in the tree
  (ambiguous-shortcut trap).
- **Verify:** `python scripts/registry-verify.py` (icons resolve, ids valid);
  `python scripts/generate-command-strings.py --check` after regeneration.
- **Deps:** none
- [x] done

### T17 — Command bindings

- **Files:** `app/qml/Commands/AppCommandBindings.qml`
- **Does:** Five `cmd…` entries (`run`, `enabled: Cpp_UI_Alarms.highestPriority >= 0` for
  acknowledge/silence/reset, `checked` for mute) added to `map`. **Commercial guard:** no
  `Cpp_Licensing_`/`Cpp_Sessions_`/`Cpp_MQTT_` reference in the file.
- **Verify:** `registry-verify.py` binding coverage; `qmllint` on the file.
- **Deps:** T14, T16
- [x] done

### T18 — `MasterAnnunciator.qml` in the taskbar

- **Files:** `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml`,
  `app/qml/MainWindow/Panes/Dashboard/Taskbar.qml`
- **Does:** Tray pill beside `_thinningBadge`: priority colour, unacknowledged count, fast
  flash (450 ms half-period) while `alerting`, slow flash (1000 ms) while `ringbackPending`,
  steady two-colour state when `Cpp_Misc_GraphicsBackend.reduceMotion`, muted glyph, click =
  acknowledge, right-click `Menu` with Silence / Reset / Test / Mute calling
  `Cpp_UI_Alarms`. Visible when `highestPriority >= 0 || muted || testing`. Animate `opacity`
  only; no width/height animation.
- **Verify:** `qmllint`; maintainer observation AC7.
- **Deps:** T14
- [x] done

## Phase F — Preferences

### T19 — Sounds preferences page

- **Files:** `app/qml/Dialogs/Settings/SettingsSoundsPage.qml`, `app/qml/Dialogs/Settings.qml`
- **Does:** New tab "Sounds" after Export (Notifications stays last, Pro-only); sections
  Output (enable, volume, output device Combo fed by `Cpp_UI_Alarms.outputDevices`, refreshed
  on page open, mute), Alarms (sequence Combo; Warning/Caution/Advisory/Ringback rows: file
  field + browse + play + clear, interval SpinBox for Warning/Caution), Events (enable + file
  + play per event), Test, Reset to bundled. File fields use the `modelValue`/unfocused-sync
  pattern; rejected picks show the reason inline. Add the page to the `implicitHeight` max and
  the dialog's reset block.
- **Verify:** `qmllint`; maintainer observation AC7.
- **Deps:** T14
- [x] done

## Phase G — Project Editor

### T20 — Band Sound column

- **Files:** `app/qml/ProjectEditor/Dialogs/AlarmBandsEditor.qml`,
  `core/Ui/ProjectEditor/EditorForms.cpp`, `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** `bandSound` role, Sound column (browse `*.wav` + clear), `collectBands()` emits
  `sound`; `bandsToVariantList` and `alarmBandsEqual` include it; `parseAlarmBandList` reads
  it `.simplified()`. Multi-selection path keeps working (shared-set equality now includes
  `sound`).
- **Verify:** CV on the two `.cpp`; `qmllint`; maintainer observation AC7 (override plays).
- **Deps:** T1
- [x] done

### T21 — Project-level sequence and channel map editing

- **Files:** `core/Ui/ProjectEditor/ProjectEditor.h`, `core/Ui/ProjectEditor/EditorForms.cpp`,
  `core/Ui/ProjectEditor/EditorCommit.cpp`
- **Does:** Signal `openChannelSoundsEditor(QVariantList)`; slots `openChannelSoundsEditor()`
  (emits the current map as rows), `commitChannelSounds(QVariantList)`,
  `setProjectSoundSequence(QString)`; Q_PROPERTY `projectSoundSequence` (empty = default).
  Both writers go through `ProjectModel::setSounds` (one undo entry each).
- **Verify:** CV; read-back that no direct `m_sounds` write bypasses the setter.
- **Deps:** T2
- [x] done

### T22 — `ChannelSoundsEditor.qml` and `ProjectView` section

- **Files:** `app/qml/ProjectEditor/Dialogs/ChannelSoundsEditor.qml`,
  `app/qml/ProjectEditor/Views/ProjectView.qml`, `app/qml/ProjectEditor/ProjectEditor.qml`
- **Does:** Dialog in the `AlarmBandsEditor` shape: rows channel / Warning / Caution /
  Advisory file pickers, add/remove row, Apply → `commitChannelSounds`; `ProjectView` gains
  an "Alarm Sounds" section with the sequence Combo (Default / A / M / R) and "Edit Channel
  Sounds…"; `ProjectEditor.qml` instantiates the dialog and connects the launcher signal.
  Popup uses the shared `PopupEnter`/`PopupExit` transitions.
- **Verify:** `qmllint`; AC11 via T28.
- **Deps:** T21
- [x] done

## Phase H — Event hooks and UI feedback

### T23 — Historian export publishers

- **Files:** `core/Storage/Sessions/HtmlReport.cpp`, `core/Storage/Sessions/DatabaseManager.cpp`
- **Does:** Publish `AppEventRaised{kAppEventExportFinished}` on `HtmlReport::finished`
  (success only) and on the CSV export busy→idle edge. Commercial TUs; the topic keeps its
  GPL publisher from T11.
- **Verify:** CV; `--bus-census` still balanced.
- **Deps:** T11
- [x] done

### T24 — UI feedback hooks

- **Files:** `app/qml/Widgets/ToolbarButton.qml`, `app/qml/Widgets/IconButton.qml`,
  `app/qml/Widgets/BigButton.qml`, `app/qml/Widgets/TaskbarButton.qml`,
  `app/qml/Widgets/MenuButton.qml`, `app/qml/Widgets/Dashboard/Output/DashboardButton.qml`,
  `app/qml/Widgets/Dashboard/Output/DashboardToggle.qml`
- **Does:** `Cpp_UI_Alarms.playEvent("button")` on `clicked` in the five shared buttons and
  the output Button; `playEvent("toggle")` on the output Toggle. One line each; guarded with
  `typeof Cpp_UI_Alarms !== "undefined"` so extension previews without the context stay
  quiet. Split into two commits if the diff reads badly (shared widgets / output widgets).
- **Verify:** `qmllint`; maintainer observation AC8 (silent by default, audible once enabled).
- **Deps:** T14
- [x] done

## Phase I — Assets and tests

### T25 — C++ unit tests

- **Files:** `app/tests/tst_annunciator_sequence.cpp`, `app/tests/tst_wav_decoder.cpp`,
  `app/tests/CMakeLists.txt`
- **Does:** `ss_add_unit_test` for both. Sequence suite: A/M/R transition tables, acknowledge
  vs silence vs reflash, arbitration order, ringback gating, reset per sequence, interval
  clamp. Decoder suite: synthetic WAVs (every accepted format, mono/stereo, 8 and 96 kHz),
  rejections (non-PCM tag, 11 s file, truncated header, wrong RIFF magic), converter output
  length. Suites recompile the `.cpp` lists per the partition-layer rule in
  `app/tests/CMakeLists.txt`.
- **Verify:** `ctest -R 'tst_annunciator_sequence|tst_wav_decoder'` against the maintainer's
  build.
- **Deps:** T4, T7
- [x] done

### T26 — Bundled sounds and REUSE

- **Files:** `app/rcc/sounds/*.wav` (14), `app/rcc/rcc.qrc`, `REUSE.toml`
- **Does:** Alex renders the four alarm bursts and ringback in Ableton per spec R8 (one burst
  per file, 48 kHz / 16-bit / mono, ≤ 2.5 s) and the nine event sounds (≤ 0.6 s, no pulse-burst
  structure). Until the render lands, a throwaway scratchpad Python synth writes IEC-shaped
  placeholders with the same names so the build and T27 work. Register in `rcc.qrc`; add
  `app/rcc/sounds/**` to the first-party annotation in `REUSE.toml`.
- **Verify:** `reuse lint`; T27.
- **Deps:** none
- [x] done

### T27 — Pure-Python asset test

- **Files:** `tests/unit/test_alarm_sounds.py`
- **Does:** `wave` + `numpy`: every bundled file is 48 kHz / 16-bit / mono; duration caps;
  envelope pulse counts 10 / 3 / 1 / 2 for warning / caution / advisory / ringback; FFT
  fundamental in 150 to 1000 Hz for the four alarm files; event files show no pulse-burst
  structure (≤ 1 envelope onset). Marker `unit`, no app.
- **Verify:** `pytest tests/unit/test_alarm_sounds.py -v` (I can run this).
- **Deps:** T26
- [x] done

### T28 — Integration suite

- **Files:** `tests/integration/test_aural_alerts.py`
- **Does:** AC1 (notification and band paths, sequences A/M/R via `alarms.setProjectSounds`),
  AC2 (arbitration and silence/reflash), AC4 (`burstStartedMs` spacing, Advisory count 1),
  AC5 (rejected `.txt`, missing `.wav` → `problems.list` finding while the point still alerts),
  AC9 (clean state after project load; disconnect during an alert under R), AC11 (channel map
  round-trip through `project.exportJson` and reload, `sounding.file` for the mapped channel).
  Uses `api_client`, `clean_state`, `device_simulator`; delays per `tests/README.md`.
- **Verify:** `pytest tests/integration/test_aural_alerts.py -v` with the app up and the API
  server enabled (`nc -z 127.0.0.1 7777` first).
- **Deps:** T15, T22, T26
- [x] done

## Phase J — Documentation

### T29 — User help

- **Files:** `doc/help/Aural-Alerts.md`, `doc/help/help.json`, `doc/help/Notifications.md`,
  `doc/help/Widget-Reference.md`
- **Does:** New page (section Configuration): the three priorities, ISA-18.1 sequences and
  the four actions, the master annunciator, the Sounds page, bundled sound shapes (IEC
  60601-1-8), project overrides (`sound` on a band, `sounds` channel map, relative paths),
  API verbs, device fallback; command icons via `cmd:` ids. Cross-links from Notifications
  and the alarm-bands section (new `sound` key). Never says "requires a license".
- **Verify:** `python scripts/documentation-verify.py`; `ss-docs` two-tier check.
- **Deps:** T15, T19, T22
- [x] done

### T30 — AI-facing docs

- **Files:** `doc/claude/architecture/dashboard.md`, `doc/claude/directory-map.md`, `CLAUDE.md`
- **Does:** Alarm-bands section gains the annunciator paragraph (transition signal, bus
  topic, root-owned facade, callback contract); directory map lists `core/Devices/IO/Audio/`
  and `core/Ui/UI/Alarms/`; one line in Project Overview naming the aural alert system.
- **Verify:** `python scripts/claim-verify.py`; `python scripts/documentation-verify.py`.
- **Deps:** T13, T14
- [x] done

## Review record (2026-09-27)

`qt-cpp-review` ran on the diff. Confirmed findings, all fixed on disk: audio callback
use-after-free (lanes re-acquire the slot per callback; the bank reads the live generation),
lane-flag ordering, regular-file bounded WAV read, `..` rejection and the API path policy on
`alarms.setProjectSounds`, Test-step priority off-by-one, interval Q_PROPERTYs, slot rename
`openChannelSoundsEditorForProject`, band points re-seeded on a tracker rebuild, annunciator
built only by a wiring root (benchmark never builds it) and read by the API handler through
the owning slot, QPointer-guarded Problem Center checker, no-device finding, the band
monitor's own `Alarms` channel excluded from notification points, backed-off device recovery
with a default-device retry, negative-cached override loads with issue clearing, sounding slot
re-resolved after a bank rebuild, dead accessors removed, QML guards and duplicate import
dropped, miniaudio define set applied to Ui and the executable, std::clamp/std::min in new
code. Left as noted: Caution/Advisory pill colours are literals (no theme key exists);
interval bounds are duplicated as SpinBox literals (the C++ clamp wins). Sanitize ran last.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC6, AC7, AC8,
      AC12 by maintainer observation; AC6's measured onset figure recorded in `plan.md`).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `python scripts/code-verify.py --singleton-census --check`, `--bus-census`,
      `--tu-census --check`, `python scripts/layer-verify.py`, `python scripts/registry-verify.py`
      all clean.
- [ ] `qt-cpp-review` run on the C++ diff; the audio-callback thread-safety finding set
      addressed or noted.
- [ ] `--benchmark-hotpath` not regressed (AC10; no hotpath TU touched, run once at the end).
- [ ] `ctest -R 'tst_annunciator_sequence|tst_wav_decoder'`, `pytest tests/unit/test_alarm_sounds.py`,
      `pytest tests/integration/test_aural_alerts.py` identified for the maintainer; the first
      two run by me against the existing build.
- [ ] `python scripts/sanitize-commit.py` run; working tree clean of lint debt; `reuse lint` clean.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (`examples/README.md`, `examples/examples.json`, `examples/Graphical Ping/` are the
      maintainer's in-progress work and stay untouched).
- [ ] `spec.md` status set to `done`.
