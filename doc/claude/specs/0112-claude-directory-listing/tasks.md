---
spec: 0112-claude-directory-listing
phase: tasks
status: approved        # draft -> approved (gate before /ss-implement)
updated: 2026-11-03
---

# Tasks 0112 — Claude directory listing: annotated MCP tools and a plugin bundle

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
- **Executor** names the task as delegable to a cheaper model against the donor it names
  (CLAUDE.md "Delegation & Cost"); a task without the line stays in the main session because
  it decides listing policy on the API layer or touches a doc the maintainer reads first.
- **Commit shape.** This file lands in its own `docs(spec):` commit before T1. Implementation
  commits in order: T1 to T4 (table of record, generator, gate), T5 (ctest), T6 and T7 (wire),
  T8 and T9 (tests), T10 to T12 (docs). The plugin repository (T13 to T18) is its own history.
  Nothing here touches the API server auth or consent files; a diff that does is a scope
  error, not a task.
- **The one security note, named once.** `isListedTool` decides *discovery* only. The remote
  gate (`API::ServerAuth::authorizeRemoteCommand`) stays the only authorizer; no task below
  changes what any command does when called.

## Tasks

### T1 — Tier the 61 untiered commands and add the `assistantTools` key

- **Files:** `app/rcc/ai/command_safety.json`
- **Does:** Adds the 61 names from the plan's "Proposed tiers" table to the five arrays (19
  `safe`, 15 `confirm`, 7 `alwaysConfirm`, 20 `deviceGated`; `blocked` unchanged). Adds
  `"assistantTools": ["fs.append", "fs.delete", "fs.list", "fs.read", "fs.search",
  "fs.write"]`, the six tiered names that are assistant sandbox verbs, not API commands.
  Extends `_comment` with one clause: the file also feeds the MCP tool hints through the
  generated `API::CommandSafety` table. Binding invariant: a name sits in exactly one of the
  five arrays, and a `deviceGated` name is never repeated in a tier array (that is how the
  397 names are laid out today). The loader (`AI::CommandRegistry::load`) validates size
  limits only and reads five named arrays, so the new key needs no schema and is ignored
  by the assistant.
- **Verify:** A scratchpad one-liner over the JSON and `app/rcc/api/api-schema.json`: the
  five arrays are pairwise disjoint; their union minus `assistantTools` equals the 452
  schema names; counts are safe 178, confirm 126, alwaysConfirm 40, deviceGated 106,
  blocked 8. `python -m json.tool` parses it. The maintainer has reviewed the table in
  `plan.md`; any change they make there is applied here, not the reverse.
- **Deps:** none
- [ ] done

### T2 — `API::CommandSafety`: the header, the lookups and the listing policy

- **Files:** `core/Api/API/CommandSafety.h` (new), `core/Api/API/CommandSafety.cpp` (new),
  `core/Api/CMakeLists.txt`
- **Does:** Header: `enum class Tier : quint8 { Safe, Confirm, AlwaysConfirm, Blocked }`,
  `struct Entry { QLatin1StringView name; Tier tier; bool device_gated; }`,
  `struct ToolHints { bool read_only; bool destructive; }`, and the declarations
  `entries()`, `assistantTools()` (both *defined by the generated TU of T3*), `tierOf`,
  `isDeviceGated`, `hintsFor`, `isListedTool`, `humanizeToolName`, every return
  `[[nodiscard]]`. Hand-written TU: `tierOf` is a `std::lower_bound` over `entries()`
  (donor `CommandCapabilities::of`); `hintsFor` maps Safe to `{true, false}`, Confirm to
  `{false, false}`, AlwaysConfirm, Blocked or device-gated to `{false, true}`, unknown to
  `{false, false}`; `isListedTool` is false for `Tier::Blocked` and for a name whose
  `CommandCapabilities::of(name, {})` carries `ControlScriptOnly`; `humanizeToolName` splits
  on `.`, maps a whole lowercase segment through the acronym table first (`api` API, `ble`
  BLE, `canbus` CAN Bus, `csv` CSV, `eip` EtherNet/IP, `hid` HID, `iec104` IEC 104, `io` IO,
  `mdf4` MDF4, `mqtt` MQTT, `opcua` OPC UA, `s7` S7, `uart` UART, `ui` UI, `usb` USB),
  otherwise splits camelCase, maps the pieces (`Ansi`, `Csv`, `Db`, `Dtr`, `Fd`, `Fps`,
  `Http`, `Id`, `Iso`, `Json`, `Tcp`, `Tls`, `Udp`, `Url`, `Uuid`, `Vt` to upper case) and
  capitalizes the rest, joining pieces with a space and segments with ` / `
  (`project.dataset.setTransformCode` is `Project / Dataset / Set Transform Code`). CMake:
  the two files plus T3's generated TU beside `CommandCapabilities.*` in both lists. Binding
  invariants: Api layer, so the includes are `API/CommandCapabilities.h`, Qt Core and the
  standard library only, no resource, no reach into `core/Ui`; `isListedTool` describes,
  it never authorizes.
