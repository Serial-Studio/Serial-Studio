---
spec: 0092-lazy-trial
title: Lazy Pro Trial & Graceful Degradation
status: done         # draft -> approved -> in-progress -> done | shelved
# AC1-AC4, AC7 await maintainer verification in the running app (next build); AC5/AC6
# partials and AC8 are written and registered but run only against a built app / build dir.
created: 2026-10-03
author: Alex Spataru
---

# Spec 0092 — Lazy Pro Trial & Graceful Degradation

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Pro build greets a first-time user with a modal welcome dialog that cannot be
dismissed: closing it quits the application, and the only way to reach the main window is
to start the 14-day trial or activate a license. Starting the trial fires a server
registration carrying a machine identifier — so the very first interaction with the app
is a licensing wall plus a network call, before the user has seen a single feature work.
The maintainer's judgment is that this alienates users at the top of the funnel: fewer
completed installs, less usage, and therefore fewer prospective customers.

The wall also wastes the trial itself. The 14-day clock starts at first launch, while the
user is still evaluating basics (does my device connect? does the console show data?), so
trials routinely expire before the user ever exercised a Pro feature — the worst possible
moment to ask for money. And while the trial is active, a recurring "days remaining"
dialog nags on launch unless explicitly silenced, which compounds the alienation.

Meanwhile the post-expiry behavior already proves the better model: "Limited Mode" opens
the app with the free (GPLv3-equivalent) feature set and per-feature Pro gates. This spec
reorders the experience around that model — the app is always usable, and the trial
becomes an opt-in moment attached to the user's first Pro intent.

## Goals

- A first-time user reaches a fully working main window with zero licensing interaction
  and zero licensing network traffic.
- The 14-day Pro trial starts at the moment of first Pro intent, via a single explicit
  question, so the trial window covers actual Pro evaluation.
- Licensing state is communicated passively (a console line), never through recurring
  dialogs — the user is told once per state transition, at most.
- An expired or unlicensed install remains a complete, indefinitely usable free app.
- Activation and purchase stay reachable on demand, without a dedicated toolbar button.

## Non-Goals

- **No change to what is Pro vs free.** The feature tier assignment stays exactly as it
  is; only *when* and *how* the gate is presented changes.
- **No change to trial mechanics server-side.** Same 14-day term, same per-machine
  registration, same server endpoint and token flow; only the client-side trigger moves.
- **No change to the GPL3 build.** It has no licensing flow today and gains none.
- **No new purchase/checkout flow.** Buying and activating keep their existing paths
  (store page, license dialog, offline license).
- **No telemetry or analytics.** The funnel reasoning stays a judgment call; this spec
  adds no usage tracking to measure it.
- **No removal of the trial-parity rule.** The trial still unlocks everything Pro.

## Requirements

1. **R1 — No first-run wall.** On a fresh install of the Pro build, the application
   boots directly to the main window. No blocking dialog, no trial registration, no
   licensing network request of any kind occurs until the user expresses Pro intent or
   opens the license dialog themselves.
2. **R2 — Trial on first Pro intent.** When a user on a device with no license and no
   registered trial attempts to use a Pro feature, the application asks one question
   (message-box style): start the free 14-day Pro trial, or not. Accepting registers the
   trial and the attempted feature proceeds immediately — same session, no restart, no
   re-invocation of the gesture. Declining leaves the feature locked and the app
   otherwise untouched; a later attempt may ask again.
3. **R3 — Every Pro gate is a trigger.** All existing Pro gates present the same trial
   question consistently: selecting a Pro driver, adding or displaying a Pro widget,
   enabling a Pro export/storage sink, dashboard freeze, and any other gated surface.
   Opening a project file that uses Pro features counts as Pro intent and prompts on
   load (maintainer decision, 2026-10-03); declining loads the project degraded with
   the existing passive Pro-features notice.
4. **R4 — Passive status, no nag.** While a trial is active, the only unsolicited
   status indication is a single line in the console, printed after the console welcome
   message (e.g. remaining days). No launch dialog, no countdown popups, no "don't nag
   me" checkbox — there is nothing to silence. The license dialog remains the place to
   see full detail on demand.
5. **R5 — Expiry is told once, then quiet.** When the trial expires, the user is told
   once (one message box at the first detection of the transition), and thereafter the
   console line reflects the expired state. Pro features lock again; attempting one
   presents an activate/purchase prompt instead of the trial question. No per-launch
   expiry dialog (maintainer decision, 2026-10-03).
6. **R6 — Free features are never hostage.** With no license, a declined trial, or an
   expired trial, every free feature works identically to the GPL3 feature set,
   indefinitely. No recurring prompt interrupts normal (non-Pro) use in any licensing
   state.
