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

#include <QVector>
#include <QQuickItem>
#include <QLineSeries>

#include <vector>
#include <liquid.h>

namespace Widgets
{
/**
 * @brief A widget that plots the FFT of a dataset.
 */
class FFTPlot : public QQuickItem
{
  Q_OBJECT
  Q_PROPERTY(double minX READ minX CONSTANT)
  Q_PROPERTY(double maxX READ maxX CONSTANT)
  Q_PROPERTY(double minY READ minY CONSTANT)
  Q_PROPERTY(double maxY READ maxY CONSTANT)
  Q_PROPERTY(double xTickInterval READ xTickInterval CONSTANT)
  Q_PROPERTY(double yTickInterval READ yTickInterval CONSTANT)

public:
  explicit FFTPlot(const int index = -1, QQuickItem *parent = nullptr);
  ~FFTPlot()
  {
    if (m_plan)
      fft_destroy_plan(m_plan);
  }

  [[nodiscard]] double minX() const;
  [[nodiscard]] double maxX() const;
  [[nodiscard]] double minY() const;
  [[nodiscard]] double maxY() const;
  [[nodiscard]] double xTickInterval() const;
  [[nodiscard]] double yTickInterval() const;

public slots:
  void draw(QLineSeries *series);

private slots:
  void updateData();

private:
  int m_size;
  int m_index;
  int m_samplingRate;

  double m_minX;
  double m_maxX;
  double m_minY;
  double m_maxY;

  double m_center;
  double m_halfRange;
  bool m_scaleIsValid;

  fftplan m_plan;
  QList<QPointF> m_data;
  std::vector<float> m_window;
  std::vector<liquid_float_complex> m_samples;
  std::vector<liquid_float_complex> m_fftOutput;
};
} // namespace Widgets