- **Verify:** `python scripts/syntax-check.py core/Api/API/CommandSafety.cpp` (a SKIP means
  "compiles" is unverified and is said so), `python scripts/code-verify.py --check
  core/Api/API/CommandSafety.h core/Api/API/CommandSafety.cpp core/Api/CMakeLists.txt`,
  `python scripts/layer-verify.py`.
- **Deps:** none
- [ ] done

### T3 — The generator and the generated table

- **Files:** `scripts/generate-command-safety.py` (new), `core/Api/API/CommandSafetyTable.cpp`
  (generated, new), `doc/claude/scripts.md`
- **Does:** Donor `scripts/generate-command-strings.py`. Reads the JSON; exits 2 naming the
  offender for a name in two arrays, a non-ASCII name, or a name over 64 characters. Writes
  the dual-license header, the "AUTO-GENERATED ... regenerate, never edit" banner,
  `#include "API/CommandSafety.h"`, a byte-order-sorted `constexpr std::array<Entry, N>
  kEntries` of `{QLatin1StringView, Tier, device_gated}` (byte order is what
  `QLatin1StringView` compares by, and `sorted()` on ASCII strings agrees), a sorted
  `kAssistantTools`, and the two accessors `entries()` / `assistantTools()`. `--check`
  renders in memory, compares with the file and exits 1 on drift. LF endings, black-clean,
  CWD-independent. `scripts.md`: one row for the generator and the generator's place in the
  `sanitize-commit.py` pipeline sentence.
- **Verify:** Run it twice, byte-identical output; `--check` exits 0; a scratchpad copy of
  the JSON with one name duplicated makes it exit 2 naming the name; `python
  scripts/code-verify.py --check core/Api/API/CommandSafetyTable.cpp
  scripts/generate-command-safety.py`; `python scripts/syntax-check.py
  core/Api/API/CommandSafetyTable.cpp`.
- **Executor:** yes, donor named above.
- **Deps:** T1, T2
- [ ] done

### T4 — The drift gate in the sanitizer and CI

- **Files:** `scripts/sanitize-commit.py`, `.github/workflows/ci.yml`
- **Does:** `run_python_step("Regenerating the command safety table", ...)` right after the
  command-strings step (so `.code-report` still postdates every generator). CI: `python3
  scripts/generate-command-safety.py --check` on the line after
  `generate-command-strings.py --check` in the "Verify generated API surfaces" step.
- **Verify:** `python scripts/sanitize-commit.py` runs through and leaves the table
  unchanged; `python -m pytest scripts/tests/ -q` (the CI-shape fixtures) passes.
- **Deps:** T3
- [ ] done

### T5 — The ctest unit `tst_command_safety_table`

