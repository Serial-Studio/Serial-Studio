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

#include <QNetworkReply>
#include <QDesktopServices>

#include "AppInfo.h"
#include "Misc/Utilities.h"
#include "Licensing/MachineID.h"
#include "Licensing/LemonSqueezy.h"

//------------------------------------------------------------------------------
// Define official Serial Studio store ID & product ID
//------------------------------------------------------------------------------

static constexpr quint64 STORE_ID = 170454;
static constexpr quint64 PRDCT_ID = 496241;

//------------------------------------------------------------------------------
// Constructor & singleton access functions
//------------------------------------------------------------------------------

/**
 * @brief Constructor for the LemonSqueezy licensing handler.
 *
 * Initializes encryption with a machine-specific key and loads
 * any previously cached license state from settings.
 */
Licensing::LemonSqueezy::LemonSqueezy()
  : m_busy(false)
  , m_seatLimit(-1)
  , m_seatUsage(-1)
  , m_activated(false)
  , m_appName(APP_NAME)
  , m_silentValidation(true)
  , m_gracePeriod(0)
{
  // Configure data encryption
  m_simpleCrypt.setKey(MachineID::instance().machineSpecificKey());
  m_simpleCrypt.setIntegrityProtectionMode(
      Licensing::SimpleCrypt::ProtectionHash);

  // Read settings
  readSettings();
}

/**
 * @brief Provides access to the LemonSqueezy singleton instance.
 * @return Static instance of the licensing manager.
 */
Licensing::LemonSqueezy &Licensing::LemonSqueezy::instance()
{
  static LemonSqueezy instance;
  return instance;
}

//------------------------------------------------------------------------------
// Member access functions
//------------------------------------------------------------------------------

/**
 * @brief Returns whether a licensing operation is currently running.
 *
 * Useful to disable UI elements while the API is validating,
 * activating, or deactivating a license key.
 */
bool Licensing::LemonSqueezy::busy() const
{
  return m_busy;
}

/**
 * @brief Returns the total number of allowed device activations.
 *
 * A value of -1 indicates unlimited activations (e.g., Elite plan).
 */
int Licensing::LemonSqueezy::seatLimit() const
{
  return m_seatLimit;
}

/**
 * @brief Returns the number of devices currently activated.
 *
 * This is the number of license instances used on Lemon Squeezy.
 */
int Licensing::LemonSqueezy::seatUsage() const
{
  return m_seatUsage;
}

/**
 * @brief Returns true if the current license has been successfully activated.
 *
 * This flag is cached from the last successful validation or activation call.
 */
bool Licensing::LemonSqueezy::isActivated() const
{
  return m_activated;
}

/**
 * @brief Checks if the stored license key is valid in format.
 *
 * Does not verify against the server — only checks local formatting.
 * Typically, a valid Lemon Squeezy license key is 36 characters long (UUID).
 */
bool Licensing::LemonSqueezy::canActivate() const
{
  return m_license.length() == 36;
}

/**
 * @brief Returns the application name, which is constructed based on the
 *        payment plan selected by the user (e.g. "Serial Studio Pro" or
 *        "Serial Studio Enterprise").
 */
const QString &Licensing::LemonSqueezy::appName() const
{
  return m_appName;
}

/**
 * @brief Returns the license key currently stored.
 *
 * This key is sent when activating or validating the license.
 */
const QString &Licensing::LemonSqueezy::license() const
{
  return m_license;
}

/**
 * @brief Returns the current instance ID from Lemon Squeezy.
 *
 * This is a UUID assigned by Lemon Squeezy when a license is activated.
 */
const QString &Licensing::LemonSqueezy::instanceId() const
{
  return m_instanceId;
}

/**
 * @brief Returns the variant name of the purchased license.
 *
 * Examples: "Pro Monthly", "Enterprise Yearly", etc.
 * Used to conditionally enable features or UI.
 */
const QString &Licensing::LemonSqueezy::variantName() const
{
  return m_variantName;
}

/**
 * @brief Returns the machine-specific instance name.
 *
 * Typically based on the device's hashed hardware ID.
 */
