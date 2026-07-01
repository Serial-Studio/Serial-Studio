---
spec: 0001-offline-activation
title: App-side offline activation (signed license certificate)
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-07-01
author: Alex Spataru
---

# Spec 0001 — App-side offline activation (signed license certificate)

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio activates commercial licenses **online only**. A machine unlocks Pro by reaching
the LemonSqueezy API over the network; once activated it can run offline for a bounded grace
period on the cached token, but the *initial* activation — and every re-validation after the
grace period lapses — requires internet. Industrial, defense, lab, and OT customers routinely
run Serial Studio on **air-gapped machines** that will never be allowed to reach an external
host, and on locked-down networks where outbound calls to `api.lemonsqueezy.com` are blocked by
policy. These customers have paid for Pro and cannot turn it on. Today the only workaround is to
temporarily bridge a regulated machine to the internet, which many of them are contractually
forbidden from doing.

This feature gives such customers a way to activate Pro on a machine that never touches the
network: they obtain a signed license certificate on a *separate* internet-connected device,
carry it across the air gap on removable media, and import it into the offline machine, which
verifies it locally with no network call. It is a **second, parallel trust path** alongside the
existing online activation and trial flows — not a replacement for them.

## Goals

- A user can fully activate Pro on a machine that never makes a network request, by importing a
  license certificate file.
- The offline machine can display its own hardware fingerprint so the user can request a
  certificate bound to that machine.
- An imported certificate is trusted only if it was issued by Serial Studio (cryptographic
  signature), was issued for *this* machine, and has not expired.
- Once imported, an offline-activated machine behaves identically to an online-activated one:
  the same Pro features unlock, through the same token machinery.
- The user is shown how long an offline certificate remains valid and is warned before it
  lapses, so they can obtain a fresh one ahead of time.
- Maintainers can mint and test certificates before the issuing web portal exists.

## Non-Goals

- **Building the issuing web portal or its signing service.** The self-service portal that
  validates a LemonSqueezy license, consumes a seat, and returns a certificate lives in a
  separate project (`cloud.serial-studio.com`). This spec defines the certificate format and the
  request/response contract that portal must honor, and stops there.
- **Online seat management from the offline machine.** An air-gapped machine cannot consume or
  release a seat. Seat accounting and deactivation happen in the portal against LemonSqueezy;
  this feature only *presents* seat/customer facts that the certificate carries.
- **Changing, weakening, or bypassing the existing online activation, trial, HMAC token seal,
  or license-guard logic.** The offline path reuses that machinery unchanged and must not alter
  how already-activated builds behave.
- **A new licensing backend or a move off LemonSqueezy.** LemonSqueezy remains the system of
  record for who owns a license; the certificate is a signed derivative of an existing entitlement.
- **Non-expiring certificates.** *Every* certificate carries an expiry the app enforces — even
  for the perpetual Lifetime tier, where the expiry is a portal-set re-issue window rather than
  the entitlement end. A certificate that never expires is explicitly out of scope; the app never
  trusts an offline certificate indefinitely.
- **Paste-in / clipboard import.** First release is file import only (`.sslic`); a paste-a-text-
  block entry point is deliberately deferred.
- **Offline *trials*.** The trial flow stays online-only.

## Requirements

Numbered, testable statements of user-facing (and contract) behavior.

1. **R1 — Offline activation is a guided in-app flow.** In the license management UI, a commercial
   build offers an "Activate Offline" action that opens a stepped wizard: (1) **export** a device
   file, (2) open the activation portal to exchange it for a license file, (3) **import** the
   license file. The whole flow is in-app; the end user never runs a command-line tool. Importing
   a valid certificate activates Pro without any network request on this machine.

2. **R2 — Device file export.** The wizard writes a machine-identifying **device file** the user
   uploads to the portal (along with their email + license key) to obtain a certificate. The file
   carries at least the machine fingerprint the certificate will be bound to, plus app/version
   metadata, and contains no personal information. (The fingerprint also remains visible in the UI.)

3. **R3 — Signature is required.** A certificate is accepted only if its signature verifies
   against the Serial Studio public key embedded in the application. A certificate with a
   missing, malformed, or invalid signature is rejected with a clear reason and does not activate
   anything.

4. **R4 — Machine binding is enforced.** A certificate is accepted only if the machine identity
   it names matches the importing machine. A certificate minted for a different machine is
   rejected with a clear reason, even if its signature is valid.

5. **R5 — Expiry is enforced.** A certificate is accepted only if it has not expired as of the
   current effective time, where "current effective time" is floored by the same monotonic,
   clock-rewind-resistant clock the online grace period already uses. An expired certificate is
   rejected with a clear reason; a certificate cannot be revived by moving the system clock
   backward.

6. **R6 — Successful import yields a normal commercial session.** After a valid certificate is
   imported, the running application reports the same activation state, feature tier, and unlocked
   Pro capabilities it would after an equivalent online activation. No feature-gating code needs
   to distinguish the two paths.

7. **R7 — Offline activation persists across restarts.** An imported certificate re-activates the
   application on subsequent launches without re-importing and without a network call, until it
   expires or is removed.

8. **R8 — Validity is surfaced to the user.** The UI shows the certificate's remaining validity
   (e.g. days remaining / expiry date) and warns the user as expiry approaches, directing them to
   obtain a fresh certificate.

9. **R9 — Certificate format is a defined contract.** The certificate is a compact, signed,
   machine-bound artifact carrying at least: the bound machine identity, the feature tier /
   variant, the LemonSqueezy instance identifier (for seat correlation by the portal), the issue
   time, and the expiry time. The signed payload is serialized as **CBOR**, Ed25519-signed, and
   framed as base64 text in a `.sslic` **file**. The encoding (serialization, signature layout,
   text framing, and file extension) is specified precisely enough that an independent issuer
   could produce a certificate this application accepts. The tier field's accepted launch values
   are **Lifetime** and **Enterprise (yearly)**; the app must accept whichever the certificate
   names and map it to the corresponding feature tier.