- **Files:** `app/tests/tst_command_safety_table.cpp` (new), `app/tests/CMakeLists.txt`
- **Does:** Donor `app/tests/tst_command_capability_registry.cpp` and its registration
  (lines 1882 to 1891): compiles `CommandSafety.cpp`, `CommandSafetyTable.cpp` and
  `CommandCapabilities.cpp` directly, `LIBS Qt6::Core SerialStudio::Core`,
  `SS_API_SCHEMA_PATH` defined. Cases, each failure naming the offending names: (1)
  `entries()` is strictly ascending by name (the `lower_bound` precondition); (2) table names
  minus `assistantTools()` equal the schema names, both directions; (3) no name over 64
  characters; (4) for every schema name `!isListedTool` iff the tier is Blocked or the
  capabilities carry `ControlScriptOnly`, and the omitted set is exactly the nine names
  (`licensing.activate`, `activateOffline`, `deactivate`, `enableTrial`, `setLicense`,
  `validate`, `system.exec`, `system.kill`, `system.runningProcesses`); (5) `hintsFor` per
  tier, device-gated and unknown; (6) twelve fixed `humanizeToolName` cases drawn from the
  schema, among them `io.getStatus` (`IO / Get Status`), `io.opcua.trustServer`
  (`IO / OPC UA / Trust Server`), `io.iec104.getPoints` (`IO / IEC 104 / Get Points`),
  `io.eip.addTag` (`IO / EtherNet/IP / Add Tag`), `csvExport.setInterval`
  (`CSV Export / Set Interval`), `ui.window.setLayoutPattern`
  (`UI / Window / Set Layout Pattern`).
- **Verify:** `python scripts/syntax-check.py app/tests/tst_command_safety_table.cpp`,
  `python scripts/code-verify.py --check` on both files; after the maintainer builds,
  `ctest --test-dir <build> -R "tst_command_safety_table|tst_assistant_batch_tiers"`
  green (the second proves the JSON edit of T1 kept the assistant's tier test).
- **Executor:** yes, donor named above.
- **Deps:** T3
- [ ] done

### T6 — `MCP::Tool` fields and the supported revisions

- **Files:** `core/Api/API/MCPProtocol.h`
- **Does:** `Tool` gains `QString title` and `struct ToolAnnotations { bool read_only; bool
  destructive; bool open_world; }`, loses `category` and `tags`; `toJson()` emits exactly
  `name`, `title`, `description`, `inputSchema` and `annotations` (`readOnlyHint`,
  `destructiveHint`, `openWorldHint`). `kProtocolVersion` becomes
  `kSupportedProtocolVersions` (`2024-11-05`, `2025-03-26`, `2025-06-18`),
  `kLatestProtocolVersion` (`2025-06-18`) and `[[nodiscard]] bool
  isSupportedProtocolVersion(QStringView)`. The only user of the old constant is
  `MCPHandler.cpp:242`, rewritten in T7.
- **Verify:** `python scripts/code-verify.py --check core/Api/API/MCPProtocol.h`; the
  compile proof is T7's syntax check (this header has no TU of its own until the handler
  stops calling the tag functions).
- **Deps:** none
- [ ] done

### T7 — `MCPHandler`: hints on, tags off, revision negotiated

- **Files:** `core/Api/API/MCPHandler.h`, `core/Api/API/MCPHandler.cpp`
- **Does:** `generateToolsFromRegistry()` skips a name `CommandSafety::isListedTool` rejects,
  sets `title = humanizeToolName(name)` and `annotations` from `hintsFor(name)` with
  `open_world = false`. The five `tag*` functions (`tagIoSubmodule`,
  `tagToolFromCommandName`, `tagToolModule`, `tagToolCommand` and the dispatcher) and
  their declarations go. `handleInitialize` reads `params.protocolVersion`, echoes it when
  `isSupportedProtocolVersion`, otherwise answers `kLatestProtocolVersion`; nothing else in
  the handshake moves. Binding invariants named at edit time: `handleToolsCall` and the
  `RemoteDispatchScope` block are untouched, so the gate stays the only authorizer; no new
  signal, thread or lock (the table is immutable); the TU shrinks and every function stays
  under the 100-line cap.
- **Verify:** `python scripts/syntax-check.py core/Api/API/MCPHandler.cpp`, `python
  scripts/code-verify.py --check core/Api/API/MCPHandler.h core/Api/API/MCPHandler.cpp`,
  `python scripts/layer-verify.py`. After the maintainer rebuilds and relaunches with the
  API server on: the scratchpad probe (`mcp_probe.py`) reports 443 tools, every one with a
  title and annotations, and `pytest tests/integration/test_mcp.py -v` (the existing cases)
  stays green.
- **Deps:** T2, T6
- [ ] done

### T8 — Integration tests for the wire (AC1, AC3, AC4, revision echo)

