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

#include "Sessions/BlockReader.h"

// Spec 0055 R8: an archive recorded before the storage was unified must still open and decode.
// The build under test no longer writes that layout, so this cannot be checked against a fixture
// it produces itself -- these two are frozen 4.0.3 captures, documented in
// tests/fixtures/sessions/README.md. What they pin is the pair of things a wrong answer would
// silently corrupt: the per-session storage discriminator, and the legacy decode behind it.

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a fixture through a private connection on a writable copy. SQLite touches the file
 *        even for reads (journal/WAL), so the checked-in fixture is never opened in place.
 */
class FixtureDb {
public:
  explicit FixtureDb(const QString& fileName, const QString& connection) : m_connection(connection)
  {
    const QString source = QStringLiteral(SS_FIXTURE_DIR "/%1").arg(fileName);
    m_copy               = m_dir.filePath(fileName);
    m_copied             = QFile::copy(source, m_copy);

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connection);
    m_db.setDatabaseName(m_copy);
    m_opened = m_db.open();
  }

  ~FixtureDb()
  {
    if (m_db.isOpen())
      m_db.close();

    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(m_connection);
  }

  FixtureDb(FixtureDb&&)                 = delete;
  FixtureDb(const FixtureDb&)            = delete;
  FixtureDb& operator=(FixtureDb&&)      = delete;
  FixtureDb& operator=(const FixtureDb&) = delete;

  [[nodiscard]] bool ready() const noexcept { return m_copied && m_opened; }

  [[nodiscard]] QSqlDatabase& db() noexcept { return m_db; }

  [[nodiscard]] int userVersion()
  {
    QSqlQuery q(m_db);
    return (q.exec(QStringLiteral("PRAGMA user_version")) && q.next()) ? q.value(0).toInt() : -1;
  }

  [[nodiscard]] int scalar(const QString& sql)
  {
    QSqlQuery q(m_db);
    return (q.exec(sql) && q.next()) ? q.value(0).toInt() : -1;
  }

private:
  QTemporaryDir m_dir;
  QString m_connection;
  QString m_copy;
  bool m_copied = false;
  bool m_opened = false;
  QSqlDatabase m_db;
};

//--------------------------------------------------------------------------------------------------
// Test suite
//--------------------------------------------------------------------------------------------------

class SessionsLegacyArchiveTest : public QObject {
  Q_OBJECT

private slots:
  void v1ArchiveOpensAtItsRecordedVersion();
  void v2ArchiveOpensAtItsRecordedVersion();
  void legacyArchivesAreNotTreatedAsBlockBacked();
  void legacyReadingsRowsAreIntact();
  void legacyStreamBlocksStillDecode();
};

/**
 * @brief The frame-lane fixture is a v1 archive with its readings and raw bytes intact.
 */
void SessionsLegacyArchiveTest::v1ArchiveOpensAtItsRecordedVersion()
{
  FixtureDb fixture(QStringLiteral("legacy_v1_readings.db"), QStringLiteral("ss_legacy_v1_open"));
  QVERIFY2(fixture.ready(), "legacy v1 fixture could not be opened");

  QCOMPARE(fixture.userVersion(), 1);
  QCOMPARE(fixture.scalar(QStringLiteral("SELECT COUNT(*) FROM sessions")), 1);
  QCOMPARE(fixture.scalar(QStringLiteral("SELECT COUNT(*) FROM readings")), 20);
  QCOMPARE(fixture.scalar(QStringLiteral("SELECT COUNT(*) FROM raw_bytes")), 20);
}

/**
 * @brief The dense-lane fixture is a v2 archive whose samples live in stream_blocks.
 */
void SessionsLegacyArchiveTest::v2ArchiveOpensAtItsRecordedVersion()
{
  FixtureDb fixture(QStringLiteral("legacy_v2_stream.db"), QStringLiteral("ss_legacy_v2_open"));
  QVERIFY2(fixture.ready(), "legacy v2 fixture could not be opened");

  QCOMPARE(fixture.userVersion(), 2);
  QCOMPARE(fixture.scalar(QStringLiteral("SELECT COUNT(*) FROM sessions")), 1);
  QCOMPARE(fixture.scalar(QStringLiteral("SELECT COUNT(*) FROM stream_blocks")), 15);
}

