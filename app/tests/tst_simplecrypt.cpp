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

#include <QByteArray>
#include <QString>
#include <QTest>

#include "Licensing/SimpleCrypt.h"

// Every test function here is self-contained: each builds its own cipher, so Qt Test's
// declaration-order execution is never load-bearing.

/**
 * @brief Round-trip and tamper behaviour of the obfuscation the license blobs and the provider
 *        keys are stored under. This is characterisation, not a security claim: the store is
 *        obfuscated, not encrypted, and the tests say exactly that (spec 0075, M11/K6).
 */
class TstSimpleCrypt : public QObject {
  Q_OBJECT

private slots:
  void stringRoundTripsWithTheSameKey();
  void byteArrayRoundTripsWithTheSameKey();
  void wrongKeyDoesNotReturnThePlaintext();
  void tamperedCiphertextFailsTheIntegrityCheck();
  void missingKeyIsReported();
  void repeatedEncryptionProducesDifferentCiphertext();
};

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a cipher configured the way every caller in the tree configures it.
 */
static Licensing::SimpleCrypt makeCrypt(quint64 key)
{
  Licensing::SimpleCrypt crypt(key);
  crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
  return crypt;
}

//--------------------------------------------------------------------------------------------------
// Round trips
//--------------------------------------------------------------------------------------------------

/**
 * @brief A string survives the round trip byte for byte.
 */
void TstSimpleCrypt::stringRoundTripsWithTheSameKey()
{
  auto crypt           = makeCrypt(0x0123456789ABCDEFULL);
  const auto plaintext = QStringLiteral("sk-test-key-with-unicode-äöü");

  const auto cipher = crypt.encryptToString(plaintext);
  QVERIFY(!cipher.isEmpty());
  QVERIFY(!cipher.contains(QStringLiteral("sk-test")));

  auto reader = makeCrypt(0x0123456789ABCDEFULL);
  QCOMPARE(reader.decryptToString(cipher), plaintext);
  QCOMPARE(reader.lastError(), Licensing::SimpleCrypt::ErrorNoError);
}

/**
 * @brief A byte array (the shape the license blob uses) survives the round trip.
 */
void TstSimpleCrypt::byteArrayRoundTripsWithTheSameKey()
{
  auto crypt                 = makeCrypt(0xFEEDFACECAFEBEEFULL);
  const QByteArray plaintext = QByteArrayLiteral("{\"valid\":true,\"instance\":{\"id\":\"x\"}}");

  const auto cipher = crypt.encryptToString(plaintext);
  auto reader       = makeCrypt(0xFEEDFACECAFEBEEFULL);
  QCOMPARE(reader.decryptToByteArray(cipher), plaintext);
}

//--------------------------------------------------------------------------------------------------
// Failure modes
//--------------------------------------------------------------------------------------------------

/**
 * @brief Another machine's key does not yield the plaintext, which is what makes a copied
 *        settings file useless on a different install.
 */
void TstSimpleCrypt::wrongKeyDoesNotReturnThePlaintext()
{
  auto writer       = makeCrypt(0x1111111111111111ULL);
  const auto cipher = writer.encryptToString(QStringLiteral("secret-value"));

  auto reader = makeCrypt(0x2222222222222222ULL);
  QVERIFY(reader.decryptToString(cipher) != QStringLiteral("secret-value"));
}

/**
 * @brief A modified blob is rejected rather than decoded into garbage the caller trusts.
 */
void TstSimpleCrypt::tamperedCiphertextFailsTheIntegrityCheck()
{
  auto writer = makeCrypt(0x3333333333333333ULL);
  auto cipher = writer.encryptToString(QStringLiteral("license-blob"));
  QVERIFY(cipher.size() > 8);

  const auto flipped =
    cipher.at(cipher.size() - 2) == QLatin1Char('A') ? QLatin1Char('B') : QLatin1Char('A');
  cipher.replace(cipher.size() - 2, 1, flipped);

  auto reader    = makeCrypt(0x3333333333333333ULL);
  const auto out = reader.decryptToString(cipher);
  QVERIFY(out != QStringLiteral("license-blob"));
  QVERIFY(reader.lastError() != Licensing::SimpleCrypt::ErrorNoError || out.isEmpty());
}

/**
 * @brief A cipher with no key refuses to encrypt instead of writing recoverable plaintext.
 */
void TstSimpleCrypt::missingKeyIsReported()
{
  Licensing::SimpleCrypt crypt;
  QVERIFY(!crypt.hasKey());

  const auto cipher = crypt.encryptToString(QStringLiteral("value"));
  QVERIFY(cipher.isEmpty());
  QCOMPARE(crypt.lastError(), Licensing::SimpleCrypt::ErrorNoKeySet);
}

/**
 * @brief Two encryptions of the same plaintext differ, so a settings file does not reveal that
 *        two providers share a key.
 */
void TstSimpleCrypt::repeatedEncryptionProducesDifferentCiphertext()
{
  auto crypt        = makeCrypt(0x4444444444444444ULL);
  const auto first  = crypt.encryptToString(QStringLiteral("same-input"));
  const auto second = crypt.encryptToString(QStringLiteral("same-input"));

  QVERIFY(!first.isEmpty());
  QVERIFY(first != second);
  QCOMPARE(crypt.decryptToString(first), crypt.decryptToString(second));
}

QTEST_APPLESS_MAIN(TstSimpleCrypt)

#include "tst_simplecrypt.moc"