- **Files:** `tests/integration/test_mcp.py`
- **Does:** A module-level loader of `app/rcc/ai/command_safety.json` (path via
  `Path(__file__).parents[2]`, donor `tests/unit/test_updates_manifest.py`).
  `MCPClient.initialize` gains a `protocol_version` parameter defaulting to today's
  `2024-11-05`. New classes: `TestToolAnnotations` (every tool has a non-empty `title`
  different from `name`; `readOnlyHint` is a bool and true iff the name is in `safe`;
  `destructiveHint` true iff in `alwaysConfirm`, `deviceGated` or `blocked`;
  `openWorldHint` false); `TestOmittedTools` (none of the nine omitted names is listed,
  `licensing.getStatus` is, and `tools/call` on `system.runningProcesses` returns the
  gate's existing refusal as `isError`; **never call a licensing mutation**, the gate lets
  them through); `TestToolSchemaKeys` (every tool's key set is within `{name, title,
  description, inputSchema, annotations}`); three `TestMCPLifecycle` cases (`2025-06-18`
  echoed, `2024-11-05` echoed, `1999-01-01` answered with `2025-06-18`).
- **Verify:** `pytest tests/integration/test_mcp.py -v` against the rebuilt app; `python
  scripts/code-verify.py --check tests/integration/test_mcp.py`; black.
- **Executor:** yes, donors are the existing classes in the same file.
- **Deps:** T7 (needs the rebuilt app)
- [ ] done

### T9 — The bridge's failure line and its unit test (AC7)

- **Files:** `examples/MCP Client/claude_desktop_bridge.py`, `tests/unit/test_mcp_bridge.py`
  (new)
- **Does:** Bridge: the give-up branch's *last* stderr line becomes the fix, worded as the
  manual words it ("Enable the API server: Preferences, API & Plugins, Enable API Server");
  the port honors a `SERIAL_STUDIO_API_PORT` environment override (default 7777) so the
  test can aim at a closed port while the app is up; the module docstring notes that the
  plugin repository ships this same file and the two are kept byte-identical. Retry loop,
  delays and the ASCII-safe stream reconfiguration stay. Test: bind-and-release a port to
  get a closed one, spawn `sys.executable -I <bridge>` with the override, `communicate`
  with a 15 s timeout, assert exit code 1, wall time under ten seconds, and that the last
  non-empty stderr line names "Enable API Server". No new pytest marker (the ini registers
  them strictly); runnable with or without the app.
- **Verify:** `pytest tests/unit/test_mcp_bridge.py -v` with the app up and with it down;
  black on both files; `python -I "examples/MCP Client/claude_desktop_bridge.py"` with
  nothing listening exits 1 in about eight seconds.
- **Executor:** yes, donor `tests/unit/test_updates_manifest.py` for shape.
- **Deps:** none
- [ ] done

### T10 — The manual and the examples README (AC9)

- **Files:** `doc/help/API-Reference.md`, `examples/MCP Client/README.md`
- **Does:** Manual, MCP section: "Getting Started" leads with the directory plugin (add
  "Serial Studio" from the Claude directory in Claude Code or Cowork, with the app running
  and the API server enabled), keeps the manual bridge configuration as the fallback, adds
  one paragraph on the hint contract (read-only and destructive hints follow the assistant's
  tiers; the licensing mutations and the process launcher are not listed), refreshes "300+"
  to "more than 440" and names the three protocol revisions. Examples README: "Claude
  Desktop Integration" opens with the plugin route for Claude Code and Cowork and keeps the
  Claude Desktop steps; the tools section shows a tool object with `title` and
  `annotations`; "Specification Compliance" lists the three revisions. Written under the
  `ss-docs` skill's voice rules.
- **Verify:** `python scripts/documentation-verify.py` on both files, zero new findings;
  `python scripts/term-verify.py`; a read-back that every claim matches T7's wire.
- **Deps:** T7 (for the facts; may be drafted earlier)
- [ ] done

### T11 — AI-facing architecture docs

- **Files:** `doc/claude/architecture/ai.md`, `CLAUDE.md`
- **Does:** `ai.md` "The Tier Model": the table of record now also feeds the MCP tool hints
  through `scripts/generate-command-safety.py` into `core/Api/API/CommandSafetyTable.cpp`
  (drift-gated), documents `assistantTools`, and names the hint mapping in one line.
  `CLAUDE.md` Subsystem Contracts, AI assistant row: add that the table is generated into
  `CommandSafetyTable.cpp`, never hand-edited, enforced by `ctest:tst_command_safety_table`.
- **Verify:** `python scripts/claim-verify.py --no-report doc/claude/architecture/ai.md
  CLAUDE.md` resolves every path, symbol and enforcer (so T3 and T5 must exist);
  `python scripts/documentation-verify.py` on `ai.md`.
- **Deps:** T3, T5
- [ ] done

### T12 — The tests catalog and the ledger row

- **Files:** `tests/README.md`, `doc/claude/common-mistakes.md`
- **Does:** `tests/README.md`: a "Unit" row in the category table for `tests/unit/` (pure
  Python, no app, run with `pytest tests/unit/ -v` since the ini's `testpaths` does not
  include it), a file row for `test_mcp_bridge.py`, and the `test_mcp.py` integration row
  extended with the annotation, omission, key-set and revision cases. `common-mistakes.md`
  row 168 closes with `Codified: ctest:tst_command_safety_table`.
- **Verify:** `python scripts/ledger-verify.py --check` (one row moved from debt to
  codified; re-seed only downward); `python scripts/claim-verify.py --no-report
  tests/README.md doc/claude/common-mistakes.md`; `python scripts/documentation-verify.py
  tests/README.md`.
- **Deps:** T5, T8, T9
- [ ] done

### T13 — Plugin: the launcher spike (scratchpad only, records a decision)

- **Files:** scratchpad `plugin-probe/serial-studio/.claude-plugin/plugin.json` and
  `.mcp.json` (nothing in either repository)
- **Does:** Declares `userConfig.python` (`type: string`, `default: python3`) and sets the
  server `command` to `${user_config.python}`. With the app up, runs
  `claude --plugin-dir <dir> -p "call io.getStatus"` and reads `claude mcp list` (or the
  bridge's startup line on stderr) to see whether the reference was substituted. Also
  records whether `python3` resolves on this Windows machine. Outcome written here as one
  line: **(c) ship `${user_config.python}`** or **(a) ship `python3` plus the README note**.
- **Verify:** The outcome line below this task, with the command that showed it.
- **Deps:** none
- [ ] done — outcome: _pending_

### T14 — Plugin repository: manifest and server

- **Files (new repository `Serial-Studio/claude-plugin`, checked out beside this one at a
  path the maintainer names):** `.claude-plugin/plugin.json`, `.mcp.json`, `bridge.py`
- **Does:** `plugin.json`: `name: serial-studio`, `displayName: Serial Studio`, `version:
  1.0.0`, `description`, `author` (name and `alex@serial-studio.com`), `license:
  GPL-3.0-or-later`, `homepage: https://serial-studio.com`, `repository`, `keywords`, and
  `userConfig` only if T13 chose (c). `.mcp.json`: one server `serial-studio`, `command`
  per T13, `args: ["${CLAUDE_PLUGIN_ROOT}/bridge.py"]`. `bridge.py`: byte-identical copy of
  T9's examples bridge. Text files only, each under 256 KiB, no `bin/`, no shell wrapper.
