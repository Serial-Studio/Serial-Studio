---
spec: 0102-mcp-native-transport
title: Native MCP Transports and a Curated Tool Surface
status: draft        # draft -> approved -> in-progress -> done | shelved
created: 2026-10-19
author: Alex Spataru
---

# Spec 0102 — Native MCP Transports and a Curated Tool Surface

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio has shipped a Model Context Protocol (MCP) server for a long time: JSON-RPC 2.0
over the API server's TCP socket, with tools, two frame resources, one prompt, and about ninety
integration tests behind it. No MCP host can reach it. Hosts launch a child process and talk
over its stdin and stdout, or they open an HTTP endpoint; none of them speak newline-delimited
JSON over a raw TCP socket. The manual tells the user to "configure your MCP client to connect
to the Serial Studio MCP endpoint", which is not possible as written. The working path is a
Python script in the examples folder that pipes stdin to the socket and the socket to stdout. It
needs Python 3 on the machine, an absolute path into a source checkout in the host's config, it
cannot reconnect (a dropped socket ends the host's session), it is documented for one host
only, and its startup banner still announces "182+ commands" against a catalog of about 390.

The session the bridge hands over is also wrong in three ways that a conforming host notices:

- **It is noisy.** Every new API connection streams every processed frame by default, and
  nothing switches that off when the connection turns out to be an MCP session. The bridge
  forwards those frame lines verbatim into the host's JSON-RPC reader. With a device streaming,
  that is hundreds of malformed messages per second arriving at the host, which logs or drops
  each one and may close the session.
- **It is oversized.** `tools/list` returns every registered command, about 390 schemas in one
  reply. A host puts the whole list into the model's context on every turn. The in-app
  assistant hit exactly this limit and answered it with a curated set of about fifty tools plus
  discovery tools that search, describe and run the rest; the MCP surface has no equivalent, so
  an external agent gets the largest list and no way to search it.
- **It is opaque.** Tools carry no annotations, so a host cannot tell a read from a destructive
  write and cannot apply its own approval policy, even though Serial Studio already classifies
  every command twice (the remote capability classes and the assistant's safety tiers). A
  command that fails or is refused comes back as a JSON-RPC protocol error rather than a tool
  result, so hosts present a wrong baud rate the same way they present a broken server, and the
  diagnostic hints the server builds for the model (which parameter, what type it received)
  often never reach it. Resource subscriptions are accepted and never fire.

The cadcraft comparison that prompted spec 0101 made this visible. Its "agents drive it like a
user" claim is credible because its MCP server is something a host can launch. Serial Studio's
command surface is larger and better gated, and it is the one nobody can plug in. Spec 0101 adds
the window-driving commands to the catalog; this spec gives every command in that catalog a host
that can reach it. The two are independent and either may land first.

## Goals

- A user installs Serial Studio, enables the API server, copies one configuration snippet from
  Preferences into Claude Desktop, Claude Code, Cursor or VS Code, and the host lists Serial
  Studio's tools. Nothing beyond the installed application is required: no interpreter, no
  script, no source checkout.
- An MCP session carries MCP messages and nothing else, on every transport.
- The default tool list fits comfortably in a model's context, and every other command is one
  discovery call away and callable from inside the same session.
- A host can see what each tool may do before calling it, and the server's answer to a call is
  the same answer the existing remote gate gives over TCP.
- A failed or refused tool call is visible to the model as a tool result it can reason about.
- Existing TCP MCP clients, the integration suite first among them, keep working.

## Non-Goals

- Replacing or changing the legacy JSON-RPC API, the gRPC server, the mirror stream or the
  typed stream-block subscription. They are other clients of the same server.
- Changing the in-app assistant's tool surface, tiers or meta tools. The assistant keeps its
  roster; this spec only stops the MCP surface from being a second, worse copy of the idea.
- Server-to-client requests of any kind: sampling, elicitation, roots. The server never asks
  the host for anything.
- OAuth or any new authentication scheme. The existing token for external connections is the
  only credential, and remote hosts outside the machine remain an external-connections
  decision the user makes today.
- A rolling frame history, a streaming resource at frame rate, or any new telemetry exit. The
  two frame resources stay as they are.
- New commands beyond the discovery tools. The window-driving commands are spec 0101's.
- Any change to which commands are Pro, licensed or trial-gated.
- A browser or WebAssembly viewer.

## Requirements

### A. Transports

1. **R1 — HTTP transport.** The API server offers the MCP Streamable HTTP transport at one
   endpoint URL. It shares the API server's enable switch, port, external-connections setting
   and token. With external connections off, the endpoint answers only on the loopback
   interfaces. A host that implements the transport completes initialize, tool listing, tool
   calls, resource reads and prompt reads over it. A host that never opens the server-to-client
   notification stream still has the full surface, because the server initiates nothing a tool
   call cannot do without.
2. **R2 — Child-process transport.** A host that can only launch a child process and talk over
   its standard streams launches something that ships with Serial Studio. That process connects
   to the running instance, forwards MCP messages in both directions and nothing else, exits
   when the host closes its input, and when the application is not running or the API server is
   off it exits non-zero within a few seconds with a plain message on its error stream saying
   what to enable. It never shows a window, never loads a project and never writes settings. It
   has exactly the trust of any other loopback client.
3. **R3 — TCP stays.** The raw TCP path keeps accepting MCP messages. Existing clients see
   additive changes only, with one exception: the error shape of R7, which is a correction
   toward the MCP specification. The integration suite passes with only those error tests
   rewritten.
4. **R4 — Session hygiene.** From the moment a connection sends its MCP initialize, no bytes
   that are not MCP messages are written to it: no per-frame broadcast, no mirror payload, no
   stream block. A plain TCP client on the same server keeps its broadcast. An MCP session that
   wants telemetry asks for it through a tool or resource.
5. **R5 — Browser isolation.** The HTTP endpoint refuses any request that carries a web-page
   origin, so a page open in the user's browser cannot drive the application through a local
   endpoint it can guess. With external connections on, every HTTP request presents the token.

### B. Tool surface

6. **R6 — Curated default roster.** The default tool list is a curated set of roughly the
   assistant's size that lets an agent do the manual's MCP use cases without discovery:
   connection control, driver configuration, reading telemetry, loading and editing a project,
   export, console, and the discovery tools of R7. Membership is one checked-in table, covered
   by a test that every name resolves to a registered command. A session can switch to the
   full catalog and back, and the host is told the list changed.
7. **R7 — Discovery tools.** Four tools, registered in the command catalog like every other so
   that TCP, gRPC and the assistant can use them too: list categories, search commands by name
   or words in their description, describe one command with its full schema and annotations,
   and run a command by name with arguments. The run-by-name tool carries the capabilities of
   the command it dispatches, so the remote gate, the sealed policy and the assistant's tiers
   see through it; it never widens what a session may do.
8. **R8 — Annotations.** Every tool carries a human title, a read-only hint and a destructive
   hint. A tool marked read-only changes no project, device or application state. A tool whose
   command sits in the assistant's always-confirm or blocked tier, or carries a device-write,
   program, install or project-load capability, is marked destructive. The hints are derived
   from the two classifications that already exist; there is no third hand-maintained table, and
   a registered command without an annotation fails a test.
9. **R9 — Failures are tool results.** A command that fails validation, is refused by a gate, or
   errors while running returns a successful JSON-RPC response whose tool result is flagged as an
   error and carries the message and the diagnostic hints. JSON-RPC errors are reserved for
   protocol faults: an uninitialized session, an unknown tool name, a missing name, malformed
   parameters.
10. **R10 — Protocol revision.** The server advertises the MCP revision that defines the
    Streamable HTTP transport and tool annotations, or a newer one it fully implements, and still
    accepts a client on the earlier revision the server speaks today. Resource subscription is
    either delivered (an update notification at most once per second, never per frame) or no
    longer advertised. The default is to stop advertising it.

### C. Discoverability

11. **R11 — Preferences shows the way in.** The API section of Preferences shows the MCP
    endpoint and offers copyable configurations for the common hosts (Claude Desktop, Claude
    Code, a generic stdio entry, a generic HTTP entry), generated from the current port,
    external-connections and token state. The token appears in a snippet only when external
    connections are on, and the dialog says so. The snippets stay correct when the port changes.
12. **R12 — The manual tells the truth.** The API reference's MCP section describes the two
    native transports and the TCP path, the curated roster and the discovery tools, the
    annotation meaning and the error shape. The examples README is rewritten around the native
    transports, the Python bridge is removed, and the example TCP client handles the new error
    shape. Terminology follows the existing one-name-per-concept rule.

### D. Security and sessions

13. **R13 — One gate for every transport.** A tool call over HTTP, over the child process or
    over TCP runs under the same remote dispatch scope: the per-project consent for device
    writes and program launches, the capability check, the sealed policy, and licensing's view
    of a remote origin. The headless-consent environment variable grants the gates for CI on
    every transport the way it does today on TCP.
14. **R14 — Sealed sessions.** In a sealed deployment session every transport answers exactly as
    TCP does: refused commands return the sealed-session error, and resource content is stripped.
    No transport adds an exit for project content or script source.
15. **R15 — Registration ritual.** Every discovery tool is classified in the capability tables,
    the sealed policy, the assistant's safety tiers and the API reference. The two tests that
    fail on an unclassified name are the backstop, not the process.

## Acceptance Criteria

- [ ] **AC1** — pytest (integration, a new HTTP client fixture beside the TCP one in the MCP
  suite): over HTTP, initialize, tools/list, tools/call, resources/read and prompts/get succeed
  and return the same results as the same requests over TCP; a call before initialize is refused
  the same way. (R1, R3)
- [ ] **AC2** — pytest (security tier): an HTTP request carrying a web-page origin is refused;
  with external connections on, a request without the token is refused and the same request
  with it succeeds; with external connections off, the endpoint is not reachable on a
  non-loopback address. (R5, R13)
- [ ] **AC3** — pytest (integration, binary under test named by the suite's environment):
  launching the shipped child-process transport with the application running yields a session
  whose initialize and tools/list succeed over its standard streams; closing its input ends the
  process; with the API server disabled it exits non-zero within five seconds with a message on
  its error stream. (R2)
- [ ] **AC4** — pytest (integration, with the suite's frame-producing fixture active): for five
  seconds after initialize, an MCP session over each transport receives only JSON-RPC messages,
  while a plain TCP session opened at the same time still receives frame broadcasts. (R4)
- [ ] **AC5** — pytest: the default tools/list is at most one fifth the size of the full catalog,
  contains the four discovery tools, and every name in it describes successfully; after the
  full-catalog switch the list contains every registered command and the host was notified; the
  run-by-name tool refuses a device write without consent with the same error the direct call
  gets. ctest: the roster table resolves every name against the registry. (R6, R7, R13)
- [ ] **AC6** — ctest: every registered command has annotations; no read-only tool carries a
  capability or sits outside the assistant's safe tier; every command in the always-confirm or
  blocked tier, or with a device-write, program, install or load capability, is marked
  destructive. (R8)
- [ ] **AC7** — pytest: a baud-rate call with a string argument returns a successful JSON-RPC
  response whose result is flagged as an error and names the parameter and the received type;
  an unknown tool, a missing name and an uninitialized session still return JSON-RPC errors.
  The existing error tests are rewritten to this shape and the example client handles it. (R9)
- [ ] **AC8** — pytest: initialize succeeds for a client on the earlier revision and for one on
  the advertised revision, and the capabilities no longer claim resource subscription (or, if the
  maintainer chooses delivery, a subscribed session receives no more than one update
  notification per second while frames flow). (R10)
- [ ] **AC9** — pytest (the existing sealed-session suite, extended): in a sealed session,
  tools/list over HTTP and over the child process, a refused tools/call, and a resource read
  answer identically to TCP, and the frame resource content is stripped. (R14)
- [ ] **AC10** — Maintainer observation, one desktop platform at minimum and all three before
  done: copying the Claude Desktop snippet from Preferences into the host's configuration and
  the Claude Code command into a terminal yields a host that lists Serial Studio's tools and
  answers "what is the connection status" by calling the status tool, with nothing installed
  beyond the application. Changing the port in Preferences changes the snippets. (R2, R11)
- [ ] **AC11** — Docs and ritual: the manual's MCP section and the examples README are rewritten,
  the Python bridge is gone, and the registry, terminology, claim and documentation verifiers
  pass; the capability-registry and sealed-policy ctests pass with the discovery tools
  classified; the assistant's static corpus test shows the discovery tools in a tier. (R12, R15)
- [ ] **AC12** — Regression: the MCP integration suite is green over TCP with only the R9 error
  tests changed; the non-destructive integration suite is green; `--benchmark-hotpath` is
  untouched because nothing here is on the hotpath; and in the performance tier, an idle MCP
  session held open for ten minutes with frames flowing leaves the process footprint flat. (R3,
  R4)

## Constraints & Invariants

- **The API server's auth and consent surface is decision-visible.** A new listener and a new
  transport are behavioral changes there. Each is named in chat with its failure modes before it
  lands and never rides inside a bulk or agent package.
- **Remote origin is a dispatch scope, never a per-site check.** Every transport enters the same
  scope TCP uses; the child-process transport is a network client of the running instance and
  therefore gets it for free, and a design that gave it a shortcut would be wrong.
- **Nothing on the hotpath.** MCP work is request-rate. No per-frame emission, allocation or
  lock is added for any transport; a resource update, if delivered at all, is at most tick-rate.
  The frame resource keeps reading the processed frame on demand.
- **One classification, derived twice.** Annotations and the curated roster derive from the
  capability tables and the safety tiers that exist. No third hand-maintained list of what a
  command may do; a drift between the sources is a test failure, not a doc note.
- **The run-by-name tool widens nothing.** It inherits the capabilities of its target for the
  remote gate, the sealed policy and the assistant's tiers, and it is classified as a dispatcher.
- **Sealed sessions leak nothing.** Every new transport and every new tool answers the seal as
  TCP does; stripped content stays stripped on every path.
- **No new third-party dependency.** A Qt module already present in the kit is a plan-level
  choice, with the licensing note in the open questions.
- **The child-process transport runs before the composition root.** It is a client, like the
  schema dump: it constructs no application module, touches no singleton, opens no window, and
  reads only what it needs to find the running instance.
- **Additive on the TCP wire.** Existing fields and messages keep their meaning; the only
  non-additive change is R9, which the maintainer confirms below.
- **Preferences strings go through the normal translation flow.** The translation files are
  the maintainer's working tree; this work adds source strings and does not regenerate them.
- **Landing shape.** One squashed commit, suite green, spec folder shared, the decision-visible
  change named in the commit body.

## Open Questions

- **Error shape (R9).** Following the MCP specification changes what existing TCP MCP clients
  see on a failed call. Recommendation: follow the specification. The clients we know of are the
  integration suite and the example client, both ours; hosts are the audience this spec exists
  for, and they already treat JSON-RPC errors as transport faults.
- **HTTP listener.** The installed Qt kit includes an HTTP server module, offered under the same
  GPL-or-commercial terms as the MQTT module the application already links, so it introduces no
  new category of licensing obligation; the alternative is a minimal HTTP handling of the MCP
  endpoint on the existing listener and port. Recommendation: same port, same switch, same
  firewall rule, with the plan choosing the smaller of the two implementations after measuring
  it; confirm that the licensing reading is acceptable.
- **Where the child-process transport lives.** A mode of the application binary (one artifact to
  sign, notarize and certify) or a small companion executable. Recommendation: a mode of the
  binary. Open: on the Windows Store install the binary is not on a path a host can name, so an
  app execution alias may be needed; the plan verifies on an MSIX install.
- **Roster membership.** The assistant's roster is editor-centric (its file tools and workflow
  helpers). An external host's audience leans toward operating the application. Recommendation:
  the MCP roster has its own membership, maintained through the same mechanism and test as the
  assistant's, and the two lists are reviewed together when either changes.
- **Full-catalog switch.** A session-scoped tool followed by a list-changed notification works
  on all three transports; a URL parameter would cover HTTP only. Recommendation: the tool.
- **Resource subscription.** Deliver at tick-rate or stop advertising. Recommendation: stop
  advertising; a host that wants to watch a value polls a tool, and the capability returns if a
  host asks for it.
- **The example client.** Keep the TCP example client as a TCP example and delete the bridge, or
  delete both. Recommendation: keep the client, delete the bridge.
