---
spec: 0098-trust-follows-the-author
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement) (approved 2026-10-14)
updated: 2026-10-14
---

# Tasks 0098 — Code never runs with more trust than whoever supplied it

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). Milestone order:
> M1 (classification and gate), M2 (inert surface), M3 (launch consent), M4 (assistant tiers),
> M5 (viewer), M6 (restore), M7 (docs). Each milestone leaves the tree coherent. Every C++
> task's Verify includes `python3 scripts/syntax-check.py <files>` and
> `python3 scripts/code-verify.py --check <files>`; they are not repeated below. A new TU is
> missing from the compile database until the maintainer reconfigures, so it borrows a
> same-library donor's compile entry (sanctioned 2026-10-13); where that is impossible the
> SKIP is reported at handoff.

## Conventions

- One task = one focused, reviewable change. If a task touches more than 3 files or needs a
  paragraph to describe, split it. A `.h/.cpp` pair counts as one file.
- **Deps** lists task IDs that must land first.
- **Main session** marks a protected surface (API auth/consent) or a security judgment that is
  never delegated. Before each such edit, its failure modes are named in chat. Unmarked tasks
  may run in a Sonnet subagent from a half-page brief; the main session reviews the diff.
- **P** marks the protected API auth/consent files. A behavioral change there is never batched
  with other work.
- The GPL build must compile unchanged after every task. The Process driver and `io.process.*`
  stay commercial-only; the launch authorizer and `system.exec` path exist in every build.

## M1: Capability classification and the remote gate

### T1 — Capability table

- **Files:** `core/Api/API/CommandCapabilities.{h,cpp}` (new), `core/Api/CMakeLists.txt`
- **Does:**
  - Defines the capability flags: `ControlScriptOnly`, `InstallsScript`, `LoadsProject`,
    `WritesDevice`, `ChoosesProgram`, `StartsProgram`, `RunsInertCode`, `Dispatches`, plus a
    `Conditional` marker.
  - Fills a constexpr table with every command in `app/rcc/api/api-schema.json`, sorted for
    binary search like `SealedPolicy`.
  - `of(command, params)` returns `std::optional<Capabilities>`. Conditional entries route to
    `refine()`:
    - `project.dataset.update` / `group.update` / `outputWidget.update` install script when
      their code key is present;
    - `project.action.update` writes to the device under today's rules;
    - `project.source.setProperty` / `setProperties` choose a program for the keys
      `executable`, `arguments`, `workingDir`, `mode`;
    - `io.setBusType` / `project.source.update` choose a program when the bus type is Process.
  - New members versus today: `assistant.restore` and `sessions.regress` / `verify` / `replay`
    load a project; `extensions.install` installs script.
  - *Invariant:* the table never allows by omission. An unknown name returns empty, and the
    gate refuses it.
- **Verify:** T2's sweep; a read-back of every non-empty entry against plan.md's gate map.
- **Deps:** none · **Main session**
- [x] done

### T2 — Classification sweep

