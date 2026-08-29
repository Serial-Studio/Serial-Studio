---
spec: 0044-session-reproducibility
title: Session Reproducibility Verification
status: done         # draft -> approved -> in-progress -> done | shelved
# Implementation complete 2026-08-05; acceptance criteria await the maintainer's pytest +
# --benchmark-hotpath runs (AC checkboxes below stay open until then).
created: 2026-08-05
author: Alex Spataru
---

# Spec 0044 — Session Reproducibility Verification

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The Session Database is pitched on one sentence: it stores the raw device bytes *and* the
project configuration that interpreted them, so any archived session can be regenerated and
audited later. Today that is a promise, not a demonstrated invariant. Nothing in the product
ever re-derives the processed values from the archived raw bytes and compares them against
what was recorded at capture time. If a parser change, a transform change, a locale/formatting
change, or a numeric regression silently alters what the same bytes produce, no user finds out
until an auditor does — and for the regulated/industrial audience (test benches, calibration
labs, flight-test telemetry) that silent drift is exactly the failure mode they buy tooling to
avoid.

The gap is verifiable today: an archived session carries raw bytes, the recorded
raw-and-final values per dataset, and the full project JSON, which is everything needed to
replay interpretation from scratch — yet replay deliberately reads the recorded *final*
values (transforms cannot re-run because their live inputs no longer exist). So the one path
that could prove reproducibility is never exercised. A built-in, mechanical regression check
turns "we trust it" into "we can show it", and is worth more to this audience than any new
widget.

## Goals

- A user can select an archived session and run a **verification pass** that re-interprets
  the archived raw bytes using the archived project configuration on the current build.
- The pass produces a clear verdict: **reproduced** (regenerated values match the recorded
  values), **diverged** (they do not, with the first divergences shown: dataset, timestamp,
  recorded vs regenerated value), or **not mechanically verifiable** (the session depends on
  inputs that were not captured, with the specific reason named).
- New sessions record enough fingerprint material at capture time (content hashes of the
  recorded streams, app version, format version) that a later verification pass can state
  *what* changed, not merely *that* something changed.
- The verdict is durable: verification results can be stored with the session and re-checked
  later, so a lab can demonstrate "session X, captured under version A, still reproduces
  under version B" without keeping old binaries around.
- Honesty over green checkmarks: sessions whose processing pipeline depended on
  uncaptured live state (data-table-fed transforms, control-script interaction, device
  round-trips) are reported as such — the check never claims reproducibility it did not
  mechanically establish.

## Non-Goals

- **Not a determinism guarantee for live capture.** The check proves that stored raw bytes +
  stored config regenerate the stored outputs; it says nothing about timing, sampling, or the
  physical measurement chain, and the docs around it must say so plainly (adjacent to, but
  distinct from, the "not a safety function" disclaimer work).
- **Not schema freezing / format documentation.** Publishing a versioned spec of the
  project-file and session-database formats with migration rules is its own follow-up spec;
  this spec only *consumes* a format-version stamp if one exists and records app/format
  versions in new sessions.
- **Not calibration records.** Calibration as a first-class object is a separate follow-up
  spec.
- **Not retroactive magic for legacy sessions.** Sessions captured before this feature lack
  fingerprints; they get best-effort verification (value comparison only) or an honest
  "insufficient capture" verdict — no fabricated confidence.
- **Not a change to live capture cost.** No per-frame work is added to the capture path
  beyond what fingerprinting strictly needs, and nothing touches the dashboard hotpath.
- **Not CSV/MDF4 export verification.** Those are one-way exports; scope is the session
  database only.

## Requirements

1. **R1 — On-demand verification.** From the session browsing UI, the user can invoke
   "Verify reproducibility" on any completed archived session. The pass runs offline (no
   device connection) against the current build and must not disturb a live capture or an
   open replay.
2. **R2 — Re-interpretation from raw.** Verification re-runs frame extraction and parsing
   over the archived raw byte stream using the archived project configuration — the same
   interpretation engine the live path uses, not a parallel reimplementation — and produces
   regenerated per-dataset values.
3. **R3 — Comparison and verdict.** Regenerated values are compared against the recorded
   values. Numeric comparison is bit-exact on the stored numeric representation; text values
   compare exactly. The session verdict is *reproduced* only when every comparable reading
   matches; otherwise *diverged*.
4. **R4 — Divergence reporting.** A diverged verdict names, at minimum: the affected
   dataset(s), the count of mismatching readings per dataset, the first mismatch per dataset
   (timestamp, recorded value, regenerated value), and the interpretation stage that produced
   the value (parse vs transform), so the user can localize the cause.
5. **R5 — Reproducibility classification at capture.** Each new session records whether its
   processing pipeline was mechanically self-contained. Sessions that used features whose
   inputs are not archived (per-dataset transforms fed by live data tables, control scripts
   that mutate parsing state, or any interpretation input not stored in the session) are
   marked, per feature, as not mechanically verifiable — and verification reports that
   classification instead of a false verdict.
