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
 *
 * Populated by @c ReportData::buildFromSession() via a single SQL aggregation
 * query over the @c readings table. Non-numeric datasets are included with
 * @c numericSamples == 0 and zeroed stats — renderers should display a dash
 * (or equivalent) for those rows.
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
 *
 * Used by chart sections to draw trend lines without loading every reading
 * (sessions can hold hundreds of thousands of rows). Populated lazily by
 * @c loadChartSeries() — the base @c ReportData does not carry this payload.
 */
struct DatasetSeries {
  int uniqueId;
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
 *
 * This is the single input to any report renderer (PDF today, HTML/Markdown
 * tomorrow). Fields are populated once by @c buildFromSession() so renderers
 * can iterate without touching the database. Keep this struct rendering-
 * agnostic — no fonts, colours, or page sizes.
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

  /**
   * @brief Build a @c ReportData by querying the given session from @p db.
   *
   * Returns an invalid bundle (@c valid == false) if the session is unknown
   * or the query fails. Always single-threaded and synchronous — callers can
   * invoke from the UI thread for typical session sizes.
   */
  [[nodiscard]] static ReportData buildFromSession(QSqlDatabase& db, int sessionId);
};

/**
 * @brief Loads decimated time-series samples for every numeric parameter.
 *
 * @param db              Open database connection.
 * @param sessionId       Session to query.
 * @param maxSamples      Upper bound on samples per parameter (evenly spaced).
 * @return                One @c DatasetSeries entry per numeric parameter that
 *                        produced at least two samples. Parameters with a
 *                        flat trace (min == max) are still included so the
 *                        chart can render them as a horizontal line.
 */
[[nodiscard]] std::vector<DatasetSeries> loadChartSeries(QSqlDatabase& db,
                                                         int sessionId,
                                                         int maxSamples);

}  // namespace Sessions

#endif  // BUILD_COMMERCIAL
