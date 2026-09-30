---
spec: 0089-hdr-rendering
title: HDR Rendering Support
status: done         # draft -> approved -> in-progress -> done | shelved
created: 2026-09-29
author: Alex Spataru
---

# Spec 0089 — HDR Rendering Support

> **Phase 1 of 4 — the WHAT and the WHY.** No implementation detail; no file paths, no
> class names, no signal wiring (that is `plan.md`). Gate: do not start `/ss-plan` until
> a human marks this `approved`.

## Problem / Motivation

Serial Studio renders everything at standard dynamic range, even on displays with real HDR
headroom — every XDR MacBook panel and every Windows HDR monitor. Two costs follow. First,
alarm salience: a Critical alarm flash can be no brighter than ordinary white, so on a bright
dashboard the most important visual signal in the app competes with plot backgrounds and
chrome instead of standing above them. Second, fidelity: 8-bit output visibly bands across
the gradient-heavy instrument faces (gauges, meters, compasses, clocks) and quantizes the
waterfall colormap into stepped bands, which users read as rendering artifacts on otherwise
smooth data.

A session-long render-path survey (2026-09-29, this spec's sibling analysis) established
where HDR is real: alarm flashes, LED indicators, FFT markers, plot curves and the waterfall
colormap can all genuinely exceed SDR white or gain precision, while console, GPS, Canvas
widget and image views are CPU-rasterized at 8 bits and gain nothing. This spec scopes HDR
to where it pays.

## Goals

- On an HDR-capable display, Critical alarm flashes (value boxes, digital readouts, master
  annunciator) render brighter than SDR white, visibly separating them from all non-alarm UI.
- Lit LED Panel indicators and FFT plot markers render emissively (brighter than SDR white,
  with their glow intact).
- Plot, MultiPlot and FFT curve strokes can render above SDR white, controlled by the user's
  intensity setting.
- The waterfall colormap renders without visible banding, and its top-of-scale peaks exceed
  SDR white on HDR displays.
- GPU-rendered gradients (instrument faces and chrome across the dashboard) show visibly
  reduced banding when HDR output is active.
- The user controls all of this with one HDR on/off preference plus one intensity slider,
  and a user without an HDR display or on an unsupported platform never sees a difference
  or a broken state.
- Works on macOS (per-window EDR, no OS setting needed) and on Windows with the OS HDR
  toggle enabled — both are v1 targets.

## Non-Goals

- No HDR for CPU-rasterized widgets: console/terminal, GPS map, Canvas widget. They stay SDR
  permanently; the rework is disproportionate to the benefit.
- No HDR image/video passthrough: ImageView keeps showing frames as-is; no HDR decode.
- No HDR for the 3D plot in v1 (its background is CPU-rasterized; its curves are a niche win).
- No HDR contract for installable extension widgets — they keep rendering correctly at SDR;
  authoring guidance may follow later, enforcement never.
- No Linux/Vulkan HDR in v1, and no support under the OpenGL or Software rendering backends.
- No theme-wide HDR color authoring: themes remain SDR-referenced; only designated emissive
  elements exceed white.
- No HDR in exports or recordings (video, images, MDF4 are untouched).
- No de-banding work for the gauge/meter/compass chrome rings' shadowed layers and no
  Gyroscope glow upgrade in v1 (candidate follow-ups, explicitly out).
- No license gating: HDR ships in every build, free.

## Requirements

1. **R1 — Preference.** A persisted "HDR output" preference (default off) exists alongside
   the rendering-backend selection. It is offered only when the active backend/platform can
   support HDR (macOS/Metal, Windows/Direct3D); elsewhere it is hidden or disabled with an
   explanation. Like a backend change, it takes effect after an app restart, and the user is
   prompted to restart.
2. **R2 — Crash safety.** If the app fails to reach a working UI after enabling HDR, the next
   launch automatically reverts to SDR, same as a failed backend change.
3. **R3 — SDR parity.** With the preference off, rendering is visually identical to today on
   every platform and backend.
4. **R4 — Graceful degradation.** With the preference on but no HDR-capable display/OS state
   (SDR monitor, Windows HDR toggle off), the app renders correctly at SDR with no visual
   defects and no user-facing errors. Moving the window between an HDR and an SDR monitor
   keeps rendering correct on each.
5. **R5 — Windows SDR white.** On Windows with OS HDR enabled, all ordinary (non-emissive)
   UI content renders at the user's configured OS SDR brightness — neither dimmed to a fixed
   reference level nor blown out.
6. **R6 — Intensity (amended 2026-09-29).** Originally a user-facing slider; replaced by
   maintainer decision after first hands-on use ("just enable/disable HDR and select best
   intensity"): the app picks the intensity automatically — 2× SDR white, clamped per window
   to the display's reported headroom — and it scales only the designated emissive elements,
   never text, backgrounds, or ordinary chrome. No brightness UI beyond the on/off toggle.
7. **R7 — Emissive alarms.** With HDR active and intensity above 1×, Critical alarm flashes
   in Bar, Gauge and Meter widgets and the master annunciator's critical flash render
   brighter than SDR white. Warning-level (steady) alarm colors are unchanged. On SDR output
   all alarm visuals are identical to today.
8. **R8 — Emissive LED Panel.** Lit LEDs (including blinking) render brighter than SDR white
   with their glow preserved; unlit LEDs are unchanged.
9. **R9 — Emissive FFT markers.** FFT band/point markers, including their alarm blink state,
   render brighter than SDR white.
10. **R10 — Emissive curves.** Plot, MultiPlot and FFT curve strokes render brighter than
    SDR white proportionally to the intensity slider; at 1× they match today's rendering.
11. **R11 — Waterfall.** The waterfall colormap shows no visible banding on HDR output, its
    top-of-scale colors exceed SDR white, and the change does not increase the per-frame
    data volume sent to the GPU.
12. **R12 — No silent clamping.** Every element designated emissive actually reaches the
    display above SDR white — no intermediate rendering step may silently reduce it back to
    SDR (a "looks slightly dim" failure is a defect, not a degradation).

## Acceptance Criteria

- [ ] **AC1 (R1, R2)** — On an XDR MacBook: enable HDR, restart, confirm the preference
      survives; simulate a startup crash with the change pending and confirm the next launch
      reverts to SDR. Maintainer observation.
- [ ] **AC2 (R3)** — Side-by-side screenshots (setting off, before/after builds) of the
      default dashboard show no visual difference on macOS and Windows. Maintainer
      observation.
- [ ] **AC3 (R4)** — With HDR on, drag the window between an HDR and an SDR monitor:
      both render correctly, emissive elements clamp gracefully on the SDR one. Maintainer
      observation.
- [ ] **AC4 (R5)** — On Windows with OS HDR on, change the OS SDR-brightness slider and
      confirm the app's ordinary UI tracks it. Maintainer observation.
- [ ] **AC5 (R6, amended)** — With HDR active, a Critical flash renders at the automatic
      intensity (min(2× SDR white, headroom)); on an SDR display it matches SDR appearance.
      Maintainer observation.
- [ ] **AC6 (R7–R10)** — Demo project with a Critical alarm, an LED panel, FFT markers and
      a plot: on an XDR display each designated element visibly exceeds a reference white
      patch shown beside it. Maintainer observation.
- [ ] **AC7 (R11)** — Waterfall with a slow gradient signal shows no banding steps at HDR;
      GPU upload per frame measured (or reasoned in review) to be at or below today's.
      Maintainer observation plus plan-level measurement.
- [ ] **AC8 (R12)** — A checklist in the plan enumerates every emissive element's route to
      the display; each is verified on-screen against the reference-white patch. Review
      artifact plus observation.
- [ ] **AC9 (hotpath)** — `--benchmark-hotpath` gates unchanged (the feature must not touch
      the acquisition pipeline at all). CI gate.
- [ ] **AC10 (regressions)** — Existing `pytest` integration suite and `ctest` tier pass
      unchanged with HDR off and on. Automated.

## Constraints & Invariants

- **Strictly additive.** HDR off means today's renderer, bit-for-bit in behavior; HDR on
  with an SDR display means today's visuals. All risk is confined behind the preference.
- **The acquisition pipeline is untouched.** This is a presentation-only feature: no change
  to frame parsing, publishing, or the 256 kHz benchmark gates.
- **No new per-frame heap allocation** on any widget render path, and no increase in
  per-frame GPU upload volume (the waterfall requirement R11 makes this explicit).
- **Emissive is for small indicator/highlight areas only** — alarm boxes, LEDs, markers,
  strokes, colormap peaks. Text, backgrounds and large surfaces never exceed SDR white;
  readability and eye comfort outrank spectacle.
- **Existing accessibility and fallback behavior keeps working**: reduce-motion, the
  effects-off state of the Software backend, and the backend selector's crash-revert flow.
- **Alarm semantics are unchanged** — HDR alters only how alarm states look, never when
  they trigger, blink, or clear (the annunciator's ISA-18.1 behavior is out of bounds).
- **No new third-party dependency.**
- **Multi-monitor mixed HDR/SDR must be correct**, not just tolerated (R4).
- **Free in every build** — no license, trial or tier interaction anywhere in the feature.

## Open Questions

- **Windows SDR-white ownership.** Does Qt 6.11's 2D scene graph scale SDR content by the
  OS SDR white level automatically, or must the app apply it? Windows is committed either
  way (maintainer decision, 2026-09-29); the answer decides plan effort and risk, and must
  be resolved from Qt documentation/source plus a Windows probe before `/ss-plan` is
  finalized.
- **Third-party plot chrome under HDR.** The plot grid/axes are drawn by a Qt module we do
  not control; whether they render acceptably in an HDR window needs a runtime check on
  real hardware. If they degrade, does the plot fall back to SDR or do we accept the look?
- **Curve emissive default.** R10 ties curve brightness to the global intensity slider. Is
  a per-project or per-group opt-out wanted, or is the global slider enough for v1?
  (Recommendation: global only.)
- **HDR status visibility.** Should the UI indicate somewhere that HDR is actually active
  on the current display (vs. enabled-but-unavailable)? (Recommendation: yes, a passive
  note in Preferences only.)
