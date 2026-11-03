---
spec: 0111-device-declaration-library
title: Device Declarations and the Firmware Library
status: approved     # approved 2026-11-02: main repo + CI publish, MIT, Quick Plot-only request, full scope incl. binary
approved: 2026-11-02
created: 2026-11-02
author: Alex Spataru
---

# Spec 0111 — Device Declarations and the Firmware Library

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every importer in the app builds a whole project from a file some vendor wrote: a DBC for CAN,
a register map for Modbus, a `.proto` for Protobuf, a birth certificate for Sparkplug. The one
device with no file format is the user's own firmware, and it is the only device the issue
tracker talks about. The sketch already knows every channel's name, unit, range, which widget it
deserves and how its channels group. Today that knowledge is retyped by hand in the Project
Editor, and the closed issues about customising the layout or editing the project file are
people paying that tax.

Spec 0106 built half the bridge: names and units now ride the wire into Quick Plot. The other
half is missing. A sketch has no way to say "these three are an accelerometer", "this one is a
gauge from 0 to 100", "this code means Reverse", or "I accept a setpoint between 0 and 255".
The gap is measurable: the MPU6050 example's project file is 10,045 bytes, and the declaration
needed to produce it from the sketch's own knowledge is 467 bytes.

The audience is the one Serial Studio is known for, and the first hour of use is where it is
won or lost. The goal is that the first hour never opens the Project Editor at all: the sketch
declares, the dashboard builds itself on the first connect, and the Project Editor becomes the
tool for the second week.

## Goals

- A device can declare its dashboard on the wire, once, and the dashboard builds itself on
  connect without a project file and without the Project Editor.
- A firmware library for Arduino, plain C (ESP-IDF, Zephyr, anything with a UART) and Python
  emits the declaration and the frames, so a sketch declares a channel in the line where it
  already prints it.
- The declaration vocabulary is the dataset and group property registry, formats and labels of
  spec 0108 included. There is nothing to learn that the Project Editor does not already show,
  and nothing on the wire the editor cannot edit afterwards.
- The generated project is a real project: saveable, editable, and identical to what the Project
  Editor would produce for the same choices.
- A device may declare the controls it accepts, and the library parses the resulting commands
  for the sketch, with spec 0110 supplying the reply.
- A device that does not declare anything is a Quick Plot stream exactly as today, and a
  pre-spec app treats a declaration as a harmless text line.
- A device that sends packed binary frames declares their layout the same way, so a binary
  device gets the same first-connect dashboard as a text one.

## Non-Goals

- No code on the wire. A declaration never carries a transform, a parser script, a transmit
  script or a command; it carries properties the registry already defines.
- No device-stored project, no project synchronisation, no firmware update path.
- No library for every ecosystem. One C++ header for Arduino, one C file for everything else,
  one Python module. Other languages follow the same wire format later.
- Not a replacement for the importers or for Quick Plot's detection; it sits beside both.

## Requirements

1. **R1 — The declaration.** A device may send a declaration that names the project title,
   its groups with their widget, and its datasets with any property the dataset registry holds
   for a dataset: title, units, widget, range, plot and FFT flags, LED threshold, alarm bands,
   format and labels (spec 0108). Dataset order in the declaration is frame order. A property
   that is not in the registry does not exist on the wire.
2. **R2 — Self-building dashboard.** In Quick Plot, a valid declaration replaces detection: the
   live dashboard is built from it, and Save as Project writes the project it describes. No
   operation-mode switch is involved (spec 0100): the declared dashboard is a Quick Plot session
   whose structure came from the device instead of from the stream.
3. **R3 — Never over a user's project.** In Project File mode a declaration never rebuilds the
   loaded project. If it differs from the loaded project the app says so once, with the choice
   to adopt it or keep the project; adopting is an ordinary project load the user can undo.
4. **R4 — Re-declaration.** A device that sends its declaration again (reset, reconnect, a
   request from the app) with identical content changes nothing. A changed declaration rebuilds
   the Quick Plot dashboard as a structure change does today. Every declaration is visible in
   the console.
5. **R5 — Frames after a declaration.** A frame carries values in declared order. A declared
   stream may also use the named form of spec 0106, in which case channels map by name and a
   missing name yields no sample for that frame.
6. **R6 — The app asks.** On connect in Quick Plot, and only there, the app sends one short
   request line for the declaration, and the library answers it; the Setup panel can turn the
   request off. Console Only and Project File mode never send it. A device that declared at boot, before the app was
   listening, is therefore still discovered, including after the device resets.
