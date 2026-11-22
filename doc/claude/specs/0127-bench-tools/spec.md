---
spec: 0127-bench-tools
title: "Bench tools: bus monitor, console automation, VOFA+ input and plot lanes"
status: draft  # draft -> approved -> in-progress -> done | shelved
created: 2026-11-21
author: Alex Spataru
---

# Spec 0127 — Bench tools: bus monitor, console automation, VOFA+ input and plot lanes

> **Step 1 of 2 — what, why, and the design.** Everything above **Design** is readable without
> the codebase: no file paths, no class names, no signal wiring. Gate: do not start
> `/ss-implement` until a human marks this `approved`. It is the only gate.

## Problem / Motivation

Serial Studio is strong once a stream is flowing into a dashboard, and thin in the hour before
that: the bench hour, where someone is bringing up a board or chasing a fault on a shared line.
A read of the manual and the code on 2026-11-21 found nine places where that user has to write a
script, or cannot do the job at all:

- On an RS-485 line the Modbus driver can only be the master. It polls, discards error replies
  and cannot listen. The annotation template for Modbus RTU labels bytes but does not check the
  CRC, pair a request with its response, or notice a slave that never answered.
- Firmware written for the VOFA+ wire formats (JustFloat, FireWater) has no ready-made input
  format. JustFloat needs a hand-written parser.
- Console pins only fill the send field. Sending five commands in order with a pause between
  them needs a control script or a macro.
- Answering a received message automatically (a handshake, a heartbeat, a simulated device)
  needs a frame-parser script that writes back to the device.
- The console colours text only when the device sends ANSI codes. A user cannot say "show ERROR
  in red".
- The CTS, DSR, DCD and RI line levels are readable through the API and shown nowhere.
- Multi-Plot overlays every channel on one Y axis, so a 0-1 flag and a 0-4000 count cannot be
  read together.
- Nothing shows when messages were exchanged: the annotation track runs on byte offset, so gaps
  between messages are invisible.
- Decoding a binary frame with mixed field widths, bit fields and scaling needs code. The
  no-code options cover one uniform width per frame, or one format per dataset.

The people who feel it are firmware and field engineers, most of whom meet the product through
the console and Quick Plot before they ever build a project.

## Goals

- A user can leave a Modbus RTU line monitored and read, per exchange, who spoke, what was asked,
  what came back and what went wrong.
- A user can see message timing: when each message was sent or received and the gap between them.
- A user can run a saved list of console commands in order, once or in a loop, with a delay per
  item.
- A user can make the console answer matching messages by filling in a table.
- A user can colour keywords in the console by filling in a table.
- A JustFloat or FireWater device plots without the user writing a parser.
- A user can describe a mixed binary frame field by field without code.
- A user can see the serial control-line levels while connected.
- A user can split a Multi-Plot into one lane per channel.

## Non-Goals

- No listen-only Modbus *driver*: sniffed registers do not become dashboard datasets. The monitor
  is a console tool.
- No Modbus TCP or ASCII monitoring; RTU only.
- No JustFloat auto-detection in Quick Plot. JustFloat is a Built-In template for Project File
  mode (decided 2026-11-21).
- No regular expressions on hex auto-reply rules; text rules only (decided 2026-11-21).
- No step debugger, breakpoints or pass/fail expressions for command lists. A list sends and
  waits; it does not evaluate.
- No forwarding of bytes from one device to another.
- No new API commands, and nothing new in the project file: command lists, reply rules and colour
  rules are app-wide settings (decided 2026-11-21).
- No change to what manual console send may do in a sealed deployment session.
- Plot lanes do not add per-lane triggers, per-lane cursors on Y, or per-lane recording.

## Requirements

**Group A — Console automation (free)**

1. **R1** — Each pinned command has an enabled checkbox and a delay in milliseconds (0 to
   3,600,000, default 1000). Both persist with the pin.
2. **R2** — "Run list" sends every enabled pin in list order, waiting each pin's delay before
   sending it, using the hex, line-ending, checksum and encoding settings stored with that pin.
   It stops after the last pin. "Loop" does the same and starts again from the first pin until
   stopped.
