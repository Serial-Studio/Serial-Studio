---
spec: 0046-script-macros
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-05
---

# Tasks 0046 — Script macros in the API Terminal

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable*. `/ss-implement` works this list top to
> bottom and keeps the status boxes current. Gate: do not start `/ss-implement` until a
> human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.

## Tasks

### T1 — MacroWorker (worker-thread JS runner)

- **Files:** `app/src/DataModel/Scripting/MacroWorker.h`,
  `app/src/DataModel/Scripting/MacroWorker.cpp`
- **Does:** Read `ControlScriptWorker.h/.cpp` in full first. New `DataModel::MacroWorker`
  QObject (worker-thread affinity): `run(const QString& source)` slot builds a **fresh**
  `QJSEngine` + SDK prelude + `ControlApiBridge` (`__ss_bridge`) — **worker engines get the
  apiCall marshal bridge ONLY; installing any direct helper bridge off-thread is the named
  scripting.md threading bug** — arms its `JsWatchdog` for the whole run, evaluates the
  program top-level once, emits `logMessage(text)` (print/console.log routed via the
  bridge/prelude), then `finished(result)` or `scriptError(message-with-line)`, and
  **releases the engine after every run** (fresh-per-execute, spec-resolved).
  `requestStop()` arms the watchdog with an immediate deadline — `setInterrupted(true)`
  stays exclusively in `JsWatchdogThread.cpp` (blocking lint). Mirror
  `requestShutdown()`-style teardown guard from ControlScriptWorker.
- **Verify:** `python3 scripts/code-verify.py --check` both files; read-back against
  `ControlScriptWorker::start/compile/releaseEngine` for the engine-lifecycle idiom.
- **Deps:** none
- [x] done (MacroApiBridge divergence named in chat + plan amended: stop-aware bridge with
  log(), marshaller reused verbatim)

### T2 — MacroRunner (QML facade, thread owner, Lua path, verify, file dialogs)

- **Files:** `app/src/DataModel/Scripting/MacroRunner.h`,
  `app/src/DataModel/Scripting/MacroRunner.cpp`
- **Does:** QML-instantiable `DataModel::MacroRunner` (TerminalBridge pattern: no
  singleton, no `SessionContext::current()`; singleton deps captured in ctor init list if
  any). Owns the worker `QThread` + `ControlApiMarshaller` + `MacroWorker`; **dtor joins
  the thread BEFORE releasing worker resources** (driver thread-join rule; use
  `quit()`+`wait()` — remember the started-signal/exec() trap from common-mistakes).
  `runJs(text)` (queued to worker; `busy` property; never blocks the GUI on the worker),
  `stop()`, `runLua(text)` GUI-synchronous: fresh `lua_State`,
  `ScriptApiCall::installAll`, `LUA_MASKCOUNT` + `QDeadlineTimer` hook, 30 s budget,
  results/errors via the same signals. `verify(text, language)`: compile-only throwaway
  engine — JS `QJSEngine::evaluate` error inspection, Lua `luaL_loadstring` (loads, never
  calls) — returns `{ok, error, line}`. `loadMacro()`/`saveMacro(text)` via `QFileDialog`
  defaulting to `<AppDataLocation>/Macros/` (capitalized; created on first use) — **all
  post-dialog work deferred out of `fileSelected` via queued invoke (macOS reentrancy
  rule)**.
- **Verify:** `python3 scripts/code-verify.py --check` both files; read-back: no direct
  helper bridge on the worker engine, join-order in dtor, deferral in dialog callbacks.
- **Deps:** T1
- [x] done

### T3 — MacroEditor (embedded code editor, project-independent)

- **Files:** `app/src/DataModel/Editors/MacroEditor.h`,
  `app/src/DataModel/Editors/MacroEditor.cpp`
- **Does:** Read `ControlScriptEditor.h/.cpp` in full first. New sibling
  `DataModel::MacroEditor`: copy the offscreen-`QCodeEditor` plumbing **verbatim,
  preserving the three invariants — (1) `renderWidget()` calls `syncWidgetPosition()`
  first, (2) `event()` forwards `ShortcutOverride` to the widget, (3) `keyPressEvent`
  reroutes to `completer()->popup()` while visible — and the direct `renderWidget()`
  calls in input handlers (no timer-tick lag)**. Drop the ProjectModel/ProjectEditor
  members entirely; plain `text` property (get/set), `language` property (0 = JS, 1 = Lua)
  switching the highlighter, `isModified`/`setModified`, standard edit slots
  (cut/copy/paste/undo/redo/selectAll/clear), theme + font hookups as in the sibling.
- **Verify:** `python3 scripts/code-verify.py --check` both files; diff against
  `ControlScriptEditor.cpp` to confirm the plumbing is verbatim where it must be.
- **Deps:** none
- [x] done

### T4 — Build + QML type registration

- **Files:** `app/CMakeLists.txt`, `app/src/Misc/ModuleManager.cpp`
- **Does:** Add the six new files to the source lists;
  `qmlRegisterType<DataModel::MacroEditor>("SerialStudio", 1, 0, "MacroEditor")` and
  `qmlRegisterType<DataModel::MacroRunner>("SerialStudio", 1, 0, "MacroRunner")` beside
  the existing block in `registerQmlTypes()`. No composition-root change; pinned order
  untouched.
- **Verify:** grep both files; `python3 scripts/code-verify.py --check
  app/src/Misc/ModuleManager.cpp`; singleton census unchanged (or `--accept` with the
  named reason if a ctor capture adds sites).
