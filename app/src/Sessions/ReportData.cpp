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

#ifdef BUILD_COMMERCIAL

#  include "Sessions/ReportData.h"

#  include <algorithm>
#  include <cmath>
#  include <limits>
#  include <map>
#  include <QDateTime>
#  include <QSqlDatabase>
#  include <QSqlError>
#  include <QSqlQuery>
#  include <QtDebug>
#  include <QVariantMap>

#  include "DSP.h"

//--------------------------------------------------------------------------------------------------
// File-local helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Computes duration between two ISO8601 timestamps in milliseconds.
 *
 * Returns 0 when either string cannot be parsed — callers render the value
 * as "—" so a parse failure degrades gracefully.
 */
static qint64 computeDurationMs(const QString& startedIso, const QString& endedIso)
{
  const auto start = QDateTime::fromString(startedIso, Qt::ISODate);
  const auto end   = QDateTime::fromString(endedIso, Qt::ISODate);
  if (!start.isValid() || !end.isValid())
    return 0;

  return start.msecsTo(end);
}

//--------------------------------------------------------------------------------------------------
// ReportData::buildFromSession
//--------------------------------------------------------------------------------------------------

/**
 * @brief Queries session metadata, columns, tags, and per-dataset stats.
 *
 * The stats query uses a single pass over @c readings with @c AVG(v*v) so we
 * can derive stddev without a second query. First/last values are fetched via
 * window-style subqueries because SQLite lacks @c FIRST_VALUE without CTEs.
 */
Sessions::ReportData Sessions::ReportData::buildFromSession(QSqlDatabase& db, int sessionId)
{
  ReportData out;
  out.valid      = false;
  out.sessionId  = sessionId;
  out.durationMs = 0;
  out.frameCount = 0;

  if (!db.isOpen() || sessionId < 0)
    return out;

  // Fetch session header row (title, timestamps, notes, frame count)
  QSqlQuery q(db);
  q.prepare(
    "SELECT s.project_title, s.started_at, s.ended_at, s.notes, "
    "       (SELECT COUNT(DISTINCT timestamp_ns) FROM readings WHERE session_id = s.session_id) "
    "FROM sessions s WHERE s.session_id = ?");
  q.bindValue(0, sessionId);
  if (!q.exec() || !q.next())
    return out;

  out.projectTitle = q.value(0).toString();
  out.startedAt    = q.value(1).toString();
  out.endedAt      = q.value(2).toString();
  out.notes        = q.value(3).toString();
  out.frameCount   = q.value(4).toLongLong();
  out.durationMs   = computeDurationMs(out.startedAt, out.endedAt);

  // Load column metadata (group/title/units per unique_id)
  QSqlQuery colQ(db);
  colQ.prepare("SELECT unique_id, group_title, title, units "
               "FROM columns WHERE session_id = ? ORDER BY column_id ASC");
  colQ.bindValue(0, sessionId);
  if (!colQ.exec())
    return out;

  std::vector<DatasetStats> datasets;
  while (colQ.next()) {
    DatasetStats ds;
    ds.uniqueId       = colQ.value(0).toInt();
    ds.group          = colQ.value(1).toString();
    ds.title          = colQ.value(2).toString();
    ds.units          = colQ.value(3).toString();
    ds.numericSamples = 0;
    ds.stringSamples  = 0;
    ds.minValue       = 0.0;
    ds.maxValue       = 0.0;
    ds.mean           = 0.0;
    ds.stddev         = 0.0;
    ds.firstValue     = 0.0;
    ds.lastValue      = 0.0;
    datasets.push_back(std::move(ds));
  }

  // One-pass aggregate over numeric readings (count, min, max, mean, mean-of-squares)
  QSqlQuery aggQ(db);
  aggQ.prepare("SELECT unique_id, COUNT(*), MIN(final_numeric_value), MAX(final_numeric_value), "
               "       AVG(final_numeric_value), AVG(final_numeric_value*final_numeric_value) "
               "FROM readings WHERE session_id = ? AND is_numeric = 1 GROUP BY unique_id");
  aggQ.bindValue(0, sessionId);
  if (!aggQ.exec())
    return out;

  while (aggQ.next()) {
    const int uid = aggQ.value(0).toInt();
    auto it       = std::find_if(
      datasets.begin(), datasets.end(), [uid](const DatasetStats& d) { return d.uniqueId == uid; });
    if (it == datasets.end())
      continue;

    it->numericSamples  = aggQ.value(1).toLongLong();
    it->minValue        = aggQ.value(2).toDouble();
    it->maxValue        = aggQ.value(3).toDouble();
    it->mean            = aggQ.value(4).toDouble();
    const double meanSq = aggQ.value(5).toDouble();
    const double var    = std::max(0.0, meanSq - it->mean * it->mean);
    it->stddev          = std::sqrt(var);
  }

  // Count string samples per dataset (separate query — keeps the aggregate pass tight)
  QSqlQuery strQ(db);
  strQ.prepare("SELECT unique_id, COUNT(*) FROM readings "
               "WHERE session_id = ? AND is_numeric = 0 GROUP BY unique_id");
  strQ.bindValue(0, sessionId);
  if (strQ.exec()) {
    while (strQ.next()) {
      const int uid = strQ.value(0).toInt();
      auto it       = std::find_if(datasets.begin(), datasets.end(), [uid](const DatasetStats& d) {
        return d.uniqueId == uid;
      });
      if (it != datasets.end())
        it->stringSamples = strQ.value(1).toLongLong();
    }
  }

  // First numeric value per dataset (ties broken by reading_id)
  QSqlQuery firstQ(db);
  firstQ.prepare("SELECT unique_id, final_numeric_value FROM readings "
                 "WHERE reading_id IN ("
                 "  SELECT MIN(reading_id) FROM readings "
                 "  WHERE session_id = ? AND is_numeric = 1 "
                 "  GROUP BY unique_id)");
  firstQ.bindValue(0, sessionId);
  if (firstQ.exec()) {
    while (firstQ.next()) {
      const int uid = firstQ.value(0).toInt();
      auto it       = std::find_if(datasets.begin(), datasets.end(), [uid](const DatasetStats& d) {
        return d.uniqueId == uid;
      });
      if (it != datasets.end())
        it->firstValue = firstQ.value(1).toDouble();
    }
  }

  // Last numeric value per dataset (ties broken by reading_id)
  QSqlQuery lastQ(db);
  lastQ.prepare("SELECT unique_id, final_numeric_value FROM readings "
                "WHERE reading_id IN ("
                "  SELECT MAX(reading_id) FROM readings "
                "  WHERE session_id = ? AND is_numeric = 1 "
                "  GROUP BY unique_id)");
  lastQ.bindValue(0, sessionId);
  if (lastQ.exec()) {
    while (lastQ.next()) {
      const int uid = lastQ.value(0).toInt();
      auto it       = std::find_if(datasets.begin(), datasets.end(), [uid](const DatasetStats& d) {
        return d.uniqueId == uid;
      });
      if (it != datasets.end())
        it->lastValue = lastQ.value(1).toDouble();
    }
  }

  // Collect session tags (sorted alphabetically by the SQL query)
  QSqlQuery tagQ(db);
  tagQ.prepare("SELECT t.label FROM tags t "
               "JOIN session_tags st ON st.tag_id = t.tag_id "
               "WHERE st.session_id = ? ORDER BY t.label");
  tagQ.bindValue(0, sessionId);
  if (tagQ.exec())
    while (tagQ.next())
      out.tags.append(tagQ.value(0).toString());

  out.datasets = std::move(datasets);
  out.valid    = true;
  return out;
}

