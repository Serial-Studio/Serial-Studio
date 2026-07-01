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

#include "API/Handlers/LicensingHandler.h"

#include "API/CommandRegistry.h"
#include "Licensing/CommercialToken.h"
#include "Licensing/LemonSqueezy.h"
#include "Licensing/OfflineLicense.h"
#include "Licensing/Trial.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all licensing commands with the registry.
 */
void API::Handlers::LicensingHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  {
    QJsonObject props;
    props[QStringLiteral("licenseKey")] = QJsonObject{
      {       QStringLiteral("type"),           QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("License key UUID")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("licenseKey")};
    registry.registerCommand(QStringLiteral("licensing.setLicense"),
                             QStringLiteral("Set the license key (params: licenseKey)"),
                             schema,
                             &setLicense);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.activate"),
                             QStringLiteral("Activate the stored license key against the server"),
                             emptySchema,
                             &activate);
  }

  registerOfflineCommand();

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(
      QStringLiteral("licensing.deactivate"),
      QStringLiteral("Deactivate the license on this machine, freeing a seat"),
      emptySchema,
      &deactivate);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.validate"),
                             QStringLiteral("Re-validate the current license with the server"),
                             emptySchema,
                             &validate);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.getStatus"),
                             QStringLiteral("Get current license activation status"),
                             emptySchema,
                             &getStatus);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.getGuardStatus"),
                             QStringLiteral("Run all build-time license guards and report results"),
                             emptySchema,
                             &guardStatus);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.getTrialStatus"),
                             QStringLiteral("Get current trial status"),
                             emptySchema,
                             &trialGetStatus);
  }

  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.enableTrial"),
                             QStringLiteral("Start the trial period for this machine"),
                             emptySchema,
                             &trialEnable);
  }
}

/**
 * @brief Registers the offline certificate activation command.
 */
void API::Handlers::LicensingHandler::registerOfflineCommand()
{
  auto& registry = CommandRegistry::instance();

  QJsonObject props;
  props[QStringLiteral("path")] = QJsonObject{
    {       QStringLiteral("type"),                       QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("Path to a .sslic certificate")}
  };

  QJsonObject schema;
  schema[QStringLiteral("type")]       = QStringLiteral("object");
  schema[QStringLiteral("properties")] = props;
  schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("path")};

  registry.registerCommand(
    QStringLiteral("licensing.activateOffline"),
    QStringLiteral("Activate Pro from an offline license certificate (params: path)"),
    schema,
    &activateOffline);
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Store the supplied license key for later activation.
 */
API::CommandResponse API::Handlers::LicensingHandler::setLicense(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("licenseKey"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: licenseKey"));
  }

  Licensing::LemonSqueezy::instance().setLicense(
    params.value(QStringLiteral("licenseKey")).toString());

  return CommandResponse::makeSuccess(id, {});
}

/**
 * @brief Activate Pro from a signed offline certificate file, with no network access.
 */
API::CommandResponse API::Handlers::LicensingHandler::activateOffline(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("path"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: path"));
  }

  auto& offline = Licensing::OfflineLicense::instance();
  if (!offline.activateFromFile(params.value(QStringLiteral("path")).toString()))
    return CommandResponse::makeError(id, ErrorCode::ExecutionError, offline.lastError());

  return CommandResponse::makeSuccess(id, {});
}

/**
 * @brief Activate the stored license key against the LemonSqueezy server.
 */
API::CommandResponse API::Handlers::LicensingHandler::activate(const QString& id,
                                                               const QJsonObject&)
{
  auto& ls = Licensing::LemonSqueezy::instance();

  if (!ls.canActivate()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("No valid license key set (must be 36 chars)"));
  }

  if (ls.busy()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Licensing operation already in progress"));
  }

  ls.activate();
  return CommandResponse::makeSuccess(id, {});
}

/**
 * @brief Deactivate the current license on this machine, freeing a seat.
 */
API::CommandResponse API::Handlers::LicensingHandler::deactivate(const QString& id,
                                                                 const QJsonObject&)
{
  auto& ls = Licensing::LemonSqueezy::instance();

  if (!ls.isActivated()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("No active license to deactivate"));
  }

  if (ls.busy()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Licensing operation already in progress"));
  }

  ls.deactivate();
  return CommandResponse::makeSuccess(id, {});
}

/**
 * @brief Re-validate the active license against the LemonSqueezy server.
 */
