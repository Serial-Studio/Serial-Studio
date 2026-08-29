---
spec: 0008-assistant-auto-canary
title: Assistant Context Health & Compounding Workflows (Weak-Model-First)
status: done          # closed 2026-08-20
created: 2026-07-14
author: Alex Spataru
---

# Spec 0008 — Assistant Context Health & Compounding Workflows (Weak-Model-First)

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Long AI Assistant conversations degrade silently. As a chat grows, the model's effective
grip on its instructions weakens — it starts ignoring rules from its system prompt, drifting
on facts stated earlier, or producing subtly wrong tool calls — and the user gets no signal
until the output is visibly wrong, which for telemetry work can mean a bad dashboard edit or
a destructive command issued with confidence. This repo already runs the same experiment on
itself: the CLAUDE.md Context Canary (a fixed line of load-bearing constants reproduced from
memory every turn) has proven that a *mutated or missing sentinel is a leading indicator of
context degradation*, arriving before the visible failures do. Assistant users have no
equivalent probe, and unlike the repo canary — which relies on a human noticing a drifted
value — an in-app probe can be checked by the application itself.

Degradation detection is only half of the reliability story. The assistant's per-turn
machinery is strong — progressive tool disclosure, on-demand skills, four-tier command
safety, dry-run pre-flight, rolling checkpoints — but everything that spans turns or chats
rides on the model's own discipline. The system prompt *asks* the model to load a skill
before its first tool call in a domain, *asks* it to verify work, and offers meta tools for
both; frontier models comply, but the default model is Haiku-class, and most non-Anthropic
providers' popular tiers (flash/mini/small/local) are in the same band — the app already
detects these and shrinks their tool surface. A weak model that forgets the skill load
choreographs tools wrong; one that skips verification reports "done" on a broken parser; and
none of them can compensate for the fact that every chat starts cold: a user who spent one
chat teaching the assistant their device setup, preferred scripting language, and one painful
correction re-types all of it next session or watches the same mistake happen again.

The two halves share one root cause and one fix. The complicating reality is that the
assistant supports many providers and model tiers, from frontier hosted models down to small
local ones. Weak models drop trailing-format boilerplate *while otherwise working fine*; a
naive probe would alarm constantly on exactly the cheap models users choose for cost
efficiency, training everyone to ignore it. And those same weak models will not reliably
self-orchestrate memory, skill loading, or verification no matter how the prompt begs. So
the application must carry the discipline deterministically: probe health itself, route
knowledge itself, verify work itself, and carry context forward itself — with the probe
silent-by-construction on models that never complied, cheap enough that token-conscious
users are not punished, and loud only when it carries real signal. When degradation *is*
detected, recovery must not cost the user their working state: a fresh chat that forgets
everything is a punishment, not a fix — which is why the probe and the compounding
mechanisms ship as one feature.

## Goals

- The user is warned, gently and inside the assistant window, when the current conversation
  shows evidence of context degradation — before they act on degraded output.
- The warning carries a one-click path to recovery (start a fresh chat) that preserves the
  working context of the chat it replaces.
- The probe is invisible in healthy operation: no sentinel text ever appears in the
  displayed reply, no chrome appears while checks pass.
- Users of cheap/small/local models keep using them efficiently: no false-alarm noise from
  models that never follow the sentinel instruction, and a hard off-switch that removes the
  probe's token cost entirely.
- The sentinel encodes facts the assistant's own rules depend on, so a *which-value-drifted*
  diagnosis is possible, not just a binary alarm.
- A new chat on a Haiku-class model already knows the durable facts the user established in
  earlier chats (device conventions, preferred parser language, standing corrections) without
  the user re-typing them.
- A correction the user makes once does not recur in later chats.
- Skill knowledge for a task's domain reaches the model before its first tool call in that
  domain, regardless of whether the model would have asked for it.
- After the assistant applies a mutation, the user sees the application's own pass/fail
  verification evidence — not the model's self-assessment.

## Non-Goals

- Not a token counter or context-fill meter. The application could measure prompt size
  deterministically; that is complementary and out of scope. This feature measures the
  model's *behavioral* health, which token counting cannot.
- No automatic conversation truncation, summarization, or forced restart. The user decides;
  the feature only informs and offers.
- No curated per-provider/per-model capability lists to maintain. Adaptation to model
  capability must be automatic (learned from observed behavior), not hand-configured.
- No change to the repo-side CLAUDE.md Context Canary or the Claude Code hook — those are
  separate, already-shipped mechanisms for a different audience.
- No blocking UX: never a modal, never a refusal to send the next message.
- No cloud sync, sharing, or telemetry of remembered facts; storage is local to the machine.
- Not a document store or general retrieval over user data — memory holds small curated
  facts, not files or logs (the bundled-docs BM25 search already covers reference lookup).
- No automatic model switching or tier routing; the user's provider/model choice is theirs.
- No change to the four-tier safety model, the device-control gate, or approval semantics.

## Requirements

### Context-health probe

