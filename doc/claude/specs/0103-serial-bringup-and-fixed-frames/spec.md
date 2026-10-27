---
spec: 0103-serial-bringup-and-fixed-frames
title: Serial Bring-Up Sequences, Line Control and Fixed-Length Frames
status: approved     # draft -> approved -> in-progress -> done | shelved
created: 2026-10-26
author: Alex Spataru
---

# Spec 0103 — Serial Bring-Up Sequences, Line Control and Fixed-Length Frames

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

A class of serial devices cannot be used with Serial Studio today without a vendor program or
an external bridge script sitting between the port and the application. These are devices
that bridge another bus or instrument onto a serial port and that expect the host to bring
them up before they stream data. The concrete case that prompted this spec is a two-channel
bus adapter in an engine test cell: it powers up at 9600 baud, resets to a known state on an
RS-232 BREAK of at least 10 ms, answers the reset with one to three identification bytes, is
switched to a faster baud by a command byte sent twice, must see DTR asserted and RTS/CTS flow
control before it will accept anything, and then streams five-byte binary frames whose only
boundary marker is a sync bit set in the first byte and clear in the other four. Its vendor
software is Windows-only and twelve years old; the only way to see its data in Serial Studio
is a script that performs the handshake and re-emits text lines through a Process source.

The same shape recurs well beyond that adapter. Modems and modem-style instruments want an
attention exchange before data. GPS and IMU modules are switched into a binary protocol and a
higher baud by a configuration message, then stream fixed-size packets. RC receiver protocols
stream fixed-length frames with a header byte at 100 kbaud. MIDI marks status bytes with a set
high bit and data bytes with a clear one. LIN and DMX use a BREAK field as part of their
protocol, and many bootloaders and instruments reset on BREAK. Every one of these needs some
combination of three things the UART source does not offer:

- **Line control.** The port opens at one baud rate for the life of the connection, DTR can be
  asserted on open, and RTS is only ever driven by hardware flow control. There is no way to
  hold RTS, to send a BREAK, or to change the baud rate once the port is open. No command in
  the JSON-RPC API, no control-script global and no action can do any of these.
- **A bring-up sequence.** The only connect-time hook is an action with Auto-Execute on
  Connect. Every such action fires at once, in no defined order, after the pipeline has
  already started consuming bytes; nothing can wait for a reply, time a pause, or stop the
  connect when the device does not answer. A control script's `setup()` runs after connection
  too, inside a two-second budget, and has no sleep, no BREAK and no baud switch. In every
  variant the handshake's bytes reach the frame parser as if they were data.
- **Fixed-length framing.** The four frame-detection modes (End Delimiter Only, Start
  Delimiter Only, Start + End Delimiter, No Delimiters) all need a delimiter byte sequence. A
  frame whose boundary is a bit pattern rather than a byte value has to use No Delimiters,
  which hands the parser whatever chunk the port driver returned, so every project for such a
  device must carry its own stateful resynchronisation code and loses the pipeline's
  checksum handling and frame counters.

Each gap is small on its own. Together they are the difference between "plug it in and pick
the port" and "write and ship a second program".

## Goals

- A user connects to a device that needs a BREAK reset, an identification exchange and a baud
  switch by configuring the UART source alone, with no script, no second program and no
  vendor software, and the connection succeeds the same way from the Connect button, the
  JSON-RPC API, the command line and an automatic reconnect.
- The handshake's bytes never reach the frame parser, the dashboard or any recording; they are
  visible in the console for diagnosis.
- A device that does not answer its bring-up fails the connect with a message that names the
  step that failed, instead of connecting to silence.
- BREAK, RTS, DTR and a live baud change are available as first-class operations to the API
  and to scripts, so an ad-hoc exchange can be driven by hand or from a control loop.
- A fixed-length binary protocol is configured with a frame length and, where the protocol
  has one, a sync rule on its header byte, and the frame parser then receives exactly one
  frame per call, resynchronised and checksum-validated by the pipeline like every other mode.
- None of this costs the acquisition pipeline anything when it is not used.

## Non-Goals

- A new bus type or driver for any specific bus or adapter. Bus adapters do not share a host
  protocol; the generic layer is the serial bring-up and the framing, which this spec adds to
  the UART source. A dedicated driver remains possible later on top of these primitives.
- Decoding the words of any bus carried by such an adapter (label, sign, status and
  binary-coded fields). That is frame-parser and transform-library work in a project, and a
  candidate for a Built-In parser template in a later spec.
