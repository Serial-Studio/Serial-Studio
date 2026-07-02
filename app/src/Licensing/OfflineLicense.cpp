/*
 * Serial Studio - https://serial-studio.com/
 *
 * Copyright (C) 2020-2025 Alex Spataru <https://aspatru.com>
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

#include "Licensing/OfflineLicense.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>

#include "AppInfo.h"
#include "Licensing/CommercialToken.h"
#include "Licensing/LemonSqueezy.h"
#include "Licensing/MachineID.h"
#include "Licensing/MonotonicClock.h"
#include "Licensing/OfflineCertificate.h"
#include "Licensing/OfflinePublicKey.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Offline activation portal (server that mints certificates from a device file)
//--------------------------------------------------------------------------------------------------

static constexpr auto kActivationPortal = "https://serial-studio.com/offline-activation";

/**
 * @brief Shows a localized offline-activation error dialog (no-op when headless).
 */
static void showOfflineError(const QString& text, QMessageBox::Icon icon = QMessageBox::Warning)
{
  Misc::Utilities::showMessageBox(text, QString(), icon, QObject::tr("Offline Activation"));
}

/**
 * @brief Builds and installs a commercial token for a verified offline certificate.
 */
static void installOfflineToken(const Licensing::CertificateFields& fields, int daysRemaining)
{
  Licensing::CommercialToken token;
  token.setVariantName(fields.variant);
  token.setInstanceName(Licensing::MachineID::instance().machineId());
  token.setGraceDaysRemaining(daysRemaining);
  token.setFeatureTier(Licensing::tierFromCertVariant(fields.variant));
  token.seal();
  Licensing::CommercialToken::setCurrent(token);
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the handler, wires activation-signal forwarding, then restores
 * any stored certificate so a startup token install re-triggers device rebuilds.
 */
Licensing::OfflineLicense::OfflineLicense() : m_activated(false)
{
  m_simpleCrypt.setKey(MachineID::instance().machineSpecificKey());
  m_simpleCrypt.setIntegrityProtectionMode(SimpleCrypt::ProtectionHash);

  connect(&LemonSqueezy::instance(),
          &LemonSqueezy::activatedChanged,
          this,
          &OfflineLicense::onOnlineActivationChanged);

  connect(this,
          &OfflineLicense::activatedChanged,
          &LemonSqueezy::instance(),
          &LemonSqueezy::activatedChanged);
  connect(this,
          &OfflineLicense::activatedChanged,
          &LemonSqueezy::instance(),
          &LemonSqueezy::licenseDataChanged);

  readSettings();
}

/**
 * @brief Provides access to the OfflineLicense singleton instance.
 */
Licensing::OfflineLicense& Licensing::OfflineLicense::instance()
{
  static OfflineLicense instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Member access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns this device's fingerprint, the value a certificate must be bound to.
 */
const QString& Licensing::OfflineLicense::machineId() const
{
  return MachineID::instance().machineId();
}

/**
 * @brief Returns the activation portal URL, shown so users can type it elsewhere.
 */
QString Licensing::OfflineLicense::activationUrl() const
{
  return QString::fromUtf8(kActivationPortal);
}

/**
 * @brief Returns whether a valid offline certificate is currently installed.
 */
bool Licensing::OfflineLicense::isActivated() const noexcept
{
  return m_activated;
}

/**
 * @brief Returns whole days remaining before the offline certificate expires.
 */
int Licensing::OfflineLicense::daysRemaining() const
{
  if (!m_activated)
    return 0;

  return qMax(0, static_cast<int>(QDateTime::currentDateTime().daysTo(m_expiresAt)));
}

/**
 * @brief Returns the expiry timestamp of the installed offline certificate.
 */
const QDateTime& Licensing::OfflineLicense::expiresAt() const noexcept
{
  return m_expiresAt;
}

/**
 * @brief Returns the variant name of the installed offline certificate.
 */
const QString& Licensing::OfflineLicense::variantName() const noexcept
{
  return m_variantName;
}

/**
 * @brief Returns the reason for the most recent failed import.
 */
const QString& Licensing::OfflineLicense::lastError() const noexcept
{
  return m_lastError;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Removes the offline certificate from this device after user confirmation;
 * the seat itself can only be released manually by support, so the prompt tells
 * the user to email their device ID or .ssmachine file first.
 */
void Licensing::OfflineLicense::deactivate()
{
  if (!m_activated)
    return;

  const int answer = Misc::Utilities::showMessageBox(
    tr("Remove the offline license from this device?"),
    tr("This deletes the local activation certificate, but it does not free up "
       "your license seat automatically.\n\nTo reuse this seat on another device, "
       "email alex@serial-studio.com with your device ID or your .ssmachine file "
       "so the seat can be released manually."),
    QMessageBox::Question,
    tr("Offline Activation"),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No);
  if (answer != QMessageBox::Yes)
    return;

  if (!LemonSqueezy::instance().isOnlineActivated())
    CommercialToken::clearCurrent();

  clearOfflineLicense();
}

/**
 * @brief Imports and activates a certificate from a local file or file URL.
 */
bool Licensing::OfflineLicense::activateFromFile(const QString& path)
{
  const QString local = path.startsWith(QStringLiteral("file:")) ? QUrl(path).toLocalFile() : path;

  QFile file(local);
  if (!file.open(QIODevice::ReadOnly)) {
    m_lastError = tr("Could not open the certificate file.");
    showOfflineError(m_lastError, QMessageBox::Critical);
    return false;
  }

  constexpr qint64 kMaxCertBytes = 64 * 1024;
  if (file.size() > kMaxCertBytes) {
    m_lastError = tr("The certificate file is too large to be valid.");
    showOfflineError(m_lastError, QMessageBox::Critical);
    return false;
  }

  const QByteArray framed = file.readAll();
  file.close();
  return applyCertificate(framed, true);
}

/**
 * @brief Writes this device's fingerprint and app info to a .ssmachine file for the portal.
 */
bool Licensing::OfflineLicense::exportMachineInfo(const QString& path)
{
  const QString local = path.startsWith(QStringLiteral("file:")) ? QUrl(path).toLocalFile() : path;

  QJsonObject info;
  info.insert(QStringLiteral("type"), QStringLiteral("serial-studio-machine"));
  info.insert(QStringLiteral("version"), 1);
  info.insert(QStringLiteral("machineId"), machineId());
  info.insert(QStringLiteral("app"), QStringLiteral(APP_NAME));
  info.insert(QStringLiteral("appVersion"), QStringLiteral(APP_VERSION));
  info.insert(QStringLiteral("generatedAt"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));

  QFile file(local);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    m_lastError = tr("Could not write the device file.");
    showOfflineError(m_lastError, QMessageBox::Critical);
    return false;
  }

  file.write(QJsonDocument(info).toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

/**
 * @brief Opens the offline activation portal where the device file is exchanged for a license.
 */
void Licensing::OfflineLicense::openActivationPortal()
{
  QDesktopServices::openUrl(QUrl(QString::fromUtf8(kActivationPortal)));
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Restores and re-verifies a stored certificate on startup; a failed restore
 * only drops the in-memory activation, never the stored blob, so a transient
 * fingerprint or clock glitch cannot destroy a certificate that may still be valid.
 */
void Licensing::OfflineLicense::readSettings()
{
  m_settings.beginGroup("offlineLicense");
  const auto stored = m_settings.value("cert", "").toString();
  m_settings.endGroup();

  if (stored.isEmpty())
    return;

  const auto framed = m_simpleCrypt.decryptToByteArray(stored);
  if (framed.isEmpty())
    m_lastError = tr("The stored certificate could not be decrypted on this machine.");
  else if (applyCertificate(framed, false))
    return;

  qWarning() << "[OfflineLicense] stored certificate not restored:" << m_lastError;
  resetActivationState();
}

/**
 * @brief Persists the verified certificate, encrypted, for the next launch.
 */
void Licensing::OfflineLicense::writeSettings(const QByteArray& framedCert)
{
  m_settings.beginGroup("offlineLicense");
  m_settings.setValue("cert", m_simpleCrypt.encryptToString(framedCert));
  m_settings.endGroup();
}

/**
 * @brief Restores the offline token when no valid commercial token is in effect
 * (e.g. an online license cleared it); the guard also blocks a forwarding loop.
 */
void Licensing::OfflineLicense::onOnlineActivationChanged()
{
  if (m_activated && !CommercialToken::current().isValid())
    readSettings();
}

/**
 * @brief Clears the stored certificate and offline activation state (explicit
 * deactivation only; startup restore failures go through resetActivationState).
 */
void Licensing::OfflineLicense::clearOfflineLicense()
{
  m_settings.beginGroup("offlineLicense");
  m_settings.setValue("cert", "");
  m_settings.endGroup();

  resetActivationState();
}

/**
 * @brief Drops the in-memory activation state and notifies listeners.
 */
void Licensing::OfflineLicense::resetActivationState()
{
  m_activated   = false;
  m_variantName = QString();
  m_expiresAt   = QDateTime();

  Q_EMIT activatedChanged();
}

//--------------------------------------------------------------------------------------------------
// Certificate application
//--------------------------------------------------------------------------------------------------

/**
 * @brief Verifies a certificate and, on success, installs the token and persists it.
 */
bool Licensing::OfflineLicense::applyCertificate(const QByteArray& framedCert, bool persist)
{
  const qint64 nowSecs = MonotonicClock::now().toSecsSinceEpoch();

  CertificateFields fields;
  const auto status =
    verifyCertificate(framedCert, kOfflinePublicKey, machineId(), nowSecs, fields);
  if (status != CertStatus::Valid) {
    m_lastError = certStatusReason(status);
    if (persist)
      showOfflineError(m_lastError);

    return false;
  }

  m_activated   = true;
  m_lastError   = QString();
  m_variantName = fields.variant;
  m_expiresAt   = QDateTime::fromSecsSinceEpoch(fields.expiresAt);

  if (!LemonSqueezy::instance().isOnlineActivated()) {
    const int days = qMax(0, static_cast<int>((fields.expiresAt - nowSecs) / 86400));
    installOfflineToken(fields, days);
  }

  if (persist)
    writeSettings(framedCert);

  Q_EMIT activatedChanged();
  return true;
}
