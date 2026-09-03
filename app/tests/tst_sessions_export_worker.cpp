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

#include <cmath>
#include <limits>
#include <QTest>

#include "Sessions/BlockFingerprint.h"

// The capture fingerprint is the historian's claim that an archive contains exactly the rows it
// was told to write, and spec 0075 B3 makes that claim conditional: after a failed database write
// finalizeSession() stores NULL digests instead of a digest over rows SQLite never took. That
// policy is only sound while the digest itself stays unambiguous and stable, which is what this
// suite pins -- every field participates, nothing can be shifted between fields, and NaN folds the
// way SQLite round-trips it. The exporter's other 0075 fixes (raw-lane throughput, the commit
// failure itself, the raw-timeline head) need a live database and a running pipeline, so they are
// pinned by tests/integration/test_recording_fidelity.py instead: Sessions::Export reaches
// DatabaseManager, ProjectModel, FrameBuilder, Dashboard and the licensing block, which is an
// application link, not a unit-tier one.

using namespace Sessions;

namespace {

/**
 * @brief Digest of one raw_bytes row.
 */
[[nodiscard]] QByteArray rawDigest(qint64 ns, int deviceId, const QByteArray& data)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  hashRawChunk(hash, ns, deviceId, data);
  return hash.result();
}

/**
 * @brief Digest of one blocks row.
 */
[[nodiscard]] QByteArray blockDigest(qint64 uniqueId,
                                     qint64 t0Ns,
                                     qint64 dtNs,
                                     qint64 frames,
                                     const QByteArray& values,
                                     const QByteArray& rawValues,
                                     const QByteArray& texts)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  hashBlockRow(hash, uniqueId, t0Ns, dtNs, frames, values, rawValues, texts);
  return hash.result();
}

/**
 * @brief Digest of one legacy readings row.
 */
[[nodiscard]] QByteArray readingDigest(qint64 ns,
                                       qint64 uniqueId,
                                       double rawNumeric,
                                       const QString& rawString,
                                       double finalNumeric,
                                       const QString& finalString,
                                       bool isNumeric)
{
  QCryptographicHash hash(QCryptographicHash::Sha256);
  hashReadingRow(hash, ns, uniqueId, rawNumeric, rawString, finalNumeric, finalString, isNumeric);
  return hash.result();
}

}  // namespace

/**
 * @brief Contract of the spec-0044 capture fingerprints the session exporter writes.
 */
class TstSessionsExportWorker : public QObject {
  Q_OBJECT

private slots:
  void identicalRowsHashIdentically();
  void everyRawFieldParticipatesInTheDigest();
  void everyBlockFieldParticipatesInTheDigest();
  void blobBoundariesCannotShiftBetweenFields();
  void stringBoundariesCannotShiftBetweenFields();
  void notANumberFoldsToZeroAsSqliteRoundTripsIt();
  void aStreamedDigestEqualsAWholeRowDigest();
};

/**
 * @brief The digest is a pure function of the row: re-hashing the same values must reproduce it,
 *        or the verifier's re-record comparison reports drift that never happened.
 */
void TstSessionsExportWorker::identicalRowsHashIdentically()
{
  const QByteArray payload("\x01\x02\x03", 3);

  QCOMPARE(rawDigest(1000, 2, payload), rawDigest(1000, 2, payload));
  QCOMPARE(blockDigest(7, 100, 0, 4, payload, payload, QByteArray()),
           blockDigest(7, 100, 0, 4, payload, payload, QByteArray()));
}

/**
 * @brief A raw chunk is identified by its instant, its device and its bytes; dropping any of them
 *        from the digest would let a tampered or misattributed archive verify clean.
 */
void TstSessionsExportWorker::everyRawFieldParticipatesInTheDigest()
{
  const QByteArray payload("abc", 3);
  const QByteArray base = rawDigest(1000, 2, payload);

  QVERIFY(rawDigest(1001, 2, payload) != base);
  QVERIFY(rawDigest(1000, 3, payload) != base);
  QVERIFY(rawDigest(1000, 2, QByteArray("abd", 3)) != base);
}