- A host-side scheduler that transmits a large table of periodic messages. Actions with timers
  cover a few periodic messages today; a scheduler for hundreds of entries at millisecond
  periods is its own feature.
- Branching, loops or scripting inside the bring-up sequence. The sequence is an ordered list
  of steps; anything needing logic belongs in a control script using the exported primitives.
- Applying the bring-up sequence to other drivers (Network, Bluetooth LE, USB). The step
  vocabulary must not preclude it, but only the UART source gets it here.
- Receiving BREAK as a frame delimiter (the DMX receive case), XON/XOFF changes, or a
  line-state monitor widget.
- Any change to which features are Pro. Everything in this spec ships in every build.

## Requirements

### A. Line control

1. **R1 — RTS as a setting.** The UART source gains a Hold RTS setting beside Send DTR Signal.
   When on, RTS is asserted when the port opens and held for the life of the connection; when
   hardware flow control is selected the setting is shown as not applicable, because the port
   driver owns RTS then. It is edited in the Setup Panel and the Project Editor, saved in the
   project and in the application settings, and readable and writable through the API like
   the DTR setting.
2. **R2 — BREAK.** The application can hold an open UART line in the BREAK condition for a
   requested number of milliseconds and release it, with the duration honoured to within a
   few milliseconds on each supported platform. Bytes the device emits while BREAK is held are
   discarded and never reach the frame parser.
3. **R3 — Live baud change.** The baud rate of an open UART source can be changed without
   closing the port and without the connection being reported as dropped. The rate saved in
   the project remains the rate the port opens at; a bring-up step or an API call changes only
   the running rate, so every reopen starts from the saved rate again.
4. **R4 — Line state readback.** The input modem lines of an open UART source (CTS, DSR, DCD,
   RI) can be read on demand, so a sequence step or a script can treat "CTS low after the
   handshake" as "device not present" instead of writing into the void.

### B. Bring-up sequence

5. **R5 — An ordered step list per UART source.** A UART source has an optional connection
   sequence: an ordered list of steps run every time the port is opened, before any byte is
   handed to the pipeline. The minimum step kinds are: hold BREAK for N ms; wait N ms; set a
   line (DTR or RTS) high or low; send bytes (plain text or hexadecimal, with the same
   end-of-line and transmit-checksum options an action has); expect bytes within a timeout;
   discard pending input; set the baud rate; set the flow-control mode. An empty sequence is
   the default and changes nothing about today's behaviour.
6. **R6 — Expect step.** An expect step names one or more acceptable byte patterns, each a
   sequence of bytes where any byte may be a wildcard, and a timeout. It succeeds when the
   received bytes match one pattern within the timeout and fails otherwise. A step may be
   marked optional, in which case a failure records a console note and the sequence continues.
7. **R7 — Runs on every open, before data flows.** The sequence runs on a manual connect, an
   API connect, a command-line or headless connect, and an automatic reconnect after the port
   was lost. While it runs the source is connecting, not connected: the dashboard receives no
   frames, auto-execute actions have not fired, timer actions have not started and the control
   script has not started. The connected state is reported once, after the last step succeeds.
8. **R8 — Failure is a failed connect.** A step that fails (an expect that times out, a write
   the port refuses, a port error) ends the sequence and fails the open with a driver error
   that names the step by position and kind and quotes what was received, as text in the
   console and the connection log, never a modal. The automatic-reconnect rules that apply to
   any failed open apply unchanged. A whole-sequence time budget bounds the worst case; its
   default is ten seconds and a project may raise it.
9. **R9 — Visible in the console.** Every byte sent or received during the sequence appears in
   the console with a visible marker that distinguishes bring-up traffic from data, and the
   step being run is named, so a device that answers in an unexpected way can be diagnosed
   from the console alone.
10. **R10 — Edited in both configuration surfaces, saved with the project.** The Setup Panel's
    UART pane and the Project Editor's source page both edit the sequence (add, remove, reorder
    and edit steps) and show the same list. A project saves the sequence with its UART source,
    so a project carries everything needed to bring the device up; a project saved before this
    spec loads with an empty sequence. The API can read and replace a source's sequence.
11. **R11 — Run on demand.** The sequence of a connected UART source can be re-run from the API
    and from a control script without disconnecting, for a device that was power-cycled while
    the port stayed open. The re-run follows R7 to R9: frames stop flowing for its duration and
    the result is reported.

