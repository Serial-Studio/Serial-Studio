# 0058 — implementation notes (overnight run 2026-08-16/17)

## Landed (working tree, uncommitted)

- **A2.1** `app/qml/Widgets/PlotWidget.qml`: `fitInterval()` walks the 1-2-5 sequence from the
  finest plausible period, formats the two grid-aligned extreme labels exactly as the axis
  delegate does (`xTickLabel` / `yTickLabel`) and measures them with `FontMetrics.advanceWidth`
  (pure call, no property write inside a binding); X gutter 12 px, Y gutter 8 px (font height);
  bounded to 40 steps. `subTickCount` follows the mantissa (3 minors under a 2, else 4); log
  axes keep the decade rule and one minor. `xPrecision`/`yPrecision` now derive from the tick
  period (`precisionForInterval`) instead of the range guess. The two `TextMetrics` template
  objects (`"-8888.88"`) are gone; the log-axis label-count estimate still uses that template
  string via `advanceWidth`.
- **A2.2** `PlotWidget.qml`: `frequencyLabel(hz, digits)` (mHz..GHz), `cursorFrequencyValid`
  (time axis, both cursors, `|dX| > 1e-12`), `cursorReadout(widthPx)` picks the longest of a
  bounded candidate list that fits `_layout.width`: full (4 sig digits + hint), no hint, 3, 2
  sig digits, then no frequency. Units are never dropped. "sample count" in the brief was read
  as the trailing hint text (there is no sample count in the readout today).
- **A2.3** new `app/src/UI/Widgets/PlotAutoScale.h` (`Widgets::AutoScale`: `ladderStep`,
  `ladderIndexFor`, `quantizeRange` with `kDivisions = 8`, `kShrinkMargin = 0.8`);
  `Plot::padDerivedRange` (now takes the per-lane `int& stepIndex`, members `m_xStepIndex` /
  `m_yStepIndex`) and `MultiPlot::padDerivedRange` (`m_yStepIndex`) quantize after the
  existing padding; the integer floor/ceil that used to follow is now the fallback for a
  refused (degenerate) range. `MultiPlot::applyDerivedYBounds` no longer rounds to integers
  before the ladder. User-set ranges (`pltMin != pltMax`) untouched. Test
  `app/tests/tst_plot_autoscale.cpp` (7 slots) registered; not built.
- **A2.4** `PlotWidget.qml`: `xMarkers` / `xZero` / `xZeroSet` / `hoverMarkerEnabled` +
  `rulerChanged()`; right-click release outside cursor mode opens `_rulerMenu` (add marker via
  `_markerNamePopup`, remove marker under the pointer, clear all, set/reset zero, hover marker
  toggle); zero line + chip, marker lines + name chips (`Repeater`), hover line + X readout;
  `relativeX()` feeds X tick labels, `displayValueX`, and the axis title ("Time from zero (s)"
  / "<label> from zero"); `tickAnchor` snaps to the zero so a tick lands on it. Deltas are
  unaffected. `Plot.qml` and `MultiPlot.qml` restore via `plot.restoreRuler(s)` and persist
  `xMarkers` (array of `{x, name}`), `xZero`, `xZeroSet`, `hoverMarker` on `rulerChanged`.
  FFTPlot also hosts `PlotWidget`, so the menu appears there too but nothing is persisted.

## Not done / caveats

- No icons or commands were added (in-widget menu only), so no registry changes.
- Markers live in axis world coordinates: on a time axis that is "seconds ago", so like the
  cursors they slide with the axis rather than pinning to a sample.
- Binding-loop review by reading only: `fitInterval` reads geometry/range/font, writes nothing;
  the delta label reads `_layout.width` (its own parent, not itself).

## For the maintainer

- Build; `ctest -R tst_plot_autoscale`; spec AC1-AC4 in the running app; watch the minor grid
  density (4-5 subdivisions per major instead of 2 is a visible change).
