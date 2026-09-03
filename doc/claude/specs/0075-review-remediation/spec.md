---
spec: 0075-review-remediation
title: Remediate the 2026-09-01 full source review
status: in-progress   # draft -> approved -> in-progress -> done | shelved
created: 2026-09-01
author: Alex Spataru
---

# Spec 0075 — Remediate the 2026-09-01 full source review

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

> Evidence lives in `findings.md` beside this file: 196 numbered findings (A1..M12) with
> location, confidence and failure scenario, produced by twelve independent per-subsystem
> reviewers reading each area's architecture doc and then its code in full. Requirements
> below cite finding ids instead of code so this document stays implementation-free; the
> ledger is the ground truth the plan anchors to. Scope decision (2026-09-01): one umbrella
> spec covering defects, hazards, CI integrity, structural debt, doc drift and the test tiers
> that pin them.

## Problem / Motivation

The 4.1.0 tree passes its own linter almost clean (ten advisories, all file-length), yet a
full semantic review found real defects the style contract cannot see. Some lose data
silently: every CSV and MDF4 recording drops or misfiles its last display tick on disconnect,
two serial sources recorded together get one source's timestamps rewritten into a
nanosecond staircase behind the other's, a full disk mid-session leaves the Historian showing
"recording" while every later batch is discarded, and one Settings change writes unsaved
project edits to disk behind the user's back. Some hang or crash: a runaway JavaScript
transform on a stream source, a Validate click in three editors, or a hostile API dry-run
request all freeze the application with no watchdog, a modal prompt raised from inside the
API's receive path can resume on freed memory, and a copy from the terminal after a clear can
index past the buffer. Some lie about state: two CAN adapters stay "connected" after unplug,
an audio capture session dies when the headset is removed, a paused session silently resumes
after a serial blip, a Sparkplug reconnect renumbers node slots so one node's metrics render
under another's name, and the CLI reports a successful deactivation while the seat stays
consumed. Two API paths let a browser page or an unguarded file argument reach authenticated
commands, extension packages install with no integrity check and record partial downloads as
complete, and the release pipeline republishes the continuous build from any branch without
tests ever gating it, using tag-pinned third-party actions that hold the code-signing
identity.

Underneath the defects is structural debt that manufactured several of them: two serial CAN
backends, two PLC drivers, four LLM providers, three replay players, three exporters and five
code-editor hosts are near-clones, so a fix in one never reaches its twin; roughly fifteen
statements in the AI-facing architecture docs no longer match the code; and whole tiers
(QML, fuzzing, cross-platform unit tests, sanitizers) do not exist, which is why the above
shipped. This spec turns that review into a contract: every verified defect fixed and pinned
by a test, every decision recorded, every structural cause addressed or explicitly deferred.

## Goals

- A recording (CSV, MDF4, Historian) contains exactly the samples the sources produced, with
  each source's own timestamps, through connect, pause, disconnect and disk errors.
- No user-supplied script, in any lane or editor, can freeze the application; every script
  execution surface reports a timeout instead.
- Every link state the toolbar shows is true: a dead adapter, a removed device or a cancelled
  dial never leaves "connected" or "connecting" lit, and reconnecting never changes the
  session's pause state.
- The project document on disk changes only when the user saves, auto-save runs on a modified
  document, or the user explicitly confirms; undo covers every editor action as one step.
- The local API cannot be driven by a browser page, cannot be pointed at arbitrary files, and
  cannot be hung or crashed by one client.
- Extensions and assistant edits reach disk only through verified, user-visible steps.
- The Linux release carries the hardening it claims; releases are published only from the
  default branch after tests pass; third-party actions holding secrets are immutable.
- Every duplicated driver, player, exporter, provider and editor pair shares one
  implementation, so the next fix lands in both by construction.
- The AI-facing docs describe the code as it is, verified by the existing claim gate.
- Every defect above is pinned by a test that fails on the pre-fix tree, and the tiers that
  were missing (QML lint, fuzz, cross-OS unit tests, sanitizer) exist and run in CI.

## Non-Goals

- Replacing the settings-file credential store with an OS keychain (shelved decision; this
  spec only forbids calling the current store "encrypted").
- Signing extension catalogs with a publisher key (per-file digests plus atomic install now;
  signing is a later spec once third-party repos have a key-management story).
- Requiring an auth token from loopback API clients (decision: reject HTTP-shaped input
  before auth and keep loopback auto-auth so existing local scripts keep working).
- Reducing the count of application singletons or preparing for a second session; this spec
  freezes the census and documents the cached-reference idiom, nothing more.
- Redesigning the Audio driver's playback lane beyond making it carry continuous output.
- Multi-session, new drivers, new widgets, new UI, or any feature work.
- Rewriting the CI workflow into reusable workflows; this spec fixes its integrity gaps and
  removes duplication only where a gap forces the touch.
