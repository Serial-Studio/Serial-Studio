---
spec: 0045-api-terminal
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-08-05
---

# Tasks 0045 — API Terminal window

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

## Tasks

### T1 — TerminalBridge class

- **Files:** `app/src/API/TerminalBridge.h`, `app/src/API/TerminalBridge.cpp`
- **Does:** New `API::TerminalBridge` QObject (QML-instantiable, no singleton — never adds a
  `SessionContext::current()` call). `run(const QString& input)`: split first whitespace
  token as command, parse remainder as JSON object via `QJsonDocument` (parse error returns
  `{ok:false, error}` without dispatch), build `CommandRequest` (QUuid id), dispatch through
  `CommandHandler::processCommand(request, CommandOrigin::Trusted)` — the identical
  `ScriptApiCall.cpp:dispatchApiCall()` spine, never a second dispatch path — wrap response
  as `{ok, result | error+errorCode+errorData}` mirroring `ScriptApiCallBridge::call()`.
  `catalog()`: iterate `CommandRegistry::commands()` into
  `{name, scope, verb, description, params:[{name, type, description, required}]}` from
  `inputSchema.properties` + `required`; empty/missing schema yields empty params list.
  Header follows the repo layout (SPDX dual-license header, `[[nodiscard]]`, no in-header
  init, `SS_ASSERT` density).
- **Verify:** `python3 scripts/code-verify.py --check app/src/API/TerminalBridge.h
  app/src/API/TerminalBridge.cpp`; read-back against `ScriptApiCallBridge::call()` for
  wrap-shape parity.
- **Deps:** none
- [x] done

### T2 — Build + QML type registration

- **Files:** `app/CMakeLists.txt`, `app/src/Misc/ModuleManager.cpp`
- **Does:** Add `TerminalBridge.cpp/.h` to the source lists;
  `qmlRegisterType<API::TerminalBridge>("SerialStudio", 1, 0, "ApiTerminalBridge")` beside
  the existing `qmlRegisterType` block in `registerQmlTypes()`. No composition-root /
  `instantiateCoreModules()` change — the pinned order is untouched.
- **Verify:** grep both files for the new entries; `python3 scripts/code-verify.py --check
  app/src/Misc/ModuleManager.cpp`.
- **Deps:** T1
- [x] done

### T3 — ApiTerminal.qml window shell

- **Files:** `app/qml/Dialogs/ApiTerminal.qml` (new), `app/CMakeLists.txt`
- **Does:** `Widgets.SmartWindow` (`category: "ApiTerminal"`, minimumWidth ~880,
  `Cpp_NativeWindow.addWindow/removeWindow` on visible — DatabaseExplorer pattern, no
  Pro/operator branches). Instantiates `ApiTerminalBridge`; `Component.onCompleted` pulls
  `catalog()` once into a JS array. Two-pane layout skeleton (left discovery pane, right
  terminal pane, `PaneSplitter` if it fits naturally). Register the QML file in
  `app/CMakeLists.txt`. Theme via `Cpp_ThemeManager.colors`, all strings `qsTr()`.
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  read-back against `DatabaseExplorer.qml` for the SmartWindow idiom.
- **Deps:** T2
- [x] done

### T4 — Terminal pane: scrollback + input + history

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`
- **Does:** Read-only mono scrollback (`ScrollView` + `TextArea`, `selectByMouse`, context
  menu + shortcuts for copy/clear/select-all — R7) and input `LineField`. On accept:
  echo the line, call `bridge.run(text)`, pretty-print the JSON response (`ok` vs error
  with `errorCode`), auto-scroll to bottom. Session-only history array; Up/Down recall
  when the completion popup is closed (R2, R6).
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  maintainer smoke: AC1 (`api.getCommands` with API server off), AC3 (`foo.bar` error
  shape), AC6 (history).
- **Deps:** T3
- [x] done

### T5 — Autocomplete popup

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`
- **Does:** Completion `Popup` anchored to the input: visible while the first token has no
  trailing space and matches ≥1 command name (prefix first, then substring); Up/Down
  navigate, Tab/Return accept (inserts name + space), Esc closes. Key handling must not
  steal history Up/Down (popup-open check) and must not double-bind sequences already used
  by window shortcuts (the ambiguous-Shortcut landmine — bind each sequence once). Typing a
  complete known command points the docs panel at it (R3).
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  maintainer smoke: AC2 (`io.` prefix → scoped completions, accept inserts).
- **Deps:** T4
- [x] done

