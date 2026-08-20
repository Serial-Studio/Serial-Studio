---
spec: 0009-dataset-color-override
title: Per-dataset color override in the Project Editor
status: done          # closed 2026-08-20
created: 2026-07-14
author: Alex Spataru
---

# Spec 0009 — Per-dataset color override in the Project Editor

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Every dataset's display color is assigned automatically today: the active theme provides a
fixed palette and each dataset picks an entry by its position in the frame, cycling when the
frame has more datasets than the palette has colors. The user has no say. That breaks down
whenever color carries meaning in the user's domain: accelerometer X/Y/Z conventionally
red/green/blue, battery voltage orange, a "fault" channel red — none of which the automatic
assignment can honor. It also makes colors unstable across project edits: inserting or
reordering a dataset shifts the palette position of every dataset after it, silently
recoloring plots the user had learned to read at a glance. Large frames make it worse — once
the palette cycles, two unrelated channels share a color and a multi-channel plot becomes
ambiguous.

The fix is the obvious one every plotting tool offers: let the user pin a color on a specific
dataset in the Project Editor, while everything they don't touch keeps the automatic,
theme-driven assignment that works well for casual use.

## Goals

- A user editing a dataset in the Project Editor can set its display color explicitly, and
  can revert it to automatic at any time.
- Datasets the user never touches look and behave exactly as they do today, including on
  existing project files.
- An explicit color follows the dataset everywhere its color is shown on the dashboard
  (plots, multi-plots, gauges, bars, LED panels, GPS trails, 3D plots, widget accents), so
  one setting recolors the channel consistently.
- The choice is saved with the project file and travels with it to other machines.

## Non-Goals

- **Not** a theme editor: the automatic palette itself, and theme colors in general, stay
  untouched and uneditable.
- **Not** per-widget styling: one dataset has one color; no "different color for this
  dataset in this particular widget".
- **No** group-level or frame-level color settings — datasets only.
- **No** change to alarm-band colors, which already have their own override mechanism, nor
  to alarm/severity coloring semantics.
- **No** exposure in Quick Plot or Console modes — those have no Project Editor; their
  auto-generated datasets keep automatic colors.
- **No** change to exported data (CSV, MDF4, database contents): color is presentation only.

## Requirements

1. **R1** — Every dataset in the Project Editor shows a color setting whose default state is
   "Automatic". In the automatic state the dataset's rendered color is identical to today's
   behavior (theme palette, position-based, cycling).
2. **R2** — The user can replace "Automatic" with an explicit color chosen from a color
   picker. After the change, every dashboard visual that displays that dataset uses the
   chosen color.
3. **R3** — The user can revert an explicit color back to "Automatic" with a single obvious
   action, restoring today's behavior for that dataset.
4. **R4** — An explicit color is theme-independent: switching the application theme leaves
   it unchanged, while datasets in the automatic state continue to re-color with the theme
   as they do today.
5. **R5** — The setting persists in the project file. Saving and reloading the project
   preserves it; a project file that predates the feature (or omits the setting) loads with
   every dataset in the automatic state.
6. **R6** — Changing the color takes effect on the live dashboard the same way other dataset
   edits do — no application restart or device reconnection required.
7. **R7** — The setting is editable through the project API like other dataset properties,
   so scripted/AI-driven project editing can set and clear it.

## Acceptance Criteria

- [ ] **AC1** — Maintainer check in the running app: a fresh project shows "Automatic" for
      every dataset, and plots render with today's palette colors (R1).
- [ ] **AC2** — Maintainer check: pick red for one dataset in a multi-plot group; that curve,
      its legend entry, and every other widget bound to the dataset turn red; the group's
      other curves keep their automatic colors (R2).
- [ ] **AC3** — Maintainer check: revert the same dataset to Automatic; it returns to the
      palette color it had before (R3).
- [ ] **AC4** — Maintainer check: with one explicit and one automatic dataset visible, switch
      themes; the explicit color is unchanged, the automatic one follows the new theme (R4).
- [ ] **AC5** — `pytest` integration: set a dataset color through the project API, save,
      reload, read it back; and loading a pre-feature `.ssproj` (e.g. an existing `examples/`
      project) yields automatic state for all datasets (R5, R7).
- [ ] **AC6** — Maintainer check: change a color while connected and streaming; the dashboard
      recolors without reconnecting (R6).
- [ ] **AC7** — `--benchmark-hotpath` still clears all nine gates after the change (the
      dataset model is copied on the hotpath; see Constraints).

## Constraints & Invariants

- **Hotpath**: dataset objects are copied on the 256 kHz+ frame path; whatever state this
  feature adds must not regress any `--benchmark-hotpath` gate (AC7 is the check).
- **Zero default-behavior change**: with no override set, rendered output must be
  pixel-identical in color to today, for every widget type and theme.
- **Additive file format**: the project file gains an optional key only; no existing key
  changes meaning, and absence of the key means "automatic".
- **Mode boundary**: project state must continue to survive Quick Plot / Console mode
  crossings unchanged; the feature must not leak editor-only state into those modes.
- **No new dependencies**; color picking uses what Qt already ships.
- **Licensing tier**: free (GPL) feature — color choice is baseline UX, not a Pro
  differentiator. *(Maintainer confirmed 2026-07-14.)*

## Open Questions

- None. Tier gating (free/GPL) and theme independence of explicit colors (fixed color, not a
  palette slot — see R4) were both resolved with the maintainer on 2026-07-14.
