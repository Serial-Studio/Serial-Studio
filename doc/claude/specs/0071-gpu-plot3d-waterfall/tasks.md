---
spec: 0071-gpu-plot3d-waterfall
phase: tasks
status: approved
updated: 2026-08-26
---

# Tasks 0071 — GPU Rendering for the 3D Plot and Waterfall

> **Phase 3 of 4 — the ordered checklist.** Decompose [`plan.md`](./plan.md) into units that
> are small, ordered, and *individually verifiable* — each one a coherent diff a reviewer
> could read in isolation. `/ss-implement` works this list top to bottom and keeps the status
> boxes current. Gate: do not start `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change. If a task touches >3 files or needs a paragraph
  to describe, split it.
- **Verify** is how *this* unit is confirmed before moving on — usually
  `python scripts/code-verify.py --check <files>`, plus a test or a read-back where one fits.
- **Deps** lists task IDs that must land first.
- Order so the tree compiles (conceptually) after each task where practical.
- The maintainer builds and launches; tasks marked **(maintainer)** are theirs to run.

## Tasks

### T1 — Capture the "before" performance baseline (maintainer)

- **Files:** none (produces `doc/claude/specs/0071-gpu-plot3d-waterfall/baseline.md`)
- **Does:** With the app running on a live source and a dashboard carrying both a 3D plot and
  a waterfall, take a `sample <pid>` capture. Record which widget dominates GUI-thread time
  and note the observed frame feel. This is AC1's baseline and it decides whether the
  waterfall or the 3D plot ports first; the order below assumes the waterfall dominates and
  is revisited if the capture disagrees.
- **Verify:** `baseline.md` exists with the capture summary and the dominant-widget verdict.
- **Deps:** none
- [x] done

### T2 — Extract the shared stroke builder from `PlotCurve`

- **Files:** `app/src/UI/Widgets/GpuStroke.h`, `app/src/UI/Widgets/GpuStroke.cpp`,
  `app/CMakeLists.txt`
- **Does:** Move `PlotCurve`'s cap-section, join-index, fan, run-length and count/emit helpers
  into a standalone builder with no Qt Quick item coupling. **Pure move — no logic edits.**
  The binding invariant travels with it: the counting pass and the streaming pass must stay
  textually parallel, or the geometry buffer overruns; the `kMaxGeometry` clamp and the debug
  post-condition asserts come along unchanged. Register in the base `SOURCES`/`HEADERS` lists,
  not the commercial block.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/GpuStroke.h
  app/src/UI/Widgets/GpuStroke.cpp app/CMakeLists.txt`
- **Deps:** none
- [x] done

### T3 — Re-point `PlotCurve` at the shared builder

- **Files:** `app/src/UI/Widgets/PlotCurve.h`, `app/src/UI/Widgets/PlotCurve.cpp`
- **Does:** Drop the moved helper declarations and call into `GpuStroke`. Plot, FFT and
  MultiPlot curves are shipped, widely-used paths: emitted geometry must be identical, so this
  lands as its own bisectable diff with no behaviour change.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/PlotCurve.h
  app/src/UI/Widgets/PlotCurve.cpp`; **(maintainer)** before/after screenshots of a live plot
  and an FFT at matched data and theme show no visual difference.
- **Deps:** T2
- [x] done

### T3b — Generalize the stroke builder's colour input

- **Files:** `app/src/UI/Widgets/GpuStroke.h`, `app/src/UI/Widgets/GpuStroke.cpp`
- **Does:** `PlotCurve` strokes with one uniform colour; the 3D grid needs per-vertex alpha for
  its distance fade (T9) and the 3D trace needs a per-vertex gradient (T10). Widen the builder
  to accept a per-vertex colour source, with uniform colour as the degenerate case. Split out
  of T2 deliberately: T2 stays a pure move so a `PlotCurve` regression is bisectable, and this
  diff is verified the same way — uniform-colour output must remain byte-identical.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/GpuStroke.h
  app/src/UI/Widgets/GpuStroke.cpp`; **(maintainer)** plot and FFT curves unchanged.
- **Deps:** T3
- [x] done

### T4 — Waterfall: swap the base class and stand up an empty node tree

- **Files:** `app/src/UI/Widgets/Waterfall.h`, `app/src/UI/Widgets/Waterfall.cpp`
- **Does:** `QQuickPaintedItem` -> `QQuickItem`, `setFlag(ItemHasContents, true)`, replace
  `paint()` with `updatePaintNode` rendering only the outer and inner background quads. The
  binding invariant, named at the edit: `updatePaintNode` runs on the render thread with the
  GUI thread blocked in the sync phase — model reads happen there or in a GUI-thread slot,
  nowhere else. FFT, ring writes, marker state, zoom/pan and every property stay untouched.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Waterfall.h
  app/src/UI/Widgets/Waterfall.cpp`; **(maintainer)** widget renders its two background bands
  and no longer crashes or paints stale content.
- **Deps:** T1
- [x] done

### T5 — Waterfall: spectrogram as a ring-split textured node

- **Files:** `app/src/UI/Widgets/Waterfall.cpp`, `app/src/UI/Widgets/Waterfall.h`
- **Does:** Own a `QSGTexture` built from the history image, recreated only when a row was
  actually written. Draw it as two textured quads derived from the existing
  `computeSourceRect` wrap split, so zoom/pan become texture coordinates and the GPU does the
  scaling that `drawHistoryImage` currently does on the CPU. Delete `drawHistoryImage`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Waterfall.cpp
  app/src/UI/Widgets/Waterfall.h`; **(maintainer)** spectrogram scrolls in the same direction
  with no seam or tear at the ring wrap (R7), and zoom/pan track as before.
