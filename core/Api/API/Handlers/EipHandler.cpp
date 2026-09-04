/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2026 Alex Spataru
 *
 * This file is part of Serial Studio Pro. All rights reserved.
 *
 * SPDX-License-Identifier: LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/EipHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "Core/SSAssert.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/EthernetIp.h"

/**
 * @brief Validates one driver property against its declared descriptor, so an unknown key or an
 *        out-of-type/range value is refused before it reaches the driver's clamping setter. Returns
 *        false with @p reason set to the specific failure.
 */
[[nodiscard]] static bool eipValidateProperty(IO::Drivers::EthernetIp* driver,
                                              const QString& key,
                                              const QVariant& value,
                                              QString& reason)
{
  SS_ASSERT(driver != nullptr, return false);

  const auto props = driver->driverProperties();
  SS_ASSERT_LOG(!props.isEmpty());

  const IO::DriverProperty* p = nullptr;
  for (const auto& prop : props)
    if (prop.key == key) {
      p = &prop;
      break;
    }

  if (!p) {
    reason = QStringLiteral("Unknown property key: %1").arg(key);
    return false;
  }

  if (p->type == IO::DriverProperty::IntField) {
    bool ok     = false;
    const int n = value.toInt(&ok);
    if (!ok) {
      reason = QStringLiteral("Property \"%1\" expects an integer").arg(key);
      return false;
    }

    const int lo = p->min.toInt();
    const int hi = p->max.toInt();
    if ((p->min.isValid() && n < lo) || (p->max.isValid() && n > hi)) {
      reason = QStringLiteral("Property \"%1\" must be between %2 and %3")
                 .arg(key, QString::number(lo), QString::number(hi));
      return false;
    }

    return true;
  }

  if (p->type == IO::DriverProperty::ComboBox) {
    if (value.typeId() == QMetaType::QString) {
      const auto text = value.toString();
      if (IO::Drivers::EthernetIp::plcTypeList().contains(text))
        return true;

      reason = QStringLiteral("Unknown controller family: %1").arg(text);
      return false;
    }

    bool ok       = false;
    const int idx = value.toInt(&ok);
    if (!ok || idx < 0 || idx >= p->options.size()) {
      reason = QStringLiteral("Controller family index out of range");
      return false;
    }

    return true;
  }

  return true;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Registers every io.eip.* command with the registry.
 */
void API::Handlers::EipHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("io.eip.getStatus"),
                           QStringLiteral("Get the EtherNet/IP session status and counters"),
                           API::emptySchema(),
                           &getStatus);

  registry.registerCommand(QStringLiteral("io.eip.getConfig"),
                           QStringLiteral("Get the EtherNet/IP driver configuration"),
                           API::emptySchema(),
                           &getConfig);

  {
    const API::SchemaProp key{
      QStringLiteral("key"),
      QStringLiteral("string"),
      QStringLiteral("Property key (host, cipPath, plcType, pollInterval)")};
    const API::SchemaProp value{QStringLiteral("value"),
                                QStringLiteral("string|integer"),
                                QStringLiteral("New property value")};
    registry.registerCommand(
      QStringLiteral("io.eip.setProperty"),
      QStringLiteral("Set an EtherNet/IP driver property (params: key, value)"),
      API::makeSchema({key, value}),
      &setProperty);
  }

  {
    const API::SchemaProp tag{
      QStringLiteral("tag"), QStringLiteral("string"), QStringLiteral("CIP symbolic tag name")};
    API::SchemaProp type{QStringLiteral("type"),
                         QStringLiteral("string"),
                         QStringLiteral("Wire type code (bool, i16, i32, f32, str, ...)")};
    type.enumValues = QJsonArray::fromStringList(IO::Drivers::EthernetIp::tagTypeList());

    const API::SchemaProp name{QStringLiteral("name"),
                               QStringLiteral("string"),
                               QStringLiteral("Channel name shown on the dashboard")};
    const auto element = API::rangeProp(QStringLiteral("element"),
                                        QStringLiteral("Array element index, or -1 for a scalar"),
                                        -1,
                                        65535);
    registry.registerCommand(QStringLiteral("io.eip.addTag"),
                             QStringLiteral("Add a CIP tag (params: name, tag, type, element)"),
                             API::makeSchema({tag, type}, {name, element}),
                             &addTag);
  }

  {
    const auto index = API::rangeProp(QStringLiteral("index"),
                                      QStringLiteral("Zero-based position in the tag list"),
                                      0,
                                      IO::Drivers::OpcUaWire::kMaxTags - 1);
    registry.registerCommand(QStringLiteral("io.eip.removeTag"),
                             QStringLiteral("Remove a CIP tag by index (params: index)"),
                             API::makeSchema({index}),
                             &removeTag);
  }

  registry.registerCommand(QStringLiteral("io.eip.clearTags"),
                           QStringLiteral("Remove every configured CIP tag"),
                           API::emptySchema(),
                           &clearTags);

  registry.registerCommand(QStringLiteral("io.eip.generateProject"),
                           QStringLiteral("Build a project from the configured CIP tags"),
                           API::emptySchema(),
                           &generateProject);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the pulled diagnostics snapshot of the live session.
 */
