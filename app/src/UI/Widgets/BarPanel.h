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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#pragma once

#include <QQuickItem>
#include <QStringList>
#include <QVariantList>
#include <QVector>
#include <vector>

#include "UI/Dashboard.h"

namespace Widgets {
/**
 * @brief Multi-channel bar panel group widget: one labeled, alarm-band-zoned bar per dataset
 *        in the owning group. Fills take the active band's severity color (nearest band when
 *        the value sits outside every band); optional min/max hold markers per dataset.
 */
class BarPanel : public QQuickItem {
  // clang-format off
  Q_OBJECT
  Q_PROPERTY(int count
             READ count
             CONSTANT)
  Q_PROPERTY(QString styleMode
             READ styleMode
             CONSTANT)
  Q_PROPERTY(QStringList titles
             READ titles
             CONSTANT)
  Q_PROPERTY(QStringList units
             READ units
             CONSTANT)
  Q_PROPERTY(QVector<bool> ranged
             READ ranged
             CONSTANT)
  Q_PROPERTY(QVariantList bands
             READ bands
             CONSTANT)
  Q_PROPERTY(QStringList valueTexts
             READ valueTexts
             NOTIFY updated)
  Q_PROPERTY(QVector<bool> numeric
             READ numeric
             NOTIFY updated)
  Q_PROPERTY(QVector<double> fracs
             READ fracs
             NOTIFY updated)
  Q_PROPERTY(QVector<int> severities
             READ severities
             NOTIFY updated)
  Q_PROPERTY(QVector<bool> extremesValid
             READ extremesValid
             NOTIFY updated)
  Q_PROPERTY(QVector<double> minSeenFracs
             READ minSeenFracs
             NOTIFY updated)
  Q_PROPERTY(QVector<double> maxSeenFracs
             READ maxSeenFracs
             NOTIFY updated)
  // clang-format on

signals:
  void updated();

public:
  explicit BarPanel(const int index = -1, QQuickItem* parent = nullptr);

  [[nodiscard]] int count() const noexcept;
  [[nodiscard]] const QString& styleMode() const noexcept;
  [[nodiscard]] const QStringList& titles() const noexcept;
  [[nodiscard]] const QStringList& units() const noexcept;
  [[nodiscard]] const QVector<bool>& ranged() const noexcept;
  [[nodiscard]] const QVariantList& bands() const noexcept;
  [[nodiscard]] const QStringList& valueTexts() const noexcept;
  [[nodiscard]] const QVector<bool>& numeric() const noexcept;
  [[nodiscard]] const QVector<double>& fracs() const noexcept;
  [[nodiscard]] const QVector<int>& severities() const noexcept;
  [[nodiscard]] const QVector<bool>& extremesValid() const noexcept;
  [[nodiscard]] const QVector<double>& minSeenFracs() const noexcept;
  [[nodiscard]] const QVector<double>& maxSeenFracs() const noexcept;

private slots:
  void updateData();

private:
  struct RowBand {
    double min   = 0;  ///< Raw lower bound (inclusive)
    double max   = 0;  ///< Raw upper bound (inclusive)
    int severity = 2;  ///< AlarmSeverity enum value (0..3)
  };

  struct Row {
    bool ranged      = false;  ///< Widget range is non-degenerate; the bar track renders
    bool extremeHold = false;  ///< Dataset opted into min/max hold markers
    int hint         = -1;     ///< Last matched band (lookup fast path)
    int uniqueId     = -1;     ///< Dataset identity for the dashboard extremes store
    int decimals     = 0;      ///< Value-text decimal places
    double min       = 0;      ///< Raw scale minimum
    double max       = 0;      ///< Raw scale maximum
    std::vector<RowBand> bands;
  };

  [[nodiscard]] static int bandIndexFor(const Row& row, double value) noexcept;
  [[nodiscard]] static int nearestBandIndex(const Row& row, double value) noexcept;
  [[nodiscard]] double rowFraction(const Row& row, double value) const noexcept;
  void buildRows();
  bool refreshRow(int index, const DataModel::Dataset& dataset);

  int m_index;
  QString m_styleMode;
  QStringList m_titles;
  QStringList m_units;
  QStringList m_valueTexts;
  QVariantList m_bandsAsVariant;
  QVector<bool> m_ranged;
  QVector<bool> m_numeric;
  QVector<bool> m_extremesOk;
  QVector<int> m_severities;
  QVector<double> m_fracs;
  QVector<double> m_minSeenFracs;
  QVector<double> m_maxSeenFracs;
  std::vector<Row> m_rows;

  UI::Dashboard& m_dashboard;
};
}  // namespace Widgets