/**
 * @brief Neither archive is reported as block-backed, so every reader takes its legacy path. This
 *        is the load-bearing assertion: the discriminator is deliberately NOT PRAGMA user_version,
 *        because opening a legacy archive with a current build migrates its schema and would flip
 *        a version check while the samples still sit in the old tables.
 */
void SessionsLegacyArchiveTest::legacyArchivesAreNotTreatedAsBlockBacked()
{
  FixtureDb v1(QStringLiteral("legacy_v1_readings.db"), QStringLiteral("ss_legacy_v1_probe"));
  FixtureDb v2(QStringLiteral("legacy_v2_stream.db"), QStringLiteral("ss_legacy_v2_probe"));
  QVERIFY(v1.ready() && v2.ready());

  QVERIFY(!Sessions::sessionUsesBlocks(v1.db(), 1));
  QVERIFY(!Sessions::sessionUsesBlocks(v2.db(), 3));

  QSqlQuery migrate(v1.db());
  QVERIFY(migrate.exec(QStringLiteral("PRAGMA user_version = 3")));
  QVERIFY2(!Sessions::sessionUsesBlocks(v1.db(), 1),
           "a migrated schema version must not move an already-recorded session's storage");
}

/**
 * @brief The v1 archive's readings still carry their recorded values, raw twins and numeric flag.
 */
void SessionsLegacyArchiveTest::legacyReadingsRowsAreIntact()
{
  FixtureDb fixture(QStringLiteral("legacy_v1_readings.db"), QStringLiteral("ss_legacy_v1_rows"));
  QVERIFY(fixture.ready());

  QSqlQuery q(fixture.db());
  QVERIFY(q.exec(QStringLiteral("SELECT timestamp_ns, unique_id, raw_numeric_value, "
                                "final_numeric_value, is_numeric FROM readings "
                                "ORDER BY reading_id")));

  int rows        = 0;
  qint64 previous = -1;
  while (q.next()) {
    const qint64 ts = q.value(0).toLongLong();
    QCOMPARE(q.value(1).toInt(), 2);
    QVERIFY(q.value(4).toInt() != 0);
    QVERIFY(ts > previous);
    previous = ts;
    ++rows;
  }

  QCOMPARE(rows, 20);
}

/**
 * @brief The v2 archive's stream blobs still decode with the shipped codec, at the frame count
 *        each row declares. A blob that decoded short would shift every later sample onto the
 *        wrong timestamp, which is the failure this pins.
 */
void SessionsLegacyArchiveTest::legacyStreamBlocksStillDecode()
{
  FixtureDb fixture(QStringLiteral("legacy_v2_stream.db"), QStringLiteral("ss_legacy_v2_rows"));
  QVERIFY(fixture.ready());

  QSqlQuery q(fixture.db());
  QVERIFY(q.exec(QStringLiteral("SELECT frames, t0_ns, dt_ns, samples FROM stream_blocks "
                                "ORDER BY stream_block_id")));

  int blocks     = 0;
  qint64 samples = 0;
  while (q.next()) {
    const qint64 frames = q.value(0).toLongLong();
    const qint64 dtNs   = q.value(2).toLongLong();

    std::vector<double> decoded;
    QVERIFY2(Sessions::unpackStreamSamples(q.value(3).toByteArray(), frames, decoded),
             "a recorded stream blob failed to decode at its declared frame count");
    QCOMPARE(static_cast<qint64>(decoded.size()), frames);

    std::vector<qint64> stamps;
    QVERIFY(
      Sessions::expandBlockTimes(q.value(1).toLongLong(), dtNs, frames, QByteArray(), stamps));
    QCOMPARE(static_cast<qint64>(stamps.size()), frames);

    ++blocks;
    samples += frames;
  }

  QCOMPARE(blocks, 15);
  QCOMPARE(samples, static_cast<qint64>(7200));
}

QTEST_MAIN(SessionsLegacyArchiveTest)

#include "tst_sessions_legacy_archive.moc"
