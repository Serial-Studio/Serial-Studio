---
spec: 0098-trust-follows-the-author
title: Code never runs with more trust than whoever supplied it
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-10-14
author: Alex Spataru
---

# Spec 0098 — Code never runs with more trust than whoever supplied it

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

> Evidence: the 2026-10-13 app-level review of db2debec4 (five slice reviewers, an adversarial
> verifier per slice, and a hand re-check of the critical findings). This spec takes the six
> findings that share one root cause, the consent and trust-boundary class. All six were
> confirmed against the code and still hold at 51e00a1e5. The review's other 47 findings go to
> a separate remediation spec (0099), because changes to the API consent surface are reviewed
> on their own and never ride inside a bulk change.

## Problem / Motivation

Serial Studio lets outside callers drive it: API clients over TCP, gRPC and MCP, the in-app AI
assistant, and remote dashboard viewers attached to a publisher. Project scripts can call the
whole API with full trust, because the user wrote those scripts or consented to installing
them. To keep a caller from smuggling code into that trust, the API
asks for consent before a remote client installs script code, writes to a device, or (through
a script) starts a program.

The gate decides by **command name**. A fixed list names the commands that install script code,
another names the device writes, and every command on neither list runs without a prompt. So
any command that makes code *run*, loads a project, or chooses a program to start, without
appearing on a list, hands its caller full trust. The review found six such paths in ordinary
sessions:

| Id | What a caller can do today, with no consent prompt |
|----|----------------------------------------------------|
| api-storage:f1 (critical) | Send code to any of the four dry-run / compile-check commands. Its top level runs with the script API live, so it can write to the device, replace the control script, or reach `system.exec`, which is otherwise control-script only. The control-script dry run already makes the API inert. The other four do not. |
| api-storage:f2 | Run `sessions.regress` with a project of its own. The regression child process loads it and runs its parser and transforms with full trust. |
| api-storage:f3 | Call `assistant.restore` with any project file under the home or temp folder. Its scripts are installed over the open project, saved to the open project's file, and inherit that project's launch consent. |
| api-storage:f7 | Set a Process source's program and arguments through the generic source-property commands, then connect. The program starts with no launch consent. |
| ui-app:f1 | (The AI assistant.) Run `project.batch`, whose inner operations never pass the assistant's safety tiers. Blocked commands, device writes with device control off, and always-confirm commands all run under one approval of a batch, or silently with auto-approve on. |
| api-storage:f4 | (A malicious or spoofed publisher, over the unauthenticated LAN link.) Send a project whose parser and Canvas code run on the viewer's machine with full API trust. |

Spec 0096 closed this class for package sessions only. There, every command must be
classified, and anything unclassified is refused. Ordinary sessions, which is every user with
the API server or MCP enabled, have no equivalent, and each new command that runs code is one
forgotten list entry away from reopening the hole. The deciding constraint: **code never runs
with more trust than whoever supplied it.** Code the user wrote or consented to keeps full
trust. Code a caller supplies runs only with the caller's own permissions.

## Goals

- No remote client, through any single command or chain of commands, can run code it supplied
  with trust it does not have. The same goes for loading a project, choosing or starting a
  program, or writing to a device without the consent those actions already require.
- The assistant cannot run a command its safety tier forbids, whether it calls the command
  directly or nests it inside another command.
- A dashboard viewer attached to a publisher can only be made to display. It runs none of the
  publisher's logic, and the publisher's Canvas widgets still draw.
- A future command cannot quietly reopen the class: CI fails while any command lacks a
  classification.
- Existing integrations keep working. Data reads, dashboard reads and device control see no new
  prompt, and the newly covered commands use consent kinds integrations already handle.

## Non-Goals

- **Authenticating or encrypting the mirror link.** The LAN trust model of v1 stays. This spec
  only makes a hostile publisher harmless to the viewer.
- **New kinds of consent prompt.** The existing script-change, device-write and launch consents
  cover every case.
- **Changing the trust of code the user authored or consented to.** Installed project and
  control scripts keep full API access, including `system.exec` from the control script.
- **Package (sealed) session policy.** Spec 0096 behavior is unchanged. It composes with this
  spec, and a sealed session still refuses everything it refuses today.
