/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
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

#include <QQuickItem>
#include <QString>
#include <QVariantList>
#include <QVector>
#include <vector>

#include "DSP.h"

namespace DataModel {
struct AlarmBand;
}  // namespace DataModel

namespace Widgets {
/**
 * @brief Precomputed render data for a single alarm band on a widget.
 */
struct BarBand {
  double min     = 0;      ///< Raw lower bound
  double max     = 0;      ///< Raw upper bound
  double fracMin = 0;      ///< Normalised lower bound (0..1)
  double fracMax = 0;      ///< Normalised upper bound (0..1)
  int severity   = 2;      ///< AlarmSeverity enum value (0..3); default Warning
  bool blink     = false;  ///< Flash the indicator while the band is active
  QString customColor;
  QString label;
};

/**
 * @brief Data model for a normalized value display (e.g. bar or gauge).
 */
class Bar : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(bool alarmsDefined
             READ alarmsDefined
             CONSTANT)
  Q_PROPERTY(bool alarmTriggered
             READ alarmTriggered
             NOTIFY updated)
  Q_PROPERTY(int activeBandSeverity
             READ activeBandSeverity
             NOTIFY updated)
  Q_PROPERTY(QString activeBandLabel
             READ activeBandLabel
             NOTIFY updated)
  Q_PROPERTY(QVariantList alarmBands
             READ alarmBands
             CONSTANT)
  Q_PROPERTY(double value
             READ value
             NOTIFY updated)
  Q_PROPERTY(double minValue
             READ minValue
             CONSTANT)
  Q_PROPERTY(double maxValue
             READ maxValue
             CONSTANT)
  Q_PROPERTY(double normalizedValue
             READ normalizedValue
             NOTIFY updated)
  Q_PROPERTY(QString title
             READ title
             CONSTANT)
  Q_PROPERTY(QString units
             READ units
             CONSTANT)
  Q_PROPERTY(int displayTickCount
             READ displayTickCount
             CONSTANT)
  Q_PROPERTY(QString displayFormat
             READ displayFormat
             CONSTANT)
  // clang-format on

signals:
  void updated();

public:
  explicit Bar(const int index             = -1,
               QQuickItem* parent          = nullptr,
               bool autoInitFromBarDataset = true);

  [[nodiscard]] bool alarmsDefined() const noexcept;
  [[nodiscard]] bool alarmTriggered() const noexcept;
  [[nodiscard]] int activeBandSeverity() const noexcept;
  [[nodiscard]] const QString& activeBandLabel() const noexcept;
  [[nodiscard]] const QVariantList& alarmBands() const noexcept;
  [[nodiscard]] int displayTickCount() const noexcept;
  [[nodiscard]] const QString& title() const noexcept;
  [[nodiscard]] const QString& units() const noexcept;
  [[nodiscard]] const QString& displayFormat() const noexcept;

  [[nodiscard]] double value() const noexcept;
  [[nodiscard]] double minValue() const noexcept;
  [[nodiscard]] double maxValue() const noexcept;
  [[nodiscard]] double normalizedValue() const noexcept;

protected slots:
  virtual void updateData();

private:
  inline double computeFractional(double value) const
  {
    const double min   = qMin(m_minValue, m_maxValue);
    const double max   = qMax(m_minValue, m_maxValue);
    const double range = max - min;

    if (DSP::isZero(range))
      return 0.0;

    const double clamped = qBound(min, value, max);
    return qBound(0.0, (clamped - min) / range, 1.0);
  }

protected:
  void buildBands(const std::vector<DataModel::AlarmBand>& srcBands);
  [[nodiscard]] int bandIndexFor(double value) const noexcept;
  void recomputeActiveBand(double value);

  int m_index;
  int m_displayTickCount;
  QString m_title;
  QString m_units;
  QString m_displayFormat;

  double m_value;
  double m_minValue;
  double m_maxValue;

  QVector<BarBand> m_bands;
  QVariantList m_bandsAsVariant;
  QString m_emptyLabel;
  int m_activeBandIndex;
  int m_lastBandHint;
};
}  // namespace Widgets
