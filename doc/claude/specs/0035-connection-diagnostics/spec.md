---
spec: 0035-connection-diagnostics
title: Connection diagnostics
status: draft        # draft -> approved -> in-progress -> done | shelved
created: 2026-07-25
author: Claude (roadmap R9, with Alex)
---

# Spec 0035 — Connection diagnostics

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.
>
> Roadmap parent: spec 0030, item R9 ("connection diagnostics self-test"). Depends on
> spec 0033 (problem center) for the finding collector, and pairs with spec 0034
> (async task trees) for the bounded, non-blocking runner. Named "connection
> diagnostics" throughout — never "self-test" — so it cannot be confused with the
> licensing self-test that already exists.

## Problem / Motivation

When a connection fails, Serial Studio tells the user *that* it failed and nothing about
*why* or *what to do*. The stack has no pre-open check of any kind: not one driver probes
device access rights, adapter power, host resolvability, or backend health before it tries
to open. The single place in the whole I/O layer that knows the word "permission" is a
translated string in the serial driver's post-open error table, shown only after the open
has already been rejected, and only as a modal box that says the application lacks "the
necessary access rights" — with no indication of which group, which command, or which
device.

The consequences are concrete and each one is a recurring support thread:

- **Linux, user not in the device group.** The single most common first-run failure on
  Linux: a fresh install, a plugged-in USB-serial adapter, and an account that is not a
  member of `dialout` (Debian/Ubuntu) or `uucp` (Arch/Fedora). The port enumerates
  normally — it is visible in the port list, so nothing looks wrong — and the open is
  refused. The user sees a permission box or, worse, a stall. The fix is one command the
  application knows enough to print verbatim, and does not.
- **Windows and macOS, no USB-serial driver.** A CH340, CP2102, or FTDI adapter with no
  vendor VCP driver installed produces no serial port at all. The port list is empty or
  missing the expected entry, and the application says nothing — there is not even a
  "no serial ports were found" message, let alone a pointer at what to install.
- **Bluetooth powered off or not permitted.** The BLE driver already tracks adapter power
  in shared state and already has translated strings for four discovery-agent error
  classes, but they surface as a generic error broadcast, never as "Bluetooth is turned
  off — turn it on in system settings". On macOS the failure mode is worse: a denied
  Bluetooth permission looks identical to "no devices nearby".
- **A broker or host that is not reachable.** The network driver's only pre-open check is
  DNS resolvability, and the result is folded into a boolean that gates the Connect
  button; the user is never told that the name did not resolve. The MQTT source driver has
  no pre-connect check at all — not DNS, not TCP reachability, not even whether TLS
  support is compiled in — so a typo in a hostname, a firewall, and a wrong port all
  present as the same silent failure to connect.
- **An audio input that disappeared or was never permitted.** The audio backend can fail
  to initialize, and the selected input device can vanish between sessions. Both leave the
  Connect button dead with no explanation.

Everything needed to diagnose all five is already free or nearly free at runtime. Serial
ports are enumerated once a second and carry vendor, product, and system path. Audio
devices are enumerated once a second and carry capabilities. Bluetooth adapter power is
already a tracked boolean. Host resolvability is already computed. Nothing collects those
signals, compares them against what the user has configured, and says the one sentence
that would end the support thread. Spec 0033 built the collector, the panel, the badge,
and the notification path for exactly this kind of standing condition; this spec supplies
the checks that populate it for the I/O stack.

## Goals

- A user whose connection just failed is told what is wrong and given the exact fix —
  including, where one exists, the literal command to run — instead of a raw error or a
  timeout.
- A user who has not tried to connect yet can ask "is my machine set up for this?" from
  the welcome page and get an ordered, readable answer before plugging anything in.
- The most common first-run failure on Linux (device-group membership) is diagnosed by
  name, with the correct group for the actual device and the correct command for the
  actual user account.
- A diagnostics run is bounded in time, never blocks the interface, and can be cancelled;
  a run that cannot finish reports what it could not reach rather than hanging.
- Checks are added one file at a time. A new bus gets diagnostics without any change to
  the runner, the collector, the panel, or the API.
- Nothing about the machine leaves the machine. The only network traffic a run may
  generate is to the host the user already configured.

## Non-Goals

- **No new UI surface.** Findings render in the spec-0033 problem center, use its three
  severities, its notification summary, and its badge. This spec adds no list model, no
  panel, and no severity level.
- **No repair actions.** Diagnostics prints the command; it never runs it, never elevates
  privileges, never installs a driver, never edits a group file, and never changes a
  driver's configuration.
- **No connection is opened.** Checks probe; they do not open a data link. The one probe
  that establishes a socket (host reachability) closes it immediately and never sends or
  receives application bytes.
