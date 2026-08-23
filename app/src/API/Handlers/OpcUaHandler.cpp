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

#include "API/Handlers/OpcUaHandler.h"

#include <QJsonArray>
#include <QUrl>

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
#include "IO/ConnectionManager.h"
#include "IO/Drivers/OpcUaTagModel.h"

/**
 * @brief The UI-config OPC UA driver every command operates on.
 */
[[nodiscard]] static IO::Drivers::OpcUa* opcUaDriver()
{
  static auto& connectionManager = IO::ConnectionManager::instance();
  return connectionManager.opcUa();
}

/**
 * @brief Standard missing-parameter error.
 */
[[nodiscard]] static API::CommandResponse missingParam(const QString& id, const char* name)
{
  return API::CommandResponse::makeError(
    id,
    API::ErrorCode::MissingParam,
    QStringLiteral("Missing required parameter: %1").arg(QLatin1String(name)));
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all OPC UA commands with the registry
 */
void API::Handlers::OpcUaHandler::registerCommands()
{
  static auto& registry = CommandRegistry::instance();
  registerConfigCommands(registry);
  registerDiscoveryCommands(registry);
  registerTagCommands(registry);
  registerQueryCommands(registry);
}

/**
 * @brief Register endpoint / authentication / rate mutation commands.
 */
void API::Handlers::OpcUaHandler::registerConfigCommands(CommandRegistry& registry)
{
  registry.registerCommand(
    QStringLiteral("io.opcua.setEndpointUrl"),
    QStringLiteral("Set the OPC UA endpoint URL (params: url, e.g. opc.tcp://host:4840/path)"),
    API::makeSchema({
      {QStringLiteral("url"),
       QStringLiteral("string"),
       QStringLiteral("opc.tcp:// endpoint URL")}
  }),
    &setEndpointUrl);
  registry.registerCommand(
    QStringLiteral("io.opcua.setEndpointIndex"),
    QStringLiteral("Select a discovered endpoint by row (params: index; -1 = typed URL)"),
    API::makeSchema({
      {QStringLiteral("index"),
       QStringLiteral("integer"),
       QStringLiteral("Row in io.opcua.listEndpoints, or -1 for the typed URL")}
  }),
    &setEndpointIndex);
  registry.registerCommand(
    QStringLiteral("io.opcua.setAuthMode"),
    QStringLiteral("Set authentication (params: mode - 0=anonymous, 1=username/password)"),
    API::makeSchema({
      {QStringLiteral("mode"),
       QStringLiteral("integer"),
       QStringLiteral("0 = anonymous, 1 = username/password")}
  }),
    &setAuthMode);
  registry.registerCommand(QStringLiteral("io.opcua.setUsername"),
                           QStringLiteral("Set the username (params: username)"),
                           API::makeSchema({
                             {QStringLiteral("username"),
                              QStringLiteral("string"),
                              QStringLiteral("Username for mode 1")}
  }),
                           &setUsername);
  registry.registerCommand(
    QStringLiteral("io.opcua.setPassword"),
    QStringLiteral("Set the password (params: password; stored in the encrypted vault)"),
    API::makeSchema({
      {QStringLiteral("password"),
       QStringLiteral("string"),
       QStringLiteral("Password for mode 1")}
  }),
    &setPassword);
  registry.registerCommand(
    QStringLiteral("io.opcua.setPublishingInterval"),
    QStringLiteral("Set the publishing interval in milliseconds (params: intervalMs)"),
    API::makeSchema({
      {QStringLiteral("intervalMs"),
       QStringLiteral("integer"),
       QStringLiteral("Publishing interval in milliseconds (10-60000)")}
  }),
    &setPublishingInterval);
}

/**
 * @brief Register endpoint discovery and address-space browsing commands (async: kick, then poll).
 */
void API::Handlers::OpcUaHandler::registerDiscoveryCommands(CommandRegistry& registry)
{
  const auto empty = API::emptySchema();

  registry.registerCommand(
    QStringLiteral("io.opcua.discoverEndpoints"),
    QStringLiteral("Request the server's endpoints; poll io.opcua.listEndpoints for the result"),
    empty,
    &discoverEndpoints);
  registry.registerCommand(
    QStringLiteral("io.opcua.listEndpoints"),
    QStringLiteral("Get discovered endpoints with their policy, mode and selectability"),
    empty,
    &listEndpoints);
  registry.registerCommand(
    QStringLiteral("io.opcua.startBrowse"),
    QStringLiteral("Open a browse session; poll io.opcua.browse once browsing is true"),
    empty,
    &startBrowse);
  registry.registerCommand(
    QStringLiteral("io.opcua.browse"),
    QStringLiteral("Fetch one level of the address space (params: nodeId; empty = Objects)"),
    API::makeSchema({
      {QStringLiteral("nodeId"),
       QStringLiteral("string"),
       QStringLiteral("Folder node id (empty for the Objects folder)")}
  }),
    &browse);
  registry.registerCommand(
    QStringLiteral("io.opcua.stopBrowse"),
    QStringLiteral("Close the browse session and commit the picker selection to the tag list"),
    empty,
    &stopBrowse);
}

/**
 * @brief Register tag-list mutation commands and project generation.
 */
void API::Handlers::OpcUaHandler::registerTagCommands(CommandRegistry& registry)
{
  const auto empty = API::emptySchema();

  registry.registerCommand(
    QStringLiteral("io.opcua.setTags"),
    QStringLiteral("Replace the tag list (params: tags - array of {id, name, path, unit, t, n})"),
    API::makeSchema({
      {QStringLiteral("tags"),
       QStringLiteral("array"),
       QStringLiteral(
         "Tag objects: id (node id), name, path, unit, t (type code), n (array len)")}
  }),
    &setTags);
  registry.registerCommand(
    QStringLiteral("io.opcua.addTag"),
    QStringLiteral("Append one tag (params: id, name, t; optional path, unit, n)"),
    API::makeSchema({
      {  QStringLiteral("id"),QStringLiteral("string"),QStringLiteral("Variable node id")                                                        },
      {QStringLiteral("name"), QStringLiteral("string"),                          QStringLiteral("Display name")},
      {   QStringLiteral("t"),
       QStringLiteral("string"),
       QStringLiteral("Type code: bool,i8,u8,i16,u16,i32,u32,i64,u64,f32,f64,str")                              },
  }),
    &addTag);
  registry.registerCommand(QStringLiteral("io.opcua.removeTag"),
                           QStringLiteral("Remove a tag by position (params: index)"),
                           API::makeSchema({
                             {QStringLiteral("index"),
                              QStringLiteral("integer"),
                              QStringLiteral("Zero-based tag position")}
  }),
                           &removeTag);
  registry.registerCommand(
    QStringLiteral("io.opcua.clearTags"), QStringLiteral("Remove every tag"), empty, &clearTags);
  registry.registerCommand(
    QStringLiteral("io.opcua.generateProject"),
    QStringLiteral("Generate a project from the tag list and load it into the editor"),
    empty,
    &generateProject);
}

/**
 * @brief Register read-only queries.
 */
void API::Handlers::OpcUaHandler::registerQueryCommands(CommandRegistry& registry)
{
  const auto empty = API::emptySchema();

  registry.registerCommand(QStringLiteral("io.opcua.getConfig"),
                           QStringLiteral("Get current OPC UA configuration"),
                           empty,
                           &getConfiguration);
  registry.registerCommand(QStringLiteral("io.opcua.listTags"),
                           QStringLiteral("Get the configured tag list and wire schema"),
                           empty,
                           &listTags);
  registry.registerCommand(QStringLiteral("io.opcua.getStatus"),
                           QStringLiteral("Get session status and pulled diagnostics counters"),
                           empty,
                           &getStatus);
}

//--------------------------------------------------------------------------------------------------
// Configuration setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Set the endpoint URL
 */
API::CommandResponse API::Handlers::OpcUaHandler::setEndpointUrl(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("url")))
    return missingParam(id, "url");

  const QString url = params.value(QStringLiteral("url")).toString();
  const QUrl parsed(url);
  if (!url.startsWith(QLatin1String("opc.tcp://")) || !parsed.isValid() || parsed.host().isEmpty())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("url must be a valid opc.tcp://host[:port][/path] endpoint"));

  opcUaDriver()->setEndpointUrl(url);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("url"), url}
  });
}