1. **R1** — Every assistant request instructs the model to end its visible reply with a
   short fixed sentinel line. The sentinel's content is a small set of load-bearing facts
   the assistant's operating rules depend on (chosen at plan time), not repository build
   constants; a mutated value must identify *which* behavioral anchor drifted.
2. **R2** — The sentinel is never shown to the user: it is stripped from the rendered reply,
   including during streamed display (no visible flash of sentinel text while streaming),
   and it is excluded from copied or exported conversation text.
3. **R3** — Each completed reply is validated: sentinel present and verbatim → healthy;
   present but mutated → degraded; absent → degraded, subject to the compliance state in R5.
4. **R4** — On degradation of a *compliant* model (per R5), the assistant window shows a
   persistent, non-modal "context degraded" indicator with an inline one-click action to
   start a fresh chat. The indicator explains, on hover or expansion, what was detected
   (missing vs mutated, and the drifted segment when mutated). The fresh chat is seeded
   with the degraded chat's handoff (R14) so recovery does not cost working state.
5. **R5** — Adaptive auto-mute: a model that has never reproduced the sentinel in the
   current conversation's early replies is classified non-compliant and the indicator stays
   muted for it (validation may continue silently). A model that reproduced the sentinel at
   least once and subsequently fails it triggers the R4 indicator. Compliance classification
   is tracked per provider+model and persists across sessions, so a known-weak model never
   re-alarms in every new chat.
6. **R6** — The probe is on by default and has a settings toggle. When off, the sentinel
   instruction is not sent at all — zero added prompt or completion tokens.
7. **R7** — Bounded cost when on: the sentinel itself is at most ~30 completion tokens and
   the added instruction is comparably short. Total per-reply overhead must be small enough
   that a cost-conscious user on a cheap model sees no meaningful bill difference.
8. **R8** — Degradation state is per conversation: starting a fresh chat clears the
   indicator. Compliance classification (R5) is the only probe state that outlives a
   conversation.
9. **R9** — The probe works identically across all supported providers, including local
   models, with no provider-specific code required for correctness — provider variance is
   absorbed entirely by R5.
10. **R10** — Validation outcomes (healthy / mutated / missing / muted) are recorded in the
    assistant's existing diagnostic logging so a bug report can show the probe's history.

### Compounding workflows

11. **R11 — Persistent assistant memory.** The assistant can retain small durable facts
    across chats and application restarts, categorized as user preference,
    correction/feedback, project fact, or external reference. Project facts are scoped to
    the project they came from; the rest apply installation-wide.
12. **R12 — Router-pattern loading.** Only a compact index of remembered facts (hard-capped
    size) accompanies requests; full detail loads on demand. Total added cost per request is
    bounded and small enough that a cost-conscious user on a cheap model sees no meaningful
    bill difference. With memory disabled, requests carry zero memory content.
13. **R13 — Consent-gated writes, full user control.** The assistant proposes remembering
    something (typically after a correction or a stated preference); nothing persists
    without an explicit user action. The user can browse, edit, and delete every remembered
    fact in the UI, and a deleted fact never appears in a subsequent request. Secret-like
    content is refused or redacted before it can be stored.
