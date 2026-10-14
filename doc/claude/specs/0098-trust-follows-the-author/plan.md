---
spec: 0098-trust-follows-the-author
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-10-14
---

# Plan 0098 — Code never runs with more trust than whoever supplied it

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Four mechanisms, each placed at the one point every path already crosses.

1. **A compiled capability table.** `API::CommandCapabilities` classifies every registered
   command, with a few argument-aware refinements. The remote gate in `ServerAuth` decides from
   it instead of from AuthPrimitives' name lists, and refuses an unclassified command. A CI
   sweep over the committed API snapshot keeps it exhaustive, following the pattern of spec
   0096's `SealedPolicy`.
2. **An inert script surface.** Dry runs and compile checks build their engines on it. The live
   bridges are never installed. Stubs are installed instead, plus a table bridge over a scratch
   copy of the table mirror, so results match today's and nothing leaves the engine.
3. **Launch consent at the spawn point.** A Process source asks the same per-project launch
   consent as `system.exec`. It asks through a root-bound Core interface, because Devices
   cannot include Api.
4. **The assistant's tiers on nested commands.** They are applied through a second dispatch
   scope, separate from `RemoteDispatchScope`, which licensing reads to keep its prompts quiet.
   On top of that, the assistant's pre-flight computes a batch's strictest tier and lists its
   ops on the approval card.

The mirror viewer strips every logic field from the publisher's project before loading it, and
builds Canvas engines with draw-only bridges.

## Affected subsystems & files

Grouped by milestone; every path was confirmed by grep. **P** marks a protected surface (API
auth/consent), which is named with its failure modes before it lands and never batched.

**M1: Capability classification and the remote gate (R1–R4, R6, R13 hook)**

| File | Change |
|------|--------|
| `core/Api/API/CommandCapabilities.h/.cpp` (new) | Constexpr table, one entry per command: its capability set, plus a `Conditional` flag that routes to `refine(command, params)`. Lookup returns `std::optional` (empty means unclassified). |
| `core/Api/API/Server/AuthPrimitives.h/.cpp` **P** | Remove the three classification predicates and their name lists, moved into the table. The token functions stay. |
| `core/Api/API/Server/ServerAuth.h/.cpp` **P** | Rewrite `authorizeRemoteCommand` over capabilities (gate map below). Add a `remoteProgramChanged()` signal, emitted once a command that changes scripts, loads a project or chooses a program has cleared every gate. |
| `core/Api/API/Server.cpp` **P** | Connect `ServerAuth::remoteProgramChanged` to `ProcessLauncher` (same library, GUI thread, direct). |
| `app/tests/tst_command_capability_registry.cpp` (new), `app/tests/CMakeLists.txt` | AC7 sweep: every schema command classified once, no stale names, every conditional entry resolves both ways, a synthetic unclassified name is refused. |
| `app/tests/tst_server_auth.cpp` | Port `commandClassification` / `scriptInstallClassification` to the table, and add rows for project loads, program choice and the signal. |

