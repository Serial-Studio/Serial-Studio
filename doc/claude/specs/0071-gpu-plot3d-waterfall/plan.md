---
spec: 0071-gpu-plot3d-waterfall
phase: plan
status: approved
updated: 2026-08-26
---

# Plan 0071 — GPU Rendering for the 3D Plot and Waterfall

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Both widgets stop deriving from `QQuickPaintedItem` and become plain `QQuickItem`s that build
a scene-graph node tree in `updatePaintNode`, following the pattern already proven by
`Widgets::PlotCurve`. Work is split by how often it actually changes: everything that redraws
on every display tick becomes GPU geometry or a GPU texture, while text and decoration —
which change rarely — stay CPU-rasterized into a cached overlay image that is uploaded as a
texture only when marked dirty. For the 3D plot this replaces four full-screen images, a
per-segment pen switch, and a four-layer composite with two vertex-coloured triangle batches
and two small textures. For the waterfall it replaces the per-frame smooth-scaled rescale of
the whole history image with a texture the GPU samples, split into two quads to honour the
ring wrap. The 3D plot's line geometry reuses `PlotCurve`'s extrusion approach rather than
inventing a second one, so the project keeps a single GPU stroking technique.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/UI/Widgets/GpuStroke.h` | **New.** Shared vertex-colour stroke builder extracted from `PlotCurve`: count/emit lockstep, cap sections, join fans, `kMaxGeometry` guard. Header-only, no Qt Quick item coupling. |
| `app/src/UI/Widgets/GpuStroke.cpp` | **New.** Implementation of the stroke builder. |
| `app/src/UI/Widgets/PlotCurve.cpp` | Re-point its private emit/count helpers at `GpuStroke` so there is one implementation. No behaviour change; existing geometry output must stay byte-identical. |
| `app/src/UI/Widgets/PlotCurve.h` | Drop the helper declarations that moved into `GpuStroke`. |
| `app/src/UI/Widgets/Plot3D.h` | Base class `QQuickPaintedItem` -> `QQuickItem`; drop `paint()`, the four `QImage` layer members, `m_anaglyphImg`/`m_anaglyphMerged`, and `m_gradientPens`; add `updatePaintNode` plus overlay-dirty state. Public property/slot surface unchanged. |
| `app/src/UI/Widgets/Plot3D.cpp` | Replace `paint`, `renderData`, `renderGrid`, `renderCameraIndicator`, `drawBackground`, `drawLine3D` with node builders. Camera/interaction/projection logic kept. |
| `app/src/UI/Widgets/Plot3D/Plot3DOverlay.h` / `.cpp` | **New.** CPU rasterizer for the 3D plot's text-only overlay (grid-interval label, camera-indicator X/Y/Z glyphs) into a cached `QImage`, plus the background gradient tile. Placed in a per-widget subdirectory to match the existing split-TU convention (`Waterfall/WaterfallMath.h`, `Painter/PainterGradient.cpp`). |
| `app/src/UI/Widgets/Waterfall.h` | Base class `QQuickPaintedItem` -> `QQuickItem`; drop `paint()`, `drawHistoryImage`; add `updatePaintNode`, texture-dirty and overlay-dirty flags. Public surface unchanged. |
| `app/src/UI/Widgets/Waterfall.cpp` | Spectrogram becomes a textured node pair; `renderAxisLayer` is widened into the overlay rasterizer that also carries markers, chips, cursor, and tooltip. FFT, ring writes, marker state, zoom/pan untouched. |
| `app/src/UI/Widgets/Waterfall/WaterfallOverlay.cpp` | **New TU** carrying the overlay rasterizer, joining the existing `Waterfall/` split (`WaterfallMath.h`, `WaterfallTicks.cpp`). Keeps `Waterfall.cpp` under the 1500-line limit; splitter is `scripts/tu-cutter.py`. |
| `app/CMakeLists.txt` | Register the new source files: `GpuStroke.h/.cpp` in the base `SOURCES`/`HEADERS` lists (shared with `PlotCurve`, non-commercial), the two overlay TUs inside the `BUILD_COMMERCIAL` block alongside `Plot3D.cpp` and `Waterfall.cpp`. No new Qt module, no new dependency. |
| `doc/claude/architecture/dashboard.md` | Extend the existing "GPU Curve Rendering" section to cover the shared stroke builder and the two new GPU widgets. |
| `doc/claude/common-mistakes.md` | Add the render-thread node-update rules; the file currently has no entry for scene-graph work at all. |

Confirmed by grep: both widgets are constructed in C++ at `app/src/UI/DashboardWidget.cpp:540`
and `:557` into a `QQuickItem* m_dbWidget` (`DashboardWidget.h:138`), and registered at
`app/src/Misc/ModuleManager.cpp:598` and `:600`. Neither the QML files nor the C++ use any
`QQuickPaintedItem`-specific API, so the base-class swap needs no call-site changes.

## Architecture & data flow

Both items keep their current inputs unchanged: `Misc::TimerEvents` drives `updateData()`,
which reads `UI::Dashboard` and calls `update()`. The change is entirely below that line.

`updatePaintNode` runs on the render thread during the synchronization phase, while the GUI
thread is blocked. Reading GUI-thread-owned model data there is safe for exactly that reason,
and it is the convention already established by `PlotCurve`, which reads
`m_source->points()` inside its own `updatePaintNode`. Both widgets follow the same rule and
no other one: model reads happen in `updatePaintNode` or in a GUI-thread slot, never from any
other render-thread callback.

**3D plot** node tree, in draw order:

1. Background — one textured quad sampling a small cached radial-gradient tile. Rebuilt only
   when the theme changes; bilinear magnification makes tile resolution irrelevant.
2. Grid and trace — vertex-coloured triangles in one geometry node per depth group, so the
   existing camera-angle depth-ordering rule (grid before trace past the current threshold,
   after it otherwise) is expressed as node order. The distance fade that currently forces a
   pen change on each of 40 sub-segments per grid line becomes per-vertex alpha, evaluated
   once per vertex, drawn in one batch.
3. Camera indicator — the axis lines and dots join the geometry batch; only its X/Y/Z glyphs
   live in the overlay texture, positioned per frame as quads over a cached three-glyph tile.
4. Overlay — one textured quad for the grid-interval label, re-rasterized only when the label
   text or the theme changes.

Projection stays on the CPU for now, in the existing `screenProjection`, because the camera
logic (auto-centre easing, auto-scale, fit) reads projected extents. Moving the matrix into a
vertex shader is a later, separable optimization and is deliberately not in this plan.

**Waterfall** node tree, in draw order:

1. Outer and inner background — two flat-coloured quads, replacing the two `fillRect` calls.
2. Spectrogram — the history `QImage` becomes a `QSGTexture` owned by the item and recreated
   only when a row was actually written. The ring wrap is expressed the way
   `drawHistoryImage` already expresses it: as two source sub-rectangles, drawn here as two
   textured quads sharing the texture. GPU sampling replaces the per-frame smooth-scaled
   rescale entirely, and the existing `computeSourceRect` zoom/pan window becomes texture
   coordinates.
3. Overlay — a single textured quad carrying the existing cached axis layer *plus* the
   markers, marker chips, hover cursor, and tooltip that are currently drawn on every paint.
   It is re-rasterized only when one of its inputs changes, which extends the dirty-tracking
   that `m_axisDirty` already does for the axes.

Marker chip hit-testing keeps reading `m_chipHitRects`, which the overlay rasterizer
populates exactly as the current paint path does.

## Hotpath & threading impact

- **Touches the hotpath?** **No.** This is presentation only. `FrameReader`,
  `CircularBuffer`, `FrameBuilder`, the span fast lane, and `Dashboard::onDisplayTick` are
  not modified. Both widgets remain consumers that read `UI::Dashboard` from the GUI thread on
  the existing tick. `--benchmark-hotpath` is run as a regression check (AC5), not because the
  pipeline changes.
- **New cross-thread signal/slot?** **No.** No new connection of any kind, and no new timer.
  Repaints stay driven by the existing `Misc::TimerEvents` tick through `update()`.
- **New render-thread data access.** `updatePaintNode` reads item members and `UI::Dashboard`
  while the GUI thread is blocked in the sync phase — the same access pattern `PlotCurve`
  already relies on. The binding rule: nothing else in either widget may touch model state
  from the render thread, and no work may be deferred out of `updatePaintNode` into a later
  render-thread callback where the GUI thread is running again.
- **New input to a cached hotpath flag?** **No.** Neither widget feeds `m_operationMode`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven`, or `m_streamAvailable`.
- **Timestamp ownership.** Unaffected. Neither widget stamps or re-stamps anything; both read
  already-stamped dashboard state.