### T6 — Discovery pane: search + scope list + docs panel

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`
- **Does:** `Widgets.SearchField` above a `ListView` (`section.property` = scope, sticky
  section labels) over the catalog filtered by name + description substring (R4). Selected
  row drives the docs panel: full name, description, parameter table (name, type,
  description, required marker) — catalog data only, no hand-written command text (R5).
  Double-click / Return inserts the command into the input line. Empty-params commands
  render a "no parameters" row.
- **Verify:** `python3 scripts/code-verify.py --check app/qml/Dialogs/ApiTerminal.qml`;
  maintainer smoke: AC4 (filter + clear restores), AC5 (`project.addMany` docs match
  `api-schema.json`).
- **Deps:** T3
- [x] done

### T7 — Window plumbing in main.qml

- **Files:** `app/qml/main.qml`
- **Does:** `DialogLoader { id: apiTerminalLoader; source: ...ApiTerminal.qml }` +
  `function showApiTerminal() { apiTerminalLoader.activate() }` — `showDatabaseExplorer`
  pattern minus the Pro/operator gating (R9: no license gate).
- **Verify:** `python3 scripts/code-verify.py --check app/qml/main.qml`; read-back that no
  `Cpp_CommercialBuild` guard wraps it.
- **Deps:** T3
- [x] done

### T8 — Command registration (manifest + binding)

- **Files:** `app/rcc/commands/app.json`, `app/qml/Commands/AppCommandBindings.qml`
- **Does:** Manifest entry `app.apiTerminal` (title/tooltip, icon `commands/api-terminal`,
  kind `action`, contexts `["app","dashboard"]`, category `tools`, no `pro`). Binding
  `cmdApiTerminal` with `run() → app.showApiTerminal()` + map entry. Binding references no
  `Cpp_Licensing_/Cpp_Sessions_/Cpp_MQTT_` symbol, so the commercial-guard scan stays
  clean; binding in the app set is reachable from both palette models (R1).
- **Verify:** `python3 scripts/registry-verify.py`;
  `python3 scripts/generate-command-strings.py --check` (expected drift → T9).
- **Deps:** T7
- [x] done

### T9 — Generated strings + pipeline sweep

- **Files:** `app/src/UI/CommandStrings.cpp` (regenerated; script-owned)
- **Does:** Run `python3 scripts/generate-command-strings.py` to fold the new
  title/tooltip into the lupdate stub, then the full static gate:
  `registry-verify.py`, `code-verify.py --check` over all touched files. **Never hand-edit
  the generated file; never touch `.ts`/`.qm`** (translation refresh is the maintainer's
  lupdate run).
- **Verify:** `python3 scripts/generate-command-strings.py --check` clean;
  `python3 scripts/registry-verify.py` clean.
- **Deps:** T8
- [x] done

### T10 — Self-review + handoff

- **Files:** none (review pass)
- **Does:** Re-read the full diff: scope = plan's file list only (icons/qrc already landed
  pre-spec this session, named in plan). Run `qt-cpp-review` on the new C++. Counterfactual
  check: riskiest rule = second-dispatch-path drift — evidence: `run()` calls
  `CommandHandler::processCommand` directly, no reimplemented validation. Hand the
  maintainer the in-app AC list (AC1-AC8) + AC10 CI note.
- **Verify:** `qt-cpp-review` findings addressed or noted; diff read-back clean.
- **Deps:** T1-T9
- [x] done

### T11 — Maintainer live-run polish (added during /ss-implement)

- **Files:** `app/qml/Dialogs/ApiTerminal.qml`, `app/src/API/TerminalBridge.cpp` (review
  fixes), `app/src/API/TerminalBridge.h`, `doc/claude/specs/0045-api-terminal/plan.md`
- **Does:** From the maintainer's first live run + qt-cpp-review: root switched
  SmartWindow to SmartDialog (titlebar inset + drag + Close shortcut; own `Settings`
  geometry block; 1 px titlebar separator); send + clear IconButtons beside the input;
  tree-insert and completion-accept fill a JSON parameter skeleton
  (`command.name { "parameter": value }` with type-based placeholders); search
  placeholder "Search commands..."; QML key-handling fixes (Tab/Esc accepted-after-hide,
  Return on exact match runs instead of completing). Review fixes: SS_ASSERT
  `continue`-action dead guard replaced with `SS_ASSERT_LOG` + explicit skip; exception
  catches report `EXECUTION_ERROR`; empty line reports `INVALID_MESSAGE_TYPE`;
  whitespace (not space-only) tokenizer; singletons captured in ctor init list
  (census re-baselined, +2 deliberate).
- **Verify:** `python3 scripts/code-verify.py --check` clean on both files;
  `registry-verify.py` clean; maintainer re-run of the window.
- **Deps:** T10
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there (AC1-AC8 are
      maintainer in-app observations — pending the maintainer's run; AC9 scripted, clean;
      AC10 CI benchmark gate on push).
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff (6 agents); all confirmed findings fixed
      (SS_ASSERT continue-action, error-code semantics, whitespace tokenizer, singleton
      member capture); census re-baselined (+2 deliberate ctor captures).
- [x] Hotpath untouched (plan says none) — no `--benchmark-hotpath` delta expected; CI gate
      confirms.
- [x] No new `pytest` targets (no wire-surface change) — existing API integration suite is
      the regression net.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — no scope creep, no foreign files touched
      (in particular: no `.ts`/`.qm`, no generated files edited by hand).
- [x] `spec.md` status set to `done`.