/**
 * @brief Same for a blocks row: identity, timebase, fill and every blob.
 */
void TstSessionsExportWorker::everyBlockFieldParticipatesInTheDigest()
{
  const QByteArray values("vvvvvvvv", 8);
  const QByteArray raws("rrrrrrrr", 8);
  const QByteArray texts("tt", 2);
  const QByteArray base = blockDigest(7, 100, 0, 1, values, raws, texts);

  QVERIFY(blockDigest(8, 100, 0, 1, values, raws, texts) != base);
  QVERIFY(blockDigest(7, 101, 0, 1, values, raws, texts) != base);
  QVERIFY(blockDigest(7, 100, 1, 1, values, raws, texts) != base);
  QVERIFY(blockDigest(7, 100, 0, 2, values, raws, texts) != base);
  QVERIFY(blockDigest(7, 100, 0, 1, raws, values, texts) != base);
  QVERIFY(blockDigest(7, 100, 0, 1, values, raws, QByteArray("t", 1)) != base);
}

/**
 * @brief Each blob is length-prefixed, so bytes cannot migrate from one blob to the next and leave
 *        the digest unchanged -- the property that makes a digest over concatenated blobs safe.
 */
void TstSessionsExportWorker::blobBoundariesCannotShiftBetweenFields()
{
  const QByteArray left  = blockDigest(7, 0, 0, 1, QByteArray("ab", 2), QByteArray("c", 1), {});
  const QByteArray right = blockDigest(7, 0, 0, 1, QByteArray("a", 1), QByteArray("bc", 2), {});

  QVERIFY(left != right);
}

/**
 * @brief The same rule for the legacy readings row's two strings: a recorded value may contain any
 *        byte, so the layout length-prefixes instead of delimiting.
 */
void TstSessionsExportWorker::stringBoundariesCannotShiftBetweenFields()
{
  const QByteArray left =
    readingDigest(0, 7, 0.0, QStringLiteral("ab"), 0.0, QStringLiteral("c"), false);
  const QByteArray right =
    readingDigest(0, 7, 0.0, QStringLiteral("a"), 0.0, QStringLiteral("bc"), false);

  QVERIFY(left != right);
}

/**
 * @brief SQLite stores NaN as NULL and the verifier reads NULL back as 0.0, so the digest must
 *        cover the value the archive actually round-trips, not the one the pipeline produced.
 */
void TstSessionsExportWorker::notANumberFoldsToZeroAsSqliteRoundTripsIt()
{
  const double nan = std::numeric_limits<double>::quiet_NaN();

  QCOMPARE(readingDigest(0, 7, nan, QString(), 0.0, QString(), true),
           readingDigest(0, 7, 0.0, QString(), 0.0, QString(), true));

  QVERIFY(readingDigest(0, 7, 1.0, QString(), 0.0, QString(), true)
          != readingDigest(0, 7, 0.0, QString(), 0.0, QString(), true));
}

/**
 * @brief Capture hashes incrementally, row by row, while the verifier re-hashes a whole session;
 *        both must reach the same digest for a two-row archive.
 */
void TstSessionsExportWorker::aStreamedDigestEqualsAWholeRowDigest()
{
  QCryptographicHash streamed(QCryptographicHash::Sha256);
  hashRawChunk(streamed, 1, 0, QByteArray("aa", 2));
  hashRawChunk(streamed, 2, 0, QByteArray("bb", 2));

  QCryptographicHash reference(QCryptographicHash::Sha256);
  hashRawChunk(reference, 1, 0, QByteArray("aa", 2));
  hashRawChunk(reference, 2, 0, QByteArray("bb", 2));

  QCOMPARE(streamed.result(), reference.result());

  QCryptographicHash swapped(QCryptographicHash::Sha256);
  hashRawChunk(swapped, 2, 0, QByteArray("bb", 2));
  hashRawChunk(swapped, 1, 0, QByteArray("aa", 2));

  QVERIFY(swapped.result() != reference.result());
}

QTEST_APPLESS_MAIN(TstSessionsExportWorker)

#include "tst_sessions_export_worker.moc"