- **Files:** `app/tests/tst_command_capability_registry.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** AC7, modeled on `tst_sealed_policy_registry`:
  - every schema command is classified exactly once, and no table name is stale;
  - the table is sorted and unique;
  - each conditional entry resolves both ways from sample params;
  - unknown and empty names return empty.
- **Verify:** the suite builds against `CommandCapabilities.cpp` and Qt6::Core only (CMake
  read-back). The maintainer runs ctest.
- **Deps:** T1
- [x] done

### T3 — Remote gate decides by capability

- **Files:** `core/Api/API/Server/ServerAuth.{h,cpp}` **P**,
  `core/Api/API/Server/AuthPrimitives.{h,cpp}` **P**
- **Does:**
  - `authorizeRemoteCommand` follows plan.md's gate map, in order: unclassified → refused;
    ControlScriptOnly → refused; InstallsScript or LoadsProject → script-change consent;
    WritesDevice → device-write consent. Every gate that applies must pass, where today the
    first match wins.
  - Once all gates pass for a command that installs script, loads a project or chooses a
    program, it emits `remoteProgramChanged()`.
  - The three classification predicates and their name lists are deleted from
    AuthPrimitives; the token functions stay.
  - *Invariant:* this stays the ONE policy for outer and nested remote calls, and its refusal
    texts and codes are unchanged for every command that was gated before.
- **Verify:** `grep -rn "commandInstallsScript\|commandWritesToDevice\|commandIsControlScriptOnly"`
  finds nothing outside tests; T4.
- **Deps:** T1 · **Main session**
- [x] done

### T4 — Port the server-auth tests

- **Files:** `app/tests/tst_server_auth.cpp`, `app/tests/CMakeLists.txt`
- **Does:**
  - Rewrites `commandClassification` / `scriptInstallClassification` against
    `CommandCapabilities`.
  - Adds `remoteCommandPolicy` rows: refused when unclassified, project loads ask consent,
    a program choice passes but emits the signal, and an unanswered gate emits nothing.
  - Adds `CommandCapabilities.cpp` to the suite's SOURCES.
- **Verify:** CMake read-back; the maintainer runs ctest.
- **Deps:** T3
- [x] done

## M2: Inert surface for dry runs and compile checks

### T5 — Spike: confirm the inert table seam

- **Files:** none (findings recorded in this task's notes)
- **Does:** Answers three questions before T6:
  - Do `TableScriptBridge`'s JS and Lua closures bind only their own store and context, with
    no reach into FrameBuilder?
  - Which accessor gives the GUI-side table mirror snapshot without a routed read?
  - What reply shape do the dry-run commands use, so the `console` array has a home?
  If the closures reach FrameBuilder, T6 builds a minimal table shim over a `QHash` instead of
  a scratch `TableScriptBridge`; that is recorded here.
- **Verify:** findings written here, with file:line anchors.
- **Deps:** none
- **Findings (2026-10-14):**
  - The closures bind only `TableApiContext {store, owner, mirror}` (`TableScriptBridge.cpp:414-421`).
    A scratch `TableScriptBridge` over a store owned by a GUI-thread `QObject` reads and writes
    directly (`readTableView`'s first branch, `DataTable.h:241`), so no fallback shim is needed.
  - The GUI-side snapshot is reachable as `FrameBuilder::guiTableApiContext().mirror`
    (`FrameBuilder.h:162`), but that call registers a permanent GUI table user (review finding),
    so the store is seeded from `ProjectModel` alone, at its starting values.
  - `mqttPublish` lives on the table bridge (`DataTable.h:307`, `TableScriptBridge.cpp:466`), so
    the JS surface wraps `__ss` (the real bridge is reachable only through a closure) and the Lua
    surface overwrites the global.
  - Dry-run replies carry no console field, and API callers never received console output (it
    went to the app's terminal). The surface drops it, and the `console` reply array is dropped
    from the plan.
  - SDK evaluation is split out of `ScriptApiCall::installJS` / `installLua` into
    `evaluateSdk()`, done in T6 rather than T7, so the inert surface can evaluate the SDK over
    its stand-ins.
- [x] done

### T6 — InertScriptSurface

- **Files:** `core/Pipeline/DataModel/Scripting/InertScriptSurface.{h,cpp}` (new),
  `core/Pipeline/CMakeLists.txt`, `core/Pipeline/DataModel/Scripting/ScriptApiCall.{h,cpp}`
  (the `evaluateSdk()` split, see T5)
- **Does:** `installJs(QJSEngine*)`, `installLua(lua_State*)` and `verify()` install, under the
  live global names:
  - an `apiCall` stub returning `{ok:false, error:'dryRun'}`;
  - no-op recorders for deviceWrite, actionFire, the dashboard API, notifications and
    mqttPublish;
  - the table API over a scratch store at the project's starting values (amended in review:
    the GUI mirror is not read, see the T5 notes);
  - a console and `print` that drop their output;
  - checksums, which have no effects and stay real.
  After install, `verify()` asserts that every bridge global is the inert object.
  *Invariant:* no live installer is called on this path, and no table read is routed into the
  pipeline thread, so no nested event loop runs.
- **Verify:** read-back that the file includes no live installer; `layer-verify.py`.
- **Deps:** T5 · **Main session**
- [x] done

### T7 — Surface choice in installAll and the JS engine

- **Files:** `core/Pipeline/DataModel/Scripting/ScriptApiCall.{h,cpp}`,
  `core/Pipeline/DataModel/Scripting/JsScriptEngine.{h,cpp}`
- **Does:**
  - `installAll` gains `Surface surface = Surface::Live`; `Inert` delegates to
    InertScriptSurface.
  - `JsScriptEngine(Surface = Live)`. An Inert engine never calls
    `Core::Prompt::showMessageBox` after timeouts, and reports errors into its own diagnostics
    instead of `qWarning`.
  - *Invariant (FrameParser's engine):* the Live install sequence and order stay byte-for-byte
    today's. The branch runs once at construction, never per frame. FrameParser keeps
    constructing the default.
- **Verify:** a `git diff` read-back showing the Live path unchanged line for line.
- **Deps:** T6
- [x] done

### T8 — Surface choice in the Lua engine

- **Files:** `core/Pipeline/DataModel/Scripting/LuaScriptEngine.{h,cpp}`
- **Does:** `LuaScriptEngine(Surface = Live)`. Inert states install InertScriptSurface instead
  of `installAll`, always run with the Safe-mode deadline and memory budget whatever the
  project's mode, and report errors into their own diagnostics. *Invariant:* the same Live
  byte-identity as T7. The state still strips `dofile` / `loadfile` (`:120`).
- **Verify:** a diff read-back of the Live path.
- **Deps:** T6
- [x] done

### T9 — Dry-run commands build inert engines

- **Files:** `core/Pipeline/DataModel/Scripting/FrameParserPipeline.{h,cpp}`,
  `core/Api/API/Handlers/ProjectDryRunCommands.cpp`
- **Does:**
  - `runFrameParserPipelineWithCode` and `makeScriptEngine` build Inert engines.
  - endToEnd's transform engines are Inert too.
  - The endToEnd hint that says the table API is not injected becomes true: it is injected,
    but inert.
- **Verify:** grep finds no `std::make_unique<DataModel::JsScriptEngine>()` /
  `LuaScriptEngine>()` without an Inert argument in the file.
- **Deps:** T7, T8
- [x] done

### T10 — Compile checks and previews go inert

- **Files:** `core/Api/API/Handlers/ProjectFileCommands.cpp`,
  `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp`
- **Does:** `project.validate` compiles on Inert engines. The transform editor's previews
  (`:1034`, `:1144`) install InertScriptSurface instead of the live bridges.
- **Verify:** a read-back of both call sites.
- **Deps:** T6, T7, T8
- [x] done

### T11 — Close the bare-engine leaks

- **Files:** `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.cpp`,
  `core/Pipeline/DataModel/Scripting/ScriptDryRun.cpp`
- **Does:** The Judging surface also masks `__nc` and `__ss_db`, and its verify step covers
  them. ScriptDryRun's Lua state strips `dofile` and `loadfile`, matching LuaScriptEngine.
- **Verify:** a read-back of the mask list against the bridges `installAll` installs.
- **Deps:** none
- [x] done

### T12 — Dry runs leave no trace (AC1)

- **Files:** `tests/security/test_caller_code_inert.py` (new)
- **Does:**
  - Covers every `RunsInertCode` command in JS and Lua: the dry runs and `project.validate`
    (which compiles project code, so the test loads it first).
  - Each case's top-level code tries `io.writeData` (against a loopback TCP device's byte
    count), `controlScript.setCode`, a marker-file `system.exec`, `tableSet` (read back with
    `project.dataTable.getValue`) and a notification (unread count unchanged).
  - Valid and invalid code still return today's diagnostics.
- **Verify:** `python -m py_compile`; the maintainer runs it against a GUI instance.
- **Deps:** T9, T10, T11
- [x] done

## M3: Launch consent at the spawn point

### T13 — ConsentGate revoke

- **Files:** `core/Api/API/Server/ConsentGate.{h,cpp}` **P**, `app/tests/tst_consent_gate.cpp`
- **Does:** `revoke()` puts the state back to Unset and removes the persisted key. An empty key
  touches no setting. The next `authorize()` asks again. Tests: grant, persist, revoke, ask
  again; revoke on an empty key.
- **Verify:** the maintainer runs ctest.
- **Deps:** none · **Main session**
- [x] done

### T14 — LaunchConsent

- **Files:** `core/Api/API/LaunchConsent.{h,cpp}` (new) **P**, `core/Api/CMakeLists.txt`,
  `app/tests/tst_launch_consent.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:**
  - Owns the per-project `ConsentGate`: same key format, same AUTO_CONSENT handling, and it is
    rebound on a project path change.
  - R13: `noteRemoteChange()` sets the session taint and clears the current key once. While
    tainted, every key the gate later rebinds to is cleared once. A key cleared and granted
    again stays granted for the session.
  - `setOperatorConsentedPath(path)`: binding that path starts Granted in memory unless R13
    has cleared it.
  - Under AUTO_CONSENT, clearing is a no-op.
  - Tests cover AC11, save-as onto a consented key, the operator grant and AUTO_CONSENT.
