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
#include <QVector>
#include <vector>

namespace Widgets {
/**
 * @brief Multi-LED status indicator panel widget. Each LED renders the alarm band its dataset
 *        value sits in (band color, label, optional blink); datasets without bands fall back
 *        to the legacy single ledHigh threshold.
 */
class LEDPanel : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             CONSTANT)
  Q_PROPERTY(QStringList titles
             READ titles
             CONSTANT)
  Q_PROPERTY(QStringList colors
             READ colors
             NOTIFY updated)
  Q_PROPERTY(QStringList labels
             READ labels
             NOTIFY updated)
  Q_PROPERTY(QVector<bool> states
             READ states
             NOTIFY updated)
  Q_PROPERTY(QVector<bool> blinks
             READ blinks
             NOTIFY updated)
  // clang-format on

signals:
  void updated();

public:
  explicit LEDPanel(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] const QList<bool>& states() const noexcept;
  [[nodiscard]] const QList<bool>& blinks() const noexcept;
  [[nodiscard]] const QStringList& colors() const noexcept;
  [[nodiscard]] const QStringList& labels() const noexcept;
  [[nodiscard]] const QStringList& titles() const noexcept;

private slots:
  void updateData();
  void onThemeChanged();

private:
  struct LedBand {
    double min   = 0;      ///< Raw lower bound (inclusive)
    double max   = 0;      ///< Raw upper bound (inclusive)
    int severity = -1;     ///< AlarmSeverity enum value (0..3); -1 = use the dataset color
    bool blink   = false;  ///< Flash the LED while this band is active
    QString customColor;
    QString resolvedColor;
    QString label;
  };

  struct Led {
    int hint       = -1;  ///< Last matched band (lookup fast path)
    int activeBand = -1;  ///< Band the value currently sits in; -1 = LED off
    QString offColor;
    std::vector<LedBand> bands;
  };

  [[nodiscard]] static int bandIndexFor(const Led& led, double value) noexcept;
  [[nodiscard]] static QString resolveBandColor(const LedBand& band, const QString& datasetColor);
  void buildLeds();
  void refreshDisplayState(int index);

  int m_index;
  QVector<bool> m_states;
  QVector<bool> m_blinks;
  QStringList m_titles;
  QStringList m_colors;
  QStringList m_labels;
  std::vector<Led> m_leds;
};

}  // namespace Widgets