## Data model & persistence

No change. No `Keys::` additions, no schema or writer version bump, no `widgetSettings` or
project-JSON shape change, no Sessions DB change. Every persisted waterfall and 3D plot
setting keeps its current key and meaning, so existing `.ssproj` files load unchanged.

## API / SDK surface

No change. No new or modified API handlers, no `EnumLabels.cpp` entries, no generated-SDK
regeneration, no new script reach. Both widgets stay behind `BUILD_COMMERCIAL` exactly as
they are today.

## QML / UI

No new QML components and no changes to `app/qml/Widgets/Dashboard/Plot3D.qml` or
`Waterfall.qml`. Both files bind only to properties that survive the base-class swap, and
neither uses a `QQuickPaintedItem`-specific property. The registered QML type names
(`Plot3DWidget`, `WaterfallModel`) and their property surfaces are unchanged, so no project
file, no theme, and no user-facing control is affected.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Overall rendering shape | (A) all-GPU including glyph nodes; (B) hybrid — GPU for per-tick content, cached CPU texture for text; (C) port both to Qt 6.12 `QCanvasPainter` | **B.** Puts GPU work where the per-tick cost actually is; text redraws rarely and buys nothing on the GPU. C additionally needs a Qt pin bump, which the spec rules out. |
| 3D line stroking | Reuse `PlotCurve`'s extrusion; write a second stroker for 3D; use `QSGGeometry::DrawLines` | **Reuse, extracted into `GpuStroke`.** One stroking technique in the project. `DrawLines` gives no width control and no antialiasing without MSAA. |
| Waterfall texture updates | Recreate the texture when a row is written; partial single-row upload through `QRhi` | **Recreate on row write.** The expensive part today is the per-frame *rescale*, not the upload; the source image is small and rows arrive at tick rate. Partial upload is a named follow-up if profiling shows upload dominating — not speculative work now. |
| Anaglyph merge | Colour-masked double draw with per-vertex channel masking; custom material with additive blending; keep the CPU merge | **Colour-masked double draw.** The spec accepts visual equivalence over bit-parity, and this removes the full-frame CPU merge with no custom shader. A custom additive material is the fallback if overlapping traces read wrong. |
| 3D projection location | Keep on CPU in `screenProjection`; move into a vertex shader | **Keep on CPU.** The camera easing and auto-scale logic reads projected extents; moving the matrix to the GPU is separable and belongs in its own change. |
| Waterfall overlay granularity | One overlay texture for axes + markers + cursor; separate small node for marker chips | **One overlay texture.** Simpler, and it extends dirty-tracking the code already has. Split the chips out only if marker-driven re-rasterization shows up in a profile. |
| Software backend | Accept degradation; maintain a parallel CPU path; remove the backend option | **Accept degradation**, per the approved spec. Matches what `PlotCurve` already does; no duplicate renderer to maintain. |