- Accessibility (screen-reader roles, focus policy) across the QML tree: recorded as a
  follow-up, out of scope here.

## Requirements

Grouped by outcome. Each group names the findings it closes; the plan maps every cited id to
at least one task and lists any id it leaves open, with the reason.

### R1 — Recording fidelity (A2, A11, B1, B2, B3, B4, B14, B18, E7)

1. **R1.1** — Disconnecting or pausing a source delivers every sample already parsed to
   every active sink before the sink closes; no sink receives samples after it closed and
   no sink creates a second file for the tail of a session.
2. **R1.2** — Each recorded sample carries the timestamp its source assigned; a sink never
   rewrites a frame-lane timestamp, and two sources recorded together keep independent time
   columns.
3. **R1.3** — A sink that cannot write (disk full, I/O error, failed transaction) surfaces
   the failure to the user within one display tick, stops reporting itself as recording, and
   counts the dropped samples; it never finalizes a session over rows that were not written.
4. **R1.4** — The Historian's raw-byte lane keeps up with the raw ingress rate the frame
   lane sustains; any overrun is counted and shown, never silent.
5. **R1.5** — The session currently being recorded cannot be deleted or edited from the
   explorer; the action is refused with a reason.
6. **R1.6** — A synthetic republish emits only the datasets it exists for (table-fed), never
   duplicate samples of live channels into any sink.
7. **R1.7** — The InfluxDB line-protocol encoder escapes every character the wire format
   reserves in every position, so a dataset title cannot cause a batch to be rejected.
8. **R1.8** — Raw bytes captured before the first parsed block carry real capture timestamps,
   not a fabricated ramp.

### R2 — Script execution never hangs the application (A1, H2, I4, J3)

1. **R2.1** — Every place a user- or model-supplied script runs (frame parser, dataset
   transform in both lanes, control script, output widget, painter, editor Validate/Test/Apply,
   API dry-run) enforces the same execution deadline; on expiry the caller receives a timeout
   error and the application remains responsive.
2. **R2.2** — A stream-lane transform that times out reports the timeout on the source's
   diagnostics and falls back to raw values, as the scripting doc already promises.
3. **R2.3** — Assistant tools that read or search files run off the GUI thread and can be
   cancelled; the display tick keeps running while they work.

### R3 — Memory safety and crash freedom (A3, A4, A5, B6, B7, B8, E1, F2, F10, I1, I5, J4, K4)

1. **R3.1** — No pipeline-thread code reads state owned by the GUI thread without going
   through a cached flag or the marshal rules; the two known per-frame cross-thread reads
   (player-open probe, MQTT publisher topic) go through cached, atomically updated state.
2. **R3.2** — A modal prompt raised while servicing an API client cannot outlive the
   connection state it was raised for; the receive path holds no reference into the
   connection table across any call that can spin an event loop.
3. **R3.3** — A project synchronisation requested while the pipeline is parked inside a
   script call is applied once the pipeline resumes, never dropped.
4. **R3.4** — Terminal copy, select and clear are bounds-safe in every order, including
   clear-then-copy with a stale selection and ANSI erase sequences that shrink the buffer.
5. **R3.5** — Replay loaders for MDF4 and Historian archives use memory proportional to the
   columns being replayed, not to channels-times-instants; a ten-minute 48 kHz stream-lane
   recording replays on a machine with 4 GB free.
6. **R3.6** — A wire response from an S7 controller with a zero-length success item is
   rejected as a protocol error, never asserted on.
7. **R3.7** — Toggling the API off while a gRPC command is executing completes without
   deadlock.
8. **R3.8** — An assistant approve/deny click that arrives while a reply is still streaming
   is queued, never starts a second live reply.
9. **R3.9** — A startup failure after the composition root has run (for example the UI
   failing to load) shuts the session down in the pinned reverse order before exit.
10. **R3.10** — A sink whose worker falls behind is throttled by a bounded mechanism that
    performs no allocation on the publish path.

### R4 — The GUI thread never blocks on I/O (D6, D15, E5, E9, K11, K12, K13)

1. **R4.1** — No driver dial blocks the GUI thread: host-name resolution, connect, and
   protocol handshake for every network-backed driver (TCP, OPC UA, S7, EtherNet/IP,
   IEC 104, Modbus TCP, MQTT) run asynchronously with a bounded, user-visible timeout and
   a working Cancel.
2. **R4.2** — Closing a Process source, launching or stopping an extension plugin, and
   enumerating processes or hardware never stalls the GUI beyond one display tick.
3. **R4.3** — Machine identification at startup does not spawn synchronous helper
   processes on the GUI thread.
4. **R4.4** — No network reply handler raises a modal dialog from inside the reply's
   delivery; user-facing errors are queued to a safe point.

### R5 — Link state truthfulness (C1, C2, C3, C8, C11, C12, C13, D1, D2, D3, D4, D5, D7, D8,
D9, D10, D19, D20, E4, E6, E10, E12, E13, E15, E16)