//--------------------------------------------------------------------------------------------------
// loadChartSeries
//--------------------------------------------------------------------------------------------------

namespace {

/**
 * @brief Column metadata for one plotted parameter.
 */
struct ChartMeta {
  int uid;
  QString group;
  QString title;
  QString units;
};

/**
 * @brief Reads a query result set into parallel DSP ring buffers.
 */
std::size_t readAxisData(QSqlQuery& rows,
                         qint64 originNs,
                         qint64 reservation,
                         DSP::AxisData& x,
                         DSP::AxisData& y,
                         double& globalMin,
                         double& globalMax)
{
  const std::size_t capacity = static_cast<std::size_t>(std::max<qint64>(reservation, 1));
  x.resize(capacity);
  y.resize(capacity);
  x.clear();
  y.clear();

  globalMin         = std::numeric_limits<double>::infinity();
  globalMax         = -std::numeric_limits<double>::infinity();
  std::size_t count = 0;

  while (rows.next()) {
    const double val = rows.value(1).toDouble();
    if (!std::isfinite(val))
      continue;

    const qint64 ts   = rows.value(0).toLongLong();
    const double tSec = static_cast<double>(ts - originNs) / 1.0e9;
    x.push(tSec);
    y.push(val);

    globalMin = std::min(globalMin, val);
    globalMax = std::max(globalMax, val);
    ++count;
  }

  return count;
}

/**
 * @brief Appends @p index to @p indices when it is not already present.
 */
void appendUniqueIndex(std::vector<std::size_t>& indices, std::size_t index)
{
  Q_ASSERT(indices.size() <= 8);

  for (const auto value : indices)
    if (value == index)
      return;

  indices.push_back(index);
}

/**
 * @brief Appends one bucket's representative sample indices to @p indices.
 */
void appendBucketSamples(const DSP::AxisData& y,
                         std::size_t begin,
                         std::size_t end,
                         int target,
                         std::vector<std::size_t>& indices)
{
  Q_ASSERT(begin < end);
  Q_ASSERT(target > 0);

  const std::size_t bucketSize = end - begin;
  const std::size_t goal = std::min<std::size_t>(bucketSize, static_cast<std::size_t>(target));

  std::size_t minIndex = begin;
  std::size_t maxIndex = begin;
  for (std::size_t i = begin + 1; i < end; ++i) {
    if (y[i] < y[minIndex])
      minIndex = i;

    if (y[i] > y[maxIndex])
      maxIndex = i;
  }

  std::vector<std::size_t> local;
  local.reserve(goal);
  appendUniqueIndex(local, begin);
  appendUniqueIndex(local, minIndex);
  appendUniqueIndex(local, maxIndex);
  appendUniqueIndex(local, end - 1);

  for (std::size_t i = begin + 1; i + 1 < end && local.size() < goal; ++i)
    appendUniqueIndex(local, i);

  std::sort(local.begin(), local.end());
  indices.insert(indices.end(), local.begin(), local.end());
}

/**
 * @brief Appends evenly-spaced samples until @p indices reaches @p budget.
 */
void appendBudgetFillSamples(std::size_t count,
                             std::size_t budget,
                             std::vector<std::size_t>& indices)
{
  Q_ASSERT(count >= budget);
  Q_ASSERT(indices.size() <= budget);

  std::vector<unsigned char> used(count, 0);
  for (const auto index : indices) {
    Q_ASSERT(index < count);
    used[index] = 1;
  }

  const std::size_t divisor = budget - 1;
  for (std::size_t i = 0; i < budget && indices.size() < budget; ++i) {
    const std::size_t index = (i == divisor) ? count - 1 : (i * (count - 1)) / divisor;
    if (used[index])
      continue;

    used[index] = 1;
    indices.push_back(index);
  }

  for (std::size_t i = 0; i < count && indices.size() < budget; ++i) {
    if (used[i])
      continue;

    used[i] = 1;
    indices.push_back(i);
  }
}

/**
 * @brief Copies the selected samples into @p series in chronological order.
 */
void writeSelectedSamples(const DSP::AxisData& x,
                          const DSP::AxisData& y,
                          const std::vector<std::size_t>& indices,
                          Sessions::DatasetSeries& series)
{
  Q_ASSERT(!indices.empty());
  Q_ASSERT(series.timesSec.empty());
  Q_ASSERT(series.values.empty());

  series.timesSec.reserve(indices.size());
  series.values.reserve(indices.size());

  for (const auto index : indices) {
    Q_ASSERT(index < x.size() && index < y.size());
    series.timesSec.push_back(x[index]);
    series.values.push_back(y[index]);
  }
}

/**
 * @brief Writes either the raw samples or a fixed-budget decimated series.
 */
void writeReportSamples(const DSP::AxisData& x,
                        const DSP::AxisData& y,
                        std::size_t count,
                        int maxSamples,
                        Sessions::DatasetSeries& series)
{
  Q_ASSERT(count > 0);
  Q_ASSERT(maxSamples >= 2);
  Q_ASSERT(count <= x.size() && count <= y.size());

  const std::size_t budget = static_cast<std::size_t>(maxSamples);
  if (count <= budget) {
    std::vector<std::size_t> indices;
    indices.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
      indices.push_back(i);

    writeSelectedSamples(x, y, indices, series);
    return;
  }

  const std::size_t bucketCount = std::max<std::size_t>(1, budget / 4);
  const std::size_t baseTarget  = budget / bucketCount;
  const std::size_t remainder   = budget % bucketCount;
  Q_ASSERT(bucketCount > 0);
  Q_ASSERT(baseTarget > 0);

  std::vector<std::size_t> indices;
  indices.reserve(budget);

  for (std::size_t bucket = 0; bucket < bucketCount; ++bucket) {
    const std::size_t begin = (bucket * count) / bucketCount;
    const std::size_t end   = ((bucket + 1) * count) / bucketCount;
    if (begin >= end)
      continue;

    const int target = static_cast<int>(baseTarget + (bucket < remainder ? 1 : 0));
    appendBucketSamples(y, begin, end, target, indices);
  }

  if (indices.size() < budget)
    appendBudgetFillSamples(count, budget, indices);

  std::sort(indices.begin(), indices.end());
  writeSelectedSamples(x, y, indices, series);
}

/**
 * @brief Enumerates numeric parameters for @p sessionId in column order.
 */
std::vector<ChartMeta> loadChartParameters(QSqlDatabase& db, int sessionId)
{
  std::vector<ChartMeta> metas;

  QSqlQuery q(db);
  q.prepare("SELECT unique_id, group_title, title, units FROM columns "
            "WHERE session_id = ? ORDER BY column_id ASC");
  q.bindValue(0, sessionId);

  if (!q.exec()) {
    qWarning() << "[Sessions::ReportData] column enumeration failed:" << q.lastError().text();
    return metas;
  }

  while (q.next()) {
    ChartMeta m;
    m.uid   = q.value(0).toInt();
    m.group = q.value(1).toString();
    m.title = q.value(2).toString();
    m.units = q.value(3).toString();
    metas.push_back(std::move(m));
  }

  return metas;
}

/**
 * @brief Returns the session's earliest and latest @c timestamp_ns values.
 */
std::pair<qint64, qint64> loadSessionTimeSpan(QSqlDatabase& db, int sessionId)
{
  QSqlQuery q(db);
  q.prepare("SELECT MIN(timestamp_ns), MAX(timestamp_ns) FROM readings WHERE session_id = ?");
  q.bindValue(0, sessionId);

  if (!q.exec() || !q.next()) {
    qWarning() << "[Sessions::ReportData] time-span query failed:" << q.lastError().text();
    return {0, 0};
  }

  return {q.value(0).toLongLong(), q.value(1).toLongLong()};
}

/**
 * @brief Numeric-sample count per parameter, keyed by @c unique_id.
 */
std::map<int, qint64> loadSampleCounts(QSqlDatabase& db, int sessionId)
{
  std::map<int, qint64> counts;

  QSqlQuery q(db);
  q.prepare("SELECT unique_id, COUNT(*) FROM readings "
            "WHERE session_id = ? AND is_numeric = 1 GROUP BY unique_id");
  q.bindValue(0, sessionId);

  if (!q.exec()) {
    qWarning() << "[Sessions::ReportData] sample-count query failed:" << q.lastError().text();
    return counts;
  }

  while (q.next())
    counts.emplace(q.value(0).toInt(), q.value(1).toLongLong());

  return counts;
}

}  // namespace

