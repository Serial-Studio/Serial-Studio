/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
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

#include <functional>
#include <QMap>
#include <QVarLengthArray>
#include <QVector>
#include <unordered_map>
#include <vector>

namespace CSV {

/**
 * @brief Hard ceiling on tracked sources: the per-row presence mask is one byte, so a
 *        recording with more sources than this leaves the extras untracked (they still
 *        replay, they just never take part in the sparse backfill).
 */
inline constexpr int kMaxTrackedSources = 8;

/**
 * @brief One replay column's identity: the dataset it carries and the source that owns it.
 *        Column order is the export schema's order, which is what the file's cells follow.
 */
struct ReplayColumnRef {
  int uniqueId = 0;
  int sourceId = 0;
};

/**
 * @brief One backfill target: the row a stale source must be republished from.
 */
struct SourceRowRef {
  int row      = -1;
  int sourceId = 0;
};

/**
 * @brief Column-to-source layout of a multi-source CSV recording: which file cells belong to
 *        which source, the per-source local column order the FrameBuilder replay map needs,
 *        and the per-row source-presence bitmap the sparse backfill scans (spec 0064). The
 *        owner passes the schema columns in and installs the returned replay map itself.
 */
class MultiSourceMap {
public:
  using ColumnMapper    = std::function<int(int)>;
  using ReplayColumnMap = std::unordered_map<int, std::unordered_map<int, int>>;

  explicit MultiSourceMap();

  [[nodiscard]] bool multiSource() const;
  [[nodiscard]] const QVector<int>& bitSourceIds() const;
  [[nodiscard]] const QVector<quint8>& fileColumnSourceBit() const;
  [[nodiscard]] const QMap<int, QVector<int>>& sourceColumnsByIndex() const;
  [[nodiscard]] QVarLengthArray<SourceRowRef, kMaxTrackedSources> staleSources(int playheadRow);

  void clear();
  void resetLastSourceRows();
  void appendRowSourceBits(const QVector<quint8>& bits);
  [[nodiscard]] ReplayColumnMap build(const std::vector<ReplayColumnRef>& columns,
                                      int fileColumnCount,
                                      const ColumnMapper& toFileColumn);

private:
  void assignSourceBits(const std::vector<ReplayColumnRef>& columns,
                        int fileColumnCount,
                        const ColumnMapper& toFileColumn);

private:
  bool m_multiSource;

  QVector<int> m_bitSourceIds;
  QVector<int> m_lastSourceRow;
  QVector<quint8> m_rowSourceBits;
  QVector<quint8> m_fileColumnSourceBit;
  QMap<int, QVector<int>> m_sourceColumnsByIndex;
};

}  // namespace CSV
