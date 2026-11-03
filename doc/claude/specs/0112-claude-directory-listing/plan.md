---
spec: 0112-claude-directory-listing
phase: plan
status: approved        # draft -> approved (gate before /ss-tasks)
updated: 2026-11-03
---

# Plan 0112 — Claude directory listing: annotated MCP tools and a plugin bundle

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

The assistant's tier table `app/rcc/ai/command_safety.json` stays the one hand-edited home.
A new generator, `scripts/generate-command-safety.py`, projects it into a checked-in constexpr
table in the API library (`core/Api/API/CommandSafety.cpp`, the shape of
`CommandCapabilities.cpp`), with a `--check` drift gate in `sanitize-commit.py` and the CI
generator step; `AI::CommandRegistry` keeps its runtime parse of the same JSON, so the two
consumers cannot diverge without the gate failing. `API::MCPHandler` reads the table to attach
a humanized `title` and the `readOnlyHint` / `destructiveHint` annotations to every tool, skips
the blocked tier and the control-script-only class, drops the consumer-less `category`/`tags`
keys, and negotiates the protocol revision among the three it can honor so `title` and
`annotations` are served under a revision that defines them. The 61 untiered commands get tiers
in the JSON, a new ctest unit ties the table to the API schema snapshot both ways, and the
pytest MCP file asserts the wire. The plugin lives in a new public repository
`Serial-Studio/claude-plugin` (plugin at the root) with the manifest, a copy of the stdio
bridge, one skill, the README with its privacy section, a license and the icon; the examples
bridge keeps its bounded-exit failure message and gains a unit test for it.

## Affected subsystems & files

Main repository:

Amended during `/ss-tasks` (2026-11-03), three findings from the ground truth: the API-layer
ctests compile their TUs directly rather than linking the library, so the listing policy
lives in a hand-written `CommandSafety.cpp` beside a generated data TU instead of as
`MCPHandler` statics; the JSON loader validates size limits only, so `assistantTools` needs
no schema; `tests/unit/` has no catalog row and no CI caller today, so the bridge test gets
a category row and an environment override to aim at a closed port.

