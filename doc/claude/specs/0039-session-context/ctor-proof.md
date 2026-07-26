---
spec: 0039-session-context
phase: verification
status: recorded
updated: 2026-07-25
---

# Ctor-edge proof re-run — spec 0039 (T3)

Spec 0001 pinned the construction order of the core singletons and proved that no module
constructs another one out of order. Spec 0039 publishes a `SessionContext` inside the same
composition root, so that proof has to be re-run rather than assumed. This file records the
five checks named in `plan.md`, the exact commands, and the results as observed on the
working tree of 2026-07-25.

Everything below was run from the repository root.

## Scope of the spec-0039 diff inside the composition root

```
git diff app/src/Misc/ModuleManager.cpp
```

Spec 0039 contributes three lines to `ModuleManager.cpp`:

| Line | Content |
|------|---------|
| 89 | `#include "SessionContext.h"` |
| 663-664 | two lines of `@brief` prose on `setupCrossModuleConnections()` |
| 669 | `(void)SessionContext::current();` |

`instantiateCoreModules()` gains **nothing** from this spec. The working tree does show two
added entries in that function — `Misc::ProblemCenter` and `Misc::ConnectionDiagnostics` at
lines 629-630 — but they belong to the parallel specs 0033 and 0035, carry their own
ctor-edge argument, and are outside this diff. The claim this file makes is therefore the
precise one: *spec 0039 adds zero entries to the pinned order and reorders none.*

## Check 1 — grep symmetry against the pinned order

```
# every (void)X::instance() inside instantiateCoreModules(), in source order,
# then a search for each class in the remainder of the file
```

The function force-constructs 31 classes (26 in a GPL build; five are
`BUILD_COMMERCIAL`-only):

```
 1 Misc::Translator              12 AppState                      23 Sessions::Player *
 2 Misc::TimerEvents             13 Licensing::MachineID *        24 Sessions::Export *
 3 Misc::CommonFonts             14 Licensing::LemonSqueezy *     25 Sessions::DatabaseManager *
 4 Misc::WorkspaceManager        15 Licensing::OfflineLicense *   26 MQTT::Publisher *
 5 DataModel::NotificationCenter 16 Licensing::Trial *            27 CSV::Export
 6 Misc::ProblemCenter           17 DataModel::FrameBuilder       28 MDF4::Export
 7 Misc::ConnectionDiagnostics   18 IO::ConnectionManager         29 Console::Export
 8 Misc::ThemeManager            19 Console::Handler              30 DataModel::FrameParser
 9 Misc::ExtensionManager        20 API::Server                   31 UI::Dashboard
10 DataModel::ControlScript      21 CSV::Player
11 DataModel::ProjectModel       22 MDF4::Player
```

`ProjectModel` (11) precedes `AppState` (12); `UI::Dashboard` is last. Both spec-0001
anchors hold.

Symmetry result: 30 of the 31 classes are named again below the function — in the wiring
body, in a `setupExternalConnections()` call, or in a context-property registration. The one
exception is `Licensing::MachineID`, which is constructed for its side effect (the activation
fingerprint) and is reached transitively rather than by name:

```
grep -rn "MachineID::instance()" app/src | grep -v ModuleManager.cpp
  app/src/Licensing/LemonSqueezy.cpp:80,245,553,740
  app/src/Licensing/OfflineLicense.cpp:61,82,118
  app/src/Licensing/MonotonicClock.cpp:61
  app/src/AI/KeyVault.cpp:43
```

This matches the spec-0001 table, where `MachineID` is pinned ahead of `LemonSqueezy`
precisely because `LemonSqueezy` forces it. Nothing about that changed here.

**Documentation drift observed, not fixed:** the pinned-order paragraph in
`doc/claude/architecture/startup.md` (lines 30-36) still omits `ProblemCenter`,
`ConnectionDiagnostics`, `OfflineLicense`, and `Trial`. Those entries came from specs 0033,
0035, and 0029; correcting that paragraph belongs to whichever spec owns it, not to 0039.

## Check 2 — INV-1: wiring before project restore

```
grep -n "restoreLastProject()" app/src/Misc/ModuleManager.cpp
  729:  appState->restoreLastProject();
```

Line 729 is the last statement of `setupCrossModuleConnections()`; every
`setupExternalConnections()` call in the function precedes it (lines 679-699), and the new
`(void)SessionContext::current();` at line 669 precedes all of them. INV-1 holds.