- **Verify:** the maintainer runs ctest.
- **Deps:** T13 · **Main session**
- [x] done

### T15 — Core launch authorizer interface

- **Files:** `core/Core/IO/LaunchAuthorizer.{h,cpp}` (new), `core/Core/CMakeLists.txt`
- **Does:**
  - `IO::ILaunchAuthorizer::authorizeLaunch(program, arguments, workingDir)` returns
    `IO::LaunchVerdict` (Allowed / ConsentRequired / Denied).
  - `IO::setLaunchAuthorizer` / `IO::launchAuthorizer` follow the `DataModel::setMqttPublisher`
    shape.
  - When nothing is bound, the free helper `IO::authorizeLaunch(...)` logs an assert and
    returns Denied.
  - *Invariant:* the file includes only Qt and Core headers.
- **Verify:** `layer-verify.py`.
- **Deps:** none · **Main session**
- [x] done

### T16 — ProcessLauncher authorizes every launch

- **Files:** `core/Api/API/ProcessLauncher.{h,cpp}` **P**, `core/Api/API/Server.{h,cpp}` **P**
- **Does:**
  - Replaces the inline `ConsentGate` with a `LaunchConsent` and implements
    `IO::ILaunchAuthorizer` on it. The detail line names the program and its arguments.
  - When the consent is granted while a refused Process start is pending and the link is down,
    it queues one `connectDevice()`. Queued, so the reconnect never runs under the prompt's
    stack (the K13 precedent).
  - Adds `onRemoteProgramChanged()`, which forwards to `LaunchConsent::noteRemoteChange()`.
    `Server` forwards `ServerAuth::remoteProgramChanged` as its own signal (the
    `authTokenChanged` precedent), and `ProcessLauncher::setupExternalConnections` connects
    it through `handlerContext().server`, so no new singleton reach is added.
  - *Invariant:* read the existing signal wiring in both files before adding the connection.
    It is a GUI-thread direct connection, and `system.exec` keeps its exact consent behavior.