1. **R5.1** — Every driver detects loss of its underlying device or port (serial CAN
   adapters, custom-path serial ports, USB, HID, BLE, network) and reports it through the
   one established-drop path, so the toolbar, the log and the quick-plot header agree.
2. **R5.2** — Every synchronous failure inside an open attempt, including internal re-dials,
   settles the pending verdict exactly once; a connect button never stays lit on a failed
   dial.
3. **R5.3** — Reconnecting a dropped device preserves the session's pause state.
4. **R5.4** — An audio capture session runs regardless of whether an output device is
   selected or present; removing an output device affects playback only. The playback lane
   carries continuous audio when written at its sample rate, and full-queue drops are counted.
5. **R5.5** — Applying a driver setting from a project file or the API never raises a modal
   dialog; settings that need consent are refused with a reason until the user grants it in
   the UI.
6. **R5.6** — Every driver property with a UI binding notifies on every change path,
   including project-driven application.
7. **R5.7** — Choosing the "no port" placeholder sticks; persisted port auto-selection runs
   once per enumeration change, not per tick.
8. **R5.8** — File transfers resend on NAK or timeout, count protocol errors independently of
   UI language, finish exactly once, and keep blank lines in plain-text mode.
9. **R5.9** — Cancelling a dial does not fire session-closed side effects for a link that
   never opened.
10. **R5.10** — Sparkplug: a host-application reconnect keeps every node's slot indices;
    an edge-node rebirth restarts its sequence at zero as the specification requires.
11. **R5.11** — Modbus: a skipped or failed poll cannot shift later frames onto the wrong
    register group; each frame identifies its group explicitly. Coils and discrete inputs
    honour their own per-request limit, and the RTU frame builder emits a valid checksum.
12. **R5.12** — OPC UA: a server offering only deprecated security policies is never dialed
    automatically; the trust action overrides self-signed hostname/expiry failures; write
    returns the same failure convention as sibling drivers; idle sessions do not wake the
    GUI at 100 Hz.
13. **R5.13** — MQTT driver: a CA path that is a directory is rejected with a reason; the
    credential vault is never written with an empty password; a read-only ACL does not
    silently drop rebirth commands.
14. **R5.14** — Serial CAN backends reject malformed frames instead of publishing them on
    ID 0, bound their receive buffer, and verify the adapter's open acknowledgement.

### R6 — Dashboard correctness and rendering cost (A8, F1, F3, F4, F5, F6, F7, F9, F15,
F17, F18, F19, G1, G2, G3, G10, K10)

1. **R6.1** — Every widget that consumes a source is fed from both the irregular and the
   uniform-grid lanes; a Samples-axis plot, multiplot or GPS widget driven by audio or a
   replayed stream group shows data.
2. **R6.2** — Changing the plot point count preserves each plot's sweep configuration,
   trigger state, retained segments and pause state.
3. **R6.3** — The Waterfall widget's per-tick GPU upload is proportional to the rows that
   changed, its colour mapping uses a lookup table or the shared SIMD kernels, and its
   overlay re-rasterizes only when axis, markers or theme change.
4. **R6.4** — Widgets that tabulate datasets rebuild their row structure on structure
   change and update values in place per tick, with no per-tick allocation proportional to
   widgets-times-datasets.
5. **R6.5** — A value displays the same text live, replayed and exported.
6. **R6.6** — Theme colours are exposed to QML so that one colour read costs one
   conversion, not a whole-palette conversion; every canvas that paints theme colours repaints
   on theme change.
7. **R6.7** — The console annotation track redraws only when its data or geometry changed.
8. **R6.8** — Reading a trial or license property never writes settings.
9. **R6.9** — Resizing a window that hits the canvas clamp applies the clamped rectangle
   instead of discarding the gesture; ANSI colour rows stay aligned with text rows across
   erase sequences with colour support toggled.

### R7 — Project document integrity (H1, H3, H4, H5, H6, H7, H9, H10, H11)

1. **R7.1** — The document on disk changes only through Save, Save As, auto-save of a
   modified document, or an explicit user confirmation; a display setting change never
   writes the document, and a locked document is never written.
2. **R7.2** — Every mutation reachable from the editor or the API marks the document
   modified, schedules auto-save, and is captured as exactly one undo step; a compound
   editor action (for example picking a parser template) undoes as one step.
3. **R7.3** — Bulk deletion removes every selected item regardless of nesting order.
4. **R7.4** — A failed reload after an external file change leaves the document attached to
   its path with its previous content, and tells the user why the reload failed.
5. **R7.5** — A new source's parser editor starts empty, never showing another source's
   script.
6. **R7.6** — Dashboard actions reflect an edit to their payload or interval immediately.
7. **R7.7** — Editing a delimiter field re-synchronises the pipeline at most once per idle
   interval, not per keystroke, and bulk group deletion regenerates auto-workspaces once.
