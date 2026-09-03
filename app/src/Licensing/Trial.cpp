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

#include "Trial.h"

#include <QJsonDocument>
#include <QJsonParseError>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>

#include "CommercialToken.h"
#include "LemonSqueezy.h"
#include "MachineID.h"
#include "Misc/Utilities.h"
#include "MonotonicClock.h"

static Licensing::Trial* s_trial = nullptr;

/**
 * @brief Builds and installs a commercial token for an active trial.
 */
static void installTrialToken(int daysRemaining)
{
  static auto& machineId = Licensing::MachineID::instance();

  Licensing::CommercialToken token;
  token.setVariantName(QStringLiteral("Trial"));
  token.setInstanceName(machineId.machineId());
  token.setGraceDaysRemaining(daysRemaining);
  token.setFeatureTier(Licensing::FeatureTier::Trial);
  token.seal();
  Licensing::CommercialToken::setCurrent(token);
}

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructs the Trial licensing system. Trial state loads unconditionally, even when a
 *        cached license restored as activated: reassertTokenIfEntitled() must know the real
 *        entitlement if that license is later revoked mid-session. installTrialToken() stays
 *        gated on trialEnabled(), so loading state on a licensed machine cannot take the slot.
 */
Licensing::Trial::Trial()
  : m_busy(false)
  , m_silentFetch(false)
  , m_trialEnabled(false)
  , m_deviceRegistered(false)
  , m_daysRemaining(0)
  , m_trialExpiry(QDateTime::currentDateTimeUtc())
{
  static auto& lemonSqueezy = Licensing::LemonSqueezy::instance();
  static auto& machineId    = Licensing::MachineID::instance();

  connect(this,
          &Licensing::Trial::enabledChanged,
          &lemonSqueezy,
          &Licensing::LemonSqueezy::notifyEntitlementMaybeChanged);
  connect(&lemonSqueezy,
          &Licensing::LemonSqueezy::licenseDataChanged,
          this,
          &Licensing::Trial::availableChanged);
  connect(&lemonSqueezy,
          &Licensing::LemonSqueezy::licenseDataChanged,
          this,
          &Licensing::Trial::enabledChanged);

  connect(&m_manager, &QNetworkAccessManager::finished, this, &Licensing::Trial::onServerReply);

  m_crypt.setKey(machineId.machineSpecificKey());
  m_crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);

  readSettings();

  s_trial = this;
}

/**
 * @brief Gets the singleton instance of the Trial licensing class.
 */
Licensing::Trial& Licensing::Trial::instance()
{
  static Trial instance;
  return instance;
}

/**
 * @brief Re-installs the trial token after another licensing path cleared the shared token slot,
 *        so a failed validation or an offline deactivation cannot silently strip Pro features
 *        from a machine still in trial. Reads the constructed-instance pointer, not instance():
 *        LemonSqueezy's constructor can reach a clear, which would recurse the Meyers guard.
 */
void Licensing::Trial::reassertTokenIfEntitled()
{
  if (!s_trial || !s_trial->trialEnabled() || CommercialToken::current().isValid())
    return;

  installTrialToken(s_trial->daysRemaining());
}

//--------------------------------------------------------------------------------------------------
// Status queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a trial query operation is currently running.
 */
bool Licensing::Trial::busy() const noexcept
{
  return m_busy;
}

/**
 * @brief Checks whether the user has ever started a trial.
 */
bool Licensing::Trial::firstRun() const
{
  return !m_deviceRegistered && !m_trialEnabled && trialAvailable();
}

/**
 * @brief Checks whether a trial period is currently active.
 */
bool Licensing::Trial::trialEnabled() const
{
  return m_deviceRegistered && m_trialEnabled && trialAvailable() && daysRemaining() > 0;
}

/**
 * @brief Checks whether a trial period expired.
 */
bool Licensing::Trial::trialExpired() const
{
  return m_deviceRegistered && trialAvailable() && (!m_trialEnabled || daysRemaining() <= 0);
}

/**
 * @brief Checks if a trial can be started.
 */
bool Licensing::Trial::trialAvailable() const
{
  static auto& lemonSqueezy = LemonSqueezy::instance();
  return !lemonSqueezy.isActivated();
}

/**
 * @brief Gets the number of days remaining in the active trial. The answer only changes when the
 *        date rolls over or the expiry moves, so it is cached against both: this is a
 *        Q_PROPERTY read, and computing it floors the monotonic clock, which persists to the
 *        settings store (K10).
 */
int Licensing::Trial::daysRemaining() const
{
  const auto today = QDate::currentDate();
  if (m_daysCachedOn != today) {
    m_daysCachedOn  = today;
    m_daysRemaining = MonotonicClock::now().toUTC().daysTo(m_trialExpiry);
  }

  return m_daysRemaining;
}

/**
 * @brief Drops the cached day count; every path that moves the expiry calls it.
 */
void Licensing::Trial::invalidateDaysCache()
{
  m_daysCachedOn = QDate();
}

//--------------------------------------------------------------------------------------------------
// Trial management
//--------------------------------------------------------------------------------------------------

/**
 * @brief Starts the trial if available.
 */
void Licensing::Trial::enableTrial()
{
  if (trialAvailable())
    fetchTrialState();
}

/**
 * @brief Reads trial data from persistent storage.
 */
