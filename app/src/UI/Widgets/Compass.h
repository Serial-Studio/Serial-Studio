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
 * @class Widgets::Compass
 * @brief Compass widget for visualizing heading/bearing data.
 *
 * The Compass class provides a visual compass display showing directional
 * heading data. It displays both the numeric heading value and a corresponding
 * cardinal/intercardinal direction text (N, NE, E, SE, S, SW, W, NW).
 *
 * Key Features:
 * - **Heading Display**: Shows numeric heading in degrees (0-360°)
 * - **Cardinal Directions**: Automatically displays compass direction labels
 * - **Real-time Updates**: Smooth tracking of heading changes
 * - **Intuitive Visualization**: Traditional compass rose presentation
 *
 * The widget automatically converts numeric heading values to appropriate
 * cardinal or intercardinal direction labels for easy interpretation.
 *
 * Typical Use Cases:
 * - GPS navigation displays
 * - Magnetometer/compass sensor visualization
 * - Vehicle/robot heading indication
 * - Directional antenna alignment
 */
class Compass : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double value READ value NOTIFY updated)
  Q_PROPERTY(QString text READ text NOTIFY updated)

signals:
  void updated();

public:
  explicit Compass(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double value() const;
  [[nodiscard]] QString text() const;

private slots:
  void updateData();

private:
  int m_index;
  double m_value;
  QString m_text;
};
} // namespace Widgets
