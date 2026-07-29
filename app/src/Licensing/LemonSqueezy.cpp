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

#include "Licensing/LemonSqueezy.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QNetworkReply>

#include "AppInfo.h"
#include "Licensing/CommercialToken.h"
#include "Licensing/MachineID.h"
#include "Licensing/MonotonicClock.h"
#include "Licensing/OfflineLicense.h"
#include "Licensing/Trial.h"
#include "Misc/Utilities.h"

//--------------------------------------------------------------------------------------------------
// Define official Serial Studio store ID & product ID
//--------------------------------------------------------------------------------------------------

static constexpr quint64 STORE_ID = 170454;
static constexpr quint64 PRDCT_ID = 496241;

//--------------------------------------------------------------------------------------------------
// Constructor & singleton access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Constructor for the LemonSqueezy licensing handler.
 */
Licensing::LemonSqueezy::LemonSqueezy()
  : m_busy(false)
  , m_seatLimit(-1)
  , m_seatUsage(-1)
  , m_activated(false)
  , m_appName(APP_NAME)
  , m_silentValidation(true)
  , m_revalidatingCache(false)
  , m_gracePeriod(0)
{
  static auto& machineId = MachineID::instance();

  m_simpleCrypt.setKey(machineId.machineSpecificKey());
  m_simpleCrypt.setIntegrityProtectionMode(Licensing::SimpleCrypt::ProtectionHash);

  readSettings();
}

/**
 * @brief Provides access to the LemonSqueezy singleton instance.
 */
Licensing::LemonSqueezy& Licensing::LemonSqueezy::instance()
{
  static LemonSqueezy instance;
  return instance;
}

//--------------------------------------------------------------------------------------------------
// Member access functions
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns whether a licensing operation is currently running.
 */
bool Licensing::LemonSqueezy::busy() const
{
  return m_busy;
}

/**
 * @brief Returns the total number of allowed device activations.
 */
int Licensing::LemonSqueezy::seatLimit() const
{
  return m_seatLimit;
}

/**
 * @brief Returns the number of devices currently activated.
 */
int Licensing::LemonSqueezy::seatUsage() const
{
  return m_seatUsage;
}

/**
 * @brief Returns true when Pro is active via an online license or an imported
 * offline certificate; the single "is Pro activated" check used app-wide.
 */
bool Licensing::LemonSqueezy::isActivated() const
{
  static auto& offlineLicense = OfflineLicense::instance();
  return m_activated || offlineLicense.isActivated();
}

/**
 * @brief Returns true only for an online (LemonSqueezy) activation, ignoring offline.
 */
bool Licensing::LemonSqueezy::isOnlineActivated() const noexcept
{
  return m_activated;
}

/**
 * @brief Checks if the stored license key is valid in format.
 */
bool Licensing::LemonSqueezy::canActivate() const
{
  return m_license.length() == 36;
}

/**
 * @brief Returns the application name.
 */
const QString& Licensing::LemonSqueezy::appName() const
{
  return m_appName;
}

/**
 * @brief Returns the license key currently stored.
 */
const QString& Licensing::LemonSqueezy::license() const
{
  return m_license;
}

/**
 * @brief Returns the current instance ID from Lemon Squeezy.
 */
const QString& Licensing::LemonSqueezy::instanceId() const
{
  return m_instanceId;
}

/**
 * @brief Returns the variant name of the purchased license.
 */
const QString& Licensing::LemonSqueezy::variantName() const
{
  return m_variantName;
}

/**
 * @brief Returns the machine-specific instance name.
 */
const QString& Licensing::LemonSqueezy::instanceName() const
{
  return m_instanceName;
}

/**
 * @brief Returns the customer name as registered during purchase.
 */
const QString& Licensing::LemonSqueezy::customerName() const
{
  return m_customerName;
}

/**
 * @brief Returns the customer's email address tied to the license.
 */
const QString& Licensing::LemonSqueezy::customerEmail() const
{
  return m_customerEmail;
}

/**
 * @brief Returns the full licensing metadata received from the server.
 */
const QJsonObject& Licensing::LemonSqueezy::licensingData() const
{
  return m_licensingData;
}

