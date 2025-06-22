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
 * @brief A widget that displays the accelerometer data.
 */
class Accelerometer : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double theta READ theta NOTIFY updated)
  Q_PROPERTY(double magnitude READ magnitude NOTIFY updated)

signals:
  void updated();

public:
  explicit Accelerometer(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double theta() const;
  [[nodiscard]] double magnitude() const;

private slots:
  void updateData();

private:
  int m_index;
  double m_theta;
  double m_magnitude;
};

} // namespace Widgets
