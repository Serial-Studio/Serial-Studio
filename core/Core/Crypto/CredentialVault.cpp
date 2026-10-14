/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#ifdef BUILD_COMMERCIAL

#  include "Core/Crypto/CredentialVault.h"

#  include <QChar>
#  include <QCryptographicHash>
#  include <QHash>
#  include <QLoggingCategory>
#  include <QMutex>
#  include <QSet>

#  include "Core/Crypto/MachineKey.h"
#  include "Core/Runtime.h"
#  include "Core/SSAssert.h"

Q_LOGGING_CATEGORY(lcMqttVault, "serialstudio.mqtt.vault", QtCriticalMsg)

static QMutex s_overlayMutex;
static QHash<QString, QString> s_overlay;
static QSet<QString> s_packageOwned;
static int s_transientDepth = 0;

static const QString kUserField = QStringLiteral("user");
static const QString kPassField = QStringLiteral("pass");
static const QString kKeyField  = QStringLiteral("keyPass");

//--------------------------------------------------------------------------------------------------
// Construction
//--------------------------------------------------------------------------------------------------

/**
 * @brief Configures SimpleCrypt with the per-machine key and its integrity hash. The key is
 *        derived on this machine from data the binary can reproduce, so the result is obfuscation
 *        with a tamper check, not encryption; the OS keychain is a shelved decision.
 */
MQTT::CredentialVault::CredentialVault(const QString& scope)
  : m_group(scope + QStringLiteral("/credentials"))
{
  m_simpleCrypt.setKey(Core::Crypto::machineKey());
  m_simpleCrypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);
}

//--------------------------------------------------------------------------------------------------
// Transient scope (spec 0096)
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens a transient scope; only a sealed session may open one.
 */
MQTT::CredentialVault::TransientScope::TransientScope()
{
  SS_ASSERT_LOG(Core::Runtime::sealed());

  const QMutexLocker locker(&s_overlayMutex);
  ++s_transientDepth;
}

/**
 * @brief Closes the scope; slots it wrote stay package-owned for the rest of the session.
 */