- **No protocol-level validation.** Whether a broker accepts the credentials, whether a
  BLE peripheral exposes a usable characteristic, and whether a device speaks the expected
  frame format are out of scope — those are connect-time and parse-time concerns, and the
  link checkers from spec 0033 already cover the parse side.
- **No telemetry, no reporting, no upload.** Results are session-local and are never
  transmitted, aggregated, or persisted.
- **No replacement for the licensing self-test.** Different subsystem, different name,
  no shared surface.
- **Not a Pro feature.** Diagnostics for the GPL buses ship in GPL builds. Checks for a
  Pro-only bus compile out with that bus, not the feature.
- **No per-driver rewrite.** This spec reads driver state; it does not restructure any
  driver's open path. Rewriting the open flows is spec 0034.
- **No background polling.** Diagnostics run when asked or when a connection fails. There
  is no periodic run and no watchdog.

## Requirements

1. **R1 — One ordered runner.** A single runner executes a declared, ordered list of
   checks and reports when the whole run is complete. Order is stable across runs, so two
   runs on the same machine produce the same sequence of results. Adding a check is a
   declaration plus one implementation; the runner is not modified.
2. **R2 — Every check yields a verdict and a remedy.** Each check completes with exactly
   one of: pass (nothing reported), warning, or failure. A warning or failure carries a
   short title, one or two sentences naming the concrete cause with the concrete value
   involved (the port path, the host name, the group name, the device name), and a remedy
   that states what the user must do. A check may also pass with an informational note
   when the pass itself is worth stating (for example, that no serial ports are present).
3. **R3 — Instant checks never wait; probing checks are bounded.** Checks are declared as
   either *instant* — answerable from information the process already has or can obtain
   without contacting anything outside itself — or *probing* — requiring a name lookup, a
   socket, or a radio scan. Instant checks complete within the call. Every probing check
   carries an explicit timeout, and a timeout is itself a reportable verdict with its own
   remedy. No check, of either class, may block the user interface for any measurable
   time.
4. **R4 — Findings flow through the problem center.** Results are published as findings in
   the spec-0033 collector, one registered checker per bus, so they appear in the existing
   panel, badge, notification summary, and read API with no new surface. A finding whose
   condition has been fixed disappears on the next run, exactly like every other finding.
5. **R5 — Runs can be scoped to a bus.** A run may cover every bus the build supports, or
   only the bus relevant to a specific situation. A serial-port failure must not cause a
   broker to be contacted, and a broker failure must not cause a Bluetooth scan.
6. **R6 — Serial checks.** At minimum: whether any serial ports are present at all; whether
   the port the user selected still exists; whether the current process can actually read
   and write that port's device node; and, on Linux, when it cannot, which group owns the
   node, whether the account is already a member of that group, and the exact command that
   would add it. Where the account is already a member but the running session is not, the
   remedy says so — the fix is to log out and back in, not to run the command again.
7. **R7 — Bluetooth checks.** At minimum: whether the platform supports Bluetooth Low
   Energy at all; whether an adapter is present and powered on; and, on platforms that
   require an explicit user grant, whether the application has been granted Bluetooth
   permission. Checking adapter state must reuse the state the driver already tracks and
   must never start a second discovery scan alongside the driver's own.
8. **R8 — Host and broker reachability.** When a network source or an MQTT source is
   configured, the run resolves the configured host name and, if it resolves, opens and
   immediately closes a bounded TCP connection to the configured port. The three failure
   modes are reported distinctly, because they have three different remedies: the name did
   not resolve, the connection was refused, and the connection timed out. The probe sends
   no application bytes and performs no protocol handshake.
9. **R9 — Audio checks.** When the audio bus is available in the build: whether the audio
   backend initialized; whether any input device is present; whether the previously
   selected input device is still present; and, on platforms that require an explicit user
   grant, whether the application has been granted microphone permission.
10. **R10 — Remedies are copyable and literal.** Where the remedy is a command, that
    command appears verbatim, with the real group name and the real account name
    substituted, and it is not translated — the surrounding sentence is translated, the
    command is not. The user can select and copy it from the panel without editing it.
11. **R11 — A connection failure triggers diagnostics automatically.** When an open
    attempt fails, the instant checks for that bus run immediately, and any failure they
    find is included in the message the user already sees — not only in a panel they would
    have to know to open. The probing checks for that bus then run in the background and
    publish their findings normally. Repeated failures do not repeatedly re-probe: an
    automatic run is rate-limited per bus.