const QString &Licensing::LemonSqueezy::instanceName() const
{
  return m_instanceName;
}

/**
 * @brief Returns the customer name as registered during purchase.
 *
 * Retrieved from Lemon Squeezy during activation/validation.
 */
const QString &Licensing::LemonSqueezy::customerName() const
{
  return m_customerName;
}

/**
 * @brief Returns the customer's email address tied to the license.
 *
 * Useful for showing account info or customer support context.
 */
const QString &Licensing::LemonSqueezy::customerEmail() const
{
  return m_customerEmail;
}

/**
 * @brief Returns the full licensing metadata received from the server.
 *
 * This includes product info, store ID, variant, customer data, etc.
 */
const QJsonObject &Licensing::LemonSqueezy::licensingData() const
{
  return m_licensingData;
}

//------------------------------------------------------------------------------
// Public slots
//------------------------------------------------------------------------------

/**
 * @brief Opens the Lemon Squeezy product link for Serial Studio Pro.
 */
void Licensing::LemonSqueezy::buy()
{
  auto url = QStringLiteral("https://store.serial-studio.com/buy/"
                            "ba46c099-0d51-4d98-9154-6be5c35bc1ec");
  QDesktopServices::openUrl(QUrl(url));
}

/**
 * @brief Activates the license key using the Lemon Squeezy API.
 *
 * Sends the machine ID and license key to the API. On success,
 * a new instance is registered and validated.
 */
void Licensing::LemonSqueezy::activate()
{
  // Skip if license key format is invalid
  if (!canActivate())
    return;

  // Avoid repeat activation requests
  if (busy())
    return;

  // Enable busy status
  m_busy = true;
  Q_EMIT busyChanged();

  // Obtain machine ID & build JSON payload
  QJsonObject payload;
  payload.insert("license_key", license());
  payload.insert("instance_name", MachineID::instance().machineId());

  // Generate the JSON data
  auto url = QUrl("https://api.lemonsqueezy.com/v1/licenses/activate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  // Setup network request
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  // Send the activation request
  auto *reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    readActivationResponse(reply->readAll());
    reply->deleteLater();
  });
}

/**
 * @brief Validates the current license key and instance ID.
 *
 * Checks with the Lemon Squeezy API whether the license is still valid,
 * not expired, and assigned to this machine.
 */
void Licensing::LemonSqueezy::validate()
{
  // Skip if license key format is invalid
  if (!canActivate())
    return;

  // Avoid repeat validation requests
  if (busy())
    return;

  // Enable busy status
  m_busy = true;
  Q_EMIT busyChanged();

  // Obtain machine ID & build JSON payload
  QJsonObject payload;
  payload.insert("license_key", license());
  payload.insert("instance_id", instanceId());

  // Generate the JSON data
  auto url = QUrl("https://api.lemonsqueezy.com/v1/licenses/validate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  // Setup network request
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  // Send the activation request
  auto *reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    readValidationResponse(reply->readAll(), false);
    reply->deleteLater();

    writeSettings();
  });
}

/**
 * @brief Deactivates the license key instance on this machine.
 *
 * Frees up one activation seat on Lemon Squeezy. Useful when switching devices.
 */
void Licensing::LemonSqueezy::deactivate()
{
  // Skip if license is not active
  if (!isActivated())
    return;

  // Avoid repeat requests
  if (busy())
    return;

  // Enable busy status
  m_busy = true;
  Q_EMIT busyChanged();

  // Obtain machine ID & build JSON payload
  QJsonObject payload;
  payload.insert("license_key", license());
  payload.insert("instance_id", instanceId());

  // Generate the JSON data
  auto url = QUrl("https://api.lemonsqueezy.com/v1/licenses/deactivate");
  auto payloadData = QJsonDocument(payload).toJson(QJsonDocument::Compact);

  // Setup network request
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/vnd.api+json");
  req.setRawHeader("Accept", "application/vnd.api+json");

  // Send the activation request
  auto *reply = m_manager.post(req, payloadData);
  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    readDeactivationResponse(reply->readAll());
    reply->deleteLater();

    writeSettings();
  });
}

