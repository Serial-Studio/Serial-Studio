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
#include "Licensing/Trial.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

void API::Handlers::LicensingHandler::registerCommands()
{
  // Obtain registry and register licensing commands
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

  // activate schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.activate"),
                             QStringLiteral("Activate the stored license key against the server"),
                             emptySchema,
                             &activate);
  }

  // deactivate schema (no params)
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

  // validate schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.validate"),
                             QStringLiteral("Re-validate the current license with the server"),
                             emptySchema,
                             &validate);
  }

  // getStatus schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.getStatus"),
                             QStringLiteral("Get current license activation status"),
                             emptySchema,
                             &getStatus);
  }

  // guardStatus schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.guardStatus"),
                             QStringLiteral("Run all build-time license guards and report results"),
                             emptySchema,
                             &guardStatus);
  }

  // trialGetStatus schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.trial.getStatus"),
                             QStringLiteral("Get current trial status"),
                             emptySchema,
                             &trialGetStatus);
  }

  // trialEnable schema (no params)
  {
    QJsonObject emptySchema;
    emptySchema.insert("type", "object");
    emptySchema.insert("properties", QJsonObject());
    registry.registerCommand(QStringLiteral("licensing.trial.enable"),
                             QStringLiteral("Start the trial period for this machine"),
                             emptySchema,
                             &trialEnable);
  }
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

API::CommandResponse API::Handlers::LicensingHandler::setLicense(const QString& id,
                                                                 const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("licenseKey"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: licenseKey"));
  }

  Licensing::LemonSqueezy::instance().setLicense(
    params.value(QStringLiteral("licenseKey")).toString());

  return CommandResponse::makeSuccess(id, {});
}

API::CommandResponse API::Handlers::LicensingHandler::activate(const QString& id,
                                                               const QJsonObject&)
{
  // Obtain instance and validate state
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

API::CommandResponse API::Handlers::LicensingHandler::deactivate(const QString& id,
                                                                 const QJsonObject&)
{
  // Obtain instance and validate state
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

API::CommandResponse API::Handlers::LicensingHandler::validate(const QString& id,
                                                               const QJsonObject&)
{
  // Obtain instance and validate state
  auto& ls = Licensing::LemonSqueezy::instance();

  if (ls.busy()) {
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("Licensing operation already in progress"));
  }

  ls.validate();
  return CommandResponse::makeSuccess(id, {});
}

API::CommandResponse API::Handlers::LicensingHandler::getStatus(const QString& id,
                                                                const QJsonObject&)
{
  // Obtain instance and validate state
  const auto& ls = Licensing::LemonSqueezy::instance();
  const auto& tk = Licensing::CommercialToken::current();

  // Map feature tier to human-readable name
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

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Runs every generated license guard and reports individual pass/fail.
 *
 * This is a diagnostic command for CI: it proves the built binary has a
 * consistent salt embedded in all guard functions. If any guard fails, the
 * build is broken and must not ship.
 */
API::CommandResponse API::Handlers::LicensingHandler::guardStatus(const QString& id,
                                                                  const QJsonObject&)
{
  // Obtain instance and validate state
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

API::CommandResponse API::Handlers::LicensingHandler::trialGetStatus(const QString& id,
                                                                     const QJsonObject&)
{
  // Obtain instance and validate state
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

API::CommandResponse API::Handlers::LicensingHandler::trialEnable(const QString& id,
                                                                  const QJsonObject&)
{
  // Obtain instance and validate state
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