- **Deps:** T4
- [x] done

### T6 — Waterfall: overlay rasterizer TU

- **Files:** `app/src/UI/Widgets/Waterfall/WaterfallOverlay.cpp`, `app/CMakeLists.txt`
- **Does:** Move `renderAxisLayer`, `drawXAxis`, `drawYAxis` into a new TU under the existing
  `Waterfall/` split and widen it to also rasterize markers, marker chips, the hover cursor
  and the tooltip into the one cached overlay image, populating `m_chipHitRects` exactly as
  the current paint path does. Register inside the `BUILD_COMMERCIAL` block.
- **Verify:** `python scripts/code-verify.py --check
  app/src/UI/Widgets/Waterfall/WaterfallOverlay.cpp app/CMakeLists.txt`; TU census shows
  `Waterfall.cpp` under the 1500-line limit.
- **Deps:** T5
- [x] done

### T7 — Waterfall: overlay dirty-tracking and node

- **Files:** `app/src/UI/Widgets/Waterfall.cpp`, `app/src/UI/Widgets/Waterfall.h`
- **Does:** Extend the existing `m_axisDirty` tracking to cover marker values, cursor position
  and tooltip state, and draw the overlay image as one textured quad re-uploaded only when
  dirty. Delete the per-paint `drawMarkers` / `drawCursor` calls.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Waterfall.cpp
  app/src/UI/Widgets/Waterfall.h`; **(maintainer)** axes, marker chips, click-to-spotlight,
  hover readout and tooltip all behave as before (R8), Campbell mode and log-X included.
- **Deps:** T6
- [x] done

### T8 — Plot3D: swap the base class and add the background tile

- **Files:** `app/src/UI/Widgets/Plot3D.h`, `app/src/UI/Widgets/Plot3D.cpp`,
  `app/src/UI/Widgets/Plot3D/Plot3DOverlay.h`
- **Does:** `QQuickPaintedItem` -> `QQuickItem`, `setFlag(ItemHasContents, true)`, replace
  `paint()` with `updatePaintNode` drawing only the background: a small cached radial-gradient
  tile as one textured quad, rebuilt on theme change. Delete `drawBackground` and `m_bgImg`.
  Same render-thread invariant as T4, named at the edit. Camera state, interaction handlers
  and every property stay untouched.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Plot3D.h
  app/src/UI/Widgets/Plot3D.cpp app/src/UI/Widgets/Plot3D/Plot3DOverlay.h`; **(maintainer)**
  background gradient matches the previous look across both themes.
- **Deps:** T3b
- [x] done

### T8b — Dash support in the shared stroke builder

- **Files:** `app/src/UI/Widgets/GpuStroke.h`, `app/src/UI/Widgets/GpuStroke.cpp`
- **Does:** The 3D grid draws `Qt::DashLine`; the builder only strokes solid, and R4 requires
  the grid to read as it does today. Add an arc-length dasher that rewrites a pixel-space
  polyline into on/off spans separated by non-finite points, interpolating the per-point colour
  at each split. Non-finite separators are already how the builder breaks runs, so the stroking
  path itself is unchanged. Output buffers are caller-owned and reused, so a camera-driven
  rebuild does not allocate per line.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/GpuStroke.h
  app/src/UI/Widgets/GpuStroke.cpp`
- **Deps:** T3b
- [x] done

### T9 — Plot3D: grid and axes as vertex-coloured geometry

- **Files:** `app/src/UI/Widgets/Plot3D.cpp`, `app/src/UI/Widgets/Plot3D.h`
- **Does:** Build grid lines and the X/Y axes through `GpuStroke`, carrying the current
  distance fade as **per-vertex alpha** instead of a pen change per sub-segment. Preserve the
  near-plane clip and the off-screen-ratio cull that `drawLine3D` performs. Delete
  `renderGrid`, `drawLine3D` and `m_gridImg`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Plot3D.cpp
  app/src/UI/Widgets/Plot3D.h`; **(maintainer)** grid fade, dash appearance and axis colours
  read as before while orbiting.
- **Deps:** T8
- [x] done

### T9b — Point-quad builder in the shared stroke builder

