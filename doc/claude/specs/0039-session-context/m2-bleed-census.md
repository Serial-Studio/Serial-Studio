---
spec: 0039-session-context
phase: verification (milestone M2, task M2-T4)
status: recorded
updated: 2026-07-25
---

# Second-instance bleed census — the eight session subsystems

M2 makes a second `SessionContext` *ownable*; it does not make one *safe*. Class-level state —
file-scope statics, function-local statics inside member functions, `qApp` hooks, and registrations
into process-global registries — is shared by every instance regardless of who owns the object. This
file records what a second instance of each of the eight would share or double-register, so M3's
plan starts from a list rather than from a survey.

Read-only. Nothing here is fixed by M2; the plan says so
([`m2-plan.md`](./m2-plan.md), "M2 versus M3", point 4).

Line numbers are as observed on 2026-07-25 on a tree with several parallel specs in flight.

## Verdict table

| Class | Second instance safe today? | The blocker, in one line |
|-------|-----------------------------|--------------------------|
| `DataModel::NotificationCenter` | **No** | Every Lua/JS `notify*` binding caches `instance()`, and a second one shows a second tray icon |
| `Console::Handler` | **No** | Mutable file-scope timestamp cache + the process message handler is hard-wired to `instance()` |
| `DataModel::FrameParser` | **No** | Doubles an `aboutToQuit` connection and two `TimerEvents`/`Translator` connections |
| `AppState` | **Nearly** | No statics and no hooks of its own, but it holds `ProjectModel::instance()` as a reference member and shares two `QSettings` keys |
| `DataModel::ProjectModel` | **No** | `qmlRegisterType` already lets QML build extras; a second one adds a second autosave writer and a second file watcher on the same path |
| `DataModel::FrameBuilder` | **No** | `__ss` script global and nine Lua globals are last-writer-wins; `aboutToQuit` doubles; a `static bool warned` swallows the second instance's pool-exhaustion warning |
| `IO::ConnectionManager` | **No** | `aboutToQuit -> disconnectAllDevices` doubles, and QML holds pointers to the *first* instance's driver objects |
| `UI::Dashboard` | **No** | Registers its widgets into the process-global `UI::WidgetRegistry`; the three inline helpers in `Dashboard.h` bind to the first instance program-wide |

None of the eight is safe to duplicate today. The two that would fail *silently* rather than loudly
are `Console::Handler` (a shared mutable timestamp cache) and `FrameBuilder` (a latched warning
flag); the rest fail visibly — duplicated timers, duplicated device disconnects, a second tray icon.

## `DataModel::NotificationCenter`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `NotificationCenter.cpp:83` | `static NotificationCenter self;` |
| static | `NotificationCenter.cpp:46,48` | the two settings-key constants |
| fn-static | `NotificationCenter.cpp:502,517,532,547,562` | five `static auto& nc = NotificationCenter::instance();` caches — **inside its own Lua bindings**, so every script `notify*` call reaches instance 0 forever |
| class-static | `NotificationCenter.h:88-89` | `installScriptApi(lua_State*)` / `installScriptApi(QJSEngine*)` reach `instance()` internally |
| class-static | `NotificationCenter.h:141-144` | `kMaxHistory`, `kDedupWindowMs`, `kTrayTimeoutMs`, `kMaxDedupEntries` (constants; harmless) |
| qApp hook | `NotificationCenter.cpp:65-66` | `moveToThread(qApp->thread())` in the ctor |
| registry | `NotificationCenter.cpp:585-612` | eight Lua globals (`Info`, `Warning`, `Critical`, `notify`, `notifyInfo`, `notifyWarning`, `notifyCritical`, `notifyClear`) |
| registry | `NotificationCenter.cpp:623,624,627,628` | `nc = &instance()`, `setObjectOwnership(nc, CppOwnership)`, `js->newQObject(nc)`, `global.setProperty("__nc", ...)` |
| registry | `ModuleManager.cpp:804` | QML context property `Cpp_Notifications` |
| registry | `ModuleManager.cpp:213,223` | the process message handler posts into `instance()` |
| QSettings | `NotificationCenter.cpp:69,321` / `:70,340` | `NotificationCenter/systemNotifications`, `NotificationCenter/routeWarningsToNotifications` |
| thread/timer | `NotificationCenter.cpp:375,377` | owns a `QSystemTrayIcon` and shows it |