**M2: Inert surface for dry runs and compile checks (R5)**

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/Scripting/InertScriptSurface.h/.cpp` (new) | Installs into a QJSEngine or a lua_State:<ul><li>`apiCall` stubbed, returning `{ok:false, error:'dryRun'}` as `controlScript.dryRun` does;</li><li>no-op recorders for deviceWrite, actionFire, dashboard, notify and mqttPublish;</li><li>a table bridge over a scratch `DataTableStore` at the project's starting values;</li><li>console output dropped (amended 2026-10-14: API callers never received it, so no reply field is added).</li></ul>Then `verify()` asserts that every bridge global is the stub. |
| `core/Pipeline/DataModel/Scripting/ScriptApiCall.h/.cpp` | `installAll` gains a `Surface` argument (`Live` by default, which keeps today's exact call sequence). `Inert` delegates to InertScriptSurface. |
| `core/Pipeline/DataModel/Scripting/JsScriptEngine.h/.cpp`, `LuaScriptEngine.h/.cpp` | Constructor takes `Surface` (default `Live`). Inert engines skip the timeout message box and the qWarning-to-notification path, and Lua always uses the Safe deadline and budget. |
| `core/Pipeline/DataModel/Scripting/FrameParserPipeline.h/.cpp` | The with-code dry-run entry builds Inert engines. |
| `core/Api/API/Handlers/ProjectDryRunCommands.cpp` | `makeScriptEngine` and endToEnd's transform engines are Inert. |
| `core/Api/API/Handlers/ProjectFileCommands.cpp` | `project.validate` compiles on Inert engines. |
| `core/Pipeline/DataModel/Scripting/TransmitScriptEnvironment.cpp` | The Judging surface also masks `__nc` and `__ss_db`, which it leaves live today. |
| `core/Pipeline/DataModel/Scripting/ScriptDryRun.cpp` | Lua state strips `dofile` and `loadfile`, matching `LuaScriptEngine.cpp:120`. |
| `core/Ui/ProjectEditor/Editors/DatasetTransformEditor.cpp` | Editor previews (`:1034`, `:1144`) install the Inert surface. |

**M3: Launch consent at the spawn point (R8, R13)**

| File | Change |
|------|--------|
| `core/Core/IO/LaunchAuthorizer.h/.cpp` (new) | `IO::ILaunchAuthorizer::authorizeLaunch(program, arguments, workingDir)` returns a Core-level verdict (Allowed / ConsentRequired / Denied). It comes with `setLaunchAuthorizer` / `launchAuthorizer` binders, following `DataModel::setMqttPublisher`. If nothing is bound, the launch is denied and an assert is logged. |
| `core/Devices/IO/Drivers/Process.cpp` | `open()` in Launch mode asks the authorizer after resolving the executable and before `m_process->start` (`:283`). Anything but Allowed logs the reason and returns false. NamedPipe mode is unchanged. |
| `core/Api/API/LaunchConsent.h/.cpp` (new) **P** | Per-project launch consent, extracted from ProcessLauncher so it can be unit-tested. It owns the `ConsentGate`, the R13 policy (a session taint plus the set of keys cleared this session), and the headless operator grant. |
| `core/Api/API/Server/ConsentGate.h/.cpp` **P** | `revoke()`: state back to Unset and the persisted key removed. |
| `core/Api/API/ProcessLauncher.h/.cpp` **P** | Owns a `LaunchConsent` and implements `ILaunchAuthorizer`; `launch()` uses the same consent. When the consent is granted after a refused Process start, it queues one `connectDevice()`. Adds a slot for `remoteProgramChanged`. |
| `core/Api/API/ProcessLauncher.cpp` (wiring) | Binds itself as the authorizer when wired (amended 2026-10-14: `bindInterfaces()` would grow the singleton census). |
| `app/src/Misc/CLI.cpp`, `core/Core/Runtime.{h,cpp}` | A headless run records its command-line project as `Core::Runtime::operatorProject()`, and the launcher hands it to `LaunchConsent`. |
| `app/tests/tst_launch_consent.cpp` (new), `app/tests/tst_consent_gate.cpp` | AC11 plus R13's taint and save-as cases, and `revoke()`. |

**M4: Assistant tiers on every op (R9, R4)**

| File | Change |
|------|--------|
| `core/Api/API/CommandRegistry.h/.cpp` | New `DispatchPolicyScope`, an RAII stack beside `RemoteDispatchScope` whose policy receives the nesting depth. `execute` consults it after the remote scope. `RemoteDispatchScope` itself is untouched. |
| `core/Api/API/CommandHandler.cpp` | The trusted (non-remote) path also opens an empty policy scope, so consented scripts keep full trust under an assistant call. |
| `core/Ui/AI/CommandRegistry.h/.cpp` | `plannedCommands(name, args)` lists what a forwarder will run: the batch's and bulkApply's ops, `assistant.script.apply`'s setter for its `kind`, and addTile's five commands. `effectiveSafety(name, args)` is the strictest of the outer command and its planned commands; a planned dispatcher counts as Blocked. |
| `core/Ui/AI/Conversation.cpp` | `dispatchByCallSafety` uses `effectiveSafety`. The approved tier travels with the call. |
| `core/Ui/AI/Tools/ToolDispatch.cpp` | `executeCommand` opens a `DispatchPolicyScope` that refuses, for every registry call under it, Blocked commands, commands above the approved tier, and a dispatcher at depth > 0. |
| `core/Ui/AI/Tools/ToolBulkTools.cpp`, `ToolSchemas.cpp` | `innerOpAllowed` also rejects dispatchers, which closes bulkApply-in-bulkApply. Fix the schema text that claims this already. |
| `core/Ui/AI/Conversation/ToolTurnRunner.cpp`, `app/rcc/chat-viewer.html`, `app/qml/AI/ToolCallCard.qml` | The card carries `ops` (command plus a short args summary) for dispatchers. It renders through the unused `payloadPreview` branch in HTML and an ops list in QML. |
| `app/rcc/ai/skills/project_basics.md` | Explains the batch approval: the strictest op sets the tier and the card lists every op. |
| `app/tests/tst_assistant_batch_tiers.cpp` (new) | AC5. |

**M5: Viewer runs no publisher logic (R10, R11)**

| File | Change |
|------|--------|
| `core/Pipeline/DataModel/Project/ProjectSealing.h/.cpp` | `stripPublisherLogic(QJsonObject&)` removes the logic fields and keeps `painterCode`. Removed: the per-source parser code and params, the legacy root `frameParser`, transform code and params, both transform libraries, the control script, `transmitFunction`, connection settings, `mqttPublisher` and `influxSink`. |
| `core/Api/API/Mirror/MirrorSession.cpp` | `applyRemoteState` strips before `loadFromJsonDocument` (`:543`). |
| `core/Ui/UI/Widgets/Painter.h/.cpp` | Built while `API::MirrorSession::mirroring()` is true, the constructor installs the Inert surface instead of its live sequence (`:161-171`). `compile()` refuses to evaluate user code if mirroring is on but the engine is live, so it fails closed. |
| `app/tests/tst_publisher_logic_strip.cpp` (new) | Every code-bearing key, including legacy `frameParser`, is gone; `painterCode` and layout survive. |
| `tests/manual/mirror_hostile_publisher.py` (new) | A fake v1 publisher whose project's parser, transform and Canvas code try marker effects. Used for AC6. |

**M6: Restore accepts only the app's own checkpoints (R7)**

| File | Change |
|------|--------|
| `core/Ui/Misc/BackupManager.{h,cpp}`, `core/Ui/ApiHandlers/AssistantHandler.cpp` | `isOwnCheckpoint()` accepts only a file in the open project's backup folder, named like a snapshot, carrying `_backupMeta`, and `assistant.restore` refuses anything else before writing (amended 2026-10-14: the check is on the API path because the local recovery dialog also restores, and the sha1 is not verified). |

**Integration tests (AC1–AC4)**

| File | Change |
|------|--------|
| `tests/security/test_caller_code_inert.py` (new) | AC1. |
| `tests/security/test_project_load_consent.py` (new) | AC2, AC3. |
| `tests/security/test_process_launch_consent.py` (new) | AC4. Commercial builds only. |

**M7: Honest docs (R12)**

| File | Change |
|------|--------|
| `doc/help/API-Reference.md` | Consent per capability class; dry runs have no side effects. |
| `doc/help/Remote-Dashboard.md` | Trust model: the viewer runs no publisher logic and Canvas code only draws. Correct the "structure only" claim. |
| `doc/help/Painter-Widget.md` | Canvas on a viewer is draw-only. |
| `doc/help/Drivers-Process-IO.md` | First start asks the project's launch consent, plus the headless rule. |
| `doc/help/JavaScript-API.md` | Dry runs and apiCall. |
| `doc/claude/architecture/io.md`, `ai.md`, `mirror.md`, `scripting.md` | Contracts for the new seams. |
| `CLAUDE.md` | The "installs script code, launches a process" Subsystem Contracts row points to the capability table, its ctest, and the launch authorizer. |

## Architecture & data flow

**Remote gate map** (`ServerAuth::authorizeRemoteCommand`, the one policy for outer and nested
remote calls). Gates apply in this order, and every one that applies must pass:

| Capability | Gate | Members (non-exhaustive) |
|---|---|---|
| unclassified | refused (R3) | any name missing from the table |
| `ControlScriptOnly` | refused | `system.exec`, `system.kill`, `system.runningProcesses` |
| `InstallsScript` | script-change consent | the setters and loaders of today's set; `extensions.install` (found while planning: it installs extension code from any repository URL) |
| `LoadsProject` | script-change consent | `project.open`, `loadJson`, `template.apply`, `assistant.restore`, `sessions.regress`, `sessions.verify`, `sessions.replay` |
| `WritesDevice` | device-write consent | today's set, including the `project.action.update` refinement |
| `ChoosesProgram` | none here; the launch asks (R8) | `io.process.setExecutable` / `setArguments` / `setWorkingDir` / `setMode`. Conditional: `project.source.setProperty` / `setProperties` (keys `executable`, `arguments`, `workingDir`, `mode`), `io.setBusType` and `project.source.update` (busType Process) |
| `StartsProgram` | none here; the launch asks (R8) | `io.connect` |
| `RunsInertCode` | none: the surface is inert (R5) | the eight dry runs, `project.validate`, `assistant.script.dryRun` |
| `Dispatches` | each inner command is gated | `project.batch`, `assistant.project.bulkApply`, `assistant.script.apply` |
| none of the above | allowed | reads, UI state, driver config |

Once every gate has passed, a command that changes scripts, loads a project or chooses a
program emits `remoteProgramChanged()`.

**Launch consent (R8, R13).**

- *Every start asks.* `Process::open` (GUI thread) calls `IO::launchAuthorizer()->authorizeLaunch(...)`.
  That is `ProcessLauncher` → `LaunchConsent::authorize(detail)`, the same `ConsentGate` and
  per-project key (`API/ScriptExecConsent/<sha1(path)>`) that `system.exec` uses.
  - An unanswered consent posts the existing prompt, with the program and its arguments as the
    detail line, and the connect fails with "Waiting for permission to launch …".
  - When the prompt is granted, `ProcessLauncher` queues one `connectDevice()` if a refused start
    is pending and the link is still down. It is queued so the reconnect never runs under the
    prompt's stack (the K13 precedent).
- *A remote change clears consent once per project per session.*
  - `remoteProgramChanged()` sets `LaunchConsent`'s session taint and clears the current key once
    for this session.
  - While tainted, each key the gate later rebinds to (a save-as or a load) is also cleared once.
    This closes "load a hostile document, then save it over a file that already has consent".
  - A key cleared and granted again stays granted for the rest of the session.
- *Overrides.* AUTO_CONSENT keeps granting and makes clearing a no-op. In a headless run, the
  command-line project path starts Granted in memory, and R13 still applies to it.

**Inert surface (R5).** An engine built Inert never receives a live bridge.

- **Table writes.** They land in a scratch store laid out from the project at its starting
  values (amended 2026-10-14 in review: reading the GUI mirror registers a permanent GUI table
  user, so the live values are not copied). One endToEnd call shares one surface across its
  engines, so a transform's `tableSet` is visible to the next as it is live. There is no routed
  read into the pipeline thread, and therefore no nested event loop.
- **Everything else effectful** is a recorder. The surface's `verify()` asserts after install,
  failing loudly in debug and refusing to run in release.
- **The bare-engine dry runs** (painter, outputWidget, controlScript, transformLibrary) already
  avoid the live bridges. Only the Judging masks and the Lua file loaders change.

**Assistant (R9).** `Conversation::dispatchByCallSafety` → `effectiveSafety`.

- If a planned op is Blocked or is itself a dispatcher, the call is denied before anything runs,
  and the denial card names the op.
- If a planned op is AlwaysConfirm, the call needs a click even with auto-approve on.
- Approval → `ToolDispatch::executeCommand` opens a `DispatchPolicyScope` holding the approved
  tier. Every `CommandRegistry::execute` under it is re-checked against that tier.
- A script's `apiCall` dispatches as Trusted. The trusted path shadows the policy scope, so a
  consented control script woken by an approved `io.connect` keeps full trust.

**Viewer (R10, R11).**

1. `MirrorSession::applyRemoteState` strips the document, then loads it. The parser, transforms,
   control script and transmit code are gone before `frameParserCodeChanged` or
   `compileTransforms` fire.
2. Canvas widgets are built after attach (`s_mirroring` is set before `attachedChanged`), so they
   get the Inert surface. Drawing runs through `__pp`, `ctx` and `theme`, which never needed a
   bridge.
3. Wire format: the project is opaque, and dataset order and `wireUniqueId` are unchanged, so
   there is no `kWireVersion` bump and no fixture regeneration.

## Hotpath & threading impact

- **Touches the hotpath?** No.
  - `JsScriptEngine` and `LuaScriptEngine` (FrameParser's engines) gain a constructor argument.
    FrameParser keeps the default `Live`, whose install sequence stays byte-for-byte today's.
    The branch runs once per engine construction, never per frame.
  - `FrameBuilder`, `CircularBuffer`, `FrameReader` and `Dashboard` are not edited.
  - The scratch table store belongs to the dry-run engine. It never touches the builder's store
    or the SPSC ring.
  - AC10 runs `--benchmark-hotpath` as the guard.
- **New cross-thread signal/slot?** No.
  - `ServerAuth::remoteProgramChanged` → `ProcessLauncher` connects two GUI-thread objects
    directly.
  - The reconnect after a grant is a queued call on the GUI thread itself (re-entrancy, not
    threading).
  - `Process::open`'s authorizer call runs on the GUI thread, which the driver already uses for
    its QProcess. The authorizer asserts it.
- **New input to a cached hotpath flag?** None.
- **Timestamp ownership.** Untouched; no driver read path changes.

## Data model & persistence

- No project-JSON, `Frame.h` key or Historian schema change. `stripPublisherLogic` only removes
  keys from a document the viewer would otherwise load.
- **QSettings.**
  - The launch-consent keys keep their name, and `revoke()` removes one.
  - `ProcessDriver/*`, the global saved Process settings, are unchanged. A new instance still
    inherits them, but starting now needs consent. The inheritance itself is a candidate for
    spec 0099.
- **Checkpoints.** The `_backupMeta` sha1, written since format 1, is now verified on restore.
  Checkpoints without `_backupMeta` are refused through the API.

## API / SDK surface

- No new commands, error codes or schema changes.
  - Unclassified refusals use `EXECUTION_ERROR`: "… has no capability classification".
  - Consent refusals reuse today's `CONSENT_REQUIRED` and "denied by the user" responses.
- **New consent on existing commands** (remote clients only):
  - `assistant.restore`, `sessions.regress`, `sessions.verify`, `sessions.replay`,
    `extensions.install`: script-change consent.
  - Any Process start, from any origin: launch consent.
- **Dry-run replies.** Unchanged in shape. `apiCall` inside dry-run code
  returns `{ok:false, error:"dryRun"}`.
- **Not a new gate.** `api-schema.json` is not regenerated by hand; the new sweep reads the
  committed snapshot as `tst_sealed_policy_registry` does.

## QML / UI

- **Approval card.** It gains an ops list for dispatchers, in both renderers (`chat-viewer.html`
  through its unused `payloadPreview` branch, and `ToolCallCard.qml`). There are no new models,
  settings or restore-race paths.
- **Launch consent prompt.** It is the existing `ConsentGate` message box, with the Process
  program and arguments as its detail line.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the classification lives | (a) constexpr table beside SealedPolicy; (b) a required argument on `registerCommand`; (c) a JSON resource | **(a)**. (b) touches 446 registration sites across ~40 handler files, which is a bulk diff on a protected surface. (c) fails soft if the resource is missing, as `command_safety.json` does today (everything becomes Confirm). |
| Merge with `SealedPolicy` | One table with a sealed column, or two sweeps over one snapshot | **Two**. The spec keeps 0096 unchanged, and that table landed one commit ago. |
| `LoadsProject` consent | Ask always, or only when the document holds scripts | **Always**, as `project.open` does today. Content-aware gating would make the gate parse caller files and databases. |
| Making dry runs inert | (a) never install live bridges; install stubs; (b) install live, then mask (Judging); (c) run every dry run on the bare `ScriptDryRun` engine | **(a)**. (b) is how Judging leaked `__nc`/`__ss_db`, which proves masks miss things. (c) loses parity with the real engines' extraction and `parse()` probes, so diagnostics would change. |
| Tables in a dry run | Scratch store at the project's starting values; refuse writes; live (today) | **Scratch** (amended in review: not seeded from the GUI mirror, which would register a permanent GUI table user). Refusing writes changes stateful transforms' results, and live writes are the bug. |
| A registry-level backstop refusing apiCall during any dry run | Add / skip | **Skip**. A dry run that spins a nested event loop can run a live control script, whose consented apiCall would then be refused. `verify()` plus the AC1 sweep catch a missed stub instead. |
| Process launch gate | At the spawn for every origin, or at the API boundary only | **Spawn** (maintainer, 2026-10-14). One check covers setters, bus switches that reuse the saved program, loads, reconnects and disk projects. |
| How Devices reaches the consent | A Core interface bound by the root; a `std::function` probe like License's | **Interface**, matching `IMqttPublisher` and `IDeviceWriter`: typed, and asserted when unbound. |
| R13 scope | Clear only the key current at the change; or taint the session and clear each rebound key once | **Taint**. With only the current key, "load a hostile document, then `project.save` it over a consented file" inherits that file's grant. Cost: after a remote change, opening another consented project asks once. |
| Headless launches | A command-line project counts as operator consent for the run; or require AUTO_CONSENT | **Command-line grant**. AUTO_CONSENT grants every gate, which is too broad for a production headless box. |
| Assistant nested gate | (a) a second scope stack; (b) give `RemoteDispatchScope` a kind | **(a)**. `RemoteDispatchScope::active() != nullptr` is the licensing probe that keeps Pro and trial prompts quiet for remote calls (`TrialGate.cpp:94`, `ProFeatureNotice.cpp:91`, `ModuleManager.cpp:1167`). Reusing it would quiet them during assistant calls, a licensing behavior change and the spec's non-goal. |
| Blocked op inside a batch | Deny the whole call before it runs, or refuse just that op | **Deny the call** (the pre-flight), so nothing partially applies. The runtime per-op refusal stays as the backstop. |
| `assistant.script.apply` forwarding to setters | Re-check its setters' tiers | Amended 2026-10-14: it has no control-script kind, and its three setters are Confirm like itself, so `effectiveSafety` leaves it at Confirm. No behavior change. |
| Where the viewer strips | Viewer side, publisher side, or both | **Viewer side**: it is the only side that protects the viewer from a hostile or spoofed publisher. Publisher-side stripping would only add confidentiality, which is not a goal. |
| Restore authenticity | Directory, name and metadata checks; or a MAC with a machine key | **Checks** (sha1 dropped 2026-10-14, see T31). The threat is a confused deputy (a remote or AI caller naming a file). A local process that can write the backup folder can already run code. |
| Lua dry runs in Fast mode, which have no deadline today | Always use the Safe deadline in dry runs, or leave it | **Safe deadline**. Strictly adjacent to R5, so it is named here for veto: a remote dry run of `while true do end` freezes the GUI thread for good. |

## Risks & mitigations

Failure modes on the protected surface, named before landing:

- **Gating too broadly breaks integrations.**
  - What changes: dry runs stay ungated but inert. New prompts appear only on project-loading
    commands, `extensions.install` and Process starts.
  - What doesn't: AUTO_CONSENT still grants everything (AC8). MCP and AI validation flows see
    the same diagnostics.
  - AC8 runs the full integration suite.
- **Gating too narrowly: a command slips through.**
  - Classification is mandatory and refused when missing (AC7).
  - Driver keys that pick a program no longer matter, because the launch asks whatever set them.
  - A new spawn site must go through `ILaunchAuthorizer` or `ProcessLauncher`. The CLAUDE.md
    row records this; it is unenforced debt.
- **The inert surface misses a bridge.** Three defenses:
  - one installer per engine kind, with no live path inside it;
  - `verify()` after install;
  - AC1, which runs every `RunsInertCode` command in JS and in Lua.
- **Live-surface regression in the FrameParser engines.** `Live` keeps the existing call
  sequence, and AC10 plus the full suite check it.
- **Consent latch.** `ConsentGate` keeps a "No" until the next rebind. A user who refuses a
  Process start is asked again only after reopening the project or restarting. That is today's
  `system.exec` behavior, and the help page says so.
- **Reconnect storm.** The queued `connectDevice()` fires once per grant, and only while a
  refused start is pending and the link is down.
- **A Canvas widget built before attach.** `compile()` refuses user code when mirroring is on
  but the engine is live, so it falls back to the "(no script)" template and never runs live
  code.
- **The strip misses a key.** The unit fixture carries every code key the loader reads,
  including the legacy `frameParser` that `ProjectLoader.cpp:1125` copies back.
- **Restore after a save-as.** Checkpoints recorded under the old path are refused for the new
  one. That matches R7 ("for the open project"), and the API reference says so.
- **Silent-breakage classes in play** (common-mistakes.md):
  - Signal wiring: one new GUI-thread connection. The existing ServerAuth and ProcessLauncher
    connections are read before being edited.
  - Undo scope: restore keeps its loader path, so no new mutator is added.
  - macOS modal reentrancy: the consent prompt is posted queued, the existing ConsentGate
    contract.

## Test & verification plan

- **Unit (ctest; the maintainer builds, and I may run against the existing build dir):**
  - `tst_command_capability_registry`: AC7.
  - `tst_server_auth`: the gate map and the `remoteProgramChanged` emission.
  - `tst_consent_gate`: `revoke`.
  - `tst_launch_consent`: AC11, plus the taint and save-as case, the operator grant and
    AUTO_CONSENT.
  - `tst_assistant_batch_tiers`: AC5.
  - `tst_publisher_logic_strip`: the R10 half of AC6.
  - Existing `tst_sealed_policy_registry` and `tst_path_policy_registry` stay green: AC8's
    compatibility half.
- **Integration / security (pytest; the maintainer runs the app):**
  - **Consent suites run headless.** `test_project_load_consent.py` and
    `test_process_launch_consent.py` run against a `SerialStudio --headless` instance with the
    API server on and no AUTO_CONSENT. There, every unanswered gate answers Denied
    deterministically, and no modal is left to poison later tests.
  - `tests/security/test_caller_code_inert.py`: AC1.
    - Every dry-run command plus `project.validate`, in JS and Lua.
    - Top-level code tries: a device write (checked against a loopback TCP device's byte
      counter), `controlScript.setCode`, a marker-file `system.exec`, `tableSet` (read back
      through `project.dataTable.getValue`) and a notification (unread count unchanged).
    - Valid and invalid code keep today's diagnostics.
  - `test_project_load_consent.py`:
    - AC2 for `sessions.regress` / `verify` / `replay` and `assistant.restore`, with
      marker-file scripts;
    - AC3: a foreign path is refused and the project file's hash is unchanged; a mutation's
      `backupPath` still restores.
  - `test_process_launch_consent.py`: AC4 (a) and (b), and the baud-rate control. Skipped on
    GPL builds.
  - Then `pytest tests/integration/ -v` against a normal GUI instance, for AC8.
- **Maintainer observation:**
  - AC4: a disk project asks once, then connects without asking again.
  - AC5: the card lists the batch's ops.
  - AC6: attach a viewer to `tests/manual/mirror_hostile_publisher.py`. No markers appear, and
    the Canvas and data widgets draw.
- **Hotpath:** `--benchmark-hotpath --min-fps 256000`, compared with the spec-0097 T0 table
  (AC10).
- **Docs:** `documentation-verify.py` on the edited help pages (AC9), and `claim-verify.py`
  for CLAUDE.md and the architecture docs.
- **Static, per milestone:**
  - `scripts/syntax-check.py` on every edited or new TU. New test TUs borrow a same-library
    donor's compile entry.
  - `code-verify.py --check`, `--tu-census --check`, `--singleton-census` and `layer-verify.py`.
    The new Core interface keeps Devices → Core, and Api → Core and Devices.
  - `reuse lint-file` on new files.
  - `qt-cpp-review` before handoff, and `sanitize-commit.py` before the commit.