//--------------------------------------------------------------------------------------------------
// Public slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Opens the Lemon Squeezy product link for Serial Studio Pro.
 */
void Licensing::LemonSqueezy::buy()
{
  auto url = QStringLiteral(
    "https://store.serial-studio.com/checkout/buy/e33e6d04-639f-46b7-bd68-b46d341c5b16");
  QDesktopServices::openUrl(QUrl(url));
}

/**
 * @brief Activates the license key using the Lemon Squeezy API.
 */
void Licensing::LemonSqueezy::activate()
{
  if (!canActivate())
    return;

  if (busy())
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  QJsonObject payload;
  payload.insert("license_key", license());
  static auto& machineId = MachineID::instance();
  payload.insert("instance_name", machineId.machineId());

  auto url         = QUrl("https://api.lemonsqueezy.com/v1/licenses/activate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkRequest req(url);
  req.setTransferTimeout(15 * 1000);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  auto* reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() != QNetworkReply::NoError)
      qWarning() << "[LemonSqueezy] Activation network error:" << reply->errorString();

    readActivationResponse(reply->error() == QNetworkReply::NoError ? reply->readAll()
                                                                    : QByteArray());
    reply->deleteLater();
  });
}

/**
 * @brief Validates the current license key and instance ID.
 */
void Licensing::LemonSqueezy::validate()
{
  if (!canActivate())
    return;

  if (busy())
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  QJsonObject payload;
  payload.insert("license_key", license());
  payload.insert("instance_id", instanceId());

  auto url         = QUrl("https://api.lemonsqueezy.com/v1/licenses/validate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkRequest req(url);
  req.setTransferTimeout(15 * 1000);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  auto* reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() != QNetworkReply::NoError)
      qWarning() << "[LemonSqueezy] Validation network error:" << reply->errorString();

    readValidationResponse(
      reply->error() == QNetworkReply::NoError ? reply->readAll() : QByteArray(), false);
    reply->deleteLater();

    const bool silent   = m_revalidatingCache;
    m_revalidatingCache = false;
    if (!silent || m_activated)
      writeSettings();
  });
}

/**
 * @brief Startup re-check of a stored license: runs even when the cached restore was rejected
 *        (that is exactly the state a live verdict must resolve), but marks the attempt silent
 *        so a failure can neither box the user nor touch the stored blob. Only a confirmed
 *        activation writes anything back.
 */
void Licensing::LemonSqueezy::revalidateCachedLicense()
{
  if (!canActivate() || isOnlineActivated())
    return;

  m_revalidatingCache = true;
  validate();
  if (!busy())
    m_revalidatingCache = false;
}

/**
 * @brief Returns true when a validation failure is a live server verdict the app may act on:
 *        a cached restore and a silent startup re-check must never clear stored state, or a
 *        transient rejection permanently destroys a valid license (spec 0042).
 */
bool Licensing::LemonSqueezy::liveVerdict(const bool cachedResponse) const
{
  return !cachedResponse && !m_revalidatingCache;
}

/**
 * @brief Deactivates the license on this machine; an offline certificate routes
 * to OfflineLicense::deactivate() so the UI, CLI, and API share one entry point.
 */
void Licensing::LemonSqueezy::deactivate()
{
  static auto& offlineLicense = OfflineLicense::instance();
  if (offlineLicense.isActivated()) {
    offlineLicense.deactivate();
    return;
  }

  if (!isOnlineActivated())
    return;

  if (busy())
    return;

  m_busy = true;
  Q_EMIT busyChanged();

  QJsonObject payload;
  payload.insert("license_key", license());
  payload.insert("instance_id", instanceId());

  auto url         = QUrl("https://api.lemonsqueezy.com/v1/licenses/deactivate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  QNetworkRequest req(url);
  req.setTransferTimeout(15 * 1000);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  auto* reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() != QNetworkReply::NoError)
      qWarning() << "[LemonSqueezy] Deactivation network error:" << reply->errorString();

    readDeactivationResponse(reply->error() == QNetworkReply::NoError ? reply->readAll()
                                                                      : QByteArray());
    reply->deleteLater();

    writeSettings();
  });
}

/**
 * @brief Opens the Lemon Squeezy customer portal in the default browser.
 */