`installScriptApi(QJSEngine*)` has six call sites (`Painter.cpp:158`, `ScriptApiCall.cpp:577,604`,
`FrameBuilder.cpp:2500`, `LuaScriptEngine.cpp:184`, `DatasetTransformEditor.cpp:635,716`), each of
which would re-bridge instance 0. `__nc` itself appears only at `NotificationCenter.cpp:616,628`.

**M2 relevance:** the `CppOwnership` at `:624` is the one M2 must not break. The pointer is handed to
every Pro script engine; a script engine that outlived the session would hold a dangling wrapper.
Nothing does today, and INV-4 plus the `shutdown()` point keep it that way — but a wave that
introduced an earlier release of the notification slot would create exactly that dangling wrapper.

## `Console::Handler`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `Handler.cpp:145` | `static Handler singleton;` |
| fn-static (**mutable**) | `Handler.cpp:54,55` | `static qint64 s_lastMs = -1;` / `static QString s_cached;` — a shared per-second timestamp cache inside `cachedTimestampStr()` |
| fn-static (**mutable**) | `Handler.cpp:318` | `static QStringList list;` in `checksumMethods()` — lazily filled and `tr()`-translated once, so it also freezes the locale |
| fn-static | `Handler.cpp:437,452` | two `QRegularExpression` caches (immutable; harmless) |
| registry | `ModuleManager.cpp:797` | QML context property `Cpp_Console_Handler` |
| registry | `ModuleManager.cpp:193,204,205` | the process message handler uses `instance()` as both receiver and sink |
| QSettings | `Handler.cpp:97-111` (14 keys, read) and the matching setters | the whole `Console/*` group |
| timer | `Handler.cpp:129` | ctor connects `TimerEvents::instance()::uiTimeout`; a second instance adds a second per-tick slot |
| caches | — | 4 singleton caches of other classes (`CommonFonts` x3, `TimerEvents` x1) |

**M2 relevance:** this is the class that proves the session/application split survives ownership —
its ctor pulls `CommonFonts`, which is *not* adopted. INV-3 (message handler after the handler
exists) and the "no warning after `shutdown()`" ordering are both exercised here.

## `DataModel::FrameParser`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `Scripting/FrameParser.cpp:79` | `static FrameParser singleton;` |
| file static | — | none |
| fn-static | `Scripting/FrameParser.cpp:165` | `static constexpr DelimitedVariant kDelimited[]` (immutable) |
| class-static | `Scripting/FrameParser.h:69,70,73` | `defaultTemplateCode`, `nativeEquivalentForFile`, `fileForNativeTemplate` (pure) |
| **qApp hook** | `Scripting/FrameParser.cpp:63-64` | `connect(qApp, &QCoreApplication::aboutToQuit, ...)` clearing `m_engines` — a second instance double-registers |
| timer/translator | `Scripting/FrameParser.cpp:53,57` | `TimerEvents::timeout1Hz -> collectGarbage`, `Translator::languageChanged -> loadTemplateNames` |
| QSettings | — | none |
| caches | — | 9 (`TimerEvents` x1, `Translator` x1, `ProjectModel` x7) |

**M2 relevance:** Wave B2's ctor edge. `engineForSource(0)` reaches Lua engine construction, which
reaches `FrameBuilder::instance()` (`LuaScriptEngine.cpp:167`) — the first adoption whose ctor edge
becomes a live assertion against an earlier-pinned module.

## `AppState`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `AppState.cpp:58` | `static AppState singleton;` |
| file static / fn-static / class-static data | — | **none** |
| qApp hook | — | **none** |
| ctor reference member | `AppState.cpp:42` | `m_projectModel(DataModel::ProjectModel::instance())` — a second `AppState` still binds to project model 0 |
| registry | `ModuleManager.cpp:779` | QML context property `Cpp_AppState` |
| QSettings | `AppState.cpp:45,178,199` / `:133,205` | `operation_mode`, `project_file_path` |
| caches | — | 0 |

The cleanest of the eight: no class-level mutable state at all. Its whole bleed surface is the
ctor capture and two settings keys.

**M2 relevance:** the `main.cpp:155` pre-root reach (relocated by M2-T3) exists because
`setEphemeralSession` is the switch that suppresses those two `QSettings` writes, and it has to be
set before `restoreLastProject()` runs.

