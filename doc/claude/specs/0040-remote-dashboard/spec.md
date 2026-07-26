---
spec: 0040-remote-dashboard
title: Remote dashboard attach
status: in-progress  # draft -> approved -> in-progress -> done | shelved
# Retro-flip 2026-07-25: implemented under the spec-0030 campaign approval; the
# per-phase gates were never formally run. T1-T23 + T26 done; T24/T25/T27 and all
# ACs open (maintainer-run).
created: 2026-07-25
author: Claude (roadmap item R7, spec 0030)
---

# Spec 0040 — Remote dashboard attach

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R7 — the **capstone**. Hard dependency: R4 (spec 0039,
> session context) — specifically its **M2** milestone (the context *owns* session
> subsystems), not its M1 seam. Spec 0039 names R7 against its M3: *"a remote dashboard is a
> second context ... which is only expressible once M3 exists."* That is true of the *end
> state* — a GUI holding its own capture and a remote view at once. This spec's v1 is
> narrower: one dashboard whose feed is swapped, which needs M2 and explicitly refuses
> concurrency. **M3 gates P2/P3, not v1. No implementation work under this spec may begin
> before 0039 M2 has landed.** Design, protocol, fixture, and measurement work can and should
> start earlier — see "Phasing".

## Problem / Motivation

A Serial Studio capture runs where the hardware is. The dashboard runs where the person is.
Today those must be the same process on the same machine.

`--headless` already exists and already works: the flag suppresses QML and GUI loading, but
the pinned core-module order still runs in full, so a headless process holds a live project
document, a live frame pipeline, and a live dashboard — it simply has no window. The API
server can be turned on alongside it. What is missing is the other half: a GUI that can point
at that process and *see* it. The GUI renders only the data its own process parsed.

The consequences are the ones the roadmap named and that support conversations keep
re-surfacing in different costumes:

- A long capture on a lab bench, a rack machine, a vehicle, or a Raspberry Pi next to the
  device can only be watched by sitting at that machine, or by exporting after the fact.
- Watching a capture means *owning* it. There is no way for a second person to look without
  either taking over the session or starting a competing one against the same port.
- The existing remote story is "read the API from a script". That gives a developer parsed
  frames over TCP; it gives an engineer nothing they could point at and read. Every
  visualization the product exists to provide — plots, gauges, alarms, widget layout — stops
  at the process boundary.
- A capture that must not be interrupted (a burn-in run, a flight, an endurance test) cannot
  be inspected at all without risking it, because the only viewer is the process doing the
  capture.

The pieces to close this are, unusually, mostly already in the repo: a headless mode that
constructs the whole session; a network transport with authentication, framing, and rate
limits already shipped and already exercised by the test suite; a dashboard that already
coalesces its updates to a display-rate tick rather than the parse rate; and a proven
precedent for feeding the dashboard from something other than live hardware (file replay
pushes already-split channels into the frame pipeline, and every widget downstream is none
the wiser). What is missing is not a transport and not a rendering path. What is missing is
the ability for the GUI process to hold *a session that is not its own* — which is exactly
the thing spec 0039 is built to make expressible, and exactly why this item is the capstone
rather than an early win.

## Goals

- A person running the GUI on one machine can watch a headless capture on another machine,
  live, with the real dashboard — real widgets, real layout, real alarm behavior — not a
  read-out of numbers.
- Attaching does not disturb the capture. Detaching does not stop it. Reattaching resumes.
- More than one viewer can watch the same capture at the same time without any of them
  affecting it or each other.
- Someone looking at an attached dashboard can always tell, at a glance, that it is remote,
  which host it is attached to, and whether the data is currently live or stale.
- The in-process case — the way every user runs the product today — is unchanged: same
  startup, same throughput, same behavior, with the mirroring machinery not merely disabled
  but not present on that path at all.
- The feature costs no new listening port, no new build-time dependency, and no new network
  exposure that the operator did not explicitly turn on.
- Where the mirror cannot faithfully represent something, the product says so rather than
  showing a plausible-looking wrong thing.

## Non-Goals

- **Not remote control.** v1 is read-only: an attached viewer cannot connect or disconnect a
  device, trigger actions, drive output widgets, edit the project, or start or stop exports
  on the remote session. A control channel is a named later phase, not a stretch goal.
- **Not a web viewer.** The client is the desktop GUI. Nothing here delivers a browser-based
  dashboard, and nothing here should be designed as a step toward one without saying so.
- **Not remote project editing.** The project editor stays local to the session that owns the
  document.
- **Not sample-rate fidelity in v1.** Widgets that need every sample rather than a display-rate
  snapshot — spectrum, waterfall, 3D trajectory — are explicitly out of the v1 mirror. They
  come back in a named later phase with a different channel, or they stay local-only.