| File | Change |
|------|--------|
| `app/rcc/ai/command_safety.json` | Add the 61 untiered commands (table below); add `assistantTools`, the allow-list of assistant-only names (`fs.*`) that are tiered but are not API commands; extend `_comment` to say the file also feeds the MCP annotations. |
| `scripts/generate-command-safety.py` | New generator: reads the JSON, writes `core/Api/API/CommandSafetyTable.cpp` as a sorted constexpr array of `{name, tier, deviceGated}` plus the `assistantTools` array and the two accessors; `--check` reports drift and exits non-zero; refuses a duplicated, non-ASCII or over-64-character name. Mirrors `generate-command-strings.py`. |
| `core/Api/API/CommandSafety.h` | Hand-written: `enum class Tier : quint8 { Safe, Confirm, AlwaysConfirm, Blocked }`, `struct Entry`, `struct ToolHints`, `[[nodiscard]] std::optional<Tier> tierOf(QStringView)`, `[[nodiscard]] bool isDeviceGated(QStringView)`, `[[nodiscard]] ToolHints hintsFor(QStringView)`, `[[nodiscard]] bool isListedTool(QStringView)`, `[[nodiscard]] QString humanizeToolName(QStringView)`, `[[nodiscard]] std::span<const Entry> entries()`, `[[nodiscard]] std::span<const QLatin1StringView> assistantTools()`. |
| `core/Api/API/CommandSafety.cpp` | Hand-written: the lookups and the listing policy over `entries()` (`tierOf`, `isDeviceGated`, `hintsFor`, `isListedTool`, `humanizeToolName`). |
| `core/Api/API/CommandSafetyTable.cpp` | Generated: `kEntries`, `kAssistantTools`, `entries()`, `assistantTools()`. Never hand-edited (banner says so). |
| `core/Api/CMakeLists.txt` | Add the three files beside `CommandCapabilities.*`. |
| `core/Api/API/MCPProtocol.h` | `Tool` gains `title` and `annotations` (`readOnlyHint`, `destructiveHint`, `openWorldHint`), loses `category`/`tags`; `kProtocolVersion` becomes `kSupportedProtocolVersions` (`2024-11-05`, `2025-03-26`, `2025-06-18`) plus `kLatestProtocolVersion` and `isSupportedProtocolVersion`. |
| `core/Api/API/MCPHandler.h` / `.cpp` | `generateToolsFromRegistry()` skips names `CommandSafety::isListedTool` rejects and attaches `humanizeToolName` and `hintsFor`; `handleInitialize` echoes the client's revision when supported, else the latest; the five tagging functions go. |
| `scripts/sanitize-commit.py` | Run the generator before `.code-report`, next to `generate-command-strings.py`. |
| `.github/workflows/ci.yml` | Add `python3 scripts/generate-command-safety.py --check` to the generator-drift step (the block at lines 2238-2241). |
| `doc/claude/scripts.md` | A row for the generator and its place in the sanitize pipeline sentence. |
| `app/tests/tst_command_safety_table.cpp` | New ctest unit, the `tst_command_capability_registry` mechanism (the three TUs compiled directly): every `api-schema.json` name has exactly one tier; every table name is a schema name or an `assistantTools` name; no name exceeds 64 chars; the blocked tier and `ControlScriptOnly` names are exactly the set `isListedTool` rejects; `hintsFor` per tier; twelve `humanizeToolName` cases. |
| `app/tests/CMakeLists.txt` | Register the unit with `SS_API_SCHEMA_PATH`, as the capability test is registered (lines 1882-1890). |
| `tests/integration/test_mcp.py` | New tests: annotations and titles against the JSON (AC1), omitted names absent from `tools/list` while a control-script-only name still answers with the gate's refusal (AC3; a licensing mutation is never called), tool key set within the MCP schema (AC4), negotiated revision equals the requested one when supported. |
| `tests/unit/test_mcp_bridge.py` | New: spawns `examples/MCP Client/claude_desktop_bridge.py` against a closed port through the override, asserts exit 1 within ten seconds and the fix named on the last stderr line (AC7). |
| `examples/MCP Client/claude_desktop_bridge.py` | Keep the retry loop; make the final stderr line the one that names the Preferences fix; a `SERIAL_STUDIO_API_PORT` environment override of the port; header note that the plugin repository ships the same file. |
| `examples/MCP Client/README.md` | "Claude Desktop Integration" section leads with the directory plugin; manual config stays as the fallback. |
| `doc/help/API-Reference.md` | MCP section: the directory plugin as the first install path, the annotation contract in one paragraph, the tool count claim refreshed. |
| `tests/README.md` | A "Unit" category row for `tests/unit/` (none today); integration table row for the new `test_mcp.py` cases; file row for the bridge test. |
| `doc/claude/architecture/ai.md` | "The Tier Model": the table now also feeds MCP annotations through the generated `API::CommandSafety`; document `assistantTools`. |
| `doc/claude/common-mistakes.md` | Row 168 (an assistant command missing from the tier table) closes with `Codified: ctest:tst_command_safety_table`. |
| `CLAUDE.md` | Subsystem Contracts, AI assistant row: the tier table is generated into `core/Api/API/CommandSafety.cpp`, never hand-edited. |

Plugin repository (`Serial-Studio/claude-plugin`, plugin at the root):