## `DataModel::ProjectModel` (+ 18 `Project/` TUs)

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `ProjectModel.cpp:199` | `static ProjectModel singleton;` |
| file statics | 43 sites across `ProjectModel.cpp` and `Project/*.cpp` | all are internal-linkage *helper functions* and `constexpr` limits (`ProjectHistory.cpp:29-32`), not mutable state |
| fn-static (**shared across TUs**) | `Project/ProjectModelShared.h:76` | `static const QRegularExpression kSuffixRe` inside an inline header function |
| fn-static (**shared across TUs**) | `Project/ProjectEditorShared.h:64` | `static auto& registry = Misc::IconRegistry::instance();` inside an inline header function |
| fn-static (**mutable**) | `Project/ProjectEditorCommit.cpp:484` | `static QStringList eolKeys;` — lazily filled from the *first* editor instance's `m_eolSequences` |
| fn-static | `Project/ProjectModelLoading.cpp:1082`, `ProjectEditorCommit.cpp:137,424` | immutable regex / key tables |
| class-static | `ProjectEditor.h:742` | `static const QHash<int, QByteArray> kNames` inside `roleNames()` |
| qApp hook | `Project/ProjectModelPersistence.cpp:157`, `ProjectModelLoading.cpp:226,451` | `new QFileDialog(qApp->activeWindow(), ...)` — parenting only, no registration |
| **registry** | `ModuleManager.cpp:565` | `qmlRegisterType<DataModel::ProjectModel>(...)` — **QML can already instantiate additional ProjectModel objects today** |
| registry | `ModuleManager.cpp:799` | QML context property `Cpp_JSON_ProjectModel` |
| QSettings | — | **none** (widget settings live in the project JSON) |
| thread/timer | `ProjectModel.cpp:98,102,106-110` | a 1.5 s autosave `QTimer` and a `QFileSystemWatcher` — a second instance means a second writer and a second OS watch on the same file |
| caches | — | 103 (19 in `ProjectModel.cpp`, 84 across `Project/`) |

**M2 relevance:** Wave C2, the highest-risk wave. Note the standing re-proof rule: nothing reachable
from the ctor closure (`newJsonFile`, `watchProjectFile`, `scheduleAutoSave`, `ControlScript::setCode`)
may name `SessionContext`.

## `DataModel::FrameBuilder`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `FrameBuilder.cpp:184` | `static FrameBuilder singleton;` |
| file statics | `FrameBuilder.cpp:1160,2334,2934-3158` | one formatter helper plus eleven `lua_CFunction` trampolines (internal linkage, stateless) |
| fn-static (**mutable**) | `FrameBuilder.cpp:269` | `static bool warned = false;` in `notePoolExhausted()` — once instance 0 warns, a second instance's slot-pool exhaustion is silent |
| class-static | `FrameBuilder.h:149-152,271-273,291` | watchdog / pool / span limits (constants) |
| class-static | `FrameBuilder.h:391` | `transformLuaWatchdogHook` installed as the Lua hook |
| **qApp hook** | `FrameBuilder.cpp:172-173` | `connect(qApp, &QCoreApplication::aboutToQuit, ...)` tearing down transform engines — doubles |
| registry | `FrameBuilder.cpp:3196-3228` | nine Lua globals (`tableGet`, `tableSet`, `tableHandle`, `tableHandleMany`, `tableGetH`, `tableSetH`, `datasetGetRaw`, `datasetGetFinal`, `mqttPublish`) whose upvalue is *this* instance's `m_tableStore` |
| registry | `FrameBuilder.cpp:3243,3246,3247` | `__ss` JS bridge — **last instance to inject wins the name** |
| registry | `FrameBuilder.cpp:2353` | nils `dofile`/`loadfile`/`load` in the sandbox |
| registry | `FrameBuilder.cpp:2504-2506` | Lua registry key `__ss_transform__` + `lua_sethook` |
| registry | `ModuleManager.cpp:803` | QML context property `Cpp_JSON_FrameBuilder` |
| QSettings / threads | — | none |
| caches | — | 42 |

**M2 relevance:** Wave D1, the hotpath head, and the first wave where `--benchmark-hotpath` blocks.
The slot pool moves from `.bss` to the heap with the object, which is the locality change the
benchmark is gating.

