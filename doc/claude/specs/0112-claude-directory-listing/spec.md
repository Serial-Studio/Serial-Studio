---
spec: 0112-claude-directory-listing
title: Claude directory listing: annotated MCP tools and a plugin bundle
status: in-progress     # draft -> approved -> in-progress -> done | shelved
created: 2026-11-02
author: Alex Spataru
---

# Spec 0112 — Claude directory listing: annotated MCP tools and a plugin bundle

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio has shipped an MCP server since July 2025: every API command is an MCP tool,
and the live frame is an MCP resource. Reaching it from Claude today means finding a bridge
script in the examples folder, copying it somewhere permanent, and hand-editing Claude
Desktop's JSON config with an absolute path. Nobody discovers that by accident, and the
manual says so in one paragraph of the API Reference. Anthropic's directory is where people
on paid Claude plans find integrations from inside claude.ai, the desktop app, Cowork and
Claude Code; Serial Studio is not in it.

Researched on 2026-11-02 against Anthropic's current docs: the directory lists two things, a
remote HTTPS server with OAuth, or a plugin bundle read from a public GitHub repository.
A server that lives on the user's machine gets in only inside a plugin bundle; the older
desktop-extension listings are deprecated and refused. A plugin's local server runs in Claude
Code and in Cowork sessions on the user's computer; the Chat surface ignores it by design.

The second half of the problem is inside the server. Measured on the running app (Serial
Studio Pro 4.2.1): 452 tools, none with a title, none with the read-only or destructive
hint that MCP clients use to decide whether a call may run without a per-call prompt. Claude
therefore treats reading a live value and deleting a group alike. The
in-app assistant already classifies commands into safety tiers for exactly this purpose, but
that table covers 397 names: 61 listed commands have no tier (the alarm, OPC UA,
EtherNet/IP, S7, IEC 104, Historian, InfluxDB and CAN families among them), and an unlisted
name silently falls through to the middle tier. The classification exists; it is incomplete
and never reaches an external client.

## Goals

- A person on a paid Claude plan finds "Serial Studio" in the Claude directory, adds it, and
  Claude Code or Cowork can read live telemetry and operate the running app with no config
  file edited by hand.
- Every MCP tool carries a human title and the hint that matches its tier: read-only tools
  run without a prompt, reversible mutations prompt, destructive and device-touching actions
  always prompt.
- The read/write classification has one home. A command's MCP hint and its in-app assistant
  tier are the same fact, and every registered command has one.
- A bundled skill teaches Claude when Serial Studio is relevant, what has to be true first
  (the app running, the API server enabled), and the safe order of work, so Claude does not
  infer a workflow from 452 tool names.
- The listing passes the portal's automated validation on the first submission.

## Non-Goals

- A hosted remote connector (a relay the desktop app dials out to). That is the only way
  into the Chat surface, and it is a separate product decision with its own spec.
- Changing the MCP transport or wire framing, or chasing the newest protocol revision beyond
  what a current client needs to connect. The measured handshake already works.
- Curating or trimming the tool surface (decided 2026-11-02: all commands stay tools, except
  the ones the remote gate refuses unconditionally, see R5).
- Support for claude.ai Chat, the mobile apps, or the desktop app's chat tab. A local server
  cannot reach them.
- Rewriting the bridge in another language, or packaging it as an MCPB bundle.
- Changing what any command does, its consent prompt, or its remote-gate class. Annotations
  describe; they never authorize.
- A Pro gate. The API server ships in every build and the plugin must not imply otherwise.

## Requirements

1. **R1** — Every tool in the MCP tool list carries a human-readable `title` distinct from
   its dotted command name, and an `annotations` object with `readOnlyHint` set for every
   tool. Tools in the destructive families (delete, clear, new, open-replace, install,
   uninstall, device writes, credential and TLS configuration, control-script installs) also
   carry `destructiveHint: true`.
2. **R2** — The hints derive from the single existing per-command safety-tier table. A
   command's MCP hint and its assistant tier can never disagree, and no second list exists.
   A command missing from the table is reported as not read-only, never as read-only.
