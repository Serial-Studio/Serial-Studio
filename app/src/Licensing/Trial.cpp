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
#include "MachineID.h"
#include "LemonSqueezy.h"
#include "Misc/Utilities.h"

#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QNetworkRequest>

/**
 * @brief Constructs the Trial licensing system.
 *
 * Initializes encryption with a machine-specific key and loads any
 * cached trial data from persistent settings. If a trial was previously
 * enabled, it automatically revalidates it with the backend server.
 */
Licensing::Trial::Trial()
  : m_trialEnabled(false)
  , m_deviceRegistered(false)
  , m_trialExpiry(QDateTime::currentDateTimeUtc())
{
  // Sync-activation dependent modules to trial module
  connect(this, &Licensing::Trial::enabledChanged,
          &Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged);
  connect(&Licensing::LemonSqueezy::instance(),
          &Licensing::LemonSqueezy::activatedChanged, this,
          &Licensing::Trial::availableChanged);

  // Read server responses
  connect(&m_manager, &QNetworkAccessManager::finished, this,
          &Licensing::Trial::onServerReply);

  // Configure data encryption
  m_crypt.setKey(MachineID::instance().machineSpecificKey());
  m_crypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);

  // Read settings
  if (!Licensing::LemonSqueezy::instance().isActivated())
    readSettings();
}

/**
 * @brief Gets the singleton instance of the Trial licensing class.
 * @return Reference to the static Trial instance.
 */
Licensing::Trial &Licensing::Trial::instance()
{
  static Trial instance;
  return instance;
}

/**
 * @brief Returns whether a trial query operation is currently running.
 *
 * Useful to disable UI elements while the API is validating,
 * activating, or deactivating a license key.
 */
bool Licensing::Trial::busy() const
{
  return m_busy;
}

/**
 * @brief Checks whether the user has ever started a trial.
 * @return true if the user never started a trial, false otherwise.
 */
bool Licensing::Trial::firstRun() const
{
  return !m_deviceRegistered && !m_trialEnabled && trialAvailable();
}

/**
 * @brief Checks whether a trial period is currently active.
 * @return true if trial is active and valid, false otherwise.
 */
bool Licensing::Trial::trialEnabled() const
{
  return m_deviceRegistered && m_trialEnabled && trialAvailable();
}

/**
 * @brief Checks whether a trial period expired.
 * @return true if trial has been enabled and expired, false otherwise.
 */
bool Licensing::Trial::trialExpired() const
{
  return m_deviceRegistered && !m_trialEnabled && trialAvailable();
}

/**
 * @brief Checks if a trial can be started.
 *
 * Trial is only available if the app is not activated via Lemon Squeezy.
 *
 * @return true if a trial can be started, false otherwise.
 */
bool Licensing::Trial::trialAvailable() const
{
  const bool activated = LemonSqueezy::instance().isActivated();
  return !activated;
}

/**
 * @brief Gets the number of days remaining in the active trial.
 * @return Days remaining, or 0 if expired or not started.
 */
int Licensing::Trial::daysRemaining() const
{
  return QDateTime::currentDateTimeUtc().daysTo(m_trialExpiry);
}

/**
 * @brief Starts the trial if available.
 *
 * Initiates a backend request to activate the trial. Does nothing
 * if a license is already active or the trial is not available.
 */
void Licensing::Trial::enableTrial()
{
  if (trialAvailable())
    fetchTrialState();
}

/**
 * @brief Reads trial data from persistent storage.
 *
 * Decrypts saved data and restores trial activation state and expiry date.
 * If a trial was previously enabled, revalidates the trial status with backend.
 */
void Licensing::Trial::readSettings()
{
  m_trialEnabled = false;
  m_trialExpiry = QDateTime::currentDateTimeUtc();

  m_settings.beginGroup("trial");
  auto expStr = m_crypt.decryptToString(m_settings.value("expiry").toString());
  auto enaStr = m_crypt.decryptToString(m_settings.value("enabled").toString());
  auto regStr = m_crypt.decryptToString(m_settings.value("registd").toString());
  m_settings.endGroup();

  if (!expStr.isEmpty())
  {
    QDateTime expiry = QDateTime::fromString(expStr, Qt::ISODate).toUTC();
    if (expiry.isValid())
      m_trialExpiry = expiry;
  }

  const bool enabledStored = (enaStr == "true");
  const bool registeredStored = (regStr == "true");
  const bool notExpired = QDateTime::currentDateTimeUtc() <= m_trialExpiry;

  m_deviceRegistered = registeredStored;
  m_trialEnabled = enabledStored && notExpired && registeredStored;

  if (trialAvailable() && m_deviceRegistered)
    fetchTrialState();
}

