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
 * @class Widgets::Accelerometer
 * @brief Visual widget for displaying 3D acceleration vector data.
 *
 * The Accelerometer class visualizes acceleration data from three axes (X, Y,
 * Z), computing derived values such as total G-force, pitch and roll tilt
 * angles, and peak tracking. A configurable polar plot displays the 2D
 * projection (magnitude + theta) while an info strip shows computed values.
 *
 * Key Features:
 * - **3-Axis Input**: Reads X, Y, Z acceleration datasets
 * - **Polar Representation**: 2D magnitude and direction on configurable range
 * - **Tilt Angles**: Computes pitch and roll from raw acceleration
 * - **Peak Tracking**: Tracks session maximum G-force
 * - **Configurable Range**: User-adjustable max G for polar plot
 * - **Input Unit Toggle**: Supports both m/s^2 and G-force input
 */
class Accelerometer : public QQuickItem
{
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(double accelX
             READ accelX
             NOTIFY updated)
  Q_PROPERTY(double accelY
             READ accelY
             NOTIFY updated)
  Q_PROPERTY(double accelZ
             READ accelZ
             NOTIFY updated)
  Q_PROPERTY(double g
             READ g
             NOTIFY updated)
  Q_PROPERTY(double pitch
             READ pitch
             NOTIFY updated)
  Q_PROPERTY(double roll
             READ roll
             NOTIFY updated)
  Q_PROPERTY(double peakG
             READ peakG
             NOTIFY updated)
  Q_PROPERTY(double theta
             READ theta
             NOTIFY updated)
  Q_PROPERTY(double magnitude
             READ magnitude
             NOTIFY updated)
  Q_PROPERTY(double maxG
             READ maxG
             WRITE setMaxG
             NOTIFY configChanged)
  Q_PROPERTY(bool inputInG
             READ inputInG
             WRITE setInputInG
             NOTIFY configChanged)
  // clang-format on

signals:
  void updated();
  void configChanged();

public:
  explicit Accelerometer(const int index = -1, QQuickItem *parent = nullptr);

  [[nodiscard]] double accelX() const;
  [[nodiscard]] double accelY() const;
  [[nodiscard]] double accelZ() const;
  [[nodiscard]] double g() const;
  [[nodiscard]] double pitch() const;
  [[nodiscard]] double roll() const;
  [[nodiscard]] double peakG() const;
  [[nodiscard]] double theta() const;
  [[nodiscard]] double magnitude() const;
  [[nodiscard]] double maxG() const;
  [[nodiscard]] bool inputInG() const;

  Q_INVOKABLE void resetPeakG();

private slots:
  void updateData();
  void setMaxG(const double maxG);
  void setInputInG(const bool enabled);

private:
  int m_index;
  double m_x;
  double m_y;
  double m_z;
  double m_g;
  double m_pitch;
  double m_roll;
  double m_peakG;
  double m_theta;
  double m_magnitude;
  double m_maxG;
  bool m_inputInG;
  bool m_filterInitialized;
};

} // namespace Widgets