- **Verify:** wiring read-back; `--singleton-census --check` shows no new cross-library reach.
- **Deps:** T3, T14, T15 · **Main session**
- [x] done

### T17 — The Process driver asks before it starts

- **Files:** `core/Devices/IO/Drivers/Process.cpp`
- **Does:**
  - In Launch mode, `open()` calls `IO::authorizeLaunch(resolved, arguments, workingDir)`.
    The call comes after `resolveExecutable` and before the QProcess is created; today
    `m_process->start` is at `:283`.
  - Anything other than Allowed logs "Waiting for permission to launch …" (for
    ConsentRequired) or "Launch denied" through `logDriverError` and returns false.
  - NamedPipe mode is unchanged.
  - *Invariant:* `open()` stays synchronous, on the GUI thread, and creates no QProcess on
    refusal.
- **Verify:** a read-back showing that the only `start` call sits behind the verdict.
- **Deps:** T15
- [x] done

### T18 — Root binding and the headless operator grant

- **Files:** `app/src/Misc/CLI.cpp`, `core/Core/Runtime.{h,cpp}`, `core/Api/API/ProcessLauncher.cpp`
  (amended 2026-10-14, see the note below)
- **Does:**
  - `ProcessLauncher::setupExternalConnections` binds itself as the launch authorizer
    (`IO::setLaunchAuthorizer(this)`). Every run that can connect a device wires the launcher,
    since both GUI and `--headless` go through `initializeQmlInterface`. The verify and regress
    children never wire it, so their launches fail closed (Denied); they never connect devices.
  - A headless run records its command-line project as `Core::Runtime::setOperatorProject`
    before the CLI opens it. `ProcessLauncher` hands that path to `LaunchConsent` on every
    project change, including before its unchanged-path early return.
  - *Invariant:* no change to `instantiateCoreModules()` order, no new reach in a constructor
    closure, and no growth in the singleton census.
- **Amendment:** `ModuleManager::bindInterfaces()` and a CLI → `ProcessLauncher` call would each
  add an `instance()` reach, and the census refuses any growth. The CLI also opens its project
  after the launcher is wired. So the binding moved into the launcher's own wiring, and the
  operator path became a `Core::Runtime` run fact beside `sealed()`. `ModuleManager.cpp` is not
  edited.
