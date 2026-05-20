/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru
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
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "Misc/PasswordHash.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QPasswordDigestor>
#include <QRandomGenerator>
#include <QStringList>

static constexpr int kSaltLen      = 16;
static constexpr int kKeyLen       = 32;
static constexpr int kLegacyHexLen = 32;

/**
 * @brief Constant-time byte-wise equality check that does not short-circuit.
 */
static bool constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
  if (a.size() != b.size())
    return false;

  unsigned char diff = 0;
  const int n        = a.size();
  for (int i = 0; i < n; ++i)
    diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);

  return diff == 0;
}

/**
 * @brief Generates kSaltLen bytes of cryptographically strong randomness.
 */
static QByteArray randomSalt()
{
  QByteArray salt(kSaltLen, Qt::Uninitialized);
  QRandomGenerator::system()->generate(salt.begin(), salt.end());
  return salt;
}

/**
 * @brief Standard PBKDF2-SHA256 derivation against a known salt and iteration count.
 */
static QByteArray derivePbkdf2(const QByteArray& password, const QByteArray& salt, int iterations)
{
  return QPasswordDigestor::deriveKeyPbkdf2(
    QCryptographicHash::Sha256, password, salt, iterations, kKeyLen);
}

/**
 * @brief Hashes @p plain with PBKDF2-SHA256 and returns a PHC-style `pbkdf2-sha256$iter$salt$hash`.
 */
QString Misc::PasswordHash::hashPassword(const QString& plain)
{
  const auto salt    = randomSalt();
  const auto derived = derivePbkdf2(plain.toUtf8(), salt, kPbkdf2Iterations);

  return QStringLiteral("pbkdf2-sha256$%1$%2$%3")
    .arg(kPbkdf2Iterations)
    .arg(QString::fromLatin1(salt.toBase64(QByteArray::OmitTrailingEquals)))
    .arg(QString::fromLatin1(derived.toBase64(QByteArray::OmitTrailingEquals)));
}

/**
 * @brief Verifies @p plain against @p storedHash, accepting both the PBKDF2 format and legacy MD5.
 */
bool Misc::PasswordHash::verifyPassword(const QString& plain, const QString& storedHash)
{
  if (storedHash.isEmpty())
    return false;

  if (isLegacyHash(storedHash)) {
    const auto computed = QCryptographicHash::hash(plain.toUtf8(), QCryptographicHash::Md5).toHex();
    return constantTimeEquals(computed, storedHash.toLatin1());
  }

  const auto parts = storedHash.split(QLatin1Char('$'));
  if (parts.size() != 4)
    return false;

  if (parts.at(0) != QLatin1String("pbkdf2-sha256"))
    return false;

  bool ok        = false;
  const int iter = parts.at(1).toInt(&ok);
  if (!ok || iter <= 0 || iter > 10000000)
    return false;

  const auto salt = QByteArray::fromBase64(parts.at(2).toLatin1());
  const auto hash = QByteArray::fromBase64(parts.at(3).toLatin1());
  if (salt.isEmpty() || hash.size() != kKeyLen)
    return false;

  const auto computed = derivePbkdf2(plain.toUtf8(), salt, iter);
  return constantTimeEquals(computed, hash);
}

/**
 * @brief Returns true when @p storedHash is the bare 32-hex-char MD5 written by Serial Studio 3.2.
 */
bool Misc::PasswordHash::isLegacyHash(const QString& storedHash)
{
  if (storedHash.size() != kLegacyHexLen)
    return false;

  for (const QChar& c : storedHash) {
    const auto u = c.unicode();
    const bool isHex =
      (u >= u'0' && u <= u'9') || (u >= u'a' && u <= u'f') || (u >= u'A' && u <= u'F');
    if (!isHex)
      return false;
  }

  return true;
}