/**
 * @brief Select a discovered endpoint row
 */
API::CommandResponse API::Handlers::OpcUaHandler::setEndpointIndex(const QString& id,
                                                                   const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("index")))
    return missingParam(id, "index");

  const int index = params.value(QStringLiteral("index")).toInt();
  auto* driver    = opcUaDriver();
  driver->setEndpointIndex(index);
  if (driver->endpointIndex() != index)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Endpoint row is unknown or not selectable"));

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("index"), index}
  });
}

/**
 * @brief Set the authentication mode
 */
API::CommandResponse API::Handlers::OpcUaHandler::setAuthMode(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("mode")))
    return missingParam(id, "mode");

  const int mode = params.value(QStringLiteral("mode")).toInt();
  if (mode < 0 || mode > 1)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("mode must be 0 or 1"));

  opcUaDriver()->setAuthMode(mode);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("mode"), mode}
  });
}

/**
 * @brief Set the username
 */
API::CommandResponse API::Handlers::OpcUaHandler::setUsername(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("username")))
    return missingParam(id, "username");

  const QString username = params.value(QStringLiteral("username")).toString();
  opcUaDriver()->setUsername(username);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("username"), username}
  });
}

/**
 * @brief Set the password (never echoed back)
 */
API::CommandResponse API::Handlers::OpcUaHandler::setPassword(const QString& id,
                                                              const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("password")))
    return missingParam(id, "password");

  opcUaDriver()->setPassword(params.value(QStringLiteral("password")).toString());
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("stored"), true}
  });
}

