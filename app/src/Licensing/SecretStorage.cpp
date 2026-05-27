/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru <https://aspatru.com>
 *
 * This file is part of the proprietary feature set of Serial Studio
 * and is licensed under the Serial Studio Commercial License.
 *
 * Redistribution, modification, or use of this file in any form
 * is permitted only under the terms of a valid commercial license
 * obtained from the author.
 *
 * This file may NOT be used in any build distributed under the
 * GNU General Public License (GPL) unless explicitly authorized
 * by a separate commercial agreement.
 *
 * For license terms, see:
 * https://github.com/Serial-Studio/Serial-Studio/blob/master/LICENSE.md
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "Licensing/SecretStorage.h"

#ifdef BUILD_COMMERCIAL

#  include <QSettings>

#  include "Licensing/MachineID.h"
#  include "Licensing/SimpleCrypt.h"
#  include "Platform/SecretStore.h"

//--------------------------------------------------------------------------------------------------
// Internal helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Logical service name licensing secrets are filed under in the OS keystore.
 */
static QString secretService()
{
  return QStringLiteral("SerialStudioLicensing");
}

/**
 * @brief Builds the OS keystore account name for a (group, key) pair.
 */
static QString vaultAccount(const QString& group, const QString& key)
{
  return QStringLiteral("%1/%2").arg(group, key);
}

/**
 * @brief Process-wide QSettings used for the legacy encrypted store.
 */
static QSettings& legacySettings()
{
  static QSettings settings;
  return settings;
}

/**
 * @brief Process-wide SimpleCrypt configured with the per-machine key.
 */
static Licensing::SimpleCrypt& legacyCrypt()
{
  static Licensing::SimpleCrypt crypt = [] {
    Licensing::SimpleCrypt c(Licensing::MachineID::instance().machineSpecificKey());
    c.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
    return c;
  }();
  return crypt;
}

/**
 * @brief Decrypts and returns the legacy SimpleCrypt value, or empty on miss/failure.
 */
static QString legacyLoad(const QString& group, const QString& key)
{
  auto& settings = legacySettings();
  settings.beginGroup(group);
  const auto cipher = settings.value(key).toString();
  settings.endGroup();

  if (cipher.isEmpty())
    return {};

  auto& crypt      = legacyCrypt();
  const auto plain = crypt.decryptToString(cipher);
  if (crypt.lastError() != Licensing::SimpleCrypt::ErrorNoError)
    return {};

  return plain;
}

/**
 * @brief Encrypts and writes a value to the legacy QSettings store.
 */
static void legacySave(const QString& group, const QString& key, const QString& value)
{
  auto& settings = legacySettings();
  settings.beginGroup(group);
  settings.setValue(key, legacyCrypt().encryptToString(value));
  settings.endGroup();
}

/**
 * @brief Deletes a value from the legacy QSettings store.
 */
static void legacyRemove(const QString& group, const QString& key)
{
  auto& settings = legacySettings();
  settings.beginGroup(group);
  settings.remove(key);
  settings.endGroup();
}

//--------------------------------------------------------------------------------------------------
// SecretStorage
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the secret, preferring the OS keystore over the legacy store.
 */
QString Licensing::SecretStorage::load(const QString& group, const QString& key)
{
  const auto account = vaultAccount(group, key);

  // Preferred path: OS keystore (DPAPI / Keychain / libsecret).
  if (Platform::SecretStore::available()) {
    if (const auto secret = Platform::SecretStore::retrieve(secretService(), account))
      return *secret;

    // Transparent one-time migration off the legacy encrypted-QSettings store.
    const auto legacy = legacyLoad(group, key);
    if (!legacy.isEmpty()) {
      if (Platform::SecretStore::store(secretService(), account, legacy))
        legacyRemove(group, key);

      return legacy;
    }

    return {};
  }

  // Fallback: legacy encrypted QSettings (no OS keystore on this machine).
  return legacyLoad(group, key);
}

/**
 * @brief Stores the secret in the OS keystore when available, else the legacy store.
 */
void Licensing::SecretStorage::save(const QString& group, const QString& key, const QString& value)
{
  if (value.isEmpty()) {
    remove(group, key);
    return;
  }

  const auto account = vaultAccount(group, key);

  // Preferred path: OS keystore. Scrub any legacy blob so no weaker copy lingers.
  if (Platform::SecretStore::available()
      && Platform::SecretStore::store(secretService(), account, value)) {
    legacyRemove(group, key);
    return;
  }

  // Fallback: legacy encrypted QSettings.
  legacySave(group, key, value);
}

/**
 * @brief Removes the secret from both the OS keystore and the legacy store.
 */
void Licensing::SecretStorage::remove(const QString& group, const QString& key)
{
  if (Platform::SecretStore::available())
    (void)Platform::SecretStore::remove(secretService(), vaultAccount(group, key));

  legacyRemove(group, key);
}

#endif  // BUILD_COMMERCIAL
