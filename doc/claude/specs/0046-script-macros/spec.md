---
spec: 0046-script-macros
title: Script macros in the API Terminal (mini-VBA)
status: done         # draft -> approved -> in-progress -> done | shelved
supersedes: 0045 "not a scripting console" non-goal (maintainer 2026-08-05); 0045 window hosts this
created: 2026-08-05
author: Alex Spataru
---

# Spec 0046 — Script macros in the API Terminal

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The API Terminal (spec 0045) proved the discovery/execution loop but exposed its ceiling in
the first live session: a single-line input is hopeless for real commands —
`workspace.addTile` auto-fills a JSON skeleton wider than the window — and one command per
Enter cannot express any user logic at all (loop over datasets, check a response, branch,
compose). Meanwhile the app already embeds a JS and Lua SDK: `apiCall()` plus the full SDK
is installed into every script engine, but users can only reach it from places tied to a
project's lifecycle (frame parser, transforms, control scripts, MQTT publisher). There is
no way to write and run an *ad-hoc* automation — "the VBA of Serial Studio" — against a
running instance.

Automation-minded users currently script externally over TCP/gRPC, paying connection setup,
serialization, and round-trip latency for every call; an in-process macro runs the same
command surface with none of that overhead.

## Goals

- A user can write a multi-line JS or Lua script in a real code editor (syntax
  highlighting, mono font) inside the API Terminal window, with the full SDK/`apiCall`
  surface available.
- A toolbar drives the macro lifecycle: **load, save, verify, execute, clear** — a script
  round-trips to a file on disk and can be checked without running.
- Script output (results, `print`/`console.log`, errors with line numbers) lands in the
  terminal scrollback, alongside single-command output.
- A runaway script (infinite loop) is interruptible — it can never permanently hang the
  application.
- The single-command input line keeps working exactly as it does today; macros are an
  addition, not a replacement.

## Non-Goals

- No QML-generating plugin tier — named as the future extension this design must not
  preclude, but out of scope here.
- No background, scheduled, or auto-run-on-startup execution; a macro runs only when the
  user presses execute.
- No new API commands and no change to the wire surface.
- No replacement of project control scripts, frame parsers, transforms, or the MQTT
  publisher script — those stay project-scoped; macros are user-scoped and ad-hoc.
- No macro marketplace/sharing format beyond plain script files on disk.

## Requirements

1. **R1** — The API Terminal window gains a script editor area (tab or pane; layout is
   plan's call) hosting the existing embedded code-editor widget with JS syntax
   highlighting; the user can switch the macro language between JS and Lua.
2. **R2** — Scripts run with the same SDK/`apiCall` environment the in-app engines already
   provide: every registered API command reachable, same response shapes, Trusted origin —
   identical semantics to the 0045 single-command path at the dispatch level.
3. **R3** — Toolbar actions: **Load** (file dialog, plain `.js`/`.lua` files), **Save** /
   save-as, **Verify** (parse/compile check without executing; errors with line numbers),
   **Execute**, **Clear** (editor). Execute is disabled while a macro is running.
4. **R4** — Script output goes to the terminal scrollback: explicit print/log calls,
   the script's final value (if any), and runtime errors with line information.
5. **R5** — A running macro is interruptible: the UI offers a stop control, and the
   watchdog discipline that already guards frame-parser scripts bounds a hung macro. An
   interrupted or crashed macro never crashes or wedges the host (same guarantee the
   existing script engines give).
6. **R6** — Macros execute against the live application state exactly like the terminal's
   single commands (same thread-of-truth, same destructive-verb parity — a macro can do
   what the socket API can do, nothing more).
7. **R7** — Unsaved editor content survives closing and reopening the window within a
   session; the window prompts before discarding unsaved changes on load/clear.
8. **R8** — Same build availability as spec 0045: present and functional in GPL builds.
9. **R9** — The 0045 surfaces (discovery pane, docs, completion, single-line input,
   history) remain fully functional and unchanged in behavior.

## Acceptance Criteria

- [ ] **AC1** — Write a JS macro that calls `apiCall("api.getCommands")`, iterates the
  result, and prints a count: executing prints the count in the scrollback. (In-app.)
- [ ] **AC2** — Same macro shape in Lua works after switching the language. (In-app.)
- [ ] **AC3** — Verify on a script with a syntax error reports the error and line number
  without executing anything; Execute is refused until it parses. (In-app.)
- [ ] **AC4** — Save a macro, clear the editor, load it back: byte-identical content.
  (In-app.)
- [ ] **AC5** — Execute `while (true) {}`: the app stays responsive enough to hit stop /
  the watchdog fires, and the terminal reports the interruption; the app keeps working
  afterwards. (In-app.)
- [ ] **AC6** — A macro that throws mid-run prints the error with line info; subsequent
  macros and single commands still work. (In-app.)
- [ ] **AC7** — Single-command input, completion, docs panel, and history behave exactly
  as before the change. (In-app regression pass.)
- [ ] **AC8** — GPL build: feature present and functional. (Maintainer build check.)
- [ ] **AC9** — `registry-verify.py`, `generate-command-strings.py --check`,
  `code-verify.py --check`, sanitize pipeline all clean. (Scripted.)
- [ ] **AC10** — `--benchmark-hotpath` CI gate unaffected. (CI.)

## Constraints & Invariants

- **All engine work is user-initiated and off the frame path** — nothing per-frame, no
  hotpath contact; an idle editor costs zero.
- **Reuse the existing script-engine discipline**: guarded calls, watchdog-driven
  interruption (the flag is flipped only from the sanctioned watchdog thread), the
  established Lua exception-safety plumbing. No new engine architecture.
- **The embedded code editor's three invariants** (position sync, ShortcutOverride
  forwarding, completer-popup rerouting) must be preserved — its host plumbing is a known
  protected surface.
- **Dispatch parity**: macros reach commands through the same gateway the terminal and
  scripts already use; no third dispatch path.
- **File dialogs follow the macOS reentrancy rule** (defer work out of `fileSelected`).
- **Future-proofing only, not building**: nothing in the macro format or storage may
  assume scripts are always JS/Lua (the later QML-plugin tier extends the same mental
  model), but no QML execution ships here.
- No new third-party dependency.

## Open Questions

- ~~Editor placement~~ RESOLVED (maintainer 2026-08-05): tabs — "Terminal" and "Script"
  tabs in the right pane.
- ~~Macro storage~~ RESOLVED: dedicated `Macros/` directory (capitalized) under app data
  as the file-dialog default; free navigation allowed.
- ~~Execution context~~ RESOLVED: fresh engine per execute (reproducible); REPL
  persistence is a possible later extension.
- ~~Verify depth~~ RESOLVED: parse-only for v1 (syntax/compile + line numbers, no
  execution).
- ~~Lua stop semantics~~ RESOLVED: JS-first — JS macros interruptible via the existing
  watchdog; Lua v1 runs to completion (documented); a debug-hook interrupt is a later
  addition.