/**
 * @brief Opens the Lemon Squeezy customer portal in the default browser.
 *
 * The portal allows the user to manage licenses, billing, and view orders.
 */
void Licensing::LemonSqueezy::openCustomerPortal()
{
  auto url = QStringLiteral("https://store.serial-studio.com/billing");
  QDesktopServices::openUrl(QUrl(url));
}

/**
 * @brief Updates the license key stored locally.
 *
 * This does not trigger activation — it's typically followed by a call
 * to validate() or activate(). Emits licenseChanged().
 *
 * @param license New license key to store (UUID format expected).
 */
void Licensing::LemonSqueezy::setLicense(const QString &license)
{
  m_license = license.simplified();
  Q_EMIT licenseChanged();
}

//------------------------------------------------------------------------------
// Private slots
//------------------------------------------------------------------------------

/**
 * @brief Loads and decrypts cached licensing data from QSettings.
 *
 * If available, decrypts and parses the stored license key and metadata.
 * Automatically re-validates the data using readValidationResponse().
 */
void Licensing::LemonSqueezy::readSettings()
{
  // Obtain encrypted JSON data
  m_settings.beginGroup("licensing");
  auto license = m_settings.value("license", "").toString();
  auto data = m_settings.value("data", "").toString();
  auto dt = m_settings.value("lastCheck", "").toString();
  m_settings.endGroup();

  // Data is empty abort
  if (data.isEmpty() || license.isEmpty())
    return;

  // Descrypt data
  m_license = m_simpleCrypt.decryptToString(license);
  auto decryptedData = m_simpleCrypt.decryptToByteArray(data);

  // Check if we can use application offline
  m_gracePeriod = 0;
  if (!dt.isEmpty())
  {
    auto dateTime = m_simpleCrypt.decryptToString(dt);
    auto currentDt = QDateTime::currentDateTime();
    auto lastCheck = QDateTime::fromString(dateTime, Qt::RFC2822Date);
    if (lastCheck.isValid() && lastCheck < currentDt)
      m_gracePeriod = qMax(0, 30 - lastCheck.daysTo(currentDt));
  }

  // Parse data & validate it
  readValidationResponse(decryptedData, true);
  Q_EMIT licenseChanged();
}

/**
 * @brief Encrypts and writes the current license key and metadata to QSettings.
 *
 * This stores the license key and API response securely using SimpleCrypt.
 * If no license or metadata is valid, clears the stored data.
 */
void Licensing::LemonSqueezy::writeSettings()
{
  // Get JSON data
  auto json = QJsonDocument(m_licensingData).toJson(QJsonDocument::Compact);

  // Write encrypted data if JSON is valid
  if (!json.isEmpty() && canActivate())
  {
    m_settings.beginGroup("licensing");
    m_settings.setValue("license", m_simpleCrypt.encryptToString(m_license));
    m_settings.setValue("data", m_simpleCrypt.encryptToString(json));
    m_settings.endGroup();
  }

  // Data is empty
  else
  {
    m_settings.beginGroup("licensing");
    m_settings.setValue("data", "");
    m_settings.setValue("license", "");
    m_settings.setValue("lastCheck", "");
    m_settings.endGroup();
  }
}

/**
 * @brief Clears all in-memory licensing state and optionally the stored
 *        license key.
 *
 * Resets activation status, instance info, and cached metadata. Emits state
 * change signals for any bound UI or reactive logic.
 *
 * @param clearLicense If true, also removes the license key from memory.
 */
void Licensing::LemonSqueezy::clearLicenseCache(const bool clearLicense)
{
  m_busy = false;
  m_seatLimit = -1;
  m_seatUsage = -1;
  m_instanceId = "";
  m_variantName = "";
  m_instanceName = "";
  m_customerName = "";
  m_activated = false;
  m_customerEmail = "";
  m_appName = APP_NAME;
  m_activationDate = QDateTime();
  m_licensingData = QJsonObject();
  qApp->setApplicationDisplayName(appName());

  if (clearLicense)
  {
    m_license = "";
    Q_EMIT licenseChanged();
  }

  Q_EMIT busyChanged();
  Q_EMIT activatedChanged();
  Q_EMIT licenseDataChanged();

  writeSettings();
}

