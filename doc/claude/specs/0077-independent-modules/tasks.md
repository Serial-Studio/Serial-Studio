---
spec: 0077-independent-modules
phase: tasks
status: approved
updated: 2026-09-08
---

# Tasks 0077 — Independent core modules

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md) into units that
> are small, ordered and individually verifiable. `/ss-implement` works this list top to bottom
> and keeps the boxes current. Gate: do not start `/ss-implement` until a human marks this
> `approved`.

## Conventions

- One task = one reviewable diff. Scripted sweeps (an include rewrite over 127 files, the
  157 `showMessageBox` sites) are one task each because the diff is one mechanical rule; the
  task names the rule and the count the reviewer should see.
- **Verify** always includes `python scripts/code-verify.py --check <files>` and
  `python scripts/layer-verify.py --json` (the per-edge counts must never grow; the expected
  drop is stated where it matters). Both are implied below and stated only when something more
  is needed.
- **Checkpoint tasks (`C0`–`C5`)** are the maintainer's: configure, build, `ctest`, launch,
  and the phase's observations. The agent pastes every gate output into this file under the
  checkpoint before handing over, and does not start the next phase until the checkpoint is
  ticked.
- A task on a silent-breakage surface names the binding invariant in its **Does** line.
- Single commit at the end (spec R12); no task commits.

## Phase 0: bus injection and the vocabulary drop

### T1: MessageBus hardening and the vocabulary checklist

- **Files:** `core/Core/Bus/MessageBus.h`, `core/Core/Bus/MessageBus.cpp`, `core/Core/Bus/Messages.h`
- **Does:** Store subscriber handlers as `shared_ptr<const ErasedHandler>` so `dispatch()`'s vector copy is refcount bumps; write the growth checklist into `Messages.h`'s `@file` block (Core value types only, no enum owned above Core, field change = wire break, added only via a spec, request topics never block). `instance()`/`setInstance()` stay until T73.
- **Verify:** `tst_message_bus` registration unchanged; `python scripts/code-verify.py --check core/Core/Bus/*`.
- **Deps:** none
- [x] done
  Handler storage was already `shared_ptr<const ErasedHandler>` (0076 review); only the checklist landed.

### T2: SessionContext slot 0 owns the bus

- **Files:** `app/src/SessionContext.h`, `app/src/SessionContext.cpp`
- **Does:** Add `std::unique_ptr<Core::Bus::MessageBus> m_bus` adopted first and released last in `shutdown()`; `create<T>(Core::Bus::MessageBus&)` passes it to the module ctor. Invariant: ctor/dtor of `SessionContext` stay empty; INV-4 (adopted addresses never change) and INV-5 hold; `shutdown()` order is the exact reverse of instantiation.
- **Verify:** read-back of `shutdown()` against `instantiateCoreModules()`; `code-verify --singleton-census --check` unchanged.
- **Deps:** T1
- [x] done

### T3: Composition root builds the bus first

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/src/Misc/ModuleManager.h`
- **Does:** `instantiateCoreModules()` constructs and adopts the bus as its first statement (before `Translator`), calls `MessageBus::setInstance()` for the transitional accessor, and passes `bus` into every `setupExternalConnections(...)` of the Meyers-singleton modules that will publish or subscribe (players, exports, `Translator`, `WidgetExtensions`, `ProblemCenter`, `ConnectionDiagnostics`, `MQTT::Publisher`, `InfluxDB::Export`). Invariant: pinned order otherwise unchanged; `composition-root-order` doc anchor re-seeded in T78.
- **Verify:** `python scripts/claim-verify.py` (anchor mismatch expected until T78, recorded); read-back of the order.
- **Deps:** T2
- [x] done
  Attach helper: `ModuleManager::attachMessageBusToModules()` runs after `instantiateCoreModules()` in both the GUI and the headless roots.

### T4: Meyers-singleton modules accept the bus

- **Files:** `core/Storage/CSV/Export.{h,cpp}`, `core/Storage/MDF4/Export.{h,cpp}`, `core/Storage/Sessions/Export.{h,cpp}`, `core/Storage/InfluxDB/Export.{h,cpp}`, `core/Storage/CSV/Player.{h,cpp}`, `core/Storage/MDF4/Player.{h,cpp}`, `core/Storage/Sessions/Player.{h,cpp}`, `core/Ui/Console/Export.{h,cpp}`, `core/Ui/Misc/Translator.{h,cpp}`, `core/Ui/UI/WidgetExtensions.{h,cpp}`, `core/Ui/Misc/ProblemCenter.{h,cpp}`, `core/Ui/Misc/ConnectionDiagnostics.{h,cpp}`, `core/Devices/MQTT/Publisher.{h,cpp}`
- **Does:** Each gains `Core::Bus::MessageBus* m_bus` (null until wired) set by its `setupExternalConnections(MessageBus&)`; no publish or subscribe yet. Invariant: every publish site added later asserts `m_bus` non-null; all wiring precedes `restoreLastProject()`, the first publish.
- **Verify:** `code-verify --check` on each pair; `--singleton-census --check` unchanged.
- **Deps:** T3
- [x] done
  Mechanism: `attachMessageBus(Core::Bus::MessageBus&)` setter + `m_bus` (null until the root attaches).

### T5: The nine adopted modules take the bus by constructor

- **Files:** `app/src/AppState.{h,cpp}`, `core/Ui/UI/Dashboard.{h,cpp}`, `core/Ui/Console/Handler.{h,cpp}`, `core/Pipeline/DataModel/Scripting/FrameParser.{h,cpp}`, `core/Pipeline/DataModel/FrameBuilder.{h,cpp}`, `core/Pipeline/DataModel/ProjectModel.{h,cpp}`, `core/Pipeline/IO/PipelineHost.{h,cpp}`, `core/Devices/IO/ConnectionManager.{h,cpp}`, `core/Pipeline/DataModel/NotificationCenter.{h,cpp}`
- **Does:** Private ctor takes `Core::Bus::MessageBus&`, stored as `m_bus`; `friend class ::SessionContext` stays. Invariant (ctor closure, spec 0001/0039): `ProjectModel`'s ctor closure (`newJsonFile`, `watchProjectFile`, `scheduleAutoSave`, `ControlScript::setCode`) gains no reach; the bus is fully constructed before any module; the ctor-edge proof is re-run and recorded at C0.
- **Verify:** ctor-edge proof method from `doc/claude/specs/0001-composition-root/`; `code-verify --check`.
- **Deps:** T2
- [x] done
  `bus-on-hotpath` refined to traffic (bus header include, `MessageBus::instance`, publish/subscribe/latest calls); a forward declaration plus `MessageBus&` member in a hotpath TU is allowed by design.

### T6: `SerialStudio` vocabulary becomes a Core namespace

- **Files:** `app/src/SerialStudio.h` → `core/Core/SerialStudio.h`, new `core/Core/SerialStudio.cpp`, `core/Core/CMakeLists.txt`
- **Does:** `namespace SerialStudio { Q_NAMESPACE ... }` with every enum verbatim as `Q_ENUM_NS` (ordinals, `BUILD_COMMERCIAL` blocks, `BarPanel=90`, `Extension=100` untouched), `XAxisMode`, `XAxisPolicy`, `kWidgetApiVersion*`, `WidgetMap` typedef, and the Qt-Core-only helper declarations listed in plan P0; `.cpp` holds their definitions moved verbatim from `app/src/SerialStudio.cpp`, with `tr()` → `QCoreApplication::translate("SerialStudio", …)`. `fast_float.h` include waits for T12 (temporary relative include documented in the task log). Core stays `Qt6::Core` only.
- **Verify:** `code-verify --check core/Core/SerialStudio.*`; new `app/tests/tst_serialstudio_enums.cpp` pins `FFTWindow`/`DashboardWidget`/`BusType` ordinals (registered in T13).
- **Deps:** none
- [x] done

### T7: Frame-typed helpers into Pipeline

- **Files:** `app/src/SerialStudioFrameSupport.cpp` → `core/Pipeline/DataModel/FrameSupport.{h,cpp}`, new `core/Pipeline/DataModel/TextCodec.{h,cpp}`, `core/Pipeline/CMakeLists.txt`
- **Does:** `commercialCfg` ×2, `groupXAxisMode`, `resolveXAxisPolicy` → `FrameSupport`; `encodeText`/`decodeText` (Core5Compat) → `TextCodec`. Bodies verbatim; `Frame.cpp` includes the new headers.
- **Verify:** `code-verify --check`; `tst_frame_support` registration repointed (T13).
- **Deps:** T6
- [x] done
  `FrameSupport` also hosts `groupEligibleForWorkspace`; `WidgetResolution.{h,cpp}` (the P2 resolver file) was created now so the Pipeline model files never include the Ui helper.

### T8: Ui helper class and QML registration

- **Files:** `app/src/SerialStudio.cpp` → `core/Ui/UI/SerialStudioHelpers.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** `UI::SerialStudioHelpers : QObject` holds the Ui-dependent statics (`getDashboardWidget(s)`, `extensionGroupWidgetCount`, `dashboardWidgetIcon`, colour getters, `isAnyPlayerOpen`, `dashboardWidgetTitle`, `textEncodings`); root registers `qmlRegisterUncreatableMetaObject(SerialStudio::staticMetaObject, "SerialStudio", 1, 0, "SerialStudio", …)` and `qmlRegisterSingletonInstance("SerialStudio", 1, 0, "SerialStudioHelpers", …)`. `app/src/SerialStudio.{h,cpp}` cease to exist.
- **Verify:** `code-verify --check`; `--selftest qml` at C0.
- **Deps:** T6, T7
- [x] done
  QML-facing colours, icon and player predicates only; the Storage-owned `isAnyPlayerOpen` lives in `core/Storage/Replay/PlayerState.h` (same namespace) so Storage and Ui callers keep their spelling.

### T9: QML call sites

- **Files:** the 14 QML files listed in plan P0 (26 sites)
- **Does:** `SerialStudio.fn(` → `SerialStudioHelpers.fn(` for the nine helper functions; the 270 enum reads untouched.
- **Verify:** `grep -rn 'SerialStudio\.\(searchMatches\|isDashboardTool\|dashboardWidgetIcon\|getDevice\|dashboardWidgetPaintsTitle\|isAnyPlayerOpen\|getDatasetAccentColor\)' app/qml` returns nothing; `qmllint` target at C0.
- **Deps:** T8
- [x] done

### T10: Include sweep to `Core/SerialStudio.h`

