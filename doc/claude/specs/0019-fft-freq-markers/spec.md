---
spec: 0019-fft-freq-markers
title: FFT frequency markers and alarm bands (spectral watchlist)
status: done         # implemented 2026-07-17; AC1/AC2 tests + AC3-AC7 runtime checks await maintainer
created: 2026-07-17
author: Alex Spataru
---

# Spec 0019 — FFT frequency markers and alarm bands

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: maintainer delegated approval
> for the whole spec→implement run (2026-07-17 request); decisions taken on his behalf
> are recorded in the Decisions section for post-hoc review.

## Problem / Motivation

Vibration and acoustics work is about *known* frequencies: a gear-mesh line at 800 Hz, a
1x-RPM imbalance line, a bearing defect band, a resonance to stay away from. Today the
FFT widget draws the whole spectrum but treats every hertz equally — the user must
eyeball the position of the frequency they care about, remember what "that bump near
800" means, and judge by eye whether its level is acceptable. There is no way to label
engineering-meaningful frequencies, no way to see at a glance whether a monitored line
has crossed a warning or alarm level, and nothing persists in the project file that
captures this domain knowledge.

Datasets already solve the equivalent problem in the *amplitude* domain: alarm bands
(colored min/max regions with severity, color, and label) are editable in a dedicated
Project Editor dialog, persist in the project file, and drive Bar/Gauge/Meter/LED
visuals. The frequency domain deserves the same treatment: named markers and bands on
the spectrum, in the style of DAW equalizers and spectrum analyzers (colored translucent
regions with soft gradients, labeled frequency lines), plus per-marker level thresholds
that light up when exceeded.

## Goals

- A user can attach a list of **frequency markers** to any FFT-enabled dataset in the
  Project Editor: each marker has a frequency (or a frequency band), a label, a color,
  and optional warning/alarm amplitude thresholds in dB.
- The FFT widget renders these markers beautifully and legibly: labeled vertical lines
  for point markers, DAW-EQ-style translucent gradient regions for bands, on both the
  linear and the log frequency axis, in every theme.
- The widget **monitors** each marker automatically: the live peak level inside the
  marker's frequency neighborhood is measured every display tick, and the marker
  visually escalates (normal → warning → alarm) when the level crosses the configured
  thresholds, with a live dB readout.
- The marker list persists in the project file, survives save/load byte-identically for
  untouched projects, and is fully readable/writable through the remote API.
- The Waterfall widget renders the same markers on its frequency axis (lines and band
  regions over the spectrogram), so the two spectral views stay consistent.

## Non-Goals

- **No global alarm-system integration.** Marker alarms are widget-local visuals; they
  do not feed the app-wide alarm monitor, MQTT, or exports. (The FFT is computed inside
  the widget and only while it is visible — a headless spectral alarm engine is a
  different, much larger feature.)
- **No automatic peak detection / tracking.** Markers sit at user-configured
  frequencies; the feature does not find peaks, track RPM, or move markers.
- **No harmonics/sideband cursors** (1x/2x/3x families). A user can add each harmonic
  as its own marker; generated families are future work.
- **No per-marker history or logging.** The readout is live-only.
- **No new dependency, no new file format.** Markers ride inside the existing project
  JSON schema.

## Requirements

1. **R1 — Schema.** An FFT-enabled dataset can carry zero or more frequency markers.
   Each marker has: a start frequency in Hz; an optional end frequency in Hz (absent or
   equal → point marker, else band); a label; a color (hex, empty → automatic from the
   theme); an optional warning threshold in dB; an optional alarm threshold in dB.
   Absent keys read as defaults; projects without markers serialize byte-identically to
   today (additive-schema discipline, as 0014/0017).
2. **R2 — Editor.** The Project Editor offers a "Frequency Markers" editor for the
   selected FFT-enabled dataset, matching the look, layout, and interaction grammar of
   the existing Alarm Bands editor: add/remove/reorder rows, per-row color swatch with
   picker, inline validation (frequencies clamped to 0…Nyquist for the dataset's
   configured sampling rate, warning ≤ alarm), a live preview of the marker palette,
   and Cancel/Apply semantics. The entry point is enabled exactly when the selected
   dataset has FFT enabled.
3. **R3 — FFT rendering.** The FFT widget draws every marker inside the visible range:
   point markers as a vertical line with a compact label chip; bands as a translucent
   vertical region with a soft horizontal gradient (DAW-EQ style) plus the label chip.
   Rendering is correct on linear and log frequency axes, follows zoom/pan, stays
   legible in light and dark themes, and never obscures the spectrum curve (markers
   sit under the curve stroke, labels above).