- **Verify:** `code-verify.py --singleton-census --check` stays at baseline (800); a read-back
  of the open-then-wire order in `main.cpp`.
- **Deps:** T16 · **Main session**
- [x] done

### T19 — Process launches need consent (AC4)

- **Files:** `tests/security/test_process_launch_consent.py` (new)
- **Does:**
  - Runs against `SerialStudio --headless` with the API server on and no AUTO_CONSENT, so
    every gate answers Denied deterministically.
  - (a) Sets the program through `project.source.setProperty` / `setProperties`, then
    `io.connect`.
  - (b) Switches the bus with `io.setBusType` to Process so the saved program is reused, then
    `io.connect`.
  - Asserts no marker file appears and the reply names the launch refusal.
  - A baud-rate `setProperty` still succeeds.
  - Skipped on GPL builds (`io.listBuses` has no Process entry).
- **Verify:** `python -m py_compile`; the maintainer runs it.
- **Deps:** T17, T18
- [x] done

## M4: Assistant tiers on every op

### T20 — DispatchPolicyScope

- **Files:** `core/Api/API/CommandRegistry.{h,cpp}`, `core/Api/API/CommandHandler.cpp`
- **Does:**
  - Adds `DispatchPolicy`, with the gate signature plus a nesting-depth argument, and an RAII
    `DispatchPolicyScope` stack (GUI-thread only).
  - `execute` consults it after the remote scope and before the path policy.
  - The trusted (non-remote) branch of `CommandHandler::processCommand` also opens an empty
    policy scope.
  - *Invariant:* `RemoteDispatchScope` and its `active()` stay untouched, because licensing's
    prompt probes read them.
- **Verify:** `grep RemoteDispatchScope` shows only the existing users; the existing pytest
  `tests/security/test_remote_dispatch_gate.py` is in the maintainer's run list.
- **Deps:** none · **Main session**
- [x] done

### T21 — Planned commands and effective tier

- **Files:** `core/Ui/AI/CommandRegistry.{h,cpp}`
- **Does:**
  - `plannedCommands(name, args)` lists the batch's and bulkApply's `ops[].command`, the setter
    `assistant.script.apply` uses for its `kind`, and addTile's five commands.
  - `effectiveSafety(name, args)` is the strictest of `safetyOf(name)` and every planned
    command's tier. A planned command that is itself a dispatcher counts as Blocked.
  - `safetyOf` is unchanged, including the device-gated rule.
- **Verify:** T26.
- **Deps:** none · **Main session**
- [x] done

### T22 — Dispatch uses the effective tier and re-checks nested calls

- **Files:** `core/Ui/AI/Conversation.cpp`, `core/Ui/AI/Tools/ToolDispatch.cpp`
- **Does:**
  - `dispatchByCallSafety` decides from `effectiveSafety`: Blocked shows the denial card
    (its reply names the offending op), AlwaysConfirm always waits for a click, and Confirm
    auto-approves as today. `executeCommand` recomputes the same tier at run time rather than
    having it passed in, so device control switched off between approval and run still blocks.
  - `executeCommand` opens a `DispatchPolicyScope` that refuses Blocked commands, commands
    above the approved tier, and a dispatcher at depth > 0. Refusals use the existing blocked
    reply shape.
  - *Invariant:* the autosave checkpoint and the AutoVerifier read-backs stay outside the
    scope.
- **Verify:** T26; a read-back that `meta.executeCommand` still re-enters
  `dispatchByCallSafety`.
- **Deps:** T20, T21 · **Main session**
- [x] done

### T23 — Bulk apply rejects nested dispatchers

- **Files:** `core/Ui/AI/Tools/ToolBulkTools.cpp`, `core/Ui/AI/Tools/ToolSchemas.cpp`
- **Does:** `innerOpAllowed` also rejects `assistant.project.bulkApply` (and any other
  dispatcher) as an inner op. The schema text at `ToolSchemas.cpp:301-302` already said so and
  now matches the behavior, so that file is not edited.
- **Verify:** a read-back.
- **Deps:** none
- [x] done

### T24 — The approval card lists a batch's ops