- **Verify:** `claude plugin validate .` passes; `cmp bridge.py "<main repo>/examples/MCP
  Client/claude_desktop_bridge.py"` identical.
- **Deps:** T9, T13
- [ ] done

### T15 — Plugin repository: README, license and icon

- **Files:** `README.md`, `LICENSE`, `.claude-plugin/icon.png`
- **Does:** README (well over 40 words): what the plugin does, requirements (Serial Studio
  running, API server enabled in Preferences, Python 3 on PATH), install from the directory
  and `claude --plugin-dir` for development, a usage walk-through, the omitted commands and
  the consent prompts, a **Privacy Policy** section stating that nothing leaves the machine
  except to the local Serial Studio instance on port 7777 and that the plugin collects
  nothing and phones home nowhere, the Windows launcher note from T13, support links.
  `LICENSE`: the GPL-3.0-or-later text. Icon: a square 1024 px PNG rendered from the app
  icon SVG under `app/deploy/linux/` in the main repository; the maintainer confirms the
  asset before T18 because the portal locks it on first save.
- **Verify:** `claude plugin validate --profile directory .` reports no refusal and no
  warning; a read-back of the README against spec R12.
- **Executor:** yes for README and LICENSE against the spec's R7, R10 and R12 text.
- **Deps:** T14
- [ ] done

