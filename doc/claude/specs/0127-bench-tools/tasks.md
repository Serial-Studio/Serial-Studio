---
spec: 0127-bench-tools
phase: tasks
updated: 2026-11-21
---

# Tasks 0127 — Bench tools: bus monitor, console automation, VOFA+ input and plot lanes

> **The ordered checklist.** `/ss-implement` decomposes the Design in [`spec.md`](./spec.md)
> into units that are small, ordered, and *individually verifiable*, works the list top to
> bottom and keeps the boxes current. No approval gate: the spec was the gate.

## Conventions

- One task = one focused, reviewable change.
- **Verify** is `python scripts/code-verify.py --check <files>` plus
  `python scripts/syntax-check.py <C++ files>` unless a task says more.
- Groups land as separate commits (A, B, C, D), so a group's tasks never edit another group's
  new files.

## Tasks

### Group C — input formats

#### T1 — JustFloat Built-In template

- **Files:** `NativeTemplates/BinaryJustFloat.h/.cpp`, `NativeTemplates/BinaryTemplates.cpp`,
  `app/rcc/scripts/native/justfloat.md`, `app/rcc/rcc.qrc`, `core/Pipeline/CMakeLists.txt`
- **Does:** decodes little-endian float32 frames, end marker optional. Pipeline thread: the
  parser allocates nothing per frame and sizes its scratch once.
- **Verify:** default; `tst_justfloat` (T3)
- **Deps:** none
- [x] done

#### T2 — Field Layout Built-In template

- **Files:** `NativeTemplates/BinaryFieldLayout.h/.cpp`, `NativeTemplates/BinaryTemplates.cpp`,
  `app/rcc/scripts/native/field_layout.md`, `app/rcc/rcc.qrc`
- **Does:** parses the JSON field table once per parser, extracts bounded bit fields per frame.
  A field past the frame end yields NaN, never a failed frame.
- **Verify:** default; `tst_field_layout` (T3)
- **Deps:** T1
- [x] done

#### T3 — Group C tests

- **Files:** `app/tests/tst_justfloat.cpp`, `tst_field_layout.cpp`,
  `tst_quickplot_format_state.cpp`, `tst_quickplot_roundtrip.cpp`, `app/tests/CMakeLists.txt`
- **Does:** AC10, AC11, AC12.
- **Verify:** syntax check with a donor compile entry
- **Deps:** T1, T2
- [x] done

#### T4 — Field Layout editor

- **Files:** `core/Ui/ProjectEditor/Editors/FrameParserModel.cpp` (+ header),
  `app/qml/ProjectEditor/Views/NativeParserPane.qml`, `FieldLayoutEditor.qml`,
  `app/CMakeLists.txt`
- **Does:** rows in and out of the JSON parameter, sample decode. Edits go through the existing
  `updateSource` path so undo keeps working.
- **Verify:** default
- **Deps:** T2
- [x] done

#### T5 — Group C manual

- **Files:** `doc/help/Frame-Parser.md`, `doc/help/Operation-Modes.md`
- **Verify:** `scripts/documentation-verify.py`
- **Deps:** T1, T2
- [x] done

### Group D — small UI

#### T6 — Serial control-line watch

- **Files:** `core/Devices/IO/Drivers/UART/UartLineWatch.h/.cpp`, `UART.h/.cpp`,
  `core/Devices/CMakeLists.txt`
- **Does:** viewer-gated 4 Hz poll of the modem lines. `UART.cpp` has 36 lines of headroom:
  only ownership and forwarding go there.
- **Verify:** default
- **Deps:** none
- [x] done

#### T7 — Control-line indicators

- **Files:** `app/qml/MainWindow/Panes/SetupPanes/Drivers/UART.qml`
- **Verify:** `code-verify --check`
- **Deps:** T6
- [x] done

#### T8 — Multi-Plot per-curve ranges

- **Files:** `core/Ui/UI/Widgets/MultiPlot.h/.cpp`
- **Does:** stores each curve's min/max from the scan that already runs. Display tick: no second
  pass, no allocation after configuration, no per-view reduction.
- **Verify:** default; `ss-hotpath` read first
- **Deps:** none
- [x] done

#### T9 — Multi-Plot lanes UI

- **Files:** `app/qml/Widgets/Dashboard/MultiPlot.qml`,
  `app/qml/Widgets/PlotWidget/PlotLaneLabels.qml`, `app/CMakeLists.txt`