12. **R12 — Runnable from the surface the user is already looking at.** The roadmap calls
    this "the welcome page"; there is no welcome page in this application, and the honest
    equivalents are the device setup pane — the surface a user configures a connection on
    before ever pressing Connect — and the connecting overlay, the surface a user stares
    at while a connection is failing. Both offer a way to run diagnostics and reach the
    result, so a machine can be checked before anything is plugged in and again at the
    moment it does not work.
13. **R13 — Available as a command.** Running diagnostics is a registered command, so it
    appears in the command palette and the Start menu and can carry a shortcut, like every
    other surface.
14. **R14 — Readable and startable over the API.** An agent driving the API can start a
    run, learn when it has finished, and read the resulting findings. Starting a run must
    not block the API connection or the interface for the duration of the run. The findings
    themselves are read through the existing problem-center read command; this spec does
    not duplicate that surface.
15. **R15 — Bounded, read-only, and idle when unused.** A full run has a stated worst-case
    duration. Diagnostics never mutate the project, driver configuration, or persisted
    settings; never open a data link; and consume no CPU and no timers when no run is in
    progress.

## Acceptance Criteria

- [ ] **AC1 (R6, R10) — the roadmap criterion.** On a fresh Linux machine whose account is
      not a member of the device group, connecting to a serial port fails and the message
      the user sees names the port, names the owning group, and contains the exact command
      that fixes it. Adding the account to the group and starting a new session makes both
      the finding and the failure disappear. *Maintainer-run, requires a Linux machine and
      a USB-serial adapter.*
- [ ] **AC2 (R6)** — On the same machine, with the account already added to the group but
      the session not yet restarted, the finding says the membership exists and the session
      must be restarted, and does **not** repeat the `usermod` command. *Maintainer-run,
      Linux.*
- [ ] **AC3 (R6)** — With no serial adapter connected on any platform, a run reports that
      no serial ports were found, and the remedy names the common USB-serial driver
      families to install on that platform. *Maintainer-run, all three platforms.*
- [ ] **AC4 (R7)** — With Bluetooth switched off in system settings, a run reports the
      adapter as powered off with a remedy naming the setting to change; switching it on
      and re-running clears the finding. The driver's own device discovery is unaffected by
      the run — a scan already in progress is neither interrupted nor duplicated.
      *Maintainer-run, requires a machine with a Bluetooth adapter.*
- [ ] **AC5 (R8, R3)** — `pytest tests/integration/test_connection_diagnostics.py`:
      configuring a network source with an unresolvable host name produces a
      "name did not resolve" finding; a resolvable host with a closed port produces a
      "connection refused" finding; a routable address that black-holes traffic produces a
      "timed out" finding within the declared timeout and not later. The three findings
      carry three different remedies.
- [ ] **AC6 (R8)** — Same test file: with a listener bound to a local port and configured
      as the source host and port, a run reports no reachability finding, and the listener
      records a connection that carried zero application bytes.
- [ ] **AC7 (R9)** — With the audio bus available and the previously selected input device
      removed, a run reports that the configured input device is gone and names it.
      *Maintainer-run, commercial build, requires unplugging an input device.*
- [ ] **AC8 (R14)** — Same test file: the API run command returns immediately with the list
      of checks it started; polling the status command reports the run as finished within
      the declared worst case; the problem-center read command then returns the diagnostics
      findings alongside any other findings. The commands appear in the assistant safety
      manifest — asserted by a runnable static test under `tests/scripts/`.
- [ ] **AC9 (R5, R11)** — Same test file: a failed open on a network source produces
      network findings and produces no Bluetooth, serial, or audio findings; a second
      failed open within the rate-limit window does not start a second probing run.
- [ ] **AC10 (R4, R12, R13)** — Maintainer observation: the command appears in the palette
      and the Start menu; the setup pane offers the run and shows the outcome; findings
      appear in the problem-center panel with the other findings, contribute to the badge
      counts, and produce one notification summary on first appearance and none on an
      unchanged re-run.
- [ ] **AC11 (R3, R15)** — Maintainer observation: during a full run against an
      unreachable host, the interface stays responsive — windows redraw, menus open, and
      the run can be cancelled — and no dialog, wait cursor, or frozen frame appears. With
      no run in progress the application has no diagnostics timer active.
- [ ] **AC12 (R15)** — `--benchmark-hotpath` shows no regression against the pre-change run
      on all nine gated tiers. *Maintainer-run.*
- [ ] **AC13** — `python scripts/code-verify.py --check` and `python scripts/registry-verify.py`
      clean; `python scripts/sanitize-commit.py` runs with no new lint debt.

## Constraints & Invariants

- **Nothing may block.** No `waitFor*`, no nested event loop, no thread sleep, no
  synchronous DNS. These are precisely the defects spec 0034 exists to remove from the I/O
  stack; this spec must not add new instances of them. Every probing check is expressed on
  the async task-tree engine with an explicit timeout.