### C. Exported primitives

12. **R12 — API commands and script globals.** BREAK (R2), the live baud change (R3), the line
    states (R1, R4), and run-sequence (R11) are JSON-RPC commands on the UART source, each
    taking the source identifier in a multi-source project, and are reachable from a control
    script through the same generated wrappers as the existing commands. Every one carries the
    capability classification the remote gate and the sealed-session policy require, is
    classified in the assistant's safety tiers, and sits behind the Allow device control
    toggle when the in-app assistant issues it.
13. **R13 — Documented like the rest.** The manual's UART page documents the new settings, the
    sequence editor and the step kinds; the API reference lists the commands; the control
    script reference lists the globals.

### D. Fixed-length frames

14. **R14 — A fifth frame-detection mode.** Fixed Length joins the four existing modes
    wherever a mode is chosen: the Project Editor's source page, the API, the assistant's tool
    schema and the frame-parser test dialog. It takes a frame length in bytes, at least one,
    and the checksum option the other modes have; when a checksum is selected its bytes are
    the last bytes of the fixed length and the frame handed to the parser excludes them.
15. **R15 — Optional sync rules.** The mode has two optional byte rules, each a mask and a
    value: a header rule the first byte of a frame must satisfy, and a body rule every other
    byte must satisfy. With no rules, bytes are cut into frames of the given length from the
    first byte received. With a header rule, bytes are discarded until one satisfies it, and a
    frame begins there. With a body rule, a frame whose later byte violates it is discarded up
    to the next byte that satisfies the header rule, so one lost byte costs at most one frame.
16. **R16 — Same contract as the other modes.** Frames are stamped from the chunk that held
    their last byte, counted in the connection diagnostics, dropped and counted when the queue
    is saturated, and ignored in Quick Plot, exactly as for the delimiter modes. A Binary
    decoder hands the parser the frame's bytes; a project that selects the mode with a length
    of zero or a rule the length cannot satisfy is repaired to No Delimiters and the repair is
    logged, as the existing modes repair a missing delimiter.
17. **R17 — Documented and generated.** The manual's frame-detection list and glossary entry
    gain the mode; the project-generation paths that write a detection mode keep writing what
    they write today.

## Acceptance Criteria

- [ ] **AC1 (R1)** — A UART source saved with Hold RTS on reopens with RTS asserted; the API
  reads the setting back; the Setup Panel and the Project Editor show the same value.
  Integration test over the API plus a maintainer observation with a line-state LED or a
  second port's CTS input.
- [ ] **AC2 (R2)** — A BREAK of 55 ms requested through the API measures between 55 and 65 ms
  on a logic analyser on each supported platform with a native and a USB serial adapter;
  bytes received during the BREAK are absent from the frame parser's input. Maintainer
  observation; a unit test covers the discard rule.
- [ ] **AC3 (R3)** — Changing the baud rate of a connected source through the API keeps the
  connected state, frames continue at the new rate, and the project's saved rate is unchanged
  after a save. Integration test over the API with a loopback adapter.
- [ ] **AC4 (R5 to R8)** — A sequence of BREAK, wait, expect, send, set-baud and send steps
  brings up the test-cell adapter from the Connect button, from the API, from the command
  line and after its power is cycled with auto-reconnect on, with no vendor software and no
  script; with the adapter unplugged from its bus power the connect fails within the budget
  and the console names the expect step. Maintainer observation on the adapter; a unit test
  drives the sequence's step logic against a scripted fake port covering success, each
  failure kind, the optional flag and the budget.
- [ ] **AC5 (R7)** — With a sequence configured, no auto-execute action fires and the control
  script's `setup()` does not run until the sequence has completed; a frame counter read
  through the API is zero during the sequence. Integration test over the API with a loopback
  adapter and a sequence that waits.
- [ ] **AC6 (R9)** — Every byte of the handshake is visible in the console with the bring-up
  marker, and none appears in a CSV export of the same session. Maintainer observation plus
  an integration test that reads the console and the export.
- [ ] **AC7 (R10)** — A project saved with a sequence loads with the same sequence; a project
  saved before this spec loads with an empty one; the API replaces a sequence and the Project
  Editor shows the change. Integration test over the API with fixture projects.
- [ ] **AC8 (R12)** — Every new command resolves to a capability classification and a sealed
  policy entry (the existing registry tests fail otherwise), appears in the API reference, is
  refused to the assistant when Allow device control is off, and is reachable from a control
  script. Existing ctest registry tests plus an integration test.
