---
spec: 0042-license-token-hardening
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-07-28
---

# Plan 0042 — CommercialToken lifecycle hardening

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Four surgical moves, no new abstractions. (R1, landed as an approved hotfix) the startup
revalidation gates on the stored key (`canActivate()`), not on in-memory licensing data a
failed cached restore just cleared. (R2) the licensing block (`MachineID`, `LemonSqueezy`,
`OfflineLicense`, `Trial`) moves to the top of `instantiateCoreModules()`, directly after
`Translator` (licensing ctors emit `tr()` strings; Translator consumes no entitlement) and
before everything else; the spec-0001 ctor-edge proof is re-run for the new order. (R3) a
consumer inventory (`consumers.md` in this spec dir) classifies every
`CommercialToken::current()` TU; the audit relies on the existing funnel — Trial and
OfflineLicense already forward their transitions into `LemonSqueezy::activatedChanged` — so
"wired" means connected to that one signal; any bakes-state consumer found unwired gets the
connection. (R4) a `Misc::ProblemCenter` checker reports, per loaded project source whose
bus is commercial-gated, "requires Serial Studio Pro or an active trial" whenever
`CommercialToken::current().isValid()` is false; it reads token + `ProjectModel::sources()`
synchronously (spec-0035 pull rule) and clears on the next poll after entitlement arrives.
(R5) the `[mqtt-debug]` lines and the user's `qDebug` probe are removed.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/Misc/ModuleManager.cpp` | R1 (done): revalidation gate on `canActivate()`. R2: move the `#ifdef BUILD_COMMERCIAL` licensing block from after `adoptAppState` to directly after `(void)Misc::Translator::instance()`; update the function's `@brief` order note. |
| `doc/claude/specs/0001-composition-root/` | R2: append the re-run ctor-edge proof note for the new order (licensing ctors reach only MachineID/SimpleCrypt/QSettings/QNAM/qApp — verified reads this session; recorded formally here). |
| `doc/claude/specs/0042-license-token-hardening/consumers.md` | R3: the audited inventory — every TU from `grep -rl "CommercialToken::current()"`, classified sample-per-op vs bakes-state(+wire site). |
| (any consumer found unwired by R3) | Add the missing `activatedChanged` connection — expected zero to two files; named in chat before touching if outside this table. |
| `app/src/Misc/ConnectionDiagnostics.cpp` (or sibling checker TU per its registration pattern) | R4: `licensing.entitlement` checker — synchronous, reads token validity + project sources' bus types, emits one finding per gated-but-unlicensed source; registered like the existing bus checkers. |
| `app/src/IO/ConnectionManager.cpp` | R5: remove the five `[mqtt-debug]` blocks (restore original bodies). |
| `app/src/IO/Drivers/MQTT.cpp` | R5: remove the maintainer's diagnostic `qDebug` in `configurationOk()` (added during this bug hunt; removal is the agreed cleanup, not a foreign-file touch). |
| `CLAUDE.md` | R2: one-line update to the Startup section (licensing block now first; late-token hazard note stays as history). |

## Architecture & data flow

- **R2 order**: `Translator -> [MachineID, LemonSqueezy, OfflineLicense, Trial] ->
  TimerEvents -> ... -> ProjectModel -> AppState -> ... -> Dashboard`. LemonSqueezy's ctor
  synchronously restores the cached license (installing the token) or leaves it clear;
  OfflineLicense/Trial ctors install for their entitlements. From that point the token is
  final-for-startup before ProjectModel's ctor closure runs, killing the
  "ProjectModel-derived state baked before entitlement known" class at the root.
  `restoreLastProject()` and the queued `validate()` remain where they are.
- **R3 funnel**: entitlement transitions all reach `LemonSqueezy::activatedChanged`
  (LemonSqueezy emits on flip/clear; Trial `enabledChanged` and OfflineLicense
  `activatedChanged` are ctor-connected into it). The inventory documents this as the one
  signal a bakes-state consumer must wire.
- **R4 checker**: pull-model; no signals, no per-frame work. Poll happens on the existing
  ProblemCenter cadence; the finding text names the bus and the fix path (activate/trial).

## Hotpath & threading impact

- **Touches the hotpath?** No. Composition-root order, a startup gate, a synchronous
  checker, log removal.
- **New cross-thread signal/slot?** None.
- **New input to a cached hotpath flag?** None.
- **Timestamp ownership** — untouched.

## Data model & persistence

None. No settings keys, no project JSON, no schema changes.

## API / SDK surface

None. (`licensing.validate` API already exists and now benefits from R1's gate fix.)

## QML / UI

None beyond the ProblemCenter entry rendering through its existing UI.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| R4 channel | ProblemCenter checker / one-shot notification | **ProblemCenter** — persistent while true, self-clearing, matches the spec-0035 pull model; a notification misses projects loaded later. |
| R2 placement | very first / after Translator | **After Translator** — licensing ctors produce translated user-facing strings; Translator has no entitlement surface. Everything else moves below licensing. |
| R3 mechanism | new token-change broker QObject / existing activatedChanged funnel | **Existing funnel** — Trial/OfflineLicense already forward into `activatedChanged`; a new broker adds a second source of truth for zero coverage gain. The inventory makes the funnel's completeness auditable instead. |
| Token-install visibility | emit on every `setCurrent` / keep source-level emission | **Keep source-level** — `setCurrent` also runs inside `GuardSelfTest`'s tamper juggling; emitting there would broadcast transient invalid states. |

## Risks & mitigations

- **Ctor-edge regression from the reorder** (the composition root's own hazard class):
  licensing ctors were read in full this session — they touch `MachineID`, `SimpleCrypt`,
  `QSettings`, `QNetworkAccessManager`, `qApp`, and each other, and nothing constructed
  later; the proof note records this. Any future licensing-ctor edit re-triggers the check
  by the existing spec-0001 rule.
- **Message boxes from licensing ctors pre-QML**: ctor paths are silent by construction
  (`m_silentValidation` true at restore; Trial/Offline boxes live in interactive/server
  handlers only) — re-verified during implementation.
- **R4 checker touching drivers**: it must read only `ProjectModel::sources()` bus types +
  token validity — never driver instances (diagnostics never touch driver config,
  spec-0035).
- **R5 removes exactly the diagnostic lines** — restore-to-original verified against git
  diff, nothing else in those hunks.

## Test & verification plan

- **AC1/AC2/AC4/AC5** — maintainer runtime checks as written in the spec (grace-forced
  relaunch with network up/down; GUI/headless/CLI boots; mid-session activation).
- **AC3** — `consumers.md` present; re-running the generating grep matches the table.
- **AC6** — `grep -r "mqtt-debug" app/` empty.
- **Static** — `code-verify.py --check` on touched files; `sanitize-commit.py` at commit
  time (maintainer, tree carries unrelated campaigns).
