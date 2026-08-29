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

#include "API/Server/AuthPrimitives.h"

#include <QRandomGenerator>
#include <QSet>

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

constexpr int kAuthTokenBytes    = 32;
constexpr int kMinAuthTokenChars = 32;

//--------------------------------------------------------------------------------------------------
// Token generation & validation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Generates a cryptographically random hex token for external API auth.
 */
QString API::Auth::generateToken()
{
  QByteArray raw;
  raw.reserve(kAuthTokenBytes);

  auto* rng = QRandomGenerator::system();
  for (int i = 0; i < kAuthTokenBytes / int(sizeof(quint32)); ++i) {
    const quint32 value = rng->generate();
    raw.append(reinterpret_cast<const char*>(&value), sizeof(value));
  }

  return QString::fromLatin1(raw.toHex());
}

/**
 * @brief Normalizes a caller-supplied token, returning an empty string when it is not a hex
 *        credential of at least 32 characters.
 */
QString API::Auth::normalizeToken(const QString& token)
{
  const auto trimmed = token.trimmed().toLower();
  if (trimmed.size() < kMinAuthTokenChars)
    return QString();

  for (const QChar character : trimmed)
    if (!character.isDigit() && (character < QLatin1Char('a') || character > QLatin1Char('f')))
      return QString();

  return trimmed;
}

/**
 * @brief Compares two byte arrays in constant time to avoid token timing side channels.
 */
bool API::Auth::constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
  if (a.size() != b.size())
    return false;

  quint8 diff = 0;
  for (qsizetype i = 0; i < a.size(); ++i)
    diff |= static_cast<quint8>(a[i]) ^ static_cast<quint8>(b[i]);

  return diff == 0;
}

//--------------------------------------------------------------------------------------------------
// Command classification
//--------------------------------------------------------------------------------------------------

/**
 * @brief Whether a command may only be issued by an in-process control script.
 */
bool API::Auth::commandIsControlScriptOnly(const QString& command)
{
  static const QSet<QString> kControlScriptOnlyCommands = {
    QStringLiteral("system.exec"),
    QStringLiteral("system.kill"),
    QStringLiteral("system.runningProcesses"),
  };

  return kControlScriptOnlyCommands.contains(command);
}

/**
 * @brief Whether a command reaches the connected hardware, and therefore has to clear the
 *        device-write consent gate before a remote client may run it.
 */
bool API::Auth::commandWritesToDevice(const QString& command)
{
  static const QSet<QString> kDeviceWriteCommands = {
    QStringLiteral("io.writeData"),
    QStringLiteral("io.ble.writeCharacteristic"),
    QStringLiteral("console.send"),
  };

  return kDeviceWriteCommands.contains(command);
}