- [ ] **AC9 (R14, R15)** — Frame-reader unit tests cover: plain cutting at the length; a frame
  split across chunks; a header rule skipping leading garbage; a body-rule violation dropping
  to the next header; checksum accepted, rejected and incomplete; a length of one; a length
  above the buffer rejected at configuration. Expressed as ctest units beside the existing
  frame-reader mode tests.
- [ ] **AC10 (R14, R16)** — A project with the Fixed Length mode and the Binary decoder shows
  the parser exactly the frame's bytes in the frame-parser test dialog and live; Quick Plot
  ignores the mode; a zero length repairs to No Delimiters with a log line. Integration test
  over the API plus maintainer observation.
- [ ] **AC11 (R16)** — The hotpath benchmark gate passes unchanged with Fixed Length selected
  on a saturated source and with the mode absent.
- [ ] **AC12 (R13, R17)** — The manual pages, the API reference and the control-script
  reference describe the features; the documentation, terminology and claim checks pass.

## Constraints & Invariants

- **The acquisition pipeline is not touched by the bring-up.** The sequence runs at the link,
  before the pipeline sees a byte; the frame reader keeps its single-producer design and gains
  no lock, no allocation per frame and no per-byte signal. The fixed-length reader is one more
  mode of the same reader with the same stamping, counting and saturation behaviour.
- **One connect, one verdict.** Opening a UART source with a sequence becomes an asynchronous
  open like the dial of a network or Modbus source: the open call returns, the source reports
  connecting, and success or failure is reported exactly once. Nothing in the sequence blocks
  the GUI thread, and a sequence still running when the user disconnects is cancelled cleanly.
- **The sequence is the source's, not the application's.** It lives in the source's
  connection settings, travels with the project, and the exclusive-port check, the Pro
  connect gate and the launch consent apply before it as they do today. It never reads from
  or writes to another source.
- **New commands carry their classifications.** Every new API command has a capability class
  and a sealed-session class, and the assistant's tiers list it; an unclassified name is
  refused by the existing gates and fails the existing registry tests.
- **One name per concept.** User-facing labels are fixed once and used everywhere (UI, API
  descriptions, manual, assistant corpus): Connection Sequence for the step list, Hold RTS,
  Send BREAK, Fixed Length for the detection mode, Header Rule and Body Rule for the sync
  rules. The terminology gate enforces it.
- **Backward compatible project files.** A project without the new keys loads unchanged and
  behaves as today; a project with them loads in an older build with the sequence ignored and
  the detection mode repaired, never refused.
- **No new dependency**, no platform left behind: BREAK, RTS and the live baud change must
  work on Windows, macOS and Linux with native ports and with USB serial adapters, and a
  platform or adapter that cannot time a BREAK reports the actual behaviour rather than
  claiming the request was honoured.
- **Every build.** Nothing here is Pro-gated.

## Open Questions

- **BREAK timing fidelity.** Some USB serial adapters implement BREAK as a fixed-length pulse
  and ignore the requested duration; some port drivers queue the release behind pending
  writes. Which adapters the maintainer owns should set the acceptance tolerance of AC2, and
  whether an adapter that cannot hold BREAK is reported as a sequence failure or a warning.
  Recommendation: a warning, since many devices accept any BREAK over their minimum.
- **Alternate opening baud.** The test-cell adapter has a variant that powers up at a
  different baud. A sequence has one opening rate; a device that answers only at another needs
  either a second project or a fallback. Recommendation: out of scope for this spec; a
  control script can detect the failed expect and switch the saved rate through the API.
- **Where the sequence editor lives in the Setup Panel.** The UART pane is a compact form;
  a step list may not fit it. Recommendation: a button that opens the same editor dialog the
  Project Editor uses, so there is one editor.
- **Maximum fixed frame length.** The frame reader's buffer bounds it. Recommendation: accept
  any length the buffer can hold at least two of, and reject larger at configuration time.
- **Multi-byte header rules.** The header and body rules each test one byte with a mask. A
  protocol with a two-byte header is served today by Start Delimiter Only; whether Fixed
  Length should also accept a header byte sequence is a scope call. Recommendation: not in
  this spec.
- **Console marker.** Whether bring-up traffic is marked in the console by colour, prefix or
  a separate annotation lane. Recommendation: the frame annotation layer, which already exists
  for marking frames, with a prefix fallback in exported console text.