8. **R7.8** — Derived dataset fields (source id) are normalised on every mutation path,
   including source deletion and API updates.

### R8 — API surface hardening (I2, I3, I6, I8, I9, I10, I12)

1. **R8.1** — A connection whose first bytes form an HTTP request line is closed before any
   command is parsed; before a JSON handshake completes, non-JSON input is never forwarded as
   raw device bytes. Loopback auto-authentication stays.
2. **R8.2** — Every command that accepts a file path resolves it through the single path
   policy; the policy is enforced at one choke point, not per handler, and a command that
   would create files outside the allowed roots is refused.
3. **R8.3** — Per-connection outbound buffering is capped on every lane; a client that stops
   reading is disconnected with a counted reason.
4. **R8.4** — gRPC and TCP enforce the same raw-write size cap, identify loopback peers by
   parsed address, and use the shared error-code enumeration for every failure, including
   authentication.
5. **R8.5** — The API accepts IPv6 loopback clients; the port is configurable through the
   same setting surface as the enable flag.
6. **R8.6** — Byte accounting counts each received byte once.

### R9 — Extension and assistant trust (F16, J1, J2, J6, J8, K3, K5, K6, K12, E14)

1. **R9.1** — Extension catalogs carry a digest for every file; an install verifies each
   file against it and is applied atomically: any mismatch or failed download leaves the
   previously installed version intact and reports the failure. Plain-HTTP repositories are
   refused.
2. **R9.2** — Update detection compares versions numerically; a lower remote version is not
   offered as an update. The install location is chosen by the local package type, never by
   a remote field alone.
3. **R9.3** — With auto-approve on, assistant edits land in the in-memory document and a
   checkpoint; the document on disk changes only on user Save or through an explicit
   Confirm-tier assistant tool. The toggle's copy states this.
4. **R9.4** — A local model provider exposes its context window as a user setting and history
   is trimmed to it, so the system prompt is never truncated away by the server.
5. **R9.5** — No log line contains any part of a stored secret; provider transports refuse
   redirects and plain HTTP except to loopback.
6. **R9.6** — No user-facing string or document describes the settings-file credential store
   as "encrypted"; no widget-extension surface is described as sandboxed.
7. **R9.7** — Stream parse-error handling is the same across providers: recoverable frame
   errors are skipped, fatal ones end the turn.

### R10 — CLI and licensing truthfulness (K1, K2, K9, K14)

1. **R10.1** — `--activate` with an invalid key exits promptly with a non-zero status and the
   server's reason; `--deactivate` reports success only when the server confirms the seat was
   released, and the local license state matches the server's answer.
2. **R10.2** — `--reset` clears the same settings store the application reads, preserving
   the keys the crash-recovery path preserves.
3. **R10.3** — Activation state changes reach consumers through one latch, so a change
   is announced once per real transition on every path (online, offline certificate).
4. **R10.4** — An API token supplied to the CLI can be read from the environment or a file
   so it need not appear on the command line.

### R11 — Build and CI integrity (L1, L2, L3, L4, L5, L6, L7, L9, L10, L12, L13, L14)

1. **R11.1** — Linux release builds carry fortification; the flag present on the
   compile line is asserted by a configure-time or CI check.
2. **R11.2** — The continuous release is republished only from the default branch, at most
   one run at a time, and only after the unit and integration tiers pass; a tagged release
   never overlaps a continuous one.
3. **R11.3** — Every third-party action is pinned to a commit hash; actions that receive
   signing material or write permissions are reviewed on every bump.
4. **R11.4** — Python test dependencies are locked with hashes and installed reproducibly;
   warnings-as-errors cannot be tripped by an upstream release.
5. **R11.5** — The source-rewriting linter has its own test suite covering every rule, does
   not rewrite files unless asked explicitly, and the retired splitter tool is removed with its
   advisory text updated.
6. **R11.6** — The unit-test tier builds and runs on all three CI platforms; a sanitizer
   (thread + address) job runs the unit tier and the hotpath benchmark on every push.
7. **R11.7** — The throughput gate gets one automatic retry on a fresh runner before
   failing, and its result is recorded per platform so drift is visible.
8. **R11.8** — The application build file lists each source once, sets each definition
   once, installs once, and does not fetch anything unpinned at configure time.
9. **R11.9** — Secrets enter jobs through environment blocks, never inline in scripts;
   every job declares least-privilege permissions; a training run that fails to activate
   fails the job.
10. **R11.10** — Every expected failure in the test suites references a finding id or issue,
    and "by design" expectations are deleted rather than expected-to-fail.

### R12 — Structural debt: one implementation per concern (A6, A7, A14, B11, B12, B13,
C4, C6, C9, C14, D11, D12, D13, D14, D16, D18, E8, E11, F11, F12, F13, F14, F20, G4, G5,
G6, G7, G8, H8, H12, I7, I14, J5, J7, K8, L8, L11)