API::CommandResponse API::Handlers::LicensingHandler::validate(const QString& id,
                                                               const QJsonObject&)
{
  auto& ls = Licensing::LemonSqueezy::instance();

  if (ls.busy()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Licensing operation already in progress"));
  }

  ls.validate();
  return CommandResponse::makeSuccess(id, {});
}

/**
 * @brief Get the current license activation state and customer info.
 */
API::CommandResponse API::Handlers::LicensingHandler::getStatus(const QString& id,
                                                                const QJsonObject&)
{
  const auto& ls = Licensing::LemonSqueezy::instance();
  const auto& ol = Licensing::OfflineLicense::instance();
  const auto& tk = Licensing::CommercialToken::current();

  QString tierName;
  switch (tk.featureTier()) {
    case Licensing::FeatureTier::Hobbyist:
      tierName = QStringLiteral("Hobbyist");
      break;
    case Licensing::FeatureTier::Trial:
      tierName = QStringLiteral("Trial");
      break;
    case Licensing::FeatureTier::Pro:
      tierName = QStringLiteral("Pro");
      break;
    case Licensing::FeatureTier::Enterprise:
      tierName = QStringLiteral("Enterprise");
      break;
    default:
      tierName = QStringLiteral("None");
      break;
  }

  QJsonObject result;
  result[QStringLiteral("busy")]             = ls.busy();
  result[QStringLiteral("isActivated")]      = ls.isActivated();
  result[QStringLiteral("canActivate")]      = ls.canActivate();
  result[QStringLiteral("appName")]          = ls.appName();
  result[QStringLiteral("license")]          = ls.license();
  result[QStringLiteral("instanceId")]       = ls.instanceId();
  result[QStringLiteral("variantName")]      = ls.variantName();
  result[QStringLiteral("instanceName")]     = ls.instanceName();
  result[QStringLiteral("customerName")]     = ls.customerName();
  result[QStringLiteral("customerEmail")]    = ls.customerEmail();
  result[QStringLiteral("seatLimit")]        = ls.seatLimit();
  result[QStringLiteral("seatUsage")]        = ls.seatUsage();
  result[QStringLiteral("featureTier")]      = tierName;
  result[QStringLiteral("featureTierValue")] = static_cast<int>(tk.featureTier());

  result[QStringLiteral("machineId")]            = ol.machineId();
  result[QStringLiteral("offlineActivated")]     = ol.isActivated();
  result[QStringLiteral("offlineDaysRemaining")] = ol.daysRemaining();
  result[QStringLiteral("offlineExpiresAt")] =
    ol.isActivated() ? ol.expiresAt().toString(Qt::ISODate) : QString();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Runs every generated license guard and reports individual pass/fail.
 */
API::CommandResponse API::Handlers::LicensingHandler::guardStatus(const QString& id,
                                                                  const QJsonObject&)
{
  const auto& table = Licensing::Guards::guardTable();
  const int count   = static_cast<int>(table.size());

  int passed = 0;
  int failed = 0;
  QJsonArray failures;

  for (int i = 0; i < count; ++i) {
    if (table[static_cast<unsigned>(i)]())
      ++passed;
    else {
      ++failed;
      failures.append(i);
    }
  }

  QJsonObject result;
  result[QStringLiteral("total")]    = count;
  result[QStringLiteral("passed")]   = passed;
  result[QStringLiteral("failed")]   = failed;
  result[QStringLiteral("failures")] = failures;
  result[QStringLiteral("allOk")]    = (failed == 0);

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get the current trial status and remaining days.
 */
API::CommandResponse API::Handlers::LicensingHandler::trialGetStatus(const QString& id,
                                                                     const QJsonObject&)
{
  const auto& trial = Licensing::Trial::instance();

  QJsonObject result;
  result[QStringLiteral("busy")]           = trial.busy();
  result[QStringLiteral("firstRun")]       = trial.firstRun();
  result[QStringLiteral("trialEnabled")]   = trial.trialEnabled();
  result[QStringLiteral("trialExpired")]   = trial.trialExpired();
  result[QStringLiteral("trialAvailable")] = trial.trialAvailable();
  result[QStringLiteral("daysRemaining")]  = trial.daysRemaining();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Start the trial period for this machine.
 */
API::CommandResponse API::Handlers::LicensingHandler::trialEnable(const QString& id,
                                                                  const QJsonObject&)
{
  auto& trial = Licensing::Trial::instance();

  if (!trial.trialAvailable()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::ExecutionError,
      QStringLiteral("Trial is not available (license already active or trial already used)"));
  }

  if (trial.busy()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Trial operation already in progress"));
  }

  trial.enableTrial();
  return CommandResponse::makeSuccess(id, {});
}
