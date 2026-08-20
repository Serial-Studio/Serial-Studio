---
spec: 0033-problem-center
title: Problem center (project + link diagnostics)
status: done          # closed 2026-08-20
created: 2026-07-25
author: Claude (drafted with Alex)
---

# Spec 0033 — Problem center (project + link diagnostics)

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R8. The roadmap ranks it "highest support-load
> reduction per line of code, no dependencies, start any time". Roadmap item R9
> (connection diagnostics self-test) depends on this spec: its checks must be able to
> register into the collector this spec builds, so the registration surface is part of
> the contract, not an afterthought.

## Problem / Motivation

When a Serial Studio dashboard comes up blank, nothing in the application tells the user
why. The three failure families that produce that outcome are all currently silent or
near-silent:

- **Project-schema mistakes fail quietly.** The only project validation that exists is a
  save gate: it checks that the project has a title, at least one group, and at least one
  dataset, and it speaks only through a modal box on save. Nothing checks that two datasets
  in the same source claim the same frame index, that a dataset's X-axis or waterfall Y-axis
  points at a dataset that has since been deleted, that a group has no datasets and no
  output widgets, that a plot's minimum is above its maximum, or that a workspace tile
  references a widget that no longer exists. Each of those produces a widget that is empty,
  duplicated, or wrong, with no message anywhere.
- **Link problems are reported as one-shot noise or not at all.** A checksum mismatch logs a
  hex dump per failing frame and increments no counter, so a stream that is 3% corrupt looks
  identical to a stream that is 100% corrupt: a wall of console text and no number. A wrong
  delimiter — the single most common support question — yields bytes arriving, zero frames
  extracted, and complete silence. Frame-queue drops and ring-buffer overruns already keep
  counters internally, but they surface only as transient notifications when they first
  happen, never as a standing "this link is losing data right now" state with a remedy.
- **Script failures are counted nowhere.** A per-dataset transform that throws on every
  frame degrades to raw values silently; the JavaScript watchdog posts one notification when
  the frame budget is exceeded and then says nothing for the next hour. There is no answer to
  "which script is failing, how often, and with what message".

The existing notification center is the right *destination* for these findings but the wrong
*model* for them. It is a transient, deduplicated, capped event log — it answers "what just
happened", newest first, and forgets. A problem is a standing condition: it is either true
right now or it is not, it must be listable, explainable, and clickable, and it must
disappear by itself when the user fixes the cause. Nothing in the application holds that
kind of state today.

The support cost is concrete and recurring: "my widgets are empty", "my data looks wrong",
"nothing shows up" are answered today by a maintainer reading the user's project file by
hand. The same reading is mechanical and can be done by the application.

## Goals

- A user whose dashboard is blank or wrong can open one place, see a list of what is wrong,
  read one sentence explaining each item, and click it to land on the thing that must change.
- A project that is inconsistent (duplicate frame indices, dangling references, empty groups,
  inverted ranges) says so at load time and keeps saying so until it is fixed.
- A link that is receiving bytes but producing no frames, failing checksums at a measurable
  rate, dropping frames, or overrunning its buffer reports that as a standing condition with
  a rate, not as a burst of log lines.
- A failing parser or transform script reports its error text and how many times it has
  failed, not a single notification an hour ago.
- Findings reach the user through the surface they already watch: the notification center
  and a visible severity indicator, not only a panel they must know to open.
- An LLM agent driving the API can read the current problem list and act on it, so
  "why is my widget empty" is answerable programmatically as well as visually.
- A new checker — including the I/O checks that roadmap item R9 will add — is registered
  against a documented collector API without touching the panel, the model, the badge, or
  the API handler.

## Non-Goals

- **No automatic repair.** The problem center reports and navigates; it never mutates the
  project. "Fix it for me" is out of scope for v1 (and, when it arrives, belongs behind
  undo — spec 0031).
- **No replacement for the notification center.** Notifications stay the transient event log.
  The problem center summarizes into it; it does not absorb it.
- **No replacement for the save-blocker flow.** The existing save gate keeps its modal
  behavior; the same conditions may additionally appear as problems, but saving is not newly
  blocked or unblocked by this feature.
- **No I/O self-test checks in this spec.** Port permissions, driver enumeration, broker
  reachability, and BLE stack state are roadmap item R9. This spec only guarantees that they
  can register into the collector without changing it.
- **No per-frame diagnostics.** Nothing here inspects individual frames, records their
  content, or adds per-frame work to the parse path. Link findings are derived from
  aggregate counters sampled on a timer.
- **No persistence.** The problem list is session state. It is not written to the project
  file, not restored across restarts, and never changes the project's on-disk bytes.
- **No new severity taxonomy beyond three levels.** Findings map onto the levels the
  notification center already uses.
- **Not a Pro feature.** Diagnostics are the support-load fix; they ship in GPL builds.

## Requirements

1. **R1 — One collector, many checkers.** A single session-scoped collector holds the
   current problem list. Any subsystem registers a checker against it by declaring an id, a
   human-readable name, and when the checker runs. Registering a checker requires no change
   to the collector, the model, the panel, the badge, or the API surface.