1. **R12.1** — Each of the following pairs or families shares one implementation of its
   common behaviour so a fix in one cannot miss the other: the two serial CAN backends; the
   two polled-PLC drivers; the OpenAI-compatible LLM providers and the three reply state
   machines; the three replay players' seek, catch-up and clock logic; the three exporters'
   structure and naming logic; the five code-editor hosts' input handling; the instrument
   widgets' chrome, tick math and page persistence; the driver tag-picker dialogs.
2. **R12.2** — Dead code identified by the review is removed: the vestigial frame slot pool
   and its budget, never-emitted signals, unused scratch buffers, unreachable console-only
   branches, unused state fields, and stale comments describing removed mechanisms.
3. **R12.3** — Every rule violation the linter cannot see is closed: wildcard slot
   disconnects, a class split across two translation units, header section order, the
   trial-parity wording, and the two duplicated hardware-context instances per driver share
   native state with reference counting.
4. **R12.4** — Every driver setter on a live (non-UI) instance leaves global defaults
   untouched, matching the industrial drivers.
5. **R12.5** — QML context globals are registered through one typed registry with no
   unused entries, so QML tooling can resolve every name.
6. **R12.6** — The three largest QML monoliths are split into components along their page
   boundaries; dialog Escape handling and editor context menus are shared components.
7. **R12.7** — The singleton census does not grow, and the cached-reference idiom and its
   single-session assumption are documented where the census is described.
8. **R12.8** — Every facade over the pinned 1500-line file cap that the review named is
   under the cap after its concerns move to sub-objects, without any class split across files.
9. **R12.9** — Every vendored library records its upstream version or commit in one
   machine-readable place; vendored trees carry no foreign AI or tooling files.

### R13 — Documentation truth (A6, B21, C5, E10, F8, G11, H13, J9, K8)

1. **R13.1** — Every drifted statement the review listed is corrected or removed, and the
   claim gate pins the corrected facts.
2. **R13.2** — The AI assistant subsystem has an architecture document covering the safety
   tier model, autosave semantics, the meta-tool seam and the provider abstraction.
3. **R13.3** — The QML style guidance covers canvas theme-repaint hooks and the cost of
   whole-map bindings.
4. **R13.4** — The pinned composition order in the startup doc is verified against the
   code by the claim gate.

### R14 — Verification tiers (M1..M12, G12)

1. **R14.1** — Every finding cited in R1..R10 is pinned by at least one automated test that
   fails on the pre-fix tree and passes after; the plan lists the test per id.
2. **R14.2** — A QML lint tier runs in CI with a checked-in baseline, and a self-test
   instantiates every QML file under a stub context and fails on unresolved names, in both
   GPL and commercial builds.
3. **R14.3** — Every parser of untrusted bytes (S7, ISO-TSAP, IEC 104 APCI/ASDU, Sparkplug
   payload, OPC UA wire, CSV row, MDF4 reader, Historian block codec, API JSON, SSE stream)
   has a fuzz harness that runs in the sanitizer job.
4. **R14.4** — The acquisition path has unit coverage for block staging, flush on cap and
   tick, mask semantics, the disconnect tail, the marshal abandon path and the parked-flag
   path.
5. **R14.5** — Sinks have tests for two-source time columns, raw-lane throughput, write
   failure, live-session protection, and pause/scrub/resume timing for all three players.
6. **R14.6** — The connection manager's verdict matrix (sync failure, cancel mid-dial,
   rebuild mid-dial, drop with pending dial, reconnect preserving pause) is unit-tested with
   a fake driver; every driver has at least a drop/reconnect test against a simulator or
   loopback.
7. **R14.7** — The API has tests for HTTP-shaped input, path policy on every path-taking
   command, hostile dry-run scripts, write-cap enforcement, and gRPC stop-while-in-flight.
8. **R14.8** — The assistant has unit tests for the stream reader, think-tag splitter, reply
   state machines against a fake transport, the file sandbox, the redactor, the turn loop
   against a scripted provider, and the sentinel probe.
9. **R14.9** — Licensing, CLI early exits, extension install (partial failure, containment),
   session-context lifecycle, project history, workspace references, loader migrations and
   persistence paths each have unit coverage.
10. **R14.10** — Test documentation states the expected-failure policy and lists every tier
    including the new ones.


### R15 — Rendering and thread-priority cost under load (N1-N4; added 2026-09-02 from the maintainer's BADAQ profiling)

1. **R15.1** — A waterfall widget's per-tick GPU work is proportional to the rows that changed
   and allocates no texture per tick: one persistent texture per widget updated in place (row
   ring with the scroll applied as a UV offset, or a sub-rect upload), rebuilt only when the
   history geometry changes; no row is written when no samples arrived since the last tick; the
   overlay re-rasterizes only on axis, size, marker or theme change; a hidden widget releases
   its image and textures.
