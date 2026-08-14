---
spec: 0052-bar-panel-alarm-visuals
title: Bar Panel group widget + severity-first alarm visual language
status: in-progress  # draft -> approved -> in-progress -> done | shelved  (code-complete; ACs pending a rebuild)
created: 2026-08-12
author: Alex Spataru
---

# Spec 0052 — Bar Panel group widget + severity-first alarm visual language

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

The BADAQ project (TAM-Firmware) needed a painter script (~900 lines of user JavaScript,
"APS500 Synoptic") to get a readable multi-channel instrument panel: rows of labeled bars
where the full alarm-band structure is visible as muted zones on the track, the fill takes
the color of the band the value currently sits in, and gauge needles recolor the same way.
The result is instantly legible — anyone can see at a glance which channel is OK, cautioned,
or alarmed, without reading a single number — precisely because color encodes exactly one
thing: severity.

Serial Studio's built-in widgets cannot produce this today:

- There is no group-level bar widget. Monitoring 8 channels means 8 separate Bar widgets,
  each with its own title strip, value box, and tick scale — heavy chrome, no shared visual
  rhythm, and no compact "rake" view.
- The built-in Bar draws alarm bands as thin strips along the reading edge and always fills
  with the dataset color; the Gauge and Meter behave similarly. Severity shows up only in a
  flashing value box. The band structure is a decoration, not the message.
- Default dataset colors cycle through the theme palette by index, so a 10-channel dashboard
  shows 10 unrelated hues. On instrument widgets that rainbow carries zero information and
  actively competes with the one color axis that matters (severity). Multi-series plots are
  the only place per-series hues genuinely disambiguate anything.

## Goals

- A user can drop one group-level widget ("Bar Panel") on a group and get a labeled bar per
  dataset — label, band-zoned track, severity-colored fill, live value — comparable to the
  BADAQ painter panels, with zero scripting.
- The Bar Panel renders both as horizontal rows (synoptic panel style) and vertical columns
  (EGT-rake style), selectable in the project editor with an automatic default.
- On Bar, Gauge, Meter, and the Bar Panel, defined alarm bands are always visible in full
  (muted track zones / gauge ring arcs), and the moving indicator (fill, needle) takes the
  active band's severity color — including the OK color — so "confirmed normal" is a
  positive signal, not the absence of one.
- Non-plot widgets default to a single accent color instead of the per-index palette cycle;
  explicit per-dataset color overrides keep working everywhere. Plot-based widgets
  (Plot, MultiPlot, FFT, Plot3D, Waterfall) keep the multicolor cycle.
- A value outside every defined band renders with the nearest band's severity, never as
  an unclassified/neutral state (overrange must read as critical, not calm).
- A dataset can opt into extreme-hold markers: the widget shows where the highest and
  lowest values since the last data reset sit on the scale (the painter's vibration
  peak-hold tick, without needing firmware/script support).

## Non-Goals

- No new alarm-band data model. Datasets already carry `alarmBands` (severity, color,
  label, blink); this spec only changes how widgets render them.
- No changes to alarm *notifications* (central monitor, cooldowns, LED blink semantics).
- No changes to plot-based widget rendering or their color assignment.
- No migration or rewriting of stored per-dataset color overrides in existing projects.
- Not a replacement for the painter widget: no free-form layout, dials-in-panel composites,
  or custom headers. Users who need the full synoptic keep the painter.
- No externally-fed hold markers (the painter's table-fed pk-hold stays a painter
  capability); the widget option tracks the value stream itself. No decaying/windowed
  peak modes.
- No LED Panel rendering changes (its band consumption already encodes severity).

## Requirements

1. **R1 — Bar Panel widget.** A new group-level dashboard widget is selectable in the
   project editor for any group; it renders one bar per (visible, numeric-ranged) dataset
   in the group, each with the dataset title, a track showing the dataset's full range,
   and the live value formatted with the dataset's units/decimals settings.
2. **R2 — Orientation.** The Bar Panel has a project-editor style option with values
   Auto / Horizontal / Vertical. Auto (default) picks the orientation from the widget's
   aspect ratio and channel count; the explicit choices force it. The setting persists in
   the project file.
3. **R3 — Bands on the track.** On Bar Panel, Bar, Gauge, and Meter, every defined alarm
   band is drawn across its full extent of the scale as a muted zone (track segment or
   ring arc) whose hue derives from the band's severity color (or the band's custom color
   when set), visible regardless of the current value.
