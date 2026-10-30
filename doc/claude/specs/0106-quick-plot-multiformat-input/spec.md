---
spec: 0106-quick-plot-multiformat-input
title: Quick Plot multiformat input
status: done         # draft -> approved -> in-progress -> done (2026-10-30)
created: 2026-10-30
author: Alex Spataru
---

# Spec 0106 — Quick Plot multiformat input

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Quick Plot accepts exactly one line shape: comma-separated numbers. Everything else a
first-run user is likely to print produces a broken dashboard, and spec 0100 made first-run
users the audience of this mode:

- **Other separators.** Sketches print with spaces, tabs and semicolons at least as often as
  with commas; the Arduino IDE's own Serial Plotter accepts comma, space and tab, and
  `label:value` pairs. European firmware prints decimal commas. Each of these shapes gives a
  one-channel dashboard that shows text, or nothing.
- **Labels are fragile.** Channel titles come only from a header row received while the
  header check is armed. The manual documents the failure itself: a device that is already
  streaming when Connect is pressed loses its labels for the session. A device that puts the
  name on every line (`temp:23.5,hum:60`) carries no such risk, and that is the most common
  labelled shape in the wild.
- **Units never arrive.** Datasets have a unit field and the widgets render it, but Quick
  Plot has no way to fill it. A device that prints `23.5°C` today yields a text field, not a
  value with a unit.
- **Debug lines churn the dashboard.** After data has started, any line without commas is
  a one-field frame. A single `WiFi connected` rebuilds the whole dashboard down to one
  channel, and the next data line rebuilds it back. The line is already visible in the
  console; the dashboard gains nothing from it.
- **The app's own default framing is rejected.** Project File mode frames with `/*` and `*/`
  by default, and two bundled examples emit it. A user who follows the project tutorial and
  then tries Quick Plot sees `/*23.5` as a text channel.

The Getting Started page's claim that non-numeric text "blocks parsing" was wrong as well
(fixed 2026-10-30, independently of this spec): such text is kept as a string field.

## Goals

- A device printing numbers in any of the common one-line shapes gets a correct dashboard
  without a project file and without touching a setting.
- Names and units carried on the wire become channel titles and units.
- Debug and status lines pass through to the console without disturbing the dashboard.
- Every Quick Plot session can still be saved as a project that parses the same stream.
- Streams that work today keep working exactly as they do today.

## Non-Goals

- **No manual format override in the UI.** Detection plus "Save as Project" is the contract
  (maintainer decision, 2026-10-30). A stream the detector cannot settle is a project-file
  stream.
- No binary, hexadecimal, base64 or fixed-width input; no multi-line JSON; no CSV quoting or
  escaping rules (the CSV player owns those).
- No device-to-dashboard configuration protocol: widget type, ranges and colours stay in the
  project editor.
- No per-channel configuration inside Quick Plot (ranges, alarms, widget choice): the
  documented limitation stands.
- No change to the frame boundary: one line, ended by CR, LF or CRLF, is one frame.
- No change to Project File mode parsing or to the Built-In parser templates' behavior.
- No locale-driven behavior: a decimal comma is inferred from the stream, never from the
  operating system.

## Requirements

1. **R1 — Separator detection.** A line of values separated by commas, semicolons, tabs or
   runs of spaces is split into one field per value. The separator is determined from the
   stream; nothing is configured.
2. **R2 — Decimal comma.** When the detected separator is a semicolon or a space, `23,5`
   reads as 23.5.
3. **R3 — Named values.** A field of the form `name:value` or `name=value`, with optional
   whitespace after the colon or equals sign, yields a channel titled `name`. In a named
   stream channels are identified by name, not position: a line that omits a name or
   reorders names never shifts a value into another channel. A name may contain spaces when
   the separator is not a space. A line may mix named and unnamed fields; unnamed fields keep
   their positional title. A named field whose value is not a number (`status:OK`) yields a
   text channel: shown in the data grid, never plotted.
4. **R4 — Units.** A numeric field with a short trailing suffix (`23.5°C`, `1013hPa`, `45%`,
   `3.3V`, `9.81m/s²`) yields the number as the value and the suffix as the channel's unit.
   The unit is taken when the channel is established and appears wherever the dashboard
   shows units today. A later line's suffix never changes the unit.
5. **R5 — One-line JSON.** A line that is a JSON object yields one channel per numeric leaf.
   Nested objects flatten to dotted names (`imu.ax`), arrays to indexed names (`acc[0]`,
   `acc[1]`); strings, booleans and nulls are ignored. Channel order follows the order of
   members in the line, not alphabetical order. An array index not seen before is a new name
   under R11; an index that disappears keeps its channel and receives no sample.
6. **R6 — Wrapper strip.** A line that starts with `/*` and ends with `*/` is parsed as its
   contents under R1 to R5.
