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

#pragma once

#include "DSP.h"
#include "SerialStudio.h"

namespace Widgets {

/**
 * @brief The plot state Plot, MultiPlot and FFTPlot hold identically (spec 0075, F14): the
 *        interpolation mode, the visible-X window, the log-X scratch ring and the sweep
 *        configuration. Composed, so each widget keeps its own Q_PROPERTY surface; every mutator
 *        returns whether the value MOVED, because the caller owns the side effects.
 */
class PlotBase {
public:
  PlotBase();

  [[nodiscard]] bool setInterpolationMode(SerialStudio::InterpolationMode mode) noexcept;
  [[nodiscard]] SerialStudio::InterpolationMode interpolationMode() const noexcept;

  void setVisibleXWindow(const double lo, const double hi) noexcept;
  void clampToVisibleX(double& lo, double& hi, const int dataW) const;

  void buildLogXScratch(const DSP::AxisData& x, const double floor);
  [[nodiscard]] const DSP::AxisData& logXScratch() const noexcept;

  [[nodiscard]] bool setSweepEnabled(const bool enabled) noexcept;
  [[nodiscard]] bool setTriggerLevel(const double level) noexcept;
  [[nodiscard]] bool setHoldoff(const double milliseconds) noexcept;
  [[nodiscard]] bool setSweepTimebase(const double milliseconds) noexcept;
  [[nodiscard]] bool setSweepMode(const SerialStudio::SweepMode mode) noexcept;
  [[nodiscard]] bool setTriggerEdge(const SerialStudio::TriggerEdge edge) noexcept;

  [[nodiscard]] bool sweepEnabled() const noexcept;
  [[nodiscard]] double triggerLevel() const noexcept;
  [[nodiscard]] double holdoffMs() const noexcept;
  [[nodiscard]] double timebaseMs() const noexcept;
  [[nodiscard]] SerialStudio::SweepMode sweepMode() const noexcept;
  [[nodiscard]] SerialStudio::TriggerEdge triggerEdge() const noexcept;

private:
  bool m_sweepEnabled;
  double m_triggerLevel;
  double m_holdoffMs;
  double m_timebaseMs;
  double m_visLoX;
  double m_visHiX;
  SerialStudio::SweepMode m_sweepMode;
  SerialStudio::TriggerEdge m_triggerEdge;
  SerialStudio::InterpolationMode m_interpolationMode;
  DSP::AxisData m_logXScratch;
};

}  // namespace Widgets
