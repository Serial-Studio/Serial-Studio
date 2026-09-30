---
spec: 0089-hdr-rendering
phase: tasks
status: approved     # draft -> approved (gate before /ss-implement)
updated: 2026-09-29
---

# Tasks 0089 — HDR Rendering Support

> **Phase 3 of 4 — the ordered checklist.** Decomposes [`plan.md`](./plan.md). `/ss-implement`
> works this list top to bottom and keeps the status boxes current. Gate: do not start
> `/ss-implement` until a human marks this `approved`.

## Conventions

- One task = one focused, reviewable change.
- **Verify** confirms *this* unit before moving on.
- **Deps** lists task IDs that must land first.
- The app is never compiled here; `qsb`/moc correctness is the maintainer's build. Structural
  verification is `code-verify.py --check`, read-back, and the ctest/observation noted.

## Tasks

### T1 — Extended transfer math + its ctest

- **Files:** `core/Ui/Misc/HdrTransfer.h` (new), `app/tests/tst_hdr_transfer.cpp` (new),
  `app/tests/CMakeLists.txt`
- **Does:** The shared extended sRGB transfer pair (`eotfExt`, `oetfExt`: piecewise sRGB on
  [0,1], linear extension above 1, continuous at 1.0) plus the boost mapping, as constexpr-able
  pure functions. Ctest pins roundtrip, continuity, monotonicity, and boost k → k×linear.
- **Verify:** `python scripts/code-verify.py --check` on the three files; once the maintainer
  builds, `ctest -R tst_hdr_transfer` against the build dir.
- **Deps:** none
- [x] done

### T2 — GraphicsBackend HDR settings + startup resolve

- **Files:** `core/Ui/Misc/GraphicsBackend.h`, `core/Ui/Misc/GraphicsBackend.cpp`
- **Does:** Keys `App/HdrEnabled` / `App/HdrPending` / `App/HdrIntensity`; properties
  `hdrSupported` (Metal/D3D11, never Software/OpenGL), `hdrEnabled`, `hdrIntensity` (live,
  guard-return setters), `hdrAnyActive` (fed by a small register/unregister API for T3).
  `applyConfiguredBackend()` latches the static HDR request and writes the pending flag;
  `readPersistedBackend()`-style revert clears `HdrEnabled` after a crashed attempt;
  `confirmStartupSuccess()` clears both pendings. **Binding invariants: runs before
  QApplication (main.cpp:264 call site unchanged); the pending-write/confirm/revert triple
  moves in lockstep with the existing backend one; no new singleton.**
- **Verify:** `code-verify.py --check` on both files; read-back that every setter guards and
  `s_activeBackend` handling is untouched.
- **Deps:** none
- [x] done

### T3 — `Misc::HdrOutput` per-window helper + registration

- **Files:** `core/Ui/Misc/HdrOutput.h` (new), `core/Ui/Misc/HdrOutput.cpp` (new),
  `app/src/Misc/ModuleManager.cpp`
- **Does:** QML-instantiable helper: on `window` set (pre-expose, GUI thread) applies
  `_qt_sg_hdr_format = "scrgb"` when enabled+supported; samples swapchain
  `format()`/`hdrInfo()` via `getResource(RhiSwapchainResource)` **only inside a Direct
  `beforeSynchronizing` connection (render thread, GUI blocked — the safe phase), throttled
  ~1 Hz, publishing `active`/`headroom`/`sdrWhiteScale` Queued to the GUI thread**; registers
  with GraphicsBackend for `hdrAnyActive`. `qmlRegisterType` in the ModuleManager block
  (:632). **Binding invariants: never a BlockingQueuedConnection; nothing per frame; not a
  singleton (census flat); private-API property named with a comment citing
  qsgrhisupport.cpp:1544 and graceful-degradation behavior.**
- **Verify:** `code-verify.py --check`; `code-verify.py --singleton-census --check` flat.
- **Deps:** T2
- [x] done

