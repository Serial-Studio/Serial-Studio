---
spec: 0041-mqtt-mutual-tls
title: MQTT mutual TLS (client certificates) for AWS IoT Core
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-07-27
author: Alex Spataru
---

# Spec 0041 — MQTT mutual TLS (client certificates) for AWS IoT Core

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio speaks MQTT on two surfaces — the subscriber data source (telemetry in) and
the publisher (telemetry out) — and both speak TLS today only in the one-way direction: the
client verifies the broker (CA certificates, protocol version, peer verification mode) and
authenticates itself with a username/password at most. Brokers that require **mutual TLS**
— where the client must present its own X.509 certificate and prove possession of the
matching private key during the handshake — cannot be used at all. The dominant such broker
is AWS IoT Core, whose standard device workflow issues every device a certificate/key pair
and rejects plain or password-authenticated connections on its MQTT endpoint. A user asked
for AWS IoT support directly; Azure IoT Hub and most corporate PKI-fronted brokers have the
same requirement, so the gap locks Serial Studio out of the mainstream cloud IoT ingestion
path in both directions: it can neither subscribe to device telemetry arriving at AWS IoT
nor publish its own frames there. Both MQTT surfaces are in scope (maintainer-confirmed
2026-07-27).

A second, related gap: AWS IoT (and some corporate networks) offer MQTT on port 443
distinguished by an ALPN protocol name, for devices behind firewalls that only allow
HTTPS-looking traffic. Without a way to request an ALPN protocol during the handshake,
port-443 MQTT endpoints are unreachable even with valid credentials.

## Goals

- A user can connect either MQTT surface (subscriber data source or publisher) to an AWS
  IoT Core endpoint on port 8883 using the
  certificate, private key, and root CA files downloaded from the AWS console, with no
  external tooling or file conversion.
- A user can alternatively connect to the same endpoint on port 443 by enabling ALPN, for
  networks where 8883 is blocked.
- A user with a passphrase-protected private key (corporate PKI) can supply the passphrase
  once and have it remembered as securely as the existing MQTT username/password.
- Misconfiguration (missing file, unreadable PEM, wrong passphrase, key/cert mismatch)
  produces a clear, actionable error before or at connection time — never a silent hang or
  a generic socket error.

## Non-Goals

- No AWS-specific provisioning, discovery, or SDK integration: no fleet provisioning, no
  Cognito/SigV4 WebSocket authentication, no endpoint lookup. The user pastes the endpoint
  hostname and picks their files, exactly as with any other broker.
- No certificate management: Serial Studio does not generate, renew, inspect, or store
  certificates beyond remembering which files the user chose.
- No changes to MQTT subscribe/receive behavior, publisher modes, topic handling, QoS, or
  retain semantics. AWS-side constraints (QoS 2 unsupported, retained messages off by
  default) remain the user's responsibility; documentation may note them.
- No mutual-TLS support for non-MQTT transports (TCP driver, API server) — both MQTT
  surfaces only.

## Requirements

1. **R1** — Both MQTT setup UIs (data-source pane and publisher settings) offer, when TLS
   is enabled, an optional client certificate file and an optional private key file (PEM;
   the key may live in the same file as the certificate when the user selects one combined
   PEM for both). Both empty keeps today's server-verified-only behavior unchanged.
2. **R2** — With a valid certificate/key pair configured, the TLS handshake presents them
   to the broker, and a connection to an AWS IoT Core endpoint configured with a matching
   thing certificate and permissive policy succeeds on port 8883 — subscribing on one
   surface, publishing on the other.
3. **R3** — An optional private-key passphrase can be entered in the setup UI; it is stored
   and restored with the same security guarantees as the existing MQTT username/password
   (the machine-keyed encrypted credential vault), never in plaintext settings or project
   files.
4. **R4** — An ALPN toggle (with the AWS protocol preset available, editable for other
   brokers) can be enabled when TLS is on; with it, the same AWS IoT endpoint is reachable
   on port 443.
5. **R5** — Selected certificate/key file paths and the ALPN setting persist across
   application restarts alongside the other MQTT settings; the passphrase persists per R3.
6. **R6** — Configuration errors are surfaced to the user with a message naming the failing
   piece: file missing/unreadable, not valid PEM, passphrase wrong or required. A failed
   mutual-TLS handshake reports the TLS error rather than a generic connection failure,
   naming client-certificate authentication as the likely cause when a client identity is
   configured (a certificate/key mismatch is only detectable at the handshake).
7. **R7** — Client-certificate fields are visible but inert while TLS is disabled, and
   their values do not affect non-TLS connections.

## Acceptance Criteria

- [ ] **AC1** — Maintainer observation: against a real AWS IoT Core thing (cert + key +
      Amazon Root CA 1 from the console), the publisher connects on port 8883 and published
      frames appear in the AWS IoT MQTT test client. (No CI path — needs live AWS
      credentials.)
- [ ] **AC2** — Maintainer observation: same endpoint, ALPN enabled, port 443 — connection
      and publish succeed.
- [ ] **AC3** — Local mutual-TLS broker check (e.g. mosquitto with `require_certificate
      true`): connection succeeds with a matching pair, is refused without a client
      certificate, and the refusal is reported as a TLS/authentication error in the UI.
- [ ] **AC4** — Restart round-trip: configure cert, key, passphrase, ALPN; quit; relaunch;
      all fields restore and a reconnect succeeds without re-entry. Passphrase is absent
      from every settings/config file on disk.
- [ ] **AC5** — Negative-path sweep: nonexistent file path, a non-PEM file, and an
      encrypted key with wrong passphrase each produce a distinct, user-readable error at
      selection/load time; a key that does not match the certificate produces a handshake
      error naming client-certificate authentication. No crash or hang in any case.
- [ ] **AC6** — With both new fields left empty, existing TLS and non-TLS MQTT connections
      behave byte-for-byte as before (regression: current MQTT integration tests still
      pass).

## Constraints & Invariants

- **Secrets never touch disk in plaintext.** Passphrase storage must reuse the existing
  machine-keyed encrypted credential vault; certificate/key *paths* may persist in
  settings/project files, file *contents* and passphrases may not.
- **Broker I/O stays off the main thread.** Credentials must be fully resolved into the
  connection configuration before the handshake starts; no mid-handshake callbacks into UI
  or main-thread state.
- **No new dependencies.** Qt's existing TLS stack must carry the feature.
- **Zero hotpath impact.** All new work happens at connection setup; nothing per frame or
  per publish changes. MQTT is an export-side consumer and must stay one.
- **Backward compatibility.** Existing saved MQTT configurations load unchanged; absent new
  keys mean the feature is off.
- **Cross-platform.** Must work on Windows, macOS, Linux with the same UI and file formats
  (PEM everywhere; DER out of scope).

## Open Questions

- None — ALPN inclusion and passphrase-via-keychain were confirmed by the maintainer
  (2026-07-27). AC1/AC2 need the maintainer's AWS account; if none is available at
  verification time, AC3 plus a documented manual AWS run by a user stands in.