## `IO::ConnectionManager`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `IO/ConnectionManager.cpp:152` | `static ConnectionManager singleton;` |
| file statics / fn-statics / class-static data | — | **none** |
| **qApp hook** | `IO/ConnectionManager.cpp:116` | `connect(qApp, &QApplication::aboutToQuit, this, &ConnectionManager::disconnectAllDevices)` — doubles |
| registry | `ModuleManager.cpp:781,784,785,792` + the commercial block | eleven QML context properties expose the *first* instance's owned driver objects (`Cpp_IO_Serial`, `Cpp_IO_Network`, `Cpp_IO_Bluetooth_LE`, `Cpp_IO_Audio`, `Cpp_IO_CANBus`, `Cpp_IO_Modbus`, `Cpp_IO_USB`, `Cpp_IO_HID`, `Cpp_IO_Process`, `Cpp_IO_Mqtt`, `Cpp_IO_Manager`) |
| QSettings | `IO/ConnectionManager.cpp:920,1199,1321,1553` / `:924,925,1202,1600` | `IOManager/busType`, `IOManager/userBusType` |
| timer | `IO/ConnectionManager.cpp:98-113` | a 750 ms debounce timer whose lambda calls `saveJsonFile(false)` — two instances means two writers to the same project file |
| threads | — | none at class level; worker threads live per-device inside `DeviceManager`/`FrameReader` |
| caches | — | 44 |

**M2 relevance:** Wave D2 is where the teardown claim is proven. `~ConnectionManager` (and every
driver destructor it owns: thread joins, `libusb_exit`, `hid_exit`) moves from `__cxa_finalize` to
the live-`qApp` shutdown point — instance 3 of the crash class, fixed structurally. It also
interacts with `main.cpp:183`, which uses the manager *after* `~ModuleManager` and must stay before
`shutdown()`.

## `UI::Dashboard`

| Kind | Site | Finding |
|------|------|---------|
| Meyers | `UI/Dashboard.cpp:294` | `static Dashboard instance;` |
| file statics | `UI/Dashboard.cpp:53-65,71-127,2861` | plot/ring constants and internal-linkage helpers (stateless) |
| fn-static | `UI/Dashboard.cpp:707,729,775,788,801,814,828,842,856,870,884,907` | twelve `static const` empty sentinels returned by reference on the miss path — a second instance hands back the same shared objects (immutable, so benign) |
| **header inline statics** | `UI/Dashboard.h:588,594,600` | `GET_GROUP` / `GET_DATASET` / `VALIDATE_WIDGET` each hold `static auto& dashboard = UI::Dashboard::instance();` — one shared static per program, bound on first call |
| qApp hook | — | **none** |
| **registry** | `UI/Dashboard.cpp:2211-2258` | registers every group and dataset widget into the process-global `UI::WidgetRegistry` (`createWidget`, `updateWidget`, batch begin/end) |
| registry | `ModuleManager.cpp:789` | QML context property `Cpp_UI_Dashboard` |
| registry | `Scripting/DashboardApi.cpp:81,103,122,366-384` | seven Lua globals whose bodies cache `instance()` |
| QSettings | `UI/Dashboard.h:439` + `Dashboard.cpp:218-286` / `:979-1532` | eleven `Dashboard/*` keys |
| timer | `UI/Dashboard.cpp:244,3305-3308` | the `TimerEvents::uiTimeout` connection plus a full set of auto-firing action timers that write to the device |
| caches | — | 41 (38 in the `.cpp`, 3 in the header inlines) |

**M2 relevance:** Wave D3. The three header inline helpers are the point of the wave — proving INV-4
by *not* touching `Dashboard.h`. `Dashboard.h` must not appear in that wave's diff.

## What this means for M3

Four ordered consequences, in the order M3 will hit them:

1. **Script-API globals are process-wide and last-writer-wins.** `__ss`, `__nc`, and the ~26 Lua
   globals bind to whichever instance installed them last. A second session's scripts would drive
   the first session's data unless the bridges become per-engine-instance rather than per-name.
2. **Three `aboutToQuit` connections double** (`FrameParser`, `FrameBuilder`, `ConnectionManager`).
   Each is idempotent-ish in isolation, but a second registration means a second teardown pass over
   a *different* object, which is fine, and over shared statics, which is not.
3. **Two process-global registries take per-session content**: `UI::WidgetRegistry` (from
   `Dashboard::registerWidgets`) and the QML type registry (`qmlRegisterType<ProjectModel>`).
   Neither is keyed by session.
4. **Two mutable function-local statics are genuine silent bleed**: `Handler.cpp:54-55` (timestamp
   cache) and `FrameBuilder.cpp:269` (latched warning). Both would need to become members.

Everything else on this page is either immutable shared data (safe) or duplicated-but-visible
behavior (timers, tray icons, file watchers) that a second session would want scoped anyway.
