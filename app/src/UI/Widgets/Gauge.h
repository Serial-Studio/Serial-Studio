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
 * @brief A widget that displays a gauge.
 */
class Gauge : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(QString units READ units CONSTANT)
  Q_PROPERTY(double value READ value NOTIFY updated)
  Q_PROPERTY(double minValue READ minValue CONSTANT)
  Q_PROPERTY(double maxValue READ maxValue CONSTANT)
  Q_PROPERTY(double alarmValue READ alarmValue CONSTANT)

signals:
  void updated();

public:
  explicit Gauge(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] const QString &units() const;

  [[nodiscard]] double value() const;
  [[nodiscard]] double minValue() const;
  [[nodiscard]] double maxValue() const;
  [[nodiscard]] double alarmValue() const;

private slots:
  void updateData();

private:
  int m_index;
  QString m_units;
  double m_value;
  double m_minValue;
  double m_maxValue;
  double m_alarmValue;
};
} // namespace Widgets
