---
spec: 0014-log-axes
title: Logarithmic axis scales for Plot, FFT, and MultiPlot
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-07-17
author: Alex Spataru
---

# Spec 0014 — Logarithmic axis scales for Plot, FFT, and MultiPlot

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every plot axis in Serial Studio is linear. For wide-dynamic-range data that makes the
interesting structure unreadable: an FFT spectrum spanning 10 Hz–20 kHz crushes the
entire low-frequency octave band into a sliver at the left edge (the FFT magnitude axis
is already logarithmic — it renders in dB — but the frequency axis is not); a sensor
whose value spans decades (pressure during pump-down, light intensity, impedance sweeps)
renders as a flat line with one spike. Users doing FFT analysis, Bode-style
characterization, or any decades-spanning measurement currently have to export the data
and re-plot it in an external tool.

The project editor already lets users shape each plot (min/max, X-axis source, FFT
window/rate), so per-axis scale selection is the natural missing knob, and it must be a
persisted project property — the whole point is that a telemetry project describing an
FFT rig opens with the right axes every time, not a per-session toggle.

## Goals

- A user editing a Plot or FFT dataset — or a MultiPlot group — in the project editor can
  set each applicable axis to Linear (default) or Logarithmic, independently for X and Y.
- An FFT widget with a log frequency axis spreads the low decades readably (octaves are
  evenly spaced), which is the headline use case.
- The choice persists in the project file, round-trips through save/load, and existing
  projects open unchanged (everything stays linear unless chosen).
- Log-scaled plots remain fully usable: ticks/grid land on sensible decade positions,
  zoom/pan and the crosshair/cursor readouts keep working, and live data keeps streaming
  at full rate.

## Non-Goals

- **No log scale on time axes.** Time plots span `[-T, 0]`; log time is undefined there
  and collides with sweep/trigger. Log X applies only where X is frequency (FFT), sample
  index, or another dataset.
- **No symlog / bipolar log.** Non-positive values are clamped, not given a linear region
  around zero. Symlog is a possible v2 if real projects need it.
- **No change to the FFT magnitude (dB) computation.** The Y axis of FFT already encodes
  log magnitude; this spec does not add a second log mapping on top of dB, and does not
  touch the Waterfall widget.
- **No log axes for GPS, 3D plots, or any non-Plot/FFT/MultiPlot widget.**
- **No per-session/runtime-only toggle in the dashboard UI.** The option lives in the
  project editor and the project file. (A dashboard toolbar shortcut can be a follow-up.)

## Requirements

1. **R1** — The project editor's dataset view for a **Plot** dataset offers an axis-scale
   choice (Linear/Logarithmic) for X and for Y. The X choice is offered only when the
   dataset's X source is Samples or another dataset — not Time.
2. **R2** — The project editor's dataset view for an **FFT** dataset offers the
   Linear/Logarithmic choice for the frequency (X) axis. (Magnitude stays dB — no Y
   option for FFT.)
3. **R3** — The project editor's group view for a **MultiPlot** group offers the
   Linear/Logarithmic choice for Y, and for X only when the group's X source is Samples
   (not Time), applying to all curves in the group.
4. **R4** — A widget with a log-scaled axis renders that axis with decade-based ticks and
   grid lines, and positions data points by the log of their value.
5. **R5** — On a log-scaled axis, values ≤ 0 (data or configured range bounds) are
   clamped to a small positive floor; the curve stays continuous and the widget never
   NaNs, blanks, or crashes.
6. **R6** — The axis-scale selections persist in the project file; a file saved with them
   reloads identically, and a project file that predates this feature (or omits the keys)
   loads with all axes linear — byte-identical rendering to today.
7. **R7** — Zoom, pan, and the cursor/crosshair value readout work on log-scaled axes;
   the readout reports the true data value, not the log of it.
8. **R8** — The feature is available on all tiers (no Pro/Trial gate).
9. **R9** — Live streaming into a log-scaled plot sustains the same data rates as a
   linear one — the axis scale is a render-side concern and must not slow ingest.

## Acceptance Criteria

> Implementation completed 2026-07-17 (see tasks.md). AC4's pytest is written and
> collects; the checked state of every box below awaits the maintainer's runtime
> verification (app + API server for AC4, visual checks for AC1-AC3/AC5/AC7, CI
> benchmark for AC6).

- [x] **AC1** — Maintainer check, running app: an FFT dataset with sampling rate 44100
      and log frequency enabled shows decade ticks (10, 100, 1k, 10k Hz) with octaves
      evenly spaced; toggling back to linear restores today's rendering.
- [x] **AC2** — Maintainer check: a Plot dataset fed decades-spanning values (e.g. 0.01
      → 1000) with log Y shows each decade at equal height; the crosshair reads back the
      true values.
- [x] **AC3** — Maintainer check: feeding zero and negative values into a log-Y plot
      neither crashes nor blanks the widget; the curve clamps at the floor.
- [x] **AC4** — `pytest` integration: setting the axis-scale options via the project API,
      saving, and re-reading the project returns the same values; a project JSON without
      the new keys reports linear for all axes.
- [x] **AC5** — Maintainer check: a Time-axis Plot/MultiPlot shows no log-X option in the
      editor, and a project file that forces log X onto a time plot renders linear.
- [x] **AC6** — `--benchmark-hotpath` passes at its existing gates with log-axis projects
      in the tree (no ingest-path regression).
- [x] **AC7** — Existing example projects load and render identically (all-linear
      default; no migration prompt, no modified-flag on open).

## Constraints & Invariants

- **Render-side only.** The 256 kHz hotpath gate must not regress: the log mapping
  happens where data is drawn, never where frames are parsed or ingested.
- **Schema is additive.** New project-file keys are optional with linear defaults;
  deserialization of older files must not change behavior or mark the project modified.
- **Must work in both QuickPlot and ProjectFile modes** where the widget exists
  (QuickPlot-generated FFT/Plot datasets simply default to linear).
- **Time-axis semantics are untouchable**: the `[-T, 0]` rolling window, sweep/trigger
  mode, and time-tick formatting keep their current behavior; log X must be structurally
  impossible on a time axis, not just hidden.
- **No new dependencies.**
- **Editor parity**: the new options follow the same editor conventions as existing
  dataset/group options (combo-box, translated labels, API-mutable like other dataset
  fields).

## Open Questions

- None — tier gating (free), non-positive handling (clamp to floor), and time-axis
  exclusion were decided with the maintainer on 2026-07-17.