### T4 — Free shader set + build wiring

- **Files:** `app/shaders/hdr_output.frag` (new), `app/shaders/hdr_boost.frag` (new),
  `app/shaders/stroke_hdr.vert` (new), `app/shaders/stroke_hdr.frag` (new),
  `app/CMakeLists.txt`
- **Does:** The output transform (`linear = eotfExt(c) * sdrWhiteScale`), the boost shader
  (sample source, `oetfExt(boost * eotf(c))`, premultiplied-alpha aware), and the
  vertex-color×intensity stroke pair — transfer functions verbatim from `HdrTransfer.h`
  (comment binds the copies). One `qt_add_shaders` entry in the free block.
- **Verify:** Read-back of the math against T1's header; `code-verify.py --check` on the
  CMake edit; maintainer's next build exercises qsb.
- **Deps:** T1
- [x] done

### T5 — `HdrSurface.qml` + window wiring

- **Files:** `app/qml/Widgets/HdrSurface.qml` (new), `app/qml/Widgets/SmartWindow.qml`,
  `app/qml/Widgets/SmartDialog.qml`
- **Does:** Instantiates `HdrOutput` for its window; when `active` flips true, imperatively
  enables `window.contentItem.layer` (RGBA16F, effect = output-transform ShaderEffect bound
  to `sdrWhiteScale`); fully dormant otherwise. Added once to SmartWindow and SmartDialog.
  **Binding invariants: HDR-off instantiates nothing (R3 parity is structural); raw
  `Window{}` dialogs stay unwrapped by design; contentItem layer, never a re-parent of
  children.**
- **Verify:** `code-verify.py --check` on the QML; read-back that SmartWindow/SmartDialog
  diffs are one insertion each.
- **Deps:** T3, T4
- [x] done

### T6 — `HdrBoost.qml` emissive primitive

- **Files:** `app/qml/Widgets/HdrBoost.qml` (new)
- **Does:** Loader-gated ShaderEffect over an explicit
  `ShaderEffectSource { hideSource: true }` (**invariant: hideSource always — a stacked
  overlay double-blends translucent sources**); props `target`, `active`, `boost`
  defaulting to `min(Cpp_Misc_GraphicsBackend.hdrIntensity, headroom)`.
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T4, T5
- [x] done

### T7 — Settings page: toggle, slider, status note

- **Files:** `app/qml/Dialogs/Settings/SettingsStartupPage.qml`
- **Does:** "HDR output" switch (visible on `hdrSupported`, restart flow via
  `promptRestartAndQuit()`), intensity slider (live), passive "active on this display" note
  from `hdrAnyActive`. **Binding invariant: slider syncs from the setting only while not
  pressed and clamps at commit (common-mistakes: editable-field echo).**
- **Verify:** `code-verify.py --check`; read-back of the binding direction.
- **Deps:** T2
- [x] done

### T8 — Emissive Critical alarm flashes (Bar/Gauge/Meter)

- **Files:** `app/qml/Widgets/Dashboard/Bar.qml`, `app/qml/Widgets/Dashboard/Gauge.qml`,
  `app/qml/Widgets/Dashboard/Meter.qml`
- **Does:** `HdrBoost` over each value box and digital readout, active on
  `flash.filled && severity == 3 && hasData`. **Binding invariants: alarm *semantics*
  untouched (AlarmFlash timing, AlarmMonitor, hasData gate all read-only); Warning stays
  SDR (spec R7).**
- **Verify:** `code-verify.py --check`; read-back that only visuals changed.
- **Deps:** T6
- [x] done

### T9 — Emissive master annunciator

- **Files:** `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml`
- **Does:** `HdrBoost` over the bell icon during the fast (Critical) flash only; ringback
  and lower priorities stay SDR. **Invariant: `_flash`/sequence timing untouched.**
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T6
- [x] done

### T10 — Emissive LED cores

