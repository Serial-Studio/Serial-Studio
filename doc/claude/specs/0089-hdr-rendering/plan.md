---
spec: 0089-hdr-rendering
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-09-29
---

# Plan 0089 — HDR Rendering Support

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Grounded in the Qt 6.11.2 sources (qtbase `qrhimetal.mm`,
> `qrhid3d11.cpp`, `qdxgihdrinfo.cpp`; qtdeclarative `qsgrhisupport.cpp`) and the repo code
> read this session.

## Approach (one paragraph)

Each opted-in window requests an FP16 swapchain per window (the `_qt_sg_hdr_format` window
property `qsgrhisupport.cpp` reads, with automatic per-window SDR fallback), and wraps its
`contentItem` in an RGBA16F layer whose `layer.effect` is a fullscreen **output transform**
shader. Inside the layer everything composites exactly as today (encoded-sRGB math,
byte-identical — SDR parity is structural, not audited); the transform owns the one thing Qt
Quick provably does not do (no linearization anywhere in the 2D scene graph, `sdrWhiteLevel`
only ever debug-printed): sRGB→linear conversion, extended >1.0 pass-through, and the
Windows SDR-white multiply (`sdrWhiteLevel/80`, scene-referred; macOS is display-referred,
multiplier 1). Emissive elements are then trivial: anything that writes encoded values >1.0
into the layer exceeds SDR white on screen. Three producers of >1.0 values get built: a
reusable `HdrBoost` ShaderEffect wrapper (alarm flashes, LEDs, FFT markers), an
`EmissiveStroke` scene-graph material behind the existing `GpuStroke::MaterialFactory` hook
(plot curves), and a waterfall ring material that moves the colormap LUT onto the GPU
(16-bit magnitude ring, de-banding + peaks, half today's upload bytes).

## Affected subsystems & files

| File | Change |
|------|--------|
| `core/Ui/Misc/GraphicsBackend.h/.cpp` | New persisted keys `App/HdrEnabled`, `App/HdrPending` (crash-revert, same pattern as `pendingKey()`), `App/HdrIntensity` (double, live). New properties: `hdrSupported` (platform+backend: Metal/D3D11 and not Software), `hdrEnabled`, `hdrIntensity`, `hdrAnyActive` (aggregated from registered `HdrOutput` instances, for the Preferences status note). |
| `core/Ui/Misc/HdrOutput.h/.cpp` **(new)** | QML-instantiable per-window helper (`qmlRegisterType`, not a singleton — census untouched). `window` property; on set (GUI thread, pre-expose): `window->setProperty("_qt_sg_hdr_format", "scrgb")` when enabled+supported. Samples `QRhiSwapChain::format()`/`hdrInfo()` via `QSGRendererInterface::getResource(RhiSwapchainResource)` inside a `beforeSynchronizing` Direct connection (render thread, GUI blocked ⇒ safe), throttled ~1 Hz; publishes `active`, `headroom`, `sdrWhiteScale` to the GUI side queued. Registers/unregisters itself with `GraphicsBackend` for `hdrAnyActive`. |
| `core/Ui/Misc/HdrTransfer.h` **(new)** | The shared C++ copy of the extended transfer pair (encode/decode, linear >1 extension, continuous at 1.0) so a ctest can pin the math the shaders implement. |
| `app/shaders/hdr_output.frag` **(new)** | Output transform: `linear = srgbEOTF_ext(c) * sdrWhiteScale`. |
| `app/shaders/hdr_boost.frag` **(new)** | Emissive boost: samples source, `out = oetf_ext(boost * eotf(c))`, premultiplied-alpha aware. |
| `app/shaders/stroke_hdr.vert/.frag` **(new)** | Vertex-color stroke × `intensity` uniform (EyeMaterial vendored-shader pattern). |
| `app/shaders/waterfall_hdr.vert/.frag` **(new)** | Ring-texture sample (R16 magnitude) → LUT texture lookup → top-of-scale boost ramp. |
| `app/CMakeLists.txt` | `qt_add_shaders` entries: hdr_output/hdr_boost/stroke_hdr free; waterfall_hdr under `BUILD_COMMERCIAL` (next to `plot3d_eye_shaders`, :879). |
| `app/qml/Widgets/HdrSurface.qml` **(new)** | Per-window wrapper: instantiates `HdrOutput`, and when it goes active enables `window.contentItem.layer` (format RGBA16F, effect = the output-transform ShaderEffect) imperatively. Dormant (zero cost, nothing instantiated) when HDR is off or unsupported. |
| `app/qml/Widgets/HdrBoost.qml` **(new)** | Loader-gated ShaderEffect over an explicit `ShaderEffectSource { hideSource: true }` (a stacked overlay would double-blend translucent sources). Props: `target`, `boost` (defaults to the global intensity), `active`. |
| `app/qml/Widgets/SmartWindow.qml`, `SmartDialog.qml` | Add `HdrSurface` — covers MainWindow (derives from SmartWindow) and the 47 consumer windows, widget pop-outs included. The ~9 raw `Window{}` dialogs (`OpcUaTagBrowser`, `Welcome`, editors' test dialogs, …) intentionally stay SDR. |
| `core/Ui/UI/Widgets/StrokeHdrMaterial.h/.cpp` **(new)** | `QSGMaterial` + shader (EyeMaterial pattern): premultiplied vertex color × intensity uniform; `compare()` on intensity so batching survives. |
| `core/Ui/UI/Widgets/PlotCurve.cpp` (:358 material site), `GpuStroke.cpp` | Select `StrokeHdrMaterial` through the existing `MaterialFactory` hook when the item's window is HDR-active and intensity > 1; stock `QSGVertexColorMaterial` otherwise (parity + batching). `PlotAreaFill` stays stock (fills are large surfaces — spec constraint). |
| `core/Ui/UI/Widgets/Waterfall.cpp/.h` | HDR path: history image becomes `Format_Grayscale16` normalized magnitude (row bake stops applying the LUT on CPU, :598 area); path chosen once at texture creation from `HdrOutput` state. SDR path byte-identical to today (RGB32 + CPU LUT). |
| `core/Ui/UI/Widgets/Waterfall/WaterfallRingTexture.cpp/.h` | Format parameter: `R16` (2 B/px, half of BGRA8) alongside today's `BGRA8`; `supported()` gains an `isTextureFormatSupported(R16)` arm. Staging contract (stage at sync, upload at prepare, own memory only) unchanged. |
| `core/Ui/UI/Widgets/Waterfall/WaterfallHdrMaterial.h/.cpp` **(new)** | Material + shader: ring texture, 256×1 LUT texture (from `WaterfallColorMap`'s existing bake, uploaded once per map change), boost uniform ramping the top ~20 % of scale to `intensity`. |
| `core/Ui/UI/Widgets/Waterfall/WaterfallSpectrogramNodes.cpp/.h` | Ring path draws a `QSGGeometryNode` with `WaterfallHdrMaterial` when HDR; `QSGSimpleTextureNode` otherwise. Tile fallback untouched (stays the SDR CPU-LUT path in every mode). |
| `app/qml/Widgets/Dashboard/Bar.qml`, `Gauge.qml`, `Meter.qml` | `HdrBoost` over the value box + digital readout, active on `flash.filled && severity == 3` (Critical only, per spec R7). |
| `app/qml/MainWindow/Panes/Dashboard/MasterAnnunciator.qml` | `HdrBoost` over the bell icon during the fast (Critical) flash. |
| `app/qml/Widgets/Dashboard/LEDPanel.qml` | `HdrBoost` over the lit LED core, stacked above the two existing `MultiEffect` glows (their RGBA8 source stays SDR — the glow halo remains SDR, the core goes emissive; no double-blend since the LED is opaque). |
| `app/qml/Widgets/Dashboard/FFTPlot.qml` | `HdrBoost` on band-edge strokes, point-marker core and alarm-blink state (translucent band bloom uses the hideSource path). |
| `app/qml/Dialogs/Settings/SettingsStartupPage.qml` | HDR toggle + intensity slider + passive "HDR active on this display" note, next to the Rendering Backend rows; restart prompt reuses `promptRestartAndQuit()`. Slider follows the `TableDelegate` unfocused-sync rule (common-mistakes: editable field echo). |
| `app/src/Misc/ModuleManager.cpp` (:632 block) | `qmlRegisterType<Misc::HdrOutput>`. |
| `core/Ui/CMakeLists.txt` | Source-list entries for the new file pairs (explicit lists, no glob) — added during implement, named in chat 2026-09-29. |
| `app/tests/` (+ its CMake) | `tst_hdr_transfer`: pins encode/decode roundtrip, continuity at 1.0, monotonicity, boost mapping (pure functions from `HdrTransfer.h`). |
| `doc/claude/architecture/dashboard.md` | New "HDR output" subsection (transform-layer contract, the >1.0-producer inventory, the MultiEffect-RGBA8 clamp rule); waterfall section gains the dual-format ring. |
| `CLAUDE.md` | One line in the subsystem-contracts table pointing at the new subsection (architectural change future-me would miss). |

## Architecture & data flow

**Startup.** `applyConfiguredBackend()` (`main.cpp:264`, pre-QApplication) additionally
resolves the HDR decision: `hdrEnabled && hdrSupported` latches a static `s_hdrRequested`,
writes `App/HdrPending`, and the crash-revert in `readPersistedBackend()`'s pattern clears
`HdrEnabled` if the previous attempt never confirmed startup (`confirmStartupSuccess()`
clears both pendings). No composition-root ordering change; nothing touches
`instantiateCoreModules()`.

**Per window.** `HdrSurface` (inside SmartWindow/SmartDialog) creates `HdrOutput`, which
sets the swapchain-format property before first expose. Qt requests the FP16 swapchain and
falls back to SDR itself when the display can't (`qsgrhisupport.cpp` `applySwapChainFormat`
— R4 comes from Qt, we only report it). When `HdrOutput.active` flips true, `HdrSurface`
enables the contentItem RGBA16F layer with the output-transform effect; when false
(SDR monitor, setting off) nothing is instantiated and rendering is today's, byte for byte.

**Runtime values.** `HdrOutput` samples format + `hdrInfo()` on the render thread during
sync (GUI blocked — the only phase where `getResource` is race-free with the threaded
render loop), ~1 Hz. `headroom` = `maxColorComponentValue` (macOS, display-referred) or
`maxLuminance/sdrWhiteLevel` (Windows); `sdrWhiteScale` = 1.0 or `sdrWhiteLevel/80`.
Windows OS-slider moves land within a second (AC4). The transform effect binds
`sdrWhiteScale`; every boost consumer binds `min(GraphicsBackend.hdrIntensity, headroom)`
— the slider is live with no restart (R6).

**Emissive producers.** All three write encoded values through the shared extended transfer
(`HdrTransfer.h` ⟷ shader copies): `HdrBoost` (QML overlay), `StrokeHdrMaterial` (curves),
`WaterfallHdrMaterial` (colormap). The output transform maps encoded >1.0 linearly through,
so "boost k" reads as k × SDR white on screen, on both platforms, after white scaling.

**Waterfall.** Per dashboard.md's contract: rows still stage at sync into preallocated
slots and upload at prepare; only the bytes change (2-byte magnitudes instead of 4-byte
LUT colors — R11's "no increase" is a halving). The LUT becomes a 256×1 texture created
from `WaterfallColorMap`'s existing bake via `createTextureFromImage`, re-created only on
colormap change. Overlay (axes/markers/Campbell) stays the SDR CPU raster it is today.

## Hotpath & threading impact

- **Touches the hotpath?** No. No file on the acquisition pipeline (`FrameReader`,
  `CircularBuffer`, `FrameBuilder`, drivers, publish path) is edited; Dashboard ingest and
  push tables untouched. All changes are widget render-side (scene-graph sync/prepare) and
  window composition. `--benchmark-hotpath` runs offscreen SDR and must be unchanged (AC9);
  the `ss-hotpath` skill will still be invoked at implement time before the Waterfall edits
  since they border Dashboard-fed state.
- **New cross-thread signal/slot?** Yes, one: `HdrOutput` connects
  `QQuickWindow::beforeSynchronizing` **Direct** (runs on the render thread by design; GUI is
  blocked during sync, which is the documented-safe window for `getResource`) and publishes
  results to the GUI thread **Queued** at ~1 Hz. Nothing per frame, nothing blocking, no
  GUI→render wait — the forbidden `BlockingQueuedConnection` pattern does not appear.
- **New input to a cached hotpath flag?** None. No flag in the dataflow.md set gains an
  input.
- **Timestamp ownership** — untouched; no timestamps involved.
- **Render-side budget (named because it is the real cost):** per HDR window, one FP16
  content layer (≈ 8 B/px: ~66 MB at 4K, ~118 MB at 5K retina) plus one fullscreen transform
  pass, plus the FP16 swapchain Qt allocates. HDR-off cost is exactly zero (nothing
  instantiated). Waterfall per-row upload halves; `StrokeHdrMaterial` adds one small uniform
  buffer per curve, no per-frame allocation anywhere (materials own their buffers, geometry
  path untouched — spec constraint upheld).

## Data model & persistence

QSettings only: `App/HdrEnabled` (bool), `App/HdrPending` (crash revert), `App/HdrIntensity`
(double 1.0–8.0, clamped to live headroom at the consumer). No `Keys::`, no project JSON, no
schema bump, no Sessions DB change, no `widgetSettings`. Old installs see defaults (off).

## API / SDK surface

None. No API handler, no SDK, no script reach (spec has no such requirement). Free feature —
no `BUILD_COMMERCIAL` gating except the waterfall material living with its Pro widget.

## QML / UI

Covered in the file table: two new reusable components (`HdrSurface`, `HdrBoost`), six
consumer QML edits, one Settings page. Theme untouched — colors stay SDR-referenced; boost
is multiplicative, never a theme color. Registry/commands/icons untouched (no new command,
no new icon). `HdrBoost` is Loader-gated so non-HDR sessions instantiate nothing.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Output correctness stage | (a) per-window FP16 content layer + transform pass; (b) native CAMetalLayer colorspace override (nonlinear extended sRGB); (c) per-material linearization | **(a)** — (b) is macOS-only (DXGI FP16 admits only the linear G10 space, `qrhid3d11.cpp:5358`) and fights Qt re-setting the colorspace on every swapchain resize; (c) is impossible for Qt's stock materials. (a) also makes SDR parity and the linear-blending/QtGraphs risks *structural* — composition inside the layer is byte-identical to today. |
| HDR activation mechanism | per-window `_qt_sg_hdr_format` property vs process-wide `QSG_RHI_HDR` env var | **Per-window** — raw `Window{}` dialogs without the wrapper must stay SDR (an FP16 swapchain without the transform renders wrong gamma). The property is private API; risk row below. |
| Setting semantics | restart-applied + crash-revert vs live toggle | **Restart** — matches the backend selector UX and machinery; a live toggle needs swapchain recreation Qt doesn't expose. Intensity slider is the live half. |
| Waterfall HDR | (a) R16 magnitude ring + GPU LUT, HDR-gated; (b) FP16 CPU-colored ring; (c) boost-only shader on BGRA8 | **(a)** — (b) doubles upload (violates R11); (c) keeps both quantizations (no de-banding). Gating on HDR keeps R3 parity: an SDR user's waterfall is bit-identical to today. |
| Curve material swap | swap to `StrokeHdrMaterial` only when HDR-active && intensity>1 vs always | **Conditional** — stock material keeps today's batching and guarantees parity; at 1× the two are visually identical anyway (AC5's slider-at-1× check). |
| Emissive primitive | QML ShaderEffect wrapper vs C++ QQuickItem+material | **ShaderEffect** (`HdrBoost`) — the consumers are flat boxes/icons/strokes already drawn by existing items; sampling them and multiplying is 1 file + 1 shader, no registration, no geometry code. The C++ material form still exists where it must (curves, waterfall). |
| LED glow under HDR | re-plumb the MultiEffect chain to RGBA16F vs emissive core over SDR halo | **Core-over-halo** — the halo's RGBA8 clamp is invisible under a >1.0 core; re-formatting MultiEffect intermediates costs memory on every LED for no visible gain. (Chrome-ring de-banding stays a spec non-goal for the same reason.) |

## Risks & mitigations

- **`_qt_sg_hdr_format` is private Qt API** (read in `qsgrhisupport.cpp:1544`). Mitigation:
  behavior on removal is graceful (property ignored → SDR swapchain → `active` stays false →
  wrapper dormant — degraded, never broken); a comment names the Qt source line; re-verify on
  any Qt bump (the 6.11.2 canary anchor discipline).
- **`getResource(RhiSwapchainResource)` thread validity** — sampled only inside
  `beforeSynchronizing` (GUI blocked). The 1 Hz throttle keeps it off the per-frame cost.
- **Double-blend on translucent boost sources** — `HdrBoost` always uses
  `ShaderEffectSource.hideSource: true`; named in the component's doc comment.
- **Transform-layer interaction with popups/Overlay** — QML `Popup`s render inside
  `contentItem`, so they're covered; real child `Window`s are separate swapchains (SDR unless
  wrapped). Verified at runtime on macOS as part of AC2/AC3.
- **Slider echo loop** (common-mistakes "editable field with live binding") — the Settings
  slider syncs from the setting only while not pressed; clamp at commit.
- **Setter guards** — every new property setter follows the guard-return rule.
- **Waterfall regressions** — the SDR path is untouched code; the HDR path preserves the
  staging/upload split and the `tst_waterfall_tiles` seam math (tiles fallback unchanged).
  The `supported()` endianness reasoning doesn't apply to R16 (single channel), noted in the
  format arm.
- **Silent emissive clamping (spec R12)** — the AC8 checklist below enumerates every
  producer's route; the one known clamp (LED halo) is a named, accepted SDR element.
- **Memory on 5K + HDR** — named in the budget above; it is the opt-in cost of the feature,
  stated in the Settings help text.
- **Implement-time amendments (2026-09-29):** `PlotCurve` builds its scene-graph node
  directly, so the `GpuStroke::MaterialFactory` hook went unused — the material is selected
  inline in `updatePaintNode` (`GpuStroke.cpp` unchanged); `app/CMakeLists.txt` also carries
  the two new QML files in `QML_SOURCES`; the qt-cpp-review pass added the roster
  `destroyed()` hook, NaN guards on intensity, the shared `HdrOutput::effectiveBoost`
  reader, the sync-time caps cache, and the ring-failure rebuild path. Post-shakedown
  maintainer requests (same day): intensity slider dropped for automatic
  min(2 x SDR white, headroom); the HDR machinery gates on the launch-latched `hdrRequested`
  (a live toggle re-plumbing running windows crashed); BarPanel Critical bars and Gauge
  Critical band arcs joined the emissive set; steady-lit LEDs take a softened boost (flash =
  full); PlotAreaFill takes half the window boost (overriding the earlier "fills stay
  stock" line).
- **Trust/lane**: the file table above is the lane; anything discovered mid-implement
  (e.g. a 48th window base) gets named in chat before it's touched.

## Test & verification plan

- **Unit (I can run):** `ctest -R tst_hdr_transfer` against an existing build dir — pins the
  transfer math (roundtrip, continuity at 1.0, monotonic, boost mapping k → k×linear).
  Existing `tst_waterfall_tiles`, `tst_plot_curve_geometry` must stay green (waterfall seam
  math and stroke geometry contracts unchanged).
- **Static (I run):** `python scripts/code-verify.py --check` on every touched file;
  `qt-cpp-review` before handoff; `python scripts/sanitize-commit.py` pre-commit;
  layer/singleton censuses must be flat (no new singleton, no cross-library reach — all new
  C++ sits in `SerialStudioUi` and includes downward only).
- **Regression (AC9/AC10):** `--benchmark-hotpath` unchanged (CI gate; offscreen SDR path
  doesn't instantiate any of this); `pytest tests/ -m "not destructive"` + ctest tier with
  HDR off and on (maintainer runs the HDR-on app; I run the suites against it).
- **Maintainer observations (XDR MacBook + a Windows HDR box):**
  - AC1 — enable → restart → survives; kill during startup with pending set → reverts.
  - AC2 — HDR-off before/after screenshots identical (structural, but verified).
  - AC3 — drag between HDR/SDR monitors; per-window fallback is Qt's, wrapper follows.
  - AC4 — Windows OS SDR-brightness slider tracks within ~1 s.
  - AC5 — intensity slider live during a Critical flash; 1× matches SDR appearance.
  - AC6 — demo project beside a reference-white patch: flash boxes, annunciator bell, LED
    cores, FFT markers, curves each visibly exceed it.
  - AC7 — slow-gradient waterfall shows no banding steps; upload bytes halved (reasoned +
    `QSG_INFO` log check).
  - AC8 — the emissive-route checklist: valueBox/readout boost (Bar/Gauge/Meter), bell
    boost, LED core boost, FFT marker boosts, `StrokeHdrMaterial`, `WaterfallHdrMaterial` —
    each confirmed on-screen above reference white; LED halo documented as the accepted SDR
    element.