7. **R7 — Declared controls.** A device may declare the controls it accepts: button, toggle,
   slider, knob and numeric text field with title, range, step and units (spec 0109's fields).
   The generated project holds an Output Panel using one documented command format, and the
   library parses that format and hands the sketch the control's name and value. Commands are
   in the first version; once spec 0110 lands, the library echoes the token and replies with the
   sketch's result.
8. **R8 — The library.** A sketch declares with one call per group and per channel and sends
   with one call per frame. The Arduino header allocates nothing at runtime, keeps declaration
   text in flash, has no dependency beyond the core, and the MPU6050 example fits a classic Uno
   with room to spare. The C variant is C99 with no RTOS glue; the Python module has no
   dependency beyond the standard library.
9. **R9 — Fidelity.** Declaring a dashboard and saving it yields the same project the Project
   Editor would produce for the same choices, and the saved project parses the same stream to
   the same titles, units and values as the live session did (spec 0106 R12 extended).
10. **R10 — Safety.** A declaration is data: titles and labels are bounded in length, the number
    of groups, datasets and labels is capped, a malformed or oversized declaration is ignored with
    a console message, and a declaration never writes a file, changes a connection setting or
    changes the operation mode. A declaration from a network Device is treated no differently
    from one over UART.
11. **R11 — Versioning.** The declaration carries a version. The app accepts the versions it
    knows and ignores an unknown one with a console message. The line is shaped so that spec
    0106's detector and a pre-spec app treat it as a text line that never establishes a format.
12. **R12 — Examples and manual.** The MPU6050 and TinyGPS examples gain declared variants of
    their sketches, the Getting Started page leads with the declaration for firmware authors, one
    manual page documents the wire format and the library calls, the assistant's corpus knows it,
    and the terminology gate passes.
13. **R13 — Distribution.** The library lives in this repository in its own directory, so the
    wire format, the registry and the library change in one commit, and CI publishes it on
    release the way its users install libraries: the Arduino Library Manager and PyPI, with the C
    file as a plain download. The examples pin the version they were written against.
14. **R14 — Binary frames.** A declaration may carry the Built-In parser's binary layout for the
    datasets it declares (framing, field types, widths and byte order), and the library emits
    packed frames that match it. The declaration itself is always a text line, and R2 to R11
    hold for a binary device exactly as for a text one.

## Acceptance Criteria

- [ ] **AC1** — An integration test drives the Python library over a loopback network Device,
  reads the dashboard structure back over the API and finds the declared groups, widgets,
  titles, units and ranges; Save as Project then yields a project that parses the same stream
  identically.
- [ ] **AC2** — A unit test feeds a malformed, an oversized and an unknown-version declaration
  and observes no rebuild; feeds the same declaration twice and observes one build; feeds a
  changed one and observes a second build.
- [ ] **AC3** — An integration test loads a project, sends a differing declaration and observes
  a notification with no change to the loaded project; adopting it loads the declared project.
- [ ] **AC4** — An integration test disconnects and reconnects with the Python library declaring
  only at start-up, and finds the dashboard built after the reconnect through the app's request.
- [ ] **AC5** — An integration test declares a slider, drives it through the driving surface and
  observes the Python library's callback receiving the control's name and the value.
- [ ] **AC6** — Maintainer observation: the declared MPU6050 sketch on an Uno builds the same
  three-group dashboard the example project file produces, and the compiler's RAM report stays
  under half the Uno's RAM.
- [ ] **AC7** — The hotpath gate (`--benchmark-hotpath`) reports no regression; the declaration
  is handled on the text-line path and frames are unchanged.
- [ ] **AC8** — A declaration line sent to the current release shows in the console and leaves
  the Quick Plot dashboard unchanged.
- [ ] **AC9** — The registry gate fails when a property is added to the wire format without
  being in the dataset registry, or added to the registry without a wire form.
- [ ] **AC10** — An integration test declares a binary layout through the Python library, sends
  packed frames, and reads back the same titles, units and values the text variant produced.

## Constraints & Invariants

- One vocabulary. The wire format, the library's calls and the Project Editor are three views of
  the dataset and group property registry, generated from it where generation exists today. A
  second schema language is the failure mode this spec exists to avoid, and the registry gate
  of AC9 enforces it.
- No code on the wire, ever. Output controls in a generated project use an app-side template
  the device never sees.
- Quick Plot's zero-setup path and every existing stream behave exactly as today; the
  declaration adds a branch, it does not replace detection.
- No operation-mode switch from a device line; a declaration lives inside Quick Plot and is only
  ever adopted into Project File mode by the user.
- The declaration is parsed off the per-frame path, at most once per content change; frames
  stay on the span fast lane.
- Trial parity: a declared dashboard is free; declared controls are Pro exactly as output
  controls are, and the manual says so with the trial named.
- The library is MIT-licensed, so it can be embedded in any firmware, including closed
  commercial firmware, without the app's dual license reaching into the user's product.

## Premise Check

- **Question:** How much of a real UART project can the firmware declare with the registry's
  vocabulary alone, and does the declaration fit a constrained MCU and a single serial line?
- **Measured:** From the examples tree, the declaration needed to reproduce each project's
  titles, units, widgets, ranges and flags, in a compact text form:

  | Example | Groups | Datasets | Project file | Declaration |
  |---|---|---|---|---|
  | MPU6050 | 3 | 7 | 10,045 bytes | 467 bytes |
  | TinyGPS | 1 | 3 | 5,821 bytes | 151 bytes |
  | HexadecimalADC | 2 | 12 | 15,884 bytes | 524 bytes |

  Every property those three projects use is in the registry; none needs code. The largest
  declaration is under one serial line at 115200 baud (about 45 ms) and fits in flash on an Uno
  with the sketch's own strings. Binary layouts (R14) were not measured: all three examples are
  text streams, and the plan carries the descriptor-size check for a binary device.
- **Verdict:** The vocabulary covers the UART examples completely and the cost on the device is
  negligible; proceed. Adoption is unmeasurable in advance: the only signals are recurring
  users and 151 sales, so the examples and the Getting Started page are part of the deliverable,
  not a follow-up.

## Open Questions

None at the gate. The draft's six questions (where the library lives, its license, the
connect-time request, alarm bands and LED thresholds, declared controls before spec 0110, binary
frames) were decided by the maintainer on 2026-11-02 and are folded into R1, R6, R7, R13, R14
and the constraints above. The one that widened the scope is binary frames: the maintainer chose
the full scope for the first version, which makes the Built-In descriptor part of the wire
vocabulary and the C library a packed-frame emitter as well as a text one.
