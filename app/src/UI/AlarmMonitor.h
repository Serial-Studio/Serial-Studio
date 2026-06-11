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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <array>
#include <QObject>
#include <QString>
#include <vector>

namespace UI {
/**
 * @brief Dataset-level alarm-band tracker; posts band-entry notifications independently of
 *        whether any widget displaying the dataset is instantiated or visible.
 */
class AlarmMonitor : public QObject {
  Q_OBJECT

private:
  explicit AlarmMonitor();
  AlarmMonitor(AlarmMonitor&&)                 = delete;
  AlarmMonitor(const AlarmMonitor&)            = delete;
  AlarmMonitor& operator=(AlarmMonitor&&)      = delete;
  AlarmMonitor& operator=(const AlarmMonitor&) = delete;

public:
  [[nodiscard]] static AlarmMonitor& instance();

  void setupExternalConnections();

private slots:
  void rebuildTrackers();
  void evaluateAlarms();

private:
  struct Band {
    double min;
    double max;
    int severity;
    QString label;
  };

  struct Tracker {
    int uniqueId;
    int hint;
    int lastFiredBand;
    bool initialized;
    double rangeMin;
    double rangeMax;
    QString title;
    QString units;
    std::array<qint64, 4> lastFireMs;
    std::vector<Band> bands;
  };

  [[nodiscard]] static int bandIndexFor(const Tracker& tracker, double value) noexcept;
  void processValue(Tracker& tracker, double value);

  std::vector<Tracker> m_trackers;
};
}  // namespace UI