## Check 3 — INV-2: context properties after wiring, before the QML load

```
grep -n "setupCrossModuleConnections()\|registerCoreContextProperties\|registerCommercialContextProperties\|registerAppMetadataProperties\|registerImageProvidersAndLoadQml\|m_engine.load" app/src/Misc/ModuleManager.cpp
  592:  setupCrossModuleConnections();
  600:  registerCoreContextProperties(c);
  602:  registerCommercialContextProperties(c);
  604:  registerAppMetadataProperties(c, grpcAvailable);
  607:    registerImageProvidersAndLoadQml();
  897:  m_engine.load(QUrl("qrc:/serial-studio.com/gui/qml/main.qml"));
```

Order is wiring (592) → context properties (600-604) → QML load (607 → 897). The three pilot
context properties are unmoved: `Cpp_JSON_ProtoImporter` at 773, `Cpp_Misc_BackupManager` at
793, `Cpp_JSON_DBCImporter` at 813. INV-2 holds.

## Check 4 — INV-3: message handler after Console::Handler and NotificationCenter

```
grep -n "qInstallMessageHandler" app/src/Misc/ModuleManager.cpp
  360:  qInstallMessageHandler(nullptr);
  594:  qInstallMessageHandler(MessageHandler);
  595:  qAddPostRoutine([]() { qInstallMessageHandler(nullptr); });
```

Installation at 594 follows `setupCrossModuleConnections()` at 592, which force-constructs
`DataModel::NotificationCenter` (pinned entry 5) and `Console::Handler` (entry 19). INV-3
holds.

## Check 5 — the new out-edge check on SessionContext

The preservation argument in `plan.md` rests on the context constructing nothing. Two
mechanical checks:

**Empty constructor body.** `app/src/SessionContext.cpp:42`

```
SessionContext::SessionContext(int session_id) : m_sessionId(session_id) {}
```

No statement, no member initializer other than the `int`, and the destructor at line 47 is
likewise empty.

**No module-typed member.** The private section of `app/src/SessionContext.h` (lines 74-76)
declares `int m_sessionId;` and nothing else. The header names the eight session subsystems
only as forward declarations and as accessor return types, so no include of a module header
appears in the header at all — the eight includes live in the `.cpp`.

```
grep -nE "Sessions::|MQTT::|Licensing::|Modbus|CanBus|BUILD_COMMERCIAL" app/src/SessionContext.h app/src/SessionContext.cpp
  (no output)
```

The context therefore names no commercial symbol either, which is what lets the commercial
pilot compile out cleanly in a GPL build.

**Publication point is after the whole pinned order.** `(void)SessionContext::current();` sits
at line 669, immediately after `instantiateCoreModules()` returns at 668. Even if the context's
constructor later grew an edge, every module it could reach is already constructed at that
point, so no new construction edge can form at that call site.

**No pilot is in the pinned order, and none is reachable from ProjectModel's ctor closure.**

```
grep -n "BackupManager\|ProtoImporter\|DBCImporter" app/src/Misc/ModuleManager.cpp
  685:  Misc::BackupManager::instance().setupExternalConnections();
  773:  ...setContextProperty("Cpp_JSON_ProtoImporter", ...)
  793:  ...setContextProperty("Cpp_Misc_BackupManager", ...)
  813:  ...setContextProperty("Cpp_JSON_DBCImporter", ...)

grep -rn "SessionContext" app/src/DataModel/ProjectModel.cpp app/src/DataModel/Project/ app/src/main.cpp app/src/Misc/CLI.cpp
  (no output)
```

First construction of each pilot is at 685, 773, and 813 — all strictly after line 669. The
context is not named in `ProjectModel.cpp`, in any `Project/` translation unit, in `main.cpp`,
or in `CLI.cpp`, so the protected ctor closure is untouched.

## Verdict

The spec-0001 ctor-edge proof is **preserved, not re-derived**. Spec 0039 adds one object
whose constructor has zero out-edges, constructs it after the entire pinned order has run,
edits no entry of `instantiateCoreModules()`, converts no pinned module to constructor
injection, and leaves INV-1, INV-2, and INV-3 holding at their existing call sites.

Outstanding for the maintainer (the spec-0001 build gate, which no agent can run): launch in
ProjectFile, QuickPlot, and ConsoleOnly and confirm startup, project restore, a backup
snapshot on edit, a `.proto` import, and — on a commercial build — a `.dbc` import.