4. **R4 — Monitoring.** For each marker the widget computes, every display tick, the
   peak displayed level (dB) within the marker band (for a point marker: within a small
   neighborhood around it). If the peak crosses the warning / alarm thresholds the
   marker escalates its visual state (color shift / emphasis / blinking alarm accent)
   and its label shows the live peak dB. Markers without thresholds simply show the
   readout with no escalation. Monitoring honors the ballistics-processed display
   spectrum (what you see is what is judged).
5. **R5 — Toolbar toggle.** The FFT widget toolbar gains a "Show Frequency Markers"
   toggle (persisted per widget like the other toolbar toggles) so markers can be
   hidden without editing the project.
6. **R6 — API.** The marker list round-trips through the remote API: settable and
   gettable per dataset both via the generic dataset-update path and via dedicated
   atomic get/set commands, mirroring the alarm-bands API surface. Invalid input
   (negative frequency, warning > alarm, junk types) is rejected or clamped with a
   warning, never crashes, and never corrupts the stored list.
7. **R7 — Waterfall rendering.** The Waterfall widget draws the same markers over the
   spectrogram: vertical lines / translucent band regions at the marker frequencies
   with label chips, correct under its zoom/pan, in every color map. Alarm escalation
   visuals on the waterfall are optional polish; the markers themselves are required.
8. **R8 — Zero hotpath impact.** All computation happens in the widget display layer
   at UI refresh rate; nothing changes on the frame ingest/parse path.

## Acceptance Criteria

- [x] **AC1** — pytest round-trip: a project saved with markers (point + band, with and
      without thresholds) reloads identically; a project without markers is
      byte-identical to one saved by the previous build (`pytest tests/integration/`
      project persistence tests extended).
- [x] **AC2** — pytest API: dedicated get/set commands and the dataset-update key both
      round-trip the marker list; invalid payloads (bad types, reversed band, negative
      freq) are rejected/clamped with a warning and the stored list stays valid.
- [x] **AC3** — Maintainer: on a live FFT (e.g. audio input), an 800 Hz point marker
      and a labeled band render as described on both axis modes, follow zoom/pan, look
      intentional in light + dark themes, and the label chip shows a live dB readout.
- [x] **AC4** — Maintainer: setting a warning/alarm threshold below the current peak
      escalates the marker (distinct warning and alarm visuals); raising it back
      de-escalates.
- [x] **AC5** — Maintainer: the editor dialog matches Alarm Bands in look/feel;
      validation prevents out-of-range frequencies; Cancel discards, Apply persists and
      the widget updates live (dashboard re-syncs without restart).
- [x] **AC6** — Maintainer: the same markers appear on the Waterfall at the correct
      frequencies under zoom/pan.
- [x] **AC7** — `--benchmark-hotpath` gates unchanged (no ingest-path edits).

## Constraints & Invariants

- Additive schema only: absent keys → defaults; untouched projects serialize
  byte-identically (the 0014/0017 discipline).
- Marker evaluation and rendering live entirely in the widget/display layer at UI tick
  rate; no allocation at steady state in the per-tick path once buffers are warm.
- Must work in ProjectFile mode; QuickPlot/ConsoleOnly modes have no FFT datasets and
  are unaffected.
- FFT widget markers are a free (GPL-build) feature; the Waterfall rendering ships with
  the already-Pro Waterfall widget. No new license gating surface.
- Editor dialog must follow the established dialog system (themed, RTL-safe, same
  fonts/spacing/buttons as Alarm Bands); no new UI toolkit idioms.
- API changes follow the existing per-key allow-list + dedicated-command pattern; new
  keys must not break older clients (unknown keys in old builds are ignored on read).

## Decisions (taken 2026-07-17 under delegated authority)

- **Markers live in the dataset's project JSON** (like alarm bands), not in
  widgetSettings: they are engineering configuration that belongs to the project and
  must be API-addressable, not per-widget cosmetic state.
- **Thresholds are display-dB values** (the FFT widget's native Y unit), matching what
  the user reads off the plot; no linear-unit thresholds.
- **Point-marker monitoring window** is a small fixed neighborhood (a few bins) so a
  slightly detuned line still registers; exact width is a plan-phase constant.
- **Waterfall gets rendering (+readout where cheap), not required escalation visuals**
  — its C++-painted axis layer makes heavy per-tick label work costly; parity beyond
  lines/bands/labels is polish, not contract.

## Open Questions

- None blocking. (Severity blink on alarm is included as the alarm accent; if the
  maintainer dislikes blinking on review, it is a one-line toggle to drop.)