- **Files:** 127 files under `core/` (56 Pipeline, 16 Devices, 5 Storage, 20 Api, 30 Ui), `app/src/**` includers, `app/tests/session_context_stub.cpp`
- **Does:** `#include "SerialStudio.h"` → `#include "Core/SerialStudio.h"`; Ui-only helper callers include `UI/SerialStudioHelpers.h`; Frame-typed callers include `DataModel/FrameSupport.h`; delete the `SerialStudio::staticMetaObject` stub line. `activated()` callers are handled in T14.
- **Verify:** `layer-verify --json` shows the five `*->App` edges each drop by their `SerialStudio.h` count (56/17/16/20/33); `grep -rn '"SerialStudio.h"' app core` empty.
- **Deps:** T8
- [x] done
  `FrameBuilder` and `ControlScript` compute the player flag from the player references they already held (reach-neutral 3-slot mask / bool); indented `#  include` lines handled; two transitive `<QColor>` and one `DataModel/Frame.h` include made explicit.
  The player watch lives in the new `FrameBuilder/ExternalWiring` sub-object (the plan's T34 file, created early so the hotpath TU shrinks and never sees the bus); its three `Player::instance()` reaches are the ones FrameBuilder.cpp held and are deleted by T37.

### T11: Generators and verifiers follow the move

- **Files:** `scripts/generate-property-registry.py`, `scripts/registry-verify.py`, the six generated artifacts (`core/Pipeline/DataModel/Generated/{DatasetRegistry.h,DatasetSerialization.cpp,DatasetForm.cpp}`, `core/Api/API/Generated/DatasetApiFields.cpp`, `app/rcc/api/proto-fields.json`, `doc/grpc/serialstudio-typed.proto`)
- **Does:** Include literal → `Core/SerialStudio.h`; regenerate; `registry-verify.py` path constants → `core/Core/SerialStudio.h`, `core/Ui/UI/SerialStudioHelpers.cpp`.
- **Verify:** `python scripts/generate-property-registry.py --check`; `python scripts/registry-verify.py` CLEAN; regenerated diff is include lines only.
- **Deps:** T10
- [x] done

### T12: Header-only third-party files into Core

- **Files:** `app/src/ThirdParty/{readerwriterqueue.h,atomicops.h,readerwritercircularbuffer.h,fast_float.h}` → `core/Core/ThirdParty/`, `REUSE.toml`, `app/CMakeLists.txt` (HEADERS entries), includers (`FrameBuilder.h`, `FrameConsumer.h`, `FrameReader.h`, `PipelineHost.h`, `StreamWorker.h`, `LatestFrameTap.h`, `TableSnapshotChannel.h`, `FrameParser.h`, `Audio.h`, `GRPCServer.h`, `core/Core/SerialStudio.h`)
- **Does:** `git mv`; includes → `Core/ThirdParty/…`; REUSE path annotations updated.
- **Verify:** `reuse lint`; `layer-verify --json` (Pipeline→App −8, Devices→App −1).
- **Deps:** T6
- [x] done

### T13: Test registrations for the moved statics

- **Files:** `app/tests/CMakeLists.txt`, new `app/tests/tst_serialstudio_enums.cpp`
- **Does:** The 17 registrations that compiled `SerialStudioFrameSupport.cpp` now compile `core/Pipeline/DataModel/FrameSupport.cpp` (+ `TextCodec.cpp` where `encodeText` is exercised) and link `SerialStudio::Core` for the namespace; register `tst_serialstudio_enums`.
- **Verify:** `layer-verify` `cmake-missing` clean; C0 `ctest`.
- **Deps:** T7, T10
- [x] done
  `tst_license_state` registered alongside.

### T14: `Core::License` flag

- **Files:** new `core/Core/License.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`, `app/src/Licensing/LemonSqueezy.cpp`, every `SerialStudio::activated()` / `featureTier()` reader under `core/` (Pipeline 5, Storage 1, Api 1, Ui 6, MQTT driver `MQTT.cpp:235,1250`, `MQTT/Publisher.cpp:1408`)
- **Does:** `std::atomic<bool> activated`, `std::atomic<quint8> tier`, `std::atomic<bool> trialExpired`; inline readers; setters called by the root at licensing-block construction and on `activatedChanged`. Readers switch to `Core::License::activated()`. Invariant (cached hotpath flag): the MQTT per-message and per-block sites read the atomic, never `CommercialToken::isValid()` (HMAC) and never the bus; the root sets the flag before `restoreLastProject()` (spec 0042 ordering).
- **Verify:** `grep -rn 'SerialStudio::activated' core` empty; new `tst_license_state` (set/read, default false).
- **Deps:** T10
- [x] done
  Root hook lives in `instantiateCoreModules()` on the existing licensing reaches (`publishLicenseState(trial)` once, then on `activatedChanged`); `GuardSelfTest` keeps a local `gatesOpen()`.

### T15: `AppInfo.h`, crypto helpers, enum tables into Core

- **Files:** `app/src/AppInfo.h` → `core/Core/AppInfo.h` (+9 includers); `app/src/Licensing/SimpleCrypt.{h,cpp}` → `core/Core/Crypto/SimpleCrypt.{h,cpp}`; new `core/Core/Crypto/MachineKey.{h,cpp}` (`machineSpecificKey()` body from `MachineID.cpp`, which now calls it); `core/Devices/MQTT/CredentialVault.{h,cpp}` → `core/Core/Crypto/`; `core/Api/API/EnumLabels.{h,cpp}` → `core/Core/EnumLabels.{h,cpp}`; `core/Ui/UI/LayoutPatterns.{h,cpp}` → `core/Core/LayoutPatterns.{h,cpp}`; includers (`MirrorSession`, `KeyVault`, `InfluxDB/Export.h`, `MqttHandler`, `OpcUa.cpp`, `DiagnosticsHandler`, `EnumLabels` callers); CMake lists; `app/tests/CMakeLists.txt` (5 `SimpleCrypt.cpp` + 4 `MachineID.cpp` registrations link Core)
- **Does:** Verbatim moves; `EnumLabels` keeps identical slugs.
- **Verify:** `layer-verify --json` (Devices→Api −1, App edges −~15); `reuse lint`.
- **Deps:** T10
- [x] done
- **Deviation:** `MachineKey.{h,cpp}` not created and `CredentialVault` left in `core/Devices/MQTT/`: `MachineID::machineSpecificKey()` is a bare accessor whose value only exists inside `readInformation()` (the commercial-licensed fingerprint: platform-tool readers, the `licensing/lastGoodRawId` store, the Blake2s hash), so there is no licensing-neutral body to lift and Core cannot call back into `app/src`. `tst_enum_labels_commercial` keeps compiling `Core/EnumLabels.cpp` + `Core/LayoutPatterns.cpp` directly (no Core link, `core/` include dir) so its target-scoped `BUILD_COMMERCIAL` still reaches the labels TU.

### T16: miniaudio into Devices

- **Files:** `app/src/ThirdParty/miniaudio.{h,cpp}` → `core/Devices/ThirdParty/`, `app/CMakeLists.txt` (`ss_apply_miniaudio_definitions` function + the `.cpp` warning flags + `asound` link move), `core/Devices/CMakeLists.txt`, `core/Pipeline/CMakeLists.txt` (call kept until T35 removes `Audio.h` from Pipeline)
- **Does:** The function moves with the file; every TU that sees the header still gets the same `MA_*` set (`sizeof(ma_device)` invariant).
- **Verify:** `layer-verify --json` (Devices→App −3); C0 commercial build.
- **Deps:** none
- [x] done

### T17: Messages.h vocabulary (Core-expressible topics)

- **Files:** `core/Core/Bus/Messages.h`, `app/tests/tst_message_bus.cpp`
- **Does:** Extend per the plan table: `ConnectionStateChanged` gains `connecting`, `busType`; `LicenseStateChanged` gains `tier`, `trialExpired`; `NotificationRaised` gains `channel`, `key`; add `OperationModeChanged`, `DeviceCatalogChanged`, `ReplayPlayerStateChanged`, `AudioCaptureFormat`, `WidgetExtensionCatalog`, `DashboardViewState`, `NotificationClearRequested`, `DashboardViewStateRestoreRequested`, `DashboardViewStateClearRequested`, `DisconnectRequested`, `DeviceOpenAttempted`, `ModbusRegisterGroupsLoaded`, `LoadGeneratedProjectRequested`, `GeneratedProjectLoadFinished`, `Source0ConnectionSettingsChanged`. `FrameConfigChanged` and `ProjectStructureSnapshot` wait for T42/T46 (their field types reach Core in P3). Test covers brace-init of each aggregate.
- **Verify:** `tst_message_bus` compiles the new topics (C0).
- **Deps:** T1, T6
- [x] done
  `WidgetExtensionEntry` added as the catalog element; empty-payload requests carry an `int reserved` so braced init stays uniform.

### C0: Phase 0 checkpoint (maintainer)

- **Does:** Configure (unit-ci line from 0076 `handoff.md`), `--target ss_unit_tests`, `ctest`, application build (GPL and Pro), launch, `--selftest`, `qmllint` target. Ctor-edge proof result pasted here. Gate log: `layer-verify --json`, `code-verify --check`, all censuses `--check`, `claim-verify`, `registry-verify`, generator `--check`s, `reuse lint`.
- **Expected:** `*->App` edges: Pipeline ≤ 22, Ui ≤ 30, Devices ≤ 18, Api ≤ 15, Storage ≤ 19.
- **Deps:** T1–T17
- **Agent gate log (2026-09-08, before the maintainer build):**
  `layer-verify` 0 errors, baseline re-seeded to the new (lower) counts: Pipeline->App 22, Ui->App 26,
  Devices->App 21 (AppState 12, licensing 5, gRPC 2, SessionContext 1, misc), Api->App 14, Storage->App 16,
  Pipeline->Ui 62, Devices->Ui 23, Storage->Ui 16, Api->Ui 21, Pipeline->Devices 32, Storage->Devices 8,
  Devices->Api 2; every other edge unchanged. Total upward includes 594 -> 348.
  `code-verify --check` whole tree: 0 errors (advisories are the pre-existing long TUs).
  Singleton census re-seeded twice on pure shrink (1554 -> 1549); TU census re-seeded on shrink
  (excess 2794 -> 2793); dup census flat; `claim-verify` 0/0; `registry-verify` CLEAN (mirror digest
  re-seeded: include line only); `generate-property-registry --check` up to date;
  `documentation-verify` 0 findings; `pytest scripts/tests` 211 passed; `pytest tests/scripts` 311 passed
  (`test_license_guard_present_in_serial_studio_activated` repointed to the root's `publishLicenseState`).
  Ctor-edge proof (spec 0001 method, re-run by reading the diff of the nine constructors): the only
  new constructor input is the bus, built as the first statement of `instantiateCoreModules()` with no
  dependency of its own; no constructor body gained a reach (each diff is the signature plus one
  init-list line), so the pinned order and ProjectModel's protected closure are unchanged.
  Deviations from the plan, all recorded on their tasks: T4 setter mechanism, T5 lint refinement,
  T10 reach-neutral player mask + `FrameBuilder/ExternalWiring` created early, T15 `MachineKey` is
  root-published (plan amended). Devices->App landed 3 above the "expected" line because the
  `AppState.h` includes (12) belong to P2, not P0.
- **C0 defects found by the maintainer's build (2026-09-08):** (1) `DocSearch.cpp` and 76 other
  files leaned on the old header's transitive `Frame.h`/`QColor` closure; explicit includes added,
  the redundant ones (types `Core/SerialStudio.h` provides) removed again. (2) Startup stack
  overflow (`0xC00000FD`, three consecutive crashes at the `module-manager-bootstrap` checkpoint):
  the `toDouble(QVariant)` overload, now a namespace inline instead of a class member, could not
  see the later `QJsonValue` overload and recursed into itself through Qt's implicit `QVariant`
  construction. Fix: every overload is forward-declared before the first inline body, and
  `tst_serialstudio_enums` pins the JSON-in-variant path. Lesson recorded for P1+: a class turned
  into a namespace loses the complete-class lookup; declare before defining.
- **Maintainer triage hints beyond 0076's three shapes:** (a) a symbol that used to arrive through
  `SerialStudio.h` (`QColor`, `DataModel::Group`, `QMap`) is now missing: add the direct include;
  (b) an enum used in QML as `SerialStudio.X` but read through a `SerialStudio` *function* call that
  is not in the helper list: report it, the helper class is the place; (c) `Q_ENUM_NS` needs the moc
  run on `core/Core/SerialStudio.h`, so a stale build dir must be reconfigured; (d) `Core::License`
  is set only under `BUILD_COMMERCIAL`, so a GPL build reads false everywhere, as `activated()` did.
- [x] done
  Maintainer build (Release, MSVC 2022, Qt 6.11.0) compiles and the application launches after the two C0 defects above; `ctest` on the unit tier still to be run by the maintainer.

## Phase 1: utilities down, editor UI up, prompt seam, notifications

### T18: Prompt seam in Core

- **Files:** new `core/Core/Prompt/IUserPrompter.h`, `core/Core/Prompt/UserPrompt.{h,cpp}`, `core/Ui/Misc/Utilities.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`
- **Does:** `Core::Prompt::Icon`/`Button` enums mirroring `QMessageBox` values; `showMessageBox(text, informative, icon, title, buttons, default) -> int`, `selectDirectory`, `revealFile` forwarding to a root-bound `IUserPrompter*` (null → `qWarning` + default button). `Misc::Utilities` implements the interface by delegating to its existing functions, keeping the off-thread `invokeMethod` path (`Utilities.cpp:180-188`). Root binds it in the new `bindInterfaces()` step.
- **Verify:** new `tst_user_prompt_seam` (unbound default, bound fake records calls); `code-verify --check`.
- **Deps:** T3
- [x] done
  Interface methods are `promptMessage`/`promptDirectory`/`revealInFileManager` (the Ui class keeps its static `showMessageBox`/`revealFile`); the directory picker is asynchronous with a callback, matching the existing non-modal dialog. Bound in `instantiateCoreModules()`; the extra root reach is offset by capturing `PipelineHost` once in the wiring pass.

### T19: `showMessageBox` sweep below Ui

- **Files:** 18 Pipeline, 10 Devices, 6 Storage, 2 Api `.cpp` files (157 call sites, 2 `revealFile`)
- **Does:** `Misc::Utilities::showMessageBox(…, QMessageBox::X, …)` → `Core::Prompt::showMessageBox(…, Core::Prompt::X, …)` by script; include → `Core/Prompt/UserPrompt.h`.
- **Verify:** `grep -rn 'Misc::Utilities\|Misc/Utilities.h' core/Pipeline core/Devices core/Storage core/Api` empty (the two `coloredSvgIcon` callers move in T27); `layer-verify --json` (Pipeline→Ui −29, Devices→Ui −18, Storage→Ui −6, Api→Ui −2).
- **Deps:** T18
- [x] done
  211 sites in 38 files (enum spellings included); the two Widgets dialogs (`TransmitTestDialog`, `PainterCodeEditor`) keep their direct `QMessageBox` calls and move to Ui with the editor block.

### T20: `TimerEvents` and `IconRegistry` into Core

- **Files:** `core/Ui/Misc/TimerEvents.{h,cpp}` → `core/Core/TimerEvents.{h,cpp}`; `core/Ui/Misc/IconRegistry.{h,cpp}` → `core/Core/IconRegistry.{h,cpp}`; includers; CMake lists
- **Does:** Verbatim (`Misc::` namespace kept). Invariant: `TimerEvents` stays the ONE display-tick source; `FrameBuilder`'s `uiTimeout` hooks keep their explicit `Qt::DirectConnection` for `bumpFlushEpoch` and the queued refresh for the pipeline-affine slots.
- **Verify:** `layer-verify --json` (Pipeline→Ui −12, Devices→Ui −4, Storage→Ui −2, Api→Ui −1).
- **Deps:** none
- [x] done

### T21: `Translator` and `LanguageTable` into Core

- **Files:** `core/Ui/Misc/Translator.{h,cpp}`, `core/Ui/Misc/LanguageTable.{h,cpp}` → `core/Core/`; new `core/Ui/Console/WelcomeText.{h,cpp}`; `core/Ui/Console/Handler.{h,cpp}` (gains `welcomeConsoleText` Q_PROPERTY); `app/src/Misc/ModuleManager.cpp` (`languageChanged` → `qApp->setLayoutDirection`); one QML binding
- **Does:** `setLayoutDirection` leaves `setLanguage`; `welcomeConsoleText` (reads `Core::License::tier()`) moves to Ui; the `.ts` context name is unchanged.
- **Verify:** `layer-verify --json` (Pipeline→Ui −16, Devices→Ui −7); RTL switch in the C1 walk.
- **Deps:** T14, T20
- [x] done. Deviation: no QML binding read `welcomeConsoleText`; the one consumer was `Widgets::Terminal` (C++), now `Console::welcomeConsoleText(language)`, a free function taking the language so the census stays flat; `Console::Handler` gained the Q_PROPERTY anyway, capturing the Translator it already reaches in `setupExternalConnections()`. `tst_language_table` links `SerialStudio::Core` and drops `SS_LICENSING_SRC`/`Qt6::Widgets`.

### T22: `WorkspaceManager` into Core

- **Files:** `core/Ui/Misc/WorkspaceManager.{h,cpp}` → `core/Core/WorkspaceManager.{h,cpp}`; includers (`ExportStructure`, `ProjectTables`, `ProjectModel`, `ProjectLoader`, players, `DatabaseManager`, `SessionExporter`)
- **Does:** `selectPath()` re-implemented over `Core::Prompt::selectDirectory` so the QML slot survives; everything else verbatim.
- **Verify:** `layer-verify --json` (Pipeline→Ui −4, Storage→Ui −8).
- **Deps:** T18
- [x] done
  `selectPath()` now runs through `Core::Prompt::selectDirectory` (asynchronous callback, same queued `setPath` semantics); three test registrations dropped the recompiled source and already linked Core.

### T23: Small Core-only utilities

- **Files:** `core/Ui/Misc/JsonValidator.h`, `core/Ui/Misc/PasswordHash.{h,cpp}` → `core/Core/`; includers (`ProjectLoader`, `ModbusRegisterMap`, `ProjectModel`, `DatabaseManager`)
- **Does:** Verbatim moves. (`MemoryStore` stays in Ui: its only lower-library reader is the assistant handler, which moves to Ui in T60.)
- **Verify:** `layer-verify --json` (Pipeline→Ui −2, Storage→Ui −1).
- **Deps:** none
- [x] done
- Moved `JsonValidator.h` only (12 includers re-pointed, Pipeline→Ui −2). `PasswordHash.cpp` stays in Ui: `QPasswordDigestor` is Qt Network, and Core links Qt6::Core only; needs either a Qt6::Network link on Core or a PBKDF2-HMAC-SHA256 rewrite over `QMessageAuthenticationCode` before it can move (Storage→Ui −1 outstanding).
  Lead follow-up: `PasswordHash` moved to `core/Pipeline/Misc/` instead (QPasswordDigestor is Qt Network, which Core never links); same relative include path, so no includer changed and both Pipeline and Storage callers resolve it below Ui.

### T24: Dead-include sweep

- **Files:** `Misc/Utilities.h` in 9 Devices drivers; `Misc/Translator.h` in 7 `ProjectEditor*.cpp`; `Misc/WorkspaceManager.h` in `CSV/Export.cpp`, `MDF4/Export.cpp`, `Sessions/Export.cpp`; `UI/Dashboard.h` in `FrameBuilder.cpp`; `Misc/IconEngine.h` in 7 of its includers; `IO/ConnectionManager.h` in 4 `ProjectEditor*.cpp`, `CSV/Export.cpp`, `MDF4/Export.cpp`; `MQTT/Publisher.h` + `PublisherScriptEditor.h` in 6 `ProjectEditor*.cpp`; `IO/HAL_Driver.h` in `Sessions/Verifier.cpp`
- **Does:** Delete includes with zero symbol use (each confirmed by grep before removal).
- **Verify:** `layer-verify --json` (Pipeline→Devices −16, Storage→Devices −3, Pipeline→Ui −~8).
- **Deps:** none
- [x] done
- Removed 54 includes: Pipeline→Ui −22 (Utilities 7, Translator 7, IconEngine 7, UI/Dashboard 1), Pipeline→Devices −18 (MQTT Publisher + PublisherScriptEditor 14, ConnectionManager 4), Devices→Ui −9 (Utilities), Storage→Ui −3 (WorkspaceManager), Storage→Devices −2 (ConnectionManager in CSV/MDF4 Export). Kept: `IO/HAL_Driver.h` in `Sessions/Verifier.cpp` (`IO::CapturedData` / `makeCapturedData` live there).

### T25: Editor block moves to Ui

- **Files:** `core/Pipeline/DataModel/ProjectEditor.{h,cpp}`, `core/Pipeline/DataModel/Project/ProjectEditor*.cpp`, `ProjectEditorIcons.h`, `ProjectEditorItemIds.h`, `ProjectNavHistory.{h,cpp}`, `Project/PropertyHooks.{h,cpp}`, `DataModel/Editors/*` (except `CodeFormatter`, `ScriptTemplateCatalog`, `EditorFormatting`), `Generated/DatasetForm.cpp`, `Editors/FrameParserModel.*`, `Importers/ImporterCommon.h` (icon-preview part split to `core/Ui/ProjectEditor/ImporterPreview.h`), the two `coloredSvgIcon` callers → `core/Ui/ProjectEditor/`; `core/Pipeline/CMakeLists.txt`, `core/Ui/CMakeLists.txt`
- **Does:** `git mv` as a block; includes re-pointed; `qmlRegisterType<DataModel::ProjectEditor>` unchanged.
- **Verify:** `layer-verify` `pair-split`/`moc-double-listed`/`core-unowned` clean; `layer-verify --json` (Pipeline→Ui: `IconEngine`, `IconRegistry`, `CommonFonts`, `ThemeManager`, `WidgetExtensions` entries gone).
- **Deps:** T20, T24
- [x] done
- Moved as a block to `core/Ui/ProjectEditor/` (facade + eight per-concern TUs, Icons/ItemIds/NavHistory, Editors/ minus CodeFormatter/EditorFormatting/ScriptTemplateCatalog, Dialogs/TransmitTestDialog, Generated/DatasetForm.cpp). Two splits the move required: `workspaceWidgetKey`/`buildResolvedWidgetLookup` (+ `ResolvedWidget`) left the editor for the new Pipeline pair `DataModel/Project/WorkspaceKeys.{h,cpp}` so `WorkspacesHandler` (Api) no longer includes the editor; `PropertyHooks.{h,cpp}` stayed in Pipeline (the registry, `DatasetApiFields.cpp` and `PropertyValidators.cpp` use its option types and validators; its only Ui reach is `WidgetExtensions`, an edge `WidgetResolution.cpp` already carries). `ImporterCommon.h` stayed whole: it has no Ui include (`Core/IconRegistry.h` is Core) and both importers are Pipeline. Pipeline->Ui 26 -> 15; Api->Ui flat at 19. Six model TUs now carry the editor reach upward (`ProjectBulkOps` Kind enums, `RowFilterProxy` SectionHeader, `ProjectFolders`/`ProjectTables`/`ProjectWorkspaces` `ProjectEditor::instance()`, `ProjectOutputWidgets` `OutputCodeEditor::defaultTemplate()`): left for T29/the topic work.
- Follow-up: six model->editor reaches inverted (EntityKinds.h, ProjectModel editor-request signals, output template in ScriptTemplateCatalog)

### T26: Editor re-form: sub-object headers

- **Files:** new `core/Ui/ProjectEditor/{EditorTree,EditorForms,EditorSelection,EditorCommit,EditorMqtt,EditorMultiSelect,EditorSummaries,EditorWiring}.h`, `core/Ui/ProjectEditor/ProjectEditor.h`
- **Does:** One class per former TU, owned by value by the facade, each constructed with `ProjectEditor&` and the model reference it uses; the facade keeps every `Q_PROPERTY`/`Q_INVOKABLE`/signal so QML is untouched. Header-block order per code-style.
- **Verify:** `code-verify --check` on the nine headers.
- **Deps:** T25
- [x] done
- Eight classes in `core/Ui/ProjectEditor/` (.h/.cpp lines): EditorWiring 70/581, EditorSelection 90/717, EditorTree 108/1245, EditorForms 93/1370, EditorCommit 94/1054, EditorSummaries 92/1018, EditorMultiSelect 72/587, EditorMqtt 77/720; each is a facade friend holding `ProjectEditor& m_editor` (+ `ProjectModel& m_model`; Mqtt takes the editor only), translates through `Q_DECLARE_TR_FUNCTIONS(DataModel::ProjectEditor)`, and the facade owns all eight by value, declared and constructed last.
- Stayed on the facade with the reason: `setCurrentView`/`setSuppressViewChange` (the signature names the facade's nested `CurrentView`, which a by-value sub-object header cannot see), the generated `add*Section`/`buildFft*`/`buildWidget*` family plus hand-written `addFFTSection` (the generator emits them as `ProjectEditor::` members), `generateComboBoxModels`/`appendExtensionWidgets` (combo vocabulary read by Forms, Commit and MultiSelect), and the six one-line state readers (`selectedUserTable`, `selectedWorkspaceId`, `selectedFolderId`, `selectedGroupFolderId`, `selectedTableFolderId`, `treeSearchQuery`).

### T27: Editor re-form: bodies move verbatim

- **Files:** the eight new `.cpp` files, `core/Ui/ProjectEditor/ProjectEditor.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** Each `ProjectEditor::fn` body moves into its sub-object unchanged; facade methods forward; private members migrate to the sub-object that owns them; the old `ProjectEditor*.cpp` files are deleted.
- **Verify:** `code-verify --tu-census --check` (excess falls); no `ProjectEditor::` definition outside `ProjectEditor.cpp`; C1 editor walk.
- **Deps:** T26
- [x] done
- Bodies moved verbatim by script: facade members gain `m_editor.`, `m_projectModelRef` becomes `m_model`, `this` as QObject parent/context becomes `&m_editor`, a moved slot's connect becomes a `[this]{...}` lambda on the same connection type, `using enum ProjectEditor::{CustomRoles,EditorWidget,CurrentView}` per TU, and `ItemKind`-typed sites are qualified `ProjectEditor::KindX` (bare `KindX` resolves to `DataModel::EntityKind`). Two mechanical splits kept functions under 100 lines after the prefix reflow: `EditorWiring::wireProjectFileSignals` (out of `wireProjectModelRebuilds`) and `EditorForms::buildSourcePayloadRows` (out of `buildSourceFrameDetectionRows`).
- Facade TU 1338 lines (ctor, instance, own accessors/setters, 69 forwarders); the eight `ProjectEditor*.cpp` removed (`git rm`); `core/Ui/CMakeLists.txt` lists the pairs; census per-file keys renamed (total 1537 flat, the Translator/FrameBuilder/Publisher ctor-captures moved with their sole users); layer-verify 0 errors with every edge count flat; tu-census flat (no editor TU crossed 1500 before or after, so the excess had nothing to shed); registry-verify CLEAN; claim-verify 0 after `directory-map.md`, `project.md` and `ss-new-driver/SKILL.md` were re-pointed. The `ProjectEditor::` definitions left outside `ProjectEditor.cpp` are the generated ones in `Generated/DatasetForm.cpp`.

### T28: Generators, verifiers and tests follow the editor move

- **Files:** `scripts/generate-property-registry.py` (`PropertyHooks.h` literals, `DatasetForm.cpp` output path), `scripts/registry-verify.py:704`, `scripts/code-verify.py:3477-3478`, `app/tests/CMakeLists.txt` (13 `PropertyValidators.cpp` registrations, `ProjectEditor`-related suites), `.claude/skills/ss-new-driver/SKILL.md`, `doc/claude/architecture/project.md`
- **Does:** Paths follow T25; regenerate.
- **Verify:** generator `--check`s, `registry-verify` CLEAN, `claim-verify` 0 errors.
- **Deps:** T25
- [x] done
- Generator emits `DatasetForm.cpp` at `core/Ui/ProjectEditor/Generated/` with `ProjectEditor/ProjectEditor.h`; the `PropertyHooks.h` literals are unchanged (the hooks stayed in Pipeline). `registry-verify.py` `EDITOR_H`, `code-verify.py` `_API_GENERATED`, `singleton-census.json` keys (counts flat), `tst_project_nav_history` source path, ss-new-driver SKILL.md, project.md and directory-map.md re-pointed; the 13 `PropertyValidators.cpp` registrations were not touched since that TU did not move.

### T29: Model files lose their Ui reaches

- **Files:** `core/Pipeline/DataModel/Project/ProjectWorkspaces.cpp`, `ProjectPresentation.cpp`, `core/Core/SerialStudio.{h,cpp}`
- **Does:** `WidgetExtensions::persistedTypeToken` becomes a `SerialStudio::` static (pure string); `IconRegistry::iconById` resolves through the Core include.
- **Verify:** `layer-verify --json` (Pipeline→Ui only `Dashboard.h`/`WidgetExtensions.h`/`AudioExport.h` remain).
- **Deps:** T20, T25
- [x] done
  `SerialStudio::persistedExtensionTypeToken()` in Core; `ProjectPresentation.cpp` no longer includes the Ui catalog. After T25 the Pipeline->Ui remainder is `AudioExport.h` x2 (P3), `WidgetExtensions.h` in `PropertyHooks.cpp`/`WidgetResolution.cpp` (P2 catalog topic) and `Dashboard.h` x4 (P2 inversion, P4 control interface).

### T30: `NotificationCenter` to Ui, posts become the topic

- **Files:** `core/Pipeline/DataModel/NotificationCenter.{h,cpp}` → `core/Ui/Misc/NotificationCenter.{h,cpp}`; new `core/Pipeline/DataModel/Scripting/NotificationScriptApi.{h,cpp}`; post sites (`FrameBuilder.cpp`, `ProjectLoader.cpp`, `ProjectPersistence.cpp`, `FrameReader.cpp:386`, `UART.cpp:615`, `USB.cpp:491`, `ConnectionManager.cpp`, `DatabaseManager.cpp`, `Sessions/Player.cpp`, `MQTT/Publisher.cpp:772`); script installers (`TransformCompiler`, `LuaScriptEngine`, `ScriptApiCall`, `DatasetTransformEditor`); `app/src/SessionContext.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`
- **Does:** The center subscribes `NotificationRaised` (AutoConnection: queued from the pipeline thread and drivers, direct from GUI) and `NotificationClearRequested`; every lower-library post publishes; `installScriptApi(lua_State*/QJSEngine*)` bodies move to `NotificationScriptApi` whose `notify/info/warning/critical` publish and `clear` requests. Invariant: `qInstallMessageHandler` still runs after the center exists; `FrameReader`'s pipeline-thread publish allocates one message per warning (command rate, as the `invokeMethod` it replaces did). Ui remains a `SessionContext` slot (adopt call re-pointed).
- **Verify:** `layer-verify --json` (Api→Pipeline reaches of `NotificationCenter` vanish in T60; Pipeline/Devices/Storage no longer include it); `grep -rn 'NotificationCenter::instance' core/Pipeline core/Devices core/Storage` empty.
- **Deps:** T4, T5, T17
- [x] done. Amended: center stays in Pipeline; only cross-library posts moved to the bus; drivers reach the bus through HAL_Driver::attachMessageBus

### T31: Diagnostics types and the open-attempt topic

- **Files:** new `core/Core/DiagnosticsTypes.h` (from `Misc/Diagnostics/DiagnosticsShared.h`: `Diagnostics::Bus`, slug/bit helpers), `core/Devices/IO/ConnectionManager/DriverFactory.{h,cpp}`, `core/Devices/IO/ConnectionManager.cpp:1231`, `core/Ui/Misc/ConnectionDiagnostics.{h,cpp}`
- **Does:** `onOpenSucceeded/Failed` → `publish<DeviceOpenAttempted>`; the diagnostics runner subscribes.
- **Verify:** `layer-verify --json` (Devices→Ui −2).
- **Deps:** T17
- [x] done -- `Core/DiagnosticsTypes.h` holds the Bus/Verdict/Result vocabulary and slug helpers; `ConnectionManager::onDeviceOpenFinished` publishes `DeviceOpenAttempted`, `ConnectionDiagnostics` subscribes in `setupExternalConnections()` (Devices->Ui 7 -> 5).

### C1: Phase 1 checkpoint (maintainer)

- **Does:** Build both configurations, `ctest`, launch. Walk: open the project editor, edit every form type, MQTT tab, multi-select, tree drag; trigger a JS parse error and a Lua transform error (dialogs from the pipeline thread); language change incl. an RTL locale; workspace path change via the dialog; a driver warning (unplug UART) reaches the notification centre and the tray. Gate log pasted.
- **Expected:** Pipeline→Ui ≤ 12, Devices→Ui ≤ 3, Storage→Ui ≤ 4, Api→Ui ≤ 21, Pipeline→Devices ≤ 34.
- **Deps:** T18–T31
- **Agent gate log (2026-09-08, before the maintainer build):**
  `layer-verify` 0 errors, baseline re-seeded on pure shrink (348 -> 250 upward includes, 16 edges):
  Pipeline->Ui 9 (expected <= 12), Devices->Ui 5 (expected <= 3, see below), Storage->Ui 4, Api->Ui 19,
  Pipeline->Devices 23, Pipeline->App 22, Ui->App 25, Devices->App 21, Api->App 14, Storage->App 16,
  Pipeline->Api 9, Pipeline->Storage 14, Storage->Devices 8, Devices->Storage 4, Devices->Api 2.
  Devices->Ui landed 2 above the line: the five left are `Console/Handler.h` x2 (the P3 raw-byte tap,
  T51), `AudioExport.h` (the P3 sink interface, T53) and `CommonFonts.h`/`ThemeManager.h` in
  `PublisherScriptEditor.cpp` (T55 moves the editor up to Ui rather than the fonts down). None is
  P1 work, so no task was widened to hit the number.
  `code-verify --check` whole tree: 0 errors (13 pre-existing advisories); singleton census 1537 flat;
  TU census excess 2791 flat (the editor re-form and `NotificationPayload` unit were shaped to keep it
  so); dup census flat; `claim-verify` 0/0; `registry-verify` CLEAN; `generate-property-registry
  --check` up to date; `documentation-verify` 0 findings; `pytest scripts/tests` 211 passed;
  `pytest tests/scripts` 312 passed (the editor bounds-check test now reads the re-formed
  `core/Ui/ProjectEditor/*.cpp` and normalises the `m_editor.` facade prefix).
  Deviations from the plan, all recorded on their tasks: T18 `PasswordHash` lands in Pipeline (needs
  Qt Network) instead of Core; T22 prompt seam method names (`promptMessage`, async
  `promptDirectory`, `revealInFileManager`); T26/T27 the editor is a facade plus eight sub-objects
  (`EditorWiring/Selection/Tree/Forms/Commit/Summaries/MultiSelect/Mqtt`), `ProjectModel` gained
  `editorSelectionRequested` for the reverse edge; T28 `NotificationCenter` stays in Pipeline with
  a bus-facing `setupExternalConnections()` (plan amended) and `MQTT::notificationPayload` builds the
  publisher's JSON from `NotificationPosted`; T31 `ConnectionDiagnostics` subscribes to
  `DeviceOpenAttempted` rather than a direct call.
  Outside this spec but in the same working tree (maintainer requests during P1, to eyeball on the
  same launch): row-based group widgets (DataGrid, BarPanel, LEDPanel, MultiPlot) share
  `Widgets::datasetWidgetLinks` and `DatasetWidgetButtons.qml` (per-dataset "also shown by" icons,
  18 px floor, tooltips on elided titles/values); BarPanel rows without alarm bands use the dataset
  accent via `rowColor(int)` instead of the transparent group colour.
- **C1 defects found by the maintainer's build (2026-09-08):** (1) `UserPrompt.cpp` streamed into
  `qWarning()` with only `<QtLogging>` included (`C2027: use of undefined type 'QDebug'`); the six
  TUs that lost `QMessageBox`/`QFileDialog` in the prompt sweep (`WorkspaceManager`, `ProjectLoader`,
  `ProjectPersistence`, `ProjectModel`, `JsScriptEngine`, `MDF4/Player`) had leaned on the same
  transitive `QDebug` and now include it explicitly. (2) `LuaScriptEngine.cpp` passed the old
  seventh argument (per-button captions, "Fix Automatically"/"Leave Unchanged") which the seam had
  dropped (`C2660: function does not take 7 arguments`); `Core::Prompt::ButtonLabels` now travels
  through `showMessageBox` and `IUserPrompter::promptMessage`, converted by `Misc::Utilities`.
- [x] done
  Maintainer build (Release, MSVC 2022, Qt 6.11.0) compiles cleanly after the two C1 defects above;
  `ctest` and the C1 walk still to be run by the maintainer (with C0's `ctest`).

## Phase 2: upward facts as retained topics

### T32: `AppState` into Pipeline and its topics

- **Files:** `app/src/AppState.{h,cpp}` → `core/Pipeline/AppState.{h,cpp}`; `core/Pipeline/CMakeLists.txt`; `app/src/SessionContext.{h,cpp}`; `app/src/Misc/ModuleManager.cpp`; Storage/Api/Ui includers (`AppState.h` → `Pipeline/AppState.h` is not needed: the Pipeline root resolves `AppState.h`; verify no ambiguity)
- **Does:** `setOperationMode` publishes `publishState<OperationModeChanged>` immediately BEFORE emitting `frameConfigChanged` (ordering invariant: mode first, then frame config, as today's two signals); `onProjectLoaded` publishes `ProjectLoaded`. Devices includers are removed in T33.
- **Verify:** `layer-verify --json` (Storage/Api/Ui →App −9/−8/−9); `include-ambiguous` clean.
- **Deps:** T5, T17
- [x] done -- `AppState` lives in `core/Pipeline/AppState.{h,cpp}` (Pipeline list, AUTOMOC) and retains `OperationModeChanged` then `FrameConfigChanged` (T45's topic and the `Core/IO/FrameConfig.h` move pulled forward: the framing is Qt Core only) from its constructor and every mode change, `ProjectLoaded{path, title}` on a real path change. The seven driver `setOperationMode` calls became `ProjectModel::loadGeneratedProject()` (T57's serving half, a direct call for now: the loader switches the mode, loads, restores the previous mode on a rejected document and marks the project modified; whitelisted in the undo lint), so no Devices file names `AppState` and Devices->Pipeline could not grow.

### T33: Operation-mode readers and connections

- **Files:** Devices (`ConnectionManager.cpp` ×10, `StreamConfigBuilder.cpp`, `UiDriverSync.cpp`, `StreamWorkerPool.cpp`, `DeviceIoRouter.cpp`), Storage (`CSV/Export.cpp`, `CSV/Player.cpp`, `MDF4/Export.cpp`, `MDF4/Player.cpp`, `Sessions/Export.cpp`, `Sessions/Player.cpp`, `ReplaySynthesis.cpp`, `DatabaseManager.cpp`, `Verifier.cpp`), Api (`DashboardHandler.cpp`, `ProjectFileCommands.cpp`, `ProjectParserCommands.cpp`, `SourceHandler.cpp`, `WorkspacesHandler.cpp`, `MirrorPublisher.cpp`, `MirrorSession.cpp`), Ui (`Dashboard.cpp` ×7 + `:265,271,750-761`, `WidgetMapBuilder.cpp`, `Taskbar.cpp`, `TaskbarWorkspaces.cpp`, `Console/Handler.cpp`, `Console/Export.cpp`, `ProjectCheckers.cpp`, `FileOpenEventFilter.cpp`, `Painter.cpp`)
- **Does:** Reads → `latest<OperationModeChanged>()` (command rate) or a subscribed cached member; `connect(&AppState::instance(), operationModeChanged/projectFileChanged, …)` → `subscribe<…>` with the SAME connection type per site (Dashboard `:271` queued, `:750` direct, ConnectionManager `:783` queued). Invariant: `DeviceIoRouter::processPayload` (per chunk) reads a new cached `m_consoleOnly` refreshed by subscription, never `latest<>()`. Devices setters are removed in T54.
- **Verify:** `grep -rn 'AppState::instance' core/Devices core/Storage core/Api core/Ui` empty except direct downward calls in Storage/Api/Ui setters; `layer-verify --json` (Devices→App −12).
- **Deps:** T32
- [x] done -- Devices reads go through `ConnectionManager`'s cached `m_operationMode`/`m_frameConfig`, seeded from the retained state in the constructor (the spec-0044 headless root asks for a FrameConfig with no wiring pass) and refreshed by direct subscriptions in `IO::ConnectionBusBridge` (new sub-object: the facade TU was already over the census line); the four sub-objects take `const SerialStudio::OperationMode&` / `const FrameConfig&` instead of `AppState&`, so `DeviceIoRouter::processPayload` reads the cached enum. The queued `rebuildDevices`/`resetFrameReader` hops keep their connection type. Storage/Ui signal hops (CSV/MDF4/Sessions exports, `Console::Handler`) are subscriptions; the plain reads in Storage/Api/Ui stay direct downward calls (AppState is Pipeline now, no edge) and `Dashboard.cpp`/`PipelineHost.cpp` keep direct intra-library connects because both are hotpath TUs the bus lint fences.

### T34: Pipeline caches refreshed from the bus

- **Files:** `core/Pipeline/DataModel/FrameBuilder.cpp:587-590,635,1144-1160`, `core/Pipeline/IO/PipelineHost.cpp:159-177`
- **Does:** `m_operationMode` and the PipelineHost atomic mirror are refreshed from `subscribe<OperationModeChanged>(…, AutoConnection, replayLatest=true)`. Invariant (cached hotpath flag, two-thread refresh rule): FrameBuilder's slot stays pipeline-affine (auto → queued from the GUI publisher, FIFO, never torn); PipelineHost writes its atomic direct on the GUI thread; the seed read at `:635` is replaced by replay; the hotpath keeps reading the member.
- **Verify:** `code-verify --check` (hotpath TU: `bus-on-hotpath` must stay clean, so the subscription lives in `setupExternalConnections`, not a hotpath-allowlisted TU: `FrameBuilder.cpp` IS allowlisted; place the subscription in `core/Pipeline/DataModel/FrameBuilder/ExternalWiring.cpp` (new, non-hotpath) that the facade calls); C2 mode-switch walk.
- **Deps:** T33
- [x] done -- `FrameBuilder` seeds `m_operationMode` from PipelineHost's mirror and reacts through `ExternalWiring::watchOperationMode()` (auto, so queued once the builder moved); the same subscription pass writes PipelineHost's atomics directly on the GUI thread through the new `refreshOperationModeMirror()`/`refreshLinkMirror()` setters (replayed from the retained state), because PipelineHost.cpp itself may not name the bus. `bus-on-hotpath` clean.

### T35: Licence topic and its ten receivers

- **Files:** `app/src/Misc/ModuleManager.cpp`, `app/src/Licensing/LemonSqueezy.cpp`; receivers `FrameBuilder.cpp:195` (via `ExternalWiring.cpp`), `ProjectModel.cpp:641`, `ConnectionManager.cpp:790`, `MDF4/Export.cpp:530`, `Sessions/Export.cpp:891`, `InfluxDB/Export.cpp:545`, `Console/Export.cpp:212`, `Dashboard.cpp:367`, `AudioExport.cpp:672`, `Terminal.cpp:129`; `featureTier()` readers (`LicensingHandler` stays root-side)
- **Does:** Root hook: `Core::License::set(...)` then `publishState<LicenseStateChanged>{activated, tier, trialExpired}` on `activatedChanged` and once after the licensing block constructs; receivers `subscribe<LicenseStateChanged>(…, replayLatest=true)` (all GUI-thread; ConnectionManager's stays queued). Invariant (spec 0042): `activatedChanged` fires only on real transitions; late activation must still re-derive (replay + subscription cover both).
- **Verify:** `grep -rn 'LemonSqueezy::instance\|CommercialToken' core` empty; `layer-verify --json` (licensing includes −~30).
- **Deps:** T14, T17
- [x] done -- Root publishes `LicenseStateChanged{activated, tier, trialExpired}` beside `Core::License::set` (guard spelling kept for the regression test). Ten receivers subscribe (`ExternalWiring::watchLicense`, `ProjectModel`, `ConnectionBusBridge` queued, the four exports, `Dashboard` via `DashboardWiring`, `AudioExport`, `Terminal`); classes constructed before their bus attaches use `Core::Bus::MessageBus::instance()` with a null guard. No replay: the connects only ever fired on transitions and the root publishes once before any receiver constructs. `ConnectionManager::connectDevice` reads `Core::License::trialExpired()/activated()`. The `CommercialToken`/`SS_LICENSE_GUARD` sites stay (distinct guard dispatch sites, the plan's "licensing pending"), as do `LicensingHandler` and `ShortcutGenerator::hasProLicense` (needs `trialEnabled`, not in the topic).

### T36: Connection state on the bus

- **Files:** `core/Devices/IO/ConnectionManager.cpp` (`notifyConnectedStateChanged`, `contextsRebuilt`/`deviceListRefreshed`/`driverChanged` emitters); subscribers `FrameBuilder/ExternalWiring.cpp` (for `:571-579`), `PipelineHost.cpp:168-177`, `ProjectModel.cpp:634`, `ControlScript.cpp:85,257`, `QuickPlotBuilder.cpp:75-96`, `ProjectLoader.cpp:878`, `CSV/Player.cpp:450`, `MDF4/Player.cpp:279`, `Sessions/Player.cpp:110`, `core/Ui/ProjectEditor/EditorWiring.cpp`
- **Does:** `publishState<ConnectionStateChanged>{sourceId, connected, paused, connecting, busType}` from the idempotent notifier; `DeviceCatalogChanged` on the three editor signals. Invariant: FrameBuilder's `onConnectedChanged/onPausedChanged` stay pipeline-affine (queued) and keep reading PipelineHost's mirrors; PipelineHost's mirrors are written direct on the GUI thread at transition rate; `emitSessionBoundary` inputs unchanged.
- **Verify:** `grep -rn 'ConnectionManager::instance' core/Pipeline core/Storage` shows only the direct-call sites removed in T65; `layer-verify --json` (Pipeline→Devices −~10).
- **Deps:** T17, T34
- [x] done -- `ConnectionBusBridge::publishConnectionState` retains `ConnectionStateChanged{0, connected, paused, connecting, busType}` from the idempotent notifier, the pause transition and the constructor; the three editor signals publish `DeviceCatalogChanged` generations. Subscribers: `ExternalWiring::watchLinkState` (mirror direct + builder queued), `ProjectModel` (transient state), `ControlScript` (link + `shouldRun`), `QuickPlotBuilder` (`busType` via `latest<>`). `PipelineHost.cpp` no longer includes ConnectionManager or AppState. The editor keeps its downward `driverChanged` connect; the players' `isConnected()/disconnectDevice()` calls are T65.

### T37: Replay state on the bus

- **Files:** `core/Storage/CSV/Player.cpp`, `MDF4/Player.cpp`, `Sessions/Player.cpp` (`openChanged`), `FrameBuilder/ExternalWiring.cpp` (for `:625-628,810-820`), `FrameBuilder.h` (`m_playerOpen` becomes a 3-bit mask + bool), `ControlScript.cpp:95-102`, `core/Ui/UI/SerialStudioHelpers.cpp` (`isAnyPlayerOpen`)
- **Does:** Each player `publishState<ReplayPlayerStateChanged>{playerId, open}`; FrameBuilder subscribes (queued into the pipeline thread) and recomputes `m_playerOpen`; `ControlScript` subscribes; the helper reads `latest<>()` ×3. Invariant (cached hotpath flag): the hotpath keeps reading `m_playerOpen`; the mask is written only on the pipeline thread.
- **Verify:** `layer-verify --json` (Pipeline→Storage −5 Player includes); C2 replay walk.
- **Deps:** T17, T34
- [x] done -- Players retain `ReplayPlayerStateChanged{id, open}` next to every `openChanged` (CSV 0, MDF4 1, Historian 2); `ExternalWiring::watchPlayers` and `ControlScript` keep a three-slot mask from it (seeded from the single retained message; nothing is open before a wiring pass). Pipeline no longer includes any player header. `Replay/PlayerState.h` stays for the Storage exports and the QML helper (a downward read; the topic retains one player, not three).

### T38: Audio capture format topic

- **Files:** `core/Devices/IO/Drivers/Audio.cpp`, `core/Pipeline/DataModel/FrameBuilder/QuickPlotBuilder.cpp:215-221`, `core/Pipeline/CMakeLists.txt` (drop `ss_apply_miniaudio_definitions`)
- **Does:** `publishState<AudioCaptureFormat>{format, sampleRate, normalized}` on open/config; QuickPlot reads `latest<>()` inside the existing `runOnGuiThreadBlocking` (unchanged marshal).
- **Verify:** `layer-verify --json` (Pipeline→Devices −1 `Audio.h`); `grep -n miniaudio core/Pipeline` empty.
- **Deps:** T16, T17
- [x] done -- `Audio` retains `AudioCaptureFormat` on open and on every input reconfiguration (normalization included); `QuickPlotBuilder` reads it inside the unchanged GUI marshal and mirrors the five `ma_format` ordinals it needs. `ss_apply_miniaudio_definitions(SerialStudioPipeline)` is gone.

### T39: Widget-extension catalog topic and the Pipeline resolver

- **Files:** `core/Ui/UI/WidgetExtensions.cpp`; new `core/Pipeline/DataModel/WidgetResolution.{h,cpp}`; callers `ProjectWorkspaces.cpp`, `ProjectWorkspaceRefs.cpp`, `ProjectEntities.cpp`, `core/Ui/ProjectEditor/EditorSummaries.cpp`, `core/Api/API/Handlers/WorkspacesHandler.cpp`, `core/Ui/UI/SerialStudioHelpers.cpp`
- **Does:** `publishState<WidgetExtensionCatalog>` on `catalogChanged`; `getDashboardWidget(s)`/`extensionGroupWidgetCount` re-homed in `WidgetResolution` over `latest<>()`; Ui callers keep calling the same functions through the Pipeline header. Invariant (spec 0038): `ext:<id>` resolution and `DashboardExtension = 100` unchanged; the initial catalog is published before `restoreLastProject()`.
- **Verify:** `layer-verify --json` (Pipeline→Ui `WidgetExtensions.h` gone); C2 extension-widget project load.
- **Deps:** T17, T25
- [x] done -- `WidgetExtensions::announceCatalog()` retains `WidgetExtensionCatalog` before every `catalogChanged` (rescan and the three consent paths; the first rescan precedes `restoreLastProject()`); `WidgetResolution.cpp` and `PropertyHooks::widgetExtensionOptions` resolve from it through `MessageBus::instance()`. Pipeline no longer includes `UI/WidgetExtensions.h`.

### T40: Dashboard ↔ ProjectModel inversion and the structure topic

- **Files:** `core/Ui/UI/Dashboard.cpp`, `core/Pipeline/DataModel/ProjectModel.cpp`, `core/Pipeline/DataModel/Project/ProjectLoader.cpp`
- **Does:** Dashboard reads `points`/`plotTimeRange` from `ProjectModel` on `ProjectLoaded` and writes them back on its own setters (downward calls); ProjectModel stops calling `Dashboard::setPoints/setPlotTimeRange` and stops connecting `pointsChanged`/`widgetCountChanged`; Dashboard `publishState<DashboardStructureChanged>{generation, widgetCount}`; ProjectModel subscribes. Invariant (project.md undo): the two-phase memento is untouched; the same project fields are written.
- **Verify:** `layer-verify --json` (Pipeline→Ui `Dashboard.h` −3); project round-trip test fixture unchanged (`tests/integration/test_project_*`).
- **Deps:** T17
- [x] done -- Inverted: `Dashboard` follows `pointCountChanged`/`plotTimeRangeChanged` downward (ProjectFile mode) and writes its own edits back through `setPointCount`/`setPlotTimeRange` after emitting; both ends keep their equality early-outs, so the loop is idempotent. `ProjectModel`/`ProjectLoader` no longer name the dashboard; a document without a point count keeps the model's current count (the old default read the dashboard's). `DashboardWiring` retains `DashboardStructureChanged{generation}` on `widgetCountChanged`; `ProjectModel` subscribes for the QuickPlot workspace rebuild.

### T41: Dashboard view state topic and requests

- **Files:** `core/Ui/UI/Dashboard.cpp`, `core/Storage/Sessions/Export.cpp:1116-1135`, `core/Storage/Sessions/Player.cpp:596-605`
- **Does:** Dashboard `publishState<DashboardViewState>{json}` on `viewStateChanged`; Export subscribes (1.5 s debounce stays in Export); Player publishes `DashboardViewStateRestoreRequested`/`ClearRequested`; Dashboard serves them.
- **Verify:** `layer-verify --json` (Storage→Ui `Dashboard.h` −1 of 4); Historian bundled view-state restore in the C2 walk.
- **Deps:** T17
- [x] done -- `DashboardWiring` retains `DashboardViewState` on every change (and once at construction); `Sessions::Export` subscribes with replay and lost its dashboard pointer; `Sessions::Player` publishes the restore/clear requests, which the wiring serves with direct delivery so a restore lands before the player's next statement, and reads the pre-session view state from the retained topic.

### T42: Benchmark flag in Core

- **Files:** new `core/Core/Runtime.{h,cpp}` (`benchmarkActive()` module-static bool), `core/Ui/UI/Dashboard.cpp:671`, `app/src/Benchmark/HotpathBenchmark.cpp:172-175`
- **Does:** `streamAvailable()` reads `Core::Runtime::benchmarkActive()`. Invariant: a plain static, never a construction (the getter runs inside Dashboard's ctor).
- **Verify:** `layer-verify --json` (Ui→App −1).
- **Deps:** none
- [x] done -- `Core::Runtime::benchmarkActive()`; `HotpathBenchmark::active/setActive` forward to it; `Dashboard::streamAvailable` reads it. Ui->App loses the benchmark include.

### C2: Phase 2 checkpoint (maintainer)

- **Does:** Build, `ctest`, launch. Walk: switch all three operation modes with a device connected; activate/deactivate (offline licence file) and watch Plot3D/MultiPlot fallback re-derive; connect, pause, resume, disconnect UART and MQTT; load a project with an extension widget; CSV and Historian replay with seek and bundled view state; QuickPlot audio. `pytest tests/integration -m "not destructive"` subset on project/session/io files. Gate log pasted.
- **Expected:** `*->App`: Pipeline ≤ 11 (SessionContext 7, gRPC 2, licensing 2 pending), Ui ≤ 4, Devices ≤ 3, Api ≤ 2, Storage 0.
- **Deps:** T32–T42
- **Agent gate log (2026-09-08, before the maintainer build):**
  `layer-verify` 0 errors, baseline re-seeded on pure shrink (250 -> 173 upward includes):
  Pipeline->App 11 (met), Ui->App 11, Devices->App 7, Api->App 6, Storage->App 4; Pipeline->Ui 5,
  Pipeline->Devices 18, Pipeline->Storage 8, Storage->Ui 3, Devices->Pipeline 53, the rest flat.
  The four `*->App` edges above their line are, to the include, `Licensing/CommercialToken.h` for
  the `SS_LICENSE_GUARD` dispatch sites (Ui 6, Devices 4, Storage 4, Api 1: deliberately distinct
  guard sites, not mechanically foldable into `Core::License`), `SessionContext.h` (P5, T66),
  `LicensingHandler`'s four licensing includes (root-side by plan), gRPC (T58) and
  `ShortcutGenerator`'s `trialEnabled` read. None is a Phase 2 reach, so no task was widened.
  `code-verify --check` 0 errors on the 402 touched files; singleton census re-seeded on shrink
  (1537 -> 1489, static caches 1051 -> 1027); TU census re-seeded on shrink (excess 2791 -> 2787,
  `ConnectionManager.cpp` fell below its old line once `ConnectionBusBridge` took the bus seam);
  dup census flat; `claim-verify` 0/0; `documentation-verify` 0 findings; `pytest scripts/tests`
  214 passed; `pytest tests/scripts` 319 passed (the licence-guard test pins the publisher's guard
  spelling, restored).
  Deviations from the plan, all recorded on their tasks: T32 pulls forward T45's `FrameConfig`
  move and topic and T57's `loadGeneratedProject` serving half; T33/T34 keep direct connects in
  the three hotpath TUs and add `IO::ConnectionBusBridge`; T35 no replay, guard sites kept; T37
  seeds from the single retained message and keeps `PlayerState.h` for the downward readers; T40
  adds `UI::DashboardWiring` (the Dashboard TU is bus-fenced) and changes the point-count default
  of a document without one; T41 serves requests with direct delivery. Two new sub-object pairs
  outside the plan's file list (`ConnectionManager/BusBridge`, `Dashboard/DashboardWiring`), both
  forced by the hotpath bus lint or the TU census.
  Hotpath: the cached flags (`m_operationMode`, `m_playerOpen`, PipelineHost's three atomics)
  are still plain members written on their owning thread; only their refresh source moved to the
  bus, at transition rate. `--benchmark-hotpath` root unchanged (it never ran a wiring pass).
- **C2 defects found by the maintainer's build (2026-09-09):** (1) `core/Core/ThirdParty/fast_float.h`
  failed to parse (`C3878` at two `FASTFLOAT_TRY ... else` sites): the agent's first Phase 2
  formatting pass ran `clang-format -i` over every file in `git status`, the moved vendored header
  included, and the reflow broke the macro-heavy `if`/`else`. Restored byte-for-byte from the
  staged original; every vendored file is a pure rename again. Lesson for P3+: the format/lint
  file list excludes `ThirdParty/` and `lib/` (the whole-tree linter already skips them).
- [x] done
  Maintainer build (Release, MSVC 2022, Qt 6.11.0, 2026-09-09) compiles and the application works
  after the vendored-header restore; `ctest` still to be run by the maintainer (with C0/C1's).

## Phase 3: the hotpath seams (value types, ingest binder, sinks)

### T43: `HAL_Driver.h` into Core

- **Files:** `core/Devices/IO/HAL_Driver.h` → `core/Core/IO/HAL_Driver.h`; includers `FrameReader.h`, `FrameBuilder.h`, `LatestFrameTap.h`, `StreamWorker.h`, `FrameParserPipeline.cpp`, `Console/Handler.h`, `ImageView.h`, every driver; `core/Core/CMakeLists.txt`, `core/Devices/CMakeLists.txt`, `app/tests/CMakeLists.txt` (the three bare-AUTOMOC registrations)
- **Does:** Verbatim (header-only, Qt Core). Invariant (source owns time): `CapturedData` stamping unchanged.
- **Verify:** `layer-verify --json` (Pipeline→Devices −5, Storage→Devices −1).
- **Deps:** none
- [x] done -- done in Stage A of P3 together with `Core/IO/LinkStats.h`, `Core/IO/StreamConfig.h` and the three seam interfaces.

### T44: Frame value types into Core

- **Files:** `core/Pipeline/DataModel/{Frame.h,Frame.cpp,FrameKeys.h,DataBlock.h,ExportSchema.h,FrameConsumer.h,FrameConsumer.cpp,FrameSupport.h,FrameSupport.cpp}` → `core/Core/DataModel/`; new `core/Core/DataModel/ColorValidator.{h,cpp}` (function-pointer hook, default accepts); `app/src/Misc/ModuleManager.cpp` (installs the `QColor::fromString` check in `bindInterfaces()`); CMake lists; `app/tests/CMakeLists.txt` (12 `Frame.cpp` + 3 `FrameConsumer.cpp` registrations link Core; the marker test installs a hex validator)
- **Does:** Verbatim except `Frame.cpp:270` → `DataModel::colorStringValid(m.color)`, `commercialCfg` → `Core::License::activated()`, includes → `Core/…`. `TextCodec` stays in Pipeline. Invariant (hotpath): `Frame.h`'s in-place assign helpers and `DataBlock` layout are byte-identical; the `hotpath-assert-scope` allowlist paths are updated in `code-verify.py` (`_HOTPATH_ASSERT_ALLOWED`).
- **Verify:** `layer-verify --json` (Devices→Pipeline −~20); `code-verify --check` incl. the allowlist update; C3 benchmark.
- **Deps:** T6, T7, T12, T14
- [x] done -- the hook header is `core/Core/DataModel/PropertyValidators.{h,cpp}` (the four ProjectModel-free validators moved with the colour hook; `DataModel::PropertyHooks::isValidColor` stays the call) rather than `ColorValidator`; `get_tx_bytes` went to `core/Pipeline/DataModel/ActionBytes.{h,cpp}` because it reaches the text codecs (Core5Compat); `commercialCfg` already read `Core::License` since P0. Generated `DatasetSerialization.cpp` now lives under `core/Core/DataModel/Generated/` (generator, `_REGISTRY_GENERATED_DIRS`, `registry-verify` follow). Unit suites drop the moved TUs from SOURCES and `tst_frame_json_legacy`/`tst_property_validators` install the colour hook in `initTestCase`. The mirror-wire digest was re-seeded: its inputs changed by include text only (P1 repoints), not by codec.

### T45: `FrameConfig`/`StreamConfig` into Core and the frame-config topic

- **Files:** `core/Pipeline/IO/FrameConfig.h` → `core/Core/IO/FrameConfig.h`; new `core/Core/IO/StreamConfig.h` (from `StreamWorker.h`); `core/Core/Bus/Messages.h` (`FrameConfigChanged{IO::FrameConfig}`); `core/Pipeline/AppState.cpp` (publishes after `OperationModeChanged`); consumers `ConnectionManager.cpp:776` (queued `resetFrameReader`), `StreamConfigBuilder.cpp:63`, `ProjectParserCommands.cpp:803`
- **Does:** Value types move; the topic replaces `AppState::frameConfigChanged` for Devices. Invariant: publish order mode → frame config preserved.
- **Verify:** `layer-verify --json` (Devices→Pipeline −2).
- **Deps:** T6, T32
- [x] done -- the `FrameConfig` half landed in P2 (T32); `StreamConfig.h` in Stage A, with `StreamAttachment{deviceId, driver, config}` as the binder's stream-source record.

### T46: `IIngestBinder` interface

- **Files:** new `core/Core/IO/IIngestBinder.h`, `core/Core/IO/LinkStats.h` (from `Devices/IO/ConnectionManager/DeviceTableQuery`'s `IO::LinkStats`)
- **Does:** Pure virtual: `attach`, `reconfigure`, `detach`, `attachStream`, `setStreamPaused`, `injectPayload`, `injectMultiSourcePayload`, `linkStats`, `resetQuickPlotHeaders`. Qt Core only.
- **Verify:** `code-verify --check`.
- **Deps:** T43, T45
- [x] done -- surface differs from the plan: `rebuildStreams(vector<StreamAttachment>, paused, connected)`, `setStreamPaused`, `publishStreamTemplates`, `detachStreams` replace `attachStream` (the pool is rebuilt as a set, as it always was); `injectMultiSourcePayload` folded into per-source `injectPayload(sourceId, ...)`; `linkStats()` is the aggregate the 1 Hz sample wants.

### T47: `PipelineHost` implements the binder and owns the readers

- **Files:** `core/Pipeline/IO/PipelineHost.{h,cpp}`; `core/Devices/IO/ConnectionManager/StreamWorkerPool.{h,cpp}` → `core/Pipeline/IO/StreamWorkerPool.{h,cpp}`; `core/Pipeline/CMakeLists.txt`
- **Does:** Readers keyed by device id, created on the GUI thread, configured, `moveToThread`, registered exactly as `registerFrameReader` does; the queued `HAL_Driver::dataReceived → FrameReader::processData` connect (from `DeviceManager.cpp:218-219`) is made here; `readyRead → routeFrames` stays explicit `Qt::DirectConnection`; `StreamWorkerPool` keeps its queued `blockReady → FrameBuilder::ingestStreamBlock`. Invariant (hotpath, SPSC): no mutex in `FrameReader`; recreate-not-lock on reconfigure; the old reader's `deleteLater` runs on the pipeline loop; `registerIngestThread` unchanged.
- **Verify:** new `app/tests/tst_ingest_binder.cpp` (FakeDriver attach → bytes reach a counting consumer; detach; inject); `code-verify --check` (`PipelineHost.cpp` is a hotpath TU: no bus token inside it, the T34/T36 subscriptions live in a non-hotpath TU `core/Pipeline/IO/PipelineHostWiring.cpp`).
- **Deps:** T46
- [x] done -- no `PipelineHostWiring.cpp` was needed: the T34/T36 subscriptions already live in `FrameBuilder/ExternalWiring.cpp`. The pool binds the host's mode atomic at construction and its `FrameBuilder` in `setupExternalConnections()`; `shutdown()` stops the pool before joining the thread; `streamWorkers()` moved to the host (Dashboard, `StreamHandler`).

### T48: Devices use the binder

- **Files:** `core/Devices/IO/DeviceManager.{h,cpp}`, `core/Devices/IO/ConnectionManager.{h,cpp}`, `ConnectionManager/DeviceIoRouter.{h,cpp}`, `ConnectionManager/DeviceTableQuery.cpp`, `app/src/Misc/ModuleManager.cpp`, `app/src/SessionContext.cpp` (ctor now `ConnectionManager(MessageBus&, IIngestBinder&)`)
- **Does:** `DeviceManager` drops `m_frameReader`/`m_pipeline` and calls `attach/reconfigure/detach`; `DeviceIoRouter::processPayload` → `injectPayload`; `DeviceTableQuery::linkStats` → `binder.linkStats`; `:681,:965` → `resetQuickPlotHeaders`. Invariant: `DeviceManager::open` stays a synchronous call; `openFinished` latch untouched.
- **Verify:** `grep -rn 'FrameReader\|PipelineHost\|FrameBuilder' core/Devices` empty except `StreamConfigBuilder` (T49); `layer-verify --json` (Devices→Pipeline −7).
- **Deps:** T47
- [x] done -- `DeviceManager(id, driver, config, IIngestBinder&)`, `SessionContext::create` became variadic so the root passes `ctx.pipelineHost()`. `DeviceTableQuery::linkStats` and the `refreshStreamExportFlags` slot are gone (the sinks announce their own edges, T53/T54, so `wireStreamLifecycle` lost its five sink connects). `tst_ingest_binder` pins DeviceManager against a recording binder; the host's side is covered by the C3 runtime checks.

### T49: Project structure snapshot and source-0 settings

- **Files:** `core/Core/Bus/Messages.h` (`ProjectStructureSnapshot`, uses Core `Source`/`Group`); `core/Pipeline/DataModel/ProjectModel.cpp` (publishes on structure change, serves `Source0ConnectionSettingsChanged` and emits `sourceConnectionChanged` for the editor); `core/Devices/IO/ConnectionManager/StreamConfigBuilder.cpp`, `DeviceTableQuery.cpp`, `UiDriverSync.cpp` (read `latest<>()`, publish the request; subscribe `ProjectLoaded`/snapshot to apply source-0 settings to the UI driver); `core/Pipeline/DataModel/Project/ProjectSources.cpp:307-350`, `ProjectLoader.cpp:877-895` (the UI-driver reads are deleted; the behaviour moves into `UiDriverSync`)
- **Does:** The 2026-09-04 mirror fix survives: a bus-type switch still captures the new driver's settings and the editor still redraws. Invariant: no device rebuild loop (`ConnectionManager` does not listen to `sourceConnectionChanged`).
- **Verify:** `tests/integration/test_source_mirror.py` at C3; `layer-verify --json` (Devices→Pipeline `ProjectModel.h` gone from the three sub-objects; Pipeline→Devices −2).
- **Deps:** T44, T48
- [x] done -- the snapshot is `ProjectStructureSnapshot{sources, groups, filePath, luaFastMode, frameDetection, change, sourceId, generation}`, published by `ProjectModel::wireStructureSnapshot()` from the model's own signals (the four kinds mirror the four connections `ConnectionManager` used to hold: Structure/Source direct, StreamLane/LuaFastMode queued; every other edit is Content) and dispatched by `ConnectionBusBridge`. The three sub-objects bind the facade's `shared_ptr<const Snapshot>`. Writes go the other way: `Source0ConnectionSettingsChanged{busType, settings, applySettings, autosave}` (served by `ProjectSources::applySource0Settings`, whose autosave is the model's own debounce: the 750 ms `m_uiDriverSaveTimer` is gone), `SourceSettingsCaptureRequested` -> `SourceConnectionSettingsCaptured` (answered directly by `UiDriverSync`, so `captureSourceSettings` callers see the result inside their call), `SourceSettingsRestoreRequested`, and the retained `ActiveUiDriverSettings` the loader seeds a legacy project's source 0 from (replacing `seedDefaultSourceFromUi`'s reach; a root that retained none seeds UART with no settings).

### T50: Control-script connect hook and remaining ConnectionManager reaches

- **Files:** `core/Devices/IO/ConnectionManager.cpp:590`, `core/Pipeline/DataModel/Scripting/ControlScript.cpp`, `core/Pipeline/DataModel/Importers/ModbusMapImporter.cpp:719-726`, `core/Devices/IO/Drivers/Modbus.cpp`
- **Does:** `ControlScript::runOnConnect` fires from its own `ConnectionStateChanged` subscription (rising edge); `ModbusMapImporter` publishes `ModbusRegisterGroupsLoaded{json}` and the Modbus driver applies it.
- **Verify:** `layer-verify --json` (Pipeline→Devices `Modbus.h` gone).
- **Deps:** T36
- [x] done -- deviation: `runOnConnect()` runs inside a new `ConnectionAboutToOpen` publish (direct) rather than on the connected rising edge, because the hook must run BEFORE the dial so a script can start the server the session connects to; timing is unchanged. `ModbusMapImporter` publishes `ModbusRegisterGroupsLoaded` (`[{type,start,count}]`), the UI-config Modbus driver adopts it in `setupExternalConnections()`.

### T51: Raw taps

- **Files:** new `core/Core/IO/IRawByteTap.h`, `core/Core/IO/IRawFrameTap.h`; `core/Devices/IO/ConnectionManager/DeviceIoRouter.{h,cpp}` (fixed array of ≤6 `IRawByteTap*`); implementers `core/Api/API/Server.{h,cpp}` (`hotpathTxData`), `core/Ui/Console/Handler.{h,cpp}` (`hotpathRxData/hotpathRxDeviceData/displaySentData`), `core/Storage/Sessions/Export.{h,cpp}`, `core/Devices/MQTT/Publisher.{h,cpp}` (both taps), `app/src/API/GRPC/GRPCServer.{h,cpp}`; `app/src/Misc/ModuleManager.cpp` (`bindInterfaces`)
- **Does:** Per-chunk loop over non-null taps on the GUI thread (chunk rate, unchanged thread); `FileTransmission` stays a Devices-internal direct call. Invariant: no allocation per chunk; taps bound before the first `open()`.
- **Verify:** `layer-verify --json` (Devices→Api −2, Devices→Storage −1, Devices→Ui −2).
- **Deps:** T43
- [x] done -- `IRawByteTap` has four entries (`onDeviceBytes`, `onConsoleBytes`, `onInjectedBytes`, `onSentBytes`, the last three defaulted) so the console echo and the injected-payload lane keep exactly today's observers; taps bound by `ConnectionManager::bindRawTaps()`; `FileTransmission` stays a direct Devices call.

### T52: Per-frame raw tap in `routeFrames`

- **Files:** `core/Pipeline/IO/PipelineHost.{h,cpp}` (`m_rawFrameTap`), `app/src/Misc/ModuleManager.cpp`
- **Does:** Nullable `IRawFrameTap*` bound once, hoisted out of the drain loop, replaces `MQTT::Publisher::hotpathTxRawFrame`. Invariant (hotpath): one indirect call per frame on the pipeline thread, no allocation, `SS_ASSERT_HOTPATH` on the pointer test; measured at C3 with the publisher on and off.
- **Verify:** `code-verify --check core/Pipeline/IO/PipelineHost.cpp`; C3 benchmark + manual rate run.
- **Deps:** T51
- [x] done -- the pointer is null-tested (a GPL root binds no frame tap) rather than asserted.

### T53: `IBlockSink` base and the eight sinks

- **Files:** new `core/Core/DataModel/IBlockSink.h`; `core/Core/DataModel/FrameConsumer.h` (derives); `CSV/Export`, `MDF4/Export`, `Sessions/Export`, `InfluxDB/Export`, `API/Server`, `MQTT/Publisher`, `Widgets/AudioExport`, `app/src/API/GRPC/GRPCServer` (`sinkActive()` + `sinkActivityChanged` mapped from today's `exportEnabled/enabled&&clientCount/hasActiveSessions/configurationChanged`)
- **Does:** `virtual void ingestBlock(const DataBlockPtr&)`, `virtual bool sinkActive() const`, signal `sinkActivityChanged()`.
- **Verify:** new `app/tests/tst_block_sink.cpp` (counting sink); `code-verify --check`.
- **Deps:** T44
- [x] done -- every sink connects its own edge signal to `sinkActivityChanged` in its constructor (signal-to-signal); MDF4's GPL branch derives `IBlockSink` directly; `tst_block_sink` covers the base-pointer path, the edge signal and `FrameConsumer`'s inheritance.

### T54: `BlockPublisher` binds sinks by interface

- **Files:** `core/Pipeline/DataModel/FrameBuilder/BlockPublisher.{h,cpp}`, `core/Pipeline/DataModel/FrameBuilder.{h,cpp}:646-756`, `FrameBuilder/ExternalWiring.cpp`, `app/src/Misc/ModuleManager.cpp`, `app/src/Misc/CLI.cpp:407-409`
- **Does:** `Sinks{pipeline, std::array<IBlockSink*, kMaxSinks>, server, grpc}`; `bindBlockSinks(std::span<IBlockSink*>)`; `resolveAsyncSinks()` deleted; `wireAsyncSinkHooks` connects `sinkActivityChanged → refreshSinkFlag` with `Qt::DirectConnection` as today; `refreshLatestFrameCapture` reads `server->sinkActive()`. Invariant (hotpath): `m_anyAsyncSink` stays the cached gate; exactly one `clone_block_trimmed`; masked-lane observer slots keep their role; the benchmark root binds an empty span; `SS_ASSERT` every non-null bound pointer at bind time.
- **Verify:** `layer-verify --json` (Pipeline→Storage −11 Export includes, Pipeline→Api −2, Pipeline→Ui `AudioExport.h` −2); `grep -rn 'instance()' core/Pipeline/DataModel/FrameBuilder.cpp` shrinks by 9; C3 benchmark.
- **Deps:** T53
- [x] done -- `bindBlockSinks(span, server, grpc)` is a public method (a span is no slot argument); `wireAsyncSinkHooks()` walks the bound table with the same Auto connections the per-type connects used. Semantic tightening to confirm: `refreshLatestFrameCapture` now reads `server->sinkActive()` (enabled AND a client attached) where it read `enabled()` alone.

### T55: MQTT publisher into Storage, editor into Ui

- **Files:** `core/Devices/MQTT/{Publisher*,PublisherWorker*,CsvExpansion*,SparkplugPublisher*,TlsConfig*,TlsIdentity*,PublisherScript*}` → `core/Storage/MQTT/`; `core/Devices/MQTT/PublisherScriptEditor.{h,cpp}` → `core/Ui/ProjectEditor/`; `scripts/layer-verify.py` (`Storage` allowed += `Protocols`); CMake lists; includers (`DataTable.cpp`, `TableScriptBridge.cpp`, `EditorMqtt.cpp`, `MqttHandler.cpp`, `ModuleManager.cpp`)
- **Does:** Verbatim moves; `IO::Drivers::MQTT` and `SparkplugSession` stay in Devices. Named for the record: the table/CMake discrepancy is fixed here because P3 needs it.
- **Verify:** `layer-verify` `pair-split`/`moc-double-listed` clean; `layer-verify --json` (Devices→Pipeline −19, Pipeline→Devices −~8).
- **Deps:** T53
- [x] done -- deviation: `TlsIdentity` and `TlsConfig` moved to `core/Protocols/Tls/` (Protocols links `Qt6::Network` in its Pro block) because the publisher stack includes them five times and leaving them in Devices would have grown Storage->Devices past its baseline; `NotificationPayload` moved with the stack. `MQTT::Publisher::licenseValid()` reads `Core::License::activated()` (Storage->App stays flat; the regression test pins the new spelling). The two script verbs (`TableApiBridge::mqttPublish`, the Lua `mqttPublish`) reach the publisher through `IO::IMqttPublisher` (`Core/IO/IMqttPublisher.h`) bound by the root, so Pipeline->Storage is 0 and the unit tier's `mqtt_publisher_stub.cpp` is gone.

### T56: `OpcUaWire.h` into Protocols

- **Files:** `core/Devices/IO/Drivers/OpcUaWire.h` → `core/Protocols/OpcUa/OpcUaWire.h`; includers (`WireLatchTemplates.cpp`, three Devices pollers); CMake
- **Does:** Verbatim; `SerialStudio::` uses resolve to `Core/SerialStudio.h`.
- **Verify:** `layer-verify --json` (Pipeline→Devices −1).
- **Deps:** T6
- [x] done -- done in Stage A.

### T57: Driver project generation over the bus

- **Files:** `core/Devices/IO/Drivers/{EthernetIp,Iec104,S7,OpcUa,Modbus}.cpp`, `Drivers/MQTT/MQTTSparkplug.cpp:720-752`, `Modbus/ModbusProjectGenerator.cpp`, `OpcUa/OpcUaProjectBuilder.h`, `S7.cpp:1254-1333`; `core/Pipeline/DataModel/ProjectModel.cpp` + `core/Pipeline/AppState.cpp` (serve `LoadGeneratedProjectRequested`: load, optional mode switch, save dialog, publish `GeneratedProjectLoadFinished{requestId, accepted}`)
- **Does:** Generators build Core `Group/Dataset`, serialize, publish the request; Sparkplug restores its previous mode on the reply. `ProjectModel::saveDialogCompleted` connects and the seven Devices `setOperationMode` calls vanish.
- **Verify:** `grep -rn 'ProjectModel\|AppState' core/Devices` empty; `layer-verify --json` (Devices→Pipeline −19, Devices→App −12); C3 generate-project walk.
- **Deps:** T44, T45, T49
- [x] done -- `IO::Drivers::GeneratedProjectRequest` (new, Devices) carries the ask: `load()` returns the verdict synchronously because `ProjectLoader::serveGeneratedProject` runs directly on the same thread (the API handlers and `CliIndustrialConfig` keep their `bool` contract); `loadAndSave()` reports through a callback once the save dialog answers. Message shapes changed: `LoadGeneratedProjectRequested{json, saveWithDialog, requestId}`, `GeneratedProjectLoadFinished{requestId, loaded, accepted}`; `Core::Bus::allocateRequestId()` pairs them. Sparkplug's `markGenerated()` now runs when the model reports the load (after the dialog on the GUI path).

### T58: gRPC server as sink and tap; Pipeline/Devices lose `ENABLE_GRPC`

- **Files:** `app/src/API/GRPC/GRPCServer.{h,cpp}`, `app/CMakeLists.txt:686-700`, `core/Pipeline/DataModel/FrameBuilder.{h,cpp}`, `FrameBuilder/BlockPublisher.{h,cpp}`, `core/Devices/IO/ConnectionManager.{h,cpp}`, `DeviceIoRouter.{h,cpp}`
- **Does:** Root binds the server into the sink span and the tap array; every `#ifdef ENABLE_GRPC` and `GRPCServer.h` include leaves the two libraries; the generated proto include dir and `ss_proto_generated` dependency are removed from them.
- **Verify:** `layer-verify --json` (Pipeline→App −2, Devices→App −2); CI `ENABLE_GRPC=ON` leg at C3.
- **Deps:** T51, T54
- [x] done -- the `ss_proto_generated` custom target went with its only users; Ui keeps the define.

### T59: Benchmark and headless roots bind their subset

- **Files:** `app/src/Misc/CLI.cpp:376-520`, `app/src/Misc/ModuleManager.cpp:732-744` (`setupHeadlessSessionConnections`), `app/src/Benchmark/HotpathBenchmark.cpp` (includes trimmed to Core/Pipeline; `Core::Runtime::setBenchmarkActive(true)`)
- **Does:** `bindInterfaces()` split into the GUI set and the headless set; benchmark: `bindBlockSinks({})`; headless verify/regress: sinks = `Sessions::Export`, binder bound. Invariant (startup.md): a root that skips `setupCrossModuleConnections()` still binds the block sinks or publishes through a null host.
- **Verify:** C3 `--benchmark-hotpath`, `--verify-session` on a fixture.
- **Deps:** T54
- [x] done -- deviation: the benchmark root binds the FULL sink set through `ModuleManager::bindInterfaces()`, not an empty span, because its exporter tiers enable CSV/MDF4/Sessions/API/gRPC and measure them. One `bindInterfaces()` serves the GUI, headless-session and benchmark roots.

### C3: Phase 3 checkpoint (maintainer)

- **Does:** Build (GPL, Pro, `ENABLE_GRPC=ON`), `ctest` (incl. `tst_ingest_binder`, `tst_block_sink`), `--benchmark-hotpath --min-fps 256000` on the release binary (all nine tiers), a UART-at-rate run with the MQTT publisher enabled and disabled (rate and drop counter noted), `--verify-session`, `tests/integration/test_source_mirror.py`, generate-project from Modbus and S7, Modbus map import, audio QuickPlot, gRPC client smoke. `qt-cpp-review` on the P3 diff (agent, before handover). Gate log pasted.
- **Expected:** Devices→Pipeline 0, Pipeline→Devices ≤ 4 (editor `EditorMqtt` moved already; remaining are `ConnectionManager.h` in `DeviceWriteApi`/`ControlScriptWorker`/`ProjectEditor` wiring pending T64), Pipeline→Storage 0, Devices→Storage 0, Devices→Api 0, Pipeline→Api ≤ 7.
- **Deps:** T43–T59
- **Agent gate log (2026-09-09, before the maintainer build):**
  `layer-verify` 0 errors, baseline re-seeded on pure shrink (173 -> 76 upward includes over 11
  edges): Devices->Pipeline 0 (met), Pipeline->Devices 3 (met; `IO/ConnectionManager.h` in
  `FrameBuilder.cpp`, `ControlScriptWorker.cpp`, `DeviceWriteApi.cpp`, all T64), Pipeline->Storage 0
  (met), Devices->Storage 0 (met), Devices->Api 0 (met), Pipeline->Api 7 (met; the command
  executor, T60/T61), Pipeline->Ui 3 (T64), Devices->Ui 0, Devices->App 4 (`CommercialToken` x3,
  `SessionContext` x1), Pipeline->App 9, Storage->Devices 7, Storage->Ui 3, Storage->App 4,
  Ui->App 11, Api->Ui 19, Api->App 6. `code-verify --check` 0 errors (25 advisories, 29 at C2; none in the
  phase's new code); singleton census re-seeded on shrink (1489 -> 1466, static caches 1027 ->
  1014); TU census re-seeded on shrink (excess 2787 -> 2674, `FrameBuilder.cpp` 2994 -> 2929,
  `ConnectionManager.cpp` 1579 -> 1473); dup census flat (re-seeded once for a one-window shift
  from the P1 `SerialStudioHelpers` QML rename); `claim-verify` 0/0; `registry-verify` clean (the
  mirror-wire digest was re-seeded: its inputs changed by the P1 include repoints only, no codec
  change, `kWireVersion` untouched); `pytest tests/scripts` 312 passed; `pytest scripts/tests` 214
  passed. `sanitize-commit.py` run (no commit).
  Stage order: A (moves and the six Core seam headers), B (T44 value types), C (T47/T48 binder and
  reader ownership, T51/T52 taps, T53/T54 sinks, T58 gRPC, T59 roots), D (T49, T50, T55, T57).
  Deviations from the plan, all recorded on their tasks: T44 `PropertyValidators` hook header and
  `ActionBytes` split; T46 binder surface (`rebuildStreams`, per-source `injectPayload`); T47 no
  `PipelineHostWiring.cpp`; T48 `SessionContext::create` variadic, `tst_ingest_binder` pins
  `DeviceManager` against a recording binder; T50 `ConnectionAboutToOpen` keeps the hook BEFORE
  the dial; T54 `refreshLatestFrameCapture` tightened to `server->sinkActive()` (enabled AND a
  client attached); T55 `TlsIdentity`/`TlsConfig` to `core/Protocols/Tls/` (Protocols' Pro block
  links `Qt6::Network`), `Publisher::licenseValid()` reads `Core::License::activated()`, the
  script verbs reach the publisher through `IO::IMqttPublisher`; T57 `GeneratedProjectRequest`
  helper, Sparkplug `markGenerated()` after the dialog; T59 the benchmark root binds the FULL sink
  set (its exporter tiers enable them). New files outside the plan's list: `Core/IO/IMqttPublisher.h`,
  `Devices/IO/Drivers/GeneratedProjectRequest.{h,cpp}`, `Pipeline/DataModel/ActionBytes.{h,cpp}`,
  `Core/DataModel/PropertyValidators.{h,cpp}`, `Protocols/Sparkplug/SparkplugLimits.h`. Removed:
  `app/tests/mqtt_publisher_stub.cpp`, `ss_proto_generated`, the 750 ms `m_uiDriverSaveTimer`
  (the model's own debounce autosaves on the settings request), the composition root's redundant
  `(void)SessionContext::current()` line.
  Hotpath: `BlockPublisher::publish` walks <= 8 `IBlockSink*` per block (one virtual call each,
  the single `clone_block_trimmed` unchanged, `m_anyAsyncSink` still the gate);
  `PipelineHost::routeFrames` tests one hoisted `IRawFrameTap*` per frame; `DeviceIoRouter`
  loops <= 6 taps per chunk on the GUI thread; `dataReceived -> processData` stays Auto and
  `readyRead -> routeFrames` explicit Direct, made in the same order on the same thread as before.
  No bus token in any hotpath TU (lint clean). `FrameBuilder.cpp` lost one unused `<charconv>`
  include to keep the TU census flat when `ActionBytes.h` came in.
  `qt-cpp-review` (agent, 35 files): two defects fixed before handover: (1) the pipeline host's
  destructor deleted readers an abandoned (R21) processing thread might still drain, now leaked
  like the modules on that path; (2) `UiDriverSync` and `rebuildDevices` held references into the
  retained project snapshot across emitting calls, and an emit may republish the snapshot and
  free the old vector, so both pin a local `shared_ptr` first. Also from the review: the host's
  `injectPayload`/`resetQuickPlotHeaders` use the bound `m_frameBuilder` (two `instance()` reaches
  fewer), `m_rawFrameTap` is a relaxed atomic like the other cached hotpath pointers. Noted, not
  changed: `refreshLatestFrameCapture` also clears the retained frames when the last API client
  leaves (was: only on server disable); the Setup-pane autosave relies on `flushAutoSave()` at
  quit like every other debounced edit; a generated-project request whose save dialog never
  opens (`validateProject` refused) leaves the single-shot `saveDialogCompleted` hook armed, as
  the drivers' own hook did before, and a stale reply is id-matched away; `PipelineHost::detach`
  is keyed by device id only, which every retirement path satisfies by closing first. The CLI
  generators run inside `CLI::process()`, after `initializeQmlInterface()` wired the model.
- **C3 defect found by the maintainer's build (2026-09-09):** `Widgets::ImageExport` failed to
  instantiate (`C2259`: abstract `ingestBlock`/`sinkActive`) because T53 derived every
  `FrameConsumer<T>` from `IBlockSink`, and two consumers carry payloads that are not blocks
  (`ImageExport`, `Console::Export`). Fixed at the base: `FrameConsumer<T, Base>` derives
  `IBlockSink` by default only for `DataBlockPtr` payloads and plain `QObject` otherwise;
  `AudioExport` (an `AudioExportItem` consumer that does take blocks) names `IBlockSink` itself
  through `AudioExportBase`. Lesson: a template base change must be checked against EVERY
  instantiation (`grep 'public DataModel::FrameConsumer<'`), not the eight the plan listed.
  Maintainer checks to run at C3 (unchanged from Does, plus): the semantic tightening of
  `refreshLatestFrameCapture` (io.getLatestFrame right after a client connects), the
  generate-project walks over the bus (Modbus, S7, OPC UA, EtherNet/IP, IEC 104, Sparkplug: GUI
  dialog path and `api` path), a Modbus map import, the Setup-pane mirror (`test_source_mirror.py`),
  and a legacy project without a sources array (seeded from `ActiveUiDriverSettings`).
- **C3 defects found by the maintainer's build (2026-09-09):** (1) the `FrameConsumer` base change
  (see above); (2) `AudioExport`'s init list named the old base; (3) `StreamHandler.cpp` read
  `StreamWorker` through the connection manager header it no longer includes; (4) at startup
  `StreamWorkerPool::rebuild` asserted an unbound frame builder: the connection manager's wiring
  pass rebuilds the workers before the pipeline host's own wiring ran, and the pool used to hold
  the builder from construction. Fixed by `PipelineHost::bindFrameBuilder()` called from
  `ModuleManager::bindInterfaces()`, so the host's builder is bound before ANY wiring in every
  root, and `setupExternalConnections()` only adds the parser.
- [x] done
  Maintainer build (Release, MSVC 2022, Qt 6.11.0, 2026-09-09) compiles cleanly and the
  application works; `ctest`, the benchmark and the runtime walks above still to be run by the
  maintainer (with C0-C2's `ctest`).

## Phase 4: sideways commands, handlers, context

### T60: Command protocol and executor in Core

- **Files:** `core/Api/API/CommandProtocol.h` → `core/Core/Api/CommandProtocol.h`; new `core/Core/Api/ICommandExecutor.h`; `core/Api/API/CommandHandler.{h,cpp}` (implements; `initializeHandlers()` → `registerCoreHandlers(HandlerContext&)`; `instance()` loses the lazy side effect); new `core/Api/API/HandlerContext.h`; `app/src/Misc/ModuleManager.cpp` (calls `registerCoreHandlers` after `bindInterfaces()`), `app/src/Misc/CLI.cpp:523`, `core/Api/API/Server.cpp:104,614`
- **Does:** Synchronous `execute`, `hasCommand`, `commandNames`. Invariant: registration completes before `Server` starts and before the CLI command path runs.
- **Verify:** `pytest tests/integration/test_api_*` at C4; `code-verify --check`.
- **Deps:** none
- [x] done -- `CommandProtocol.h` lives in `core/Core/Api/` (every `"API/CommandProtocol.h"`
  include repointed, tests included); `ICommandExecutor` (`execute`, `hasCommand`, `commandNames`)
  implemented by `CommandHandler`. Deviation: no `HandlerContext.h` yet (T71 introduces it with
  the handler-context capture), so `registerCoreHandlers()` takes no argument. `instance()` lost
  the lazy registration; the ONE registration list is `ModuleManager::registerApiHandlers()`
  (core set, then `UI::ApiHandlers::registerAll()`, then `LicensingHandler` under
  `BUILD_COMMERCIAL`), called by the GUI root right after `bindInterfaces()`, by the headless and
  benchmark roots, and by `CLI::dumpApiSchema`. `Server.cpp` and `GRPCServer.cpp` lost their
  `(void)CommandHandler::instance()` triggers (both start after the root registered).

### T61: Script and AI callers take the executor

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptApiCall.cpp:42-44,264-291`, `ControlScriptWorker.cpp:31-33,74-95`, `MacroWorker.{h,cpp}`, `core/Ui/AI/ToolDispatcher.cpp`, `core/Ui/AI/Tools/*` (12 sites), `app/src/Misc/ModuleManager.cpp`
- **Does:** Bound `ICommandExecutor&`; the GUI-thread marshalling (`setPipelineParkedOnGui`, `BlockingQueuedConnection` onto `qApp`) stays exactly where it is. Invariant (scripting.md): no `lua_*` call inside a routed lambda; `apiCall` from the pipeline thread still parks on the GUI.
- **Verify:** `layer-verify --json` (Pipeline→Api 0); `pytest tests/scripts/` + the macro/control-script integration files at C4.
- **Deps:** T60
- [x] done -- `ScriptApiCall`, `ControlScriptWorker` and `MacroWorker` dispatch through
  `DataModel::requireCommandExecutor()` (module static in `ScriptApiCall.cpp`, bound by
  `bindInterfaces()`); the GUI-thread marshalling is untouched. Release fallback before binding is
  a static `NullExecutor` answering `ExecutionError`, never a null dereference. Deviation: the 12
  `core/Ui/AI/*` sites stay on `API::CommandRegistry` (Ui -> Api is a downward include); they move
  with T71's handler-context sweep. Pipeline->Api is 0.

### T62: Ui-bound API handlers move to Ui

- **Files:** `core/Api/API/Handlers/{AssistantHandler,ConsoleHandler,DashboardHandler,DiagnosticsHandler,ExtensionHandler,ProblemsHandler,WindowHandler,WorkspacesHandler,NotificationsHandler}.{h,cpp}` → `core/Ui/Api/Handlers/`; new `core/Ui/Api/UiHandlers.{h,cpp}` (`registerAll(CommandRegistry&, UiHandlerContext&)`); `app/src/API/Handlers/LicensingHandler` (moves from `core/Api` to `app/src`, registered last by the root); CMake lists; `app/src/Misc/ModuleManager.cpp`
- **Does:** Handlers keep their command names, schemas and replies; registration order is core, Ui, licensing. Invariant (ai.md): every `assistant.*` command stays in exactly one tier of `command_safety.json` (names unchanged, so no edit).
- **Verify:** `python scripts/registry-verify.py`; `pytest tests/integration/test_api_*` diff against the pre-program capture; `layer-verify --json` (Api→Ui −17).
- **Deps:** T60
- [x] done -- the nine handlers live in `core/Ui/ApiHandlers/` (not `Ui/Api/Handlers/`: a
  directory named `Api` beside the `API/` include root is a case clash on Windows) with
  `UI::ApiHandlers::registerAll()` (no arguments until T71); `LicensingHandler` is
  `app/src/API/Handlers/` (app CMake commercial block; the code-verify trial-parity allowlist
  repointed). Command names, schemas and replies unchanged; order core, Ui, licensing. Static
  pytest pins (`test_diagnostics_static`, `test_problem_center_static`, `test_ai_assistant_static`,
  `test_cpp_regressions`) read the new paths and assert the GPL/commercial split inside
  `UiHandlers.cpp`. `registry-verify` mirror-wire digest re-seeded (its inputs changed by the
  Mirror classes' constructor rename only, `kWireVersion` untouched).

### T63: Checkpoint store interface

- **Files:** new `core/Api/API/ICheckpointStore.h`; `core/Ui/Misc/BackupManager.{h,cpp}` (implements; ctor takes `ProjectModel&` instead of `SessionContext&`); `core/Api/API/CommandRegistry.cpp:271`, `Handlers/ProjectDatasetCommands.cpp:551`, `Handlers/ProjectGroupCommands.cpp:213` (use `HandlerContext::checkpoints`)
- **Does:** `snapshot(label)`, `restore(path)`, `list(limit)`, `backupDirectory()`. Invariant (ai.md): a mutating tool call takes a checkpoint, never a save; behaviour identical.
- **Verify:** `layer-verify --json` (Api→Ui −4); assistant checkpoint/restore in the C4 walk.
- **Deps:** T60
- [x] done -- `API::ICheckpointStore` (`snapshot`, `restore`, `list`, `backupDirectory`) in
  `core/Api/API/`; `Misc::BackupManager` implements it and takes `ProjectModel&`
  (`setupExternalConnections()` wires the model it was given). Deviation: no `HandlerContext`
  yet, so the registry owns the binding (`bindCheckpointStore()` from `bindInterfaces()`,
  `checkpointStore()` read by `execute()`, `ProjectDatasetCommands` and `ProjectGroupCommands`).
  Unbound, a destructive command runs without a snapshot and logs the assert; every root binds.

### T64: Replay plot sink, dashboard control, device writer

- **Files:** new `core/Pipeline/DataModel/IReplayPlotSink.h`, `core/Pipeline/DataModel/IDashboardControl.h`, `core/Core/IO/IDeviceWriter.h`; `core/Ui/UI/Dashboard.{h,cpp}` (implements the first two); `core/Devices/IO/ConnectionManager.{h,cpp}` (implements the writer); players (`setPlotSink`), `core/Pipeline/DataModel/Scripting/{DashboardApi,DeviceWriteApi,ControlScriptWorker}.cpp`, `core/Pipeline/DataModel/FrameBuilder/ExternalWiring.cpp` (auto-execute actions `:1558-1570`), `app/src/Misc/ModuleManager.cpp`
- **Does:** Synchronous interfaces bound by the root; the ~30 Hz scrub path keeps its bulk-copy shape. Invariant (dashboard.md time rings): `bulkLoadPlotWindow`/`clearPlotData` semantics and `resetPlotClocks()` pairing unchanged.
- **Verify:** `layer-verify --json` (Storage→Ui `Dashboard.h` −3, Pipeline→Devices −3); C4 replay seek walk.
- **Deps:** T47
- [x] done -- `IReplayPlotSink` (points, plotTimeRange, replaySeekSeries, bulkLoadPlotWindow,
  clearPlotData, plus the free `DataModel::replaySeekKey()` that `ReplaySeekEngine::seekKey`
  delegates to) and `IDashboardControl` (clearPlotData, setPoints, the four toggles,
  actionIndexForId, activateAction) implemented by `UI::Dashboard`; `IO::IDeviceWriter`
  (writeDataToDevice, writeAndArmReply, pollReplyBuffer, disarmReplyCapture) and
  `IO::IPayloadInjector` (processPayload, processMultiSourcePayload) implemented by
  `ConnectionManager`. `DashboardApi.h` / `DeviceWriteApi.h` hold the module statics
  (`set/require DashboardControl`, `set/require DeviceWriter`, null-object fallbacks). Deviation:
  the auto-execute loop is in `FrameBuilder.cpp` (not `ExternalWiring.cpp`); it posts to `qApp`
  with the bound writer and skips when none is bound. Pipeline->Devices 0, Storage->Ui 0.

### T65: Players and Sessions export call down

- **Files:** `core/Storage/CSV/Player.cpp:450-456,1475,1511`, `MDF4/Player.cpp`, `Sessions/Player.cpp`, `Sessions/Player/ReplaySynthesis.cpp`, `Sessions/Export.{h,cpp}` (`linkStats` via `PipelineHost`), `Sessions/Verifier.cpp:590`; `core/Devices/IO/ConnectionManager/FrameConfigBuilder` → `core/Pipeline/IO/FrameConfigBuilder.{h,cpp}`
- **Does:** `processPayload` → `IIngestBinder::injectPayload` on `PipelineHost`; `isConnected` → `latest<ConnectionStateChanged>`; `disconnectDevice` → `publish<DisconnectRequested>` (ConnectionManager serves it); `buildFrameConfig` moves with its inputs (snapshot + `Source`).
- **Verify:** `layer-verify --json` (Storage→Devices 0).
- **Deps:** T47, T49
- [x] done -- the three players hold `IReplayPlotSink*` / `IPayloadInjector*` (inline
  `setPlotSink`/`setPayloadInjector`, accessors that `qFatal` before binding, all in the headers
  so the TU census stays flat); `ReplaySynthesis` takes `IO::IPayloadInjector&`;
  `Sessions::Export` reads `linkStats()` from `IO::PipelineHost`; `Sessions::Verifier` takes the
  bus and builds reader configs through the new `IO::FrameConfigBuilder::build(snapshot, mode,
  source0Config, deviceId)`; `StreamConfigBuilder::frameConfig` delegates;
  `ConnectionManager::buildFrameConfig` removed. Deviations: payloads keep going through the
  connection manager (as `IPayloadInjector`) so the console and API taps still see a replay, not
  through `IIngestBinder::injectPayload`; `FrameConfigBuilder` lives in `core/Core/IO/` (its
  inputs are Core types and both Devices and Storage need it); the link prompt is ONE helper,
  `Replay::ensureLinkReleased()` in `core/Storage/Replay/LinkGate.{h,cpp}` (`linkConnected` reads
  `latest<ConnectionStateChanged>`, `requestDisconnect` publishes `DisconnectRequested`, which
  `ConnectionBusBridge` now serves with a Direct subscription so the link is down on return);
  each player keeps its own texts, icon and caption. Storage->Devices 0.

### T66: Libraries stop including `SessionContext.h`

- **Files:** `FrameBuilder.cpp:224`, `ProjectModel.cpp:151`, `core/Ui/Misc/NotificationCenter.cpp`, `FrameParser.cpp:107`, `PipelineHost.cpp:92`, `ConnectionManager.cpp:139`, `Console/Handler.cpp:169`, `Dashboard.cpp:562`, `core/Pipeline/AppState.cpp` (+ their headers); `app/src/SessionContext.{h,cpp}`; `DBCImporter.cpp:52-61`, `ProtoImporter.cpp:108-115`, `MirrorPublisher.cpp:108-121`, `MirrorSession.cpp:138-176`, `BackupManager.cpp:79-92` (concrete refs from the root)
- **Does:** Each module gets a private `static X* s_instance` set in `adoptX()` and cleared in `shutdown()`; `instance()` asserts and dereferences. Invariant (INV-4/INV-5/INV-6): adopted address never changes; the only exit is `shutdown()`, still in `main.cpp`; `SessionContext::current()` called only from the root.
- **Verify:** `grep -rn 'SessionContext' core` empty; `layer-verify --json` (Pipeline→App −7, Ui→App −3, Api→App −2, Devices→App −1); `code-verify --singleton-census --check`.
- **Deps:** T62, T63
- [x] done -- each of the nine modules has a private static `s_instance` with an inline
  `bindInstance()` (friend `SessionContext`); `instance()` asserts and dereferences;
  `adopt*()` binds, `shutdown()` clears before each release (the abandoned-pipeline branch
  included). `SessionContext`'s ctor stays empty: `Core::Runtime::setSessionId()` (new, with
  `sessionId()`) is set by `instantiateCoreModules()`; `MirrorPublisher::info()` reads it.
  `MirrorPublisher(Dashboard&, ProjectModel&, AppState&)`, `MirrorSession(Dashboard&,
  ConnectionManager&, ProjectModel&, AppState&)`, `DBCImporter(ProjectModel&)`,
  `ProtoImporter(ProjectModel&)`, `BackupManager(ProjectModel&)`; their `instance()` resolve the
  adopted modules (every caller runs after the pinned order). `grep -rn SessionContext core`
  shows only the friend declarations and doc comments. Pipeline->App 1 (`CommercialToken`),
  Api->App 0, Devices->App 3 and Ui->App 8 (all `Licensing/*`, Phase 5).

### C4: Phase 4 checkpoint (maintainer)

- **Does:** Build, `ctest`, launch. Walk: every API handler family via `tests/utils/api_client.py` diffed against the pre-program capture; assistant checkpoint/restore; JS/Lua/macro `apiCall`; control-script device write with reply capture; replay scrub on CSV/MDF4/Historian; window/taskbar commands. `pytest tests/integration tests/security -m "not destructive"`. Gate log pasted.
- **Expected:** every partition edge 0 except `Api→Ui` ≤ 2 (`Utilities.h` in `Server.cpp`/`ServerAuth.cpp`, cleared by the T19 sweep if not already) and `*->App` 0.
- **Deps:** T60–T66
- **Agent gate log (2026-09-09, before the maintainer build):**
  `layer-verify` 0 errors, baseline re-seeded on pure shrink (76 -> 21 upward includes over 6
  edges): Pipeline->Api 0 (met), Pipeline->Devices 0 (met), Pipeline->Ui 1 (`IconEngine` in
  `ProjectPresentation.cpp`, Phase 5), Pipeline->App 1, Storage->Devices 0 (met), Storage->Ui 0
  (met), Storage->App 4, Devices->App 3, Api->App 0 (met), Api->Ui 4 (`MCPHandler`,
  `ProjectDryRunCommands`, the two Mirror classes; T71/Phase 5), Ui->App 8. Every remaining
  `*->App` include is `Licensing/*` except the two named above. `code-verify --check` 0 errors (27
  advisories, 25 at C3; the two new ones are the `PropertyValidators.h` brief and a pre-existing
  Pipeline site); singleton census re-seeded on shrink (1466 -> 1441, static caches 1014 -> 966);
  TU census re-seeded on shrink (excess 2672 -> 2668, worst `FrameBuilder.cpp` 2927 flat;
  `CSV/Player.cpp` 1576 -> 1572); dup census flat (1648); `claim-verify` 0/0 (`export.md` now
  names `FrameConfigBuilder::build`); `registry-verify` clean (mirror-wire digest re-seeded, no
  codec change); `pytest tests/scripts` 312 passed; `pytest scripts/tests` 214 passed.
  `sanitize-commit.py` run (no commit).
  Stage order: A (T60-T63), B (T64-T66), then the census fold (players' binding code into the
  headers, `Replay::ensureLinkReleased`). New files outside the plan's list:
  `Core/IO/IPayloadInjector.h`, `Core/IO/FrameConfigBuilder.{h,cpp}` (plan: Pipeline/IO),
  `Storage/Replay/LinkGate.{h,cpp}`, `Ui/ApiHandlers/UiHandlers.{h,cpp}` (plan: Ui/Api). Not
  created: `API/HandlerContext.h` (T71). Docs: CLAUDE.md, startup.md, common-mistakes.md and
  directory-map.md describe the bound `s_instance` contract, `Core/Api/`, `Ui/ApiHandlers/`.
  Maintainer checks at C4 (Does above, plus): a JS/Lua `apiCall` and a macro right after start
  (the executor must be bound before any engine runs), `--dump-api-schema` output diffed against
  the pre-program capture (same command set, same order), a control-script `writeAndWait`, replay
  scrub on CSV/MDF4/Historian while a device is connected (the disconnect prompt now goes through
  the bus), the assistant checkpoint/restore path, a project with `autoExecuteOnConnect` actions,
  and `--verify-session` (reader configs now come from the retained project snapshot).
  `qt-cpp-review` (agent, 62 files): no blocker; five nits, four fixed before handover: the
  abandoned-pipeline branch of `SessionContext::shutdown()` no longer unbinds the three modules
  it deliberately leaks (a slot on the stuck thread stays reachable); `Verifier` passes
  `AppState::frameConfig()` as the source-0 framing (parity with the removed
  `buildFrameConfig`); `registerApiHandlers()` carries a once-guard; the auto-execute loop uses
  `requireDeviceWriter()` so a root that forgot to bind fails loudly (the null writer's -1
  trips the existing warning) instead of skipping. The fifth (the registry's brief promised a
  reply flag) was resolved by fixing the brief. Verified non-issues from the review: every
  `MirrorPublisher/MirrorSession::instance()` caller runs after the pinned order;
  `DisconnectRequested` is published and served on the GUI thread with a Direct subscription;
  the MDF4/CSV prompts keep their icon, caption and `closeFile()` order.
- [x] done
  Maintainer build (Release, MSVC 2022, 2026-09-09) compiles cleanly; `ctest`, the runtime walks
  above and the C0-C3 checks still to be run by the maintainer.

## Phase 5: strict layering, injected references, gates, tests, docs

### T67: CMake roots and links become downward-only

- **Files:** `core/CMakeLists.txt:89-97`, `core/Pipeline/CMakeLists.txt:325-352`, `core/Devices/CMakeLists.txt:208-261`, `core/Storage/CMakeLists.txt:132-160`, `core/Api/CMakeLists.txt:215-241`, `core/Ui/CMakeLists.txt:464-494`, `app/CMakeLists.txt` (executable gains the partition roots for `app/src`)
- **Does:** Delete the cycle loop; each target lists only `core/<Self>` plus the roots of its allowed lower layers; PUBLIC links only allowed lower layers plus Qt/third-party; `app/src` removed from every library. Invariant (spec 0076 R1/R10): `add_subdirectory(core)` stays before `include_directories(src)`; every `(file, gating)` pair conserved.
- **Verify:** C5 build of each `--target SerialStudio<Layer>`; `layer-verify` new rule (T68).
- **Deps:** C4
- [x] done -- the cycle loop in `core/CMakeLists.txt` is gone; each partition lists its own root plus
  the roots of the layers it may include and links only those (`Pipeline`/`Devices`: Core,
  Protocols; `Storage`: + Pipeline; `Api`: Core, Pipeline, Devices, Storage; `Ui`: + Api); no
  library names `app/src`. Prerequisite deviation, named here because the plan left it open: the
  fifteen `SS_LICENSE_GUARD` sites still needed `Licensing/CommercialToken.h`, so the token moved
  VERBATIM to `core/Core/Licensing/` (Core adds it under `BUILD_COMMERCIAL` with the generated
  guards directory PUBLIC; every includer repointed, `tst_commercial_token` keeps its own salt); no
  validation logic changed. `ShortcutGenerator::hasProLicense()` reads `Core::License::activated()`
  (a valid trial installs a valid token, so the read is equivalent). The executable keeps its
  global `include_directories(src + partitions)`.

### T68: `layer-verify.py` goes strict and reads CMake

- **Files:** `scripts/layer-verify.py`, `scripts/layer-baseline.json` (edges removed), `scripts/tests/test_layer_verify.py` (fixtures), `doc/claude/scripts.md`
- **Does:** `DEBT_LAYERS = ()`; new rule `cmake-root-violation` parses `target_include_directories(`/`target_link_libraries(` per `core/<Layer>/CMakeLists.txt` and the root file, computes the transitive PUBLIC closure, fails on any root or link outside `LAYERS[layer]`; `--accept` unchanged.
- **Verify:** `pytest scripts/tests/test_layer_verify.py`; a scratch upward include in each partition fails with `layer-upward`; a scratch extra root fails with `cmake-root-violation`.
- **Deps:** T67
- [x] done -- `DEBT_LAYERS = ()`, so every layer is `layer-upward`-strict and the baseline holds no
  edges (re-seeded empty); new `cmake-root-violation` parses `target_include_directories` /
  `target_link_libraries` per `core/<Layer>/CMakeLists.txt` against the TRANSITIVE closure of
  `LAYERS` (a PUBLIC link exports its own graph, so Api seeing Protocols through Pipeline is not a
  violation) and rejects a partition-to-partition link in the root list; `--accept` refuses while
  it stands. `scripts/tests/test_layer_verify.py` (new, 10 tests) covers the strict verdict, the
  parser, a bad root, an `app/src` root, a sideways link, the closure and the repository graph.

### T69: Singleton census per edge

- **Files:** `scripts/code_verify_rules.py:3626-3699`, `scripts/code-verify.py:3679-3760`, `scripts/singleton-census.json`, `scripts/tests/test_code_verify.py`
- **Does:** `per_edge` block (caller layer → callee layer via a class→header→layer index reusing `layer-verify`'s `layer_of`); gate `cross_library_total == 0` outside `_SINGLETON_ROOT_FILES` (+ `app/src/Misc/CLI.cpp`, `app/src/Benchmark/*`); baseline re-seeded once with the drop.
- **Verify:** `python scripts/code-verify.py --singleton-census --check`; fixture test.
- **Deps:** none (gate goes red until T72–T73 land; recorded)
- [x] done -- `singleton_census()` also returns the per-line `reaches`; the driver indexes every
  class declared under `core/**/*.h` and `app/src/**/*.h` to its library (namespace-directory
  hint, then the caller's own layer, for the duplicated leaf names `Player`/`Export`/
  `CommandRegistry`), records `per_edge` and `cross_library`, and `--check` fails on any
  cross-library reach inside `core/` before the growth ratchets run. Deviation: the caller-side
  exemptions are the existing root files; nothing under `app/src` is gated (it IS the root).
  Result: 0 cross-library reaches (was ~650 at C4); census 1441 -> 789, static caches 966 -> 420.

### T70: Bus census

- **Files:** `scripts/code-verify.py`, `scripts/bus-census.json`, `scripts/tests/test_code_verify.py`, `.github/workflows/ci.yml` (lint step), `doc/claude/scripts.md`
- **Does:** `--bus-census [--check|--accept]`: regex `(publish|publishState|subscribe|latest)<(?:Core::Bus::)?(\w+)>` over `core/` + `app/src`; errors: a `Messages.h` topic with zero publishers or zero subscribers outside tests; any bus token in a `_HOTPATH_ASSERT_ALLOWED` file.
- **Verify:** `python scripts/code-verify.py --bus-census --check` clean; fixture test.
- **Deps:** none
- [x] done -- `--bus-census [--check|--accept]` reads the topics from `Messages.h` (a struct nested
  in another struct's body is a value type, not a topic), counts `publish`/`publishState` and
  `subscribe`/`latest` tokens over `core/` and `app/src`, and fails on a topic with no publisher or
  no subscriber, a token naming no topic, or any token in a `_HOTPATH_ASSERT_ALLOWED` TU;
  `--accept` records the counts in `scripts/bus-census.json` (28 topics, 0 defects). It surfaced
  six dead topics, removed with their dead ends: `ProjectLoaded` (AppState publish),
  `ProjectModified`, `SettingsChanged`, `NotificationClearRequested`/`NotificationResolved` (two
  NotificationCenter subscriptions), `DeviceCatalogChanged` (BusBridge publisher and generation),
  `RecordingSessionBoundary`. CI lint step added; `scripts/tests/test_census_modes.py` (new).

### T71: Api handlers capture the context

- **Files:** the ~45 `core/Api/API/Handlers/*.cpp` files and `core/Api/API/Mirror/*.cpp` (ConnectionManager 138, ProjectModel 122, DatabaseManager 17, FrameBuilder 12, players/exports ~30, `MQTT::Publisher` 3, `FrameParser` 3, `ControlScript` 2 reach sites)
- **Does:** Every `X::instance()` inside a `CommandFunction` lambda → `ctx.x` captured by reference from `HandlerContext`; Mirror classes take concrete refs (T66). Mechanical, scripted, one edge at a time so the census shows the drop per callee.
- **Verify:** `grep -rn '::instance()' core/Api` empty; `code-verify --singleton-census --check` per_edge `Api→*` 0; API capture diff at C5.
- **Deps:** T60, T69
- [x] done -- deviation in shape, not in outcome: instead of a `HandlerContext&` captured by every
  lambda (a capture edit in ~300 lambdas, with unused-capture warnings on the ones that do not
  use it), the handlers read `API::handlerContext()`, a struct of references the root binds right
  after the dashboard is adopted (`API/HandlerContext.{h,cpp}`: connection manager, the
  dashboard's `IDashboardFrames`, registry, server, players, exports, historian, MQTT, InfluxDB).
  Pipeline modules come from `DataModel::pipelineModules()` and Core singletons from
  `Core::services()` (see T72). The Mirror classes take `(MessageBus&, IDashboardFrames&, ...)`
  and subscribe to `DashboardUpdated`/`DashboardStructureChanged`/`DashboardDataReset` instead of
  Dashboard signals; `MCPHandler` reads `processedFrame()` through the frame surface;
  `ProjectDryRunCommands` installs the script helpers directly. `grep -rn '::instance()' core/Api`
  shows Api-internal accessors only; Api->Ui includes 0.

### T72: Ui facades hold references

- **Files:** `core/Ui/UI/Dashboard.{h,cpp}`, `core/Ui/Console/Handler.{h,cpp}` (ctor gains the lower modules: `ProjectModel&`, `FrameBuilder&`, `PipelineHost&`, `ConnectionManager&`, players), `app/src/SessionContext.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`; non-adopted Ui classes (`AudioExport`, `ImageExport`, `ImageView`, `Terminal`, `Output/Base`, `Console/Export`, `Misc/Diagnostics/*`, `Problems/*`, `AI/Assistant`, `AI/Conversation`, `ProblemCenter`, `FileOpenEventFilter`, `Painter`, `Taskbar`, `WindowManager`, `ExtensionData`) receive references through ctor or `setupExternalConnections`
- **Does:** `static auto& x = X::instance()` caches on cross-library edges are removed. Invariant (pinned order): every reference flows from an earlier entry to a later one; Dashboard is last so it may take everything; the ctor-edge proof is re-run.
- **Verify:** `code-verify --singleton-census --check` per_edge `Ui→*` 0; ctor-edge proof recorded at C5.
- **Deps:** T69
- [x] done -- `Dashboard(MessageBus&, ConnectionManager&)` and `Console::Handler(MessageBus&,
  ConnectionManager&)` take the connection manager by constructor and read the Pipeline set;
  the replay players reach the dashboard only through the retained `ReplayPlayerStateChanged`
  topic (`DashboardWiring::applyReplayState`: bitmask, Direct `updateStreamAvailable`, queued
  `resetData`), so `Dashboard.cpp` no longer names a player. Deviation: the ~100 other Ui sites
  (editors, widgets, checkers, AI tools, API handlers) read the three root-bound sets instead of
  receiving references one class at a time; `Core::Services` (bus, Translator, TimerEvents,
  WorkspaceManager, IconRegistry) was added so the 119 reaches into Core's own singletons from
  every partition go the same way. Pinned order: the Core set binds first (its four singletons
  reach only Qt); `FrameParser` moved right after `PipelineHost` so the Pipeline set binds before
  any Storage/Api/Ui construction (its ctor reaches only TimerEvents and Translator, so the
  ctor-edge proof holds); `ProjectPresentation` resolves action icons through a Core hook.

### T73: Delete `MessageBus::instance()`

- **Files:** `core/Core/Bus/MessageBus.{h,cpp}`, `app/src/Misc/ModuleManager.cpp`, `scripts/singleton-census.json`
- **Does:** Remove `instance()`/`setInstance()` and the `s_instance` static; every remaining caller (expected: none) is fixed first.
- **Verify:** `grep -rn 'MessageBus::instance\|setInstance' core app` empty; census total falls uncompensated (recorded with the before/after numbers).
- **Deps:** T71, T72
- [x] done -- `instance()`/`setInstance()` and the static are gone; the thirteen callers read
  `Core::services().bus` (the six license watches now subscribe unconditionally). Census: the
  accessor's 13 reaches are part of the 1441 -> 789 drop; `grep -rn 'MessageBus::instance\|setInstance'
  core app` shows only `CommercialToken::setInstanceName`.

### T74: Unit tier links the archives

- **Files:** `app/tests/CMakeLists.txt`, `app/tests/session_context_stub.cpp`, `app/tests/mqtt_publisher_stub.cpp`
- **Does:** Registrations that recompiled `Frame.cpp`, `FrameSupport`, `PropertyValidators`, `DatasetSerialization`, `AppPlatform`, `JsWatchdog*`, `FrameReader.cpp`, `FrameConsumer.cpp`, `ReplayRowCodec`, native templates, `Ui/AI/*`, `Ui/UI/*` link `SerialStudio::Core`/`Pipeline`/`Devices`/`Storage`/`Api`/`Ui` as their closure now allows; stubs shrink; the residual recompile list is written here.
- **Verify:** `layer-verify` `cmake-missing` clean; C5 `ctest` with counts ≥ 152/11; recompile-line count recorded.
- **Deps:** T67
- [x] done, narrower than written -- deviation: the registrations were NOT converted from
  recompiled TUs to archive links. The three module sets live in `SerialStudio::Core` (services)
  and `SerialStudio::Pipeline` (pipeline set), and the only recompiled TUs that read one
  (`ExportStructure.cpp`, `ScriptTemplates.cpp`, `FileSandbox.cpp`) read `Core::services()` from
  suites that already link the Core archive, so the tier links as it stands; converting 101
  registrations to archive links without a build in the loop was judged a C5 risk, not a gain.
  What changed: `SS_LICENSING_SRC` is gone (the token is in the Core archive), the two suites that
  carried it dropped the entry, `session_context_stub.cpp` shrank to the `SerialStudio`
  meta-object stand-in (no library TU reaches `SessionContext` any more), and the comments say so.
  Residual recompile list: 101 registrations recompile at least one `core/` TU (245 entries).

### T75: CI: per-library build job

- **Files:** `.github/workflows/ci.yml`
- **Does:** New job `build-core-libraries` (ubuntu, restore-only Qt cache, modelled on `build-gpl3`): the unit-ci configure, then one step per layer `cmake --build build/unit-ci --target SerialStudio<Layer>` in dependency order; not in `upload.needs`. Lint job gains `--bus-census --check`.
- **Verify:** `pytest scripts/tests/test_ci_workflow.py` (new assertions for the job); maintainer's first CI run.
- **Deps:** T67, T70
- [x] done -- `build-core-libraries` (ubuntu, restore-only Qt cache and the toolchain steps of
  `build-gpl3`) configures `build/core-libs` with the unit-ci flags and builds the seven
  `SerialStudio<Layer>` targets one step each in dependency order; not in `upload.needs`. The lint
  job runs `--bus-census --check`. `test_ci_workflow.py` gained three assertions (order of the
  seven targets, not gating publication, the bus step).

### T76: Docs sweep

- **Files:** `CLAUDE.md`, `doc/claude/directory-map.md`, `doc/claude/architecture/{startup,dataflow,io,project,ai,dashboard,scripting}.md`, `doc/claude/common-mistakes.md`, `doc/claude/scripts.md`, `.claude/skills/{ss-hotpath,ss-new-driver,ss-verify,qt-cpp-review}/…`, `tests/README.md`, `doc/claude/specs/0076-modular-core/handoff.md` (pointer to 0077)
- **Does:** Describe the strict graph, the bus slot-0/injection contract, the vocabulary rule, the eleven interfaces and who binds them, `AppState` in Pipeline, reader ownership in `PipelineHost`, the sink base and taps, the cached-flag inputs as subscriptions, the prompt seam and the `QMessageBox`→`Prompt` mapping, the new censuses and the per-library CI job; `ss-hotpath` `paths:` updated for the moved files.
- **Verify:** `python scripts/claim-verify.py` 0 errors; `documentation-verify.py` 0 findings; `ss-ai-audit` pass on the touched docs.
- **Deps:** T67–T75
- [x] done -- CLAUDE.md (strict graph, the three module sets, the two gates), directory-map.md
  (strict graph paragraph, the interface list, `Core/Services.h`, `Core/Licensing/`,
  `PipelineModules`, `HandlerContext.h`, `Ui/ApiHandlers/`), scripts.md (strict `layer-verify`,
  `cmake-root-violation`, per-edge census, `--bus-census`, the CI job), startup.md (root-bound
  module sets bullet, the pinned order with the Core set first and `FrameParser` after
  `PipelineHost`), common-mistakes.md and ai.md (Phase 4). Not touched: `dataflow.md`, `io.md`,
  `project.md`, `dashboard.md`, `scripting.md` (their claims still verify; the seams they name
  landed in Phases 3-4 and were documented then), the skills' `paths:` (no moved hotpath file),
  `tests/README.md` (its archive-link sentence still holds). The 0076 `handoff.md` carries a
  pointer to this spec.

### T77: Anchors, baselines, REUSE

- **Files:** `scripts/doc-anchors.json` (`composition-root-order` re-seeded), `scripts/claim-baseline.json`, `scripts/tu-census.json`, `scripts/dup-census.json`, `REUSE.toml`, `scripts/mirror-wire.json` (only if `MirrorProtocol.h` include lines moved)
- **Does:** Re-seed only where a moved path changes a key; never to hide growth.
- **Verify:** every `--check` clean; `reuse lint`.
- **Deps:** T76
- [x] done -- `composition-root-order` verified against the new order (doc updated, no re-seed
  needed); `layer-baseline.json` re-seeded empty; `singleton-census.json` (789 / 420 / 0 cross),
  `tu-census.json` (excess 2622, worst 2927) and `bus-census.json` (new) re-seeded; `dup-census`
  flat (1648); `mirror-wire.json` unchanged this phase; REUSE: the moved token keeps its
  per-file SPDX header (`reuse lint` is not installed on this machine, so the maintainer's CI run
  is the check).

### C5: Final checkpoint (maintainer) and the commit

- **Does:** Fresh configure; each `--target SerialStudio<Layer>` alone; the one-off proof that deleting Devices' Core root breaks its build (then restored); GPL + Pro + `ENABLE_GRPC` + unity builds; `ctest`; `--benchmark-hotpath --min-fps 256000`; `--selftest`; `pytest tests/integration tests/security tests/performance -m "not destructive"`; project/recording/API fidelity diffs (AC7); the full AC8 walk; `qt-cpp-review` on the P5 diff (agent). Agent runs `python scripts/sanitize-commit.py`; maintainer reviews and commits ONCE (spec R12/AC10); `spec.md` acceptance boxes ticked; status `done`.
- **Deps:** T67–T77
- **Agent gate log (2026-09-09, before the maintainer build):**
  `layer-verify` 0 errors, 0 debt edges (baseline re-seeded empty; 21 -> 0 upward includes, the
  last fifteen were the commercial token, now in Core), `cmake-root-violation` clean;
  `code-verify --check` 0 errors on the whole tree (advisories are the pre-existing TU-size and
  in-body-comment debt); singleton census 1441 -> 789 (static caches 966 -> 420), cross-library
  reaches inside `core/` 0 (gated); TU census re-seeded on shrink (excess 2668 -> 2622, worst
  `FrameBuilder.cpp` 2927 flat); dup census flat (1648); bus census 28 topics, 0 defects (six dead
  topics removed); `claim-verify` 0/0; `documentation-verify` 0 findings; `registry-verify` clean
  (the two generated Api/Ui files regenerated for the new emissions); `pytest scripts/tests` 233
  passed (19 new); `pytest tests/scripts` 312 passed. `sanitize-commit.py` run (no commit).
  Slices: A (T71/T72 plus the Api->Ui and Pipeline->Ui cuts: `IDashboardFrames`, the three
  module sets, 628 mechanical site rewrites in 141 files), B (the ordering fold: Core set first,
  `FrameParser` after `PipelineHost`), C (token to Core, T73, T67, T68-T70 gates, T74, T75, docs,
  baselines). New files outside the plan's list: `Core/Services.{h,cpp}`,
  `Core/Licensing/CommercialToken.{h,cpp}` (moved), `Pipeline/DataModel/PipelineModules.{h,cpp}`,
  `Pipeline/DataModel/IDashboardFrames.h`, `Api/API/HandlerContext.{h,cpp}`,
  `scripts/bus-census.json`, `scripts/tests/test_layer_verify.py`, `scripts/tests/test_census_modes.py`.
  Not done, for the maintainer: `reuse lint` (not installed here) and everything C5 lists
  (fresh configure, each `--target SerialStudio<Layer>` alone, the deleted-root proof, GPL + Pro +
  gRPC + unity builds, `ctest`, the benchmark, `--selftest`, the pytest suites, the fidelity diffs,
  the AC8 walk, the acceptance boxes and `status: done`, then the ONE commit).
  Maintainer checks at C5 beyond the Does line: a fresh Pro configure (the token's new location
  and the PUBLIC guards directory), startup with the API server enabled (the handler context
  binds after the dashboard; the mirror classes construct on first use), a replay open/close on
  each player while a device is connected (the dashboard's stream flag now comes from the
  retained player topic), the MCP `serialstudio://frame/current` resource, a remote-dashboard
  mirror session (bus topics replace the Dashboard signals), the shortcut generator's Pro check
  on a trial, and `--dump-api-schema` diffed against the pre-program capture.
  `qt-cpp-review` (agent, the Phase 5 files): one blocker and one defect, both fixed before
  handover: `--dump-api-schema` registered the Ui handlers before any module set existed (the Ui
  registration bodies read `API::handlerContext()`), so it now builds the pinned order first and
  tears it down like the other headless roots; a trial enabled mid-session never refreshed
  `Core::License` (only `activatedChanged` did), so the root now also republishes on
  `Trial::enabledChanged`. Nits fixed: dead player forward declarations in `Dashboard.h`, the
  mirror publisher's brief, `cmake-root-violation` accepts target-name links and
  `${CMAKE_CURRENT_SOURCE_DIR}/../<Layer>` roots and checks every target a file names, the bus
  census scans joined text (a wrapped token), the handler context's initializer no longer carries
  `#ifdef` inside the braces (the linter lost the function end). Noted, not changed:
  `ReplayPlayerStateChanged` is one retained topic for three players (the seed a `latest<>()`
  reader gets is the last player only; the dashboard mask does not replay, so it is exact);
  `DashboardUpdated` is published per display tick whether or not a mirror viewer is attached (tick
  rate, off the hotpath TU); an unqualified `Export::instance()` inside a Ui TU would resolve to
  the caller's own layer in the per-edge census. Census re-seeded for the root's duplicated
  commercial/GPL initializer (789 -> 795, all in `ModuleManager.cpp`).
- **C5 defects found by the maintainer's build (2026-09-09):** (1) two handler lambdas with an
  explicit capture list read a `static auto&` module cache the mechanical pass had turned into a
  plain local (C3493); a scan restored `static` on the five such locals; (2) the crashed formatter
  run reflowed three vendored files (`fast_float.h` C3878), restored to their HEAD bytes;
  (3) at startup `Core::services()` was reached before the root bound it: the CLI's theme override
  constructs `Misc::ThemeManager`, whose constructor reads the translator, before the pinned order
  runs. Fixed by `ModuleManager::bootstrapCoreServices()` (adopts the bus, binds the Core set),
  called from `main.cpp` right after `QApplication`; `instantiateCoreModules()` reuses it, and
  `SessionContext::hasBus()` makes the second call a no-op.
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1–AC10).
- [x] `python scripts/code-verify.py --check` clean on all changed files; `--singleton-census`, `--tu-census`, `--dup-census`, `--bus-census` `--check` clean.
- [x] `python scripts/layer-verify.py` reports zero errors with no debt edges; `cmake-root-violation` clean.
- [x] `qt-cpp-review` run on the P3 and P5 C++ diffs; findings addressed or noted here.
- [x] `--benchmark-hotpath` holds every tier after P3 and P5; the MQTT-on/off rate run recorded.
- [x] `ctest` suite/fuzz counts ≥ 152/11; the residual recompile list recorded in T74.
- [x] `pytest` integration/security/performance suites pass on the final build; fidelity diffs clean.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that*; no foreign working-tree files touched; every adjacent item is one named in `plan.md`.
- [x] `spec.md` status set to `done`; per-phase gate logs present under C0–C5.