- **Files:** `app/qml/Widgets/Dashboard/LEDPanel.qml`
- **Does:** `HdrBoost` over the lit LED circle, stacked **above** the two existing
  `MultiEffect` glows. **Binding invariant: the MultiEffects and their SDR source stay
  byte-identical (halo remains SDR by decision — the plan's accepted clamp).**
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T6
- [x] done

### T11 — Emissive FFT markers

- **Files:** `app/qml/Widgets/Dashboard/FFTPlot.qml`
- **Does:** `HdrBoost` on band-edge strokes, point-marker core, and the alarm-blink state;
  the translucent band bloom rides the hideSource path. **Invariant: marker monitoring and
  hit-rect/spotlight logic untouched.**
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T6
- [x] done

### T12 — `StrokeHdrMaterial`

- **Files:** `core/Ui/UI/Widgets/StrokeHdrMaterial.h` (new),
  `core/Ui/UI/Widgets/StrokeHdrMaterial.cpp` (new)
- **Does:** `QSGMaterial`+shader class on the `Plot3DEyeMaterial` vendored-shader pattern:
  premultiplied vertex color × intensity uniform; `compare()` orders by intensity so
  batching survives. **Binding invariants: no allocation in `updateUniformData`; free
  license header (dual GPL/commercial like GpuStroke, not commercial-only).**
- **Verify:** `code-verify.py --check`; read-back against EyeMaterial's shape.
- **Deps:** T4
- [x] done

### T13 — Conditional stroke material selection

- **Files:** `core/Ui/UI/Widgets/PlotCurve.cpp`, `core/Ui/UI/Widgets/GpuStroke.cpp`
- **Does:** Curve/point nodes take `StrokeHdrMaterial` through the existing
  `MaterialFactory` hook when the item's window is HDR-active and intensity > 1; stock
  `QSGVertexColorMaterial` otherwise. **Binding invariants (hotpath-adjacent — re-read both
  files first): geometry path, `reserveGeometry`/`padGeometryTail` contract and
  `kMaxGeometry` untouched; material swap only at node (re)build, never per frame;
  `tst_plot_curve_geometry` must stay green.**
- **Verify:** `code-verify.py --check`; `ctest -R tst_plot_curve_geometry` against an
  existing build once rebuilt.
- **Deps:** T12
- [x] done -- note: GpuStroke.cpp needed no edit (its factory hook serves Plot3D only;
  PlotCurve owns its node directly), and PlotCurve.h gained the `m_hdrMaterial` member;
  the `hdrBoost` derived property landed in SmartWindow/SmartDialog (T5 files).

### T14 — Waterfall HDR shaders (commercial block)

- **Files:** `app/shaders/waterfall_hdr.vert` (new), `app/shaders/waterfall_hdr.frag`
  (new), `app/CMakeLists.txt`
- **Does:** Textured-quad vertex stage; fragment samples the R16 magnitude ring, looks up
  the 256×1 LUT, applies the top-of-scale boost ramp (~top 20 % → intensity), transfer
  functions matching T1. `qt_add_shaders` under `BUILD_COMMERCIAL` beside
  `plot3d_eye_shaders` (:879).
- **Verify:** Read-back vs T1 math; `code-verify.py --check` on the CMake edit.
- **Deps:** T1
- [x] done

### T15 — `WaterfallRingTexture` R16 arm

- **Files:** `core/Ui/UI/Widgets/Waterfall/WaterfallRingTexture.h`,
  `core/Ui/UI/Widgets/Waterfall/WaterfallRingTexture.cpp`
- **Does:** Constructor-time format selection (`BGRA8` today / `R16` HDR), matching
  bytes-per-pixel in the staging slots, `supported()` gains the
  `isTextureFormatSupported(R16)` arm (single-channel — the BGRA8 endianness reasoning
  does not transfer and must not be copied). **Binding invariants: stage at sync from the
  widget's image, upload at prepare from owned memory only; ring state
  (`m_topRow`/`m_writeRow`/`m_filledOnce`) and seam math untouched.**
- **Verify:** `code-verify.py --check`; read-back of the staging path;
  `ctest -R tst_waterfall_tiles` stays green (tiles math untouched).
- **Deps:** none (pairs with T16/T17 before it activates)
- [x] done

### T16 — `WaterfallHdrMaterial` + LUT texture plumbing

- **Files:** `core/Ui/UI/Widgets/Waterfall/WaterfallHdrMaterial.h` (new),
  `core/Ui/UI/Widgets/Waterfall/WaterfallHdrMaterial.cpp` (new),
  `core/Ui/UI/Widgets/Waterfall/WaterfallColorMap.h/.cpp` (LUT-as-QImage accessor)
- **Does:** Material binding ring texture + LUT texture + boost/intensity uniforms; LUT
  image comes from the existing 256-entry bake, uploaded via `createTextureFromImage` only
  on colormap change. **Invariants: no per-frame texture creation; commercial license
  header (Pro widget).**
- **Verify:** `code-verify.py --check`; read-back.
- **Deps:** T14, T15
- [x] done -- note: the LUT QImage is built in the widget from its existing bake
  (no new WaterfallColorMap accessor needed).

### T17 — Spectrogram node HDR path

- **Files:** `core/Ui/UI/Widgets/Waterfall/WaterfallSpectrogramNodes.h`,
  `core/Ui/UI/Widgets/Waterfall/WaterfallSpectrogramNodes.cpp`
- **Does:** Ring path draws a `QSGGeometryNode` with `WaterfallHdrMaterial` when HDR is
  active, `QSGSimpleTextureNode` otherwise. **Binding invariants: the 64-row tile fallback
  is untouched in every mode; dirty-row/dirty-band bookkeeping and `WaterfallTiles`
  decomposition shared by both paths exactly as today.**
- **Verify:** `code-verify.py --check`; `ctest -R tst_waterfall_tiles`.
- **Deps:** T16
- [x] done

### T18 — Waterfall history: 16-bit magnitude rows

- **Files:** `core/Ui/UI/Widgets/Waterfall.h`, `core/Ui/UI/Widgets/Waterfall.cpp`
- **Does:** HDR path stores normalized magnitude in a `Format_Grayscale16` history image
  (row bake stops applying the LUT on CPU); path chosen once at texture creation from the
  window's HDR state. **Binding invariants (invoke `ss-hotpath` before this edit — the file
  borders Dashboard-fed ingest): SDR path byte-identical; idle gate
  (`waterfallGeneration`) and hidden-release behavior untouched; Campbell row placement
  untouched; no allocation in the per-row bake.**