- **Files:** `app/src/UI/Widgets/GpuStroke.h`, `app/src/UI/Widgets/GpuStroke.cpp`
- **Does:** With interpolation off the 3D trace draws each sample as a dot, not a stroke. Add a
  builder that emits one vertex-coloured quad per finite point, sized to match the current
  two-pixel dot, sharing the same node/material assembly as the stroke path so both go through
  one technique. Non-finite points are skipped, matching how the stroke path breaks runs.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/GpuStroke.h
  app/src/UI/Widgets/GpuStroke.cpp`
- **Deps:** T3b
- [x] done

### T10 — Plot3D: trace as gradient geometry

- **Files:** `app/src/UI/Widgets/Plot3D.cpp`, `app/src/UI/Widgets/Plot3D.h`
- **Does:** Stroke the projected trace through `GpuStroke` with the head-to-tail gradient as
  per-vertex colour, replacing the 64-entry pen array and the per-segment pen switch. Keep the
  interpolation-off dot mode. Preserve the camera-angle depth ordering by node order. Delete
  `renderData`, `m_plotImg` and `m_gradientPens`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Plot3D.cpp
  app/src/UI/Widgets/Plot3D.h`; **(maintainer)** trace colour ramp, line width and the
  grid/trace ordering flip past the camera-angle threshold all match (R4).
- **Deps:** T9, T9b
- [x] done

### T11 — Plot3D: camera indicator and label tiles

- **Files:** `app/src/UI/Widgets/Plot3D.cpp`, `app/src/UI/Widgets/Plot3D/Plot3DOverlay.cpp`,
  `app/CMakeLists.txt`
- **Does:** Rasterize the camera indicator whole into a fixed ~100 px tile, and the grid-interval
  label into a text-sized tile, both rebuilt only on a camera, grid-step or theme change and
  drawn as one textured quad each. Keeps the existing depth sort and the 240 px minimum-size
  gate. Register the new TU inside the `BUILD_COMMERCIAL` block. Delete `renderCameraIndicator`
  and `m_cameraIndicatorImg`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Plot3D.cpp
  app/src/UI/Widgets/Plot3D/Plot3DOverlay.cpp app/CMakeLists.txt`; **(maintainer)** indicator
  labels stay legible and correctly placed while orbiting, and vanish below 240 px as before.
- **Deps:** T10
- [x] done

### T12 — Plot3D: anaglyph via colour-masked double draw

- **Files:** `app/src/UI/Widgets/Plot3D.cpp`, `app/src/UI/Widgets/Plot3D.h`
- **Does:** Render the geometry twice with the existing eye transforms, masking each eye's
  per-vertex colour to its channels, replacing the two extra full-screen images and the
  full-frame scanline merge. Target is visual equivalence, not bit-parity, per the approved
  spec. Delete `m_anaglyphImg` and `m_anaglyphMerged`.
- **Verify:** `python scripts/code-verify.py --check app/src/UI/Widgets/Plot3D.cpp
  app/src/UI/Widgets/Plot3D.h`; **(maintainer)** red-cyan output reads correctly through
  glasses, eye separation and invert-eyes controls still behave (R6). If overlapping traces
  read wrong, fall back to the custom additive material named in the plan.
- **Deps:** T11
- [x] done

### T13 — Document the scene-graph rules

- **Files:** `doc/claude/architecture/dashboard.md`, `doc/claude/common-mistakes.md`
- **Does:** Extend the "GPU Curve Rendering" section to cover the shared stroke builder and
  both new GPU widgets. Add the render-thread node-update rules to `common-mistakes.md`, which
  currently has no scene-graph entry at all — the count/emit lockstep and the "model reads only
  inside `updatePaintNode`" rule are exactly the silent-breakage class it exists to catch.
- **Verify:** `python scripts/claim-verify.py`; `python scripts/documentation-verify.py` on the
  touched files.
- **Deps:** T12
- [x] done

### T14 — Capture the "after" baseline and close the acceptance criteria (maintainer)

- **Files:** `doc/claude/specs/0071-gpu-plot3d-waterfall/spec.md`
- **Does:** Repeat T1's capture on the finished build and compare (AC1). Run the interaction
  pass covering R5, R8, R9 and R10; the license activate/deactivate check (AC7); and the
  before/after screenshot comparison (AC3, R6). Check off each criterion in `spec.md`.
- **Verify:** every AC box in `spec.md` checked, with the two captures recorded side by side.
- **Deps:** T13
- [x] done

## Definition of Done

- [x] Every acceptance criterion in `spec.md` is met and checked off there.
- [x] `python scripts/code-verify.py --check` is clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] `--benchmark-hotpath` not regressed (AC5) — run as a regression check; the hotpath is
      not touched by this work.
- [x] TU and singleton census gates show no regression (AC6).
- [x] No `pytest` tier covers widget rendering; verification is the maintainer observations
      listed in `plan.md`. Nothing is claimed as automated that is not.
- [x] `python scripts/sanitize-commit.py` run; working tree clean of lint debt.
- [x] Diff is *what was asked, and only that* — the plan's file table is the lane; nothing
      outside it touched without raising it first.
- [x] `spec.md` status set to `done`.