- **Files:** `core/Ui/AI/Conversation/ToolTurnRunner.cpp`, `app/rcc/chat-viewer.html`,
  `app/qml/AI/ToolCallCard.qml`, `app/qml/AI/MessageBubble.qml` and `app/qml/AI/MessageWebView.qml`
  (added 2026-10-14: the first passes the card's fields into `ToolCallCard`, the second translates
  the web card's labels). Review fix: ops stricter than Confirm are always listed, whatever the
  50-op cap, so the op that raised the batch's tier cannot hide behind "+N more".
- **Does:**
  - `appendCard` adds `ops` (command plus a one-line args summary, capped) for any call with
    planned commands.
  - The HTML renders it as its own block above the Approve buttons, outside the collapsed body
    (the `payloadPreview` branch is the device-write preview, so it was not reused).
  - The QML card shows the same list above the collapsed args.
- **Verify:** `scripts/code-verify.py --check` on the QML file; the maintainer observes AC5's
  card.
- **Deps:** T21
- [x] done

### T25 — Skill text for batch approval

- **Files:** `app/rcc/ai/skills/project_basics.md`
- **Does:** States that a batch takes its strictest op's tier, that a blocked op denies the
  whole batch, and that the card lists every op, so the model plans batches that will run.
- **Verify:** `documentation-verify.py` on the file.
- **Deps:** T22
- [x] done

### T26 — Assistant batch tiers (AC5)

- **Files:** `app/tests/tst_assistant_batch_tiers.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:**
  - Loads `command_safety.json` into `AI::CommandRegistry` and checks `effectiveSafety` for a
    batch holding a Blocked op, a device-gated op with device control off and on, and an
    AlwaysConfirm op. Each must equal the tier of the same op called directly.
  - Also checks that a nested dispatcher counts as Blocked, and that the fixed forwarders
    (`assistant.script.apply`, `assistant.workspace.addTile`) plan their commands and stay
    Confirm. Amended 2026-10-14: `assistant.script.apply` has no control-script kind; its
    setters are all Confirm, so it was never a bypass.
  - `app/tests/support/ai-safety.qrc` (new) aliases the shipped `command_safety.json`, as
    `core-icons.qrc` does for icons.
- **Verify:** the maintainer runs ctest.
- **Deps:** T21
- [x] done

## M5: Viewer runs no publisher logic

### T27 — stripPublisherLogic

- **Files:** `core/Pipeline/DataModel/Project/ProjectSealing.{h,cpp}`,
  `app/tests/tst_publisher_logic_strip.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:**
  - Removes from a project document: every source's parser code and params; the legacy root
    `frameParser`; dataset transform code and params; both transform libraries; the control
    script; output `transmitFunction`; connection settings; `mqttPublisher`; `influxSink`.
  - Keeps `painterCode`, the layout and every display field.
  - The test fixture carries every key the loader reads, and asserts each is gone and
    `painterCode` survives.
- **Verify:** the maintainer runs ctest.
- **Deps:** none · **Main session** (key list)
- [x] done

### T28 — The viewer strips before it loads

- **Files:** `core/Api/API/Mirror/MirrorSession.cpp`
- **Does:** `applyRemoteState` runs `stripPublisherLogic` on the `project` object before
  `loadFromJsonDocument` (`:543`). *Invariant:* dataset order and `wireUniqueId` are untouched,
  so there is no `kWireVersion` bump and no fixture regeneration.
- **Verify:** a read-back; `tests/integration` mirror protocol tests stay in the maintainer's
  run.
- **Deps:** T27 · **Main session**
- [x] done

### T29 — Canvas on a viewer only draws

- **Files:** `core/Ui/UI/Widgets/Painter.{h,cpp}`
- **Does:**
  - Built while `API::MirrorSession::mirroring()` is true, the constructor installs
    InertScriptSurface in place of the live sequence (`:161-171`) and records that choice.
  - `compile()` refuses to evaluate user code when mirroring is on but the engine is live, and
    draws the fallback instead.
  - `__pp`, `ctx` and `theme` are unchanged. A draw-only painter takes no table-API user, so
    its destructor releases none.
- **Verify:** a read-back; layer check (Ui may include Api).
- **Deps:** T6 · **Main session**
- [x] done

### T30 — Hostile publisher fixture (AC6) — BLOCKED

- **Files:** `tests/manual/mirror_hostile_publisher.py` (new; not written)
- **Does:** A fake v1 publisher serving a project whose parser, transforms, control script,
  transmit function and Canvas code each try a marker effect, for the maintainer to attach a
  viewer to.
- **Status (2026-10-14):** a safety classifier stopped the subagent's write of this attack
  harness, and it was not retried or worked around. The decision is the maintainer's.
- **Proposed instead:** the R10 half of AC6 is covered by `tst_publisher_logic_strip` (every
  logic key, including legacy `frameParser`, removed; `painterCode` kept), and R11 by the Painter
  draw-only path. The end-to-end check becomes a maintainer observation with two real instances:
  publish an example project that has a Canvas widget from one Serial Studio, attach a viewer
  from the other, and confirm the Canvas and data widgets draw.
- **Deps:** T28, T29
- [ ] done

## M6: Restore accepts only the app's own checkpoints

### T31 — Checkpoint validation

- **Files:** `core/Ui/Misc/BackupManager.{h,cpp}`, `core/Ui/ApiHandlers/AssistantHandler.cpp`
  (amended 2026-10-14)
- **Does:** `BackupManager::isOwnCheckpoint(path)` accepts only:
  - an existing file whose canonical folder is the open project's `backupDirectory()`;
  - named the way `snapshot()` names them (timestamp, optional `__label`, `.ssproj`);
  - carrying `_backupMeta`.

  `assistant.restore` refuses anything else, before the pre-restore snapshot or any load.
  *Invariant:* a refusal leaves the open project and its file untouched.
- **Amendment:**
  - The check sits on the API path, not inside `BackupManager::restore`, because the local
    Backup Recovery dialog also calls `restore()` and R7 is about the API.
  - The stored sha1 is not verified. A local writer could forge it too (the confused-deputy
    threat names files; it does not write them), and re-serializing the JSON could refuse a
    genuine checkpoint.
- **Verify:** the name pattern checked with PySide6 `QRegularExpression` against `snapshot()`
  names and foreign ones; a read-back of the handler order (check, snapshot, restore, save).
- **Deps:** none · **Main session**
- [x] done

### T32 — Project loads ask consent (AC2, AC3)

- **Files:** `tests/security/test_project_load_consent.py` (new)
- **Does:**
  - Against a headless instance with no AUTO_CONSENT, `sessions.regress` (with
    `projectJson`), `sessions.verify`, `sessions.replay` and `assistant.restore` on a
    marker-script project are each refused with the script-change denial. No marker appears.
  - `assistant.restore` with a foreign path is refused, and the project file's hash is
    unchanged.
  - The `backupPath` of a mutation still restores. That part runs under AUTO_CONSENT, in a
    second marked group.
- **Verify:** `python -m py_compile`; the maintainer runs it. Guards: the denial cases run only
  with `SS_TEST_HEADLESS_NO_CONSENT=1`, the positive restore case only with
  `SS_TEST_AUTO_CONSENT=1`.
- **Deps:** T3, T31
- [x] done

## M7: Honest docs

### T33 — API and dashboard help pages

- **Files:** `doc/help/API-Reference.md`, `doc/help/Remote-Dashboard.md`,
  `doc/help/Painter-Widget.md`
- **Does:**
  - States which consent each capability class needs, and that dry runs have no side effects
    (and apiCall's `dryRun` reply).
  - Rewrites the trust model: viewers never run publisher logic, Canvas on a viewer can only
    draw, and the publisher sends its whole project, not "structure only".
  - Notes that checkpoints restore only for the open project.
- **Verify:** `documentation-verify.py` on the three files reports no new findings (AC9).
- **Deps:** T3, T9, T28, T29, T31
- [x] done

### T34 — Process and scripting help pages

- **Files:** `doc/help/Drivers-Process-IO.md`, `doc/help/JavaScript-API.md`
- **Does:** Documents that a Process source's first start asks the project's launch consent,
  that a "No" holds until the project is reopened, and the headless command-line rule. Also
  states that apiCall in dry-run code returns `dryRun`.
- **Verify:** `documentation-verify.py`; the trial-parity lint stays clean.
- **Deps:** T17, T18, T9
- [x] done

### T35 — Architecture docs: launch and scripting

- **Files:** `doc/claude/architecture/io.md`, `doc/claude/architecture/scripting.md`
- **Does:**
  - io.md "Opening a Link" gains the launch-authorizer contract: one spawn gate, any origin,
    the reconnect on grant, and the headless grant.
  - scripting.md documents the Inert surface: dry runs never get live bridges, plus the
    `verify()` rule.
- **Verify:** `claim-verify.py` reports 0 errors.
- **Deps:** T16, T17, T7
- [x] done

### T36 — Architecture docs: assistant and mirror

- **Files:** `doc/claude/architecture/ai.md`, `doc/claude/architecture/mirror.md`
- **Does:**
  - ai.md: the effective tier for dispatchers, and the policy scope that re-checks nested
    calls (and why it is separate from `RemoteDispatchScope`).
  - mirror.md: the viewer-side strip, Canvas draw-only, and no wire bump.
- **Verify:** `claim-verify.py` reports 0 errors.
- **Deps:** T22, T28, T29
- [x] done

### T37 — CLAUDE.md contract row

- **Files:** `CLAUDE.md`
- **Does:** The "An API command that installs script code, launches a process, or re-dispatches
  other commands" row now says:
  - a new command needs a capability in `API::CommandCapabilities` (Enforced:
    ctest:tst_command_capability_registry);
  - a new spawn goes through `IO::authorizeLaunch` / `ProcessLauncher`;
  - assistant nesting goes through `DispatchPolicyScope`.
- **Verify:** `claim-verify.py` reports 0 errors.
- **Deps:** T2, T16, T20 · **Main session**
- [x] done

## Review follow-ups (qt-cpp-review, 2026-10-14)

Applied in the first pass: the approval card always lists ops stricter than Confirm; the web
card's labels are translated; the transform editor's unpaired table-API leases and live-store
refresh are gone; Painter keeps its surface alive on install failure; the scratch store no longer
reads the GUI mirror; one endToEnd call shares one surface; the two project strips share one core;
a denied launch clears its pending reconnect; inert Lua closes no longer marshal into the
pipeline; every dispatch scope shadows a caller's policy; an unbound launch authorizer refuses
without aborting; the R13 signal is `remoteProgramChanged`.

Applied in the second pass (adversarial findings; protected surface, named in chat before
landing):
- **Lua sandboxes accept text only.** The inert engine and `ScriptDryRun` load their top-level
  chunk with `luaL_loadbufferx(..., "t")` and drop `loadstring` (`load`/`dofile`/`loadfile`/
  `string.dump` were already gone), so dry-run code cannot load a precompiled chunk. The live,
  user-authored path keeps the default loader. Confirmed the "t" mode rejects a binary chunk in
  the bundled LuaJIT.
- **`extensions.addRepository` is `InstallsScript`.** With auto-update on, adding a repository can
  replace installed extension code, so a remote add now needs the script-change consent (locked
  by `tst_server_auth`).
- **Assistant tiers are argument-aware for program choice.** `argumentAwareSafety` escalates a
  call whose arguments pick a program or write to a device to the device-gated treatment, so a
  generic `project.source.setProperty{key:"executable"}` is gated like `io.process.setExecutable`.
  Used by both the card tier and the runtime nested re-check.
- **Script setters are always-confirm for the assistant.** The parser/transform/library/painter
  setters move to `alwaysConfirm`, beside `controlScript.setCode`, so an auto-approve cannot run a
  full-trust install silently.

Open, for the maintainer to decide (not changed; each a decision already made or a larger design):
- R13's once-per-session clear can be spent by a harmless program change before a harmful one,
  because launch consent is per project, not per program (the model chosen on 2026-10-14). Closing
  it means per-program consent, which the spec did not take.
- The `assistant.script.apply` / addTile fan-out lists restate their tools' command lists (drift
  risk only; the runtime policy re-checks every nested call, and the capability sweep covers the
  names).

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` (AC1–AC11) is met and checked off there; the
      runtime ones are marked pending the maintainer's build and run.
- [ ] `python3 scripts/code-verify.py --check` is clean on all changed files (no new errors),
      with `--tu-census --check`, `--singleton-census --check` and `layer-verify.py` clean.
- [ ] `syntax-check.py` passes on every edited or new TU, and each SKIP is named at handoff.
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `--benchmark-hotpath` not regressed against the spec-0097 T0 table (AC10, the maintainer
      runs it).
- [ ] The maintainer's run list is handed over: the new ctest suites,
      `tests/security/test_caller_code_inert.py`, `test_project_load_consent.py` and
      `test_process_launch_consent.py` (headless, no AUTO_CONSENT),
      `test_remote_dispatch_gate.py`, then `pytest tests/integration/ -v` (AC8).
- [ ] `python3 scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [ ] The diff is *what was asked, and only that*: no foreign files touched, and every
      protected-surface change was named with its failure modes before it landed.
- [ ] `spec.md` status set to `done`.
