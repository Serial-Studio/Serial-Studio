---
spec: 0075-review-remediation
package: WP-K (rendering and thread-priority cost, R15 / N1-N4)
tasks: WPK-T1 .. WPK-T5
status: complete (all five ticked; two census baselines need re-seeding, see "Patches")
---

# Handoff — WP-K

Worked on the merged integration tree (`wt-int`, WP0 + WP-A..I). All five tasks are done and
ticked in `tasks.md`.

## Gate state at handoff

| Gate | Result |
|------|--------|
| `code-verify.py --check` (whole tree) | 0 errors, 0 new advisories; **no advisory in any file this package touched** |
| `--tu-census --check` | 2775 excess (baseline 2775), worst 2983 — **flat** |
| `--singleton-census --check` | **grew 1550 -> 1551** (`ModuleManager.cpp` 153 -> 154) — see Patches |
| `--dup-census --check` | **grew** on two QML pairs (Bar\|Meter 132 -> 133, Bar\|Gauge 83 -> 84) — see Patches |
| `claim-verify.py --quiet` | 9 errors, **0 new** (the startup.md ordered anchor WP-J owns) |
| `registry-verify.py` | CLEAN; `generate-property-registry --check` up to date |
| `pytest tests/scripts scripts/tests -q` | 529 passed |
| `pytest tests --collect-only -q` | 1764 collected (unchanged) |

