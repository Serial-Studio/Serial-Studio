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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

//
// Shared value-formatting helpers for the instrument widgets (Bar, Gauge, Meter).
// Deliberately not a `.pragma library` so Cpp_UI_Dashboard stays reachable through
// the importing component's context.
//

//
// Formats a value honoring the dataset displayFormat ("auto", "0d".."3d", "sci",
// printf-style "%.Nf" / "%.Ne"); anything else falls back to the dashboard's
// range-driven formatter.
//
function formatValue(model, val) {
  const fmt = model.displayFormat
  if (!fmt || fmt === "" || fmt === "auto")
    return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)

  if (fmt === "0d") return Math.round(val).toString()
  if (fmt === "1d") return val.toFixed(1)
  if (fmt === "2d") return val.toFixed(2)
  if (fmt === "3d") return val.toFixed(3)
  if (fmt === "sci") return val.toExponential(2)
  const fm = fmt.match(/^%[\d\.]*\.(\d+)f$/)
  if (fm) return val.toFixed(parseInt(fm[1]))
  const fe = fmt.match(/^%[\d\.]*\.(\d+)e$/)
  if (fe) return val.toExponential(parseInt(fe[1]))
  return Cpp_UI_Dashboard.formatValue(val, model.minValue, model.maxValue)
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
// Range-driven precision, left-padded for stable digital-box width (matches VisualRange).
//
function getPaddedText(model, val) {
  const a = Cpp_UI_Dashboard.formatValue(model.minValue, model.minValue, model.maxValue)
  const b = Cpp_UI_Dashboard.formatValue(model.maxValue, model.minValue, model.maxValue)
  const v = Cpp_UI_Dashboard.formatValue(val,            model.minValue, model.maxValue)
  const refLen = Math.max(a.length, b.length, v.length)
  const pad = " ".repeat(Math.max(0, refLen - v.length))
  const units = model.units.length > 0 ? " " + model.units : ""
  return pad + v + units
}

//
// Padded text honoring the dataset displayFormat.
//
function getPaddedFormattedText(model, val) {
  const a = formatValue(model, model.minValue)
  const b = formatValue(model, model.maxValue)
  const v = formatValue(model, val)
  const refLen = Math.max(a.length, b.length, v.length)
  const pad = " ".repeat(Math.max(0, refLen - v.length))
  const units = model.units.length > 0 ? " " + model.units : ""
  return pad + v + units
}
