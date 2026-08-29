---
spec: 0042-license-token-hardening
title: CommercialToken lifecycle hardening (licensing first, no silent gaps)
status: done          # closed 2026-08-20
created: 2026-07-28
author: Alex Spataru
---

# Spec 0042 — CommercialToken lifecycle hardening

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The commercial-entitlement token has produced a recurring class of shipped bugs: a consumer
samples the token at the wrong moment, or a licensing path fails to (re)install it, and a
Pro feature silently degrades or dies with no message. Documented hits: fallback widgets
baked into workspaces on late activation (2026-07-09), the license-gated device rebuild gap
(2026-07), and today: a machine with a stored license key opened the bundled MQTT Subscriber
example and got a permanently dead Connect button. Root cause of today's hit, confirmed by
instrumentation: when the cached license restore is rejected at startup (grace expired,
machine-id drift, decrypt failure), the in-memory license data is cleared -- and the startup
online revalidation is gated on that same in-memory data being present. Exactly the case
that needs a live server verdict is the case that never asks for one; the machine stays
unlicensed all session even though the stored key is fine and the network is up.

The deeper pattern: token installation is spread across three sources (online license,
offline certificate, trial), consumers must each know when to re-sample, and initialization
order relative to the rest of the app is implicit. Each new consumer or licensing tweak
re-exposes the same class. The maintainer's directive: licensing initializes first, and
this class of error gets closed structurally, not patched per-symptom.

## Goals

- A machine with a stored, still-valid license key always ends up entitled after startup
  whenever the activation server is reachable -- regardless of whether the cached restore
  succeeded, its grace state, or what was loaded first.
- Every entitlement source (online, offline certificate, trial) is fully constructed and
  has had its chance to install the token before any other subsystem that consumes
  entitlement is constructed or restores state.
- Any mid-session entitlement change (activation, revocation, trial expiry, offline
  import/removal) reaches every consumer that bakes entitlement into derived state; no
  consumer can be "forgotten" without it being visible in one audited list.
- A project whose data source requires an entitlement the session does not have tells the
  user so, instead of presenting a dead control.

## Non-Goals

- No change to what is entitled: feature tiers, trial semantics, grace-period length, seat
  logic, and the store backend stay exactly as they are.
- No new licensing UI beyond the one "source requires Pro" surfacing.
- No cryptographic or anti-tamper changes to the token itself.
- No removal of the token self-test.

## Requirements

1. **R1** -- At startup, if a license key is stored, an online revalidation attempt is made
   even when the cached restore failed or was rejected. A reachable server verdict then
   either restores entitlement (and every gated consumer recovers automatically) or
   definitively clears it.
2. **R2** -- The licensing subsystem is initialized before every entitlement-consuming
   subsystem, as the first block of application construction; the pinned-order proof is
   re-run for the new order.
3. **R3** -- One audited inventory exists of every token consumer, classifying each as
   sample-per-operation (safe by construction) or bakes-derived-state (must be wired to the
   entitlement-change notification); every consumer in the second class is wired. The
   notification fires for all three sources' transitions.
4. **R4** -- Loading a project whose source bus requires an entitlement the session lacks
   surfaces a visible, actionable problem ("requires Serial Studio Pro or an active trial")
   instead of a silently disabled Connect; the finding clears when entitlement arrives.
5. **R5** -- The temporary [mqtt-debug] instrumentation added during diagnosis is removed.

## Acceptance Criteria

- [x] **AC1** -- Repro of today's bug: with a stored license whose cached restore fails
      (e.g. force grace to 0), launch with network up: entitlement restores within the
      validation round-trip and the MQTT Subscriber example's Connect button enables
      without user action. (Maintainer observation.)
- [x] **AC2** -- Same setup, network down: Connect stays disabled but the problem surface
      names the licensing cause (R4); no crash, no silent dead-end.
- [x] **AC3** -- Consumer inventory table exists in the spec directory with every
      `CommercialToken` consumer classified and, where required, wired; re-running the
      grep that generated it shows no unclassified TU.
- [x] **AC4** -- Ctor-order proof re-run and recorded for the new initialization order;
      app boots identically in GUI, headless, and CLI modes. (Maintainer builds/runs.)
- [x] **AC5** -- Mid-session transitions still recover: activating a license while a
      Pro-bus project is open enables Connect without restart (existing behavior kept).
- [x] **AC6** -- No [mqtt-debug] strings remain in the tree.

## Constraints & Invariants

- **Composition-root order is a protected surface**: any reorder re-runs the ctor-edge
  proof (spec 0001); licensing ctors must provably not reach any later-constructed module.
- **Never weaken the gate**: an invalid token must still deny commercial features; the fix
  is availability of the *valid* state, not permissiveness.
- **No per-frame cost**: all changes live at startup/config boundaries.
- **Licensing network behavior unchanged** except the one added startup attempt.
- **On-disk license blob is never erased by a failed cached restore** (existing rule,
  preserved).

## Open Questions

- None blocking. R4 surfacing channel (ProblemCenter vs one-shot notification) is a plan
  decision.