/**
 * @brief Set the publishing interval
 */
API::CommandResponse API::Handlers::OpcUaHandler::setPublishingInterval(const QString& id,
                                                                        const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("intervalMs")))
    return missingParam(id, "intervalMs");

  const int interval = params.value(QStringLiteral("intervalMs")).toInt();
  if (interval < 10 || interval > 60000)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("intervalMs must be between 10 and 60000"));

  auto* driver = opcUaDriver();
  driver->setPublishingInterval(interval);
  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {QStringLiteral("intervalMs"), driver->publishingInterval()}
  });
}

//--------------------------------------------------------------------------------------------------
// Discovery and browsing
//--------------------------------------------------------------------------------------------------

/**
 * @brief Kick an endpoint request
 */
API::CommandResponse API::Handlers::OpcUaHandler::discoverEndpoints(const QString& id,
                                                                    const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  driver->discoverEndpoints();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("discovering"), driver->discovering()}
  });
}

/**
 * @brief Discovered endpoints with selectability
 */
API::CommandResponse API::Handlers::OpcUaHandler::listEndpoints(const QString& id,
                                                                const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver         = opcUaDriver();
  const auto endpoints = driver->endpointsJson();

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {  QStringLiteral("discovering"),   driver->discovering()},
                                        {QStringLiteral("selectedIndex"), driver->endpointIndex()},
                                        {    QStringLiteral("endpoints"),               endpoints},
  });
}

/**
 * @brief Open the browse session
 */
API::CommandResponse API::Handlers::OpcUaHandler::startBrowse(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  driver->startBrowse();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("browsing"), driver->browsing()}
  });
}

/**
 * @brief Fetch (and return what is known of) one folder level
 */
API::CommandResponse API::Handlers::OpcUaHandler::browse(const QString& id,
                                                         const QJsonObject& params)
{
  auto* driver = opcUaDriver();
  if (!driver->browsing())
    return CommandResponse::makeError(
      id,
      ErrorCode::OperationFailed,
      QStringLiteral("No browse session; call io.opcua.startBrowse"));

  const QString nodeId = params.value(QStringLiteral("nodeId")).toString();
  auto* model          = driver->tagModel();
  const bool known     = model->fetchNode(nodeId);
  if (!known && !nodeId.isEmpty())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown node id; browse its parent first"));

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {  QStringLiteral("nodeId"),                      nodeId},
                                        {   QStringLiteral("known"),                       known},
                                        {QStringLiteral("children"), model->childrenJson(nodeId)},
  });
}

/**
 * @brief Close the browse session
 */
API::CommandResponse API::Handlers::OpcUaHandler::stopBrowse(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  driver->stopBrowse();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("tagCount"), driver->tagCount()}
  });
}

//--------------------------------------------------------------------------------------------------
// Tags and project generation
//--------------------------------------------------------------------------------------------------

/**
 * @brief Replace the tag list
 */
API::CommandResponse API::Handlers::OpcUaHandler::setTags(const QString& id,
                                                          const QJsonObject& params)
{
  if (!params.value(QStringLiteral("tags")).isArray())
    return missingParam(id, "tags");

  auto* driver       = opcUaDriver();
  const auto request = params.value(QStringLiteral("tags")).toArray();
  driver->setTags(request);

  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {QStringLiteral("tagCount"),                  driver->tagCount()},
      { QStringLiteral("dropped"), request.size() - driver->tagCount()},
      {QStringLiteral("deferred"),              driver->tagsDeferred()}
  });
}