- **Spec 0033 is the only finding surface.** No new model, no new panel, no new severity,
  no second notification path. Diagnostics register as checkers like every other checker.
- **The problem-center checker contract is synchronous.** A checker is called and must
  return findings in that call. An asynchronous run therefore cannot happen *inside* a
  checker; the run must complete first and the checker must be a reader of what the run
  produced. This is the single load-bearing design constraint of the feature.
- **The composition root is pinned (spec 0001).** Any new module's constructor must be
  inert — no reach into another singleton — and all wiring must happen in the existing
  post-construction phase, so the constructor-edge proof stays trivial.
- **The hotpath is untouched.** Nothing in this feature runs on, gates, or is gated by the
  frame parse path or any cached hotpath flag. No new per-frame work, no new signal at
  frame rate.
- **Bluetooth discovery is shared.** The BLE driver keeps a single process-wide adapter
  handle and a shared discovery agent. Diagnostics read that shared state; they must not
  construct a second local-device object, must not start a second scan, and must not stop
  or restart a scan the driver owns.
- **No unrequested permission prompts.** On platforms where querying or exercising
  Bluetooth or microphone access can raise a system permission dialog, a run must not
  raise one the user did not ask for. Permission *status* may be read; permission must not
  be *requested* as a side effect of a diagnostics run.
- **Read-only with respect to configuration.** No check may change a driver property, a
  persisted setting, the selected port or device index, or the project document.
- **Commercial gating follows the bus.** Checks for a bus that only exists in commercial
  builds are compiled out with that bus. The runner, the registration, and the API surface
  are GPL.
- **No new third-party dependency** (roadmap constraint, spec 0030). Platform-specific
  facts are obtained from the platform's own interfaces or from Qt.
- **Translatable prose, literal commands.** Every user-visible sentence is translatable;
  the shell command embedded in a remedy is not, and is not reformatted, wrapped, or
  localized.
- **Operation modes.** A run must behave correctly with no project loaded (the welcome-page
  case), in QuickPlot, and in Console-only mode.
- **The run is cancellable and re-entrant-safe.** Starting a run while one is in progress
  must not produce two concurrent runs or two sets of results; the earlier run is
  superseded or the request is refused, never interleaved.

## Open Questions

- **Does R9 land before or after spec 0034 is approved?** The probing checks are written
  against the async task-tree engine. That engine exists in the working tree but its spec
  is still `draft`. If 0034 is shelved or reshaped, this spec's runner needs a different
  foundation — worth confirming the ordering before planning.
- **The problem-center panel is currently unreachable from the interface.** Spec 0033
  declared the `app.problems` command in the manifest but bound it in no context, so the
  panel it built has no way to open. Every requirement here that ends in "the user sees
  the finding" depends on that binding existing. It is a one-line fix that belongs to
  spec 0033, not this one — confirm it lands there before this spec's acceptance is
  meaningful, rather than absorbing it here.
- **Setup-pane behavior: offered, or automatic?** Running automatically on first launch
  gives the best answer with zero user action, but on macOS a Bluetooth or microphone
  check risks raising a permission dialog before the user has expressed any intent. Offer
  a button only, run instant checks automatically and probing checks on request, or run
  everything automatically except the permission-sensitive checks?
- **How long do diagnostics findings stand?** Project and link findings are re-derived on
  every run of their trigger. Diagnostics findings are re-derived only when a run happens,
  so they persist until the next run. Should they be cleared on a successful connection to
  the affected bus, expire after a period, or simply stand until re-run? "Stand until
  re-run" is the simplest and is probably right for a permission problem, but it means the
  panel can show a stale reachability result.
- **API scope: `diagnostics.*` or fold into `problems.*`?** A separate `diagnostics` scope
  is clearer to an agent and matches the feature name; folding `run` into the existing
  `problems` scope avoids a new scope for two commands. Recommendation and rationale
  belong in the plan, but the maintainer's preference decides.
- **Is the "device present but no driver" check worth building?** On Windows and macOS the
  strong version of the missing-driver diagnosis requires enumerating USB devices and
  matching known USB-serial chip identifiers against the absent port — which is only
  possible in builds that already carry a USB layer. The weak version ("no ports found,
  here are the drivers people usually need") works everywhere and costs almost nothing.
  Ship the weak version only, or both?
- **Should a failed run be reported as a finding?** If a probing check cannot even be
  started — no network stack, no adapter object — is that a finding, a silent skip, or a
  line in the panel saying the check did not run?
- **Rate limit for automatic runs.** R11 requires one, but the window is a judgement call:
  long enough that a retry loop cannot spam probes, short enough that a user who fixes the
  problem and immediately retries sees the updated answer.