### T16 — Plugin repository: the skill

- **Files:** `skills/serial-studio/SKILL.md`
- **Does:** Frontmatter `name` and a `description` that says when to load (telemetry,
  serial or network devices, dashboards, anything naming Serial Studio). Body: the
  prerequisite and the exact failure message to expect when it is unmet; the read-first
  workflow (`io.getStatus`, then the `serialstudio://frame/current` resource or the
  dashboard reads, then project edits); the consent prompts the user will see (device
  control, script installs, interface driving) and the rule that a refusal is explained,
  never retried; the large-reply advice (use the tool list, not `api.getCommands`; page
  Historian reads); that the licensing and process-launcher commands do not appear in the list, and why.
  listed. Under 300 lines, no claim the manual does not make.
- **Verify:** `claude plugin validate --profile directory .` still clean; with the app up,
  `claude --plugin-dir . -p "Is Serial Studio connected to a device?"` answers from
  `io.getStatus` without calling a mutator (read the transcript).
- **Deps:** T14
- [ ] done

### T17 — Plugin: validation and the Windows end-to-end run (AC5 local half, AC6 Windows)

- **Files:** none (observations recorded below)
- **Does:** `claude plugin validate --profile directory .` zero findings; `claude
  --plugin-dir . -p "call io.getStatus and report the result"` returns the live status
  JSON with no config file edited (the 2026-11-02 run is the baseline: six turns, live
  data, no denials). The two command lines and their results are recorded here.
- **Verify:** The recorded outcome line.
- **Deps:** T15, T16
- [ ] done — outcome: _pending_

### T18 — Maintainer: publish, submit, observe (AC5 portal half, AC6 macOS/Linux, AC8)

- **Files:** `doc/claude/specs/0112-claude-directory-listing/portal-validation.md` (new,
  the recorded portal result)
- **Does:** Creates the public repository `Serial-Studio/claude-plugin`, pushes T14 to T16;
  runs the T17 command on macOS and Linux; submits at `claude.ai/directory/manage` under
  the permanent name `serial-studio` with the icon, the README's privacy section as the
  privacy URL and the manual's MCP section as the docs URL; records the portal's Validate
  output (zero Blocking findings) in the file above; once live, installs from the directory
  in Claude Code and reaches the T17 state.
- **Verify:** The three AC lines ticked in `spec.md` with the date.
- **Deps:** T17
- [ ] done

## Definition of Done

The whole-feature gate, checked once every task is complete.

- [ ] Every acceptance criterion in `spec.md` is met and checked off there (AC1, AC3, AC4
  by T8; AC2 by T5; AC5 by T15 and T18; AC6 by T17 and T18; AC7 by T9; AC8 by T18; AC9 by
  T10).
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff (T2, T3, T5, T6, T7); findings addressed or noted.
- [ ] Hotpath untouched by construction (no file under the pipeline, no cached flag, no
  queued hop); `--benchmark-hotpath` not required, stated as such in the handoff.
- [ ] `python scripts/syntax-check.py` ran on every edited C++ file; any SKIP is named in the
  handoff as unverified.
- [ ] Maintainer-run checks identified: a rebuild, `ctest -R
  "tst_command_safety_table|tst_assistant_batch_tiers"`, `pytest tests/integration/test_mcp.py`
  with the app up, `pytest tests/unit/test_mcp_bridge.py`.
- [ ] `python scripts/sanitize-commit.py` run before each commit; the generated table shows
  no drift; `layer-verify.py` and the singleton census unchanged.
- [ ] Diff is *what was asked, and only that*: no API server auth or consent file touched,
  `baseline-manifest.json` (spec 0036) and `LorenzAttractor.ssproj` left as the
  maintainer's work.
- [ ] The licensing-reach question from `plan.md` Risks is still open and still the
  maintainer's; this feature did not change it.
- [ ] `spec.md` status set to `done`.
