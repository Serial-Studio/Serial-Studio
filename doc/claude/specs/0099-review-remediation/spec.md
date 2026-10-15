---
spec: 0099-review-remediation
title: Remediation of the 2026-10-13 app-level review findings
status: in-progress  # draft -> approved -> in-progress -> done | shelved
created: 2026-10-14
author: Alex Spataru
---

# Spec 0099 — Remediation of the 2026-10-13 app-level review findings

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no class names or signal
> wiring (that is `plan.md`). Findings are named by their review id and by the behavior a user
> sees; the per-finding file/line detail lives in the `findings.md` ledger the plan phase
> creates. Gate: do not start `/ss-plan` until a human marks this `approved`.

> Evidence: the 2026-10-13 app-level review of db2debec4 (five slice reviewers, an adversarial
> verifier per slice, and a hand re-check of the critical findings). The review returned 53
> findings. Six, the consent and trust-boundary class, are spec 0098. The remaining 47 are this
> spec. All were confirmed against the code and still hold at 51e00a1e5.

## Problem / Motivation

A whole-app review found 47 defects outside the consent class: 1 critical, 8 high, 23 medium and
15 low, spread across the pipeline, the device and protocol drivers, recording and export, the
API server, the app services and the QML editors. They are real bugs, not style: a connect that
crashes, telemetry that records as empty, values that display stale, a restore that rolls back to
the wrong snapshot, edits that land on the wrong dataset. None is a feature request; each is a
case where the app already promises a behavior and does not keep it.

Left in a report, a backlog like this decays: the findings drift against the code, and the ones
that never get a line number again are never fixed. The deciding constraint: **the review's
findings become a tracked, verified ledger, and every fix is tied to the id it closes.** This
spec packages the 47 as one remediation, in the style of spec 0075: a `findings.md` ledger holds
each id, its location and its verification, and every requirement below cites the ids it closes.

The fixes are grouped into seven themes by subsystem and defect type, so a theme can
be planned, implemented and verified as one coherent change and reviewed on its own. Severity
orders the work: the critical and high findings (themes A–D lead) land before the low-severity
polish (theme G).

## Goals

- Every one of the 47 findings is either fixed and verified, or explicitly deferred with a
  recorded reason, and the `findings.md` ledger records the outcome of each by id.
- Each fix restores an already-promised behavior without changing a public contract: no new API
  command, no project-format change, no new user setting, no new dependency.
- Data-integrity findings (themes A, C, D) come with a regression test that fails on the old code
  and passes on the fixed code, so the defect cannot silently return.
- The acquisition hotpath keeps its 256 kHz gate, and no fix adds work to the per-frame path.

## Non-Goals

- **The six consent / trust-boundary findings** (api-storage f1–f4, f7; ui-app f1). Those are
  spec 0098 and are not touched here.
- **Mirror link authentication or encryption.** Out of scope, as in 0098.
- **Redesigning any subsystem.** Each fix is the smallest change that makes the promised behavior
  hold; a finding that would need a redesign is deferred with that noted, not expanded here.
- **New capabilities.** No new widget, driver, protocol feature, export format or API surface.
- **The review's own tooling.** The scratch clone, the report and the generator stay outside the
  repo.

## Requirements

Each requirement names the behavior to restore and cites the finding ids it closes. A fix is
complete only when its ids are marked resolved in `findings.md` with their verification.

### Theme A — Pipeline and buffer correctness

Silent data corruption and races on and beside the acquisition path. Highest value; every fix
here carries a regression test.

1. **R1** — A ring buffer distinguishes full from empty, and an overflow drops exactly the bytes
   it reports, keeping per-chunk timing aligned. No input that exactly fills or exceeds the ring
   is silently discarded. *(pipeline-core:f2, pipeline-core:f1)*
2. **R2** — The span fast lane writes parsed values back before any republish stages them, so a
   dashboard tick, a reprocess, or a second stream-lane source never republishes stale or initial
   values for a span-lane source. *(pipeline-core:f5)*
3. **R3** — The interned-key lookup cache cannot return a wrong slot after a script builds register
   names at run time and a garbage collection reuses a freed string address. *(pipeline-core:f4)*
4. **R4** — A stream worker's script engines are never torn down while a table-API marshal's nested
   event loop is still on the worker's stack. *(pipeline-core:f3)*