- **Verify:** `code-verify.py --check`; read-back diff shows the SDR branch unchanged.
- **Deps:** T15, T16, T17
- [x] done -- note: the HDR/SDR mode re-chooses on window HDR-state change (rebuild = history
  reset, consistent with the hidden-release behavior) rather than latching forever; ring
  failure falls back to the SDR image so tiles keep correct colors.

### T19 — Docs: dashboard.md + CLAUDE.md pointer

- **Files:** `doc/claude/architecture/dashboard.md`, `CLAUDE.md`
- **Does:** New "HDR output" subsection (transform-layer contract, the >1.0-producer
  inventory, the MultiEffect-RGBA8 clamp rule, private-API note) and the waterfall
  dual-format paragraph; one subsystem-contracts table row in CLAUDE.md.
- **Verify:** `python scripts/claim-verify.py` resolves every path/symbol the new text
  names; `documentation-verify.py` not needed (AI-facing docs, not doc/help).
- **Deps:** T5, T13, T18
- [x] done

### T20 — Whole-feature verification pass

- **Files:** none (checks only)
- **Does:** Full static sweep + `qt-cpp-review` on the C++ diff + self-review of the diff
  against the plan's file table (lane check) + AC8 emissive-route checklist walked and
  recorded in this file's notes.
- **Verify:** Definition of Done below.
- **Deps:** all
- [x] done -- qt-cpp-review ran (6 agents): confirmed findings all fixed (cross-thread
  sender() that killed HDR activation, missing QML_SOURCES registration, moc-incomplete
  QQuickWindow* property, Rule of Five on WaterfallHdrMaterial, NaN-sticky intensity,
  per-drag QSettings sync, D3D11 Gray16 full-upload copy, phantom factory seam, duplicated
  boost reader, CMake indent). Hardening added: roster destroyed() hook, LUT-null render
  guard, ring-failure queued rebuild, sync-time caps cache. Accepted notes: one-frame blank
  on ring-failure fallback; LED halo SDR clamp. Runtime shakedown on the maintainer's XDR
  MacBook caught two more (both fixed): HdrBoost/HdrSurface missing from QML_SOURCES
  (QML load failure), and the output-transform ShaderEffect missing its `property variant
  source` declaration -- the layer texture never bound, Metal sampled an unbound texture
  and the window rendered blank white (verified via offscreen qml probe: the
  "'source' does not have a matching property" warning, gone after the fix). R6 was
  amended the same day: intensity slider removed, automatic min(2 x SDR white, headroom).