3. **R3** — The tier table covers every registered API command. The 61 commands that have no
   tier today receive one, and a check fails the build or the test tier when a registered
   command is missing from the table or a tier entry names a command that does not exist.
4. **R4** — No tool name exceeds 64 characters (the directory's limit; the longest today is
   38), and the same check as R3 fails when one does.
5. **R5** — Commands in the table's blocked tier (the licensing mutations and the process
   launcher) and commands the remote gate refuses on classification alone (the
   control-script-only class) do not appear in the MCP tool list. Listing is discovery only:
   whether a name may run over raw JSON-RPC or a direct tool call is decided by the existing
   remote gate, which this spec leaves unchanged. (Amended during planning: the remote gate
   does not refuse the licensing mutations today; the blocked tier is the assistant's.)
6. **R6** — Tool objects carry only fields the MCP tool schema defines; the two non-standard
   keys every tool emits today either move into the protocol's extension slot or go.
7. **R7** — A public plugin bundle exists, with the plugin at the root of its own repository,
   declaring the local bridge as a plain file command, bundling at least one skill, a README
   with a Privacy Policy section, a license, and an icon. It passes the portal's validation
   with zero blocking findings and is submitted under the permanent name `serial-studio`.
8. **R8** — Installing the plugin in Claude Code with Serial Studio running and the API
   server enabled yields a connected server: the tools are listed, a read-only call returns
   live data, and the user edited no file. The same bridge works on Windows, macOS and Linux
   with a stock Python 3 install and no further dependency.
9. **R9** — When Serial Studio is not running or the API server is off, the bridge exits with
   a failure within a bounded time and its last message names the fix (enable the API server
   in Preferences), so Claude's MCP status shows the cause instead of a hung server.
10. **R10** — The bundled skill states the prerequisite, the read-first workflow (status,
    then frames, then project edits), and the consent prompts the user will see for device
    control and script changes, so Claude explains a refusal instead of retrying it.
11. **R11** — The user manual and the examples README name the directory listing as the
    primary install path and keep the manual bridge configuration as the documented fallback.
12. **R12** — The plugin's privacy policy states what is true: nothing leaves the machine
    except to the local Serial Studio instance; the plugin collects nothing and phones home
    nowhere.

## Acceptance Criteria

- [x] **AC1** (2026-11-03, pytest test_mcp.py, 99 passed on the rebuilt app) — An integration test against the running app lists the tools and asserts, for
  every tool: a non-empty `title`, a boolean `readOnlyHint`, and `readOnlyHint == true`
  exactly for the commands in the table's auto-run tier, `destructiveHint == true` exactly
  for the always-confirm, device-gated and blocked tiers (R1, R2).
- [ ] **AC2** — A unit in the C++ test tier (or an equivalent linter gate in CI) fails when a
  registered command has no tier entry, when a tier entry names an unregistered command, or
  when a command name exceeds 64 characters (R3, R4). It passes on the tree at handoff, which
  means the 61 gaps are closed.
- [x] **AC3** (2026-11-03, same run) — The integration test asserts that no tool named like a licensing mutation or
  the process launcher appears in the tool list, while the same names still answer over raw
  JSON-RPC with the existing refusal (R5).
- [x] **AC4** (2026-11-03, same run) — The integration test asserts every tool object's key set is a subset of the
  MCP tool schema's keys (R6).
- [ ] **AC5** (local half done 2026-11-03: --profile directory passed with no refusal; portal half pending) — `claude plugin validate --profile directory` on the plugin folder reports no
  refusal, and the portal's Validate step reports zero Blocking findings; the maintainer
  records the portal result in the spec directory (R7).
- [ ] **AC6** (Windows done 2026-11-03: three turns, 9.9 s, live status JSON; macOS and Linux pending) — A scripted Claude Code run with the plugin loaded calls `io.getStatus` and
  gets the live status JSON back, on each of Windows, macOS and Linux (R8). The 2026-11-02
  Windows run is the baseline: six turns, live data, no config edits.
- [x] **AC7** (2026-11-03, pytest tests/unit/test_mcp_bridge.py, exit 1 in about six seconds) — A test runnable without the app spawns the bridge with nothing listening on
  the port and asserts a non-zero exit within ten seconds whose final stderr line names the
  Preferences fix (R9).
- [ ] **AC8** — The listing is live in the directory as "Serial Studio" and installing it from
  the directory in Claude Code reaches the connected state of AC6; maintainer observation (R7).
- [x] **AC9** (2026-11-03, documentation-verify and term-verify clean) — The manual's API Reference page and the examples README pass the docs
  linter and name the directory listing first (R11).

## Constraints & Invariants

- **One classification home.** The deciding constraint: the MCP hint, the assistant tier and
  any future connector listing all read the same table. A second copy is the 2026-09
  stored-fingerprint lesson again: two sources that can disagree will.
- **Annotations describe, they never authorize.** The remote command gate (spec 0098), the
  sealed-session policy (spec 0096) and every consent prompt stay exactly as they are. A tool
  marked read-only must not mutate; a tool that mutates must not be marked read-only, and
  the test of AC1 is what says so.
- **Layering.** The tier table must be reachable from the layer that serves MCP without an
  upward include or a cross-library reach; the layer gate and the singleton census stay
  green.
- **Not a hotpath change.** Tool listing and tool calls are command-rate; no per-frame path
  is touched, and the 256 kHz gate is unaffected by construction.
- **Wire compatibility.** Existing TCP JSON-RPC clients and the gRPC server keep working;
  the new tool fields are additive and the removed non-standard keys were never documented.
- **Security-sensitive surface.** The API server's auth and consent files are
  decision-visible; this spec intends no behavioral change there, and any change the plan
  finds necessary is named in chat before it lands.
- **Directory rules bind the plugin.** Text files only, each under 256 KiB; no package
  launcher, no shell wrapper, no secrets, no top-level `bin/`; the bridge is one plain file
  run with plain arguments; the repository is public before publishing.
- **No new dependency** in the app or in the bridge.

## Premise Check

- **Question:** Will a current Claude client load the existing MCP server through a
  plugin-declared local command and call its tools, and does the tool surface fit the
  directory's limits, without first reworking the server?
- **Measured** (2026-11-02, Serial Studio Pro 4.2.1 on Windows, API server enabled):
  - Claude Code 2.1.316 with a scratch plugin that declares the examples bridge as the server
    command: the server loaded, `io.getStatus` and `api.getCommands` returned live data in
    six turns, 45.8 s wall, no config edits, no permission denials. The tool list entered
    context as roughly 32.7k cache-creation tokens.
  - Tool list: 452 tools, 195,090 bytes (about 49k tokens at 4 bytes per token), served in
    9 ms; longest name 38 characters, none over 64; 0 with a title, 0 with annotations;
    two non-standard keys on every tool.
  - Tier coverage: 452 listed against 397 tiered; 61 listed commands have no tier; 6 tiered
    names are assistant-only sandbox verbs, not API commands.
  - Handshake: a client offering protocol revision 2025-06-18 received 2024-11-05 and
    proceeded. A read-only call answered in 1 ms.
  - Directory-profile validation of the scratch plugin: one refusal (README missing), one
    warning (icon missing), both listing materials; the server command shape passed.
- **Verdict:** pass. The route works on the server as it stands; the only structural gap is
  the 61 untiered commands, which R3 closes before any hint is derived.

## Open Questions

- **Tier assignment for the 61 untiered commands.** Most are driver configuration (OPC UA
  certificates and trust, EtherNet/IP tags, S7, IEC 104 points, CAN bitrates), which the tier
  definition puts in the device-gated set; the alarm, Historian and InfluxDB verbs split
  between auto-run reads and confirm-level writes. The plan proposes the assignment family by
  family; the maintainer reviews the full list before it lands.
- **The bridge launcher name.** The Windows run used `python`; macOS and Linux ship
  `python3` and often no `python`. A plugin declares one command string for every platform,
  so the plan verifies one name on all three or finds the smallest portable alternative that
  stays a plain-file command.
- **Large replies.** `api.getCommands` returns 92 KB, which the Claude Code harness spilled
  to a file during the premise run. Not a directory requirement for a plugin; noted so the
  plan can decide whether the skill steers Claude away from it.
- **The icon.** The portal locks the listing icon on first save; the plan names the asset
  and its source before the first submission.
