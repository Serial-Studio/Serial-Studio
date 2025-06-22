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
#include <qfouriertransformer.h>

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
    m_data.clear();
    m_data.squeeze();
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

  QFourierTransformer m_transformer;

  QList<QPointF> m_data;
  QScopedArrayPointer<float> m_fft;
  QScopedArrayPointer<float> m_samples;
};
} // namespace Widgets
