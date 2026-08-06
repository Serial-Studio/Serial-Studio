---
spec: 0046-script-macros
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-05
---

# Plan 0046 — Script macros in the API Terminal

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this
> `approved`.

## Approach (one paragraph)

The API Terminal's right pane becomes a two-tab stack ("Terminal" / "Script"). The Script
tab hosts a new project-independent embedded code editor (`DataModel::MacroEditor`, a
sibling of `ControlScriptEditor` minus the ProjectModel coupling, with a JS/Lua highlighter
switch) under a small in-window toolbar (language combo + load / save / verify / run /
stop / clear). Execution goes through a new QML-instantiable `DataModel::MacroRunner`:
**JS macros run on a dedicated worker thread** via a new `MacroWorker` (sibling of
`ControlScriptWorker`, reusing `ControlApiMarshaller` verbatim; the worker carries its own
`MacroApiBridge` — same apiCall marshal over `BlockingQueuedConnection` and the SDK
prelude's control-mode fallbacks, but with stop-aware `delay()`/`call()`/`writeAndWait()`
(ControlApiBridge only honors the process-wide shutdown flag, so Stop couldn't break a
long `delay()`) and a `log()` hook that streams `print`/`console.log` to the scrollback),
so the UI stays live and Stop works; **Lua macros v1 run GUI-synchronous** with `ScriptApiCall::installAll`
and the existing `LUA_MASKCOUNT` + `QDeadlineTimer` hook as a generous safety bound. Fresh
engine per execute; Verify is compile-only in a throwaway engine (the `controlscript.dryRun`
pattern); output and errors flow back over queued signals into the shared terminal
scrollback, and Execute auto-switches to the Terminal tab so the output is visible.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/DataModel/Editors/MacroEditor.h/.cpp` | NEW — QML-embeddable editor: `QQuickPaintedItem` + offscreen `QCodeEditor`, the three hidden-widget invariants copied from `ControlScriptEditor` (syncWidgetPosition-first render, ShortcutOverride forwarding, completer-popup rerouting); plain `text` get/set, `language` property switching JS/Lua highlighter, `isModified`, no ProjectModel/ProjectEditor members |
| `app/src/DataModel/Scripting/MacroWorker.h/.cpp` | NEW — worker-thread JS runner: `run(source)` compiles then evaluates once in a fresh `QJSEngine` (SDK prelude + `ControlApiBridge` `__ss_bridge` only — never direct helper bridges off-thread), `JsWatchdog` armed for the whole run, `stop()` = immediate-deadline arm so `JsWatchdogThread` flips the interrupt; signals `finished(result)`, `scriptError(message)`, `logMessage(text)`; engine released after every run |
| `app/src/DataModel/Scripting/MacroRunner.h/.cpp` | NEW — QML-instantiable facade (`qmlRegisterType`, no singleton; TerminalBridge pattern): owns the worker QThread (join-before-teardown in dtor), `runJs`/`runLua`/`verify`/`stopRequested`/`busy`; Lua path GUI-sync via `ScriptApiCall::installAll(lua_State)` + MASKCOUNT deadline; load/save via C++ `QFileDialog` defaulting to the app-data `Macros/` dir (capitalized, consistent with sibling app-data folders; work deferred out of `fileSelected` per the macOS reentrancy rule) |
| `app/src/Misc/ModuleManager.cpp` | `qmlRegisterType<DataModel::MacroEditor>` + `qmlRegisterType<DataModel::MacroRunner>` in `registerQmlTypes()` |
| `app/CMakeLists.txt` | the six new C++ files |
| `app/qml/Dialogs/ApiTerminal.qml` | right pane wrapped in TabBar ("Terminal", "Script") + StackLayout; Script tab = toolbar row (language combo, load/save/verify/run/stop/clear IconButtons) + `MacroEditor`; MacroRunner signals appended to the shared scrollback; Execute switches to the Terminal tab; unsaved-changes prompt before load/clear (R7); editor content survives window close within the session (loader keeps state — verify DialogLoader semantics, else cache in a `property`) |

Nothing else: no API handlers, no manifests, no generated surfaces, no registry entries
(the toolbar is window-local QML, not a spec-0028 command surface).

## Architecture & data flow

- **Execute (JS)**: QML run button → `MacroRunner.runJs(text)` → queued `run(source)` on
  the worker thread → fresh `QJSEngine` + SDK prelude + `ControlApiBridge` (each `apiCall`
  marshals to the GUI thread via `BlockingQueuedConnection` into
  `ControlApiMarshaller::dispatch` — the same single dispatch spine as everything else) →
  top-level evaluation → `finished`/`scriptError`/`logMessage` queued back to QML →
  appended to the scrollback. `print`/`console.log` route through the bridge's log path
  (prelude mapping), so output streams while the macro runs.
- **Execute (Lua, v1)**: `MacroRunner.runLua(text)` GUI-synchronous: fresh `lua_State`,
  `ScriptApiCall::installAll` (GUI thread — direct bridges are correct here), MASKCOUNT +
  `QDeadlineTimer` hook with a generous budget (30 s) as the anti-hang bound; documented
  as blocking in v1.
- **Stop**: enabled only while a JS macro runs; `MacroRunner.stop()` →
  `JsWatchdog` immediate-deadline arm → `JsWatchdogThread` (20 ms poll) calls
  `setInterrupted(true)` — the only file allowed to. Error surfaces as an interruption
  message in the scrollback.
- **Verify**: throwaway engine, compile-only (`controlscript.dryRun` model): JS — engine
  evaluate of a function-wrapped parse / `QJSEngine::evaluate` error inspection with line
  numbers; Lua — `luaL_loadstring` (loads, never calls). No side effects (spec resolved:
  parse-only).
- **Fresh engine per execute** (spec resolved): `MacroWorker` releases the engine after
  every run; a crashed/interrupted run leaves no state behind.
- **Threading rule honored**: the worker engine gets ONLY `__ss_bridge` — installing the
  direct helper bridges (`__ss`, `__ss_db`, …) on a worker engine is the named threading
  bug from scripting.md and does not happen here.

## Hotpath & threading impact

- **Touches the hotpath?** No. All engine work is user-initiated; nothing per-frame; an
  idle Script tab costs zero. Macro `apiCall`s land on the GUI thread exactly like control
  scripts and the terminal — no frame-path contact beyond what those commands already do.
- **New cross-thread signal/slot?** Yes, contained: QML→worker `run` (queued), worker→QML
  `finished`/`scriptError`/`logMessage` (queued), `apiCall` marshal
  (`BlockingQueuedConnection` — the established `ControlApiMarshaller` pattern, reused not
  reimplemented). Worker thread joins in `MacroRunner`'s dtor **before** releasing worker
  resources (the driver thread-join rule).
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — untouched.
- `--benchmark-hotpath` — no expected delta; CI gate confirms (AC10).

## Data model & persistence

- No project-JSON, no `Keys::`, no DB. Macro files are plain `.js`/`.lua` on disk; default
  directory `<AppDataLocation>/Macros/` created on first use; file dialogs remember free
  navigation. Unsaved editor text: kept for the session (QML property cache), prompt
  before destructive load/clear (R7). Window geometry stays spec-0045's `Settings`.

## API / SDK surface

- None changed. Macros consume the existing SDK/`apiCall` (control-mode prelude fallbacks
  for JS worker; full GUI-thread install for Lua). No new commands, no generated-surface
  delta.

## QML / UI

- Right pane: `TabBar` + `StackLayout`. Terminal tab unchanged (R9). Script tab:
  toolbar row styled like the 0045 input row (24 px controls, `iconSize: 16`,
  `buttons/*.svg` icons) — language `Widgets.Combo` (JS/Lua), Load, Save, Verify, Run,
  Stop (JS-only enabled), Clear; `MacroEditor` fills the rest. Execute auto-switches to
  the Terminal tab so streamed output is visible; a one-line echo (`> [macro] run (js)`)
  precedes output. Run disabled while busy (R3); Stop enabled only while busy.
- Editor keyboard handling relies on the embedded editor's ShortcutOverride forwarding —
  no window-level `Shortcut` additions (ambiguity rule; SmartDialog already owns Close).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| JS execution thread | worker thread (MacroWorker) vs GUI-sync | Worker — Stop is impossible on a blocked GUI event loop, and "VBA" macros (delays, device round-trips) must not freeze the app; the marshaller/bridge already exist. |
| Lua v1 thread | GUI-sync + MASKCOUNT deadline vs worker w/ new Lua marshal | GUI-sync — a Lua worker needs new off-thread bridge plumbing on the forced-unwind surface; spec resolved Lua as bounded-blocking v1. |
| Worker reuse | new `MacroWorker` sibling vs reusing `ControlScriptWorker` | Sibling — control script's per-connection force-restart lifecycle and setup()/loop() split fight one-shot macros; the shareable parts (marshaller, bridge, watchdog) are classes, not the worker. |
| Editor | new `MacroEditor` vs reusing `ControlScriptEditor` | New sibling — ControlScriptEditor reads/writes ProjectModel; macros are project-independent. Plumbing copied, coupling dropped, highlighter made switchable. |
| Output routing | shared scrollback + auto-tab-switch vs per-tab output view | Shared scrollback (spec R4) — one output timeline for commands and macros; auto-switch keeps it visible without duplicating the view. |
| Load/save dialogs | C++ QFileDialog in MacroRunner vs QML FileDialog | C++ — matches every existing editor (`importFile()`), keeps the macOS `fileSelected` deferral in one audited place. |

## Risks & mitigations

- **Off-thread bridge misuse** (the named scripting.md threading bug): worker engine gets
  `__ss_bridge` only; review checkpoint in tasks.
- **Watchdog lint**: `setInterrupted(true)` stays exclusively in `JsWatchdogThread.cpp`;
  stop is expressed as an immediate-deadline `arm()`.
- **Editor plumbing regressions**: the three invariants are copied verbatim from
  `ControlScriptEditor`; task names them at edit time.
- **Blocking-queued deadlock** (worker blocked on GUI while GUI waits on worker): never
  wait on the worker from the GUI thread; stop is async + join only in dtor (with
  `requestShutdown`-style guard mirrored from control script for app teardown).
- **A macro that closes its own window**: worker outlives the QML item only until
  `MacroRunner` dtor joins; signals are queued so late arrivals hit a destroyed receiver
  safely (auto-disconnect).
- **Lua GUI freeze**: bounded by the MASKCOUNT deadline; documented; JS is the recommended
  language and the default.
- **macOS file-dialog reentrancy**: all post-dialog work deferred via queued invoke.

## Test & verification plan

- **Unit (I can run):** none — no `tests/scripts/` parser change.
- **Integration (maintainer runs):** none new; existing API integration suite remains the
  dispatch-path net.
- **In-app (maintainer):** AC1 (JS macro loop + print), AC2 (Lua variant), AC3 (verify
  syntax error + line, run refused), AC4 (save/clear/load round-trip), AC5
  (`while(true){}` stopped via Stop/watchdog, app healthy after), AC6 (throwing macro,
  subsequent runs fine), AC7 (0045 regression pass), AC8 (GPL build).
- **Static (AC9):** `code-verify.py --check` on all new/edited files; `registry-verify.py`
  (should be a no-op — no registry surface); sanitize pipeline; `qt-cpp-review` on the new
  C++ before handoff.
- **Hotpath (AC10):** CI `--benchmark-hotpath` gate; no delta expected.