I did not build, run ctest, or run `--benchmark-hotpath` (maintainer's).

## Binding invariants named before the edits

- **T1/T2** — render-thread work only inside `updatePaintNode`; the GUI thread writes only into
  `m_image`, and the copy into texture-owned staging happens at sync while the GUI is blocked; a
  `QSGTexture` is created and destroyed only on the render thread (primary node owns it, the alias
  never does); `m_topRow`/`m_writeRow`/`m_filledOnce` stay one ring state and the scroll is a
  source-rect offset, never a pixel move; no per-tick allocation; tiles stay the untouched fallback.
- **T3** — the registration is a one-shot startup command posted `Qt::QueuedConnection`, never a
  per-frame hop and never a blocking GUI->pipeline wait; the pipeline's in-thread
  `DirectConnection`/SPSC hops are untouched; registration only through
  `AppPlatform::registerIngestThreadWithMmcss()` and only after `qInstallMessageHandler`; each
  thread registers from inside itself because a `QThread` never inherits the band.
- **T4** — `hasData` latches on the first real sample and clears only on `resetData`; severity is
  -1 and `alarmTriggered` false until then; `Dataset::value` string propagation and the push tables
  are untouched; setters keep their guard-return and reuse the existing `updated()` notify.
- **T5** — geometry buffers never shrink below the retained capacity; `QSGGeometry` has one pair of
  counts and no capacity/size split, so reuse means holding the counts still and padding; padding
  is degenerate (index 0 / repeated last vertex) so the index count still bounds what is drawn and
  the batch bounds stay real; the emit passes keep asserting lockstep against the *used* counts.

## Files changed

### New

| File | Role |
|------|------|
| `app/src/UI/Widgets/Waterfall/WaterfallRingTexture.{h,cpp}` | One persistent `QSGTexture` over an owned `QRhiTexture`, updated by staging changed scanlines at sync and uploading them as sub-rects in `commitTextureOperations`. Also carries the idle gate (`captureRowIfChanged`) and the device-capability probe (`supported`). |
| `app/src/UI/Widgets/Waterfall/WaterfallSpectrogramNodes.{h,cpp}` | **Not in the task's file list** — see "Scope deviations". The spectrogram's scene-graph half: dirty-row/dirty-band bookkeeping plus both draw paths (ring, and WP-E's 64-row tiles as the fallback), lifted out of `Waterfall.cpp`. |
| `app/tests/tst_waterfall_ring_texture.cpp` | Staging slots and destinations, slot overflow escalating to a full upload, full upload superseding staged rows, out-of-range/foreign-image rejection, the idle gate, and the single-band seam decomposition (the ring's UV math). |
| `app/tests/tst_mmcss_registration.cpp` | The per-thread registration latch: a worker that registers leaves every other thread unregistered, and repeat calls on one thread are no-ops. |
| `app/tests/tst_value_widget_hasdata.cpp` | The nearest-band clamp still resolves 0.0 to a band (load-bearing for overrange data) and `Bands::reportedSeverity` overrides it while the widget has no data. |
| `app/tests/tst_plot_curve_geometry.cpp` | Counts reallocations through `reserveGeometry`'s boolean over 100 frames of a stationary and a wobbling count; capacity never shrinks; index capacity is whole triangles; the padded tail is degenerate. |

### Modified

| File | Change |
|------|--------|
| `app/src/UI/Widgets/Waterfall.{h,cpp}` | Node bookkeeping delegated to `m_spectrogram`; idle gate in `updateData` (skip the row write and the frame when the spectrum is bit-identical, Campbell exempt); `releaseHistoryImage()` + `itemChange(ItemVisibleHasChanged)` release the image and request a GPU teardown when hidden, and rebuild lazily on the next block; `releaseRenderResources()` replaces the two inline pointer-nulling blocks. 1487 -> 1392 lines. |
| `app/CMakeLists.txt` | Two new source pairs; **`Qt6::GuiPrivate` added to `QT_LIBS`** (see "Invariants the plan did not state"). |
| `app/src/Platform/AppPlatform.{h,cpp}` | The registration latch is `thread_local`, hoisted into a new `// Constants` banner; `mmcssRegisteredOnCurrentThread()` exposes it. |
| `app/src/IO/PipelineHost.{h,cpp}` | `registerIngestThread()`: one queued post of the registration onto the pipeline thread, skipped while the pipeline still runs on the GUI thread. |
| `app/src/IO/StreamWorker.cpp` | Each worker posts the registration onto its own event loop, next to the existing `compileEngines` post. |
| `app/src/Misc/ModuleManager.cpp` | The GUI-thread `registerIngestThreadWithMmcss()` call becomes `IO::PipelineHost::instance().registerIngestThread()`, still immediately after `qInstallMessageHandler`; the now-unused `Platform/AppPlatform.h` include dropped. |
| `app/src/UI/Widgets/Bar.{h,cpp}` | `hasData` Q_PROPERTY + `latchData()` + `resetData()` (wired to `Dashboard::dataReset`); `activeBandSeverity()` routes through `Bands::reportedSeverity`; `activeBandLabel()` empty while unlatched. |
| `app/src/UI/Widgets/Gauge.cpp`, `Meter.cpp` | The same latch call in their `updateData` overrides. |
| `app/src/UI/WidgetBands.h` | **Not in the task's file list** — `Bands::reportedSeverity()`, the pure gate the three widgets now share and the only part of the fix the unit tier can reach. |
| `app/qml/Widgets/Dashboard/{Bar,Gauge,Meter}.qml` | Both infinite blink animations and the digital-box `targetColor` per file gated on `alarmTriggered && hasData`. |
| `app/src/UI/Widgets/GpuStroke.{h,cpp}` | `reserveGeometry()` / `padGeometryTail()` + the headroom constants; both stroke/point node builders use them. |
| `app/src/UI/Widgets/PlotCurve.cpp`, `PlotAreaFill.cpp` | Same two calls replacing the exact-fit `allocate()`. |
| `app/tests/CMakeLists.txt` | Four new suites, each next to its nearest relative. |

## The exact QRhi calls I relied on, and the headers they came from

Read from the installed Qt 6.11.2 (`.../QtGui.framework/Versions/A/Headers/6.11.2/QtGui/rhi/qrhi.h`
and `.../QtQuick.framework/Versions/A/Headers/qsgtexture.h`), not from documentation:

| Call | Header, and what the header actually says |
|------|-------------------------------------------|
| `QSGTexture::commitTextureOperations(QRhi*, QRhiResourceUpdateBatch*)` | `qsgtexture.h:60`, `virtual`. This is the hook that hands a `QSGTexture` subclass the frame's resource-update batch during the render pass's prepare step. It is why no batch has to be obtained by hand. |
| `QSGTexture::rhiTexture()` | `qsgtexture.h:49`, `virtual QRhiTexture *rhiTexture() const` — const, no argument, returns the raw pointer. |
| `QQuickWindow::rhi()` | `qquickwindow.h:167`, `QRhi *rhi() const`. Null under the software renderer, which is what selects the tile fallback. |
| `QRhi::newTexture(Format, const QSize&, int, Flags)` | `qrhi.h:2012`. Followed by `QRhiTexture::create()` (`qrhi.h`, pure virtual), which is where a failure is detected. |
| `QRhi::isTextureFormatSupported(Format, Flags)` | `qrhi.h:2067` — the BGRA8 probe. |
| `QRhi::resourceLimit(QRhi::TextureSizeMax)` | `qrhi.h:2069` + the `ResourceLimit` enum at `qrhi.h:1952`. |
| `QRhiResourceUpdateBatch::uploadTexture(QRhiTexture*, const QRhiTextureUploadDescription&)` | `qrhi.h:1796`. |
| `QRhiTextureSubresourceUploadDescription::{setData,setDataStride,setSourceSize,setDestinationTopLeft}` | `qrhi.h:660-693`. **`setSourceTopLeft` is deliberately unused**: the member exists (`qrhi.h:684`) but the raw-data path has no documented source-offset semantics, so each staged row gets its own persistent `QByteArray` and `setData()` shares it by refcount instead. |
| `QRhiTextureUploadEntry(int layer, int level, const QRhiTextureSubresourceUploadDescription&)` | `qrhi.h:701`. |

**Endianness/format:** `QImage::Format_RGB32` is `0xffRRGGBB` in a `quint32`, whose little-endian
byte order is exactly `BGRA8`. `WaterfallRingTexture::supported()` therefore returns false at
compile time on a big-endian target, and the tile fallback runs. No per-tick format conversion
exists on either path.

## Not done, and why

1. **`--benchmark-hotpath` was not run.** Nothing in this package sits on the parse path; the only
   hotpath-adjacent edit is one queued post at startup. The gate is yours.
2. **The hidden-release path (T2) has no ctest.** It needs a live `QQuickItem` in a window;
   `tst_waterfall_ring_texture` covers everything below the item. The overlay-cadence half of T2
   was already satisfied by WP-E's F7 fix (`markAxisDirty()` at the end of `updateData` is gone);
   I only added the "row unchanged but overlay changed" branch, which still goes through it.
3. **`tst_value_widget_hasdata` does not exercise `Widgets::Bar`.** `Bar`'s ctor captures
   `UI::Dashboard::instance()`, so the unit tier cannot link it — the same wall WP-E hit with
   `ExtensionRowsModel`. The gate itself is real production code (`Bands::reportedSeverity`, called
   by `Bar::activeBandSeverity`), which is why I moved it into `WidgetBands.h` rather than testing a
   copy of it.

## Scope deviations (both deliberate, both named)

1. **`Waterfall/WaterfallSpectrogramNodes.{h,cpp}` is a second new file pair** the task did not
   name. It is not optional: WPK-T1+T2 added ~210 lines to a `Waterfall.cpp` that was already at
   1487 of the 1500 cap, and `--tu-census --check` failed (2775 -> 2971). The repo's prescribed fix
   for that lint is exactly this shape — a real member sub-object, one class per `.h/.cpp`, in the
   sibling directory named after the facade — and it is the only change that keeps both draw paths
   intact. With it the census is flat at the baseline. The moved tile code is WP-E's, verbatim.
2. **`app/src/UI/WidgetBands.h` gained `reportedSeverity()`.** The task's file list stops at the
   three widget pairs, but the prompt's mandatory-read list names `WidgetBands.h`, the three widgets
   all already route through `Bands::activeIndex`, and this is the only placement that makes the
   fix testable without linking a `QQuickItem` to the whole application. Additive; no existing
   `Bands::` function changed.

## Patches for the coordinator

### 1. Re-seed two census baselines (both grew by design)

```
python3 scripts/code-verify.py --singleton-census --accept
python3 scripts/code-verify.py --dup-census --accept
```

- **Singleton +1**, `ModuleManager.cpp` 153 -> 154: the GUI-thread free-function call
  `Platform::AppPlatform::registerIngestThreadWithMmcss()` had to become
  `IO::PipelineHost::instance().registerIngestThread()`, because the registration must execute
  *inside* the pipeline thread and the composition root is the only place that may sequence it
  after `qInstallMessageHandler`. `ModuleManager` is the sanctioned site for an `instance()` reach.
  I did not re-baseline on my own.
- **Dup census +1 window on two QML pairs**: `Bar.qml`, `Gauge.qml` and `Meter.qml` each gained the
  same `&& model.hasData` guard on the same two animations. WP-I's `InstrumentBase.qml` covers
  Meter/Gauge/Clock/Compass but **not** Bar, so the three blink blocks remain clones; deduplicating
  them means moving Bar onto `InstrumentBase`, which is a WP-I-shaped change, not a WP-K one.

### 2. `Qt6::GuiPrivate` is now a link dependency (decision, not a patch)

`<rhi/qrhi.h>` lives under Qt's **private** include path, so `app/CMakeLists.txt` gained
`Qt6::GuiPrivate`. The plan called QRhi "public API, Qt >= 6.6"; it is semi-public — source
compatible within a Qt minor series only. A Qt minor upgrade must re-check
`WaterfallRingTexture.cpp`, and the tile fallback is what limits the blast radius if the API moves.
The CMake comment says so at the point of the change. If you would rather not take the private
dependency, the fallback path alone is still correct (and still WP-E's 9x improvement) — revert
`WaterfallRingTexture.*` and the `supported()` branch in `WaterfallSpectrogramNodes::sync`.

### 3. `AC15`'s thread-listing wording contradicts R15.2 (`spec.md`, your call)

AC15 says "only the pipeline thread elevated". R15.2 and the WP-K plan row both say the dense
**stream-worker** threads register too, and I implemented that. On the BADAQ repro (4 x 48 kHz
IEPE) that is five elevated threads, not one. Either AC15 should read "only the pipeline and
stream-worker threads", or the `StreamWorker.cpp` post should be dropped. I implemented what the
plan and tasks say and left the spec text alone.

## Behaviour deltas, deliberate and named

- **A hidden waterfall loses its history.** `itemChange(ItemVisibleHasChanged, false)` frees the
  image; becoming visible again rebuilds it at the floor color and it refills from live data. That
  is what R15.1's "a hidden widget releases its image and textures" costs, and it is visible when
  switching workspace tabs. Say the word and I will keep the image and release only the textures.
- **An unchanged spectrum stops the scroll.** The idle gate compares the smoothed row to the
  previous tick's; identical means no row is written and no frame is scheduled. For a disconnected
  or silent source that is exactly the intent ("unconnected groups sit still"). For a *perfectly*
  constant live signal the waterfall also stops scrolling — visually indistinguishable once the
  history is uniform, distinguishable during the first fill. **Campbell mode is exempt** (its rows
  are placed by another dataset's value, which moves independently of the spectrum).
- **Geometry uploads ~1.5x more bytes per frame** in exchange for zero per-frame malloc/free. N4
  measured the allocation, not the bandwidth, and the padded tail is degenerate triangles the
  rasterizer discards, but this is a real trade and the `--benchmark-hotpath` dashboard row is
  where it would show.
- **`activeBandLabel()` is empty while `!hasData`**, not just the severity. A widget with no data
  showed its clamped band's label next to "No data"; that was the same defect.
- **`Bar::resetData()` also clears the extreme-hold state** (`m_minSeen`/`m_maxSeen`/
  `m_extremesValid`). Nothing else cleared them on `dataReset`, so a reconnect used to keep the
  previous session's extremes.

## Invariants I found that the plan did not state

- **`QSGTexture::commitTextureOperations()` runs on the render thread with the GUI thread already
  released**, unlike `updatePaintNode`. That is the whole reason the staging buffers exist: the
  texture may not read the widget's live `QImage` at upload time, only memory it owns. Everything
  else in this design follows from that one fact.
- **`QSGGeometry` has no capacity/size split.** `vertexCount()`/`indexCount()` are simultaneously
  the allocated size and the drawn size, so "allocate with headroom and draw the first N" is not
  expressible — the tail has to be made degenerate instead. The plan's "draw the first N through
  the index count" is only true because the padding indices all point at vertex 0.
- **The padded vertices must be real coordinates, not left uninitialised.** The batch renderer
  computes bounds over the whole vertex buffer; garbage there corrupts batching and clipping even
  though no index references it. `padGeometryTail` repeats the last real vertex for that reason.
- **`WaterfallTiles::decompose` already handles the ring path** with `tileRows == imageHeight`: one
  band, `tileTop == 0`, so the piece source rects come out in absolute image rows and the two runs
  are the ring seam. No new decomposition code was needed, and `tst_waterfall_tiles` still pins the
  shared function.
- **`PipelineHost` itself lives on the GUI thread.** Only `FrameBuilder` and `FrameParser` are
  relocated, so `QMetaObject::invokeMethod(this, ...)` would have registered the GUI thread — the
  exact defect. The post goes through `m_frameBuilder`, guarded on
  `m_frameBuilder->thread() == m_thread.get()` so the headless and benchmark bootstraps (which
  never call `relocateProcessingObjects()`) register nothing.
- **`Bar::updateData` early-returns unless the value *changed*.** A first sample numerically equal
  to the placeholder 0.0 would therefore never have published the latch, which is why `latchData()`
  returns whether it just closed and that feeds the emit condition.

## Docs whose claims this diff invalidated (for WP-J)

- **`doc/claude/architecture/dashboard.md`, "Waterfall / Spectrogram (Pro)"** — "Class IS the
  painted item (`QQuickPaintedItem`)" is wrong twice over: `Waterfall` has been a `QQuickItem` with
  `updatePaintNode` since WP-E, and the spectrogram's node half now lives in
  `Widgets::WaterfallSpectrogramNodes` (`Waterfall/WaterfallSpectrogramNodes.{h,cpp}`). The section
  should describe: one persistent `QSGTexture` per widget over an owned `QRhiTexture`
  (`WaterfallRingTexture`), scanline sub-rect uploads staged at sync and committed in
  `commitTextureOperations`, the scroll as a source-rect offset, WP-E's 64-row tiles demoted to the
  no-QRhi fallback, the idle gate, and the hidden-widget release.
- **`doc/claude/specs/0075-review-remediation/handoff-wpe.md` item 5** asks WP-J to document "the
  history image is uploaded as 64-row texture bands". That is now the fallback only — please write
  both paths, not that sentence.
- **`doc/claude/architecture/startup.md`, "MMCSS coexistence contract"** — "register only via
  `Platform::AppPlatform::registerIngestThreadWithMmcss()`, called AFTER `qInstallMessageHandler`
  (ModuleManager)" is still true, but the missing half is now the point: the band is **per thread**,
  the guard is `thread_local`, and the registration executes inside the acquisition threads
  (`IO::PipelineHost::registerIngestThread()` posts it onto the pipeline thread; each
  `IO::StreamWorker` posts it onto its own). The GUI thread is deliberately **not** registered.
  Add the query, `AppPlatform::mmcssRegisteredOnCurrentThread()`.
- **`doc/claude/common-mistakes.md`, "CI & Platform"** — the MMCSS row still reads as if the main
  thread is the one that registers. It should say the ingest threads register themselves and that a
  process-wide guard is the bug (spec 0075 N2), pointing at `tst_mmcss_registration`.
- **`doc/claude/architecture/dashboard.md`, "Alarm Bands"** — "`Bar` / `Gauge` / `Meter` /
  `LEDPanel` are display-only band consumers" now needs the no-data rule: they report severity -1
  and no label until the first finite sample, latched per widget and cleared on `dataReset`.
  `UI::AlarmMonitor` is unchanged and still notifies dataset-level.
- **`doc/claude/architecture/dashboard.md`, "GPU Curve Rendering" / "Area-Under-Plot Fill"** — both
  describe geometry sizing; neither mentions that the buffers are now grow-only with 1.5x headroom
  and a degenerate padded tail (`GpuStroke::reserveGeometry` / `padGeometryTail`), which is the
  reason a live curve no longer reallocates per frame.
- **`tests/README.md`** — four new C++ suites: `tst_waterfall_ring_texture`,
  `tst_mmcss_registration`, `tst_value_widget_hasdata`, `tst_plot_curve_geometry`. Worth noting that
  `tst_mmcss_registration` runs on every platform (the latch is recorded everywhere; only the
  Windows API behind it is skipped), which is not what the tasks file predicted.
- **`doc/claude/code-style.md` / the build docs** — `Qt6::GuiPrivate` is now in `QT_LIBS`, and it is
  the first private-Qt dependency in the app target. Whoever documents the build should say why and
  what re-checking a Qt upgrade means.

## Counterfactual self-check

**Which rule does this diff most risk violating?** "Render-thread work only inside
`updatePaintNode`; the GUI thread writes only into CPU staging memory the render thread reads at
sync." `WaterfallRingTexture` deliberately does work *outside* `updatePaintNode` —
`commitTextureOperations` runs later, on the render thread, while the GUI thread is free and may
already be colorizing the next scanline into `m_image`.

**What evidence says it does not violate it?** The render thread never touches `m_image` after
sync. The only two entry points that read it, `stageRow()` and `stageImage()`, are called from
`WaterfallSpectrogramNodes::stageRingUploads()`, which is reachable only from
`syncRing()` <- `sync()` <- `Waterfall::updatePaintNode` — grep for `stageRow`/`stageImage` shows
exactly those two call sites. `stageRow` memcpy's into a `QByteArray` the texture owns and sized
once in its constructor; `stageImage` takes an implicitly-shared copy of the `QImage`, so the GUI
thread's next `scanLine()` detaches its own buffer and the render thread keeps reading the one that
was current at sync. `commitTextureOperations` reads only `m_stagingImage`, `m_stagedRowData`,
`m_stagedRowIndex`, `m_size` and `m_texture` — all owned by the texture, none written after sync.
The one cross-thread field, `m_failed`, is a `std::atomic<bool>` written on the render thread and
read at sync.

**Second-closest risk:** "no allocation on the per-tick path", on the geometry padding. Each frame
now memcpy's the unused vertex tail (at most a third of the buffer, ~20-30 KB for a full-width
curve) and zero-fills the unused index tail. That is a bounded copy into an allocation that already
exists, replacing a `free()` + `malloc()` of the whole buffer — the thing N4 measured as page-fault
churn. It allocates nothing. `reserveGeometry` returns false in the steady state, which
`tst_plot_curve_geometry` pins across 100 frames of both a stationary and a wobbling count.

**Third:** the MMCSS post could deadlock or fire at the wrong time. It cannot: it is a plain
`Qt::QueuedConnection` `invokeMethod` with no waiter (not `runOnObjectThread`, not
`BlockingQueuedConnection`), it is issued once at startup after `qInstallMessageHandler`, and it is
skipped entirely when `m_frameBuilder->thread() != m_thread.get()` — so the headless verifier and
`--benchmark-hotpath`, which never relocate the pipeline, post nothing and keep the benchmark's own
`registerIngestThreadWithMmcss()` call on the thread it actually drives.
