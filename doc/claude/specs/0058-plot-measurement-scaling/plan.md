---
spec: 0058-plot-measurement-scaling
phase: plan
status: approved     # gate auto-approved, overnight run 2026-08-16 (unattended; maintainer re-reviews)
updated: 2026-08-16
---

# Plan 0058 — Plot measurement and scaling

> **Phase 2 of 4 — the HOW.** Gate auto-approved for the unattended overnight run of
> 2026-08-16.

## Approach (one paragraph)

Four independent edits, none on the frame path. (1) `PlotWidget.qml` gets one convergent
`fitInterval()` used by both linear axes: walk the 1-2-5 sequence upward from the finest
plausible period, format the two extreme tick labels exactly as the axis delegate would, measure
them with a `FontMetrics.advanceWidth()` call (a pure function, so the binding captures no
mutable dependency and cannot loop), and stop at the first period whose widest label plus a
gutter fits the pitch; minor ticks follow the mantissa; readout precision derives from the
period. (2) The cursor delta label appends `1/ΔX` in SI Hz on time axes, choosing the longest
of a bounded candidate list that fits the label width. (3) A new header-only
`Widgets::AutoScale` (`PlotAutoScale.h`) quantizes a data-derived `[min, max]` onto a 1-2-5
ladder with an integer step index kept in the widget for hysteresis; `Plot::padDerivedRange`
and `MultiPlot::padDerivedRange` call it, the user-set-range path is untouched. (4)
`PlotWidget.qml` grows an X-ruler: `xZero`, `markers`, `hoverMarkerEnabled`, a right-click
`Menu` (outside cursor mode) with add-marker (name prompt popup), set/reset zero, hover toggle,
remove-marker; `Plot.qml` and `MultiPlot.qml` restore/persist those through `widgetSettings`.

## Affected subsystems & files

| File | Change |
|------|--------|
| `app/qml/Widgets/PlotWidget.qml` | A2.1 `fitInterval` + `FontMetrics`, minor-tick counts, precision from interval; A2.2 frequency readout; A2.4 ruler state, menu, marker/zero/hover rendering, relative labels |
| `app/src/UI/Widgets/PlotAutoScale.h` | new: 1-2-5 ladder + hysteresis quantizer |
| `app/src/UI/Widgets/Plot.h` / `Plot.cpp` | A2.3: `m_yStepIndex`, `padDerivedRange` quantizes |
| `app/src/UI/Widgets/MultiPlot.h` / `MultiPlot.cpp` | A2.3: same |
| `app/qml/Widgets/Dashboard/Plot.qml`, `MultiPlot.qml` | A2.4: restore + persist `xMarkers`, `xZero`, `hoverMarker` |
| `app/tests/tst_plot_autoscale.cpp`, `app/tests/CMakeLists.txt` | ladder + hysteresis unit test |

## Architecture & data flow

QML-only for A2.1/A2.2/A2.4; the widget models (`Plot`, `MultiPlot`) already expose min/max
and the plot area. A2.3 changes only the data-derived branch of `calculateAutoScaleRange`
(draw cadence, GUI thread). Persistence goes through `Cpp_JSON_ProjectModel.saveWidgetSetting`
exactly as the sweep settings do (memory + `setModified(true)`, debounced autosave).

## Hotpath & threading impact

- **Touches the hotpath?** No. Draw-cadence C++ (`padDerivedRange`) and QML bindings.
- **New cross-thread signal/slot?** No.
- **New input to a cached hotpath flag?** No.
- **Timestamp ownership** — unchanged.

## Data model & persistence

`widgetSettings` keys added: `xMarkers` (JSON array of `{x, name}`), `xZero` (number or absent),
`hoverMarker` (bool). Absent keys mean defaults; no migration.

## API / SDK surface

None.

## QML / UI

Right-click `Menu` on the plot overlay outside cursor mode (cursor mode keeps its right-click
clear semantics). Marker name via a small `Popup` with a `TextField`. Markers render as 1 px
vertical lines with a name chip at the top; the hover marker is a dotted line with an X readout
chip; the zero line is drawn when `xZero` is inside the visible range. All strings `qsTr()`.

## Tradeoffs & alternatives considered

| Decision | Options | Chosen + why |
|----------|---------|--------------|
| Measuring labels | (a) `TextMetrics.text` mutation inside the binding; (b) `FontMetrics.advanceWidth(str)` | (b): pure call, no property write inside a binding, no loop hazard. |
| Which labels to measure | (a) every tick; (b) the two grid-aligned extremes | (b): widest label is at the largest magnitude; two calls per candidate keeps the bounded loop cheap. |
| Y ladder state | (a) step index in the widget; (b) recompute from previous range | (a): explicit integer, hysteresis is a comparison of indices. |
| Zero-point convention | (a) keep "time ago" magnitude with zero shifted; (b) signed relative time (`x - xZero`) | (b): "t = 0 here" is what the user asked for; the axis title switches to say so. |
| Ruler entry point | (a) new commands/icons; (b) right-click menu | (b): in-widget, no registry churn, no icons. |

## Risks & mitigations

- Binding loops: `fitInterval` reads only geometry, range and font; writes nothing.
- Right-click semantics: cursor mode keeps clearing; the menu opens only outside cursor mode.
- MultiPlot shares `PlotWidget`, so the ruler shows there too; persistence wired for both.

## Test & verification plan

- **Unit (maintainer builds):** `tst_plot_autoscale` — ladder monotonic, step >= extent /
  divisions, hysteresis (grow immediately, shrink only past the margin), zero on a boundary when
  bipolar.
- **Maintainer, running app:** spec AC1-AC4.
- **Static:** `code-verify.py --check` on touched files.
