/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "MQTT/CredentialVault.h"

#  include <QChar>
#  include <QCryptographicHash>
#  include <QLoggingCategory>

#  include "Licensing/MachineID.h"

Q_LOGGING_CATEGORY(lcMqttVault, "serialstudio.mqtt.vault", QtCriticalMsg)

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures SimpleCrypt with the per-machine key and HMAC integrity.
 */
MQTT::CredentialVault::CredentialVault()
{
  m_simpleCrypt.setKey(Licensing::MachineID::instance().machineSpecificKey());
  m_simpleCrypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns decrypted credentials for host:port, or empty strings on miss / decrypt failure.
 */
MQTT::Credentials MQTT::CredentialVault::credentials(const QString& host, quint16 port) const
{
  Credentials out;
  if (host.isEmpty())
    return out;

  const auto key = settingsKey(host, port);
  m_settings.beginGroup(QStringLiteral("mqtt/credentials"));
  const auto userCipher = m_settings.value(key + QStringLiteral("/user")).toString();
  const auto passCipher = m_settings.value(key + QStringLiteral("/pass")).toString();
  m_settings.endGroup();

  if (!userCipher.isEmpty()) {
    auto plain = m_simpleCrypt.decryptToString(userCipher);
    if (m_simpleCrypt.lastError() == Licensing::SimpleCrypt::ErrorNoError)
      out.username = plain;
    else
      qCWarning(lcMqttVault) << "User decrypt failed for" << key;
  }

  if (!passCipher.isEmpty()) {
    auto plain = m_simpleCrypt.decryptToString(passCipher);
    if (m_simpleCrypt.lastError() == Licensing::SimpleCrypt::ErrorNoError)
      out.password = plain;
    else
      qCWarning(lcMqttVault) << "Pass decrypt failed for" << key;
  }

  return out;
}

/**
 * @brief Returns true when either a username or a password is stored for host:port.
 */
bool MQTT::CredentialVault::hasCredentials(const QString& host, quint16 port) const
{
  if (host.isEmpty())
    return false;

  const auto key = settingsKey(host, port);
  m_settings.beginGroup(QStringLiteral("mqtt/credentials"));
  const bool present = m_settings.contains(key + QStringLiteral("/user"))
                    || m_settings.contains(key + QStringLiteral("/pass"));
  m_settings.endGroup();

  return present;
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Encrypts and writes user/pass for host:port; empty inputs clear their respective slot.
 */
void MQTT::CredentialVault::setCredentials(const QString& host,
                                           quint16 port,
                                           const QString& username,
                                           const QString& password)
{
  if (host.isEmpty())
    return;

  const auto key = settingsKey(host, port);
  m_settings.beginGroup(QStringLiteral("mqtt/credentials"));

  if (username.isEmpty())
    m_settings.remove(key + QStringLiteral("/user"));
  else
    m_settings.setValue(key + QStringLiteral("/user"), m_simpleCrypt.encryptToString(username));

  if (password.isEmpty())
    m_settings.remove(key + QStringLiteral("/pass"));
  else
    m_settings.setValue(key + QStringLiteral("/pass"), m_simpleCrypt.encryptToString(password));

  if (username.isEmpty() && password.isEmpty())
    m_settings.remove(key);

  m_settings.endGroup();
}

/**
 * @brief Removes any stored credentials for host:port.
 */
void MQTT::CredentialVault::clear(const QString& host, quint16 port)
{
  if (host.isEmpty())
    return;

  m_settings.beginGroup(QStringLiteral("mqtt/credentials"));
  m_settings.remove(settingsKey(host, port));
  m_settings.endGroup();
}

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns a stable, filesystem-safe hash of host:port for use as a QSettings sub-key.
 */
QString MQTT::CredentialVault::settingsKey(const QString& host, quint16 port)
{
  const auto endpoint = host.trimmed().toLower() + QChar(':') + QString::number(port);
  const auto digest = QCryptographicHash::hash(endpoint.toUtf8(), QCryptographicHash::Sha1).toHex();
  return QString::fromLatin1(digest);
}

#endif  // BUILD_COMMERCIAL