void Licensing::LemonSqueezy::openCustomerPortal()
{
  auto url = QStringLiteral("https://store.serial-studio.com/billing");
  QDesktopServices::openUrl(QUrl(url));
}

/**
 * @brief Updates the license key stored locally.
 */
void Licensing::LemonSqueezy::setLicense(const QString& license)
{
  const auto simplified = license.simplified();
  if (m_license == simplified)
    return;

  m_license = simplified;
  Q_EMIT licenseChanged();
}

//--------------------------------------------------------------------------------------------------
// Private slots
//--------------------------------------------------------------------------------------------------

/**
 * @brief Loads and decrypts cached licensing data from QSettings; the offline
 * grace period is recomputed from the last check, with monotonicNow() flooring
 * the clock so rewinding the system time cannot extend it.
 */
void Licensing::LemonSqueezy::readSettings()
{
  m_settings.beginGroup("licensing");
  auto license = m_settings.value("license", "").toString();
  auto data    = m_settings.value("data", "").toString();
  auto dt      = m_settings.value("lastCheck", "").toString();
  m_settings.endGroup();

  if (data.isEmpty() || license.isEmpty())
    return;

  m_license          = m_simpleCrypt.decryptToString(license);
  auto decryptedData = m_simpleCrypt.decryptToByteArray(data);

  m_gracePeriod = 0;
  if (!dt.isEmpty()) {
    auto dateTime  = m_simpleCrypt.decryptToString(dt);
    auto currentDt = monotonicNow();
    auto lastCheck = QDateTime::fromString(dateTime, Qt::RFC2822Date);
    if (lastCheck.isValid() && lastCheck < currentDt)
      m_gracePeriod = qMax(0, 30 - lastCheck.daysTo(currentDt));
  }

  readValidationResponse(decryptedData, true);
  Q_EMIT licenseChanged();
}

/**
 * @brief Encrypts and writes the current license key and metadata to QSettings.
 */
void Licensing::LemonSqueezy::writeSettings()
{
  auto json = QJsonDocument(m_licensingData).toJson(QJsonDocument::Compact);

  if (m_licensingData.isEmpty() && canActivate())
    return;

  if (!json.isEmpty() && canActivate()) {
    m_settings.beginGroup("licensing");
    m_settings.setValue("license", m_simpleCrypt.encryptToString(m_license));
    m_settings.setValue("data", m_simpleCrypt.encryptToString(json));
    m_settings.endGroup();
  }

  else {
    m_settings.beginGroup("licensing");
    m_settings.setValue("data", "");
    m_settings.setValue("license", "");
    m_settings.setValue("lastCheck", "");
    m_settings.endGroup();
  }
}

/**
 * @brief Returns now floored at the highest wall-clock ever observed (anti clock-rewind).
 */
QDateTime Licensing::LemonSqueezy::monotonicNow()
{
  return MonotonicClock::now();
}

/**
 * @brief Clears in-memory licensing state and optionally the stored license key;
 * the on-disk blob is only rewritten when persist is set, so a failed cache
 * restore cannot erase a license a later live verdict could still confirm. The
 * token slot is shared with Trial, which re-claims it before the emissions below.
 */
void Licensing::LemonSqueezy::clearLicenseCache(const bool clearLicense, const bool persist)
{
  m_busy           = false;
  m_seatLimit      = -1;
  m_seatUsage      = -1;
  m_instanceId     = "";
  m_variantName    = "";
  m_instanceName   = "";
  m_customerName   = "";
  m_activated      = false;
  m_customerEmail  = "";
  m_appName        = APP_NAME;
  m_activationDate = QDateTime();
  m_licensingData  = QJsonObject();
  CommercialToken::clearCurrent();
  Trial::reassertTokenIfEntitled();
  qApp->setApplicationDisplayName(appName());

  if (clearLicense) {
    m_license = "";
    Q_EMIT licenseChanged();
  }

  Q_EMIT busyChanged();
  Q_EMIT activatedChanged();
  Q_EMIT licenseDataChanged();

  if (persist)
    writeSettings();
}

//--------------------------------------------------------------------------------------------------
// LemonSqueezy response parsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Handles the empty-response branch of license validation: warn, decay grace. A failed
 *        cache restore clears in-memory state only; the on-disk blob survives for a later
 *        live verdict.
 */