- **Not history backfill in v1.** An attached dashboard starts at the moment of attach. It does
  not reconstruct what happened before it arrived.
- **Not transport security work.** v1 rides the existing authentication and the existing bind
  controls, unchanged. TLS, scoped tokens, rotation policy, and audit logging are a named
  later phase, and the v1 posture is documented honestly rather than implied.
- **Not a new transport.** No second listening socket, no new protocol stack, no new optional
  build dependency.
- **Not a change to the local frame path.** No file on the capture hotpath is edited by this
  spec. If one needs to be, that is a finding to bring back, not a task to absorb.
- **Not solving 0039.** This spec consumes the session-context work; it does not do it, does
  not partially do it, and does not work around it.

## Requirements

1. **R1** — A GUI user can attach to a running Serial Studio instance (headless or windowed)
   identified by host, port, and the instance's existing authentication token, and can do so
   from a discoverable place in the UI without editing a config file.
2. **R2** — While attached, the dashboard renders the remote session's structure — its groups,
   datasets, widgets, titles, colors, ranges, and alarm configuration — as the remote session
   has them, not as the local machine last had them.
3. **R3** — While attached, widget values update live at a display-rate cadence, and plots
   accumulate from the moment of attach onward.
4. **R4** — Attaching, staying attached, and detaching have no observable effect on the remote
   capture: no frames lost, no exports interrupted, no device state changed.
5. **R5** — Detaching returns the GUI to its own local session in the state it was in before
   attaching, and the remote capture keeps running.
6. **R6** — Reattaching to the same instance produces a correct dashboard again, including
   after the remote session's project structure changed while detached.
7. **R7** — At least two GUIs can be attached to the same instance simultaneously, each seeing
   correct live data, with neither affecting the other nor the capture.
8. **R8** — The attached state is unmistakable in the UI: the user can see that the session is
   remote, which endpoint it is attached to, and whether data is currently flowing.
9. **R9** — When the link degrades or drops, the dashboard visibly stops claiming to be live
   within a bounded time, rather than continuing to display the last values as current.
10. **R10** — Widgets that the v1 mirror cannot faithfully drive are shown as explicitly
    unavailable-over-remote, never as empty, zeroed, or plausibly wrong.
11. **R11** — Remote attach consumes no bandwidth proportional to the capture's parse rate:
    the wire cost is a function of the display cadence and the project's size, not of how fast
    the device is producing data.
12. **R12** — With no viewer attached, a headless or windowed instance does no mirroring work
    at all, and its behavior and throughput are indistinguishable from today.
13. **R13** — A GUI that never attaches executes no mirroring code: the local session's data
    path is the same path as today, with nothing conditional added to it.
14. **R14** — Remote attach is off unless the operator has explicitly enabled the API server
    and explicitly allowed non-loopback connections; nothing about this feature widens network
    exposure by default.
15. **R15** — The user-facing documentation states the v1 trust model plainly — what the
    transport does and does not protect — so an operator can decide whether their network
    qualifies.
16. **R16** — An instance that has never had a graphical session can be made attachable. Today
    the two settings that remote attach requires — enabling the API server, and allowing
    non-loopback connections — are reachable only from a settings dialog or a previously
    persisted value, and one of them is guarded by a modal confirmation that a windowless
    process cannot answer. A headless-only machine must have a non-interactive way to opt in,
    and that way must be as explicit as the dialog it replaces.

## Acceptance Criteria

The roadmap's stated acceptance for R7 is *"`serial-studio --headless project.json` on machine
A; GUI on machine B attaches, sees live widgets, detaches, reattaches. In-process startup time
and hotpath benchmarks unchanged."* That criterion is achievable for **scalar and plot
widgets** and is adopted below with that scope stated, because claiming a spectrum analyzer
mirrors correctly at display cadence would be false.

- [ ] **AC1 (R1, R2, R3, R8)** — Maintainer runs a headless instance with a multi-group project
  on one machine — started from a single command line, with no prior graphical session on that
  machine having configured it — and attaches from a GUI on another. The dashboard shows the
  remote project's widgets with live values; the UI identifies the session as remote and names
  the endpoint. (The roadmap's shorthand `serial-studio --headless project.json` is not
  sufficient today: the API server is not enabled by headless mode, so the real invocation is
  part of what this criterion pins down. See R16.)
- [ ] **AC2 (R4, R5, R6)** — With the attach live, the remote capture's own export/CSV output is
  compared against an unattached control run over the same input: no gaps, no dropped frames,
  no rate change. Detach; the remote keeps capturing (verified from its own logs/exports).
  Reattach after changing the remote project; the new structure renders.