2. **R15.2** — The real-time scheduling boost (MMCSS on Windows) applies to the acquisition
   pipeline thread and to dense stream-worker threads, registered from inside those threads,
   never to the GUI thread; exactly one thread reports the elevated band per registration and
   the guard is per thread.
3. **R15.3** — A value widget that has not received a sample reports no alarm severity and runs
   no blink or colour animation; the state clears on reset and latches on the first real sample.
4. **R15.4** — Plot curve, stroke and area-fill geometry buffers grow with headroom and are
   reused; steady-state rendering of a live curve performs no per-frame geometry reallocation.


## Acceptance Criteria

**How to read the boxes (WP-J, 2026-09-02).** A checked box means the criterion is satisfied by
something that runs in CI or in a suite already merged here. An unchecked box carries the command
or observation that closes it; most of them need the maintainer, because they need a build, a
running app, or hardware. Nothing is checked on the strength of the code alone.


- [ ] **AC1** (R1) — A pytest integration run records two TCP-simulator sources into CSV,
  MDF4 and Historian across connect, pause, resume and disconnect; row counts equal frames
  sent per source, each source's time column is monotonic and independent, and no second
  file appears. A disk-full simulation (quota or read-only directory) turns the sink status
  to error within one tick with a non-zero dropped count.
  **Open — maintainer.** `test_recording_fidelity.py` and `test_historian_live_guard.py` are
  merged; run `pytest tests/integration/test_recording_fidelity.py
  tests/integration/test_historian_live_guard.py -v` with the app up. The disk-full case is
  covered as a read-only historian directory, not as a quota.
- [ ] **AC2** (R2) — Integration tests submit `while(true){}` to every script surface
  (parser, transform per lane, control script, output widget, painter, editor validate via
  the API, dry-run commands); each returns a timeout error within the configured deadline and
  the API answers a ping afterwards.
  **Open — maintainer.** The ctest half is in CI (`tst_script_dryrun`, `tst_lua_deadline_hook`).
  The end-to-end half is `pytest tests/integration/test_script_deadlines.py -v` (9 cases) with the
  app up; it was never run, because on the pre-fix build the thing it drives IS the permanent
  freeze being fixed.
- [ ] **AC3** (R3) — The sanitizer CI job runs the unit tier and the hotpath benchmark clean;
  new unit tests cover terminal clear-then-copy, the S7 zero-length item, gRPC stop with a
  parked command, and the reception path with a host stub that mutates the connection table
  mid-call. A ten-minute 48 kHz stream-lane MDF4 fixture replays under a 4 GB address-space
  limit.
  **Partly done.** The `sanitize` CI job runs the unit tier, the fuzz corpora, the instrumented
  benchmark and a TSan leg; `tst_terminal_selection`, `tst_s7comm_pdu`, `tst_grpc_pending_call`
  and `tst_client_reception` are merged and gated. **Not done:** the ten-minute 48 kHz MDF4 replay
  under a 4 GB limit (`tst_mdf4_loader_memory`), which needs a fixture only a build can generate;
  a mdflib round-trip suite is the recommended replacement.
- [ ] **AC4** (R4) — With a blackholed DNS resolver, opening each network driver leaves the
  GUI responsive (display tick keeps ticking in the API status) and Cancel returns within
  one tick; observed by the maintainer with the stall-sampling recipe.
  **Open — maintainer observation.** With a blackholed resolver, open each network driver and
  confirm the GUI keeps ticking, using the `sample <pid>` recipe in common-mistakes.md.
  `IO::AsyncTcpDial` is what makes this passable; `tst_async_tcp_dial` covers the
  unresolvable-host verdict but not the GUI.
- [ ] **AC5** (R5) — Simulator-driven tests for each driver family unplug or kill the
  simulated device and assert the connection state flips within the driver's detection bound;
  the XMODEM expected-failures become passing assertions; audio loopback runs with no output
  device selected and plays a continuous tone through `write()` that the capture side
  receives; Sparkplug reconnect and rebirth tests assert slot stability and `seq == 0`;
  Modbus group attribution survives an injected timeout.
  **Partly done.** In CI: the four XMODEM xfails are now assertions, plus
  `tst_serial_can_backend`, `tst_uart_policy`, `tst_playback_ring`, `tst_iec104_slots`,
  `tst_modbus_register_groups`, `tst_sparkplug_session`, `tst_sparkplug_publisher`.
  **Maintainer:** `pytest tests/integration/test_driver_drops.py
  tests/integration/test_audio_loopback.py tests/integration/test_modbus_groups.py
  tests/integration/test_sparkplug_host.py -v` (the last needs a local mosquitto; two driver-drop
  cases need `socat` and the `websockets` module).