6. **R6 — Capture-time fingerprints.** Each new session stores integrity fingerprints
   (content hashes) of the raw byte stream and the recorded values, plus the application
   version and a session-format version. Verification uses them to distinguish "archive
   changed since capture" from "current build interprets the same archive differently".
7. **R7 — Durable verification record.** Each verification run appends a stored record
   (verified-at time, verifying app version, verdict, divergence summary) to the session, and
   the session UI shows the latest verdict.
8. **R8 — Legacy sessions.** Sessions predating this feature verify best-effort: raw bytes
   are re-interpreted and compared where recorded values exist, and the verdict is qualified
   as legacy (no fingerprints, classification unknown) rather than refused outright.
9. **R9 — Scriptable.** The verification pass is invocable through the existing external
   automation surface, returning the same verdict and divergence detail as the UI, so a lab
   can gate its own CI on "all archived sessions still reproduce".

## Acceptance Criteria

- [x] **AC1** — Capture a session from a synthetic data source (Native and JS parser
  variants), close it, run verification on the same build: verdict is *reproduced*, zero
  divergences. (pytest integration test via the API surface, R1/R2/R3/R9.)
- [x] **AC2** — Tamper with one recorded reading in a copy of the archive: verification
  reports *diverged*, names the dataset, count 1, and the exact recorded/regenerated pair —
  and the fingerprint check attributes it to archive modification. (pytest, R3/R4/R6.)
- [x] **AC3** — Alter the archived project configuration copy (e.g. change a transform
  constant): verification reports *diverged* and attributes the divergence to
  interpretation, not archive damage. (pytest, R4/R6.)
- [x] **AC4** — Capture a session using a data-table-fed transform: session is marked not
  mechanically verifiable with that reason; verification returns the classification, not
  *reproduced*. (pytest, R5.)
- [x] **AC5** — A pre-0044 session file verifies with a qualified legacy verdict and no
  crash or refusal. (pytest against a checked-in legacy fixture, R8.)
- [x] **AC6** — Verification while a live capture is running neither blocks the capture nor
  corrupts either database; the maintainer confirms live dashboard behavior is unaffected.
  (maintainer observation + destructive-marked pytest, R1.)
- [x] **AC7** — Verdict and divergence summary persist across app restart and re-display in
  the session UI. (maintainer observation, R7.)
- [x] **AC8** — `--benchmark-hotpath` gate unchanged: capture-path additions cost nothing
  per-frame on the dashboard path. (CI gate, Constraints.)

## Constraints & Invariants

- **The deciding constraint: verification must reuse the real interpretation pipeline.** A
  second implementation "for checking" would itself drift; divergence detected by the check
  must be divergence a live user would see. Corollary: verification runs offline against the
  current build's engine, so it must tolerate every archived configuration the app can load.
- **Honest verdicts outrank complete coverage.** Any session the check cannot mechanically
  re-derive is classified, never approximated. No tolerance windows, no "close enough"
  numeric fuzz — the product claim is bit-stability of the stored representation (the build
  already pins IEEE-stable math as an invariant).
- **No hotpath regression.** Nothing per-frame is added to the dashboard path; capture-side
  fingerprinting must respect the existing capture architecture (no per-frame allocation,
  locking, or signaling on the parse path) and the 256 kHz CI gate.
- **Replay semantics unchanged.** Existing session replay continues to read recorded final
  values; verification is a separate pass and must not alter replay behavior or re-record
  anything.
- **Archives are read-only evidence.** Verification never mutates recorded session data;
  the only writes are appended verification records and capture-time fingerprints for new
  sessions.
- **Old databases keep opening.** Schema additions must be backward compatible with existing
  session databases (additive migration, consistent with current practice); a pre-0044
  archive must never be rendered unreadable.
- **Pro feature.** The Session Database is Pro; verification ships under the same gate, with
  the same trial-equals-Pro behavior.
- **Truth in labeling.** User-facing text states what the check proves and what it does not
  (no determinism guarantee, not a safety function, not a calibration authority).

## Open Questions

- **Q1 — Verdict granularity for mixed sessions.** A multi-source session where one source is
  table-fed and the others are self-contained: one session-level classification, or
  per-source/per-dataset verdicts? Recommendation: per-dataset classification rolled up to a
  session verdict ("reproduced except N datasets not verifiable"), since all-or-nothing
  wastes the majority of the evidence.
- **Q2 — Raw-stream fidelity for frame extraction.** Recorded raw bytes are chunked as
  received; re-extraction must yield the same frame boundaries the live session saw.
  Is byte-stream concatenation per device sufficient ground truth for all frame-detection
  modes, or do any modes depend on arrival timing that the archive does not capture? If any
  do, those sessions belong in the R5 classification. Needs a maintainer answer before plan.
- **Q3 — Where results surface beyond the session UI.** Recommendation: session UI +
  automation API only for this spec; a printable/exportable verification report can ride the
  existing report tooling later.
- **Q4 — Fingerprint algorithm and stored form** (deliberately deferred to plan; the spec
  requirement is only: content-addressed, stable across platforms, cheap enough for capture).
- **Q5 — Bulk verification.** "Verify all sessions in this database" in one action: in scope
  now or a follow-up? Recommendation: follow-up; R9 already lets a script loop.
