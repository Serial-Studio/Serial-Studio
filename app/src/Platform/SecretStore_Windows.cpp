/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary features of Serial Studio and is
 * licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form is permitted
 * only under the terms of a valid Serial Studio Commercial License obtained
 * from the author.
 *
 * This file must not be used or included in builds distributed under the
 * GNU General Public License (GPL) unless explicitly permitted by a
 * commercial agreement.
 *
 * For details, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include <dpapi.h>
#  include <wincrypt.h>
#  include <windows.h>

#  include <QByteArray>
#  include <QSettings>

#  include "Platform/SecretStore.h"

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Builds the QSettings path that persists the DPAPI blob for one secret.
 */
static QString blobPath(const QString& service, const QString& account)
{
  return QStringLiteral("secrets/%1/%2").arg(service, account);
}

//--------------------------------------------------------------------------------------------------
// SecretStore (Windows DPAPI, user scope)
//--------------------------------------------------------------------------------------------------

/**
 * @brief DPAPI is always present on supported Windows versions.
 */
bool Platform::SecretStore::available()
{
  return true;
}

/**
 * @brief Encrypts the secret with the current user's DPAPI key and persists the blob.
 */
bool Platform::SecretStore::store(const QString& service,
                                  const QString& account,
                                  const QString& secret)
{
  QByteArray plain = secret.toUtf8();

  DATA_BLOB in;
  in.pbData = reinterpret_cast<BYTE*>(plain.data());
  in.cbData = static_cast<DWORD>(plain.size());

  DATA_BLOB out{};
  const auto ok = CryptProtectData(
    &in, L"Serial Studio API key", nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out);
  SecureZeroMemory(plain.data(), static_cast<size_t>(plain.size()));
  if (!ok)
    return false;

  const QByteArray blob(reinterpret_cast<const char*>(out.pbData),
                        static_cast<qsizetype>(out.cbData));
  LocalFree(out.pbData);

  QSettings settings;
  settings.setValue(blobPath(service, account), blob.toBase64());
  return true;
}

/**
 * @brief Reads and DPAPI-decrypts the persisted blob for the given secret.
 */
std::optional<QString> Platform::SecretStore::retrieve(const QString& service,
                                                       const QString& account)
{
  QSettings settings;
  const auto raw  = settings.value(blobPath(service, account)).toByteArray();
  QByteArray blob = QByteArray::fromBase64(raw);
  if (blob.isEmpty())
    return std::nullopt;

  DATA_BLOB in;
  in.pbData = reinterpret_cast<BYTE*>(blob.data());
  in.cbData = static_cast<DWORD>(blob.size());

  DATA_BLOB out{};
  if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &out))
    return std::nullopt;

  QString secret = QString::fromUtf8(reinterpret_cast<const char*>(out.pbData),
                                     static_cast<qsizetype>(out.cbData));
  SecureZeroMemory(out.pbData, out.cbData);
  LocalFree(out.pbData);
  return secret;
}

/**
 * @brief Removes the persisted blob for the given secret.
 */
bool Platform::SecretStore::remove(const QString& service, const QString& account)
{
  QSettings settings;
  settings.remove(blobPath(service, account));
  return true;
}

#endif  // BUILD_COMMERCIAL