void Licensing::LemonSqueezy::handleEmptyValidationResponse(const bool cachedResponse)
{
  qWarning() << "Activation server unreachable. License validation failed.";

  if (m_gracePeriod <= 0) {
    qWarning() << "Grace period expired. Clearing cached license.";
    clearLicenseCache(false, liveVerdict(cachedResponse));
  }

  else {
    qWarning() << "You have" << m_gracePeriod << "day(s) remaining in your grace period.";
  }

  m_busy = false;
  Q_EMIT busyChanged();
}

/**
 * @brief Runs the validation rule chain on a parsed JSON object; returns false on any failure.
 */
bool Licensing::LemonSqueezy::checkValidationRules(const QJsonObject& json,
                                                   const bool cachedResponse)
{
  const auto error      = json.value("error");
  const auto meta       = json.value("meta").toObject();
  const auto valid      = json.value("valid").toBool(false);
  const auto instance   = json.value("instance").toObject();
  const auto licenseKey = json.value("license_key").toObject();

  const auto instanceId    = instance.value("id").toString();
  const auto instanceName  = instance.value("name").toString();
  const auto licenseStatus = licenseKey.value("status").toString();
  const auto storeId       = meta.value("store_id").toInteger();
  const auto productId     = meta.value("product_id").toInteger();

  const bool live = liveVerdict(cachedResponse);

  if (!error.isNull() && !error.toString().simplified().isEmpty()) {
    qWarning() << "[LemonSqueezy] Validation error:" << error.toString();
    if (live)
      Misc::Utilities::showMessageBox(
        tr("There was an issue validating your license."), error.toString(), QMessageBox::Critical);

    clearLicenseCache(false, live);
    return false;
  }

  bool ok  = true;
  ok      &= json.contains("meta");
  ok      &= json.contains("instance");
  ok      &= meta.contains("store_id");
  ok      &= meta.contains("product_id");
  if (!ok)
    return false;

  if (storeId != STORE_ID || productId != PRDCT_ID) {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    if (live)
      Misc::Utilities::showMessageBox(
        tr("The license key you provided does not belong to Serial Studio."),
        tr("Please double-check that you purchased your license from the official "
           "Serial Studio store."),
        QMessageBox::Critical);

    clearLicenseCache(false, live);
    return false;
  }

  static auto& machineId = MachineID::instance();
  if (instanceName != machineId.machineId()) {
    qWarning() << "[LemonSqueezy] Machine ID mismatch";
    if (live)
      Misc::Utilities::showMessageBox(tr("This license key was activated on a different device."),
                                      tr("Deactivate it there first or contact support for help."),
                                      QMessageBox::Critical);

    clearLicenseCache(false, live);
    return false;
  }

  if (licenseStatus != "active") {
    qWarning() << "[LemonSqueezy] License status is not active:" << licenseStatus;
    if (live)
      Misc::Utilities::showMessageBox(
        tr("This license is not currently active."),
        tr("It may have expired or been deactivated (status: %1).").arg(licenseStatus),
        QMessageBox::Warning);

    clearLicenseCache(false, live);
    return false;
  }

  if (instanceId.isEmpty()) {
    qWarning() << "[LemonSqueezy] Activation response missing instance ID";
    if (live)
      Misc::Utilities::showMessageBox(tr("Something went wrong on the server."),
                                      tr("No activation ID was returned."),
                                      QMessageBox::Critical);

    clearLicenseCache(false, live);
    return false;
  }

  if (!valid) {
    qWarning() << "[LemonSqueezy] Validation failed";
    if (live)
      Misc::Utilities::showMessageBox(tr("Could not validate your license at this time."),
                                      tr("Try again later."),
                                      QMessageBox::Warning);

    clearLicenseCache(false, live);
    return false;
  }

  return true;
}

/**
 * @brief Updates m_appName from the variant name; falls back to APP_NAME on a single-token variant.
 */