//------------------------------------------------------------------------------
// LemonSqueezy response parsing
//------------------------------------------------------------------------------

/**
 * @brief Processes the response from the license validation request.
 *
 * Verifies that the license is valid and that it corresponds to the computer
 * executing Serial Studio.
 *
 * If successful, updates all internal licensing fields and persists the
 * validated state.
 *
 * If validation fails, resets cached license data and notifies the user.
 *
 * @param data Raw JSON payload returned by Lemon Squeezy API.
 */
void Licensing::LemonSqueezy::readValidationResponse(const QByteArray &data,
                                                     const bool cachedResponse)
{
  // Data is empty, log & abort
  if (data.isEmpty())
  {
    qWarning() << "Activation server unreachable. License validation failed.";

    if (m_gracePeriod <= 0)
    {
      qWarning() << "Grace period expired. Clearing cached license.";
      clearLicenseCache();
    }

    else
    {
      qWarning() << "You have" << m_gracePeriod
                 << "day(s) remaining in your grace period.";
    }

    m_busy = false;
    Q_EMIT busyChanged();
    return;
  }

  // Parse reply as JSON
  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache();
    return;
  }

  // Obtain JSON object
  auto json = doc.object();
  auto error = json.value("error");
  auto meta = json.value("meta").toObject();
  auto valid = json.value("valid").toBool(false);
  auto instance = json.value("instance").toObject();
  auto licenseKey = json.value("license_key").toObject();

  // Parse instance object
  auto instanceId = instance.value("id").toString();
  auto instanceName = instance.value("name").toString();
  auto activationDate = instance.value("created_at").toVariant().toDateTime();

  // Parse license key object
  auto licenseStatus = licenseKey.value("status").toString();
  auto activationLimit = licenseKey.value("activation_limit").toInt(-1);
  auto activationUsage = licenseKey.value("activation_usage").toInt(-1);

  // Parse meta object
  auto storeId = meta.value("store_id").toInteger();
  auto productId = meta.value("product_id").toInteger();
  auto variantName = meta.value("variant_name").toString();
  auto customerName = meta.value("customer_name").toString();
  auto customerEmail = meta.value("customer_email").toString();

  // Empty JSON
  if (json.isEmpty())
    return;

  // Non-null error
  if (!error.isNull() && !error.toString().simplified().isEmpty())
  {
    qWarning() << "[LemonSqueezy] Validation error:" << error.toString();
    Misc::Utilities::showMessageBox(
        tr("There was an issue validating your license."), error.toString(),
        QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Validate that minimum things exist
  bool ok = true;
  ok &= json.contains("meta");
  ok &= json.contains("instance");
  ok &= meta.contains("store_id");
  ok &= meta.contains("product_id");
  if (!ok)
    return;

  // Validate that store ID and product ID match
  if (storeId != STORE_ID || productId != PRDCT_ID)
  {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    Misc::Utilities::showMessageBox(
        tr("The license key you provided does not belong to Serial Studio."),
        tr("Please double-check that you purchased your license from the "
           "official Serial Studio store."),
        QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Validate that instance name is equal to the machine ID
  if (instanceName != MachineID::instance().machineId())
  {
    qWarning() << "[LemonSqueezy] Machine ID mismatch";
    Misc::Utilities::showMessageBox(
        tr("This license key was activated on a different device."),
        tr("Please deactivate it there first or contact support for help."),
        QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Validate license status
  if (licenseStatus != "active")
  {
    qWarning() << "[LemonSqueezy] License status is not active:"
               << licenseStatus;

    Misc::Utilities::showMessageBox(
        tr("This license is not currently active."),
        tr("It may have expired or been deactivated (status: %1).")
            .arg(licenseStatus),
        QMessageBox::Warning);

    clearLicenseCache();
    return;
  }

  // Validate instance ID
  if (instanceId.isEmpty())
  {
    qWarning() << "[LemonSqueezy] Activation response missing instance ID";
    Misc::Utilities::showMessageBox(tr("Something went wrong on the server."),
                                    tr("No activation ID was returned."),
                                    QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Check that the license is valid
  if (!valid)
  {
    qWarning() << "[LemonSqueezy] Validation failed";
    Misc::Utilities::showMessageBox(
        tr("Could not validate your license at this time."),
        tr("Please try again later."), QMessageBox::Warning);
    clearLicenseCache();
    return;
  }

  // Everything ok, update internal members & write settings
  m_activated = true;
  m_licensingData = json;
  m_instanceId = instanceId;
  m_variantName = variantName;
  m_instanceName = instanceName;
  m_seatLimit = activationLimit;
  m_seatUsage = activationUsage;
  m_customerName = customerName;
  m_customerEmail = customerEmail;
  m_activationDate = activationDate;

  // Set application name if variant name is present
  auto list = variantName.split("-");
  if (list.count() >= 2)
  {
    if (list.first().simplified() != "Pro")
      m_appName = tr("%1 %2").arg(APP_NAME, list.first().simplified());
    else
      m_appName = tr("%1 (%2)").arg(APP_NAME, list.last().simplified());
  }

  // Default to APP_NAME otherwise
  else
    m_appName = APP_NAME;

  // Update user interface
  m_busy = false;
  Q_EMIT busyChanged();
  Q_EMIT licenseDataChanged();

  // Update validation date time if response is from server
  if (!cachedResponse)
  {
    auto dt = QDateTime::currentDateTime().toString(Qt::RFC2822Date);
    m_settings.beginGroup("licensing");
    m_settings.setValue("lastCheck", m_simpleCrypt.encryptToString(dt));
    m_settings.endGroup();
  }

  // Display activation successfull message
  if (!m_silentValidation)
  {
    m_silentValidation = true;
    Q_EMIT activatedChanged();
    Misc::Utilities::showMessageBox(
        tr("Your license has been successfully activated."),
        tr("Thank you for supporting Serial Studio!\n"
           "You now have access to all premium features."),
        QMessageBox::Information);
  }
}

/**
 * @brief Processes the response from the license activation request.
 *
 * Checks that activation succeeded, the license belongs to the correct
 * store/product, and that it's tied to the device executing Serial Studio.
 *
 * If valid, triggers a follow-up validation to finalize activation.
 *
 * Any mismatch or error results in user notification and local cleanup.
 *
 * @param data Raw JSON payload returned by Lemon Squeezy API.
 */
void Licensing::LemonSqueezy::readActivationResponse(const QByteArray &data)
{
  // Data is empty, log & abort
  if (data.isEmpty())
  {
    qWarning() << "[LemonSqueezy] Empty activation response";
    clearLicenseCache();
    return;
  }

  // Parse reply as JSON
  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache();
    return;
  }

  // Obtain JSON object
  auto json = doc.object();
  auto error = json.value("error");
  auto meta = json.value("meta").toObject();
  auto instance = json.value("instance").toObject();
  auto licenseKey = json.value("license_key").toObject();
  auto activated = json.value("activated").toBool(false);

  // Parse instance object
  auto instanceId = instance.value("id").toString();
  auto instanceName = instance.value("name").toString();

  // Parse license object
  auto licenseStatus = licenseKey.value("status").toString();

  // Parse meta object
  auto storeId = meta.value("store_id").toInteger();
  auto productId = meta.value("product_id").toInteger();

  // Non-null error
  if (!error.isNull())
  {
    qWarning() << "[LemonSqueezy] Activation error:" << error.toString();
    Misc::Utilities::showMessageBox(
        tr("There was an issue activating your license."), error.toString(),
        QMessageBox::Critical);
    clearLicenseCache(true);
    return;
  }

  // Validate that store ID and product ID match
  if (storeId != STORE_ID || productId != PRDCT_ID)
  {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    Misc::Utilities::showMessageBox(
        tr("The license key you provided does not belong to Serial Studio."),
        tr("Please double-check that you purchased your license from the "
           "official Serial Studio store."),
        QMessageBox::Critical);
    clearLicenseCache(true);
    return;
  }

  // Validate that instance name is equal to the machine ID
  if (instanceName != MachineID::instance().machineId())
  {
    qWarning() << "[LemonSqueezy] Machine ID mismatch";
    Misc::Utilities::showMessageBox(
        tr("This license key was activated on a different device."),
        tr("Please deactivate it there first or contact support for help."),
        QMessageBox::Critical);
    clearLicenseCache(true);
    return;
  }

  // Validate license status
  if (licenseStatus != "active")
  {
    qWarning() << "[LemonSqueezy] License status is not active:"
               << licenseStatus;

    Misc::Utilities::showMessageBox(
        tr("This license is not currently active."),
        tr("It may have expired or been deactivated (status: %1).")
            .arg(licenseStatus),
        QMessageBox::Warning);

    clearLicenseCache(true);
    return;
  }

  // Validate instance ID
  if (instanceId.isEmpty())
  {
    qWarning() << "[LemonSqueezy] Activation response missing instance ID";
    Misc::Utilities::showMessageBox(tr("Something went wrong on the server..."),
                                    tr("No activation ID was returned."),
                                    QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Check that the product was activated
  if (!activated)
  {
    qWarning() << "[LemonSqueezy] Activation failed";
    Misc::Utilities::showMessageBox(
        tr("Could not activate your license at this time."),
        tr("Please try again later."), QMessageBox::Warning);
    clearLicenseCache();
    return;
  }

  // Create a validation call to save settings
  QMetaObject::invokeMethod(this, [=, this] {
    m_busy = false;
    m_instanceId = instanceId;
    m_silentValidation = false;
    validate();
  });
}

/**
 * @brief Processes the response from the license deactivation request.
 *
 * Ensures the license was successfully deactivated on the server,
 * and then clears all local activation data.
 *
 * Displays a final message confirming the deactivation to the user.
 *
 * @param data Raw JSON payload returned by Lemon Squeezy API.
 */
void Licensing::LemonSqueezy::readDeactivationResponse(const QByteArray &data)
{
  // Data is empty, log & abort
  if (data.isEmpty())
  {
    qWarning() << "[LemonSqueezy] Empty activation response";
    clearLicenseCache();
    return;
  }

  // Parse reply as JSON
  QJsonParseError parseError;
  auto doc = QJsonDocument::fromJson(data, &parseError);
  if (parseError.error != QJsonParseError::NoError)
  {
    qWarning() << "[LemonSqueezy] JSON parse error" << parseError.errorString();
    clearLicenseCache();
    return;
  }

  // Obtain JSON object
  auto json = doc.object();
  auto error = json.value("error");
  auto meta = json.value("meta").toObject();
  auto deactivated = json.value("deactivated").toBool(false);

  // Parse meta object
  auto storeId = meta.value("store_id").toInteger();
  auto productId = meta.value("product_id").toInteger();

  // Non-null error
  if (!error.isNull())
  {
    qWarning() << "[LemonSqueezy] Deactivation error:" << error.toString();
    Misc::Utilities::showMessageBox(
        tr("There was an issue deactivating your license."), error.toString(),
        QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Validate that store ID and product ID match
  if (storeId != STORE_ID || productId != PRDCT_ID)
  {
    qWarning() << "[LemonSqueezy] Store ID or Product ID mismatch";
    Misc::Utilities::showMessageBox(
        tr("The license key you provided does not belong to Serial Studio."),
        tr("Please double-check that you purchased your license from the "
           "official Serial Studio store."),
        QMessageBox::Critical);
    clearLicenseCache();
    return;
  }

  // Check that the product was deactivated
  if (!deactivated)
  {
    qWarning() << "[LemonSqueezy] Deactivation failed";
    Misc::Utilities::showMessageBox(
        tr("Could not deactivate your license at this time."),
        tr("Please try again later."), QMessageBox::Warning);
    clearLicenseCache();
    return;
  }

  // De-activate the product
  clearLicenseCache(true);
  Q_EMIT activatedChanged();
  Misc::Utilities::showMessageBox(
      tr("Your license has been deactivated."),
      tr("Access to Pro features has been removed.\n"
         "Thank you again for supporting Serial Studio!"),
      QMessageBox::Information);
}
