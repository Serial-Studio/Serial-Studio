---
spec: 0091-console-tx-library
title: Console TX Command Library
status: done         # draft -> approved -> in-progress -> done | shelved
# AC1-AC4, AC7 await maintainer verification in the running app (next build); AC5/AC6
# tests are written and registered but run only against a built app / build dir.
created: 2026-10-02
author: Alex Spataru
---

# Spec 0091 — Console TX Command Library

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

During device bring-up a user sends the same handful of commands over and over: a reset
sequence, a status query, a register poll with a CRC. The console's send history exists
only in memory — it is lost on every restart — and the only way to *save* a transmit
preset is to create a project Action through the Project Editor. That round-trip does not
fit the bring-up phase: it typically happens *before* a project exists (Quick Plot, a
bare console session), which is exactly when repeated sends are most frequent. Users keep
a second terminal tool open for this, or retype.

Two secondary pains compound it. First, the console can append a checksum to every send,
but project Actions cannot — so a command that works in the console cannot be saved as an
Action without losing the checksum the device requires. Second, the console and Actions
encode their payloads through parallel logic that has already drifted (the checksum gap
is the visible symptom); every future TX feature would widen the fork.

## Goals

- Console send history survives an application restart.
- A user can save ("pin") a send — payload plus the framing settings that produced it —
  and re-send it in at most two gestures, with no project open.
- A user can arm cyclic re-send of the current send-field content at a fixed interval,
  and see clearly that it is armed.
- A pin or history entry can be promoted to a project Action without retyping anything.
- Project Actions can append the same checksums the console offers.
- For identical payload and framing settings, a console send and an Action produce
  byte-identical output on the wire.

## Non-Goals

- **No multi-step sequences.** Ordered or conditional command flows stay the job of
  project Actions (timers) and control scripts; the cyclic feature repeats one payload.
- **No new panel or window.** Everything lives in the existing console send bar.
- **No per-project or per-device pin scoping.** Pins are one app-global list (maintainer
  decision, 2026-10-02); the per-project tier *is* project Actions.
- **No licensing gate.** This is a free feature (maintainer decision, 2026-10-02).
- **No new remote-API command surface.** Existing commands keep their behavior; the only
  externally visible schema change is the new optional checksum field on serialized
  Actions.
- **No RX-side changes.** Frame detection, receive-side checksum validation, and the
  acquisition pipeline are untouched.

## Requirements

1. **R1 — Persistent history.** The console send history is restored on the next
   application start, most recent first, with the same capacity and recall gestures
   (Up/Down) as today. Every send remains its own entry — no collapsing of consecutive
   duplicates (maintainer decision, 2026-10-02).
2. **R2 — Pins.** The user can pin the current send-field content, or any history entry.
   A pin stores the payload and the framing state that produced it: input mode
   (text/hex), line ending, checksum choice, and text encoding. Pins can be renamed,
   reordered, and deleted. Pins are stored at application level and are available in
   every session, project or not. Pins are device-agnostic: a recalled pin sends to the
   console's currently selected device, never to a remembered one (maintainer decision,
   2026-10-02).
3. **R3 — One-gesture access.** A single control in the send bar opens a list showing
   recent history, pins, and — when a project is open — that project's Actions. Choosing
   an entry fills the send field (restoring its framing state for pins); sending remains
   an explicit gesture. Project Actions chosen here fire exactly as they do from the
   dashboard actions toolbar.
4. **R4 — Cyclic re-send.** The user can arm repetition of the current send-field content
   at a user-set interval (milliseconds), minimum 1 ms, default 1000 ms. While armed, the
   state is visibly indicated. Repetition suspends while the dashboard is paused
   (matching Action timers), stops on disconnect, and never queues missed sends. The
   1 ms floor is a deliberate trust-the-user call (maintainer decision, 2026-10-02): the
   application does not protect a slow link from an interval faster than its throughput.
5. **R5 — Promote to Action.** From a pin or history entry the user can create a project
   Action pre-filled with the payload, encoding, line ending, and checksum. The new
   Action appears in the Project Editor and the dashboard actions toolbar, and the
   creation is undoable like any other project edit.
6. **R6 — Action checksum.** Project Actions gain an optional checksum, chosen from the
   same set the console offers, default "none". Project files written before this change
   load unchanged; a file without the field behaves as "none"; a file with it round-trips
   through save and load.
7. **R7 — Encoding parity.** Given the same payload, input mode, escapes, encoding, line
   ending, and checksum, the bytes written to the device are identical whether the send
   originates from the console send bar or from an Action.

## Acceptance Criteria

- [ ] **AC1** (R1) — Send three commands, quit, relaunch: Up-arrow recalls them in order.
      Maintainer observation in the running app.
- [ ] **AC2** (R2, R3) — With no project open, pin a hex payload with CRC-16 selected;
      clear the field; recall the pin from the send-bar list and send: the device
      receives the original bytes. Observation against a loopback link.
- [ ] **AC3** (R4) — Arm cyclic send at 500 ms against a local TCP loopback: the peer
      receives the payload at that cadence; pausing the dashboard suspends it;
      disconnecting disarms it. Observation, optionally scripted via the API test
      harness on the receiving side.
- [ ] **AC4** (R5) — Promote a pin: the Action shows up in the Project Editor with all
      fields carried over; one undo removes it. Maintainer observation.
- [ ] **AC5** (R6) — A `pytest` project round-trip check: saving a project whose Action
      has a checksum and reloading it preserves the choice; loading a pre-0091 project
      file produces Actions with checksum "none" and no load warnings.
- [ ] **AC6** (R7) — An automated unit check (ctest tier) feeds the same inputs through
      the console send path and the Action path and asserts byte-identical output, for
      at least: text + escapes + CRLF, hex + CRC-16-MODBUS, UTF-8 text + no EOL +
      CRC-32.
- [ ] **AC7** — `--benchmark-hotpath` gates unchanged (the feature is command-rate TX
      only and must not touch the acquisition pipeline).

## Constraints & Invariants

- **Project Actions remain the only durable, project-serialized preset tier.** The
  console tier (history, pins) is application-level scratch state and is never written
  into a project file.
- **Project schema compatibility both directions:** older application versions must
  tolerate (ignore) the new Action field; the new version must load older files with
  the documented default.
- **One encoding definition.** After this change there is a single source of truth for
  TX payload encoding (mode, escapes, text encoding, line ending, checksum); console and
  Actions both consume it. Checksum coverage keeps today's console semantics (computed
  over the payload including the appended line ending).
- **Command-rate only.** Nothing here adds work to the acquisition pipeline or any
  per-frame path; cyclic sending is a UI-rate timer.
- **Undo discipline.** Promotion mutates the project through the same undoable mutation
  path as existing Action edits.
- **UI conventions.** New send-bar UI follows the console's existing conventions,
  including reduced-motion behavior and command/icon registration rules.
- **No new third-party dependency.**

## Open Questions

None. The three drafted questions (history duplicate collapsing, cyclic interval floor,
pin device affinity) were resolved with the maintainer on 2026-10-02 and folded into
R1, R4, and R2 respectively.