## Risks & mitigations

- **Count/emit desynchronization in the shared stroke builder** overruns the geometry buffer.
  `PlotCurve` guards this with a strict lockstep requirement between its counting and
  streaming passes plus debug post-condition asserts; `GpuStroke` must carry that contract,
  and the extraction must keep the two passes textually parallel.
- **Extracting `GpuStroke` silently changes plot curves**, a widely used, already-shipped
  path. Mitigation: the extraction is a pure move with no logic edits, verified by
  before/after screenshots of a live plot, and it lands as its own task so a regression is
  bisectable.
- **The waterfall ring wrap seams** if the two quads' texture coordinates are off by a texel.
  Mitigation: derive both sub-rects from the existing `computeSourceRect` split, which already
  encodes the correct wrap arithmetic, and check the seam explicitly against R7 while data
  scrolls.
- **Device-pixel-ratio and resize handling** currently lives in each widget's image
  allocation, which is being deleted. Mitigation: the overlay rasterizer keeps the existing
  `devicePixelRatio` sizing logic, and R10 is checked by dragging the window between displays.
- **Anaglyph regression** where traces overlap and the masked double draw diverges from the
  per-pixel merge. Mitigation: checked against R6's visual-equivalence bar; the custom
  additive material is the named fallback.
- **Translation-unit growth.** `Waterfall.cpp` is already 1998 lines and `Plot3D.cpp` 1370.
  The change removes more than it adds, but the TU census gate is checked (AC6) and
  `scripts/tu-cutter.py` splits if needed.
- **No documented scene-graph rules exist.** `common-mistakes.md` has no painting or
  render-thread entry, so the render-thread access rule above is added there as part of this
  work rather than living only in this plan.

## Test & verification plan

- **Unit (runnable here):** none apply. `tests/scripts/` covers JS frame parsing only; there
  is no test tier that exercises widget rendering.
- **Performance, maintainer-run (AC1):** `sample <pid>` against the running app with a live
  source and a dashboard carrying both widgets — one capture before any code changes, one
  after. Compared for GUI-thread rasterization and image-composition frames attributable to
  either widget. The "before" capture is a prerequisite and does not exist yet.
- **Maintainer observation in the running app:** AC2 (responsiveness with both widgets live),
  AC3 (before/after screenshots at matched size, data and theme, including stereo), AC4
  (orbit/pan/zoom, marker hover and click, pause, resize, cross-display drag), AC7 (license
  activate and deactivate with a dashboard open).
- **Hotpath (AC5):** `--benchmark-hotpath` at its configured thresholds. Expected unchanged —
  run as a regression check, since this plan does not touch the pipeline.
- **Static (AC6):** `python scripts/code-verify.py --check` over every touched file, including
  the TU and singleton census gates; `qt-cpp-review` before handoff; `python
  scripts/sanitize-commit.py` before commit.
- **Documentation:** `python scripts/claim-verify.py` after the `dashboard.md` and
  `common-mistakes.md` edits, since both name real symbols and paths.