10. **R10 — API parity.** The offline activation action is invokable through the licensing API
    surface (alongside the existing activate / validate / deactivate / status commands), so it can
    be scripted and tested headlessly.

11. **R11 — Maintainer signing tool.** A maintainer-run tool can mint a valid certificate for a
    given machine fingerprint, tier, instance, and expiry, so certificates can be generated and
    tested before the portal exists. This tool holds the private key at run time from an external
    secret source; it is a development/operations tool, not part of the shipped application.

12. **R12 — Clear rejection reasons.** Every rejection path (bad signature, wrong machine,
    expired, malformed/garbage input, wrong tier) produces a distinct, user-comprehensible reason
    rather than a generic failure, and never partially activates.

## Acceptance Criteria

Each ties to a check that can actually run. The verifier and certificate parser are pure logic
and should be exercisable without a live app; end-to-end activation is a maintainer observation
in the running commercial build.

- [ ] **AC1 (R3, R9, R11)** — Given a certificate minted by the signing tool with a known good
  key, the parser/verifier accepts it; flipping any byte of the payload or signature causes
  rejection with an "invalid signature" reason. Runnable as a headless unit test over the
  verifier (no Qt UI, no network).
- [ ] **AC2 (R4)** — A certificate minted for machine fingerprint A is rejected on a machine
  reporting fingerprint B with a "machine mismatch" reason, and accepted on a machine reporting
  fingerprint A. Verifiable in the verifier unit test by injecting the fingerprint.
- [ ] **AC3 (R5)** — A certificate whose expiry is in the past is rejected as "expired"; a
  certificate valid today is accepted; setting the effective clock backward below a previously
  observed time does not turn an expired certificate valid again. Verifiable in the verifier unit
  test by injecting the effective time.
- [ ] **AC4 (R1, R6, R7)** — In a running commercial build with no network access, importing a
  valid certificate unlocks the same Pro widgets/features as online activation, the activation
  survives an app restart, and no outbound licensing request is made (observed via the API server
  / logs, maintainer check).
- [ ] **AC5 (R2)** — The license management UI shows a machine fingerprint that matches the value
  the signing tool must be given to mint an accepted certificate (maintainer check: copy from UI,
  mint against it, import succeeds).
- [ ] **AC6 (R8)** — With an imported certificate near expiry, the UI shows remaining validity and
  an approaching-expiry warning; with an expired certificate, activation does not persist and the
  UI reflects the lapsed state (maintainer check).
- [ ] **AC7 (R10)** — The offline activation command is exercisable through the licensing API and
  returns success for a valid certificate and a distinct failure reason for each rejection class
  (headless API test against a running app, maintainer-run).
- [ ] **AC8 (R12)** — Feeding malformed / truncated / non-certificate input to the import path
  yields a clear rejection and leaves activation state unchanged (verifier unit test + UI check).

## Constraints & Invariants

- **The private signing key must never enter this repository, CI, or the shipped binary.** Only
  the Ed25519 **public** key ships. This is a hard security invariant: a leaked private key lets
  anyone forge Pro activations. The signing tool obtains the key from an external secret at run
  time.
- **Asymmetric root of trust.** Offline activation is authorized by an asymmetric signature
  (public key in the app, private key held only by the issuer). The existing symmetric HMAC token
  seal is *not* sufficient as the offline root of trust and must not be repurposed as one — its
  secret is in the binary and could be extracted to forge certificates.
- **Reuse, do not fork, the token machinery.** The offline path must terminate in the same
  commercial-token seal + license-guard machinery as online activation, so downstream feature
  gates need no awareness of which path activated. Existing online/trial/guard behavior must be
  unchanged.
- **No new third-party dependency.** OpenSSL is already vendored and is the intended provider of
  Ed25519 verification; do not introduce another crypto library.
- **Pro / commercial only.** The entire feature is gated to commercial builds and has no effect
  in the GPL build.
- **Clock-rewind resistance is mandatory.** Expiry enforcement must use the existing monotonic
  "highest observed time" flooring so a user cannot extend or revive a certificate by rolling the
  system clock back.
- **Every certificate expires, including Lifetime.** The app applies the same `expiresAt` check to
  all certificates regardless of tier. Whether that date represents a subscription end (Enterprise
  yearly) or a re-issue window (Lifetime) is a portal concern the app does not distinguish.
- **Must not regress the hotpath.** Licensing runs at startup / on user action, off the data
  path; the feature must add no work to the 256 kHz frame hotpath.
- **Offline machine makes zero network calls for activation.** The whole point; verification is
  purely local.

## Open Questions

*Resolved (2026-07-01):*

- ~~**Serialization encoding.**~~ **CBOR** for the signed payload (R9). Compactness over
  human-inspectability; parsed via Qt's built-in CBOR support.
- ~~**Paste-in vs file import.**~~ **File only** (`.sslic`) for the first release (R1); paste-in
  is a deferred non-goal.
- ~~**Tier coverage.**~~ Launch tiers are **Lifetime** and **Enterprise (yearly)** (R9). Every
  certificate still carries an app-enforced expiry, including Lifetime (portal-set re-issue window).

*Still open:*

- **Certificate ↔ portal request contract ownership.** This spec defines the certificate the app
  consumes; the exact portal request shape (what the user submits: LemonSqueezy key + fingerprint)
  is co-owned with the separate portal project. The app is the source of truth for the fingerprint
  format the portal echoes back into the certificate — confirm this is captured when the portal
  contract is drafted. Does not block `/ss-plan`.