14. **R14 — Chat handoff.** Leaving or closing a chat yields a compact structured summary
    (what was done, decisions made, what's next). Starting a new chat can seed from the most
    recent handoff in one click, and the R4 recovery action carries the handoff
    automatically.
15. **R15 — Deterministic skill routing.** The application matches the user's message
    against skill trigger vocabulary and delivers the matching skill body to the model
    before its first tool call in that domain, without the model requesting it. The existing
    on-demand skill loading remains available; routing respects the reduced budgets of
    small-surface models.
16. **R16 — Harness-enforced verification.** After an apply-class mutation (script apply,
    bulk edit, project mutation), the application automatically runs the matching read-back
    or dry-run check and delivers the outcome both to the model and to the user as visible
    pass/fail chrome. A failed verification presents the existing checkpoint/restore
    recovery affordance. Verification steps are read-only/dry-run class — they never
    auto-execute anything that would itself require confirmation, and device-gated
    operations are excluded entirely.
17. **R17 — Weak-model parity.** R11–R16 are driven by the application and behave
    identically across all supported providers, including local models; model capability
    flags may tune budgets but never gate correctness.
18. **R18 — Independently togglable.** Memory, handoff seeding, skill routing, and auto
    verification each have a settings toggle; a disabled feature contributes zero tokens to
    requests.
19. **R19 — Observable.** Memory injections, skill preloads, handoff seeds, and verification
    outcomes are recorded in the same assistant diagnostic logging as R10, so a bug report
    shows what the harness did on each turn.

## Acceptance Criteria

- [x] **AC1** — With a compliant model, a healthy multi-turn chat shows no sentinel text
      anywhere in the rendered replies (including mid-stream) and no indicator. Verified by
      maintainer observation with a hosted frontier model.
- [x] **AC2** — Copying a reply and exporting the conversation yields text without the
      sentinel. Maintainer observation.
- [x] **AC3** — When a compliant model's reply arrives with the sentinel mutated or missing
      (reproducible via a local/scripted provider endpoint that replays canned replies), the
      indicator appears, names the failure kind, and its action starts a fresh chat which
      clears the indicator and carries the old chat's handoff. Maintainer observation
      against a scripted endpoint.
- [x] **AC4** — A model that never emits the sentinel (scripted endpoint returning plain
      replies from turn one) produces no indicator across an entire conversation, and is
      still muted in a *new* conversation with the same provider+model. Maintainer
      observation.
- [x] **AC5** — With the probe toggle off, the outbound request contains no sentinel
      instruction (verified in the assistant's request logging) and replies render
      unchanged.
- [x] **AC6** — The sentinel measured against a real provider costs ≤ ~30 completion tokens
      per reply (verified via the assistant's token counters).
- [x] **AC7** — All probe validation outcomes from AC3/AC4 appear in the assistant
      diagnostic log.
- [x] **AC8** — On the default Haiku model: state a durable fact in chat A, consent to
      remember it, start chat B, ask a question that depends on it — the assistant answers
      correctly without being retold. Maintainer observation.
- [x] **AC9** — Request logging shows the memory index within its size cap on every turn;
      with the memory toggle off, the outbound request contains no memory content.
      Verified via the assistant's request logging and token counters.
- [x] **AC10** — On a small-surface model, a message in a skill's domain (e.g. asking for a
      frame parser) results in that skill's body being present in the request before the
      model's first tool call in the domain, with no model-initiated skill load. Verified
      in request logging.
- [x] **AC11** — After the assistant applies a script, the verification check runs without
      the model requesting it and the chat shows a pass indicator; repeating with a
      deliberately broken apply (scripted provider endpoint replaying a canned bad tool
      call) shows a fail indicator plus the restore affordance. Maintainer observation
      against a scripted endpoint.
- [x] **AC12** — Closing a chat mid-task and starting a new seeded chat lets the user say
      "continue" and get contextually correct continuation on a Haiku-class model.
      Maintainer observation.
- [x] **AC13** — Editing and deleting a remembered fact in the UI is reflected in the next
      request's memory content (edit) or absence (delete). Request logging.
- [x] **AC14** — Attempting to remember a string containing an API key results in refusal
      or a stored fact with the secret redacted. Maintainer observation.
- [x] **AC15** — All harness actions from AC8–AC14 appear in the assistant diagnostic log.

## Constraints & Invariants

- Two deciding constraints, one per half. Probe: **no false-alarm noise on models that never
  complied** — a probe users learn to ignore is worse than no probe; R5 is load-bearing.
  Compounding: **assume the model never self-orchestrates** — every mechanism must function
  with a model that never volunteers a meta-tool call; prompt instructions may reinforce
  behavior but must never be the mechanism. Every design choice in `plan.md` is measured
  against both.
- Zero cost when disabled (R6, R18) — not "hidden," *absent from the request*.
- Must not interfere with the assistant's tool-call loop, streaming render path, or the
  existing safety/approval flow for device-controlling tools; nothing in this feature
  auto-executes confirm-class or device-gated commands.
- Sentinel stripping must be robust to partial/streamed arrival — a half-received sentinel
  must not leak to the display or be judged as a mutation before the reply completes.
- Bounded, cache-friendly cost: added always-on content must respect existing per-provider
  token/byte budgets and must not materially regress prompt-cache hit rates on providers
  that cache.
- Privacy: memory is stored locally only, and the existing secret-redaction pass applies to
  anything before it is persisted or transmitted.
- Provider-neutral correctness: no provider-specific code paths required for any requirement
  to hold.
- Memory and handoff storage are bounded (caps with an aging/eviction policy) — the store
  must not grow without limit.
- New user-facing strings are translatable; no `.ts`/`.qm` files are regenerated as part of
  this feature. No new third-party dependency.
- The indicator is advisory chrome only — it must never gate or delay message sending.

## Open Questions

- License gating: available to all assistant users, or Pro-gated? Recommendation: available
  to all — these are safety/quality features, and the users most exposed to degradation are
  those on cheap models, who are least likely to be Pro.
- Compliance memory granularity: per provider+model is proposed (R5); is that the right key,
  or should a model-version change reset classification? Recommendation: key on
  provider+model identifier as reported by the provider; a changed identifier naturally
  resets.
- How many early replies define the R5 compliance window before auto-mute is decided?
  Recommendation: decide at plan time from the tool-call loop shape (e.g. first 2-3
  visible replies), not hard-committed here.
- Consent UX shape for R13: explicit per-fact confirmation chip (recommended — mirrors the
  existing confirm-tier pattern and keeps trust) vs. an opt-in auto-save mode with
  review-after.
- Handoff seeding default: automatic for the R4 recovery path and one-click for ordinary
  new chats (recommended), or automatic everywhere?
- Skill-routing trigger source: curated per-skill trigger vocabulary (recommended — small,
  auditable, offline) vs. reusing the BM25 index for fuzzy matching. Decide at plan time
  after measuring false-positive rates.
- Does R16 verification cover MQTT/CAN/Modbus configuration mutations (which have no dry-run
  today), or only the script/project families that already expose one? Recommendation:
  start with the existing dry-run families; extending dry-run coverage is its own spec.