/**
 * @brief Saves current trial state to persistent storage.
 *
 * Encrypts and stores the trial activation flag and expiry timestamp
 * using the machine-specific encryption key.
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

/**
 * @brief Sends a trial activation or validation request to the backend.
 *
 * Builds a signed payload including machine ID and a nonce, then
 * submits it via HTTPS to the activation endpoint.
 */
void Licensing::Trial::fetchTrialState()
{
  // Stop if system is busy
  if (m_busy)
    return;

  // Mark busy flag
  m_busy = true;
  Q_EMIT busyChanged();

  // Generate timestamp and nonce
  qint64 timestamp = QDateTime::currentSecsSinceEpoch();
  QString nonce = QUuid::createUuid().toString(QUuid::WithoutBraces);

  // Create JSON payload
  QJsonObject payload;
  payload["machine_id"] = MachineID::instance().appVerMachineId();
  payload["timestamp"] = QString::number(timestamp);
  payload["nonce"] = nonce;

  // Convert JSON to string
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  // Construct request
  QUrl url(QStringLiteral("https://cloud.serial-studio.com/trial"));
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // Send a POST request to the activation server
  (void)m_manager.post(request, payloadData);
}

/**
 * @brief Handles the server response for trial activation.
 *
 * This method is called when the server responds to a trial activation
 * request. It performs the following tasks:
 * - Clears the busy flag and emits `busyChanged()`
 * - Validates the network reply for errors
 * - Parses and verifies the JSON response from the server
 * - Sets the trial activation status, registration state, and expiration date
 * - Enforces a maximum trial period of 14 days from the current UTC time
 * - Saves the updated settings and emits `enabledChanged()`
 *
 * In case of any error (network failure, malformed data, or missing fields),
 * the function will notify the user with an appropriate message box and exit
 * early.
 *
 * @param reply The network reply object containing the server's response.
 */
void Licensing::Trial::onServerReply(QNetworkReply *reply)
{
  // Unset busy flag
  m_busy = false;
  Q_EMIT busyChanged();

  // Check for network error
  if (reply->error() != QNetworkReply::NoError)
  {
    Misc::Utilities::showMessageBox(QObject::tr("Network error"),
                                    reply->errorString(), QMessageBox::Critical,
                                    QObject::tr("Trial Activation Error"));
    reply->deleteLater();
    Q_EMIT enabledChanged();
    return;
  }

  // Read and parse JSON
  const QByteArray data = reply->readAll();
  reply->deleteLater();

  // Attempt to parse JSON
  QJsonParseError parseError;
  QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject())
  {
    Misc::Utilities::showMessageBox(
        QObject::tr("Invalid server response"),
        QObject::tr("The server returned malformed data: %1")
            .arg(parseError.errorString()),
        QMessageBox::Warning, QObject::tr("Trial Activation Error"));
    Q_EMIT enabledChanged();
    return;
  }

  // Set default state
  m_trialEnabled = false;
  m_deviceRegistered = false;
  m_trialExpiry = QDateTime::currentDateTimeUtc();

  // Parse keys
  const auto object = document.object();
  const auto expiryVal = object.value("expireAt");
  const auto enabledVal = object.value("trialEnabled");
  const auto deviceRegistered = object.value("registered");
  if (expiryVal.isString() && enabledVal.isBool() && deviceRegistered.isBool())
  {
    // Accept only valid and reasonable expiry timestamps
    const QString expiryStr = expiryVal.toString();
    QDateTime expiry = QDateTime::fromString(expiryStr, Qt::ISODate).toUTC();
    if (expiry.isValid())
    {
      const QDateTime now = QDateTime::currentDateTimeUtc();
      if (expiry > now.addDays(14))
        expiry = now.addDays(14);

      m_trialExpiry = expiry;
      m_trialEnabled = enabledVal.toBool();
      m_deviceRegistered = deviceRegistered.toBool();
    }
  }

  // Unexpected response
  else
  {
    Misc::Utilities::showMessageBox(
        QObject::tr("Unexpected server response"),
        QObject::tr("The server response is missing required fields."),
        QMessageBox::Warning, QObject::tr("Trial Activation Error"));
  }

  // Save updated state and emit signal
  writeSettings();
  Q_EMIT enabledChanged();
}
