---
spec: 0067-opcua-embedded-stack
title: Embedded OPC UA Stack and Secure Channels
status: done          # closed 2026-08-25
created: 2026-08-24
author: Alex Spataru
---

# Spec 0067 — Embedded OPC UA Stack and Secure Channels

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The OPC UA driver shipped by spec 0066 does not own its protocol stack. It talks to servers
through a Qt module whose actual OPC UA implementation lives in a runtime plugin that Qt
builds, and whose availability and capabilities differ per Qt package. Two consequences are
already biting.

**The driver silently does not exist on some supported builds.** The plugin is included in the
open-source Qt packages but is absent from the commercial Qt build Serial Studio Pro is
compiled against on Windows, so a Pro user on Windows gets an OPC UA source that cannot open
a session at all. Where the plugin *is* present it is not self-contained either: on Windows it
resolves its cryptography from an OpenSSL library distributed as a separate optional Qt
component rather than with the Qt runtime libraries. If that library is missing at load time
the plugin fails to initialize and the driver reports only that the backend is unavailable.
Whether the plugin is missing or merely unloadable, the user-visible symptom is identical and
undiagnosable from inside the app, and nothing Serial Studio ships can fix it.

**Every OPC UA session is plaintext.** Because the capabilities of the plugin are decided by
whoever built Qt, spec 0066 had to restrict the driver to security policy None and grey out
every secure endpoint a server advertises. Username and password therefore cross the wire
unprotected, which spec 0066 mitigates only with a warning banner. Plant networks routinely
refuse anonymous plaintext sessions outright, and a large share of real Siemens, Beckhoff and
Kepware deployments advertise *no* None endpoint at all, so Serial Studio cannot connect to
them on any platform, however the plugin was built. Spec 0066 named this explicitly as deferred:
"a secure channel needs a toolchain change first; it is a follow-up spec, not this one." This
is that spec.

The deciding constraint is that the protocol stack and its cryptography must be compiled into
the Serial Studio binary rather than resolved from the environment at run time. That single
change removes the per-package variation, makes the driver's presence a property of our build
instead of Qt's, and puts the set of supported security policies under our control.

## Goals

- The OPC UA driver is present and functional on every platform and build configuration
  Serial Studio Pro ships, with no dependency on a Qt OPC UA module, a runtime protocol
  plugin, or any separately deployed cryptography library.
- A user can open an encrypted, signed OPC UA session against servers that offer no
  plaintext endpoint, which today are unreachable from Serial Studio entirely.
- The endpoint list stops being mostly disabled: a user picks any policy and mode their
  server advertises and sees why an endpoint is unsuitable only when it genuinely is.
- Serial Studio presents a client certificate that servers can be configured to trust, and
  the user can obtain, inspect and replace it without leaving the app.
- An unknown server certificate is a decision the user makes once, with enough information
  to make it, and it is remembered.
- Username and password can be sent over a channel that protects them, retiring the
  permanent "credentials travel unencrypted" warning for secure sessions.
- A user whose server requires a certificate as the identity token can authenticate.
- Everything spec 0066 delivers keeps working unchanged: browsing, tag selection, project
  generation, subscriptions, the poll fallback, source timestamps, and the simulator-backed
  test suite.

## Non-Goals

- OPC UA **server** mode, **PubSub**, **historical access**, **methods**, **alarms and
  conditions**, **events**, and **redundancy / failover**. Spec 0066's non-goals stand.
- **Writing** tag values from the dashboard.
- Non-binary transports (HTTPS, WebSocket).
- Complex / structured (extension-object) tag values, and arrays beyond one dimension.
- Acting as a certificate authority: Serial Studio issues only its own self-signed client
  certificate and never signs anything for a third party.
- **Global Discovery Server (GDS)** enrolment, certificate lifecycle management, automatic
  renewal, and CRL / OCSP revocation checking.
- Any change to the acquisition pipeline, frame reader, or frame builder.
- Changing which datasets, widgets or project shapes the driver generates. The wire format
  between the driver and the parser is untouched.
- Replacing or re-implementing any other driver's transport. This spec concerns OPC UA only.

## Requirements

The requirements are ordered so that R1–R6 form an independently shippable and verifiable
stage: at that point the driver has changed its protocol stack with **no** user-facing change
in capability. R7 onward add the secure channel on top of it.

### Stage 1 — own the stack, no behavior change