void Licensing::LemonSqueezy::updateAppNameFromVariant(const QString& variantName)
{
  const auto list = variantName.split("-");
  if (list.count() >= 2)
    if (list.first().simplified() != "Pro")
      m_appName = tr("%1 %2").arg(APP_NAME, list.first().simplified());
    else
      m_appName = tr("%1 (%2)").arg(APP_NAME, list.last().simplified());
  else
    m_appName = APP_NAME;
}

/**
 * @brief Persists fresh license fields, installs the commercial token, and notifies QML.
 *        A server-validated license IS the entitlement: deriving the tier from the variant
 *        string once turned a valid legacy key into an invalid token, so names only label
 *        the token and an empty one falls back instead of failing the seal.
 */
void Licensing::LemonSqueezy::applyValidatedLicense(const QJsonObject& json,
                                                    const bool cachedResponse)
{
  const auto meta       = json.value("meta").toObject();
  const auto instance   = json.value("instance").toObject();
  const auto licenseKey = json.value("license_key").toObject();

  const bool wasActivated = m_activated;

  m_activated      = true;
  m_licensingData  = json;
  m_instanceId     = instance.value("id").toString();
  m_variantName    = meta.value("variant_name").toString();
  m_instanceName   = instance.value("name").toString();
  m_seatLimit      = licenseKey.value("activation_limit").toInt(-1);
  m_seatUsage      = licenseKey.value("activation_usage").toInt(-1);
  m_customerName   = meta.value("customer_name").toString();
  m_customerEmail  = meta.value("customer_email").toString();
  m_activationDate = instance.value("created_at").toVariant().toDateTime();

  updateAppNameFromVariant(m_variantName);

  static auto& machineId = MachineID::instance();

  CommercialToken token;
  token.setVariantName(m_variantName.isEmpty() ? QStringLiteral(APP_NAME) : m_variantName);
  token.setInstanceName(m_instanceName.isEmpty() ? machineId.machineId() : m_instanceName);
  token.setGraceDaysRemaining(cachedResponse ? m_gracePeriod : 30);
  token.setFeatureTier(FeatureTier::Pro);
  token.seal();
  CommercialToken::setCurrent(token);

  m_busy = false;
  Q_EMIT busyChanged();
  Q_EMIT licenseDataChanged();

  if (!cachedResponse) {
    const auto dt = QDateTime::currentDateTime().toString(Qt::RFC2822Date);
    m_settings.beginGroup("licensing");
    m_settings.setValue("lastCheck", m_simpleCrypt.encryptToString(dt));
    m_settings.endGroup();
  }

  if (!wasActivated)
    Q_EMIT activatedChanged();

  if (!m_silentValidation) {
    m_silentValidation = true;
    Misc::Utilities::showMessageBox(
      tr("Your license has been successfully activated."),
      tr("Thank you for supporting Serial Studio!\nYou now have access to all premium features."),
      QMessageBox::Information);
  }
}

/**
 * @brief Processes the response from the license validation request.
 */
void Licensing::LemonSqueezy::readValidationResponse(const QByteArray& data,
                                                     const bool cachedResponse)
{
  if (data.isEmpty()) {
    handleEmptyValidationResponse(cachedResponse);
    return;
  }

  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache(false, liveVerdict(cachedResponse));
    return;
  }

  const auto json = doc.object();
  if (json.isEmpty())
    return;

  if (!checkValidationRules(json, cachedResponse))
    return;

  applyValidatedLicense(json, cachedResponse);
}

/**
 * @brief Runs the validation rule chain on a parsed activation response.
 */