- [ ] **AC3 (R7)** — Two GUIs attached simultaneously both show live, correct, mutually
  consistent values; closing one does not disturb the other or the capture.
- [ ] **AC4 (R9)** — The link is severed mid-stream (cable pull or firewall drop). Within a
  stated bound the dashboard marks itself stale/disconnected; it never presents the frozen last
  values as current. Restoring the link recovers without a restart.
- [ ] **AC5 (R10)** — A project containing a spectrum, waterfall, or 3D widget attaches, and
  those widgets render an explicit "not available over remote attach" state while every scalar
  and plot widget in the same project works.
- [ ] **AC6 (R11)** — A measured comparison on a real project: bytes-per-second on the mirror
  channel is recorded at a low device rate and at a rate at least two orders of magnitude
  higher, and the two are within the same order of magnitude of each other. The measurement is
  recorded in this spec directory, not asserted.
- [ ] **AC7 (R12, R13)** — `--benchmark-hotpath` before and after are within run-to-run noise.
  The diff is reviewed against the capture data path: the only permitted change there is the
  one named cached-flag input, and its change signal is confirmed wired per the dataflow rule.
  Startup time in all three operation modes is unchanged.
- [ ] **AC8 (R14, R16)** — With the API server disabled, or enabled but bound to loopback only,
  an attach attempt from another machine fails. With the token omitted or wrong, the attach is
  refused. The non-interactive opt-in is exercised on a machine with no persisted settings, and
  it does not silently widen exposure when omitted.
- [ ] **AC9 (R15)** — A documentation page states the v1 trust model, names what the transport
  does not protect, and gives the recommended deployment (trusted network or tunnel).
- [ ] **AC10 (protocol, pre-gate)** — The wire contract is written down and exercised by an
  automated client against a recorded fixture, independently of any GUI: structure message,
  snapshot message, epoch/versioning, and the reconnect handshake all round-trip.

## Constraints & Invariants

- **Hard dependency: spec 0039 M2.** Until session subsystems are *owned* by a context rather
  than reached through process-global accessors, an attach can only load the remote's document
  over the local one and never put it back — a demo, not the feature. Implementation tasks are
  gated on M2; design, protocol, fixture, and measurement tasks are not. M3 (plurality) is what
  would allow a local capture and a remote view at the same time; v1 refuses that outright, so
  M3 is a gate for the later phases and must not be waited for here.
- **The capture side's per-frame path gains nothing.** The mirror is produced from
  display-cadence state, never from per-frame code; no frame is copied, re-stamped, or
  re-published for it, and attaching a viewer costs the remote instance exactly what any API
  client connection costs today — no new cost class.
- **The viewing side needs exactly one cached-flag input, and it is named up front.** The
  dashboard's "is a stream available" cache is currently fed only by device-open and
  file-player-open signals; a remote-fed session is a new input to it. That is a known
  silent-breakage class with a documented rule (wire the change signal, direct, into the
  cache refresh), so it is called out here rather than discovered later. It is the *only*
  sanctioned hotpath-adjacent change in this spec; any second one is a finding to bring back.
- **Source owns time.** Timestamps originate at the remote session's driver boundary and are
  carried, not regenerated. The viewing GUI never re-stamps mirrored data with its own clock;
  where a display axis needs a local reference, it is derived, not substituted.
- **No new listening port and no new build-time dependency.** The feature rides transport that
  already ships and is already enabled by an existing user-visible setting. Anything requiring
  an optional, off-by-default build configuration cannot be the v1 channel, because shipped
  binaries would not have it.
- **The existing transport's limits are the budget, not a suggestion.** Message size, buffer
  size, client count, and rate caps already enforced by the server apply to the mirror channel
  unchanged; the mirror must fit inside them at the largest project the product supports, and
  the plan must say what happens when it does not.
- **Existing API clients keep working.** The mirror is additive. No existing command, response
  shape, or broadcast changes meaning for a client that does not ask for the mirror.
- **The local path is byte-identical by absence, not by branch.** Mirroring code must not be
  reachable from an unattached local session at all — not "disabled by a flag on the frame
  path", which is a different and weaker claim.
- **Operation modes are respected.** Attach interacts with ProjectFile, QuickPlot, and
  ConsoleOnly deliberately; the plan states what attaching means in each rather than assuming
  ProjectFile.
- **Licensing is decided, not inherited by accident.** The plan must state which side's license
  governs which capability, because the failure mode of getting this wrong is the known one:
  fallback widgets rendered as if they were the real thing.