3. **R3** — A running list stops when the user presses Stop, when the link closes, and cannot
   start while cyclic send is armed (and the reverse). While the link is paused the list holds
   its position and sends nothing. Run list and Loop are hidden in a sealed deployment session.
4. **R4** — An auto-reply rule has: match text, match mode (starts with, ends with, equals,
   contains, pattern, and regular expression on text rules only), a hex flag, a reply payload with its own hex, line-ending and checksum
   settings, and an enabled checkbox. Up to 64 rules. Rules persist; the armed state does not.
5. **R5** — With auto-reply armed, each received message is tested against every enabled rule and
   every matching rule sends its reply to the device the message came from. A text rule's
   message is one line. A hex rule's message is the bytes received until the line has been idle
   for the configured gap (1 to 1000 ms, default 10).
6. **R6** — "Pattern" mode uses the same syntax as the Connection Sequence's Expect Reply: hex
   bytes with `??` wildcards and `|` alternatives. "Regular expression" mode searches a text
   line for the expression; a match anywhere is enough. An invalid pattern or expression cannot
   be saved. A message longer than 4096 bytes is matched on its first 4096 bytes.
7. **R7** — Auto-reply is hidden in a sealed deployment session. It disarms when the link closes
   and never arms itself at start-up, on connect, or on project load. If more than 50 replies are sent within one second it disarms
   and writes one line to the console saying so.
8. **R8** — A colour rule has: keyword, case-sensitive flag, text colour, background colour and
   an enabled checkbox. Up to 32 rules, plain text match. Text received after a rule is enabled
   shows the keyword in those colours in every console view. A rule's colours win over
   device-sent ANSI colours; the Find highlight wins over both.
9. **R9** — Colour rules change only how text is drawn. Copy, the console log and exported text
   are unchanged.

**Group B — Bus monitor (Pro)**

10. **R10** — The console has a Bus Monitor panel with two modes: Messages and Modbus RTU. It is
    available in Pro builds with an active licence or trial, and absent from the GPL build.
11. **R11** — In Messages mode every send is one message and received bytes are split into
    messages by the idle gap of R5. Each message records time, direction, length and bytes.
12. **R12** — In Modbus RTU mode the monitor treats all traffic as one stream, splits it into RTU
    frames by function code and length, verifies each CRC, and labels each frame Master, Slave or
    Unknown. It resynchronises after corrupt bytes without losing the frames that follow.
13. **R13** — Each master request is paired with the next slave response. The monitor reports:
    no response within the timeout (10 to 10,000 ms, default 500) or before the next request;
    response address or function not matching the request; late response; response with no
    request; CRC failure; write echo that differs from its request; byte count that disagrees
    with the quantity asked for; exception response, with the standard exception name; request
    parameters outside the protocol's limits.
14. **R14** — A no-response verdict appears within the timeout even if the bus goes silent.
15. **R15** — The table shows time, role or direction, slave address, function, length, a data
    summary, CRC result, diagnosis and the raw bytes. Register values in the summary are shown
    in the formats the user ticks (unsigned 16, signed 16, unsigned 32, signed 32, float 32,
    binary). Rows with an error and rows with a warning are tinted differently. "Issues only"
    hides clean rows. Counters show frames, errors and warnings since the last clear.
16. **R16** — The timeline shows the same messages as bars on a time axis, requests and sends in
    one lane and responses and receives in another, coloured by severity, with the average
    interval between messages. The window is 200 ms to 10 minutes; hovering a bar shows its time,
    role, length, bytes and diagnosis.
17. **R17** — The monitor keeps the most recent 5000 messages, can be paused and cleared, and
    costs nothing while its panel is closed.

**Group C — Input formats (free)**

18. **R18** — A Built-In template "VOFA+ JustFloat" decodes a frame of little-endian 32-bit
    floats into one channel per float, with or without the four-byte end marker still attached,
    for any channel count up to the dashboard's channel cap. Its help page states the frame
    detection settings to use.
19. **R19** — FireWater lines (`name: v1, v2, v3` ending in a newline) plot in Quick Plot with one
    channel per value, and Save as Project produces a project that parses them identically.
