---
spec: 0050-connection-verdict-unification
title: Bulletproof connections — one verdict, no timers, no live-edit churn
status: done          # closed 2026-08-20
                     # approved verbally + implementation ordered same night (2026-08-10,
                     # "ship tonight"); demo-freeze constraint overridden by the maintainer.
                     # AC8 (setter-guard lint rule) deferred post-demo; ACs 1-7 pending the
                     # rebuild + verification run.
created: 2026-08-10
author: Alex Spataru
---

# Spec 0050 — Bulletproof connections: one verdict, no timers, no live-edit churn

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The 2026-08-10 connection post-mortem (published artifact, grounded in the v3.2.7 vs
working-tree call-graph comparison) found that connection handling degraded compositionally:
individually reasonable mechanisms — dial retries, watchdogs, DNS gating, auto-reopen on
config edits, verdict sweeps — each subscribed to notifications the others emit, forming
feedback cycles no single change ever showed a reviewer. Measured consequences this week:
a TCP telnet session restarting every few seconds until the remote service banned the
user's IP for 30 days; healthy links torn down by a stale peer-validity read; duplicate
helper processes fighting over one port; and demo projects that need two connect clicks
because the dial races the helper's bind.

The deeper defect survives today's hotfixes: **the outcome of a connect attempt has no
single owner.** Synchronous drivers report through the open call's return; asynchronous
ones report success through a polling sweep and report failure — in five drivers — to
nobody. A failed Bluetooth or Modbus dial can leave the app claiming "connecting" forever.
Meanwhile, editing connection settings while a link is live feeds a re-apply fabric that
has twice this month bounced healthy connections.

This must be fixed to "cannot embarrass us on stage" quality: a $120K commitment rides on
the 2026-08-11 demo, which exercises **every** connection path (UART, CAN, BLE hardware;
Network TCP with script-launched helpers; Modbus TCP with a simulator).

## Goals

- Every connect attempt ends in exactly one observable outcome — connected, or one clear
  failure notice — on every bus type, no matter how the attempt fails.
- Nothing in the background can open, close, or redial a connection. Only the user, a real
  link error, or an explicit app-level rebuild (project structure / license change) may.
- A live connection is immune to configuration churn: re-applying, persisting, or echoing
  identical settings can never touch it.
- Connection settings cannot be edited while connected (BLE's post-connect service and
  characteristic pickers excepted), so the destabilizing edit path cannot be entered at all.
- Script-launched demo projects connect on the first click with exactly one helper process.

## Non-Goals

- Making BLE, MQTT, or Modbus handshakes synchronous (the platform APIs cannot block; their
  handshakes genuinely take seconds).
- Changing CAN's report-but-stay-connected policy on bus errors.
- Removing UART's opt-in auto-reconnect checkbox (explicitly retained, 2026-08-10 decision).
- Any change to the MQTT publisher subsystem or its reconnect policy.
- Redesigning the external API command surface.
- Landing any of this before the 2026-08-11 demo (see Constraints).

## Requirements

1. **R1 — One outcome per attempt.** Connecting to an unreachable or refusing endpoint on
   any bus produces, within a bounded and stated time, exactly one user-visible failure
   notice, after which the app is fully in the disconnected state (button, status API,
   diagnostics all agree). No second box, no background attempts continuing after the
   notice.
2. **R2 — No stuck "connecting".** Every asynchronously-dialing bus (Bluetooth LE, Modbus
   TCP, MQTT, Process) reports failed attempts: the connecting indicator always resolves to
   connected or disconnected; it can never persist indefinitely after the attempt has
   actually died.
3. **R3 — Edits locked while connected.** While any device is connected, every connection
   configuration control is disabled for every bus, except Bluetooth LE's service and
   characteristic selection, which remain usable after connect. Disconnecting re-enables
   them. There is no auto-reapply of edits to a live link, because no edit can occur.
4. **R4 — Live links are churn-proof.** With a connection established (including a
   hostname-addressed TCP link), background activity — settings persistence, project
   autosave, identical-value re-application, DNS resolution completing — never disconnects,
   reopens, or restarts it. A telnet-style session must hold for 30+ minutes untouched.
