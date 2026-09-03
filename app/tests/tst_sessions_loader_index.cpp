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

#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QTest>

#include "Sessions/PlayerLoaderWorker.h"
#include "Sessions/StreamBlockCodec.h"

// The session player's index load (spec 0075 B7, B15). The `blocks` table holds ONE ROW PER DATASET
// per block, every row repeating that block's times blob, so expanding the blob per row produced
// datasets-times as many timestamps as the recording has instants -- 1.8 GB transient for a
// 635-dataset hour at 100 Hz -- before sort/unique threw the copies away again. And the loader
// opened the archive read-write and forced journal_mode=WAL, which is itself a write, so a
// recording on read-only media could not be opened at all.

using Sessions::PlayerLoaderWorker;
using Sessions::PlayerSessionPayloadPtr;

namespace {

/**
 * @brief Minimal archive with the tables the index load reads: one session, @p datasets column
 *        definitions, and one irregular block per entry of @p blockTimes replicated across every
 *        dataset exactly as the exporter writes it. Blocks start at t0 0, so the packed offsets
 *        are the instants themselves.
 */
[[nodiscard]] bool writeArchive(const QString& path,
                                int datasets,
                                const std::vector<std::vector<qint64>>& blockTimes)
{
  const QString connection = QStringLiteral("tst_loader_index_writer");
  bool ok                  = true;
  {
    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection);
    db.setDatabaseName(path);
    if (!db.open())
      return false;

    QSqlQuery q(db);
    ok = ok
      && q.exec(QStringLiteral("CREATE TABLE sessions (session_id INTEGER PRIMARY KEY, "
                               "started_at TEXT, project_json TEXT, view_state TEXT)"));
    ok = ok
      && q.exec(QStringLiteral("CREATE TABLE columns (column_id INTEGER PRIMARY KEY, "
                               "session_id INTEGER, unique_id INTEGER)"));
    ok = ok
      && q.exec(QStringLiteral("CREATE TABLE blocks (block_id INTEGER PRIMARY KEY, "
                               "session_id INTEGER, source_id INTEGER, unique_id INTEGER, "
                               "block_number INTEGER, t0_ns INTEGER, dt_ns INTEGER, "
                               "frames INTEGER, times BLOB)"));
    ok = ok
      && q.exec(QStringLiteral("INSERT INTO sessions (session_id, started_at, project_json) "
                               "VALUES (1, '2026-09-01T00:00:00', '{}')"));

    for (int d = 0; d < datasets; ++d) {
      q.prepare(QStringLiteral("INSERT INTO columns (session_id, unique_id) VALUES (1, ?)"));
      q.bindValue(0, d);
      ok = ok && q.exec();
    }

    qint64 blockNumber = 0;
    for (const auto& times : blockTimes) {
      ++blockNumber;
      const QByteArray blob = Sessions::packStreamTimes({times.data(), times.size()});
      for (int d = 0; d < datasets; ++d) {
        q.prepare(QStringLiteral(
          "INSERT INTO blocks (session_id, source_id, unique_id, block_number, t0_ns, dt_ns, "
          "frames, times) VALUES (1, 0, ?, ?, ?, 0, ?, ?)"));
        q.bindValue(0, d);
        q.bindValue(1, blockNumber);
        q.bindValue(2, qint64(0));
        q.bindValue(3, static_cast<qint64>(times.size()));
        q.bindValue(4, blob);
        ok = ok && q.exec();
      }
    }

    db.close();
  }

  QSqlDatabase::removeDatabase(connection);
  return ok;
}

/**
 * @brief Runs the loader synchronously and returns its payload.
 */
[[nodiscard]] PlayerSessionPayloadPtr load(const QString& path)
{
  PlayerLoaderWorker worker;
  PlayerSessionPayloadPtr result;
  QObject::connect(
    &worker,
    &PlayerLoaderWorker::loaded,
    &worker,
    [&result](const PlayerSessionPayloadPtr& payload) { result = payload; },
    Qt::DirectConnection);

  worker.openAndLoad(path, -1);
  return result;
}

}  // namespace

/**
 * @brief Contract of the session player's timestamp index.
 */