20. **R20** — A Built-In template "Field Layout" decodes one frame into one channel per declared
    field. A field has: byte offset, bit offset (0 to 7), bit length (1 to 64), type (unsigned,
    signed, float 32, float 64), byte order, scale and offset. Up to 256 fields.
21. **R21** — The Field Layout editor is a table: add, remove, reorder and edit fields, with a
    live decode of a pasted sample frame. A layout that reads past a frame's end yields no value
    for that field and never fails the frame.

**Group D — Small UI (free)**

22. **R22** — While a serial port is open, the serial setup pane shows four indicators for CTS,
    DSR, DCD and RI that follow the line levels within half a second. They are hidden when the
    port is closed.
23. **R23** — Multi-Plot has a Lanes toggle. In lane mode each visible channel gets its own
    equal-height band with its own auto-scaled Y range, all bands share the X axis, and each band
    shows its channel name and its minimum and maximum. The toggle persists per widget.
24. **R24** — In lane mode a hidden channel takes no band, and the X cursors, pause, pan and zoom
    on X behave as in overlay mode. Trigger editing is unavailable in lane mode.

## Acceptance Criteria

- [ ] **AC1** (R1-R3) — Unit test: a list of three enabled pins and one disabled pin, delays 0,
      50 and 100 ms, sends exactly three payloads in order with each pin's own settings; loop
      mode sends the fourth payload as the first again; closing the link stops it.
- [ ] **AC2** (R4-R6) — Unit test over a rule table: each match mode against matching and
      non-matching messages, text and hex; two matching rules send two replies; a pattern with
      `??` and `|` matches as the Connection Sequence does; a regular expression matches
      mid-line, is refused on a hex rule, and an invalid one cannot be stored.
- [ ] **AC3** (R3, R7) — Unit test: armed state is false after construction with stored rules;
      closing the link disarms; 51 replies in one second disarm and emit one console note; in a
      sealed session neither auto-reply nor a list run can be started.
- [ ] **AC4** (R8-R9) — Unit test on the terminal buffer: an enabled rule colours every
      occurrence in appended text and leaves the stored characters unchanged; a disabled rule
      colours nothing; ANSI-coloured text takes the rule's colours.
- [ ] **AC5** (R12-R13) — Unit test replaying the premise-check trace (20,000 exchanges, eight
      function codes, random chunk sizes of 1 to 64 bytes): every valid frame framed, every role
      correct, every dropped response, corrupt response and exception flagged on the right row.
- [ ] **AC6** (R13) — Unit test, one case per diagnosis in R13, including the 8-byte coil
      response that has the same length as its request.
- [ ] **AC7** (R14) — Unit test: a request followed by silence is flagged after the timeout with
      no further bytes fed.
- [ ] **AC8** (R11, R15-R17) — Unit test of the message store: gap splitting, the 5000-message
      cap, counters that survive trimming, pause, clear, and no ingestion with no viewer.
- [ ] **AC9** (R10, R16) — Maintainer observation in a Pro build against a USB-RS485 adapter on a
      live master and slave: table and timeline agree, unplugging the slave shows no-response
      rows in red, and the panel is absent in a GPL build.
- [ ] **AC10** (R18) — Unit test: 1, 3 and 64 floats, with and without the end marker, a NaN
      payload, and a frame whose length is not a multiple of four (no values, no failure).
- [ ] **AC11** (R19) — Three FireWater streams added to the existing Quick Plot corpus and
      round-trip tests: channel count, values, and saved project equals live parse.
- [ ] **AC12** (R20-R21) — Unit test: every type at byte-aligned and unaligned offsets in both
      byte orders, sign extension at 1, 7, 31 and 63 bits, scale and offset, and a field past
      the frame end.
- [ ] **AC13** (R22) — Maintainer observation with a loopback plug: toggling RTS and DTR lights
      CTS and DSR within half a second; indicators vanish on disconnect.
- [ ] **AC14** (R23-R24) — Unit test of the per-channel ranges the plot model reports (two
      channels of different magnitude report different ranges; a hidden channel is skipped),
      plus maintainer observation of the toggle, persistence and band labels.
