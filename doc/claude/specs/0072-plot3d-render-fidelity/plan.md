---
spec: 0072-plot3d-render-fidelity
phase: plan
status: approved     # draft -> approved (gate before /ss-tasks)
updated: 2026-08-27
---

# Plan 0072 — Plot3D Render Fidelity

> **Phase 2 of 4 — the HOW.** The technical design that satisfies every requirement in
> [`spec.md`](./spec.md). Read the relevant `doc/claude/` sub-docs and the *actual code*
> before writing this — a plan grounded in a stale mental model is worse than no plan.
> Gate: do not start `/ss-tasks` until a human marks this `approved`.

## Approach (one paragraph)

Stop trying to express "this eye owns these channels" as a *colour* and express it as a
*render state*. `QSGMaterialShader::GraphicsPipelineState` carries a `colorWrite` mask, so a
material that sets it makes the GPU leave the unowned channels alone — which is exactly what
the pre-0071 per-channel merge did, and is the only way to get there without the two eyes
attenuating each other (R1, R2). A node carries one material, so the two eyes stop sharing
accumulators and nodes: each of grid, axis and trace becomes a two-slot array, with slot 0
carrying the whole picture in mono. The eye's colour *policy* (luminance on the red channel,
true green/blue on the cyan one) moves into a small dependency-free translation unit so the
`ctest` tier can verify it over every shipped theme palette without a GUI, and the material
lives in a second one so that test does not have to link Qt Quick. The interim background-fill
tint already in the tree stays as the fallback path R7 requires, selected once by a probe for
the shader resources the material needs. Defect 2 is settled separately and much more cheaply:
the item clips itself, which bounds everything it draws rather than just the trace.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/src/UI/Widgets/Plot3D/Plot3DStereo.h` | **New.** Eye enum, per-eye vertex colour derivation, per-eye channel mask, and the `channelIsolationAvailable()` probe. Depends on `QColor` only — no Qt Quick, no project types. |
| `app/src/UI/Widgets/Plot3D/Plot3DStereo.cpp` | **New.** Implementation of the above. This is the translation unit the unit suite links. |
| `app/src/UI/Widgets/Plot3D/Plot3DEyeMaterial.h` | **New.** `QSGMaterial` + `QSGMaterialShader` pair that applies a channel mask via `updateGraphicsPipelineState()`. |
| `app/src/UI/Widgets/Plot3D/Plot3DEyeMaterial.cpp` | **New.** Implementation; one static `QSGMaterialType` per mask value. |
| `app/src/UI/Widgets/GpuStroke.h` | Add a defaulted material-factory parameter to `buildStrokeNode` / `buildPointNode`. Default `nullptr` keeps the existing `QSGVertexColorMaterial` behaviour verbatim. |
| `app/src/UI/Widgets/GpuStroke.cpp` | Honour that parameter at node-creation time only (the sole place a material is attached today). |
| `app/src/UI/Widgets/Plot3D.h` | Grid / axis / trace accumulators and node pointers become two-slot arrays; `EyeMask` moves out to `Plot3DStereo.h`; `maskEyeColor` retained as the fallback tint. |
| `app/src/UI/Widgets/Plot3D.cpp` | Per-eye accumulation and node sync; `setClip(true)`; path selection between masked material and fallback tint. **Must not exceed 1500 lines** — it is at the ceiling today. |
| `app/CMakeLists.txt` | Register the two new header/source pairs inside the existing `if(BUILD_COMMERCIAL)` `set(HEADERS ...)` / `set(SOURCES ...)` blocks, next to the existing `src/UI/Widgets/Plot3D/Plot3DOverlay.*` entries. |
| `app/tests/tst_plot3d_stereo.cpp` | **New.** AC1 and AC6. |
| `app/tests/CMakeLists.txt` | Register the suite via `ss_add_unit_test`, then `target_compile_definitions(... PRIVATE BUILD_COMMERCIAL)` — the same shape `tst_opcua_wire` and `tst_tls_identity` already use for Pro-only translation units. |

Not touched, and deliberately so: `app/qml/Widgets/Dashboard/Plot3D.qml` (clipping is set on the
C++ item, so the embedded and popped-out hosts both inherit it with no QML change), and
`app/src/UI/Widgets/Waterfall.*` (every layer it places is a `setRect`-bounded texture node
derived from its own plot rect, so it has neither defect).

## Architecture & data flow

Nothing about *when* work happens changes. `updatePolish()` still runs on the GUI thread and
fills the pixel and colour accumulators; `updatePaintNode()` still runs on the render thread
with the GUI thread blocked in synchronisation, and only consumes what `updatePolish()` left
behind. This change alters what goes *into* those buffers and which material the resulting
nodes carry.

**Colour policy.** `Plot3DStereo` exposes two pure functions over `QColor`: one mapping a
source colour and an eye to the vertex colour that eye should carry, and one mapping an eye to
the set of channels it may write. For the left eye the vertex colour carries Rec. 601
luminance on red; for the right it carries the source's own green and blue. Neither fills the
unowned channels with anything, because with a write mask in force nothing ever samples them.
Alpha passes through untouched — and critically, there is no alpha cap any more: the cap only
existed because the two eyes were blending over each other, which a write mask makes
impossible.

**Path selection.** `Plot3DStereo::channelIsolationAvailable()` probes, once, for the two
scene-graph shader resources the material needs, and caches the answer in a function-local
static. `drawGrid()` and `drawData()` read it before deriving colours, because the two paths
need *different* vertex colours: the masked path wants the unmasked-but-luminance-mapped
colour, the fallback wants today's background-filled tint with its alpha cap. This is a
render-path decision only; nothing about it is persisted or exposed.

**Per-eye accumulation.** `buildGridPolylines` and `buildTracePolyline` already take an eye
argument. They gain a destination slot instead of appending into a shared buffer: eye slot 0
for the left eye and for the whole mono picture, slot 1 for the right. `drawGrid()` and
`drawData()` clear both slots, fill slot 0 in mono or both in stereo, and `updatePaintNode()`
syncs one node per non-empty slot. An empty slot's node is released by the existing builder
contract — `buildStrokeNode` deletes the node and returns `nullptr` when there is nothing to
draw — so switching stereo off frees the second eye's nodes with no extra code.

**Material attachment.** `GpuStroke::buildStrokeNode` and `buildPointNode` attach a material
only inside their `if (!node)` branch, which is the one place a material is ever set. They gain
a trailing `QSGMaterial* (*makeMaterial)() = nullptr` parameter — a plain function pointer, not
a `std::function`, so there is no allocation and no type erasure. `nullptr` means "construct a
`QSGVertexColorMaterial`", i.e. exactly what happens today. Only `Plot3D` calls these two
functions (`PlotCurve` uses the lower-level `runLength`/`countRun`/`emitRun` helpers and builds
its own node; `Waterfall` uses neither), so the blast radius of the signature change is one
caller.

**Draw order.** Unchanged. `appendSceneNodes` still flips grid and trace around the same
camera-angle threshold; within a pass the two eyes' nodes are appended adjacently. Order no
longer matters for correctness — that is the point of the mask — but keeping it stable keeps
the mono path identical.

**Clipping.** `setClip(true)` in the item's constructor. This installs a `QSGClipNode` for the
item's rectangle, which the renderer implements as a scissor for an axis-aligned rect. It bounds
grid, axes, trace, the grid-interval label and the camera indicator uniformly, in both the
embedded dashboard and the popped-out window, with no QML change and nothing to keep in sync.
It also matches the established pattern in this codebase: `PlotCurve` items already live in
`PlotWidget.curveLayer`, a clipped item tracking the plot area
(`doc/claude/architecture/dashboard.md`, "GPU Curve Rendering").

## Hotpath & threading impact

- **Touches the hotpath?** **No.** This is entirely inside `Widgets::Plot3D`'s render path. It
  does not touch `FrameReader`, `CircularBuffer`, `FrameBuilder`, the span fast lane, or
  `Dashboard`'s ingest and push tables. The 3D series still arrives through the existing
  `Plot3DPush` table and the display tick; nothing about how it gets to the widget changes.
  `--benchmark-hotpath` is unaffected and is not part of this plan's gate.
- **New cross-thread signal/slot?** **No.** No new connections at all. The existing
  GUI-thread `updatePolish()` / render-thread `updatePaintNode()` split is preserved exactly:
  the colour policy and the path probe are read on the GUI thread during polish; the material
  is constructed on the render thread during `updatePaintNode()`, which is where every other
  scene-graph object in this widget is already constructed.
- **New input to a cached hotpath flag?** **No.** Nothing here feeds `m_operationMode`,
  `m_anyAsyncSink`, `m_captureLatestFrame`, `m_changeDriven` or `m_streamAvailable`.
- **Timestamp ownership** — not applicable; no timestamped data is read, written or forwarded.
  The driver boundary remains the only stamping site.
- **Per-tick allocation** — none added. Materials and nodes are constructed once per node
  lifetime, not per tick. Splitting each accumulator into two slots doubles the number of
  `std::vector`s but not the number of allocations: each slot is cleared and refilled in place
  exactly as the single buffer is today, and the mono path only ever touches slot 0.

## Data model & persistence

None. No `Keys::` addition, no schema or writer version bump, no `widgetSettings` change, no
Sessions DB change, no migration. The stereo toggle, eye separation and eye inversion settings
keep their existing keys and meanings.

## API / SDK surface

None. No new or changed API handler, no `EnumLabels.cpp` entry, no generated SDK change, no new
script reach. The 3D plot's existing API surface is untouched.

## QML / UI

None. No new component, no new property, no new setting. Clipping is applied on the C++ item,
so `Plot3D.qml` and its pop-out host are unchanged. There is deliberately no user-visible
control for the stereo colour treatment (spec Non-Goals).

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| How to get per-channel writes | **Masked Material** reusing Qt's built-in vertex-colour shaders / **Own Shader** through `qt_add_shaders` / **Two-Pass Layer** merged by a QML `ShaderEffect` | **Masked Material.** It is the only one that adds no build step, which is the spec's hardest constraint. Two-Pass Layer costs two full-item render targets per frame and gives back the throughput the port was for. Own Shader is the designated escape hatch below, not a rejection. |
| Shader source for that material | Qt's registered `:/qt-project.org/scenegraph/shaders_ng/vertexcolor.{vert,frag}.qsb` / our own GLSL compiled by `qsb` | **Our own** (maintainer decision, 2026-08-27, overriding the draft's pick of Qt's). Reusing Qt's resource leaves two internal details load-bearing: the resource path and the uniform block layout, the second of which is not verifiable without building. Vendoring ~12 lines of GLSL makes both ours and removes the version dependency outright. `qsb` ships in the local Qt and CI installs `qt6.11-full`, so this adds a build step but no install step; `ShaderTools` joins `QT_MODULES` inside the existing `BUILD_COMMERCIAL` block, so a GPL-only configure is unaffected. |
| Bounding the drawing (R6) | **Clip the item** / **cull the trace during projection** as the grid already is | **Clip the item.** Culling only fixes the trace, leaves the overlays and any future layer exposed, breaks a line into runs *before* the edge instead of cutting it *at* the edge, and costs CPU per point. Clipping bounds everything the widget draws for one constructor call and matches the `curveLayer` precedent. |
| Where clipping is set | C++ item / `Plot3D.qml` container | **C++.** The item is reparented and anchor-filled into the QML container at runtime, so the two are equivalent in effect, but setting it on the item means the embedded and popped-out hosts cannot diverge and a future QML edit cannot silently drop it. |
| Mono's material | Reuse the masked material with an all-channels mask / keep the plain `QSGVertexColorMaterial` | **Keep the plain material.** R5 demands mono be bit-identical. An all-channels mask produces the same pixels but a different material type, which changes scene-graph batching. Having the factory return `nullptr` for mono means the mono path runs literally the same code as today. |
| Where the colour policy lives | Inside `Plot3D` / a dependency-free translation unit | **Its own translation unit.** `Plot3D` is a `QQuickItem` wired to `Dashboard`, `ThemeManager`, `TimerEvents` and `CommonFonts`; nothing in it is unit-testable. AC1 needs the policy linkable against QtGui alone, and the repo's test tier treats each suite's link set as a deliberate decision. This also keeps `Plot3D.cpp` under its size ceiling. |
| Splitting the material out from the policy | One new translation unit / two | **Two.** The material needs Qt Quick; the policy must not, or the unit suite drags Qt Quick into a link that should be QtGui-sized. |
| Interim tint's fate | Delete once the material lands / keep as the R7 fallback | **Keep** — this is the spec's answer, not a fresh decision. The cost is two colour paths; the plan contains that cost by having exactly one branch point, read in `drawGrid()` and `drawData()`. |

## Risks & mitigations

- **Uniform block layout mismatch.** ~~First-run check.~~ **Retired 2026-08-27.** Vendoring made
  the block ours, and compiling the shaders with `qsb` and dumping their reflection confirms the
  declaration matches what `updateUniformData` writes: block `buf`, binding 0, size 68, `matrix`
  at offset 0 (64 bytes, mat4), `opacity` at offset 64 (4 bytes, float). Vertex inputs are
  `vertexCoord` vec2 at location 0 and `vertexColor` vec4 at location 1, matching
  `QSGGeometry::ColoredPoint2D`'s two floats plus four normalized bytes. Nothing here is left to
  discover at runtime.
- **`ShaderTools` missing from a contributor's Qt.** A Pro configure now fails at
  `find_package` rather than at link. Acceptable: it is part of the full Qt package CI installs,
  the failure names the missing component, and GPL-only builds never ask for it.
- **Material batching collapses the two masks.** If the two eyes' materials return the same
  `QSGMaterialType` or compare equal, the renderer may batch nodes with different masks under
  one pipeline state and apply one eye's mask to both — which looks like the current bug, not
  like a crash. Mitigation: one `static QSGMaterialType` instance per mask value, and
  `compare()` ordered by the mask, both asserted in the unit suite.
- **`UpdatesGraphicsPipelineState` omitted.** Without that flag on the shader,
  `updateGraphicsPipelineState()` is never called, no mask is applied, and the vertex colours —
  which under this design are *unmasked* — paint at full strength into every channel. That is
  worse than today's output. Mitigation: the flag is set in the shader's constructor next to the
  shader file names, and the unit suite asserts the material reports it.
- **Silent loss of the fallback.** If the probe is wrong-way-round, a working install could take
  the fallback path and nobody would notice except that stereo looks dim. Mitigation: the probe
  is a single function with a unit test on both branches (AC6).
- **Mono regression.** The accumulator split is the riskiest part of the diff for R5, because it
  touches the code mono runs. Mitigation: mono uses slot 0 only, with the same builder, the same
  material and the same draw order, and AC4 is a direct side-by-side observation.
- **Clipping and batching.** A clip node prevents the item's geometry from batching with its
  siblings' — irrelevant for one widget, noted so it is not rediscovered as a regression.
- **Translation-unit ceiling.** `Plot3D.cpp` is at exactly 1500 lines. The per-eye node sync is
  roughly line-neutral (a loop replacing three calls) and all genuinely new code lands in the new
  translation units, but `code-verify.py` must be clean and `--tu-census --check` at or below
  baseline before handoff. If it grows, `scripts/tu-cutter.py` cuts rather than the limit being
  raised.

## Test & verification plan

- **Unit (C++ `ctest` tier, maintainer builds, then either of us can run against an existing
  build dir):** `app/tests/tst_plot3d_stereo.cpp`, linking `Plot3DStereo.cpp` + `SSAssert.cpp`
  against `Qt6::Core Qt6::Gui` with `BUILD_COMMERCIAL` defined.
  - **AC1** — over every shipped theme (`default`, `fluent-dark`, `fluent-light`), feed each
    dataset colour, each axis colour and both grid colours through the policy and assert: the
    owned channels carry the expected value, the mask reports the unowned channels as not
    written, and the contrast against that theme's background clears a stated minimum for both
    eyes. Named fixtures for the two historical failures — `#e04b5a` on `#eff0f1` and `#1B8EE1`
    on `#2D2D2D` — assert both are now visible through the red lens (R3), and a case asserting
    the cyan eye still carries the source's own green and blue (R4).
  - **AC6** — assert the fallback tint still produces distinct, non-empty colours for both eyes
    when channel isolation is reported unavailable, and that the probe is what selects it (R7).
  - Also asserts the material type / `compare()` distinctness described under Risks, if that can
    be reached without linking Qt Quick; otherwise that assertion moves to the in-app
    `--selftest` tier.
