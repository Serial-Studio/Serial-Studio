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

// libsecret pulls in glib's gdbusintrospection.h, which has a struct member named
// "signals" -- include it before any Qt header turns "signals" into a macro.
#  if defined(HAVE_LIBSECRET)
#    include <libsecret/secret.h>
#  endif

#  include "Platform/SecretStore.h"

#  if defined(HAVE_LIBSECRET)

#    include <QByteArray>

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Secret Service schema keyed by (service, account) string attributes.
 */
static const SecretSchema* schema()
{
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wmissing-field-initializers"
  static const SecretSchema s = {
    "com.serial-studio.Secrets",
    SECRET_SCHEMA_NONE,
    {{"service", SECRET_SCHEMA_ATTRIBUTE_STRING},
      {"account", SECRET_SCHEMA_ATTRIBUTE_STRING},
      {nullptr, SECRET_SCHEMA_ATTRIBUTE_STRING}}
  };
#    pragma GCC diagnostic pop
  return &s;
}

//--------------------------------------------------------------------------------------------------
// SecretStore (Linux, libsecret / Secret Service)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Probes the Secret Service; false when no keyring daemon answers.
 */
bool Platform::SecretStore::available()
{
  GError* error = nullptr;
  gchar* probe  = secret_password_lookup_sync(
    schema(), nullptr, &error, "service", "__probe__", "account", "__probe__", nullptr);
  if (probe)
    secret_password_free(probe);

  if (error) {
    g_error_free(error);
    return false;
  }

  return true;
}

/**
 * @brief Stores the secret in the default collection under (service, account).
 */
bool Platform::SecretStore::store(const QString& service,
                                  const QString& account,
                                  const QString& secret)
{
  GError* error = nullptr;
  const auto ok = secret_password_store_sync(schema(),
                                             SECRET_COLLECTION_DEFAULT,
                                             "Serial Studio API key",
                                             secret.toUtf8().constData(),
                                             nullptr,
                                             &error,
                                             "service",
                                             service.toUtf8().constData(),
                                             "account",
                                             account.toUtf8().constData(),
                                             nullptr);

  if (error) {
    g_error_free(error);
    return false;
  }

  return ok == TRUE;
}

/**
 * @brief Looks up the secret stored under (service, account), if any.
 */
std::optional<QString> Platform::SecretStore::retrieve(const QString& service,
                                                       const QString& account)
{
  GError* error = nullptr;
  gchar* value  = secret_password_lookup_sync(schema(),
                                             nullptr,
                                             &error,
                                             "service",
                                             service.toUtf8().constData(),
                                             "account",
                                             account.toUtf8().constData(),
                                             nullptr);

  if (error) {
    g_error_free(error);
    return std::nullopt;
  }

  if (!value)
    return std::nullopt;

  const QString secret = QString::fromUtf8(value);
  secret_password_free(value);
  return secret;
}

/**
 * @brief Clears the secret stored under (service, account).
 */
bool Platform::SecretStore::remove(const QString& service, const QString& account)
{
  GError* error = nullptr;
  const auto ok = secret_password_clear_sync(schema(),
                                             nullptr,
                                             &error,
                                             "service",
                                             service.toUtf8().constData(),
                                             "account",
                                             account.toUtf8().constData(),
                                             nullptr);

  if (error) {
    g_error_free(error);
    return false;
  }

  return ok == TRUE;
}

#  else  // !HAVE_LIBSECRET

//--------------------------------------------------------------------------------------------------
// SecretStore (Linux without libsecret) -- forces the caller's encrypted-QSettings fallback
//--------------------------------------------------------------------------------------------------

/**
 * @brief No Secret Service backend was compiled in.
 */
bool Platform::SecretStore::available()
{
  return false;
}

/**
 * @brief No-op: storing is unavailable without libsecret.
 */
bool Platform::SecretStore::store(const QString&, const QString&, const QString&)
{
  return false;
}

/**
 * @brief No-op: retrieval is unavailable without libsecret.
 */
std::optional<QString> Platform::SecretStore::retrieve(const QString&, const QString&)
{
  return std::nullopt;
}

/**
 * @brief No-op: removal is unavailable without libsecret.
 */
bool Platform::SecretStore::remove(const QString&, const QString&)
{
  return false;
}

#  endif  // HAVE_LIBSECRET

#endif  // BUILD_COMMERCIAL