7. **R7 — No toolbar Activate button.** The expired-state "Activate" button is removed
   from the main toolbar; the Connect button keeps its place and behavior in all
   licensing states. Activation and purchase remain reachable through the existing
   non-toolbar entry points (license dialog, menus/command surface) and through the
   locked-feature prompt of R5.
8. **R8 — Mid-session transitions apply live.** Starting the trial, activating a
   license, or expiry taking effect mid-session each propagate to every gated surface
   in that same session: degraded widgets upgrade in place, locked drivers become
   selectable, and vice versa on expiry — without reopening the project or restarting
   the app.
9. **R9 — Trial start requires the server; failure is graceful.** If the trial
   registration cannot be completed (offline, server error), the user is told in a
   non-blocking way, the feature stays locked, and nothing is persisted that would
   consume or block the trial; the next Pro attempt may ask again.
10. **R10 — Existing installs migrate sanely.** Devices with a trial already running
    keep their remaining days; already-expired devices are treated as expired (R5
    behavior, including the one-time notice if not already shown); activated licenses
    are unaffected. No state is reset by the upgrade.

## Acceptance Criteria

- [ ] **AC1** (R1) — Fresh-profile launch of the Pro build reaches the main window with
      no dialog, and a network capture shows no licensing/trial request. Maintainer
      observation (plus a scripted check that boot completes with the API server up and
      no trial state persisted).
- [ ] **AC2** (R2, R3) — On an unregistered device, selecting a Pro driver (e.g.
      Modbus) raises the trial question; accepting registers the trial and the driver
      becomes usable in the same session. Maintainer observation.
- [ ] **AC3** (R3) — Opening a project containing Pro widgets on an unregistered device
      prompts; declining loads the project with fallback widgets and the passive
      notice; accepting upgrades the widgets in place. Maintainer observation.
- [ ] **AC4** (R4, R5) — With a trial active, launch shows no dialog and the console
      shows the status line after the welcome message; with the trial expired, the
      one-time notice appears exactly once across restarts, and subsequent Pro attempts
      show the activate/purchase prompt. Maintainer observation across two launches.
- [ ] **AC5** (R6, R7) — With an expired trial: the toolbar shows Connect (no Activate
      button), a free device type connects and plots normally, and a full session
      (connect, dashboard, CSV export) completes with no licensing dialog. `pytest`
      integration flow where drivable over the API, maintainer observation for the
      toolbar.
- [ ] **AC6** (R8) — With a Pro-featured project open degraded, completing activation
      (or trial start) upgrades every degraded widget and gate in place, without
      reopening the project — the spec-0042 consumer inventory re-verified. Maintainer
      observation; automated where the API can read widget types.
- [ ] **AC7** (R9) — With the network blocked, accepting the trial question reports
      failure non-blockingly, the feature stays locked, and a later attempt (network
      restored) succeeds with the full 14 days. Maintainer observation.
- [ ] **AC8** (R10) — A settings store carrying an active in-term trial yields the same
      remaining days after upgrade; an expired store yields expired behavior. `pytest`
      or scripted check against seeded settings where feasible, else maintainer
      observation.
- [ ] **AC9** — `--benchmark-hotpath` gates unchanged (licensing is command-rate UI
      state; nothing here may touch the acquisition pipeline).

## Constraints & Invariants

- **The deciding constraint: an unlicensed install is a complete product.** Nothing in
  any licensing state may block, time-limit, or nag normal use of free features.
- **Trial parity holds.** An active trial unlocks the entire Pro tier, and user-facing
  docs continue to never say "requires a license" without mentioning the trial.
- **License-gated state must re-derive on activation transitions.** Lazy trial makes
  mid-session activation the *common* path, not the edge case; every consumer that
  bakes activation into derived state must track the transition live (the 2026-07-09
  fallback-widget incident is the failure shape to prevent).
- **Licensing remains command-rate.** No licensing check, prompt, or state propagation
  may add work to the acquisition pipeline or any per-frame path.
- **Trial trust model unchanged.** Client-side state stays as tamper-resistant as
  today; the server remains the authority on per-machine trial consumption.
- **Project schema untouched.** Degraded load of a Pro-featured project keeps today's
  semantics; no new fields, no load warnings on old files.
- **UI conventions.** New prompts and console lines follow existing console/dialog
  conventions, command/icon registration rules, and reduced-motion behavior; removed
  UI (welcome wall, toolbar button) leaves no orphaned command or icon entries.
- **No new third-party dependency.**

## Open Questions

None. The four drafted questions (project-load trigger, expiry notice modality,
toolbar Activate removal, decline-and-ask-again cadence) were resolved with the
maintainer on 2026-10-03 and folded into R3, R5, R7, and R2 respectively.
