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

#include <QMap>

#include "DSP.h"

namespace UI {

/**
 * @brief The per-widget control state owned by UI::Dashboard and bound here by reference: the
 *        run flags the ingest path reads through cached `const bool*` pointers, and the sweep
 *        engines it feeds. Both stay Dashboard members because the pre-resolved push tables cache
 *        addresses into these maps; moving the containers would dangle those pointers.
 */
struct PlotControlBindings {
  QMap<int, bool>& activePlots;
  QMap<int, bool>& activeFFTPlots;
  QMap<int, bool>& activeMultiplots;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool>& activeWaterfalls;
#endif
  QMap<int, DSP::SweepEngine>& plotSweep;
  QMap<int, DSP::SweepEngine>& multiplotSweep;
};

/**
 * @brief Command-rate control surface over the dashboard's per-widget run flags and sweep/trigger
 *        engines: what the widget toolbars toggle, what the license gate admits, and what a
 *        time-range change or a replay scrub restores. Nothing here runs per block; the block path
 *        only ever reads the flags and engines these methods write.
 */
class PlotControlBank {
public:
  explicit PlotControlBank(const PlotControlBindings& bindings);
  PlotControlBank(PlotControlBank&&)                 = delete;
  PlotControlBank(const PlotControlBank&)            = delete;
  PlotControlBank& operator=(PlotControlBank&&)      = delete;
  PlotControlBank& operator=(const PlotControlBank&) = delete;

  [[nodiscard]] bool plotRunning(const int index) const;
  [[nodiscard]] bool fftPlotRunning(const int index) const;
  [[nodiscard]] bool multiplotRunning(const int index) const;

  [[nodiscard]] const DSP::SweepEngine& plotSweep(const int index) const;
  [[nodiscard]] const DSP::SweepEngine& multiplotSweep(const int index) const;

  void setPlotRunning(const int index, const bool enabled);
  void setFFTPlotRunning(const int index, const bool enabled);
  void setMultiplotRunning(const int index, const bool enabled);

#ifdef BUILD_COMMERCIAL
  [[nodiscard]] bool waterfallRunning(const int index) const;
  void setWaterfallRunning(const int index, const bool enabled);
#endif

  void setPlotSweep(const int index,
                    const bool enabled,
                    const double level,
                    const int edge,
                    const int mode,
                    const double holdoff,
                    const double timebase);
  void setMultiplotSweep(const int index,
                         const bool enabled,
                         const double level,
                         const int edge,
                         const int mode,
                         const double holdoff,
                         const int triggerCurve,
                         const double timebase);

  void armPlotSweep(const int index);
  void armMultiplotSweep(const int index);
  void setPlotSweepRetention(const int index, const int count);

  void resetSweepStates();
  void restorePlotSweepConfig(const QMap<int, DSP::SweepEngine>& saved);
  void restoreMultiplotSweepConfig(const QMap<int, DSP::SweepEngine>& saved);

private:
  QMap<int, bool>& m_activePlots;
  QMap<int, bool>& m_activeFFTPlots;
  QMap<int, bool>& m_activeMultiplots;
#ifdef BUILD_COMMERCIAL
  QMap<int, bool>& m_activeWaterfalls;
#endif
  QMap<int, DSP::SweepEngine>& m_plotSweep;
  QMap<int, DSP::SweepEngine>& m_multiplotSweep;
};

}  // namespace UI