- [ ] **AC6** (R6) — Headless dashboard fixture tests assert Samples-axis plot, multiplot and
  GPS receive uniform-grid data, and sweep state survives a point-count change; a benchmark
  measures Waterfall per-tick GUI cost at FFT 8192 / 70 s before and after; a theme switch
  with the Project Editor open repaints separators (maintainer observation).
  **Open — maintainer.** `pytest tests/integration/test_dashboard_lanes.py -v` with the app up;
  the waterfall before/after measurement and the theme-switch repaint stay observations. Already
  in CI: `tst_dashboard_ingest`, `tst_waterfall_ring_texture`, `tst_waterfall_tiles`,
  `tst_colormap_lut`, `tst_theme_property_map`.
- [ ] **AC7** (R7) — Project API tests: unsaved edits plus a point-count change leave the
  file's hash unchanged; each editor mutation increments the undo depth by one and marks
  modified; folder+table bulk delete leaves neither; a corrupted external write keeps the
  document attached; a new source's parser editor is empty.
  **Open — maintainer.** `pytest tests/integration/test_project_integrity.py -v` (12 cases; the
  two GUI-only AC7 cases `pytest.skip` with the manual recipe in the skip message).
  `tst_project_bulk_ops` and `tst_project_history` are in CI.
- [ ] **AC8** (R8) — Security tests send an HTTP POST to the API socket and assert the
  connection closes with no command executed; every path-taking command is probed with a
  traversal path and refused; a non-reading client hits the write cap and is disconnected;
  an IPv6 loopback client connects.
  **Open — maintainer.** `pytest tests/security/test_http_on_api_socket.py
  tests/security/test_path_policy_all_commands.py tests/security/test_write_backlog.py
  tests/integration/test_api_ipv6.py -v`. Already in CI: `tst_client_reception`,
  `tst_path_policy_registry`, `tst_server_worker_caps`, `fuzz_api_json`.
- [ ] **AC9** (R9) — Extension install tests corrupt one file of a catalog fixture and assert
  the previous version remains and the failure is reported; an `http://` repo is refused; an
  assistant integration test with auto-approve on performs a tool edit and asserts the file
  hash on disk is unchanged until Save; local-provider budget test with an 8 k window keeps
  the system prompt in the request.
  **Open — maintainer.** `pytest tests/integration/test_extension_install.py
  tests/integration/test_assistant_autosave.py -v`. Already in CI: `tst_extension_installer` (a
  corrupt update keeps the installed version) and `tst_conversation_turn` (the 8k-window budget
  arithmetic).
- [ ] **AC10** (R10) — CLI tests run `--activate` with a bad key against a stub server and
  assert exit within 5 s with non-zero status; `--deactivate` with `deactivated=false`
  reports failure and leaves the cache; `--reset` followed by launch shows defaults.
  **Open — maintainer.** `SS_BINARY=<path> pytest tests/integration/test_cli_licensing.py -v`. The
  source-level half runs today:
  `tests/scripts/test_cpp_regressions.py::test_cli_license_commands_wait_on_the_request_verdict`.
- [x] **AC11** (R11) — CI: fortification flag asserted from `compile_commands.json` on the
  Linux job; a push to a non-default branch produces no release update; every `uses:` in the
  workflows is a 40-hex pin; `pip install` runs with `--require-hashes`; ctest passes on all
  three platforms; the linter's own test suite runs in the lint job; a sanitizer job exists
  and is green.
  **Done.** Every clause is in `.github/workflows/ci.yml` and asserted by
  `scripts/tests/test_ci_workflow.py`, which the `lint` job runs and which passes here (`python3
  -m pytest scripts/tests -q`). Caveat worth stating: the FORTIFY assert step, the cross-platform
  ctest legs and the `sanitize` job have not been observed green on real runners from this branch.
- [ ] **AC12** (R12) — The linter's duplication report (new rule or `--tu-census` extension)
  shows no shared 10-line window above the agreed threshold between the named pairs; the
  singleton census baseline is unchanged or lower; `code-verify.py --check` reports zero
  advisories for the facades named in the ledger; QML lint resolves every context global.
  **Not met, deliberately.** The dup census exists and is seeded (1642 shared windows over 20
  pairs, all QML); the singleton census shrank and was re-seeded; the QML context globals resolve
  through `Misc::ContextRegistry` with a two-way lint. **But** `code-verify.py --check` still
  reports 9 `cxx-tu-too-long` advisories, including `FrameBuilder.cpp` 2983, `Terminal.cpp` 1988,
  `Dashboard.cpp` 1856 and `ConnectionManager.cpp` 1589 — the reasons are in plan.md "Left open".
  The qmllint baseline also ships unseeded, so that gate reports without blocking.