- **Verify:** `code-verify --check`
- **Deps:** T8
- [x] done

#### T10 — Group D tests and manual

- **Files:** `app/tests/tst_multiplot_lanes.cpp`, `app/tests/CMakeLists.txt`, `doc/help/Plots.md`,
  `doc/help/Drivers-UART.md`, `doc/help/Widget-Reference.md`
- **Does:** AC14.
- **Deps:** T8, T9
- [x] done

### Group A — console automation

#### T11 — Send with an explicit spec; command list runner

- **Files:** `core/Ui/Console/Handler.h/.cpp`, `SendLibrary.h/.cpp`
- **Does:** per-pin `enabled`/`delayMs`, run/loop/stop. Stops on link close, excluded with
  cyclic, refused when sealed. `Handler.cpp` ceiling: helper only.
- **Deps:** none
- [x] done

#### T12 — Auto-reply

- **Files:** `core/Ui/Console/AutoReply.h/.cpp`, `Handler.h/.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** rule store, line and idle-gap message assembly, matching, rate cap. Never persisted
  armed, disarms on close, replies to the device the message came from, refused when sealed.
- **Deps:** T11
- [x] done

#### T13 — Colour rules

- **Files:** `core/Ui/Console/HighlightRules.h/.cpp`,
  `core/Ui/UI/Widgets/Terminal/TerminalHighlighter.h/.cpp`, `TerminalBuffer.h/.cpp`,
  `Terminal.h/.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** colours the rows an append touched. `Terminal.cpp` has 25 lines of headroom. Stored
  characters never change.
- **Deps:** none
- [x] done

#### T14 — Group A UI

- **Files:** `ConsoleSendLibrary.qml`, `ConsoleAutoReply.qml`, `ConsoleHighlightRules.qml`,
  `Terminal.qml`, `app/CMakeLists.txt`
- **Deps:** T11, T12, T13
- [x] done

#### T15 — Group A tests and manual

- **Files:** `tst_console_send_sequence.cpp`, `tst_console_auto_reply.cpp`,
  `tst_terminal_highlight.cpp`, `app/tests/CMakeLists.txt`, `doc/help/Console-Automation.md`,
  `doc/help/help.json`, `doc/help/Toolbar-Reference.md`
- **Does:** AC1 to AC4.
- **Deps:** T11 to T14
- [x] done

### Group B — bus monitor (Pro)

#### T16 — Modbus RTU analyser

- **Files:** `core/Ui/Console/ModbusRtuAnalyzer.h/.cpp`
- **Does:** framing, CRC, roles, pairing, diagnoses, data summary. Pure, no Qt object: testable
  without the app. Commercial SPDX.
- **Deps:** none
- [x] done

#### T17 — Bus monitor store

- **Files:** `core/Ui/Console/BusMonitor.h/.cpp`, `Handler.h/.cpp`, `core/Ui/CMakeLists.txt`
- **Does:** message store, table model, timeline geometry, idle-gap splitter, viewer gate.
  Stages per chunk and publishes once per UI tick. Received time is the chunk's own stamp.
- **Deps:** T16
- [x] done

#### T18 — Bus monitor panel

- **Files:** `ConsoleBusMonitor.qml`, `Terminal.qml`, `app/CMakeLists.txt`
- **Does:** table, painted timeline, controls. The toggle follows licence changes.
- **Deps:** T17
- [x] done

#### T19 — Group B tests and manual

- **Files:** `tst_modbus_rtu_analyzer.cpp`, `tst_bus_monitor.cpp`, `app/tests/CMakeLists.txt`,
  `doc/help/Bus-Monitor.md`, `doc/help/help.json`, `doc/help/Drivers-Modbus.md`
- **Does:** AC5 to AC8.
- **Deps:** T16 to T18
- [x] done

### Shared

#### T20 — Architecture notes and deferrals

- **Files:** `doc/claude/architecture/dashboard.md`, `doc/claude/TODO.md`
- **Deps:** all
- [x] done

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met and checked off there.
- [ ] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [ ] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [ ] `ss-hotpath` checks pass / `--benchmark-hotpath` not regressed (if hotpath touched).
- [ ] Relevant `pytest` tests identified for the maintainer to run (listed in `spec.md`).
- [ ] `python scripts/sanitize-commit.py` run, only if a generator input changed.
- [ ] Diff is *what was asked, and only that* — no scope creep, no foreign files touched.
- [ ] `spec.md` status set to `done`.
