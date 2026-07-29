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

#include <QByteArray>
#include <QCryptographicHash>
#include <QString>
#include <QStringList>
#include <QTest>

#include "Misc/PasswordHash.h"

// hashPassword() runs 600000 rounds of PBKDF2-SHA256 by design (OWASP-tuned cost factor), and
// verifyPassword() pays the same cost whenever the stored hash parses into a well-formed PHC
// record. initTestCase() pays that cost exactly once for the shared kPassword/m_hash pair; every
// slot that needs a *different* hash builds it by string surgery on m_hash instead of calling
// hashPassword() again. Only the slots that test the random-salt property or a real
// password-mismatch call hashPassword()/verifyPassword() a second time.

static const QString kPassword = QStringLiteral("Correct Horse Battery Staple!");

//--------------------------------------------------------------------------------------------------
// PHC segment helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Rejoins a 4-element PHC segment list back into a `$`-delimited hash string.
 */
static QString joinPhc(const QStringList& parts)
{
  return parts.join(QLatin1Char('$'));
}

/**
 * @brief Byte-level contract of Misc::PasswordHash: PBKDF2-SHA256 PHC hashing, verification against
 *        both the current format and the legacy MD5 format, and legacy-hash detection.
 */
class TstPasswordHash : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();

  void roundTripAcceptsCorrectPassword();
  void roundTripRejectsWrongPassword();

  void hashFormatIsWellShapedPhc();
  void hashPasswordSaltsEachCallDifferently();

  void isLegacyHash_data();
  void isLegacyHash();

  void verifyPasswordHandlesLegacyMd5();
  void verifyPasswordRejectsEmptyStoredHash();
  void verifyPasswordRejectsEmptyPasswordAgainstRealHash();

  void verifyPasswordRejectsMalformedHash_data();
  void verifyPasswordRejectsMalformedHash();

private:
  QString m_hash;
};

//--------------------------------------------------------------------------------------------------
// Shared fixture
//--------------------------------------------------------------------------------------------------

/**
 * @brief Derives the one shared PBKDF2 hash every other slot reuses instead of re-hashing.
 */
void TstPasswordHash::initTestCase()
{
  m_hash = Misc::PasswordHash::hashPassword(kPassword);
  QVERIFY(!m_hash.isEmpty());
}

//--------------------------------------------------------------------------------------------------
// Round trip
//--------------------------------------------------------------------------------------------------

/**
 * @brief The password that produced m_hash must verify successfully against it.
 */
void TstPasswordHash::roundTripAcceptsCorrectPassword()
{
  QVERIFY(Misc::PasswordHash::verifyPassword(kPassword, m_hash));
}

/**
 * @brief A different password must never verify against someone else's hash.
 */
void TstPasswordHash::roundTripRejectsWrongPassword()
{
  QVERIFY(!Misc::PasswordHash::verifyPassword(kPassword + QStringLiteral("!"), m_hash));
}

//--------------------------------------------------------------------------------------------------
// PHC format shape
//--------------------------------------------------------------------------------------------------

/**
 * @brief hashPassword() emits `pbkdf2-sha256$<iterations>$<salt>$<hash>` with the shipped cost.
 */
void TstPasswordHash::hashFormatIsWellShapedPhc()
{
  const auto parts = m_hash.split(QLatin1Char('$'));

  QCOMPARE(parts.size(), qsizetype(4));
  QCOMPARE(parts.at(0), QStringLiteral("pbkdf2-sha256"));
  QCOMPARE(parts.at(1).toInt(), Misc::PasswordHash::kPbkdf2Iterations);
  QVERIFY(!parts.at(2).isEmpty());
  QVERIFY(!parts.at(3).isEmpty());
}

/**
 * @brief Two hashes of the same plaintext must differ, since each call draws a fresh random salt.
 */
void TstPasswordHash::hashPasswordSaltsEachCallDifferently()
{
  const auto second = Misc::PasswordHash::hashPassword(kPassword);

  QVERIFY(second != m_hash);

  const auto firstSalt  = m_hash.split(QLatin1Char('$')).at(2);
  const auto secondSalt = second.split(QLatin1Char('$')).at(2);
  QVERIFY(firstSalt != secondSalt);
}

//--------------------------------------------------------------------------------------------------
// Legacy MD5 detection
//--------------------------------------------------------------------------------------------------

void TstPasswordHash::isLegacyHash_data()
{
  QTest::addColumn<QString>("candidate");
  QTest::addColumn<bool>("expected");

  const QString hex32 = QStringLiteral("0123456789abcdef0123456789abcdef");
  QCOMPARE(hex32.size(), 32);

  QTest::newRow("32-hex-lowercase") << hex32 << true;
  QTest::newRow("32-hex-mixed-case") << (hex32.left(16).toUpper() + hex32.mid(16)) << true;
  QTest::newRow("31-chars-too-short") << hex32.left(31) << false;
  QTest::newRow("33-chars-too-long") << (hex32 + QStringLiteral("0")) << false;
  QTest::newRow("32-chars-non-hex") << (hex32.left(31) + QStringLiteral("g")) << false;
}

