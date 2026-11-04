---
spec: 0102-mcp-native-transport
title: MCP Session Hygiene
status: approved     # draft -> approved -> in-progress -> done | shelved
created: 2026-10-19
author: Alex Spataru
---

# Spec 0102 — MCP Session Hygiene

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

> **Rescoped 2026-11-03 (maintainer decision).** The 2026-10-19 draft asked for native MCP
> transports (Streamable HTTP and a child-process mode), a curated tool roster with discovery
> tools, tool annotations, revision negotiation and Preferences snippets. Spec 0112 (done
> 2026-11-03) delivered the annotations and the revision negotiation, made the directory plugin
> with its bridge the install path, and decided against curating the tool list and against
> changing the transport. This spec keeps only the three session defects the draft found that
> are still in the code. The directory slug keeps its original name so links stay valid.

## Problem / Motivation

An MCP session reaches Serial Studio through the API server's TCP socket, today by way of the
bridge that the Claude directory plugin (spec 0112) launches. The session that socket carries is
wrong in three ways that a conforming host notices.

- **It is noisy.** Every new API connection receives every processed frame by default, and
  nothing switches that off when the connection turns out to be an MCP session. The bridge
  forwards each frame line into the host's JSON-RPC reader. With a device streaming, that is
  hundreds of malformed messages per second; the host logs or drops each one and may close the
  session. The directory listing makes this the first thing a new user meets.
- **Failures look like a broken server.** A tool call that fails validation, is refused by a
  gate, or errors while running comes back as a JSON-RPC protocol error. MCP reserves those for
  protocol faults; a host presents a wrong baud rate the same way it presents a dead server, and
  the diagnostic hints the server builds for the model often never reach it.
- **It promises what it does not do.** The server advertises resource subscription, accepts
  subscribe requests, and never sends an update.

## Goals

- An MCP session carries MCP messages and nothing else, whatever the device is doing.
- A failed or refused tool call reaches the model as a tool result it can reason about.
- The capabilities the server advertises are capabilities it delivers.
- Existing TCP clients that are not MCP sessions see no change.

## Non-Goals

- A native transport (HTTP or child process), a curated tool roster, discovery tools,
  Preferences snippets, or any change to the bridge's framing (decided in spec 0112).
- Tool annotations or revision negotiation (delivered by spec 0112).
- Changing the legacy JSON-RPC API, gRPC, the mirror stream or the typed stream-block
  subscription for clients that are not MCP sessions.
- A streaming telemetry resource or rolling frame history.
- Any change to which commands are licensed, gated or classified.

## Requirements

1. **R1 — Session hygiene.** From the moment a connection sends an MCP `initialize`, the server
   pushes nothing to it that is not an MCP message: no per-frame broadcast, no mirror payload, no
   stream block. A command reached through `tools/call` that would
   attach a push stream to the connection does not attach it on an MCP session; it answers with a
   tool result saying how to read telemetry instead (poll a tool or read the frame resource). A
   plain TCP client on the same server keeps its broadcast.
2. **R2 — Failures are tool results.** A command that fails validation, is refused by any gate
   (remote capability, consent, sealed session, license), or errors while running returns a
   successful JSON-RPC response whose tool result is flagged as an error and carries the message
   and the diagnostic hints the server already builds. JSON-RPC errors stay reserved for protocol
   faults: an uninitialized session, an unknown tool name, a missing name, malformed parameters.
3. **R3 — Honest capabilities.** The server stops advertising resource subscription. A subscribe
   request is answered with a protocol error that says subscription is not supported, rather
   than an empty success.
4. **R4 — Docs and clients.** The API reference's MCP section states the session rules and the
   error shape. The example TCP client and the bridge handle the error shape, and the bridge's
   startup banner stops quoting a stale command count.

## Acceptance Criteria

- [ ] **AC1** — pytest (integration, with the suite's frame-producing fixture active): for five
  seconds after `initialize`, an MCP session receives only JSON-RPC messages, while a plain TCP
  session opened at the same time still receives frame broadcasts. (R1)
- [ ] **AC2** — pytest: on an MCP session, a `tools/call` of a command that would subscribe the
  connection to a push stream returns a tool result flagged as an error, and the session still
  receives only JSON-RPC messages for the next five seconds. (R1)
- [ ] **AC3** — pytest: a baud-rate call with a string argument returns a successful JSON-RPC
  response whose result is flagged as an error and names the parameter and the received type; a
  call refused by the remote gate returns the same shape; an unknown tool, a missing name and an
  uninitialized session still return JSON-RPC errors. The existing error tests are rewritten to
  this shape. (R2)
- [ ] **AC4** — pytest: the `initialize` capabilities do not claim resource subscription, and a
  `resources/subscribe` request gets a JSON-RPC error. (R3)
- [ ] **AC5** — The MCP integration suite is green with only the error-shape tests changed; the
  non-destructive integration suite is green; documentation, terminology and claim verifiers
  pass. (R4)

## Constraints & Invariants

- **The API server's auth and consent surface is decision-visible.** R1 changes what a socket
  receives and R2 changes how gate refusals are reported; neither changes whether a gate allows a
  call. Any plan step that would touch a gate's verdict is named in chat before it lands.
- **Remote origin stays a dispatch scope.** Tool calls keep running under the same remote
  dispatch scope; reshaping the reply never moves where the check happens.
- **Nothing on the hotpath.** Muting is a per-connection flag the broadcast already consults; no
  per-frame work is added.
- **Additive for non-MCP clients.** The only wire change is on connections that sent an MCP
  `initialize`, and R2's error shape, which follows the MCP specification.

## Premise Check

- **Question:** Does an MCP session on today's master actually receive frame lines?
- **Measured:** By code reading on 2026-11-03 (the app was not running, so no runtime capture
  yet): a connection's frame stream defaults on, the MCP branch of client reception records the
  handshake and never mutes the socket, the broadcast writes to every unmuted socket, and the
  bridge forwards every line it receives to the host.
- **Verdict:** Holds on the code. AC1 is written first and must fail on master before the fix,
  which is the runtime confirmation.

## Open Questions

None at the gate. The draft's open questions on the error shape and on resource subscription
were decided by the maintainer on 2026-11-03 as the draft recommended (follow the MCP
specification; stop advertising subscription). Its other questions belonged to the superseded
transport and roster scope.