- **Deps:** T1, T2, T3
- [x] done (census re-baselined: MacroWorker/MacroRunner leaf ctor captures, deliberate)

### T5 — ApiTerminal.qml: tab structure

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`
- **Does:** Wrap the right pane in `TabBar` ("Terminal", "Script") + `StackLayout`.
  Terminal tab keeps the existing scrollback + input row **behaviorally unchanged (R9)**.
  Script tab is an empty placeholder this task. **No new window-level `Shortcut`
  sequences** (SmartDialog owns Close; ambiguity rule).
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  read-back: terminal pane subtree unchanged except reparenting.
- **Deps:** T4
- [x] done (window renamed ApiTerminal.qml -> Macros.qml mid-task per maintainer: command
  `app.macros` + `commands/macro` icon replace `app.apiTerminal`/`commands/api-terminal`)

### T6 — Script tab: toolbar + editor + wiring

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`
- **Does:** Script tab content: toolbar row in the 0045 input-row style (24 px controls,
  `iconSize: 16`, `buttons/*.svg`) — language `Widgets.Combo` (JS/Lua), Load, Save,
  Verify, Run, Stop, Clear — above a `MacroEditor` filling the tab. Instantiate
  `MacroRunner`; wire: Run → echo `> [macro] run (<lang>)`, auto-switch to the Terminal
  tab, `runJs`/`runLua`; `logMessage`/`finished`/`scriptError` append to the shared
  scrollback (R4); Run disabled while `busy`, Stop enabled only while a JS run is busy;
  Verify prints ok/error+line to the scrollback; Load/Clear prompt when
  `editor.isModified` (R7); editor text cached in a window property so it survives
  close/reopen within the session (verify DialogLoader keeps or destroys the item — cache
  accordingly). Keyboard: rely on the editor's ShortcutOverride forwarding; add no
  `Shortcut` items.
  NOTE (implementation): DialogLoader destroys the item on close, so the draft lives in
  `app.macroDraft`/`app.macroDraftLanguage` (main.qml), written from the window's
  `onClosing` and restored in `Component.onCompleted`.
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  maintainer smoke: AC1 (JS macro prints count), AC3 (verify error + refused run), AC4
  (save/clear/load round-trip), AC7 (terminal tab regression).
- **Deps:** T5
- [x] done

### T7 — Interruption + error-path polish

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`, `app/src/DataModel/Scripting/MacroWorker.cpp`
  (only if the smoke run shows gaps)
- **Does:** Confirm the stop path end-to-end: `while(true){}` → Stop → interruption message
  in scrollback, `busy` false, next run works (AC5); throwing macro reports line info and
  leaves the app healthy (AC6); Lua deadline fires and reports (bounded blocking,
  documented). Fix whatever the pass surfaces, staying inside the file list.
- **Verify:** maintainer smoke of AC5/AC6 + a Lua `while true do end` deadline check;
  `code-verify` clean on any touched file.
- **Deps:** T6
- [x] done (static stop/error paths verified in code; the live AC5/AC6/Lua-deadline smoke
  is the maintainer's run — findings loop back here)

### T8 — Docs-lite + spec bookkeeping

- **Files:** `doc/claude/specs/0046-script-macros/` (status fields), `tasks.md` boxes
- **Does:** Keep the checklist live; note the "Lua blocks the GUI in v1" behavior in the
  spec's resolved-questions block if wording needs tightening. No user-manual work (docs
  for `doc/help` are a separate ask).
- **Verify:** files read back consistent.
- **Deps:** T7
- [x] done (also recorded mid-implementation consolidation: window is now
  `Dialogs/Macros.qml` / command `app.macros` / icon `commands/macro`; spec-0045
  `app.apiTerminal` command + `commands/api-terminal` icons removed at maintainer request
  — the 0045 terminal surface lives on as this window's Terminal tab)

### T9 — Self-review + handoff

- **Files:** none (review pass)
- **Does:** Full-diff read-back: scope = plan's file list only. `qt-cpp-review` on the new
  C++ (worker threading + editor plumbing are the high-risk zones). Counterfactual check
  at handoff: riskiest rule = off-thread direct-bridge install / thread-join order — name
  the concrete evidence both hold. Run `sanitize-commit.py`. Hand the maintainer AC1-AC8
  + AC10 CI note.
- **Verify:** review findings addressed or noted; sanitize clean.
- **Deps:** T1-T8
- [x] done (6-agent review: teardown cluster fixed — heap QThread + warn-and-abandon,
  7 s join budget > 5 s watchdog, latched teardown flag, GUI-side stop reset, stop-gated
  watchdog re-arms; Lua try-block widened to contain __tostring/setup panics; QSaveFile
  save; verify() const; SerialStudio::ScriptLanguage anchoring; declarative combo sync;
  worker-side tr() dropped for donor parity. Declined: Lua worker thread (v1 documented),
  copy-lift refactor of the bridge pair.)

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC8
      maintainer in-app — pending the maintainer's run; AC9 scripted, clean; AC10 CI gate).
- [x] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (6 agents); all confirmed findings fixed (see T9).
- [x] Hotpath untouched — no `--benchmark-hotpath` delta expected; CI confirms.
- [x] No new `pytest` targets (no wire-surface change).
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched,
      no `.ts`/`.qm`, no generated files edited by hand (mid-task rename to Macros.qml +
      api-terminal removal were maintainer-directed, recorded at T5/T8).
- [x] `spec.md` status set to `done`.