bool Licensing::LemonSqueezy::checkActivationRules(const QJsonObject& json)
{
  const auto error      = json.value("error");
  const auto meta       = json.value("meta").toObject();
  const auto instance   = json.value("instance").toObject();
  const auto licenseKey = json.value("license_key").toObject();
  const auto activated  = json.value("activated").toBool(false);

  const auto instanceId    = instance.value("id").toString();
  const auto instanceName  = instance.value("name").toString();
  const auto licenseStatus = licenseKey.value("status").toString();
  const auto storeId       = meta.value("store_id").toInteger();
  const auto productId     = meta.value("product_id").toInteger();

  if (!error.isNull()) {
    qWarning() << "[LemonSqueezy] Activation error:" << error.toString();
    Misc::Utilities::showMessageBox(
      tr("There was an issue activating your license."), error.toString(), QMessageBox::Critical);
    clearLicenseCache(true);
    return false;
  }

  if (storeId != STORE_ID || productId != PRDCT_ID) {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    Misc::Utilities::showMessageBox(
      tr("The license key you provided does not belong to Serial Studio."),
      tr("Double-check that you purchased your license from the official Serial Studio store."),
      QMessageBox::Critical);
    clearLicenseCache(true);
    return false;
  }

  static auto& machineId = MachineID::instance();
  if (instanceName != machineId.machineId()) {
    qWarning() << "[LemonSqueezy] Machine ID mismatch";
    Misc::Utilities::showMessageBox(tr("This license key was activated on a different device."),
                                    tr("Deactivate it there first or contact support for help."),
                                    QMessageBox::Critical);
    clearLicenseCache(true);
    return false;
  }

  if (licenseStatus != "active") {
    qWarning() << "[LemonSqueezy] License status is not active:" << licenseStatus;

    Misc::Utilities::showMessageBox(
      tr("This license is not currently active."),
      tr("It may have expired or been deactivated (status: %1).").arg(licenseStatus),
      QMessageBox::Warning);

    clearLicenseCache(true);
    return false;
  }

  if (instanceId.isEmpty()) {
    qWarning() << "[LemonSqueezy] Activation response missing instance ID";
    Misc::Utilities::showMessageBox(tr("Something went wrong on the server…"),
                                    tr("No activation ID was returned."),
                                    QMessageBox::Critical);
    clearLicenseCache();
    return false;
  }

  if (!activated) {
    qWarning() << "[LemonSqueezy] Activation failed";
    Misc::Utilities::showMessageBox(tr("Could not activate your license at this time."),
                                    tr("Try again later."),
                                    QMessageBox::Warning);
    clearLicenseCache();
    return false;
  }

  return true;
}

/**
 * @brief Processes the response from the license activation request.
 */
void Licensing::LemonSqueezy::readActivationResponse(const QByteArray& data)
{
  if (data.isEmpty()) {
    qWarning() << "[LemonSqueezy] Empty activation response";
    clearLicenseCache();
    return;
  }

  QJsonParseError parseError;
  const auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache();
    return;
  }

  const auto json = doc.object();
  if (!checkActivationRules(json))
    return;

  const auto instanceId = json.value("instance").toObject().value("id").toString();
  QMetaObject::invokeMethod(this, [this, instanceId] {
    m_busy             = false;
    m_instanceId       = instanceId;
    m_silentValidation = false;
    validate();
  });
}

/**
 * @brief Processes the response from the license deactivation request.
 */
void Licensing::LemonSqueezy::readDeactivationResponse(const QByteArray& data)
{
  if (data.isEmpty()) {
    qWarning() << "[LemonSqueezy] Empty activation response";
    clearLicenseCache();
    return;
  }

  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError) {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache();
    return;
  }

  auto json        = doc.object();
  auto error       = json.value("error");
  auto meta        = json.value("meta").toObject();
  auto deactivated = json.value("deactivated").toBool(false);

  auto storeId   = meta.value("store_id").toInteger();
  auto productId = meta.value("product_id").toInteger();

  if (!error.isNull()) {
    qWarning() << "[LemonSqueezy] Deactivation error:" << error.toString();
    Misc::Utilities::showMessageBox(
      tr("There was an issue deactivating your license."), error.toString(), QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  if (storeId != STORE_ID || productId != PRDCT_ID) {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    Misc::Utilities::showMessageBox(
      tr("The license key you provided does not belong to Serial Studio."),
      tr("Double-check that you purchased your license from the official Serial Studio store."),
      QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  if (!deactivated) {
    qWarning() << "[LemonSqueezy] Deactivation failed";
    Misc::Utilities::showMessageBox(tr("Could not deactivate your license at this time."),
                                    tr("Try again later."),
                                    QMessageBox::Warning);
    clearLicenseCache();
    return;
  }

  clearLicenseCache(true);
  Misc::Utilities::showMessageBox(
    tr("Your license has been deactivated."),
    tr("Access to Pro features has been removed.\nThank you again for supporting Serial Studio!"),
    QMessageBox::Information);
}