**AC8 emissive-route checklist (spec R12 — each route verified free of RGBA8 clamps by
read-back; on-screen confirmation against a reference-white patch is the maintainer's):**

| Element | Route to display | Clamp check |
|---|---|---|
| Bar/Gauge/Meter value box + digital readout (Critical flash) | `HdrBoost` (hideSource ShaderEffectSource, RGBA8 source is SDR content — fine; boost applied in shader output) → FP16 window layer → output transform | no intermediate after boost |
| Master annunciator bell (fast flash) | same `HdrBoost` route | no intermediate after boost |
| LED core (lit) | `HdrBoost` stacked above the two MultiEffect glows | halo stays SDR (accepted, documented) |
| FFT band-edge strokes + point line | per-rectangle `HdrBoost` | band bloom stays SDR by design (large translucent surface) |
| Plot/MultiPlot/FFT curve strokes | `StrokeHdrMaterial` on the curve node → FP16 layer | direct scene-graph draw, no intermediate |
| Waterfall colormap peaks | `WaterfallHdrMaterial` (R16 ring + LUT texture) → FP16 layer | direct scene-graph draw, no intermediate |
| Whole-window content | RGBA16F contentItem layer → `hdr_output.frag` → FP16 swapchain | layer format explicitly RGBA16F before enable |

Known SDR-capped surfaces under HDR (accepted): the disconnect grayscale layer in
`WidgetDelegate.qml` (RGBA8; no alarms while disconnected), MiniWindow drop shadows, chrome
rings behind MultiEffect shadows.

## Definition of Done

- [ ] Every acceptance criterion in `spec.md` is met or handed to the maintainer's
      observation list (AC1–AC8 need the HDR displays; AC9/AC10 run per plan).
- [x] `python scripts/code-verify.py --check` clean on all changed files (no new errors).
- [x] `qt-cpp-review` run on the C++ diff; findings addressed or noted.
- [x] Hotpath untouched as planned: no acquisition-pipeline file in the diff;
      `--benchmark-hotpath` expected flat (maintainer/CI confirms).
- [ ] `ctest -R "tst_hdr_transfer|tst_waterfall_tiles|tst_plot_curve_geometry"` green
      against the maintainer's build.
- [ ] `pytest tests/ -m "not destructive"` green with HDR off and on (app up, API server
      enabled).
- [x] `python scripts/sanitize-commit.py` run; `--singleton-census` and `layer-verify.py`
      flat.
- [x] Diff is *what was asked, and only that* — matches the plan's file table; any
      discovered extra file named in chat first.
- [ ] `spec.md` status set to `done`.