API::CommandResponse API::Handlers::EipHandler::getStatus(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  return CommandResponse::makeSuccess(id, manager.ethernetIp()->statusJson());
}

/**
 * @brief Returns the endpoint configuration and the tag list.
 */
API::CommandResponse API::Handlers::EipHandler::getConfig(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();

  QJsonObject result;
  result[QStringLiteral("host")]         = driver->host();
  result[QStringLiteral("cipPath")]      = driver->cipPath();
  result[QStringLiteral("plcType")]      = driver->plcType();
  result[QStringLiteral("pollInterval")] = driver->pollInterval();
  result[QStringLiteral("tags")]         = driver->tagsJson();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Applies one driver property through the generic round-trip surface.
 */
API::CommandResponse API::Handlers::EipHandler::setProperty(const QString& id,
                                                            const QJsonObject& params)
{
  const auto key = params.value(QStringLiteral("key")).toString();
  if (key.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: key"));
  }

  if (!params.contains(QStringLiteral("value"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: value"));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();

  const auto value = params.value(QStringLiteral("value")).toVariant();
  QString reason;
  if (!eipValidateProperty(driver, key, value, reason))
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, reason);

  driver->setDriverProperty(key, value);

  QJsonObject result;
  result[QStringLiteral("key")] = key;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Appends one CIP tag; an unknown wire type is refused up front with a specific reason so a
 *        bad API parameter never reaches the driver's user-facing problem log.
 */
API::CommandResponse API::Handlers::EipHandler::addTag(const QString& id, const QJsonObject& params)
{
  const auto tag  = params.value(QStringLiteral("tag")).toString();
  const auto type = params.value(QStringLiteral("type")).toString();
  if (tag.isEmpty() || type.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: tag or type"));
  }

  if (IO::Drivers::OpcUaWire::typeFromCode(type) == IO::Drivers::OpcUaWire::Type::Invalid) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown tag type: %1").arg(type));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();

  const int before = driver->tagCount();
  driver->addTag(params.value(QStringLiteral("name")).toString(),
                 tag,
                 type,
                 params.value(QStringLiteral("element")).toInt(-1));
  if (driver->tagCount() == before) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The tag could not be added"));
  }

  QJsonObject result;
  result[QStringLiteral("tagCount")] = driver->tagCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Removes the tag at the given position.
 */
API::CommandResponse API::Handlers::EipHandler::removeTag(const QString& id,
                                                          const QJsonObject& params)
{
  const auto indexValue = params.value(QStringLiteral("index"));
  if (!params.contains(QStringLiteral("index"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: index"));
  }

  if (!indexValue.isDouble()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Parameter \"index\" must be an integer"));
  }

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();

  const int index = indexValue.toInt();
  if (index < 0 || index >= driver->tagCount()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Invalid tag index"));
  }

  driver->removeTag(index);

  QJsonObject result;
  result[QStringLiteral("tagCount")] = driver->tagCount();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Drops every configured tag.
 */
API::CommandResponse API::Handlers::EipHandler::clearTags(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();
  driver->clearTags();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("tagCount"), driver->tagCount()}
  });
}

/**
 * @brief Builds a project from the tag list and loads it into the editor.
 */
API::CommandResponse API::Handlers::EipHandler::generateProject(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& manager = IO::ConnectionManager::instance();
  auto* driver         = manager.ethernetIp();
  if (!driver->loadGeneratedProject()) {
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("The project could not be generated"));
  }

  QJsonObject result;
  result[QStringLiteral("datasets")] = driver->wireSchema().size();
  return CommandResponse::makeSuccess(id, result);
}