2. **R2 — Three run triggers.** A checker declares that it runs on project change (load,
   mutation, save), on a periodic link-statistics sample, or on demand (an explicit
   "re-run diagnostics" request). A checker may declare more than one. Roadmap item R9's
   connection checks must be expressible as on-demand plus on-connection-failure without
   extending the trigger vocabulary in an incompatible way.
3. **R3 — Findings carry everything the user needs.** Each finding has: a severity
   (information, warning, error), a short title, one or two sentences of explanation that
   name the concrete cause, an optional remedy sentence, and an optional jump target. The
   jump target identifies either a project entity (so the editor can select it) or a
   settings/dialog location.
4. **R4 — Findings are standing, not events.** Re-running a checker replaces that checker's
   previous findings wholesale. A condition that has been fixed disappears from the list
   without user action. The same condition present across two runs does not duplicate and
   does not re-notify.
5. **R5 — Visible without being opened.** A severity indicator shows the current counts by
   severity and is visible from the main application chrome. New or newly-escalated findings
   also post to the notification center; unchanged findings do not re-post.
6. **R6 — A panel lists and navigates.** A dedicated panel lists the current findings
   grouped or sortable by severity, shows each finding's explanation, and — where the finding
   has a project jump target — selects that entity in the project editor when activated. The
   panel is reachable from the main window and from the project editor.
7. **R7 — Opening the panel is a registered command.** The panel opens through the command
   registry (spec 0028) like every other surface: it appears in the command palette and the
   Start menu, and can carry a shortcut.
8. **R8 — Initial project checkers.** At minimum: duplicate frame indices within a source;
   a dataset index that no frame can ever supply; groups with no datasets and no output
   widgets; dangling references (X-axis source, waterfall Y-axis source, workspace tile
   references, action/output-widget source ids); inverted or degenerate numeric ranges (plot,
   widget, FFT, LED threshold, alarm bands); duplicate dataset aliases.
9. **R9 — Initial link checkers.** At minimum: bytes received but no frames extracted over a
   sustained window (delimiter/frame-detection mismatch); frames extracted but none parsed;
   checksum failure rate above a threshold; frame-queue drops; ring-buffer overruns. Each
   reports the rate or count that triggered it.
10. **R10 — Initial script checkers.** Parser and per-dataset-transform failures are counted
    per source and per dataset, with the most recent error message retained, and reported
    with those counts.
11. **R11 — Readable over the API.** The current problem list is exposed as a read-only
    command under a `problems` scope following the `<scope>.<verb>` naming convention, with a
    forced re-run verb. The commands are safe (non-mutating) for the in-app assistant.
12. **R12 — No cost when nothing is wrong.** With no problems present and no checker
    reporting, the feature adds no measurable work to the frame parse path and no per-frame
    signal traffic. The 256 kHz benchmark gates are unchanged.

## Acceptance Criteria

- [x] **AC1 (R1, R2)** — A new checker is added in a single file plus one registration line;
      no other file in the problem-center subsystem changes. Demonstrated by the fact that the
      project, link, and script checker sets are added in three independent tasks, each
      touching only its own file plus the registration point.
- [ ] **AC2 (R3, R4, R8)** — `pytest tests/integration/test_problem_center.py`: loading a
      project with two datasets sharing a frame index in the same source lists exactly one
      error-severity finding naming both datasets; deleting one of them and re-running clears
      it; re-running with the problem still present leaves the list unchanged (no duplicates).
      *Code-complete; the maintainer runs the file.* Covered by
      `test_duplicate_frame_index_reported_once` + `test_finding_clears_when_condition_fixed`.
      **Severity is Warning, not Error** — `plan.md` overrides this line: two datasets
      legitimately share an index when one value drives two widgets.
- [ ] **AC3 (R8)** — Same test file: a dataset whose X-axis points at a deleted dataset
      produces a finding whose jump target is the offending dataset; an empty group produces
      a warning; a plot with min above max produces a warning. *Code-complete; maintainer
      runs `test_dangling_xaxis_reference_has_jump_target` +
      `test_empty_group_and_inverted_range_are_warnings`.*
- [ ] **AC4 (R9)** — Same test file, over a live TCP link: sending well-formed bytes with a
      delimiter the project does not use produces a "bytes received, no frames extracted"
      finding within the sample window and clears once correct frames flow. *Code-complete;
      maintainer runs `test_delimiter_mismatch_reports_no_frames_extracted`.*
- [ ] **AC5 (R9)** — Same test file: sending frames with deliberately wrong checksums raises
      a checksum-failure-rate finding reporting a rate, and the finding clears after correct
      frames resume. *Code-complete; maintainer runs
      `test_checksum_failure_rate_reported_and_cleared`. The rate accumulates for the life of
      the reader, so the test clears it by reopening the link rather than by diluting the
      old total.*