- **Licensing behavior** of any kind.
- **The review's other 47 findings**, including the API server's unbounded stream buffering
  and the extension uninstall path. Those belong to spec 0099.
- **Protection from a caller the user has already granted consent.** Consent granted means the
  action is allowed.

## Requirements

**Classification**

1. **R1** — Every command reachable through the API (TCP, gRPC, MCP), through a project script's
   API call, or through the assistant is classified by what it can do. At minimum the classes
   are: evaluates code the caller supplies, installs or replaces script code, loads a project,
   chooses or starts a program, writes to a device. The remote gate decides by these
   capabilities, never by matching a name.
2. **R2** — If a command's effect depends on its arguments, the gate follows what the given
   arguments do. Setting which program a source starts needs the same consent as starting
   one. Setting a baud rate needs nothing.
3. **R3** — A command with no classification is refused to remote clients, and a check that runs
   in CI fails while any registered command is unclassified.
4. **R4** — A command that runs other commands (batches, assistant forwards, a script's API call
   made on a caller's behalf) applies the caller's gate to every inner command. This is
   today's rule for remote clients, and it now extends to the assistant's tiers.

**Code a caller supplies**

5. **R5** — A dry run or compile check, whoever requests it, leaves no trace outside itself. It
   makes no API call with effect, no device write, no file and no process, and it does not
   change live tables or variables. It returns the same diagnostics as today. An API call in
   dry-run code reports that the API is unavailable, the way the control-script dry run already
   does.
6. **R6** — A command that loads a project from a document or path the caller supplies needs the
   script-change consent, exactly as opening a project does today, which asks whatever the
   project contains. This covers checkpoint restore and the historian's regression,
   verification and replay, which load the project stored in a database the caller chose. The
   caller's code never runs unconsented anywhere, including a helper process the app starts on
   its behalf.
7. **R7** — Restoring a checkpoint through the API accepts only checkpoints the app itself
   recorded for the open project. Any other file is refused, and the open project's file is left
   untouched. The documented flow, where a mutation's reply names a snapshot and passing it back
   to restore undoes the change, keeps working.

**Programs**

8. **R8** — No Process source starts a program without the project's launch consent, the same
   per-project consent a script's launches need. This holds whatever chose the program or
   triggered the start: an API command (dedicated or generic), a bus switch that reuses the
   saved program, a project load, a reconnect, or the local Connect button. In a headless run,
   a project named on the command line carries the operator's launch consent for that run
   (maintainer decision, 2026-10-14).

**Assistant**

9. **R9** — Every operation the assistant causes to run passes the assistant's safety tier for that
   operation, including each operation inside a batch.
   - A blocked operation is refused inside a batch exactly as it is when called directly.
   - An operation that needs confirmation is confirmed, and the approval names every operation
     the batch will run.
   - Device control stays off inside a batch when the user has it off.

**Dashboard viewer**

10. **R10** — A viewer runs none of the publisher's parser, transform or control code.
11. **R11** — Canvas widgets the publisher defines still draw on the viewer, but their code can only
    draw. It cannot call the API or reach devices, files or processes. Every widget that renders
    from data renders as it does today (maintainer decision, 2026-10-14).

**Honest docs**

12. **R12** — The API reference states which consent each command class needs, and that dry runs
    have no side effects. The remote dashboard help page states that viewers never run
    publisher logic and that Canvas code on a viewer can only draw.

**Launch consent after a remote change**

13. **R13** — The first time in a session that a remote client changes a project's scripts or
    program, or loads a project, that project's remembered launch consent is cleared, so its
    next launch asks again. Once the user grants it again, later remote changes in the same
    session do not clear it (maintainer decision, 2026-10-14).

## Acceptance Criteria

- [ ] **AC1** (R5) — Integration (pytest, consent not granted): each dry-run and compile-check
      command, in JS and Lua, receives code whose top level tries to write to the device, replace
      the control script, start a program and set a variable. None of those effects happens: the
      device sees no bytes, the control script and variables are unchanged, and no marker file
      appears. Valid and invalid code still return today's diagnostics.
- [ ] **AC2** (R6) — Integration: `sessions.regress`, `sessions.verify`, `sessions.replay` and
      `assistant.restore`, given a project that contains scripts, are refused with the
      script-change consent error. The candidate's marker code never runs, in the app or in a
      regression or verification child process. With consent granted, all four work as today.
- [ ] **AC3** (R7) — Integration: `assistant.restore` with a path outside the app's checkpoint store
      is refused, and the open project's file is byte-identical afterwards. Restoring the
      snapshot path that a mutation's reply returned still undoes that mutation.
- [ ] **AC4** (R8, R2) — Integration: without launch consent, a remote client (a) sets a Process
      source's program and arguments through the generic source-property commands and connects,
      and (b) switches the bus to Process so the saved program is reused, and connects. In
      neither case does a process start or a marker file appear. A baud-rate change through the
      same commands still needs no consent. Maintainer observation: opening a project with a
      Process source from disk and pressing Connect asks once. After Yes it connects, and later
      connects do not ask. A headless run with that project on the command line connects
      without a prompt.
- [ ] **AC5** (R9, R4) — Unit tier: the assistant dispatching a batch that contains a blocked
      operation, a device write with device control off, and an always-confirm operation
      handles each one exactly as it would a direct call. Maintainer observation: the approval
      for a batch lists its operations.
- [ ] **AC6** (R10, R11) — Integration: a publisher project's parser, transform and Canvas code try
      to reach the API and leave marker effects. A viewer attaches, and no marker effect occurs on
      the viewer. Maintainer observation: the viewer draws the Canvas widget and every data
      widget as before.
- [ ] **AC7** (R1, R3) — Unit tier, in CI: a sweep over every registered command fails when one has
      no classification, and a synthetic unclassified command is refused to a remote client.
- [ ] **AC8** (goals, compatibility) — `pytest tests/integration/` is as green as before, and its
      data-read, dashboard and device-control flows meet no new prompt.
      `SERIAL_STUDIO_API_AUTO_CONSENT=1` still grants every gate for headless runs.
- [ ] **AC9** (R12) — The API reference and the remote dashboard page carry the statements in R12,
      and `documentation-verify.py` reports no new findings on them.
- [ ] **AC10** — `--benchmark-hotpath` passes its gates unchanged.
- [ ] **AC11** (R13) — Unit tier: with launch consent granted for the open project, a remote
      script change clears it, and the next launch reports that consent is required. After the
      consent is granted again, a second remote change in the same session leaves it granted.

## Constraints & Invariants

- **Deciding constraint: trust follows the author.** Code the user wrote or consented to keeps
  full trust. Code a caller supplies runs with at most the caller's permissions. Nothing in this
  spec narrows what consented project or control scripts can do.
- **Protected surface.** This changes API auth and consent behavior. Its failure modes are named
  in review before it lands, and it never rides inside a bulk change. The two to weigh:
  - Gating too broadly breaks integrations. MCP and AI tools use the dry runs to validate code
    and read live data.
  - Gating too narrowly lets a future command that runs code slip through, which is why R3 makes
    classification mandatory and CI-checked.
- **Remote origin stays a dispatch scope.** Nested commands answer to the outer caller's gate.
- **Composes with spec 0096.** In a sealed session the 0096 policy still applies, and nothing
  here allows a command that policy refuses.
- **Local behavior changes in two places only.** Dry runs and compile checks become
  side-effect-free wherever they are triggered (R5), and a Process source asks the project's
  launch consent before its first start (R8).
- **No new consent kinds and no new user-facing setting.**
- **Every build.** Each requirement applies in every build where the command exists, the GPL
  build included.
- **The acquisition hotpath is untouched**, and the 256 kHz gate holds.
- **Layering.** `layer-verify` passes with no new baseline entries.
- **No new third-party dependency.**
- **Trial parity.** Any doc that says Pro content needs a license also names the trial.

## Open Questions

Both resolved by the maintainer on 2026-10-14.

- **Launch consent after a consented script change.** The code confirms the carry-over: launch
  consent is keyed by the project's file path, so a remote script change, or a restore that
  keeps the path, inherits it. Resolved by R13: it is asked again once per session.
- **A Process source in a project opened from disk.** Connecting one started its program with no
  launch consent. Resolved by R8: every start asks the project's launch consent, whatever its
  origin.