MQTT::CredentialVault::TransientScope::~TransientScope()
{
  const QMutexLocker locker(&s_overlayMutex);
  SS_ASSERT(s_transientDepth > 0, return);
  --s_transientDepth;
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the stored credentials for host:port, or empty strings on a miss or a failed
 *        unscramble (a settings file copied from another machine reads as a miss).
 */
MQTT::Credentials MQTT::CredentialVault::credentials(const QString& host, quint16 port) const
{
  Credentials out;
  if (host.isEmpty())
    return out;

  const auto key = settingsKey(host, port);
  out.username   = readField(key, kUserField);
  out.password   = readField(key, kPassField);
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
  if (Core::Runtime::sealed()) {
    const auto prefix = m_group + QLatin1Char('/') + key + QLatin1Char('/');
    const QMutexLocker locker(&s_overlayMutex);
    if (s_packageOwned.contains(prefix + kUserField)
        || s_packageOwned.contains(prefix + kPassField))
      return !s_overlay.value(prefix + kUserField).isEmpty()
          || !s_overlay.value(prefix + kPassField).isEmpty();
  }

  m_settings.beginGroup(m_group);
  const bool present = m_settings.contains(key + QLatin1Char('/') + kUserField)
                    || m_settings.contains(key + QLatin1Char('/') + kPassField);
  m_settings.endGroup();
  return present;
}

/**
 * @brief Returns the decrypted private-key passphrase for host:port, or empty on miss /
 *        decrypt failure.
 */
QString MQTT::CredentialVault::keyPassphrase(const QString& host, quint16 port) const
{
  if (host.isEmpty())
    return QString();

  return readField(settingsKey(host, port), kKeyField);
}

//--------------------------------------------------------------------------------------------------
// Mutations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Obfuscates and writes user/pass for host:port; empty inputs clear their respective slot.
 */
void MQTT::CredentialVault::setCredentials(const QString& host,
                                           quint16 port,
                                           const QString& username,
                                           const QString& password)
{
  if (host.isEmpty())
    return;

  const auto key          = settingsKey(host, port);
  const bool userPersists = writeField(key, kUserField, username);
  const bool passPersists = writeField(key, kPassField, password);
  if (!userPersists || !passPersists || !username.isEmpty() || !password.isEmpty())
    return;

  m_settings.beginGroup(m_group);
  m_settings.remove(key);
  m_settings.endGroup();
}

/**
 * @brief Obfuscates and writes the private-key passphrase for host:port; empty removes the slot.
 */
void MQTT::CredentialVault::setKeyPassphrase(const QString& host,
                                             quint16 port,
                                             const QString& passphrase)
{
  if (host.isEmpty())
    return;

  (void)writeField(settingsKey(host, port), kKeyField, passphrase);
}

/**
 * @brief Removes any stored credentials for host:port. Slots a deployment package owns are dropped
 *        from memory only, so the operator's own stored credentials for the host survive.
 */
void MQTT::CredentialVault::clear(const QString& host, quint16 port)
{
  if (host.isEmpty())
    return;

  const auto key    = settingsKey(host, port);
  const auto prefix = m_group + QLatin1Char('/') + key + QLatin1Char('/');
  if (Core::Runtime::sealed()) {
    const QMutexLocker locker(&s_overlayMutex);
    bool owned = false;
    for (const auto& field : {kUserField, kPassField, kKeyField}) {
      if (!s_packageOwned.contains(prefix + field))
        continue;

      owned = true;
      s_overlay.insert(prefix + field, QString());
    }

    if (owned)
      return;
  }

  m_settings.beginGroup(m_group);
  m_settings.remove(key);
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

/**
 * @brief Reads one field: a package-owned slot answers from memory in a sealed session; otherwise
 *        the settings file is unscrambled as it always was.
 */
QString MQTT::CredentialVault::readField(const QString& key, const QString& field) const
{
  if (Core::Runtime::sealed()) {
    const auto slot = m_group + QLatin1Char('/') + key + QLatin1Char('/') + field;
    const QMutexLocker locker(&s_overlayMutex);
    if (s_packageOwned.contains(slot))
      return s_overlay.value(slot);
  }

  m_settings.beginGroup(m_group);
  const auto cipher = m_settings.value(key + QLatin1Char('/') + field).toString();
  m_settings.endGroup();
  if (cipher.isEmpty())
    return QString();

  auto plain = m_simpleCrypt.decryptToString(cipher);
  if (m_simpleCrypt.lastError() != Licensing::SimpleCrypt::ErrorNoError) {
    qCWarning(lcMqttVault) << "Decrypt failed for" << field << "of" << key;
    return QString();
  }

  return plain;
}

/**
 * @brief Writes one field. Inside a transient scope, or to a slot a package already owns, the
 *        value stays in memory and false is returned; otherwise it is scrambled into the settings
 *        file (empty removes it) and true is returned.
 */
bool MQTT::CredentialVault::writeField(const QString& key,
                                       const QString& field,
                                       const QString& value)
{
  const auto slot = m_group + QLatin1Char('/') + key + QLatin1Char('/') + field;
  if (Core::Runtime::sealed()) {
    const QMutexLocker locker(&s_overlayMutex);
    if (s_transientDepth > 0 || s_packageOwned.contains(slot)) {
      s_packageOwned.insert(slot);
      s_overlay.insert(slot, value);
      return false;
    }
  }

  m_settings.beginGroup(m_group);
  if (value.isEmpty())
    m_settings.remove(key + QLatin1Char('/') + field);
  else
    m_settings.setValue(key + QLatin1Char('/') + field, m_simpleCrypt.encryptToString(value));

  m_settings.endGroup();
  return true;
}

#endif  // BUILD_COMMERCIAL