/**
 * @brief Loads downsampled numeric time-series for every parameter.
 */
std::vector<Sessions::DatasetSeries> Sessions::loadChartSeries(QSqlDatabase& db,
                                                               int sessionId,
                                                               int maxSamples)
{
  std::vector<DatasetSeries> out;
  if (!db.isOpen() || sessionId < 0 || maxSamples < 2)
    return out;

  Q_ASSERT(maxSamples >= 2);
  Q_ASSERT(db.isOpen());

  const auto metas = loadChartParameters(db, sessionId);
  if (metas.empty())
    return out;

  const auto [originNs, _] = loadSessionTimeSpan(db, sessionId);
  const auto counts        = loadSampleCounts(db, sessionId);

  DSP::AxisData x(16);
  DSP::AxisData y(16);

  for (const auto& m : metas) {
    const auto it      = counts.find(m.uid);
    const qint64 total = (it != counts.end()) ? it->second : 0;
    if (total < 2)
      continue;

    QSqlQuery rows(db);
    rows.setForwardOnly(true);
    rows.prepare("SELECT timestamp_ns, final_numeric_value FROM readings "
                 "WHERE session_id = ? AND unique_id = ? AND is_numeric = 1 "
                 "ORDER BY timestamp_ns, reading_id");
    rows.bindValue(0, sessionId);
    rows.bindValue(1, m.uid);
    if (!rows.exec()) {
      qWarning() << "[Sessions::ReportData] readings query failed for uid" << m.uid << ":"
                 << rows.lastError().text();
      continue;
    }

    double globalMin = 0.0;
    double globalMax = 0.0;
    const auto count = readAxisData(rows, originNs, total, x, y, globalMin, globalMax);
    if (count < 2)
      continue;

    DatasetSeries series;
    series.uniqueId     = m.uid;
    series.totalSamples = total;
    series.group        = m.group;
    series.title        = m.title;
    series.units        = m.units;
    series.minValue     = globalMin;
    series.maxValue     = globalMax;
    writeReportSamples(x, y, count, maxSamples, series);
    Q_ASSERT(series.timesSec.size() == series.values.size());
    Q_ASSERT(series.values.size() == std::min<std::size_t>(count, maxSamples));

    if (series.values.size() < 2)
      continue;

    out.push_back(std::move(series));
  }

  return out;
}

#endif  // BUILD_COMMERCIAL