void Licensing::Trial::readSettings()
{
  m_trialEnabled = false;
  m_trialExpiry  = QDateTime::currentDateTimeUtc();
  invalidateDaysCache();

  m_settings.beginGroup("trial");
  auto expStr = m_crypt.decryptToString(m_settings.value("expiry").toString());
  auto enaStr = m_crypt.decryptToString(m_settings.value("enabled").toString());
  auto regStr = m_crypt.decryptToString(m_settings.value("registd").toString());
  m_settings.endGroup();

  if (!expStr.isEmpty()) {
    QDateTime expiry = QDateTime::fromString(expStr, Qt::ISODate).toUTC();
    if (expiry.isValid()) {
      m_trialExpiry = expiry;
      invalidateDaysCache();
    }
  }

  const bool enabledStored    = (enaStr == "true");
  const bool registeredStored = (regStr == "true");
  const bool notExpired       = MonotonicClock::now().toUTC() <= m_trialExpiry;

  m_deviceRegistered = registeredStored;
  m_trialEnabled     = enabledStored && notExpired && registeredStored;

  if (trialEnabled())
    installTrialToken(daysRemaining());

  if (trialAvailable() && m_deviceRegistered) {
    m_silentFetch = true;
    fetchTrialState();
  }
}

/**
 * @brief Saves current trial state to persistent storage.
 */
void Licensing::Trial::writeSettings()
{
  QString enaStr = m_trialEnabled ? "true" : "false";
  QString regStr = m_deviceRegistered ? "true" : "false";
  QString expStr = m_trialExpiry.toString(Qt::ISODate);

  m_settings.beginGroup("trial");
  m_settings.setValue("expiry", m_crypt.encryptToString(expStr));
  m_settings.setValue("enabled", m_crypt.encryptToString(enaStr));
  m_settings.setValue("registd", m_crypt.encryptToString(regStr));
  m_settings.endGroup();
}

//--------------------------------------------------------------------------------------------------
// Backend communication
//--------------------------------------------------------------------------------------------------

/**
 * @brief Sends a trial activation or validation request to the backend.
 */
void Licensing::Trial::fetchTrialState()
{
  if (m_busy)
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  const qint64 timestamp = QDateTime::currentSecsSinceEpoch();
  const QString nonce    = QUuid::createUuid().toString(QUuid::WithoutBraces);

  QJsonObject payload;
  static auto& machineId = MachineID::instance();
  payload["machine_id"]  = machineId.appVerMachineId();
  payload["timestamp"]   = QString::number(timestamp);
  payload["nonce"]       = nonce;

  const auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  const QUrl url(QStringLiteral("https://cloud.serial-studio.com/trial"));
  QNetworkRequest request(url);
  request.setTransferTimeout(15 * 1000);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  (void)m_manager.post(request, payloadData);
}

/**
 * @brief Handles the trial server response (expiry capped at 14 days). A malformed reply leaves
 *        state untouched; the token slot is cleared only when the trial owns it. Messages are
 *        posted, not shown: a modal here runs its loop under the reply's stack (K13).
 *        enabledChanged fires only on a real change, which device-rebuilding consumers rely on.
 */
void Licensing::Trial::onServerReply(QNetworkReply* reply)
{
  const bool silent = m_silentFetch;
  m_silentFetch     = false;

  m_busy = false;
  Q_EMIT busyChanged();

  if (reply->error() != QNetworkReply::NoError) {
    if (!silent)
      Misc::Utilities::postMessageBox(QObject::tr("Network error"),
                                      reply->errorString(),
                                      QMessageBox::Critical,
                                      QObject::tr("Trial Activation Error"));

    reply->deleteLater();
    return;
  }

  const QByteArray data = reply->readAll();
  reply->deleteLater();

  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    if (!silent)
      Misc::Utilities::postMessageBox(
        QObject::tr("Invalid server response"),
        QObject::tr("The server returned malformed data: %1").arg(parseError.errorString()),
        QMessageBox::Warning,
        QObject::tr("Trial Activation Error"));

    return;
  }

  const QJsonObject object    = document.object();
  const auto expiryVal        = object.value("expireAt");
  const auto enabledVal       = object.value("trialEnabled");
  const auto deviceRegistered = object.value("registered");

  QDateTime expiry;
  if (expiryVal.isString() && enabledVal.isBool() && deviceRegistered.isBool())
    expiry = QDateTime::fromString(expiryVal.toString(), Qt::ISODate).toUTC();

  if (!expiry.isValid()) {
    if (!silent)
      Misc::Utilities::postMessageBox(
        QObject::tr("Unexpected server response"),
        QObject::tr("The server response is missing required fields."),
        QMessageBox::Warning,
        QObject::tr("Trial Activation Error"));

    return;
  }

  const QDateTime now = QDateTime::currentDateTimeUtc();
  if (expiry > now.addDays(14))
    expiry = now.addDays(14);

  const bool wasEnabled     = m_trialEnabled;
  const bool wasRegistered  = m_deviceRegistered;
  const QDateTime wasExpiry = m_trialExpiry;

  m_trialExpiry = expiry;
  invalidateDaysCache();
  m_trialEnabled     = enabledVal.toBool() && (expiry > now);
  m_deviceRegistered = deviceRegistered.toBool();

  if (trialEnabled())
    installTrialToken(daysRemaining());
  else if (CommercialToken::current().featureTier() == FeatureTier::Trial)
    Licensing::CommercialToken::clearCurrent();

  writeSettings();

  if (m_trialEnabled != wasEnabled || m_deviceRegistered != wasRegistered
      || m_trialExpiry != wasExpiry)
    Q_EMIT enabledChanged();
}