1. **R1** — Serial Studio's OPC UA support is built from a protocol stack compiled into the
   application. A Pro build has a working OPC UA source on every shipped platform without
   any OPC UA-specific plugin, module or library being present on the target machine.
2. **R2** — The application ships no new runtime library files for OPC UA, and removing every
   OPC UA-related file from a Qt installation does not affect a shipped build.
3. **R3** — Everything spec 0066 requires (its R1 through R14) continues to hold, verified by
   the spec 0066 test suite passing unchanged except where a later requirement here
   changes the expected behavior.
4. **R4** — A build configured without network access, or by a distribution packager who
   requires system libraries, can still produce a working application: either by consuming a
   system-provided OPC UA stack or by building with OPC UA support absent, and in the latter
   case the OPC UA source is hidden exactly as it is in a GPL build rather than appearing and
   failing.
5. **R5** — The connection failure the user sees when a session cannot be opened always names
   a condition on the network, the server, or the configuration. "The backend is not
   available in this build" is no longer reachable in a shipped Pro build.
6. **R6** — Data throughput, session establishment time and idle CPU cost of an OPC UA session
   are no worse than the current implementation under the same tag count and publishing
   interval.

### Stage 2 — secure channels

7. **R7** — The driver supports every security policy the embedded stack implements, in both
   `Sign` and `SignAndEncrypt` modes: `Basic256Sha256`, `Aes128_Sha256_RsaOaep`,
   `Aes256_Sha256_RsaPss`, and the deprecated `Basic128Rsa15` and `Basic256`.
8. **R8** — Endpoints using a policy that is deprecated or cryptographically weak are shown
   with a clear deprecation label and are not chosen by automatic endpoint selection, but the
   user can still select them for an old server.
9. **R9** — When several endpoints are advertised, the endpoint selected by default is the
   most secure one the user's chosen authentication mode can use. A user who wants a
   plaintext session can still pick it explicitly.
10. **R10** — Serial Studio holds a client certificate and private key it presents when opening
    a secure channel. If none exists, one is generated on first use without the user being
    asked anything, with a subject and an application URI that identify the installation.
11. **R11** — The user can view the client certificate's subject, fingerprint, and validity
    dates, export the certificate for loading into a server's trust list, and replace both
    certificate and key with their own.
12. **R12** — Connecting to a server whose certificate is not yet trusted presents the
    certificate's subject, issuer, fingerprint and validity window, and asks the user to trust
    or reject it. The session proceeds only on trust.
13. **R13** — A trust decision is remembered across restarts, and the user can review and
    revoke previously trusted server certificates.
14. **R14** — A server certificate that is expired, not yet valid, or whose hostname does not
    match the endpoint is reported as such in the trust prompt, distinctly from merely being
    unknown.
15. **R15** — Username and password authentication over a `SignAndEncrypt` channel does not
    show the plaintext-credentials warning spec 0066 R4 requires. The warning still appears on
    a `None` or `Sign`-only channel.
16. **R16** — The user can authenticate with an X.509 certificate as the identity token,
    supplying their own certificate and key, independently of the certificate used for the
    channel itself.
17. **R17** — Every distinct security failure (untrusted or rejected certificate, policy the
    server will not accept, a certificate the server rejected, bad identity token) produces a
    distinct, human-readable reason, reported exactly once per connection attempt, and leaves
    the connection state settled rather than stuck.
18. **R18** — The selected policy, mode, identity token type and certificate choices persist
    with the rest of the driver's configuration and survive a project save and reopen. Private
    keys and passwords never enter the project file.

## Acceptance Criteria

- [x] **AC1** — The spec 0066 integration suite passes unchanged against a build with the
      embedded stack, on every CI leg that runs the Pro tier. (R1, R3)
- [x] **AC2** — On a machine with no Qt OPC UA module and no system OpenSSL, a shipped Pro
      build connects to the example simulator and streams tags. Verified on Windows, where the
      driver is currently non-functional. (R1, R2)
- [x] **AC3** — Inspecting a shipped package shows no OPC UA protocol plugin and no
      cryptography library that was not already shipped before this spec. (R2)
- [x] **AC4** — A build configured for the no-network / packager case produces an application
      that either connects over OPC UA or does not offer OPC UA at all; it never offers a
      source that fails to open. (R4)
- [x] **AC5** — A throughput and connect-time comparison against the pre-change build, at
      equal tag count and publishing interval, shows no regression outside run-to-run noise.
      (R6)