/**
 * @brief Append one tag
 */
API::CommandResponse API::Handlers::OpcUaHandler::addTag(const QString& id,
                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")))
    return missingParam(id, "id");

  if (!params.contains(QStringLiteral("t")))
    return missingParam(id, "t");

  const auto type =
    IO::Drivers::OpcUaWire::typeFromCode(params.value(QStringLiteral("t")).toString());
  if (type == IO::Drivers::OpcUaWire::Type::Invalid)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown type code"));

  IO::Drivers::OpcUaTag tag;
  tag.nodeId = params.value(QStringLiteral("id")).toString();
  tag.name   = params.value(QStringLiteral("name")).toString(tag.nodeId);
  tag.path   = params.value(QStringLiteral("path")).toString();
  tag.unit   = params.value(QStringLiteral("unit")).toString();
  tag.type   = type;
  tag.arrayLen =
    qBound(1, params.value(QStringLiteral("n")).toInt(1), IO::Drivers::OpcUaWire::kMaxTags);

  auto* driver     = opcUaDriver();
  const int before = driver->tagCount();
  driver->addTag(tag);
  if (driver->tagCount() == before)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Tag already present or over the tag budget"));

  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("tagCount"), driver->tagCount()}
  });
}

/**
 * @brief Remove a tag by position
 */
API::CommandResponse API::Handlers::OpcUaHandler::removeTag(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("index")))
    return missingParam(id, "index");

  auto* driver    = opcUaDriver();
  const int index = params.value(QStringLiteral("index")).toInt();
  if (index < 0 || index >= driver->tagCount())
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("index out of range"));

  driver->removeTag(index);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("tagCount"), driver->tagCount()}
  });
}

/**
 * @brief Remove every tag
 */
API::CommandResponse API::Handlers::OpcUaHandler::clearTags(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)
  opcUaDriver()->clearTags();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {QStringLiteral("tagCount"), 0}
  });
}

/**
 * @brief Generate and load the project (headless: no save dialog)
 */
API::CommandResponse API::Handlers::OpcUaHandler::generateProject(const QString& id,
                                                                  const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  if (driver->tagCount() == 0)
    return CommandResponse::makeError(
      id, ErrorCode::OperationFailed, QStringLiteral("No tags configured"));

  if (!driver->loadGeneratedProject())
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, QStringLiteral("The generated project could not be loaded"));

  const auto project = driver->buildProject();
  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {  QStringLiteral("groups"), project.value(Keys::Groups).toArray().size()},
      {QStringLiteral("datasets"),                  driver->wireSchema().size()},
      { QStringLiteral("project"),                                      project},
  });
}

//--------------------------------------------------------------------------------------------------
// Queries
//--------------------------------------------------------------------------------------------------

/**
 * @brief Current configuration (password omitted)
 */
API::CommandResponse API::Handlers::OpcUaHandler::getConfiguration(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  return CommandResponse::makeSuccess(
    id,
    QJsonObject{
      {       QStringLiteral("endpointUrl"),         driver->endpointUrl()},
      {     QStringLiteral("endpointIndex"),       driver->endpointIndex()},
      {          QStringLiteral("authMode"),            driver->authMode()},
      {          QStringLiteral("username"),            driver->username()},
      {       QStringLiteral("hasPassword"), !driver->password().isEmpty()},
      {QStringLiteral("publishingInterval"),  driver->publishingInterval()},
      {          QStringLiteral("tagCount"),            driver->tagCount()},
      {   QStringLiteral("configurationOk"),     driver->configurationOk()},
  });
}

/**
 * @brief Tag list and the native template schema it generates
 */
API::CommandResponse API::Handlers::OpcUaHandler::listTags(const QString& id,
                                                           const QJsonObject& params)
{
  Q_UNUSED(params)
  auto* driver = opcUaDriver();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {  QStringLiteral("tags"),   driver->tagsJson()},
                                        {QStringLiteral("schema"), driver->wireSchema()},
  });
}

/**
 * @brief Session status and pulled counters
 */
API::CommandResponse API::Handlers::OpcUaHandler::getStatus(const QString& id,
                                                            const QJsonObject& params)
{
  Q_UNUSED(params)
  auto status = opcUaDriver()->statusJson();
  status.insert(QStringLiteral("browsing"), opcUaDriver()->browsing());
  return CommandResponse::makeSuccess(id, status);
}