5. **R5** — A project sync requested while another is in flight is applied, not dropped, so the
   pipeline never keeps a stale project. *(pipeline-core:f7)*
6. **R6** — The GUI-thread frame-parser entry points do not read or mutate pipeline-owned engine
   or language state without marshaling, and a language change is never lost to the race.
   *(pipeline-core:f6)*
7. **R7** — A GUI-side table-API marshal never runs inline against a running pipeline through a
   stale "parked" flag. *(pipeline-core:f8)*

### Theme B — Device and protocol drivers

The connect path and the wire protocols. Leads with the review's one critical finding.

8. **R8** — A failed driver dial never dereferences a device the same failure already tore down;
   a busy, denied or unplugged Modbus RTU port reports a clean connect failure instead of
   crashing. *(devices-protocols:f1, critical)*
9. **R9** — Cancelling a blocking PLC dial (S7, EtherNet/IP) does not freeze the GUI thread until
   the worker's dial times out. *(devices-protocols:f5)*
10. **R10** — CAN error frames are surfaced as errors, not published as ordinary data frames on id
    0x000. *(devices-protocols:f3)*
11. **R11** — The CAN backends read their adapter correctly: the serial-CAN open acknowledgement is
    seen, and gs_usb RX reassembly handles variable-length host frames without desyncing.
    *(devices-protocols:f4, devices-protocols:f6)*
12. **R12** — A CAN bit-timing solve never calls a clamp with an inverted range when firmware
    reports degenerate segment limits. *(devices-protocols:f9)*
13. **R13** — Large file transfers over a slow link succeed: the ZMODEM sender applies backpressure
    and times its end-of-file from send time, not queue time. *(devices-protocols:f2)*
14. **R14** — ZMODEM verifies the CRC of received headers before acting on them.
    *(devices-protocols:f8)*
15. **R15** — IEC 60870-5-104 acknowledges received I-frames promptly enough that a station is not
    capped at k APDUs per tick. *(devices-protocols:f7)*
16. **R16** — Audio playback underrun pads with true silence for every sample format, including
    unsigned 8-bit. *(devices-protocols:f10)*

### Theme C — Recording, export and session I/O

Findings where telemetry is lost or a file is written wrong.

17. **R17** — Recording resumes after a pause: a CSV (and any session recording that clears its
    structure on close) reopens and keeps recording instead of silently dropping every block after
    resume. *(api-storage:f6, high)*
18. **R18** — A write failure during recording stays reported; a queued open notification never
    resets the failure latch the worker already set. *(api-storage:f11)*
19. **R19** — The Historian's "Export Session to CSV" escapes headers and text values and reports a
    write error instead of ignoring it. *(api-storage:f10)*
20. **R20** — The CSV player backfills row 0 and steps back to it without tripping an assertion.
    *(api-storage:f12)*

### Theme D — API server robustness

Findings in the socket server's framing, flow control and teardown.

21. **R21** — A stream subscriber that stops reading cannot grow the server's socket buffer without
    bound: the stream lane acknowledges a block only once it has been sent, and a
    backlogged subscriber is capped or dropped like every other lane. *(api-storage:f5, high)*
22. **R22** — A newline-less JSON command split across TCP reads is reassembled, not judged complete
    at an inner brace and its tail sent to the device as raw bytes. *(api-storage:f8)*
23. **R23** — Tearing down the server's connections does not skip or orphan a socket by editing the
    socket set while iterating it, and bulk teardown runs the same per-connection cleanup (MCP
    session, stream flag, mirror subscriber) as a single disconnect. *(api-storage:f9,
    api-storage:f14)*
24. **R24** — A `dryRun: true` flag on a command that does not honor it does not skip the checkpoint,
    autosave and re-sync while the change still happens; such a command either honors the flag or
    ignores it without dropping its side-effect bookkeeping. *(api-storage:f13)*

### Theme E — App services

Findings in the backup, extension, console-export and sandbox services.

25. **R25** — The backup list is newest-first, so recovery and checkpoint restore preselect and
    resolve the most recent snapshot, and `assistant.listCheckpoints` returns the most recent N as
    documented. *(ui-app:f2, high)*