4. **R4 — Severity-colored indicator.** On those same widgets, when bands are defined the
   value indicator (bar fill / needle) is tinted with the active band's severity color at
   all times — OK included. Numeric value text stays neutral while OK and takes the
   severity color at warning or worse (matching the painter's text discipline).
5. **R5 — Overrange clamps to nearest band.** When bands are defined and the value falls
   outside all of them, the indicator and text take the severity of the nearest band.
6. **R6 — Single default color.** With no explicit color override, all non-plot widgets
   render with one shared accent color (theme-derived) instead of the per-index palette
   cycle. Explicit overrides win unchanged. Plot-based widgets keep the palette cycle.
7. **R7 — No bands, no noise.** A dataset with no alarm bands renders exactly as a plain
   single-color widget: default accent (or override) fill, no zones, no severity logic.
8. **R8 — Theme fidelity.** All new rendering derives colors from the active theme
   (severity colors via the existing theme alarm-color lookup); both light and dark themes
   stay legible without per-theme special cases in projects.
9. **R9 — Free tier.** The Bar Panel and all rendering changes are available in GPL builds.
10. **R10 — Extreme-hold markers.** A dataset-level option (default off) makes Bar,
    Gauge, Meter, and Bar Panel show two hold markers: one at the maximum and one at the
    minimum value observed since the last data reset (connection start, dashboard data
    reset, or replay restart). Markers hold indefinitely — no decay — and clear on data
    reset. The option persists in the project file.

## Acceptance Criteria

> **Status 2026-08-12:** implementation is code-complete and every static gate passes, but
> NO acceptance criterion has been executed — all nine need a rebuilt binary (the ctest and
> pytest suites are written and collect cleanly; the visual and benchmark checks are
> maintainer-run). The boxes stay unchecked until those runs happen.

- [ ] **AC1** — In the project editor, a group can be assigned the Bar Panel widget; the
      dashboard shows one labeled bar per dataset with live values (maintainer check with
      a multi-dataset test project; API `dashboard.getData` shows the group widget).
- [ ] **AC2** — Switching the style option Auto/Horizontal/Vertical re-renders accordingly
      and survives save/reload of the project (maintainer check + project JSON round-trip
      in a pytest integration test).
- [ ] **AC3** — With a dataset defining OK/warning/critical bands, Bar, Gauge, Meter, and
      Bar Panel show all bands as muted zones at all times, and the fill/needle color
      transitions ok→warning→critical as the value crosses band edges (maintainer check
      driving values via API/simulator).
- [ ] **AC4** — Feeding a value beyond the outermost band renders the indicator in that
      nearest band's severity color, not a neutral color (maintainer check).
- [ ] **AC5** — A fresh project with several datasets and no color overrides renders
      non-plot widgets in one accent color while a MultiPlot of the same datasets still
      cycles hues; a dataset with an explicit color override shows that color
      (maintainer check).
- [ ] **AC6** — Existing projects load with unchanged dataset data; no project-file
      schema errors; stored color overrides render as before (pytest project round-trip
      suite stays green).
- [ ] **AC7** — `--benchmark-hotpath` gates stay green: rendering changes are GUI-side
      and add nothing to the frame pipeline.
- [ ] **AC8** — Both orientations stay legible at small widget sizes (labels elide or
      drop before overlapping; maintainer visual check in light + dark themes).
- [ ] **AC9** — With the extreme-hold option enabled, driving a value up then down leaves
      the max marker at the excursion peak and the min marker at the lowest point; a data
      reset clears both; with the option off no markers render (maintainer check driving
      values via API/simulator; project JSON round-trip covers persistence).

## Constraints & Invariants

- **Color encodes one axis.** On instrument widgets, hue variation must mean severity and
  nothing else; any decorative multi-hue rendering is a defect against this spec.
- **Display-only change.** No new per-frame work, signals, or allocations on the frame
  pipeline; all band geometry precomputable at (re)configure time, per-tick work bounded
  by visible channel count.
- **Dataset alarm-band model and existing project-key semantics are frozen** — new
  persisted state limited to the Bar Panel's widget/style settings and the one new
  dataset-level extreme-hold option (additive, absent = off, old projects load
  unchanged).
- **Alarm notification behavior unchanged** (central monitor semantics, no per-widget
  notification posts).
- Must work in ProjectFile mode; QuickPlot/DeviceDefined modes must not regress (they
  simply have no bands/groups configured the same way).
- Works with the existing theme system in light and dark; no hardcoded hex in QML beyond
  derived mixes.
- No new dependencies.

## Open Questions

- None — orientation UX (editor option, Auto default), OK-state coloring (severity color
  always), tier (free), name (Bar Panel), and extreme-hold semantics (max + min markers,
  hold until data reset, no decay) were resolved with the maintainer on 2026-08-12.