- [x] **AC13** (R13) — `claim-verify.py` passes with the new pinned claims; the AI
  architecture doc exists and is indexed in the sub-documentation table.
  **Done.** `python3 scripts/claim-verify.py --quiet` reports 0 errors with an **empty** baseline
  (it carried 9 accepted ordered-anchor findings until this package), four new constants are
  pinned, and `doc/claude/architecture/ai.md` exists and is indexed in both `architecture.md` and
  CLAUDE.md's sub-doc table.
- [ ] **AC14** (R14) — The plan's id-to-test table has no empty rows; every new tier appears
  in `tests/README.md` and in CI.

  **Partly done.** Every new tier is in `tests/README.md` (fuzz, sanitizer, qmllint, the post-root
  QML self-test, `scripts/tests/`) and in CI. The id-to-test table still has open rows: the suites
  listed under "named test not written" in plan.md "Left open".
- [ ] **AC15** (R15) — On the BADAQ project with four waterfalls live: process page faults
  under 50 k/s, kernel time under 0.1 core, working set flat, peak within a few hundred MB of
  steady state (maintainer's PowerShell script). Thread listing shows the busiest thread at
  priority 13-15 and only the pipeline and dense stream-worker threads elevated. With the CAN interface disconnected,
  GPU 3D idles near zero and unconnected groups sit still while live groups animate. A
  headless ctest asserts no `allocate()` call on a `PlotCurve` fed a stationary point count for
  100 frames.
  **Open — maintainer measurement.** The headless half is done and gated:
  `tst_plot_curve_geometry` asserts zero reallocation across 100 stationary frames and
  `tst_mmcss_registration` pins the per-thread latch. The page-fault, kernel-time and working-set
  numbers need the BADAQ project and the maintainer's PowerShell script.

## Constraints & Invariants

- The 256 kHz hotpath gate and all nine tiers must not regress; every fix on the publish
  path stays allocation-free and keeps the single-producer rule.
- Pipeline-thread and GUI-thread affinity rules stay as documented: direct connections in
  the pipeline, marshalled command-rate crossings only, no new mutex on SPSC paths.
- The open/openFinished verdict contract stays exactly-once; no new async-open hook.
- The two republish lanes keep separate marks; no export publish is gated on "this pass saw
  a change".
- Source owns time: fixes may add a safety-net stamp only where a frame arrives unstamped.
- Composition order, ctor-closure protections and the session-context ownership model are
  unchanged; any edit inside ctor-reachable code re-runs the ctor-edge proof.
- One class = one file pair; facades shrink by moving concerns into real sub-objects.
- Wire formats (mirror, Historian schema, project JSON, MDF4 output) stay readable by the
  4.1.0 release; any incompatible change bumps its version and regenerates fixtures.
- Trial parity: every Pro gate message names the trial.
- No new third-party dependency without a pinned version and a REUSE entry.
- Every fix lands with its test in the same change; no "fix now, test later".
- Windows, macOS and Linux behaviour stays identical for every user-visible requirement.

## Open Questions

All resolved with the maintainer on 2026-09-01; recorded here so the plan cannot drift.

- **Fortification (L1)**: verify on the Linux CI job first by dumping the app target's compile
  line; if the cancelling `-U` is confirmed, fix to level 3 where the toolchain supports it,
  else 2, and keep a compile-line assert in CI.
- **Cross-platform unit tier (R11.6)**: all three platforms on every push, built inside the
  existing per-OS build jobs.
- **Throughput gate (R11.7)**: one automatic retry on a fresh runner; both numbers recorded
  per platform as artifacts.
- **Sparkplug slots (R5.10)**: persist the node-to-slot table in the generated project, as
  IEC 104 does; a renamed node gets a new slot until the user regenerates.
- **MDF4 master channel (B10)**: fix the writer; the reader accepts both the conforming sync
  type and the legacy zero from a Serial Studio writer, so 4.1.0 archives keep replaying.
- **Audio playback (R5.4)**: bounded pre-allocated byte ring drained at device rate,
  continuous output, underrun zero-fill with a counter, format must match the device; no
  conversion, no resampling.
- **Duplication rule (AC12)**: 10 normalized lines, more than 40 shared windows per pair;
  advisory at introduction with a committed baseline, `--accept` re-seeds, growth blocks CI
  like the TU census.
- **Singleton census (R12.7)**: freeze only; document the cached-reference idiom and the
  single-session assumption. Reduction is a later spec.
- **QML monoliths (R12.6)**: the three largest only (extension manager, assistant panel,
  report options dialog) plus the shared Escape and editor-menu components; the rest tracked
  as follow-up.
- **API port (R8.5)**: a preference plus a CLI override; the default stays 7777.
- **Delivery**: one branch per requirement group, merged in dependency order: CI integrity
  and test harnesses first, defect groups next, structural debt and docs last so refactors
  land on fixed code. Each branch reviewed before merge.
- **Execution**: one agent per work package with a written brief naming the invariants, the
  owned files and the test to write; agents edit by hand; files shared between packages are
  integrated by the coordinator; parallel packages own disjoint files.