- **Integration (`pytest`):** none. Nothing here is reachable over the API server; there is no
  meaningful integration surface for a render change.
- **Hotpath:** not applicable — see the hotpath section. `--benchmark-hotpath` is not a gate for
  this work.
- **Maintainer observation in the running app** — AC2 (sweep eye separation, neither ghost
  fades), AC3 (default light theme, grid ghosts read pale cyan and pale red with no brown cast),
  AC4 (stereo off, indistinguishable from the current build on a light and a dark theme), AC5
  (orbit and zoom until the trace leaves the widget, embedded and popped-out, nothing paints
  over neighbours, toolbar or titlebar).
- **Static:** `python scripts/code-verify.py --check` on every touched file;
  `python scripts/code-verify.py --tu-census --check` at or below baseline; `qt-cpp-review`
  before handoff; `python scripts/sanitize-commit.py` before commit.

## Implementation status (2026-08-27)

Built in one pass at the maintainer's instruction; **`/ss-tasks` was skipped deliberately**, so
this spec directory carries a spec and a plan and no task checklist. Do not read its absence as a
lost artifact.

Landed and statically verified: every file in the lane above, `code-verify.py --check` clean
across the tree, TU / singleton censuses at baseline (`Plot3D.cpp` went 1500 -> 1522 during the
work and came back to 1363 once the node concern moved to `Plot3DNodes.cpp`; the cutter refused
the file because it cannot parse the file-scope constant block, so the move was done by hand),
`claim-verify` clean, shaders compiled and reflected with `qsb`.