class TstSessionsLoaderIndex : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void everyInstantIsIndexedExactlyOnce();
  void theIndexIsAscendingAndDeduplicated();
  void aWideProjectIndexesTheSameInstantsAsANarrowOne();
  void aReadOnlyArchiveStillLoads();

private:
  QTemporaryDir m_dir;
};

/**
 * @brief Skips the whole suite when the SQLite driver is unavailable in this Qt build.
 */
void TstSessionsLoaderIndex::initTestCase()
{
  if (!QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLITE")))
    QSKIP("QSQLITE driver unavailable");

  QVERIFY(m_dir.isValid());
}

/**
 * @brief One index entry per distinct instant, whatever the dataset count: the blob is expanded
 *        once per block, not once per dataset row of that block.
 */
void TstSessionsLoaderIndex::everyInstantIsIndexedExactlyOnce()
{
  const QString path = m_dir.filePath(QStringLiteral("once.db"));
  QVERIFY(writeArchive(path,
                       64,
                       {
                         {10, 20, 30},
                         {40, 50}
  }));

  const auto payload = load(path);
  QVERIFY(payload != nullptr);
  QVERIFY2(payload->ok, qPrintable(payload->error));
  QCOMPARE(payload->timestampsNs, (std::vector<qint64>{10, 20, 30, 40, 50}));
}

/**
 * @brief Blocks written out of order still produce one ascending, duplicate-free index.
 */
void TstSessionsLoaderIndex::theIndexIsAscendingAndDeduplicated()
{
  const QString path = m_dir.filePath(QStringLiteral("order.db"));
  QVERIFY(writeArchive(path,
                       3,
                       {
                         {30, 40},
                         {10, 20},
                         {20, 30}
  }));

  const auto payload = load(path);
  QVERIFY(payload != nullptr);
  QVERIFY2(payload->ok, qPrintable(payload->error));
  QCOMPARE(payload->timestampsNs, (std::vector<qint64>{10, 20, 30, 40}));
}

/**
 * @brief The index is a property of the recording's instants alone. Before the fix a 64-dataset
 *        project expanded every instant 64 times before the sort, which is the memory blow-up.
 */
void TstSessionsLoaderIndex::aWideProjectIndexesTheSameInstantsAsANarrowOne()
{
  const std::vector<std::vector<qint64>> times{
    {1, 2, 3, 4},
    {5, 6, 7, 8}
  };

  const QString narrowPath = m_dir.filePath(QStringLiteral("narrow.db"));
  const QString widePath   = m_dir.filePath(QStringLiteral("wide.db"));
  QVERIFY(writeArchive(narrowPath, 1, times));
  QVERIFY(writeArchive(widePath, 128, times));

  const auto narrow = load(narrowPath);
  const auto wide   = load(widePath);
  QVERIFY(narrow != nullptr);
  QVERIFY(wide != nullptr);
  QVERIFY2(narrow->ok, qPrintable(narrow->error));
  QVERIFY2(wide->ok, qPrintable(wide->error));

  QCOMPARE(wide->timestampsNs, narrow->timestampsNs);
  QCOMPARE(wide->timestampsNs.size(), std::size_t(8));
}

/**
 * @brief An archive on read-only media loads: the index pass opens read-only and never issues the
 *        journal pragma, which is itself a write.
 */
void TstSessionsLoaderIndex::aReadOnlyArchiveStillLoads()
{
  const QString path = m_dir.filePath(QStringLiteral("readonly.db"));
  QVERIFY(writeArchive(path,
                       2,
                       {
                         {7, 8, 9}
  }));
  QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::ReadUser));

  const auto payload = load(path);
  QVERIFY(payload != nullptr);
  QVERIFY2(payload->ok, qPrintable(payload->error));
  QCOMPARE(payload->timestampsNs, (std::vector<qint64>{7, 8, 9}));

  QVERIFY(QFile::setPermissions(
    path, QFileDevice::ReadOwner | QFileDevice::ReadUser | QFileDevice::WriteOwner));
}

QTEST_MAIN(TstSessionsLoaderIndex)

#include "tst_sessions_loader_index.moc"
