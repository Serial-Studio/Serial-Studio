/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
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

//
// Shared value-formatting helpers for the instrument widgets (Bar, Gauge, Meter).
// Deliberately not a `.pragma library` so Cpp_UI_Dashboard stays reachable through
// the importing component's context.
//

//
// Formats a value through the one C++ formatter (DataModel::ValueFormat::formatNumber, spec
// 0108): decimalPoints >= 0 wins, then the Label Format ("auto", "0d".."3d", "sci",
// Hexadecimal, Binary, printf-style "%.Nf" / "%.Ne").
//
function formatValue(model, val) {
  if (!isFinite(val))
    return "—"

  return Cpp_UI_Dashboard.formatNumber(model.uniqueId, val)
}

//
// Tick labels drop redundant trailing zeros (3.00 -> 3, 3.50 -> 3.5); exponential
// notation is left untouched.
//
function formatTickValue(model, val) {
  const text = formatValue(model, val)
  if (text.indexOf(".") < 0 || text.indexOf("e") >= 0 || text.indexOf("E") >= 0)
    return text

  return text.replace(/\.?0+$/, "")
}

//
// Padding reference cache: the widths of the formatted range ends depend on the range alone,
// so they are resolved once per range instead of twice per sample. Not a `.pragma library`,
// so each importing component owns its own table; the cap keeps a range animated by a script
// from growing it without bound.
//
var _referenceWidths = ({})
var _referenceCount = 0

//
// Width of the longer formatted range end, memoized per (min, max) pair.
//
function referenceWidth(min, max) {
  const key = min + "/" + max
  var width = _referenceWidths[key]
  if (width === undefined) {
    if (_referenceCount >= 64) {
      _referenceWidths = ({})
      _referenceCount = 0
    }

    const a = Cpp_UI_Dashboard.formatValue(min, min, max)
    const b = Cpp_UI_Dashboard.formatValue(max, min, max)
    width = Math.max(a.length, b.length)
    _referenceWidths[key] = width
    _referenceCount += 1
  }

  return width
}

//
// Left-padded for stable digital-box width. The model's displayText (a value label or the
// Hexadecimal / Binary notation) replaces the number outright. A dataset decimalPoints >= 0
// forces fixed places; otherwise precision is range-driven. One formatValue round-trip per
// sample: the two range ends come from the memo above.
//
function getPaddedText(model, val) {
  if (model.displayText.length > 0)
    return model.displayText

  if (model.decimalPoints >= 0)
    return getPaddedFormattedText(model, val)

  const v = Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
  const refLen = Math.max(referenceWidth(model.minValue, model.maxValue), v.length)
  const pad = " ".repeat(Math.max(0, refLen - v.length))
  const units = model.units.length > 0 ? " " + model.units : ""
  return pad + v + units
}

//
// Padded text honoring the dataset displayFormat; the model's displayText replaces it.
//
function getPaddedFormattedText(model, val) {
  if (model.displayText.length > 0)
    return model.displayText

  const a = formatValue(model, model.minValue)
  const b = formatValue(model, model.maxValue)
  const v = formatValue(model, val)
  const refLen = Math.max(a.length, b.length, v.length)
  const pad = " ".repeat(Math.max(0, refLen - v.length))
  const units = model.units.length > 0 ? " " + model.units : ""
  return pad + v + units
}