26. **R26** — Uninstalling an extension deletes only the recorded installed location, never a path
    built from an unvalidated catalog field. *(ui-app:f3)*
27. **R27** — A display-tick reconfigure does not spin a nested event loop mid-teardown, and a
    marshal's result is not silently thrown away. *(ui-app:f6)*
28. **R28** — Worker threads do not read GUI-owned state that the GUI thread can reassign: the
    console-export folder and the AI file sandbox resolve their path safely, and the console folder
    sanitizes the project title. *(ui-app:f4, ui-app:f5)*

### Theme F — Editor and dialog correctness (QML)

QML findings that lose or misdirect a user's edit.

29. **R29** — A non-modal editor commits to the dataset it was opened for, never to whatever is
    selected when Apply is clicked. *(qml:f1, high)*
30. **R30** — Decimal input is parsed consistently with how it is validated, so a comma-decimal
    locale does not truncate or drop a fractional value. *(qml:f2, high)*
31. **R31** — The last field edit is not lost when Apply is clicked: an editor table commits the
    field in progress. *(qml:f8)*
32. **R32** — An in-progress edit is preserved when the selection changes or the list refreshes:
    Historian session notes and a workspace dialog's checkmarks are not discarded or shown stale.
    *(qml:f5, qml:f4)*
33. **R33** — A numeric field commits a value only when the user has entered a complete one, not NaN
    or a truncated number on every keystroke. *(qml:f6)*
34. **R34** — A tree context menu acts on the right-clicked row, not the currently selected group.
    *(qml:f3)*
35. **R35** — The annotation CSV export builds a valid path on every platform, Windows included.
    *(qml:f7)*

### Theme G — QML polish and platform (low severity)

The lowest-severity findings: motion preferences, platform path quirks, and small field-commit
bugs. This theme may be deferred as a unit (see Open Questions) without blocking A–F.

36. **R36** — Clearing a float field commits nothing rather than silently committing 0, and a
    variable rename does not commit per keystroke or drop an edit on a transient name collision.
    *(qml:f12, qml:f13)*
37. **R37** — The "Reduce Motion" preference is honored by the dashboard's slide and scale
    animations, and chrome autohide/collapse does not animate laid-out heights every frame.
    *(qml:f10, qml:f9)*
38. **R38** — Terminal hex-mode validation does not permanently overwrite the send field's themed
    text color. *(qml:f11)*
39. **R39** — Local-repository extension assets and the OPC UA tag browser render correctly: file
    URLs are well formed on Windows, and a tag-search hide re-measures row height.
    *(qml:f14, qml:f15)*

## Acceptance Criteria

Each criterion ties to a real check. The data-integrity themes get regression tests that fail on
the current code first.

- [ ] **AC1** (Theme A) — Unit tier: a ring-buffer test exercises the exact-fill and oversized-chunk
      cases (R1); a span-lane test shows a dashboard tick republishes the just-parsed values, not
      the template (R2); an interned-key-cache test survives a GC address reuse (R3). Each fails on
      db2debec4 and passes after.
- [ ] **AC2** (Theme A) — The stream-worker teardown (R4), the dropped-sync (R5) and the parked-flag
      (R7) races are covered by a unit or ctest that is deterministic under the sanitizer tier.
- [ ] **AC3** (Theme B) — Unit tier: the Modbus re-entrant-failure path returns a clean failure and
      does not dereference the torn-down device (R8); the bit-timing solve with degenerate limits
      does not abort under `_GLIBCXX_ASSERTIONS` (R12); gs_usb reassembly handles a variable-length
      frame (R11). Maintainer observation for the connect-path and PLC-cancel findings (R8, R9).
- [ ] **AC4** (Theme B) — The protocol fixes (R10, R13, R14, R15, R16) are covered by the JS/Python
      protocol tests where one exists, and by a maintainer observation on real hardware where one
      does not; the `findings.md` entry records which.
- [ ] **AC5** (Theme C) — Integration: a pause/resume cycle on a CSV recording produces a file with
      the post-resume blocks (R17); a Historian CSV export of a session with delimiter and quote
      characters round-trips through a CSV reader (R19); the CSV player steps to and from row 0
      without an assertion (R20). A forced write error stays reported (R18).