- [ ] **AC6 (R10)** — Same test file: a per-dataset transform that always throws produces a
      finding carrying the error text and a failure count greater than one. *Code-complete;
      maintainer runs `test_failing_transform_reports_error_text_and_count`.*
- [x] **AC7 (R11)** — Same test file: the read command returns the same findings the panel
      shows, and the forced re-run verb refreshes them. The commands are listed in the
      assistant safety manifest's safe tier — asserted by a runnable static test under
      `tests/scripts/`. — the safety-tier half is green
      (`tests/scripts/test_problem_center_static.py`, 10 passed); the live half is
      `test_problems_run_refreshes_list`, which the maintainer runs.
- [ ] **AC8 (R5, R6, R7)** — Maintainer observation: the severity indicator shows counts and
      opens the panel; the command appears in the palette in both the main window and the
      project editor; activating a project finding selects the right entity in the editor
      tree; the notification center receives one summary per newly-appearing finding and
      nothing on a no-change re-run. *Pending maintainer observation.*
- [ ] **AC9 (R12)** — `--benchmark-hotpath` shows no regression against the pre-change run on
      all nine gated tiers (maintainer runs). *Pending.*
- [x] **AC10** — `python scripts/registry-verify.py` and `python scripts/code-verify.py
      --check` clean; `python scripts/sanitize-commit.py` runs without new lint debt. —
      `registry-verify: CLEAN`; `code-verify --check` 0 errors on the problem-center files
      (only the known `arch-singleton-instance` advisory ratchet);
      `generate-command-strings --check` up to date; `documentation-verify` 0 findings.
      `sanitize-commit.py` still owed at commit time.

## Constraints & Invariants

- **The composition root is pinned (spec 0001).** Adding a module to
  `instantiateCoreModules()` re-triggers the ctor-edge proof. The collector's constructor
  must be inert — it may not construct or reach through any other singleton — so the proof
  is re-run against a node with no outgoing constructor edges. All wiring happens after
  construction, in the existing post-construction wiring phase.
- **Must not regress the `--benchmark-hotpath` gates and must not add per-frame signal
  traffic.** Any new counter on the parse path is a plain in-place increment on data the
  owning object already touches; aggregation and evaluation happen on an existing periodic
  tick, never per frame. No new queued connection may fire at frame rate.
- **No new input to a cached hotpath flag.** Nothing in this feature may gate, or be gated
  by, `m_operationMode`, `m_playerOpen`, `m_anyAsyncSink`, `m_captureLatestFrame`,
  `m_changeDriven`, or Dashboard `m_streamAvailable`.
- **`FrameReader` and `CircularBuffer` stay main-thread / SPSC. No mutexes.** A counter read
  from another thread must use the relaxed-atomic pattern the ring buffer's overflow counter
  already uses.
- **The problem center never mutates the project document.** It holds no reference that can
  write, and it is not a mutation site for spec 0031's undo scopes.
- **Project-file bytes are unchanged.** No new JSON keys, no schema version bump, no
  migration.
- **GPL builds get the whole feature.** No `BUILD_COMMERCIAL` gating on the collector, the
  model, the panel, the command, or the API handler. A checker that inspects a Pro-only
  subsystem is compiled out with that subsystem, not the feature.
- **Operation modes.** The feature must behave correctly in QuickPlot and Console-only modes:
  project checkers report nothing when there is no project document, and link checkers still
  work.
- **Spec 0028 is the only way to add the command** — one manifest entry plus a binding per
  context, gated by `registry-verify.py`. No hardcoded icon paths.
- **No new third-party dependency** (roadmap constraint from spec 0030).
- **Translatable.** Every user-visible title, explanation, and remedy is a translatable
  string, not a formatted log line.

## Open Questions

- **Panel shape.** A standalone window opened by a command (reachable identically from the
  main window and the project editor, GPL-clean) versus a dockable bottom pane in the project
  editor versus a dashboard tool window like the notification log (which is Pro-gated today).
  The plan phase recommends one; the maintainer picks.
- **Sampling interval and thresholds.** The link checkers need a sample window and trigger
  thresholds (checksum failure percentage, how many seconds of "bytes but no frames" before
  reporting). Defaults proposed in the plan; are they user-configurable in v1 or fixed?
- **Notification volume.** Should every newly-appearing finding post a notification, or only
  error-severity ones? A project with fifteen warnings would otherwise post fifteen events on
  load.
- **Duplicate frame indices: error or warning?** Two datasets legitimately share an index
  when the same value is shown by two widgets. The check must distinguish "same index, same
  source, both non-virtual, and the second one is likely a mistake" from the intentional case
  — or downgrade to information. Maintainer input decides the default severity.
- **Scope name.** `problems.*` (matches the roadmap wording and the panel name) versus
  `diagnostics.*` (matches roadmap item R9's naming, which deliberately avoids colliding with
  the licensing self-test). Picking one now avoids a rename when R9 lands.
- **Does R9 want its own scope?** If connection self-test results are findings in this
  collector, they are readable through this spec's commands and R9 adds no API surface. Is
  that the intent?
