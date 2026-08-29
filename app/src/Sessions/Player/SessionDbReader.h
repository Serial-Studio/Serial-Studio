/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2026 Alex Spataru
 *
 * This file is licensed under the Serial Studio Commercial License.
 *
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#pragma once

#ifdef BUILD_COMMERCIAL

#  include <optional>
#  include <QHash>
#  include <QSqlDatabase>
#  include <QSqlQuery>
#  include <QString>
#  include <QVector>
#  include <span>
#  include <vector>

#  include "Sessions/Player/ReplayFrameValues.h"
#  include "Sessions/PlayerLoaderWorker.h"

namespace Sessions {

/**
 * @brief The player's own read connection to a session file and every prepared statement that runs
 *        on it, so a prepared handle can never outlive the connection it was built on. A spec-0055
 *        archive is answered from `blocks`, an older one from `readings`. Main-thread object: a
 *        QSqlDatabase may not be used from a thread other than the one that created it.
 */
class SessionDbReader {
public:
  explicit SessionDbReader();
  ~SessionDbReader();

  SessionDbReader(SessionDbReader&&)                 = delete;
  SessionDbReader(const SessionDbReader&)            = delete;
  SessionDbReader& operator=(SessionDbReader&&)      = delete;
  SessionDbReader& operator=(const SessionDbReader&) = delete;

  [[nodiscard]] bool isOpen() const;
  [[nodiscard]] int sessionId() const noexcept;
  [[nodiscard]] bool usesBlocks() const noexcept;
  [[nodiscard]] bool hasFinalValues() const noexcept;

  [[nodiscard]] bool open(const QString& filePath, int sessionId);
  void close();

  [[nodiscard]] ReplayRowValues readFrameValues(qint64 timestampNs, const ReplayLayout& layout);
  void fillSeekWindow(std::span<const qint64> rowTimes,
                      const QHash<int, qint64>& keyByUid,
                      QHash<qint64, QVector<double>>& series);
  [[nodiscard]] bool fetchStreamSamples(const PlayerStreamBlockIndex& entry,
                                        std::vector<double>& out);

private:
  void detectFinalValueColumns();
  [[nodiscard]] ReplayRowValues readRowFromReadings(qint64 timestampNs, const ReplayLayout& layout);
  [[nodiscard]] ReplayRowValues readRowFromBlocks(qint64 timestampNs, const ReplayLayout& layout);
  void fillSeekWindowFromReadings(std::span<const qint64> rowTimes,
                                  const QHash<int, qint64>& keyByUid,
                                  QHash<qint64, QVector<double>>& series);
  void fillSeekWindowFromBlocks(std::span<const qint64> rowTimes,
                                const QHash<int, qint64>& keyByUid,
                                QHash<qint64, QVector<double>>& series);

private:
  int m_sessionId;
  bool m_usesBlocks;
  bool m_hasFinalValues;
  bool m_seekQueryPrepared;
  bool m_frameQueryPrepared;
  QString m_connectionName;
  std::optional<QSqlDatabase> m_db;
  std::optional<QSqlQuery> m_seekQuery;
  std::optional<QSqlQuery> m_frameQuery;
  std::optional<QSqlQuery> m_denseBlobQuery;
  std::optional<QSqlQuery> m_streamBlobQuery;
};

}  // namespace Sessions

#endif