7. **R7 — Text lines never touch the dashboard.** Once a structure exists, a line that yields
   no numeric field leaves the dashboard unchanged: no rebuild, no value update. The line
   still appears in the console as it does today.
8. **R8 — Header rows keep working.** For a positional stream, an all-text line received
   before the first qualifying line still provides the channel titles, as today, provided it
   has as many fields as the data line that establishes the format; boot chatter with a
   different field count is a text line, not a header. Names carried on the data lines take
   precedence over header titles. *(Clarified in planning, 2026-10-30.)*
9. **R9 — Format stability.** The format (separator, decimal comma, named or positional,
   JSON, wrapper) is established once from the first qualifying line and then fixed. It is
   re-evaluated only when a line yields no numeric field under the fixed format but
   qualifies under a fresh detection. It re-arms on every connect and on every operation-mode
   change, like the header check today.
10. **R10 — A qualifying line.** A line establishes the format only when at least half of its
    fields are numeric or named-numeric. Boot chatter such as `Connected in 1234 ms` or
    `IP: 192.168.1.42` never establishes a format and is treated as a text line.
11. **R11 — Structure changes.** A positional stream rebuilds the dashboard when the field
    count changes, as today. A named or JSON stream rebuilds only when a name appears that the
    structure does not have. A line that omits a known name keeps that channel and gives it
    no sample for that frame: plots show a gap, the data grid keeps the last value.
12. **R12 — Save as Project reproduces the stream.** The project written from a Quick Plot
    session parses the same stream to the same titles, units and values as the live session
    did: separator, decimal comma, named pairs, JSON flattening and wrapper included. One
    documented difference: a line that omits a known name repeats the last value in the saved
    project, as every key-addressed Built-In template does, where the live session gives no
    sample (R11). *(Clarified in planning, 2026-10-30.)*
13. **R13 — The first-run surfaces say so.** The mode label becomes **Quick Plot
    (Auto-Detected Format)**. Its tooltip, the console's no-data hint (the start hint names no
    format today and stays as it is), the Operation Modes and Getting Started help pages, the
    CLI help, the API command descriptions and the assistant's corpus describe the accepted
    shapes with one example each, and the terminology gate passes. *(Clarified in planning,
    2026-10-30.)*
14. **R14 — The first-run example leads with names.** The Getting Started page's primary
    Quick Plot example becomes `name:value` pairs (`temp:23.5, hum:60`); the comma-only and
    header-row shapes remain as secondary examples.

## Acceptance Criteria

- [x] **AC1 — Corpus test.** The 35-stream corpus below runs as an automated unit test
      against the real parser: every stream yields the expected sequence of (title, value)
      pairs per line, text lines yield nothing, and the number of structure rebuilds after
      the first structure equals the expected count (4 across the corpus, each a real
      channel-set change). Covers R1 to R11.
- [x] **AC2 — Labels survive a late Connect.** With a device already printing
      `temp: 23.5, hum: 60` lines, pressing Connect shows two plots titled `temp` and `hum`
      with no header row ever sent. In-app observation.
- [x] **AC3 — Units render.** `temp:23.5°C;pressure:1013hPa` shows `°C` and `hPa` in the
      data grid and on the plots, where units render today. In-app observation.
- [x] **AC4 — Debug line is inert.** Interleaving `WiFi reconnecting...` between data lines
      leaves the widget set unchanged (no rebuild, no flicker) and the line is visible in the
      console. In-app observation, plus the dashboard structure reported over the API server
      does not change across the line.
- [x] **AC5 — Save as Project round-trip.** For each format in the corpus, the project written
      by Save as Project, loaded in Project File mode and fed the same lines through the
      project dry-run API, yields identical titles, units and values. `pytest` integration.
- [x] **AC6 — Hotpath gate.** `--benchmark-hotpath` passes at its default threshold after the
      change. The comma-only path pays no per-line detection cost.
- [x] **AC7 — Surfaces and gates.** Label, tooltip, start hint, both help pages, CLI help and
      assistant corpus carry the new contract; `term-verify`, `claim-verify` and
      `documentation-verify` pass. Covers R13 and R14.
- [x] **AC8 — Replay unchanged.** A recorded Quick Plot session and a CSV file replay with the
      same structure and values as before this change. Existing player tests stay green.

### Corpus for AC1

One stream per line group; `→` gives the expected channels. Text lines expect nothing.