- [x] **AC6** — The example simulator gains secure endpoints, and an integration test connects
      over each supported policy in both modes, asserting the negotiated policy and mode and
      that tags stream. Deprecated policies are covered by the same test. (R7)
- [x] **AC7** — An integration test points the driver at a server offering *only* secure
      endpoints and asserts a successful session, the case that is impossible today. (R7, R9)
- [x] **AC8** — With several endpoints advertised, a test asserts which one is selected by
      default and that a deprecated policy is never selected automatically. (R8, R9)
- [x] **AC9** — A test asserts a client certificate and key are generated on first secure use,
      that a second connection reuses them rather than regenerating, and that the exported
      certificate is loadable by the simulator as a trusted peer. (R10, R11)
- [x] **AC10** — A test connecting to a server with an unknown certificate asserts the session
      does not proceed until trust is granted, that the prompt carries subject, issuer,
      fingerprint and validity, and that after trusting, a subsequent connection does not
      prompt again. (R12, R13)
- [x] **AC11** — Tests using an expired certificate and a hostname-mismatched
      certificate assert each is reported with its own distinct reason. (R14)
- [x] **AC12** — A test asserts the plaintext-credential warning is absent on an encrypted
      session and present on a `None` and on a `Sign`-only session. (R15)
- [x] **AC13** — A test authenticates against a simulator configured to require an X.509
      identity token, and asserts a wrong or missing user certificate fails with its own
      reason. (R16, R17)
- [x] **AC14** — For each security failure mode, a test asserts a distinct non-empty reason is
      reported exactly once and the connection state settles, matching the one-verdict contract
      spec 0066 R5 established. (R17)
- [x] **AC15** — A project configured with a secure endpoint, a policy, a mode and an identity
      token round-trips through save and reopen with all four preserved, and the saved file
      contains no private key and no password. (R18)
- [x] **AC16** — The in-app help manual documents secure connection setup, client-certificate
      export, and server-certificate trust. (R11, R12)

## Constraints & Invariants

- Pro-only. Everything here stays behind the commercial build gate; a GPL build is unchanged
  and gains no new dependency.
- The cryptography must be linked into the application. A build must not acquire a new
  runtime dependency on a cryptography library resolved from the target machine. That
  fragility is the reason this spec exists.
- The OPC UA stack must carry its own cryptography, independent of whichever TLS library the
  rest of the build links. `ENABLE_GRPC` swaps the application's TLS between the BoringSSL
  bundled with gRPC and the static OpenSSL in `lib/OpenSSL`, and every shipping configuration
  sets it. OPC UA must work identically in both, and must add no runtime dependency resolved
  from the target machine.
- Every third-party component added must be license-compatible with both distribution terms
  the project ships under, and must be declared so the repository stays REUSE-compliant.
- The acquisition pipeline is untouched: this spec changes how bytes are obtained from a
  server, never how frames are published, timestamped or consumed. The hotpath throughput
  gate must not move.
- The one-verdict connection contract holds. Opening reports success or failure exactly once
  per attempt and never leaves the connection stuck in a connecting state. The added
  handshake steps are new ways to fail, not new ways to hang.
- Source timestamps continue to come from the server, unchanged by the transport.
- Private keys and credentials never enter a project file, and never enter a log or an error
  message.
- Secure-session setup must not block the GUI thread; establishing a channel is slower than a
  plaintext one and must stay asynchronous.
- The example simulator remains the test fixture and must remain runnable by a developer with
  only the documented Python dependencies.
- No regression to any other driver. The build change must not alter how any non-OPC UA
  source is compiled or linked.

## Open Questions

- Should a secure session be the default for a *new* OPC UA source, or should the default stay
  whatever the server advertises as most secure only once the user has a certificate? Bears on
  whether a first-time user meets a trust prompt before they have connected to anything.
- When the user replaces the client certificate with their own, is a key with a passphrase
  required to be supported, and if so where is the passphrase held?
- Should the trust store be per-installation or per-project? Per-installation matches operator
  expectations; per-project would make a shared project file carry its trust assumptions.
- Does a packager-facing build that consumes a system OPC UA stack need to support the same
  policy set, or is it acceptable for its capabilities to follow the system library's build?
- Should the deprecated policies be available in a shipped build at all, or only behind a
  setting the user must opt into? R7/R8 currently require them present but never auto-selected.
