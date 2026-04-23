/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <QDateTime>
#  include <QString>
#  include <QStringList>
#  include <QVariantList>
#  include <vector>

class QSqlDatabase;

namespace Sessions {

/**
 * @brief Per-dataset aggregate statistics for a session.
 */
struct DatasetStats {
  int uniqueId;
  qint64 numericSamples;
  qint64 stringSamples;
  double minValue;
  double maxValue;
  double mean;
  double stddev;
  double firstValue;
  double lastValue;
  QString group;
  QString title;
  QString units;
};

/**
 * @brief Decimated time-series samples for a single dataset.
 */
struct DatasetSeries {
  int uniqueId;
  qint64 totalSamples;
  QString group;
  QString title;
  QString units;
  double minValue;
  double maxValue;
  std::vector<double> timesSec;
  std::vector<double> values;
};

/**
 * @brief Immutable data bundle describing a recorded session.
 */
struct ReportData {
  bool valid;
  int sessionId;
  QString projectTitle;
  QString startedAt;
  QString endedAt;
  qint64 durationMs;
  qint64 frameCount;
  QString notes;
  QStringList tags;
  std::vector<DatasetStats> datasets;

  [[nodiscard]] static ReportData buildFromSession(QSqlDatabase& db, int sessionId);
};

[[nodiscard]] std::vector<DatasetSeries> loadChartSeries(QSqlDatabase& db,
                                                         int sessionId,
                                                         int maxSamples);

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