- [ ] **AC6** (Theme D) — Integration / unit: a stream subscriber that stops reading does not grow
      the server's memory without bound and is dropped or capped (R21); a command split across two
      TCP writes is reassembled and never reaches the device as raw bytes (R22); a connection
      teardown under load orphans no socket and runs full per-connection cleanup (R23).
- [ ] **AC7** (Theme E) — Unit / observation: the backup list returns newest-first and restore
      picks the latest snapshot (R25); an extension uninstall never deletes outside the recorded
      install path (R26). The cross-thread reads (R28) are clean under the thread sanitizer.
- [ ] **AC8** (Theme F) — Maintainer observation, scripted where possible: an open editor Applies to
      its own dataset after the selection changes (R29); a comma-decimal locale keeps a fractional
      value (R30); the last field edit survives Apply (R31); session notes and workspace checkmarks
      survive a refresh (R32); the context menu acts on the right-clicked row (R34).
- [ ] **AC9** (Theme G) — Maintainer observation: the low-severity polish findings each show the
      promised behavior, or the theme is recorded as deferred in `findings.md` with a reason.
- [x] **AC10** — `findings.md` lists all 47 ids; 44 `fixed` (with verification; f5-PLC fixed partial
      with its remainder deferred) and 3 `deferred` with reasons (pipeline-core:f5 span-lane,
      devices-protocols:f2 ZMODEM backpressure, devices-protocols:f6 gs_usb). None unaccounted.
- [ ] **AC11** — `--benchmark-hotpath` passes its gates unchanged, and `pytest tests/integration/`
      is as green as before. **MAINTAINER-GATED: cannot build/benchmark/run the app here.**

> **Status note (2026-10-14):** the implementable scope has landed and is static-clean
> (code-verify 0 errors across 73 files, syntax-check per TU, layer/claim/documentation verifiers
> clean). AC1–AC9 and AC11 are maintainer-gated (ctest / `pytest tests/integration/` /
> `--benchmark-hotpath` / in-app observation) and cannot be confirmed in this environment, so the
> spec stays `in-progress` until the maintainer runs them. The run list and the census `--accept`
> decision are in `tasks.md`.

## Constraints & Invariants

- **The ledger is the contract.** `findings.md` carries every id, its location, its fix and its
  verification, as spec 0075 did. A requirement is done when its ids are resolved there.
- **Smallest change that keeps the promise.** Each fix restores an existing behavior; none changes
  a public contract, the project format, the API surface or a user-facing default.
- **Themes land and review independently.** A theme is one coherent change, reviewed on its own.
  Severity orders them: critical and high first (A–D lead), the low polish (G) last.
- **The hotpath is sacred.** Theme A fixes sit on or beside the acquisition path; none adds
  per-frame work, allocation, a lock on `FrameReader`/`CircularBuffer`, or a queued per-frame hop.
  The 256 kHz gate holds (AC11).
- **Threading rules hold.** The race fixes (R4, R6, R7, R27, R28) obey the existing marshal and
  SPSC contracts; no new `BlockingQueuedConnection` from the GUI into the pipeline.
- **No regression in scope.** A data-integrity fix lands with a test that failed on the old code.
- **Protected surfaces.** Any finding that touches licensing, the API auth/consent files, or the
  hotpath is named with its failure modes in chat before it lands and never rides inside a bulk
  package, exactly as the trust contract requires. (The backup-list fix, R25, is adjacent to the
  0098 checkpoint work and is sequenced after it.)
- **No new third-party dependency, and every fix applies in every build where the code exists,
  the GPL build included.**
- **Trial parity.** Any doc touched keeps the trial-parity rule.

## Open Questions

- **Is Theme G included in this spec, or deferred?** The 15 low-severity findings are real but cosmetic
  or platform-edge. Recommendation: keep them in 0099 but as the last milestone, and allow the plan
  to mark the whole theme deferred if the critical-through-high work fills the budget. The ledger
  records whichever way it goes.
- **How far does R9 go?** Cancelling a blocking PLC dial cleanly may be a small fix (a cancel flag
  the worker checks) or may need the dial to become interruptible. The plan should size it from the
  code and, if it needs more than a localized change, split it out rather than expand this spec.
- **R15 (IEC 104 ack cadence) touches a timing constant on a read-only telecontrol client.** The
  plan should confirm the change stays within the protocol's window rules and needs no new setting.
