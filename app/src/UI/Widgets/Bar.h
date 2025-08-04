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

namespace Widgets
{
/**
 * @class Widgets::Bar
 * @brief Data model for a normalized value display (e.g. bar or gauge).
 *
 * Holds value, range, and alarm threshold data for a single dashboard element.
 * Emits update signals when the value changes.
 *
 * Designed for use in real-time UI components like bar or gauge widgets.
 */
class Bar : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString units READ units CONSTANT)
  Q_PROPERTY(double value READ value NOTIFY updated)
  Q_PROPERTY(double minValue READ minValue CONSTANT)
  Q_PROPERTY(double maxValue READ maxValue CONSTANT)
  Q_PROPERTY(double alarmLow READ alarmLow CONSTANT)
  Q_PROPERTY(double alarmHigh READ alarmHigh CONSTANT)
  Q_PROPERTY(bool alarmsDefined READ alarmsDefined CONSTANT)
  Q_PROPERTY(bool alarmTriggered READ alarmTriggered NOTIFY updated)
  Q_PROPERTY(double normalizedValue READ normalizedValue NOTIFY updated)
  Q_PROPERTY(double normalizedAlarmLow READ normalizedAlarmLow CONSTANT)
  Q_PROPERTY(double normalizedAlarmHigh READ normalizedAlarmHigh CONSTANT)

signals:
  void updated();

public:
  explicit Bar(const int index = -1, QQuickItem *parent = nullptr,
               bool autoInitFromBarDataset = true);

  [[nodiscard]] bool alarmsDefined() const;
  [[nodiscard]] bool alarmTriggered() const;
  [[nodiscard]] const QString &units() const;

  [[nodiscard]] double value() const;
  [[nodiscard]] double minValue() const;
  [[nodiscard]] double maxValue() const;
  [[nodiscard]] double alarmLow() const;
  [[nodiscard]] double alarmHigh() const;
  [[nodiscard]] double normalizedValue() const;
  [[nodiscard]] double normalizedAlarmLow() const;
  [[nodiscard]] double normalizedAlarmHigh() const;

protected slots:
  virtual void updateData();

private:
  inline double computeFractional(double value) const
  {
    const double min = qMin(m_minValue, m_maxValue);
    const double max = qMax(m_minValue, m_maxValue);
    const double range = max - min;

    if (qFuzzyIsNull(range))
      return 0.0;

    const double clamped = qBound(min, value, max);
    return qBound(0.0, (clamped - min) / range, 1.0);
  }

protected:
  int m_index;
  QString m_units;

  double m_value;
  double m_minValue;
  double m_maxValue;
  double m_alarmLow;
  double m_alarmHigh;

  bool m_alarmsDefined;
};
} // namespace Widgets
