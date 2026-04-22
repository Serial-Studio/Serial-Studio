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
#  include <QDateTime>
#  include <QSqlDatabase>
#  include <QSqlQuery>
#  include <QVariantMap>

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

  // Fetch first numeric value per dataset (earliest timestamp)
  QSqlQuery firstQ(db);
  firstQ.prepare(
    "SELECT unique_id, final_numeric_value FROM readings "
    "WHERE session_id = ? AND is_numeric = 1 AND timestamp_ns = "
    "  (SELECT MIN(timestamp_ns) FROM readings r2 "
    "   WHERE r2.session_id = readings.session_id AND r2.unique_id = readings.unique_id "
    "   AND r2.is_numeric = 1)");
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

  // Fetch last numeric value per dataset (latest timestamp)
  QSqlQuery lastQ(db);
  lastQ.prepare(
    "SELECT unique_id, final_numeric_value FROM readings "
    "WHERE session_id = ? AND is_numeric = 1 AND timestamp_ns = "
    "  (SELECT MAX(timestamp_ns) FROM readings r2 "
    "   WHERE r2.session_id = readings.session_id AND r2.unique_id = readings.unique_id "
    "   AND r2.is_numeric = 1)");
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

/**
 * @brief Loads decimated numeric time-series for every parameter.
 *
 * Uses per-bucket first/min/max/last decimation — the same algorithm as
 * @c DSP::downsampleMonotonic in the live widgets. The session timeline
 * is divided into equal-time buckets (@p maxSamples / 4) and each bucket
 * emits up to four chronologically-ordered points. That preserves the
 * signal envelope for fast-oscillating data where naive stride sampling
 * would alias to a dense noise band.
 *
 * Non-numeric datasets are skipped. The returned series are sorted by
 * parameter insertion order (matches the columns-table order), so charts
 * line up with the measurement summary.
 */
std::vector<Sessions::DatasetSeries> Sessions::loadChartSeries(QSqlDatabase& db,
                                                               int sessionId,
                                                               int maxSamples)
{
  std::vector<DatasetSeries> out;
  if (!db.isOpen() || sessionId < 0 || maxSamples < 2)
    return out;

  // Enumerate the numeric parameters in their table-declared order
  QSqlQuery colQ(db);
  colQ.prepare("SELECT unique_id, group_title, title, units FROM columns "
               "WHERE session_id = ? ORDER BY column_id ASC");
  colQ.bindValue(0, sessionId);
  if (!colQ.exec())
    return out;

  struct Meta {
    int uid;
    QString group;
    QString title;
    QString units;
  };

  std::vector<Meta> metas;
  while (colQ.next()) {
    Meta m;
    m.uid   = colQ.value(0).toInt();
    m.group = colQ.value(1).toString();
    m.title = colQ.value(2).toString();
    m.units = colQ.value(3).toString();
    metas.push_back(std::move(m));
  }

  // Find the session time span so buckets can be sized in nanoseconds
  QSqlQuery spanQ(db);
  spanQ.prepare("SELECT MIN(timestamp_ns), MAX(timestamp_ns) FROM readings "
                "WHERE session_id = ?");
  spanQ.bindValue(0, sessionId);
  qint64 originNs = 0;
  qint64 endNs    = 0;
  if (spanQ.exec() && spanQ.next()) {
    originNs = spanQ.value(0).toLongLong();
    endNs    = spanQ.value(1).toLongLong();
  }
  const qint64 totalNs = std::max<qint64>(1, endNs - originNs);

  // Four points per bucket — divide the point budget to size the bucket count
  const int buckets = std::max(2, maxSamples / 4);

  // One pass per parameter: fetch every numeric sample, walk buckets in order
  for (const auto& m : metas) {
    QSqlQuery sQ(db);
    sQ.setForwardOnly(true);
    sQ.prepare("SELECT timestamp_ns, final_numeric_value FROM readings "
               "WHERE session_id = ? AND unique_id = ? AND is_numeric = 1 "
               "ORDER BY timestamp_ns");
    sQ.bindValue(0, sessionId);
    sQ.bindValue(1, m.uid);
    if (!sQ.exec())
      continue;

    // Per-bucket accumulator — first/last define the polyline spine,
    // min/max capture the envelope
    struct Bucket {
      bool hasData;
      double firstV, lastV;
      double minV, maxV;
      qint64 firstTs, lastTs;
      qint64 minTs, maxTs;
    };

    std::vector<Bucket> bins(static_cast<std::size_t>(buckets),
                             Bucket{false, 0.0, 0.0, 0.0, 0.0, 0, 0, 0, 0});

    qint64 sampleCount = 0;
    double globalMin   = std::numeric_limits<double>::infinity();
    double globalMax   = -std::numeric_limits<double>::infinity();

    while (sQ.next()) {
      const qint64 ts  = sQ.value(0).toLongLong();
      const double val = sQ.value(1).toDouble();
      if (!std::isfinite(val))
        continue;

      // Map timestamp to a bucket index, clamped to the valid range
      const qint64 rel = std::clamp<qint64>(ts - originNs, 0, totalNs);
      int idx          = static_cast<int>((rel * buckets) / std::max<qint64>(totalNs, 1));
      if (idx >= buckets)
        idx = buckets - 1;

      Bucket& b = bins[static_cast<std::size_t>(idx)];
      if (!b.hasData) {
        b.hasData = true;
        b.firstV = b.lastV = b.minV = b.maxV = val;
        b.firstTs = b.lastTs = b.minTs = b.maxTs = ts;
      } else {
        // Readings come in timestamp order so "last seen" is the latest
        b.lastV  = val;
        b.lastTs = ts;
        if (val < b.minV) {
          b.minV  = val;
          b.minTs = ts;
        }
        if (val > b.maxV) {
          b.maxV  = val;
          b.maxTs = ts;
        }
      }

      globalMin = std::min(globalMin, val);
      globalMax = std::max(globalMax, val);
      ++sampleCount;
    }

    if (sampleCount < 2)
      continue;

    DatasetSeries series;
    series.uniqueId = m.uid;
    series.group    = m.group;
    series.title    = m.title;
    series.units    = m.units;
    series.minValue = globalMin;
    series.maxValue = globalMax;
    series.timesSec.reserve(static_cast<std::size_t>(buckets * 4));
    series.values.reserve(static_cast<std::size_t>(buckets * 4));

    // Flatten buckets in chronological order, de-duped by timestamp
    auto emitPoint = [&](qint64 ts, double v) {
      series.timesSec.push_back(static_cast<double>(ts - originNs) / 1.0e9);
      series.values.push_back(v);
    };

    for (const auto& b : bins) {
      if (!b.hasData)
        continue;

      // Sort the four candidates by timestamp before emission
      struct Pt {
        qint64 ts;
        double v;
      };

      Pt pts[4] = {
        {b.firstTs, b.firstV},
        {  b.minTs,   b.minV},
        {  b.maxTs,   b.maxV},
        { b.lastTs,  b.lastV},
      };
      std::sort(pts, pts + 4, [](const Pt& a, const Pt& c) { return a.ts < c.ts; });

      qint64 lastTs = std::numeric_limits<qint64>::min();
      for (const Pt& p : pts) {
        if (p.ts == lastTs)
          continue;
        emitPoint(p.ts, p.v);
        lastTs = p.ts;
      }
    }

    // Drop parameters that produced no finite samples after decimation
    if (series.values.size() < 2)
      continue;

    out.push_back(std::move(series));
  }

  return out;
}

#endif  // BUILD_COMMERCIAL