/**
 * @brief isLegacyHash() accepts only the exact 32-hex-char shape written by Serial Studio 3.2.x.
 */
void TstPasswordHash::isLegacyHash()
{
  QFETCH(QString, candidate);
  QFETCH(bool, expected);

  QCOMPARE(Misc::PasswordHash::isLegacyHash(candidate), expected);
}

//--------------------------------------------------------------------------------------------------
// Legacy MD5 verification
//--------------------------------------------------------------------------------------------------

/**
 * @brief verifyPassword() accepts a bare MD5 hex digest for the matching password and rejects it
 *        for a wrong one, without ever touching the PBKDF2 path.
 */
void TstPasswordHash::verifyPasswordHandlesLegacyMd5()
{
  const auto legacyHash = QString::fromLatin1(
    QCryptographicHash::hash(kPassword.toUtf8(), QCryptographicHash::Md5).toHex());

  QVERIFY(Misc::PasswordHash::isLegacyHash(legacyHash));
  QVERIFY(Misc::PasswordHash::verifyPassword(kPassword, legacyHash));
  QVERIFY(!Misc::PasswordHash::verifyPassword(kPassword + QStringLiteral("!"), legacyHash));
}

//--------------------------------------------------------------------------------------------------
// Edge inputs
//--------------------------------------------------------------------------------------------------

/**
 * @brief An empty stored hash never verifies, regardless of the supplied password.
 */
void TstPasswordHash::verifyPasswordRejectsEmptyStoredHash()
{
  QVERIFY(!Misc::PasswordHash::verifyPassword(kPassword, QString()));
  QVERIFY(!Misc::PasswordHash::verifyPassword(QString(), QString()));
}

/**
 * @brief An empty password never verifies against a hash derived from a non-empty one.
 */
void TstPasswordHash::verifyPasswordRejectsEmptyPasswordAgainstRealHash()
{
  QVERIFY(!Misc::PasswordHash::verifyPassword(QString(), m_hash));
}

//--------------------------------------------------------------------------------------------------
// Malformed PHC records
//--------------------------------------------------------------------------------------------------

/**
 * @brief Every malformed variant is built from m_hash's own segments, so none of these rows pays
 *        for a fresh PBKDF2 derivation: the parser rejects each shape before reaching the hash.
 */
void TstPasswordHash::verifyPasswordRejectsMalformedHash_data()
{
  QTest::addColumn<QString>("malformed");

  const auto parts = m_hash.split(QLatin1Char('$'));
  QCOMPARE(parts.size(), qsizetype(4));

  QTest::newRow("wrong-segment-count") << joinPhc(parts.mid(0, 3));

  QStringList wrongTag = parts;
  wrongTag[0]          = QStringLiteral("pbkdf2-sha1");
  QTest::newRow("wrong-tag") << joinPhc(wrongTag);

  QStringList nonNumericIter = parts;
  nonNumericIter[1]          = QStringLiteral("abc");
  QTest::newRow("non-numeric-iterations") << joinPhc(nonNumericIter);

  QStringList zeroIter = parts;
  zeroIter[1]          = QStringLiteral("0");
  QTest::newRow("zero-iterations") << joinPhc(zeroIter);

  QStringList negativeIter = parts;
  negativeIter[1]          = QStringLiteral("-100");
  QTest::newRow("negative-iterations") << joinPhc(negativeIter);

  QStringList oversizedIter = parts;
  oversizedIter[1]          = QStringLiteral("99999999");
  QTest::newRow("oversized-iterations") << joinPhc(oversizedIter);

  QStringList invalidSalt = parts;
  invalidSalt[2]          = QStringLiteral("@@@@@@@@@@@@@@@@");
  QTest::newRow("invalid-base64-salt") << joinPhc(invalidSalt);

  QStringList shortHash = parts;
  shortHash[3] = QString::fromLatin1(QByteArray("short").toBase64(QByteArray::OmitTrailingEquals));
  QTest::newRow("wrong-length-hash") << joinPhc(shortHash);
}

/**
 * @brief verifyPassword() returns false and never crashes on any malformed PHC record.
 */
void TstPasswordHash::verifyPasswordRejectsMalformedHash()
{
  QFETCH(QString, malformed);

  QVERIFY(!Misc::PasswordHash::verifyPassword(kPassword, malformed));
}

QTEST_APPLESS_MAIN(TstPasswordHash)

#include "tst_password_hash.moc"
