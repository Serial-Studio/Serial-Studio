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
#include <QElapsedTimer>

namespace Widgets
{
/**
 * @brief A widget that displays the gyroscope data on an attitude indicator.
 */
class Gyroscope : public QQuickItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(double yaw
             READ yaw
             NOTIFY updated)
  Q_PROPERTY(double roll
             READ roll
             NOTIFY updated)
  Q_PROPERTY(double pitch
             READ pitch
             NOTIFY updated)
  Q_PROPERTY(bool integrateValues
             READ integrateValues
             WRITE setIntegrateValues
             NOTIFY integrateValuesChanged)
  // clang-format on

signals:
  void updated();
  void integrateValuesChanged();

public:
  explicit Gyroscope(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double yaw() const;
  [[nodiscard]] double roll() const;
  [[nodiscard]] double pitch() const;
  [[nodiscard]] bool integrateValues() const;

private slots:
  void updateData();
  void setIntegrateValues(const bool enabled);

private:
  int m_index;

  double m_yaw;
  double m_roll;
  double m_pitch;
  QElapsedTimer m_timer;

  bool m_integrateValues;
};

} // namespace Widgets