| File | Role |
|------|------|
| `.claude-plugin/plugin.json` | `name: serial-studio`, `displayName`, `version`, `description`, `author`, `license`, `homepage`, `repository`; `userConfig.python` only if the spike in Tradeoffs passes. |
| `.claude-plugin/icon.png` | Square PNG, 512 to 2048 px; locked on first portal save. |
| `.mcp.json` | `{"mcpServers": {"serial-studio": {"command": "python3", "args": ["${CLAUDE_PLUGIN_ROOT}/bridge.py"]}}}` (or the spike's `${user_config.python}`). |
| `bridge.py` | Copy of the examples bridge. |
| `skills/serial-studio/SKILL.md` | When to load, the prerequisite, the read-first workflow, the consent prompts, the large-reply advice. |
| `README.md` | What it does, how to use it, data handling, a "Privacy Policy" section, the Windows launcher note. |
| `LICENSE` | GPL-3.0-or-later, matching the bridge's GPL side. |

## Architecture & data flow

- **Source of truth → table.** `command_safety.json` (five tier arrays plus the new
  `assistantTools` array) is read by the generator, which emits one sorted array
  `kEntries[]` of `{QLatin1StringView name; Tier tier; bool deviceGated}` plus the
  `assistantTools` names as a second small array (so the ctest unit can exempt them). Lookup is
  a `std::lower_bound` on the sorted names, like `CommandCapabilities::of`. No allocation, no
  Qt resource, no runtime file: the API library and every ctest unit that links it get the
  table by linking.
- **Table → wire.** `MCPHandler::generateToolsFromRegistry()` walks
  `API::CommandRegistry::commands()` as today. For each name it asks
  `CommandSafety::isListedTool(name)` (false for `Tier::Blocked` and for any name whose
  `CommandCapabilities::of` carries `ControlScriptOnly`), then fills
  `Tool::title = humanizeToolName(name)` and `Tool::annotations` from `hintsFor(name)`:
  `Safe` → `readOnlyHint true, destructiveHint false`; `Confirm` → `false, false`;
  `AlwaysConfirm`, `Blocked`, or `isDeviceGated` → `false, true`; a name the table does not
  know → `false, false` (the assistant's Confirm fallthrough), which the ctest unit makes
  unreachable in a shipped tree. `openWorldHint` is `false` on every tool (the server talks
  to one local application). `Tool::toJson()` emits `title` and `annotations` and nothing
  else beyond `name`, `description`, `inputSchema`. The policy functions live in the
  hand-written `CommandSafety.cpp`, not in the handler, so the ctest unit compiles them
  without the API library.
- **Title.** `humanizeToolName` splits on `.`, maps a whole lowercase segment through the
  acronym table first (`api`, `ble`, `canbus`, `csv`, `eip` → `EtherNet/IP`, `hid`,
  `iec104` → `IEC 104`, `io`, `mdf4`, `mqtt`, `opcua` → `OPC UA`, `s7`, `uart`, `ui`,
  `usb`), otherwise splits the segment on camelCase boundaries, upper-cases the acronym
  pieces (`Ansi`, `Csv`, `Db`, `Dtr`, `Fd`, `Fps`, `Http`, `Id`, `Iso`, `Json`, `Tcp`,
  `Tls`, `Udp`, `Url`, `Uuid`, `Vt`), capitalizes the rest, and joins segments with " / "
  (`project.dataset.setTransformCode` → `Project / Dataset / Set Transform Code`). The
  piece list comes from a survey of the 452 live names. Pure function of the name, covered
  by the ctest unit on a dozen fixed names.
- **Revision negotiation.** `handleInitialize` reads `params.protocolVersion`; if it is one
  of the three supported strings it is echoed, otherwise `2025-06-18` is returned. Nothing
  else about the handshake changes; batch handling stays (harmless under every revision).
- **Who consumes what.** `AI::CommandRegistry` (core/Ui) keeps parsing the JSON at runtime for
  the in-app assistant; the generator's `--check` keeps the C++ table byte-identical to what
  the JSON implies, so the two consumers of the one source agree at every commit. Folding the
  assistant onto the table is a follow-up outside this lane (Tradeoffs).
- **Threads.** The annotation lookup runs wherever `ClientReception` dispatches
  `processMessage` today; it is a read of an immutable table and adds no affinity, no signal,
  no lock.
- **Plugin runtime.** Claude Code or Cowork starts `python3 bridge.py` from the plugin folder;
  the bridge dials `127.0.0.1:7777` up to five times two seconds apart, then exits 1 with the
  fix on its last stderr line. Nothing in the main repository knows the plugin exists; the
  plugin knows the main repository only through the port.

## Hotpath & threading impact

- **Touches the hotpath?** No. Nothing under `FrameReader`, `CircularBuffer`, `FrameBuilder`,
  `Dashboard` draw or the span lane changes; `tools/list` and `tools/call` are command-rate.
- **New cross-thread signal/slot?** No. The generated table is immutable and read in place.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged; no frame or block is touched.

## Data model & persistence

- No project-file, session-DB or `Frame.h` change.
- `command_safety.json` gains one key, `assistantTools` (string array). Confirmed during
  `/ss-tasks`: `AI::CommandRegistry::load` passes `Misc::JsonValidator::parseAndValidate` a
  `Limits` (size, depth, array length) and no schema, then reads five named arrays, so the
  new key needs no declaration anywhere and the assistant ignores it.
- `api-schema.json` is unchanged; it is the enumeration the ctest unit checks the table against.

## API / SDK surface

- No new command. The MCP tool object gains `title` and `annotations`, loses `category` and
  `tags` (no consumer found in code, tests, help or examples). The handshake's
  `protocolVersion` becomes negotiated. The JSON-RPC envelope, gRPC and the SDKs are untouched.
- Follow-up candidate, not in this lane: `--dump-api-schema` could carry the tier so the
  generated SDK docs and the API Reference tables show it.

## QML / UI

None.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How the API layer reaches the tier table | (a) generated constexpr table from the JSON; (b) a classifier callable bound through `API::handlerContext()` by the root, implemented in core/Ui; (c) move the JSON loader down into core/Api and have `AI::CommandRegistry` delegate; (d) a tier field on every `CommandDefinition` at its registration site, JSON deleted | **(a).** It is this repo's established shape (`CommandCapabilities.cpp`, the six property-registry artifacts, `CommandStrings.cpp`): no runtime resource in a library, trivially linked by ctest, a drift gate instead of a binding that a headless root could leave null. (b) lists hint-less tools whenever a root forgets the bind, the failure class the composition-root rules exist for. (c) drags a Qt resource and the validator dependency into the API library. (d) is 452 registration edits for a table the docs call the table of record. |
| Tool title | (a) humanized from the dotted name; (b) first clause of the description; (c) hand-written per command | **(a).** Deterministic, no second table, distinct from the name, and the acronym map is the only maintenance. (b) yields sentences, not titles. (c) is 452 strings to drift. |
| `category` / `tags` | (a) drop; (b) keep under `_meta` with a `serial-studio/` prefix | **(a).** No consumer anywhere; the tags were keyword heuristics ("contains start"). Dropping also removes the five tagging functions. |
| Protocol revision | (a) negotiate among the three the server can honor; (b) keep the constant `2024-11-05`; (c) advertise only `2025-06-18` | **(a).** `title` on a tool is defined from `2025-06-18` and annotations from `2025-03-26`; a client is entitled to ignore fields its negotiated revision lacks. (b) serves fields under a revision that does not define them. (c) drops a `2024-11-05` client that cannot step up. |
| An omitted name on `tools/call` | (a) the existing remote gate decides; (b) the MCP layer refuses every omitted name | **(a).** Listing is discovery; authorization has ONE owner (spec 0098). (b) would change what a licensing mutation does over MCP, a security-sensitive behavioral change this spec does not make. See Risks for the open licensing question. |
| Bridge launcher name | (a) `python3` plus a Windows note; (b) `python` plus a macOS note; (c) `${user_config.python}` as the `command` with default `python3` | **Spike (c) first, fall back to (a).** The docs confirm `${user_config.*}` substitution in MCP server config but do not say `command` is included, and there is no per-platform field. Task 1 of the plugin work runs `claude --plugin-dir` with the reference in `command`; if it substitutes, Windows users set `python` once and Cowork takes the default; if not, (a) ships and the README carries the note. |
| Two copies of the bridge | (a) plugin repository canonical, examples copy kept with a header note; (b) delete the examples copy and point the manual path at the plugin repository | **(a).** The manual fallback (spec R11) keeps working for every existing config that points into a checkout; the drift surface is 200 stable lines. |
| Assistant-only names in the table | (a) an `assistantTools` allow-list key in the JSON; (b) hardcode the `fs.*` tiers in core/Ui; (c) a second JSON for assistant tools | **(a).** One file, one new key, and the ctest unit can read the exemption; (b) splits the table of record; (c) is the second list the spec forbids. |
| Unknown name default | (a) `readOnlyHint false, destructiveHint false`; (b) `destructiveHint true` | **(a).** Mirrors the assistant's Confirm fallthrough and the gate makes the case unreachable; (b) would prompt on every call for a command the table merely forgot. |
| Assistant switches to the generated table | (a) now; (b) later | **(b).** Keeps core/Ui and `tst_assistant_batch_tiers` out of this lane; the drift gate already guarantees agreement. Named here so review does not relitigate it. |

## Risks & mitigations

- **A mis-tiered command among the 61.** A mutation tagged `safe` auto-runs in both the
  assistant and Claude Code. Mitigation: the proposed assignment below is reviewed by the
  maintainer before it lands; AC1 checks the wire against the JSON; the existing
  `tst_assistant_batch_tiers` exercises the assistant side.
- **Licensing mutations are remotely callable today.** `licensing.activate`, `deactivate`,
  `setLicense`, `enableTrial`, `activateOffline` and `validate` are `kPlain` for the remote
  gate, so a remote API, MCP or gRPC client can call them now. This plan only stops listing
  them (spec R5); whether they should carry a capability the gate refuses is a separate,
  decision-visible change on a security-sensitive surface for the maintainer to call.
- **Generated-file drift.** Someone edits the JSON and forgets the generator. Mitigation:
  `sanitize-commit.py` regenerates; CI's generator step runs `--check`; the banner on the
  generated file says never to hand-edit.
- **Schema validation of the JSON.** If the assistant's loader validates against a schema,
  the new `assistantTools` key must be declared there or the assistant falls back to
  Confirm for every command (its documented failure mode). Mitigation: task-phase grep and
  the existing assistant tier test.
- **A client that ignores annotations under an old revision.** Mitigation: revision
  negotiation; the pytest asserts the echo.
- **Windows launcher.** Mitigation: the spike plus the README note; AC6 runs on all three OSes.
- **Tool-list size.** 452 tools are 195 KB today; titles and annotations add roughly 25 KB.
  Claude Code already loads the list through its deferred-tool path (32.7k cache-creation
  tokens measured). Not changed by this plan; the skill steers Claude to `tools/list` over
  `api.getCommands`.
- **Silent-breakage classes from `common-mistakes.md`.** Row 168 (a command missing from the
  tier table) is the one this change is exposed to and the one it closes. None of the hotpath,
  cached-flag, timestamp or queued-hop classes apply.

### Proposed tiers for the 61 untiered commands (maintainer reviews before it lands)

| Tier | Commands |
|------|----------|
| `safe` | `alarms.state`, `alarms.getProjectSounds`, `influx.getStatus`, `io.eip.getConfig`, `io.eip.getStatus`, `io.iec104.getConfig`, `io.iec104.getPoints`, `io.iec104.getStatus`, `io.opcua.getCertificate`, `io.opcua.listTrusted`, `io.s7.getConfig`, `io.s7.getStatus`, `mirror.getInfo`, `mirror.getStructure`, `project.dataset.getFFTMarkers`, `sessions.getRegression`, `sessions.getVerification`, `stream.getInfo`, `stream.getSources` |
| `confirm` | `alarms.acknowledge`, `alarms.reset`, `alarms.silence`, `alarms.setMuted`, `alarms.test`, `alarms.setProjectSounds`, `console.setCollapseDuplicates`, `console.setSearchCaseSensitive`, `csvExport.setInterval`, `influx.setEnabled`, `io.opcua.exportCertificate`, `project.dataset.setFFTMarkers`, `project.redo`, `project.undo`, `ui.window.setLayoutPattern` |
| `alwaysConfirm` | `alarms.clear` (the clear family), `influx.setConfig` (credentials, like the MQTT configs), `io.eip.generateProject`, `io.iec104.generateProject`, `io.s7.generateProject` (open-replace family: they rebuild the project), `sessions.regress`, `sessions.verify` (both carry `LoadsProject`) |
| `deviceGated` | `io.canbus.setDataBitrate`, `io.canbus.setListenOnly`, `io.canbus.setLoopback`, `io.eip.addTag`, `io.eip.clearTags`, `io.eip.removeTag`, `io.eip.setProperty`, `io.iec104.clearPoints`, `io.iec104.setProperty`, `io.opcua.regenerateCertificate`, `io.opcua.revokeTrust`, `io.opcua.setIdentityType`, `io.opcua.setSecurityMode`, `io.opcua.setSecurityPolicy`, `io.opcua.setUserCertificate`, `io.opcua.trustServer`, `io.s7.addVariable`, `io.s7.clearVariables`, `io.s7.removeVariable`, `io.s7.setProperty` |

Rule applied: reads are `safe`; reversible app or project settings are `confirm`; the
delete/clear/open-replace/credential families are `alwaysConfirm`; every driver setting is
`deviceGated`, matching the tier definition in `ai.md`. `system.runningProcesses` keeps `safe`
in the table but is omitted from the tool list by the `ControlScriptOnly` rule.

## Test & verification plan

- **Unit (you can run):**
  - `pytest tests/unit/test_mcp_bridge.py` (AC7): bridge exit code and last stderr line with
    nothing listening on 7777.
  - `python scripts/generate-command-safety.py --check` (drift gate) after every JSON edit.
  - `claude plugin validate --profile directory ./serial-studio` in the plugin repository
    (AC5, local half).
- **C++ unit (maintainer builds, you run `ctest`):**
  - `tst_command_safety_table` (AC2): schema ⇔ table coverage both ways, the 64-char cap, the
    omitted set equals blocked ∪ `ControlScriptOnly`, and twelve fixed `humanizeToolName`
    cases.
  - `tst_assistant_batch_tiers` still passes (the JSON gained entries and a key).
- **Integration (needs the app up with the API server enabled):**
  - `pytest tests/integration/test_mcp.py` (AC1, AC3, AC4, revision echo): `tools/list`
    every tool has a non-empty `title`, a boolean `readOnlyHint`, `readOnlyHint` iff the
    name is in `safe`, `destructiveHint` iff in `alwaysConfirm` ∪ `deviceGated` ∪ `blocked`;
    no blocked or control-script-only name is listed, while `licensing.getStatus` still is
    and `system.exec` over raw JSON-RPC still answers with the existing refusal; every tool's
    key set ⊆ `{name, title, description, inputSchema, annotations}`; `initialize` with
    `2025-06-18` returns `2025-06-18`, with `2024-11-05` returns `2024-11-05`, with `1999-01-01`
    returns `2025-06-18`.
- **End to end (maintainer observes, one per OS):** `claude --plugin-dir ./serial-studio -p
  "call io.getStatus"` returns the live status JSON (AC6; the 2026-11-02 Windows run is the
  baseline); installing from the directory reaches the same state (AC8).
- **Portal:** the Validate step reports zero Blocking findings; the result is recorded in this
  spec directory (AC5).
- **Hotpath:** not touched; no benchmark run required.
- **Static:** `python scripts/syntax-check.py` on every edited C++ file; `python
  scripts/code-verify.py --check` on the touched files; `qt-cpp-review` before handoff;
  `python scripts/documentation-verify.py` on the edited Markdown; `python
  scripts/sanitize-commit.py` before each commit (spec and plan commit first, then the JSON and
  generator, then the handler, then tests and docs).