```
23.5,60,1013                      → 3 positional
23.5, 60, 1013                    → 3 positional
23.5;60;1013                      → 3 positional
23,5;60;1013,2                    → 23.5 60 1013.2
23.5<TAB>60<TAB>1013              → 3 positional
23.5 60 1013                      → 3 positional
23,5 60,2 1013                    → 23.5 60.2 1013
temp:23.5,hum:60                  → temp hum
temp: 23.5, hum: 60               → temp hum
temp=23.5;hum=60                  → temp hum
temp:23.5 hum:60                  → temp hum
temp:23.5°C;pressure:1013hPa      → temp[°C] pressure[hPa]
3.3V,25°C,45%                     → 3 positional with units V °C %
{"t":23.5,"h":60}                 → t h
{"t":23.5,"ok":true,"n":"x","h":60} → t h
{"imu":{"ax":0.1,"g":[1,2]},"t":5} → imu.ax imu.g[0] imu.g[1] t
/*23.5,60,1013*/                  → 3 positional
Temperature,Pressure,Humidity ; 23.5,1013.2,45          → header titles, then 3 values
WiFi connected ; IP: 192.168.1.42 ; Connected in 1234 ms ; 23.5,60,1013 → 3 positional
uptime=1234 ; temp=23.5,hum=60    → uptime, then temp hum (one rebuild)
23.5,60 ; WiFi reconnecting... ; 23.6,61                → 2 positional, inert text line
23.5,60 ; RSSI -67 dBm ; 23.6,61  → 2 positional, inert text line
-12.5,1.5e3,+7                    → -12.5 1500 7
23.5,60,1013,                     → 3 values, trailing empty field
12:30:45,23.5,60                  → text field, 23.5, 60
Temp:23.5<TAB>Hum:60              → Temp Hum
temp:23.5,60                      → temp, Channel 2
temp:23.5,status:OK               → temp, text channel "status"
{"acc":[1,2]} ; {"acc":[1,2,3]}   → acc[0] acc[1], then acc[2] added (one rebuild)
0x1F,0x20                         → text (hexadecimal is a non-goal)
hello ; world                     → text, text
temp:23,5;hum:60                  → temp hum (decimal comma)
Temp Sensor: 23.5, Humidity: 60   → "Temp Sensor" "Humidity"
23.5,60 ; temp:23.5;hum:60        → 2 positional, then temp hum (one rebuild)
60,1013 ; 23.5,60,1013            → 2 then 3 positional (one rebuild)
```

## Constraints & Invariants

- **One line is one frame.** The CR, LF and CRLF boundary and the line-based detection are
  untouched.
- **Detection cost is paid at establishment, not per line.** The format is settled at most once
  per connect plus the failure-triggered re-evaluations of R9; a line that parses under the
  fixed format never runs a candidate scan. No new per-line allocation beyond what the split
  does today. The 256 kHz gate stays green.
- **A text line never rebuilds the dashboard.** Rebuilds happen only on a real channel-set
  change (R11).
- **Device order is channel order**, including JSON members. Any representation that sorts
  keys must not decide the order.
- **Save as Project reproduces the live parse** (spec 0044 session reproducibility, spec 0100's
  Save as Project promise).
- **Playback is untouched.** Recorded sessions and CSV files keep replaying through the existing
  normalized path with no behavior change.
- **Spec 0100's first-run surfaces stay consistent with each other**: label, tooltip, start
  hint and help describe the same contract, and one name per concept holds under the
  terminology gate.
- **Positional comma streams keep today's contract exactly**: comma split, header row, "Channel
  N" titles, empty fields kept. Existing users see no change.
- **No new dependency.** JSON is parsed with what the app already ships.
- **Nothing here is Pro-gated.** Quick Plot stays available in every edition.

## Premise Check

- **Question:** Can a format fixed once per connect, with no per-line re-detection, parse
  realistic device streams correctly, including streams with boot chatter before the data,
  without rebuilding the dashboard line by line?
- **Measured:** The 35-stream corpus above was run through a throwaway simulator of the
  proposed rules (scratchpad, 2026-10-30) under four policies. Fix the format on the first
  line containing any number: 32/35 correct (boot chatter such as `Connected in 1234 ms`
  captures the format and the data that follows is treated as text). Fix on the first line
  with at least half its fields numeric: 33/35 (a well-formed boot line, `uptime=1234`,
  captures it, and a mid-stream format switch is lost). The same plus one re-evaluation when
  the fixed format yields no numeric field and a fresh detection qualifies: 35/35 with 4
  structure rebuilds, each a real channel-set change. Per-line detection: also 35/35 with
  the same 4 rebuilds, so it buys no accuracy over the re-evaluation rule on this corpus
  while scanning every line.
- **Verdict:** The literal premise (fix once, never re-evaluate) dies on boot chatter. It
  survives as "fix once, re-evaluate only on total failure", which is what R9 and R10 state;
  the corpus becomes AC1.

## Open Questions

None open. The five items raised in draft were resolved by the maintainer on 2026-10-30 and
folded into the requirements: a missing named field gives no sample for that frame (R11); the
first unit stays (R4); a named text value is a text channel (R3); a JSON array grows by
rebuilding once per new index (R5); the Getting Started example leads with `name:value` pairs
(R14).
