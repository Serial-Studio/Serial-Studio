---
spec: 0045-api-terminal
title: API Terminal window
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-08-05
author: Alex Spataru
---

# Spec 0045 — API Terminal window

> **Superseded in part by spec 0046 (2026-08-05):** the window shipped, then was folded
> into `Dialogs/Macros.qml` as its Terminal tab. The `app.apiTerminal` command and
> `commands/api-terminal` icons were replaced by `app.macros` / `commands/macro` at the
> maintainer's request. All terminal behavior (R1-R10) lives on inside the Macros window.

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio exposes 359 API commands (`<scope>.<verb>`) over the TCP API server, and the
same surface backs the SDK, MCP, and the script `apiCall` gateway. Today the only ways to
try a command are external: hand-craft NDJSON against `localhost:7777` with `nc`/a script,
or write a throwaway SDK snippet. There is no in-app way to discover what commands exist,
read what a command does and what parameters it takes, or run one and see the response —
even though the app already ships the complete machine-readable catalog
(`api-schema.json`: name, description, parameters, required) inside its own resources.

This hurts two audiences: users building automations against the API/SDK (discovery and
experimentation loop is slow and out-of-app), and the maintainer debugging handlers (no
quick way to fire one command and inspect the reply without a test harness).

## Goals

- A user can open an "API Terminal" window from the command palette / menus and run any
  API command interactively, seeing the response immediately.
- A user can discover commands by browsing a tree grouped by scope and by typing in a
  search box that filters the tree live.
- A user can read per-command documentation (description, parameters, which are required)
  without leaving the app or opening the online docs.
- The terminal input autocompletes command names as the user types.
- The whole flow works with **Settings → Enable API Server turned off** — no socket, no
  server dependency.

## Non-Goals

- Not a remote client: it never connects to another Serial Studio instance (remote attach /
  mirror are separate features).
- Not a scripting console: single command per invocation, no JS/Lua evaluation, no
  multi-command batches beyond what existing batch commands already offer.
- No emulation of the socket-only concerns (auth, rate limits, message-size caps) — those
  belong to the wire path and are out of scope for a local tool.
- No new API commands and no terminal-only command surface: the terminal is a pure consumer
  of the existing registry.
- No editing of the documentation content: docs render what the generated schema ships.

## Requirements

1. **R1** — A new "API Terminal" command exists in the command registry (palette, Start
   menu / toolbars per its declared contexts) with the `commands/api-terminal` icon, and
   opens the API Terminal window.
2. **R2** — The window contains a terminal area: an input line plus a scrollback log. A
   submitted command is echoed, executed **in-process**, and its JSON response (success or
   error) is printed. Invalid input (unknown command, malformed parameters) produces the
   same error shape the API server would return, never a crash or silent no-op.
3. **R3** — The input line autocompletes command names from the full registered catalog as
   the user types; accepting a completion inserts the command. After a command name is
   chosen, the terminal surfaces its parameter names (required ones distinguishable) so the
   user can fill values without consulting external docs.
4. **R4** — The window contains a command tree grouped by scope (`io`, `project`,
   `dashboard`, …) with a search field above it that live-filters by command name and
   description text. Selecting a tree entry shows that command's documentation: full name,
   description, each parameter with its schema info, and which parameters are required.
   Activating a tree entry (double-click / return) inserts the command into the input line.
5. **R5** — Documentation and completion data come from the registered command catalog —
   the same single source of truth that generates the bundled `api-schema.json` the SDK
   ships. No hand-maintained duplicate list anywhere.
6. **R6** — The terminal keeps an input history navigable with Up/Down within the session.
7. **R7** — Standard console conveniences: copy, clear, select-all on the scrollback.
8. **R8** — The window is a standalone window (same family as the Database Explorer
   window), resizable, with geometry persisted across sessions.
9. **R9** — The feature is available in GPL builds (no license gate). Commands that
   themselves touch Pro features keep whatever gating they already have — the terminal
   adds none and removes none.
10. **R10** — Executing commands from the terminal behaves identically to the socket path
    at the handler level: same registry, same handlers, same responses (socket-transport
    concerns excepted per Non-Goals).

## Acceptance Criteria

- [x] **AC1** — With the API server setting off, open the palette, run "API Terminal",
  type `api.getCommands`, press return: the response lists the full command catalog.
  (In-app observation.)
- [x] **AC2** — Typing a prefix (e.g. `io.`) in the input shows completions restricted to
  that scope; accepting one inserts it. (In-app observation.)
- [x] **AC3** — Running an unknown command (`foo.bar`) prints an error response matching
  the API server's unknown-command error shape. (In-app observation, cross-checked against
  a socket session.)
- [x] **AC4** — Search box: typing a word from a command description filters the tree to
  commands containing it; clearing the box restores the full tree. (In-app observation.)
- [x] **AC5** — Selecting `project.addMany` (or any parametered command) in the tree shows
  its description and parameter table with required parameters marked, matching
  `api-schema.json` content exactly. (In-app observation + spot-check against the file.)
- [x] **AC6** — Up-arrow recalls the previously submitted command. (In-app observation.)
- [x] **AC7** — Close and reopen the window: geometry restored. Restart app: window
  reopens via the command with default content. (In-app observation.)
- [x] **AC8** — GPL build (no `BUILD_COMMERCIAL`): the command and window are present and
  functional. (Maintainer build check.)
- [x] **AC9** — `registry-verify.py` and the sanitize pipeline pass with the new command,
  icon, and bindings registered. (Scripted check.)
- [x] **AC10** — Hotpath benchmark gate unaffected: `--benchmark-hotpath` still passes at
  the configured floors. (CI gate.)

## Constraints & Invariants

- **In-process execution happens on the GUI thread from user interaction** — never from
  the frame path; nothing the terminal does may touch the parse hotpath or add per-frame
  work. Idle cost of an open terminal window is zero on the data path.
- **Single source of truth**: completion, tree, and docs all derive from the generated
  schema; a hand-edited copy of any command metadata is a defect (spec 0036/0037 rules).
- **Spec-0028 registry discipline**: the new command is one manifest entry + one bindings
  entry; the icon resolves through `IconRegistry` (`commands/api-terminal`, tiers
  16/24/32 — already normalized and registered in rcc).
- **No new dependency** (no terminal emulation library; plain QML text components).
- All user-facing strings translatable per the existing i18n pipeline.
- Command execution reuses the existing handler dispatch; the terminal must not fork a
  second dispatch path that could drift from the server's.

## Open Questions

- Persist input history across app restarts (in Settings), or session-only? (R6 requires
  session-only; cross-restart is a cheap possible extension.)
- Should destructive commands (e.g. `app.quit`, project-clearing verbs) get a
  confirmation prompt in the terminal, or is the terminal a power tool that does exactly
  what it is told? (Current lean: no confirmation — parity with the socket path.)
- Parameter autocomplete depth: R3 requires surfacing parameter names; inline completion
  of parameter *values* (enums, file paths) is not required — worth it later?