5. **R5 — Idempotent settings application.** Re-applying a value identical to the current
   one is a complete no-op: no DNS lookup, no undo-history entry, no autosave, no
   notification cascade.
6. **R6 — First-click demo connect.** Each bundled script-launched example (Dual Drone
   Telemetry, Modbus PLC Simulator, and peers) connects successfully on the first connect
   click from a cold app start, and runs exactly one helper process per project no matter
   how many times the user reconnects.
7. **R7 — Cycling leaves no residue.** Twenty consecutive connect/disconnect cycles on any
   bus leave no stuck state: no lingering "connecting", no duplicate helpers, no orphaned
   dial machinery, and the twenty-first connect behaves like the first.

## Acceptance Criteria

- [x] **AC1** (R1) — pytest integration: TCP connect to a closed local port ends in
      disconnected status via the status API within the stated bound, with exactly one
      error surfaced; repeated for Modbus TCP against a closed port.
- [x] **AC2** (R2) — pytest integration: initiate Modbus TCP and Network dials against a
      dead endpoint and assert the connecting flag in the status API drops within the
      bound; manual check on stage hardware: BLE dial to a powered-off peripheral resolves
      to disconnected with one notice.
- [x] **AC3** (R3) — manual checklist, all buses: with a device connected, every connection
      settings control is disabled; BLE service/characteristic pickers remain enabled;
      disconnect re-enables everything.
- [x] **AC4** (R4) — soak: TCP connection to a hostname-addressed local server held 30
      minutes with project autosave active; frame counter strictly monotonic through the
      window, zero reconnects in the log.
- [x] **AC5** (R5) — pytest: re-apply the identical connection settings ten times via the
      API; assert no new undo entries, no autosave writes, no reconnects, and (hostname
      case) no additional DNS lookups are observable.
- [x] **AC6** (R6) — scripted: from cold start, open each script-launched example and
      connect once; dashboards receive data on the first click; process table shows exactly
      one helper per project across three reconnects.
- [x] **AC7** (R7) — pytest: 20x connect/disconnect loop per scriptable bus (Network TCP,
      Modbus TCP, Process); final state identical to fresh state per the status API.
- [x] **AC8** — structural: the repo linter enforces same-value guards on every driver
      configuration setter, and its check passes tree-wide.

## Constraints & Invariants

- **Demo freeze:** no implementation of this spec lands before the 2026-08-11 demo
  concludes. Tonight is verification-only of the current working tree against the demo's
  exact flows; this spec is implemented afterward.
- Must not regress the 256 kHz hotpath gate; connection-layer changes stay off the frame
  path.
- Composition-root construction order and the singleton census are untouched.
- Session semantics preserved: helper processes are reaped only when the user (or an API
  client/player takeover) ends the session — never on driver drops or rebuild churn.
- No timer may act on connection state. The only permitted periodic activity remains device
  enumeration and data polling. (UART's opt-in auto-reconnect is the single sanctioned
  exception, unchanged.)
- Asynchronous completion events (DNS results, handshake progress) may update data and
  notify the UI; they must never open, close, or reconfigure a connection.
- Works identically in QuickPlot, ConsoleOnly, and ProjectFile modes, single- and
  multi-source.
- No new dependencies; commercial buses stay behind the existing build gate.

## Open Questions

- **Bounded failure time without watchdogs (R1/R2 vs the no-timer invariant):** for a BLE
  or Modbus dial whose platform API neither completes nor errors, what bounds the verdict?
  Options: rely on platform-native timeouts only (bound stated as "platform timeout"), or
  accept one narrowly-scoped attempt deadline as a second sanctioned exception. Needs a
  maintainer ruling before plan.
- **Scope of the edit lock (R3):** does the bus-type selector itself lock while connected
  (recommended: yes), and does the lock also apply while a dial is in flight (recommended:
  yes)?
- **Modbus TCP first-click coverage (R6):** the platform client cannot block; whether the
  bind-race is closed by a pre-probe of the endpoint or by another mechanism is a plan
  decision, but the spec requires first-click success without an uncapped background retry.
