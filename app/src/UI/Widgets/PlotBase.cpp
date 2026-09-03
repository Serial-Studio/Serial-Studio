/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include "UI/Widgets/PlotBase.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <QtGlobal>

#include "SSAssert.h"
#include "UI/Widgets/PlotLogScale.h"

/**
 * @brief Starts with linear interpolation, no sweep, and an unset visible-X window: NaN bounds
 *        are what make clampToVisibleX a no-op until the view reports a real window.
 */
Widgets::PlotBase::PlotBase()
  : m_sweepEnabled(false)
  , m_triggerLevel(0)
  , m_holdoffMs(0)
  , m_timebaseMs(0)
  , m_visLoX(std::numeric_limits<double>::quiet_NaN())
  , m_visHiX(std::numeric_limits<double>::quiet_NaN())
  , m_sweepMode(SerialStudio::SweepAuto)
  , m_triggerEdge(SerialStudio::TriggerRising)
  , m_interpolationMode(SerialStudio::InterpolationLinear)
  , m_logXScratch(1)
{}

//--------------------------------------------------------------------------------------------------
// Interpolation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Stores a validated interpolation mode; an unknown value falls back to linear rather than
 *        reaching the renderer, which indexes a jump table on it. True when the mode moved.
 */
bool Widgets::PlotBase::setInterpolationMode(SerialStudio::InterpolationMode mode) noexcept
{
  SerialStudio::InterpolationMode resolved;
  switch (mode) {
    case SerialStudio::InterpolationNone:
    case SerialStudio::InterpolationLinear:
    case SerialStudio::InterpolationZoh:
    case SerialStudio::InterpolationStem:
      resolved = mode;
      break;
    default:
      resolved = SerialStudio::InterpolationLinear;
      break;
  }

  if (m_interpolationMode == resolved)
    return false;

  m_interpolationMode = resolved;
  return true;
}

/**
 * @brief The validated interpolation mode the renderer reads.
 */
SerialStudio::InterpolationMode Widgets::PlotBase::interpolationMode() const noexcept
{
  return m_interpolationMode;
}

//--------------------------------------------------------------------------------------------------
// Visible-X window and log-X scratch
//--------------------------------------------------------------------------------------------------

/**
 * @brief Records the X window the view is actually showing; the range derivation clamps to it so
 *        an off-screen outlier cannot stretch the axis.
 */
void Widgets::PlotBase::setVisibleXWindow(const double lo, const double hi) noexcept
{
  m_visLoX = lo;
  m_visHiX = hi;
}

/**
 * @brief Clamps a derived X range into the visible window plus a two-pixel margin. An unset or
 *        degenerate window leaves the range untouched, so the first frames still auto-range.
 */
void Widgets::PlotBase::clampToVisibleX(double& lo, double& hi, const int dataW) const
{
  SS_ASSERT(lo <= hi, std::swap(lo, hi));

  if (!std::isfinite(m_visLoX) || !std::isfinite(m_visHiX) || !(m_visLoX < m_visHiX))
    return;

  const double margin = (m_visHiX - m_visLoX) * (2.0 / std::max(2, dataW));
  lo                  = std::max(lo, m_visLoX - margin);
  hi                  = std::min(hi, m_visHiX + margin);
}

/**
 * @brief Refills the log-X scratch ring from @p x, reusing its storage whenever the capacity and
 *        the sample count already match, so a steady stream costs no allocation.
 */
void Widgets::PlotBase::buildLogXScratch(const DSP::AxisData& x, const double floor)
{
  if (m_logXScratch.capacity() == x.capacity() && m_logXScratch.size() == x.size())
    return;

  if (m_logXScratch.capacity() != x.capacity())
    m_logXScratch.resize(x.capacity());

  SS_ASSERT(x.raw() != nullptr, return);
  SS_ASSERT(x.size() <= m_logXScratch.capacity(), return);

  m_logXScratch.clear();

  const auto* data       = x.raw();
  const std::size_t mask = x.storageMask();
  std::size_t idx        = x.frontIndex();
  const std::size_t n    = x.size();
  for (std::size_t i = 0; i < n; ++i) {
    m_logXScratch.push(LogScale::clampedLog10(data[idx], floor));
    idx = (idx + 1) & mask;
  }
}

/**
 * @brief The log-X ring the renderer draws from while a logarithmic X axis is on.
 */
const DSP::AxisData& Widgets::PlotBase::logXScratch() const noexcept
{
  return m_logXScratch;
}

//--------------------------------------------------------------------------------------------------
// Sweep configuration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Turns the sweep on or off; true when the flag moved.
 */
bool Widgets::PlotBase::setSweepEnabled(const bool enabled) noexcept
{
  if (m_sweepEnabled == enabled)
    return false;

  m_sweepEnabled = enabled;
  return true;
}

/**
 * @brief Sets the trigger level; true when it moved.
 */
bool Widgets::PlotBase::setTriggerLevel(const double level) noexcept
{
  if (qFuzzyCompare(m_triggerLevel, level))
    return false;

  m_triggerLevel = level;
  return true;
}

/**
 * @brief Sets the re-arm holdoff in milliseconds, clamped at zero; true when it moved.
 */
bool Widgets::PlotBase::setHoldoff(const double milliseconds) noexcept
{
  const double clamped = milliseconds < 0 ? 0 : milliseconds;
  if (qFuzzyCompare(m_holdoffMs, clamped))
    return false;

  m_holdoffMs = clamped;
  return true;
}

/**
 * @brief Sets the sweep timebase in milliseconds, clamped at zero; true when it moved.
 */
bool Widgets::PlotBase::setSweepTimebase(const double milliseconds) noexcept
{
  const double clamped = milliseconds < 0 ? 0 : milliseconds;
  if (qFuzzyCompare(m_timebaseMs, clamped))
    return false;

  m_timebaseMs = clamped;
  return true;
}

/**
 * @brief Sets the sweep mode (auto, normal, single); true when it moved.
 */
bool Widgets::PlotBase::setSweepMode(const SerialStudio::SweepMode mode) noexcept
{
  if (m_sweepMode == mode)
    return false;

  m_sweepMode = mode;
  return true;
}

/**
 * @brief Sets the trigger edge; true when it moved.
 */
bool Widgets::PlotBase::setTriggerEdge(const SerialStudio::TriggerEdge edge) noexcept
{
  if (m_triggerEdge == edge)
    return false;

  m_triggerEdge = edge;
  return true;
}

/**
 * @brief True while the sweep is armed for this plot.
 */
bool Widgets::PlotBase::sweepEnabled() const noexcept
{
  return m_sweepEnabled;
}

/**
 * @brief The configured trigger level.
 */
double Widgets::PlotBase::triggerLevel() const noexcept
{
  return m_triggerLevel;
}

/**
 * @brief The configured re-arm holdoff, in milliseconds.
 */
double Widgets::PlotBase::holdoffMs() const noexcept
{
  return m_holdoffMs;
}

/**
 * @brief The configured sweep timebase, in milliseconds.
 */
double Widgets::PlotBase::timebaseMs() const noexcept
{
  return m_timebaseMs;
}

/**
 * @brief The configured sweep mode.
 */
SerialStudio::SweepMode Widgets::PlotBase::sweepMode() const noexcept
{
  return m_sweepMode;
}

/**
 * @brief The configured trigger edge.
 */
SerialStudio::TriggerEdge Widgets::PlotBase::triggerEdge() const noexcept
{
  return m_triggerEdge;
}