Two defects were caught in self-review rather than by a gate, and are worth remembering because
neither is the kind a linter finds:

- A node's material is chosen once, at creation. Toggling stereo off therefore left slot 0
  rendering the mono picture through the left eye's color-write mask - a red-only plot, and a
  direct R5 violation. `releaseStrokeNodes()` drops the stroke nodes whenever the effective mode
  flips, which is safe precisely because `updatePaintNode` has already detached them from the
  root.
- An `SS_ASSERT(..., continue)` slipped into the trace loop. CLAUDE.md forbids `continue` as a
  recovery action; the guarded form is `SS_ASSERT_LOG(cond); if (!(cond)) continue;`.

Still open, and all of it needs a build: AC1 and AC6 are written but unrun, and AC2 through AC5
plus the runtime half of AC7 are maintainer observations. Nothing has been committed.

## Resolved spec open questions

- **Sequencing of the interim tint.** Land it first, as its own commit, before this work starts.
  Three reasons: the brown grid is a live user-visible defect and there is no reason to make
  users wait for the full fix; R7 makes that code a permanent fallback path rather than
  throwaway, so it earns its own reviewable commit; and it keeps this spec's commit to one
  concern. The unrelated one-line `DashboardLayout.qml` top-margin fix currently sharing the
  working tree is a separate commit either way.
- **Overlays and channel discipline.** No change needed, and this is not a divergence from
  pre-0071. The old renderer drew the grid-interval label and the camera indicator into *both*
  eye layers, and merging two identical opaque images returns the original — so those overlays
  were opaque and unmasked before the port exactly as they are after it. They stay outside the
  channel discipline deliberately: they are UI chrome in fixed regions, not part of the stereo
  scene. Recorded so review does not read it as an oversight.
