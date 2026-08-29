/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

import QtQuick

//
// Tick fitting and label/readout formatting for PlotWidget. Non-visual: it owns the font
// metrics the fit measures with and reads every axis mode from the plot it serves.
//
Item {
  id: root

  //
  // The PlotWidget served by this formatter; axis modes, the ruler zero, the settled
  // plot-area extent and the cursor pair are all read from it.
  //
  required property Item plot

  //
  // Measures the labels the axes would actually draw. advanceWidth() is a pure call, so the
  // tick-interval bindings capture no mutable dependency and cannot loop.
  //
  FontMetrics {
    id: _tickMetrics

    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.83))
  }

  FontMetrics {
    id: _readoutMetrics

    font: (Cpp_Misc_CommonFonts.widgetFontRevision, Cpp_Misc_CommonFonts.widgetFont(0.85, false))
  }

  //
  // Tick fitting: 1-2-5 mantissas per decade, a fixed gutter between neighbouring labels, and
  // a hard bound on the convergence walk (40 steps span 13 decades)
  //
  readonly property var tickMantissas: [1, 2, 5]
  readonly property int tickGutterPx: 12
  readonly property int tickGutterPy: 8
  readonly property int maxTickSteps: 40

  //
  // Floor on the major-tick pitch: labels alone fit far tighter than the eye reads, and each
  // major carries four minor grid lines, so a label-only fit produces a mesh
  //
  readonly property int minTickPitchPx: 76
  readonly property int minTickPitchPy: 44

  //
  // Log10-axis tick interval: whole decades when >= 2 decades are visible, otherwise a
  // {1,2,5}*10^n decade fraction (capped at 0.5; every step divides a decade evenly)
  //
  function logInterval(range, maxLabels) {
    if (range >= 2.0)
      return Math.max(1.0, Math.ceil(range / maxLabels))

    const rough = Math.max(range / maxLabels, 1e-9)
    const magnitude = Math.pow(10, Math.floor(Math.log10(rough)))
    const normalized = rough / magnitude

    let niceInterval
    if (normalized <= 1.0)
      niceInterval = 1.0
    else if (normalized <= 2.0)
      niceInterval = 2.0
    else if (normalized <= 5.0)
      niceInterval = 5.0
    else
      niceInterval = 10.0

    return Math.min(0.5, niceInterval * magnitude)
  }

  //
  // Convergent tick period: walk the 1-2-5 sequence up from the finest plausible period until
  // the widest drawn label plus the gutter fits one tick pitch (extentOf = px along the axis)
  //
  function fitInterval(min, max, pixels, gutter, minPitch, extentOf) {
    const range = max - min
    if (!(range > 0) || !(pixels > 0))
      return 1.0

    const finest = range / pixels
    const magnitude0 = Math.floor(Math.log10(finest))
    let candidate = Math.pow(10, magnitude0)
    for (let step = 0; step < root.maxTickSteps; ++step) {
      const mantissa = root.tickMantissas[step % 3]
      candidate = mantissa * Math.pow(10, magnitude0 + Math.floor(step / 3))
      const pitch = pixels * candidate / range
      const first = Math.ceil(min / candidate) * candidate
      const last = Math.floor(max / candidate) * candidate
      const extent = Math.max(extentOf(first, candidate), extentOf(last, candidate))
      if (extent + gutter <= pitch && pitch >= minPitch)
        return candidate
    }

    return candidate
  }

  //
  // Labels exactly as the axis delegates draw them, so the fit measures the real thing
  //
  function xTickLabel(value, interval) {
    if (root.plot.timeAxis)
      return root.secondsAgoFormat(value, interval)

    if (root.plot.logX)
      return root.logTickFormat(value, interval)

    return root.engineeringFormat(root.plot.relativeX(value), interval)
  }

  function yTickLabel(value, interval) {
    if (root.plot.logY)
      return root.logTickFormat(value, interval)

    return root.engineeringFormat(value, interval)
  }

  //
  // Tick period for the X axis: log axes keep the decade rule, linear axes converge on the
  // measured label width
  //
  function smartIntervalX(min, max) {
    const range = max - min
    if (range <= 0)
      return 1.0

    const availableWidth = root.plot.settledPlotW
    if (root.plot.logX) {
      const labelWidth = _tickMetrics.advanceWidth("-8888.88") + 20
      return logInterval(range, Math.max(2, Math.floor(availableWidth / labelWidth)))
    }

    return fitInterval(min, max, availableWidth, root.tickGutterPx, root.minTickPitchPx,
                       function(value, period) {
      return _tickMetrics.advanceWidth(root.xTickLabel(value, period))
    })
  }

  //
  // Tick period for the Y axis: labels stack, so the extent is the font height
  //
  function smartIntervalY(min, max) {
    const range = max - min
    if (range <= 0)
      return 1.0

    const availableHeight = root.plot.settledPlotH
    if (root.plot.logY) {
      const labelHeight = _tickMetrics.height + root.tickGutterPy
      return logInterval(range, Math.max(2, Math.floor(availableHeight / labelHeight)))
    }

    return fitInterval(min, max, availableHeight, root.tickGutterPy, root.minTickPitchPy,
                       function(value, period) {
      return _tickMetrics.height
    })
  }

  //
  // QtGraphs normalizes the tick anchor with an unbounded add/subtract loop, so a non-finite
  // or far-out-of-range anchor freezes the app inside updatePolish: clamp into [min, max]
  //
  function safeTickAnchor(desired, min, max) {
    const lo = Math.min(min, max)
    const hi = Math.max(min, max)
    const anchor = isFinite(desired) ? desired : (isFinite(lo) ? lo : 0)
    if (!isFinite(lo) || !isFinite(hi))
      return isFinite(anchor) ? anchor : 0

    return Math.max(lo, Math.min(hi, anchor))
  }

  //
  // Mantissa of a tick period (1, 2 or 5), tolerant of floating-point scaling
  //
  function tickMantissa(interval) {
    if (!(interval > 0))
      return 1

    return Math.round(interval / Math.pow(10, Math.floor(Math.log10(interval))))
  }

  //
  // Minor ticks between two majors: four subdivisions under a 2, five otherwise; log axes keep
  // the single midpoint sub-tick
  //
  function subTicksFor(interval, logAxis) {
    if (logAxis)
      return 1

    return tickMantissa(interval) === 2 ? 3 : 4
  }

  //
  // Readout precision follows the tick period (one digit finer than the ticks) so cursors and
  // ticks can never disagree about how many digits matter
  //
  function precisionForInterval(interval) {
    if (!(interval > 0) || !isFinite(interval))
      return 2

    return Math.max(0, Math.min(6, Math.ceil(-Math.log10(interval) - 1e-9) + 1))
  }

  //
  // Decimals QtGraphs keeps in the tick text the delegates re-parse: its auto precision (0
  // decimals over a 0..10 range) collapses 0.5 to "1" and the axis repeats labels
  //
  function tickDecimals(interval) {
    if (!(interval > 0) || !isFinite(interval))
      return 6

    return Math.max(0, Math.min(12, Math.ceil(-Math.log10(interval) + 1e-9) + 1))
  }

  //
  // Engineering-unit formatter for tick labels (K/M/G, u/n/p)
  //
  function engineeringFormat(value, tickInterval) {
    if (!isFinite(value))
      return ""

    const abs    = Math.abs(value)
    const refMag = (tickInterval > 0) ? Math.max(abs, tickInterval) : abs

    // Pick the engineering scale from refMag, ignore millis range
    // code-verify off
    let scaleFactor = 1
    let suffix      = ""
    if      (refMag >= 1e9)  { scaleFactor = 1e9;   suffix = "G" }
    else if (refMag >= 1e6)  { scaleFactor = 1e6;   suffix = "M" }
    else if (refMag >= 1e3)  { scaleFactor = 1e3;   suffix = "K" }
    else if (refMag >= 1e-3) { scaleFactor = 1;     suffix = ""  }
    else if (refMag >= 1e-6) { scaleFactor = 1e-6;  suffix = "µ" }
    else if (refMag >= 1e-9) { scaleFactor = 1e-9;  suffix = "n" }
    else if (refMag > 0)     { scaleFactor = 1e-12; suffix = "p" }
    else                     { return "0" }
    // code-verify on

    // Decimals derive from the scaled tick interval
    let decimals
    if (tickInterval > 0) {
      const scaledInterval = tickInterval / scaleFactor
      decimals = scaledInterval >= 1
                  ? 0
                  : Math.min(6, Math.ceil(-Math.log10(scaledInterval) + 1e-9))
    } else if (abs === 0) {
      decimals = 0
    } else {
      const scaled = abs / scaleFactor
      decimals     = scaled >= 1
                      ? 0
                      : Math.min(6, Math.max(0,
                          2 - Math.floor(Math.log10(scaled))))
    }

    return (value / scaleFactor).toFixed(decimals) + suffix
  }

  //
  // Log-axis tick label formatter: pow10 back to true units, precision derived from the
  // local tick spacing so sub-decade zoom steps stay distinguishable (20.0K vs 20.5K)
  //
  function logTickFormat(logValue, logStep) {
    if (!isFinite(logValue))
      return ""

    const value = Math.pow(10, logValue)
    const spacing = logStep > 0 ? value * (Math.pow(10, logStep) - 1) : 0
    return engineeringFormat(value, spacing)
  }

  //
  // Tick formatter for the relative-time X axis: shows the magnitude (time ago) in the
  // chosen unit, so the axis reads e.g. 10 8 6 4 2 0 with 0 = now on the right.
  //
  function secondsAgoFormat(value, tickInterval) {
    if (!isFinite(value))
      return ""

    const relative = root.plot.relativeX(value)
    const scaled   = (root.plot.xZeroSet ? relative : Math.abs(value)) * root.plot.timeUnitFactor
    const scaledIv = tickInterval * root.plot.timeUnitFactor
    let decimals   = 0
    if (scaledIv > 0 && scaledIv < 1)
      decimals = Math.min(3, Math.ceil(-Math.log10(scaledIv) + 1e-9))

    return scaled.toFixed(decimals)
  }

  //
  // Cursor/readout formatter: absolute time-ago magnitude in the chosen unit (e.g. "12 ms").
  //
  function timeAgoLabel(worldX) {
    const scaled = Math.abs(worldX) * root.plot.timeUnitFactor
    return scaled.toFixed(scaled >= 100 ? 0 : (scaled >= 1 ? 1 : 3)) + " " + root.plot.timeUnitName
  }

  //
  // Readout formatters: world coordinates are log10 units on a log axis, so every
  // human-facing value converts back through pow10 before display
  //
  function displayValueX(worldX) {
    if (root.plot.timeAxis)
      return root.plot.xZeroSet ? root.relativeTimeLabel(worldX) : root.timeAgoLabel(worldX)

    if (root.plot.logX)
      return root.engineeringFormat(Math.pow(10, worldX), 0)

    return root.plot.relativeX(worldX).toFixed(root.plot.xPrecision)
  }

  //
  // Signed time relative to the ruler zero, in the axis unit (e.g. "-12.5 ms")
  //
  function relativeTimeLabel(worldX) {
    const scaled = root.plot.relativeX(worldX) * root.plot.timeUnitFactor
    const magnitude = Math.abs(scaled)
    return scaled.toFixed(magnitude >= 100 ? 0 : (magnitude >= 1 ? 1 : 3))
           + " " + root.plot.timeUnitName
  }

  function displayValueY(worldY) {
    if (root.plot.logY)
      return root.engineeringFormat(Math.pow(10, worldY), 0)

    return worldY.toFixed(root.plot.yPrecision)
  }

  function displayDeltaX() {
    if (root.plot.timeAxis)
      return root.timeAgoLabel(root.plot.deltaX)

    if (root.plot.logX)
      return root.engineeringFormat(Math.pow(10, root.plot.cursorBX)
                                    - Math.pow(10, root.plot.cursorAX), 0)

    return root.plot.deltaX.toFixed(root.plot.xPrecision)
  }

  function displayDeltaY() {
    if (root.plot.logY)
      return root.engineeringFormat(Math.pow(10, root.plot.cursorBY)
                                    - Math.pow(10, root.plot.cursorAY), 0)

    return root.plot.deltaY.toFixed(root.plot.yPrecision)
  }

  //
  // SI-prefixed frequency (mHz .. GHz) at @p digits significant digits, e.g. "1.25 kHz"
  //
  function frequencyLabel(hz, digits) {
    if (!isFinite(hz) || !(hz > 0))
      return ""

    const scales = [[1e9, "GHz"], [1e6, "MHz"], [1e3, "kHz"], [1, "Hz"], [1e-3, "mHz"]]
    let factor = 1e-3
    let unit = "mHz"
    for (let i = 0; i < scales.length; ++i) {
      if (hz >= scales[i][0]) {
        factor = scales[i][0]
        unit = scales[i][1]
        break
      }
    }

    const scaled = hz / factor
    const decimals = Math.max(0, digits - 1 - Math.floor(Math.log10(scaled)))
    return scaled.toFixed(Math.min(6, decimals)) + " " + unit
  }

  //
  // 1/dX for the cursor pair: only meaningful on a time axis with a non-zero separation
  //
  readonly property bool cursorFrequencyValid: root.plot.timeAxis && root.plot.cursorAVisible
                                               && root.plot.cursorBVisible
                                               && Math.abs(root.plot.deltaX) > 1e-12
                                               && isFinite(root.plot.deltaX)
  readonly property real cursorFrequencyHz: cursorFrequencyValid
                                            ? 1.0 / Math.abs(root.plot.deltaX) : 0

  //
  // Cursor readout that fits @p widthPx: the hint goes first, then frequency precision
  // degrades, and the frequency drops last; units are never dropped
  //
  function cursorReadout(widthPx) {
    const dx = root.displayDeltaX()
    const dy = root.displayDeltaY()
    const hint = qsTr("Drag to move, right-click to clear")
    const base = qsTr("ΔX: %1  ΔY: %2").arg(dx).arg(dy)
    if (!root.cursorFrequencyValid)
      return qsTr("%1 — %2").arg(base).arg(hint)

    const hz = root.cursorFrequencyHz
    const withHz = function(digits) {
      return qsTr("%1  1/ΔX: %2").arg(base).arg(root.frequencyLabel(hz, digits))
    }
    const candidates = [
      qsTr("%1 — %2").arg(withHz(4)).arg(hint),
      withHz(4),
      withHz(3),
      withHz(2),
      base
    ]

    for (let i = 0; i < candidates.length; ++i)
      if (_readoutMetrics.advanceWidth(candidates[i]) <= widthPx)
        return candidates[i]

    return candidates[candidates.length - 1]
  }
}
