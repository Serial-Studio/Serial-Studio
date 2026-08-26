/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include <algorithm>
#include <QCursor>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QQuickWindow>
#include <QtMath>
#include <QWheelEvent>

#include "DSPSimd.h"
#include "Misc/CommonFonts.h"
#include "Misc/ThemeManager.h"
#include "Misc/TimerEvents.h"
#include "SSAssert.h"
#include "UI/Dashboard.h"
#include "UI/Widgets/AudioExport.h"
#include "UI/Widgets/FFTWindow.h"
#include "UI/Widgets/Waterfall.h"
#include "UI/Widgets/Waterfall/WaterfallMath.h"

using namespace UI::Widgets::WaterfallDetail;

//--------------------------------------------------------------------------------------------------
// Tick generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Picks a {1,2,5}*10^n step for a given range and target tick count.
 */
Widgets::Waterfall::AxisTicks Widgets::Waterfall::computeFreqTicks(double maxFreq, int targetCount)
{
  AxisTicks out{{}, 1.0, maxFreq};
  if (!std::isfinite(maxFreq) || maxFreq <= 0.0)
    return out;

  const double target  = std::max(2, targetCount);
  const double raw     = maxFreq / target;
  const double base    = waterfallFastPow10(std::floor(std::log10(raw)));
  const double cands[] = {1.0, 2.0, 5.0, 10.0};

  double step = base;
  for (double c : cands) {
    if (raw <= c * base) {
      step = c * base;
      break;
    }
  }

  out.step       = step;
  out.displayMax = std::ceil(maxFreq / step) * step;
  for (double v = 0.0; v <= out.displayMax + 1e-6; v += step)
    out.values.push_back(v);

  return out;
}

/**
 * @brief Same algorithm as computeFreqTicks but for the seconds axis.
 */
Widgets::Waterfall::AxisTicks Widgets::Waterfall::computeTimeTicks(double maxSeconds,
                                                                   int targetCount)
{
  return computeFreqTicks(maxSeconds, targetCount);
}

//--------------------------------------------------------------------------------------------------
// Format helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Formats a frequency value as Hz / kHz / MHz with one decimal at most.
 */
QString Widgets::Waterfall::formatFreqTick(double hz)
{
  const double abs = std::fabs(hz);
  if (abs >= 1e6)
    return QString::number(hz / 1e6, 'g', 3) + QStringLiteral(" MHz");

  if (abs >= 1e3)
    return QString::number(hz / 1e3, 'g', 3) + QStringLiteral(" kHz");

  return QString::number(hz, 'g', 3) + QStringLiteral(" Hz");
}

/**
 * @brief Formats a time value -- integer seconds when step >= 1, decimals otherwise.
 */
QString Widgets::Waterfall::formatTimeTick(double seconds, double step)
{
  if (step >= 1.0)
    return QString::number(std::round(seconds), 'f', 0);

  const int decimals = std::max(0, -static_cast<int>(std::floor(std::log10(step))));
  return QString::number(seconds, 'f', decimals);
}