- **No telemetry, no analytics, no crash reporting** is added by this feature, and connection
  metadata is not phoned anywhere.

## Phasing

The v1 milestone is deliberately narrower than "remote dashboard". The phases are named so
that later work is scheduled rather than implied.

- **P0 — pre-gate (no 0039 dependency).** Wire contract, message schema, epoch/versioning
  rules, reconnect handshake, a recorded fixture stream, an automated client that exercises the
  contract, and the bandwidth measurement for AC6. Produces documents and test assets only.
- **P1 — v1, this spec's milestone (gated on 0039 M2).** Read-only mirror at display cadence:
  structure + values + link status. Attach/detach/reattach, multiple simultaneous viewers of one
  capture, staleness handling, unavailable-widget states, LAN-trust security posture. Scalar
  widgets and plots only, and one session at a time in the viewing GUI.
- **P2 — fidelity and concurrency (needs 0039 M3).** An opt-in sample-rate channel for spectrum,
  waterfall, and 3D; pre-attach history backfill; and a viewer that can hold its own capture and
  a remote view at once. Its own spec; the bandwidth story is why the first two are not v1 and
  M3 is why the third is not.
- **P3 — control.** Actions, output widgets, connect/disconnect, and project edits from an
  attached viewer, with per-capability authorization. Its own spec; it is the phase where the
  security posture below stops being adequate.
- **P4 — transport hardening.** Transport encryption, scoped and rotatable credentials, and an
  audit trail. Required before P3 ships, and required before anyone is told this is safe on an
  untrusted network.

## Security posture (v1)

Stated plainly because the honest version is neither "it's secure" nor "there is no auth".

**What already exists and v1 relies on:** the API server is off by default; when on it binds to
loopback only unless the operator explicitly enables external connections; when external
connections are enabled, a **non-loopback peer must complete a token handshake** before any
command is accepted, against a cryptographically random per-instance token, with a
per-connection attempt limit and a constant-time comparison; each connection carries a session
identity; message size, buffer size, connection count, message rate, and byte rate are all
capped; and a separate one-time consent gate stands in front of API-originated device writes,
with a small set of process-control commands hard-blocked for remote callers outright.

**What does not exist and v1 does not add:** the transport is not encrypted, so the token and
all mirrored data cross the network in the clear; a loopback client is pre-authenticated and
never sees the handshake, so the token protects the network hop only; the credential is
all-or-nothing against the whole command surface, with no per-capability scoping; the attempt
limit is per connection, so it does not survive a reconnect; and there is no rotation policy
and no audit log. The optional secondary transport is worse still — it is unencrypted *and*
has no token check at all — which is one of the reasons it is not the mirror channel.

**Therefore v1 is LAN-trust**, and the spec says so rather than implying more: remote attach is
supported on a network the operator already trusts, or across an SSH tunnel or VPN, and the
documentation says exactly that. Nothing in v1 is enabled by default. P4 is what changes this
posture, and no marketing claim about remote monitoring should outrun it.

## Open Questions

- **Is remote attach a Pro capability?** It has the shape of one (multi-machine, operator/OEM
  value), and gating it also bounds the support surface. Recommendation: Pro-gate the *attach*
  side; leave *being attached to* ungated so a headless capture does not need a license to be
  watchable. Confirm before planning.
- **Whose license governs widget availability while attached** — the rendering GUI's or the
  remote session's? Recommendation: the renderer's, because that is what is actually drawing,
  and any other answer re-creates the fallback-widget failure class. Confirm.
- **What does attach mean in QuickPlot and ConsoleOnly?** Recommendation: attach forces the
  viewer into a remote session that carries the remote's operation mode, and the local mode is
  restored on detach. Confirm, or restrict v1 to attaching only from an idle session.
- **Does the mirror cadence follow the viewer's UI refresh setting, the remote's, or a
  separately negotiated rate?** Recommendation: negotiated, defaulting below the local UI
  cadence, so a 240 Hz viewer setting cannot multiply wire cost.
- **How does an attached viewer avoid the existing unconditional per-frame broadcast?** The
  server currently pushes every parsed frame to every connected client; a viewer that also
  received that stream would be flooded at exactly the rates R11 exists to avoid. Whether this
  is solved by an opt-out negotiated at handshake, or by the mirror being a separate connection
  role, is a plan question — but it must be answered, and the answer must not break existing
  clients.
- **Is there a discovery story, or is it host:port only?** Recommendation: host:port plus
  remembered endpoints in v1; no network discovery protocol.
- **Should a headless instance be able to refuse viewers** (a max-viewers or viewers-disabled
  setting) independently of the API server being on? Recommendation: yes, one setting, default
  allowing viewers when the API server is on.