- [ ] **AC15** — `--benchmark-hotpath` on the maintainer's build shows no regression against the
      current numbers, run after groups C and D land.

## Constraints & Invariants

- **Deciding constraint: nothing here runs on the acquisition pipeline's publish path.** Every
  console feature consumes the existing raw-byte observer on the GUI thread at chunk rate.
- The 256 kHz gate must not regress. Group C adds parser templates under the existing template
  contract; group D adds no pass over plot data.
- No automatic transmission without a user gesture in the current session. Nothing in a project
  file can arm a send.
- Source owns time: received messages use the timestamp their chunk already carries. Sent
  messages have no source timestamp and are stamped by the monitor for display only; that time
  is never written to a frame, an export or the Historian.
- The bus monitor is Pro and its sources carry the commercial licence only; the GPL build must
  compile and link without them. Manual pages state that the trial unlocks it.
- Bounded everything: 64 reply rules, 32 colour rules, 5000 messages, 256 fields, 4096-byte
  messages for matching.
- Works in Quick Plot, Project File and Console Only modes, and with several devices connected
  (each tool follows the console's selected device).
- No new dependency.
- Two files are close to their size ceilings and take almost no new lines: the terminal view
  source (1972 of 1997) and the serial driver source (1464 of 1500).

## Premise Check

- **Question:** the wire's idle gaps do not survive into a byte stream, so can a monitor that
  sees only arbitrary chunks and their arrival times still frame Modbus RTU, tell master from
  slave and pair exchanges? If not, group B needs a driver and is a different spec.
- **Measured:** a throwaway Node simulation (scratchpad, 2026-11-21): 20,000 exchanges at 9600
  baud across eight function codes, 3% dropped responses, 3% exceptions, 2% corrupt responses,
  fed in random chunks of 1 to 64 bytes. Framing by function code with CRC validation and
  one-byte resynchronisation recovered 38,758 of 38,758 valid frames (100%), with 38,758 roles
  correct, 0 phantom frames, 840 of 840 dropped responses flagged on the right request, 677 of
  677 exceptions flagged, and all 402 corrupt responses counted as unanswered requests.
- **Verdict:** holds. The trace is synthetic: real adapters add noise bytes and partial frames at
  power-up, which AC9 covers on hardware. Timing resolution is the chunk's arrival, so two frames
  delivered in one chunk share a time; late-response verdicts are only as fine as the adapter's
  batching (typically 1 to 16 ms).
- **Not measured:** R19. Reading the line grammar says a FireWater line already tokenises as a
  named first field followed by positional fields, but no build was available to run it. AC11
  settles it; if it fails, R19 becomes a grammar change on the Quick Plot lane and this spec
  returns to draft.

## Design

### Approach

Everything in groups A and B becomes a small object owned by the console handler, next to the
send library and the annotation layer, and fed from the handler's existing `IO::IRawByteTap`
entry points. The bus monitor is a new Pro-only pair: a protocol-agnostic message store that is
also the table model and paints the timeline as flat geometry (the annotation strip's proven
shape), and a Modbus RTU analyser that turns bytes into messages with diagnoses. Group C is two
stateless Built-In templates under the existing registry. Group D is a line-level watcher
sub-object for the UART driver and per-curve ranges captured inside Multi-Plot's existing
auto-scale scan. Alternatives rejected: shipping the monitor as a JavaScript annotation template
(a third of the size, but a shipped script cannot be Pro and decode only runs when bytes arrive,
so R14 fails), and a listen-only Modbus driver (puts sniffing on the pipeline; deferred).

### Affected files

Landing is one commit per group (decided 2026-11-21); each group builds and tests alone.

| File | Change | ± lines |
|------|--------|---------|
| **Group A — console automation** | | |
| `core/Ui/Console/SendLibrary.h` / `.cpp` | pin `enabled`/`delayMs`, run/loop/stop, single-shot chain, exclusion with cyclic | +30 / +150 |
| `core/Ui/Console/Handler.h` / `.cpp` | own `AutoReply` and `HighlightRules`; send-with-explicit-spec helper; feed the tap to `AutoReply` | +14 / +45 |
| `core/Ui/Console/AutoReply.h` / `.cpp` (new) | rule store, line and idle-gap message assembly, matching, rate cap | +110 / +340 |
| `core/Ui/Console/HighlightRules.h` / `.cpp` (new) | rule store exposed to QML and to the terminal | +75 / +170 |
| `core/Ui/UI/Widgets/Terminal/TerminalHighlighter.h` / `.cpp` (new) | colours the rows an append touched | +45 / +130 |
| `core/Ui/UI/Widgets/Terminal/TerminalBuffer.h` / `.cpp` | report the row range an append touched; colour override hook | +6 / +30 |
| `core/Ui/UI/Widgets/Terminal.h` / `.cpp` | hold the highlighter, call it after append | +6 / +18 |
| `app/qml/Widgets/Dashboard/ConsoleSendLibrary.qml` | checkbox and delay per pin; Run list, Loop, Stop | +130 |
| `app/qml/Widgets/Dashboard/ConsoleAutoReply.qml` (new) | rule table and Arm button | +310 |
| `app/qml/Widgets/Dashboard/ConsoleHighlightRules.qml` (new) | rule table with colour pickers | +230 |
| `app/qml/Widgets/Dashboard/Terminal.qml` | two ribbon toggles | +55 |
| `core/Ui/CMakeLists.txt`, `app/CMakeLists.txt` | new sources and QML files | +9 |
| `app/tests/tst_console_send_sequence.cpp`, `tst_console_auto_reply.cpp`, `tst_terminal_highlight.cpp` (new), `app/tests/CMakeLists.txt` | AC1-AC4 | +645 |
| `doc/help/Console-Automation.md` (new), `doc/help/help.json`, `doc/help/Toolbar-Reference.md` | manual | +166 |
| **Group B — bus monitor (Pro)** | | |
| `core/Ui/Console/BusMonitor.h` / `.cpp` (new, commercial SPDX) | message store, table model, timeline geometry, idle-gap splitter, viewer gate | +150 / +430 |
| `core/Ui/Console/ModbusRtuAnalyzer.h` / `.cpp` (new, commercial SPDX) | framing, CRC, roles, pairing, diagnoses, data summary | +120 / +440 |
| `core/Ui/Console/Handler.h` / `.cpp` | own and feed the monitor under `BUILD_COMMERCIAL` | +8 / +22 |
| `app/qml/Widgets/Dashboard/ConsoleBusMonitor.qml` (new) | table, timeline canvas, mode and format controls | +540 |
| `app/qml/Widgets/Dashboard/Terminal.qml` | one ribbon toggle, Pro only | +25 |
| `core/Ui/CMakeLists.txt`, `app/CMakeLists.txt` | commercial sources | +9 |
| `app/tests/tst_modbus_rtu_analyzer.cpp`, `tst_bus_monitor.cpp` (new), `app/tests/CMakeLists.txt` | AC5-AC8 | +610 |
| `doc/help/Bus-Monitor.md` (new), `doc/help/help.json`, `doc/help/Drivers-Modbus.md` | manual | +182 |
| **Group C — input formats** | | |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/BinaryJustFloat.h` / `.cpp` (new) | template and parser | +40 / +170 |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/BinaryFieldLayout.h` / `.cpp` (new) | layout parse, bounded bit extractor, template | +70 / +300 |
| `core/Pipeline/DataModel/Scripting/NativeTemplates/BinaryTemplates.cpp` | register both | +4 |
| `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp` | get/set the layout's JSON parameter as rows; sample decode | +60 |
| `app/qml/ProjectEditor/Views/NativeParserPane.qml` | host the table for a layout parameter | +40 |
| `app/qml/ProjectEditor/Views/FieldLayoutEditor.qml` (new) | field table with live decode | +260 |
| `app/rcc/scripts/native/justfloat.md`, `field_layout.md` (new), `app/rcc/rcc.qrc` | template help | +152 |
| `app/tests/tst_justfloat.cpp`, `tst_field_layout.cpp` (new), `tst_quickplot_format_state.cpp`, `tst_quickplot_roundtrip.cpp`, `app/tests/CMakeLists.txt` | AC10-AC12 | +470 |
| `doc/help/Frame-Parser.md`, `doc/help/Operation-Modes.md` | manual | +38 |
| **Group D — small UI** | | |
| `core/Devices/IO/Drivers/UART/UartLineWatch.h` / `.cpp` (new) | viewer-gated 4 Hz poll of the modem lines | +60 / +110 |
| `core/Devices/IO/Drivers/UART.h` / `.cpp` | own the watch, expose four read-only properties | +10 / +14 |
| `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml` | four indicators | +70 |
| `core/Ui/UI/Widgets/MultiPlot.h` / `.cpp` | per-curve Y range captured in the existing scan | +14 / +55 |
| `app/qml/Widgets/Dashboard/MultiPlot.qml` | Lanes toggle, band geometry per curve, persistence | +110 |
| `app/qml/Widgets/PlotWidget/PlotLaneLabels.qml` (new) | name and range label per band | +90 |
| `app/tests/tst_multiplot_lanes.cpp` (new), `app/tests/CMakeLists.txt` | AC14 | +165 |
| `doc/help/Plots.md`, `doc/help/Drivers-UART.md`, `doc/help/Widget-Reference.md` | manual | +48 |
| **Shared** | | |
| `doc/claude/architecture/dashboard.md`, `doc/claude/TODO.md` | console-tier objects and their rules; deferrals | +45 |
| **Total** | code / tests / docs | **≈ 5,130 / 1,890 / 630** |

### Hotpath & threading impact

- **Touches the hotpath?** Not the publish path. Two adjacent surfaces:
  - *Built-In templates (group C)* run on the pipeline thread like every other template. Both
    are new `INativeParser` instances under the existing contract: the layout is parsed once
    when the parser is made, the output scratch is sized then, and a frame decode allocates
    nothing. No change to `FrameBuilder`, `FrameReader`, `CircularBuffer` or the span fast lane.
  - *Multi-Plot (group D)* is display-tick drawing on the GUI thread. The per-curve range is
    stored from the min/max the auto-scale scan already computes per curve, into an array sized
    at widget configuration. No second pass, no allocation, no per-view reduction.
  - `--benchmark-hotpath` is run by the maintainer after groups C and D (AC15). Groups A and B
    cannot move it: with their panels closed they execute one early return per chunk.
- **New cross-thread signal/slot?** No. Console objects live on the GUI thread and are called
  directly from the handler's tap entry points. The line watch lives on the UART driver's own
  thread beside the existing DTR/RTS properties.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — received messages use `CapturedData::timestamp`. Sent messages are
  stamped once in the monitor for display; the clock census will list that call site and needs
  an accepted re-seed, named in the commit.

### Data model, API and UI

- **Settings, app-wide:** `Console/SendPins` gains `enabled` and `delayMs` per pin (absent reads
  as enabled, 1000 ms, so existing pins keep working). New keys `Console/AutoReplyRules`,
  `Console/AutoReplyGapMs`, `Console/HighlightRules`, and a `ConsoleBusMonitor` settings
  category for the panel's view state. Nothing in the project schema, so no migration.
- **Project JSON:** the two new templates use the existing `frameParserTemplate` and
  `frameParserParams` fields. Field Layout's parameter is a `NativeParamType::Json` array.
  Multi-Plot's lane toggle is one more key in the widget's existing settings entry.
- **API:** no new commands, so no entries in the capability, safety or sealed tables.
- **UI:** three console ribbon toggles (Auto Reply, Colour Rules, Bus Monitor) and their panels,
  the extended command library popover, the Field Layout table inside the Built-In parser pane,
  four indicators in the serial setup pane, and one Multi-Plot toolbar button. Ribbon and
  toolbar icons go through the icon registry.

### Tradeoffs

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Where the monitor lives | console C++ objects; annotation script; listen-only driver | **Console C++.** The script cannot be Pro and misses R14; the driver puts sniffing on the pipeline. |
| Auto-reply match modes | pattern syntax only; add regular expressions | **Both** (decided): the Connection Sequence pattern syntax for text and hex, regular expressions for text rules. A regex runs on the GUI thread per line with no cost bound, so the message is capped at 4096 bytes and the rule editor says a slow expression slows the console. |
| Sealed sessions | same as manual send; hidden | **Hidden** (decided). A kiosk operator cannot set up automatic transmission. |
| What a "message" is for auto-reply | console chunk; parsed project frame; line or idle-gap packet | **Line or idle-gap packet.** Chunks split arbitrarily, and project frames exist only on the pipeline thread. |
| Rule storage | project file; app settings | **App settings** (decided). A loaded project can never arm a transmission. |
| Tier | all free; monitor Pro; automation Pro too | **Monitor and its timeline Pro**, the rest free (decided). |
| JustFloat reach | template; plus Quick Plot auto-detect | **Template** (decided). Auto-detect needs declared framing without a start sequence, which the current framing switch refuses. |
| Field Layout vs existing packed struct | extend packed struct; new template | **New template.** Packed struct is the device-declaration wire format with a fixed vocabulary; adding offsets, bit fields and scaling to it changes what firmware may declare. |
| Colour rule scope | recolour the whole scrollback on change; new text only | **New text only.** Recolouring 100k lines on the GUI thread per edit is a stall; Clear gives a clean slate. |
| Landing | one commit; four; nine | **Four, by group** (decided). |

### Risks

- **Automatic transmission.** Auto-reply and Loop send bytes without a click per send. Failure
  modes and their guards: an echoing device causes a reply storm (rate cap, R7); a reply reaches
  the wrong device after the user switches the console selector (a reply goes to the device its
  message came from, not the selected one); a rule set is left armed (never persisted, disarms
  on close).
- **Regex cost.** A backtracking expression can stall the GUI thread once per received line.
  The 4096-byte cap bounds the input, not the expression; this is an accepted risk, stated in
  the rule editor and the manual.
- **File ceilings.** `Terminal.cpp` has 25 lines of headroom and `UART.cpp` 36, `Handler.cpp`
  ends near 1464 of 1500 after both groups. New logic goes in new files; if a ceiling is hit the
  task stops rather than re-seeding the census.
- **Two console views.** The console pane and the dashboard console tool can both exist, so
  colour rules and the monitor's viewer gate are shared objects on the handler, not per view
  (the annotation layer's viewer-set rule).
- **Model churn.** The monitor stages messages and publishes once per UI tick, as the annotation
  model does; one model transaction per frame was the 2026-08-17 failure.
- **Licence-derived state.** The Bus Monitor toggle must follow activation changes, not read the
  licence once at load, or a late activation leaves it hidden.
- **Modem-line polling** is a system call per tick; it runs only while the port is open and the
  setup pane is visible.
- **Trial parity.** The Bus Monitor page must say the trial unlocks it or the lint blocks CI.
- **Size.** About 5,100 lines of code is the largest single spec this year; all nine features
  stay (decided 2026-11-21). The four commits are the mitigation: each group builds and is
  tested alone.

### Tests

- AC1-AC4: `tst_console_send_sequence`, `tst_console_auto_reply`, `tst_terminal_highlight`.
- AC5-AC8: `tst_modbus_rtu_analyzer` (the premise trace generator is ported into the test),
  `tst_bus_monitor`.
- AC10-AC12: `tst_justfloat`, `tst_field_layout`, added streams in
  `tst_quickplot_format_state` and `tst_quickplot_roundtrip`.
- AC14: `tst_multiplot_lanes`.
- AC9, AC13, AC14 (visual part): maintainer observation.
- AC15: `--benchmark-hotpath`, maintainer.
- All C++ units run through `ctest` on the maintainer's build; none is run by the agent beyond
  the per-file syntax check.

## Amendments after implementation (2026-11-21, awaiting re-confirmation)

The build diverged from the Design above in these places. The sections above are left as
approved so the difference stays readable.

- **Size.** Measured net lines: code 8,406, tests 3,172, docs 462, total 12,040 against the
  approved 7,650 (1.57 times), which is over the 1.5 limit: the spec is back to `draft`. Largest overruns: the Modbus analyser (plausibility-gated waiting
  and the idle resync pass, both added after the simulation lost frames without them), the bus
  monitor panel, and the tests.
- **Colour rules are drawn at paint time**, not written into the buffer. `TerminalBuffer` is not
  changed. The buffer keeps colour cells only while ANSI colours are on, and a rewrite would
  change what Copy and the Console Log see. Consequences: rules apply to the whole scrollback
  (the "new text only" tradeoff row no longer holds), and they are not drawn while Find is open
  or in a right-to-left layout.
- **Lane ranges.** The auto-scale scan accumulates one shared extent and is skipped when datasets
  declare ranges, so "the min/max the scan already computes per curve" did not exist. The scan
  now folds per-curve locals into the shared extent, and declared ranges are used when present.
- **Built-In templates** build one string row per frame, as the packed-struct donor does. "A
  frame decode allocates nothing" was wrong for both.
- **Clock census.** Unchanged at 16 reads, so no re-seed is needed. Sent messages are stamped in
  the console handler for display only.
- **Acceptance criteria stay unticked.** AC1 to AC8, AC10 to AC12 and AC14 each have their test
  written, but no test has run.
- **Files outside the approved list:** `core/Ui/Console/SendSequence.h` / `.cpp` and
  `AutoReplyRule.h` / `.cpp` (split out so the tests need no application singleton and
  `SendLibrary.cpp` stays small), `core/Ui/UI/Widgets/PlotLaneRanges.h`,
  `NativeTemplates/BinaryPackedStruct.h` / `.cpp` (one helper made public for JustFloat),
  `core/Ui/ProjectEditor/Editors/FrameParserModel.h`, `core/Pipeline/CMakeLists.txt`,
  `core/Devices/CMakeLists.txt`, `app/tests/tst_cframe_parser.cpp` (template count),
  `doc/help/Console-Annotations.md`.
- **File ceilings held:** `Handler.cpp` 1,495, `UART.cpp` 1,483, `Terminal.cpp` 1,988 (baseline
  1,997), `ConnectionManager.cpp` exactly 1,500.
- **Line watch.** The setup pane binds to the settings driver, which never opens a port, so the
  watch asks for the live driver of source 0 on each poll. That is one line in
  `core/Devices/IO/ConnectionManager.cpp`, outside the approved list.
- **Review (six read-only lenses) fixed before handoff:** the line watch above; an out-of-range
  row after a staging overflow in the monitor; chunks larger than the carry losing their tail;
  auto-reply sending before the triggering text was shown, accepting a malformed hexadecimal
  reply with a line ending, and a fixed-window rate limit (now sliding); "issues only" emptying
  the Messages table; a stale timeline tooltip; a 4 KB buffer pinned per Messages row.
- **Review findings left open:** a failed write still counts as a sent reply or list step; a
  running list follows the console's device selector; a master retry inside the timeout can be
  read as the slave's echo; big-endian field extraction runs one pass per bit (unmeasured); two
  unused parameters and unnamed Modbus limits in the monitor code.
- **Generated:** `app/rcc/ai/search_index.json` was rebuilt by `sanitize-commit.py`.
- **Icons.** The four new buttons borrow existing glyphs; the real ones are in `TODO.md`.
- **Unverified.** Nothing was built or run. The C++ passes the per-file syntax check and the
  linter; the analyser's logic was checked by a JavaScript port over the premise trace and every
  unit scenario. AC9, AC13, AC15 and the visual half of AC14 need the maintainer.

## Open Questions

None open. Resolved on 2026-11-21:

- Scope: all nine features stay.
- Auto-reply gets regular expressions on text rules.
- Auto Reply and Run list are hidden in a sealed deployment session.
- The "CAN Signal Map" and "Modbus Register Map" template help pages are orphans: both templates
  were added on 2026-06-04 and removed on 2026-06-11 when the importers went back to generating
  scripts. The pages and their resource entries are outside this spec's file list. The removed
  source (`MapTemplates.cpp` at commit `47dd6bd95`) is the donor for Field Layout's bit
  extractor.
