/*
 * Serial Studio
 * https://serial-studio.com/
 *
 * Copyright (C) 2020–2025 Alex Spataru
 *
 * This file is dual-licensed:
 *
 * - Under the GNU GPLv3 (or later) for builds that exclude Pro modules.
 * - Under the Serial Studio Commercial License for builds that include
 *   any Pro functionality.
 *
 * You must comply with the terms of one of these licenses, depending
 * on your use case.
 *
 * For GPL terms, see <https://www.gnu.org/licenses/gpl-3.0.html>
 * For commercial terms, see LICENSE_COMMERCIAL.md in the project root.
 *
 * SPDX-License-Identifier: GPL-3.0-only OR LicenseRef-SerialStudio-Commercial
 */

#include "API/Handlers/ExtensionHandler.h"

#include <QJsonArray>

#include "API/CommandRegistry.h"
#include "DataModel/ProjectModel.h"
#include "Misc/ExtensionManager.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all extension management commands with the registry.
 */
void API::Handlers::ExtensionHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  // Empty schema for parameterless commands
  QJsonObject emptySchema;
  emptySchema.insert(QStringLiteral("type"), QStringLiteral("object"));
  emptySchema.insert(QStringLiteral("properties"), QJsonObject());

  registry.registerCommand(
    QStringLiteral("extensions.list"),
    QStringLiteral("List all available extensions from configured repositories"),
    emptySchema,
    &listAddons);

  {
    QJsonObject props;
    props[QStringLiteral("extensionId")] = QJsonObject{
      {       QStringLiteral("type"),                             QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Unique identifier of the extension")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("extensionId")};
    registry.registerCommand(
      QStringLiteral("extensions.getInfo"),
      QStringLiteral("Get detailed info for a specific extension (params: extensionId)"),
      schema,
      &getAddonInfo);
  }

  {
    QJsonObject props;
    props[QStringLiteral("addonIndex")] = QJsonObject{
      {       QStringLiteral("type"),                                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Index of the extension in the addon list")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("addonIndex")};
    registry.registerCommand(
      QStringLiteral("extensions.install"),
      QStringLiteral("Install an extension by selecting it (params: addonIndex)"),
      schema,
      &installExtension);
  }

  {
    QJsonObject props;
    props[QStringLiteral("addonIndex")] = QJsonObject{
      {       QStringLiteral("type"),                                  QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Index of the extension in the addon list")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("addonIndex")};
    registry.registerCommand(
      QStringLiteral("extensions.uninstall"),
      QStringLiteral("Uninstall an extension by selecting it (params: addonIndex)"),
      schema,
      &uninstallExtension);
  }

  registry.registerCommand(QStringLiteral("extensions.refresh"),
                           QStringLiteral("Refresh extension catalogs from all repositories"),
                           emptySchema,
                           &refreshRepositories);

  {
    QJsonObject props;
    props[QStringLiteral("pluginId")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Unique identifier of the plugin")}
    };
    props[QStringLiteral("state")] = QJsonObject{
      {       QStringLiteral("type"),                         QStringLiteral("object")},
      {QStringLiteral("description"), QStringLiteral("Plugin state object to persist")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")] =
      QJsonArray{QStringLiteral("pluginId"), QStringLiteral("state")};
    registry.registerCommand(
      QStringLiteral("extensions.saveState"),
      QStringLiteral("Save plugin state to the project file (params: pluginId, state)"),
      schema,
      &savePluginState);
  }

  {
    QJsonObject props;
    props[QStringLiteral("pluginId")] = QJsonObject{
      {       QStringLiteral("type"),                          QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("Unique identifier of the plugin")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("pluginId")};
    registry.registerCommand(
      QStringLiteral("extensions.loadState"),
      QStringLiteral("Load plugin state from the project file (params: pluginId)"),
      schema,
      &loadPluginState);
  }

#ifdef BUILD_COMMERCIAL
  registry.registerCommand(QStringLiteral("extensions.listRepositories"),
                           QStringLiteral("List configured addon repository URLs"),
                           emptySchema,
                           &listRepositories);

  {
    QJsonObject props;
    props[QStringLiteral("url")] = QJsonObject{
      {       QStringLiteral("type"),                      QStringLiteral("string")},
      {QStringLiteral("description"), QStringLiteral("URL of the addon repository")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("url")};
    registry.registerCommand(QStringLiteral("extensions.addRepository"),
                             QStringLiteral("Add a new addon repository URL (params: url)"),
                             schema,
                             &addRepository);
  }

  {
    QJsonObject props;
    props[QStringLiteral("index")] = QJsonObject{
      {       QStringLiteral("type"),                           QStringLiteral("integer")},
      {QStringLiteral("description"), QStringLiteral("Index of the repository to remove")}
    };
    QJsonObject schema;
    schema[QStringLiteral("type")]       = QStringLiteral("object");
    schema[QStringLiteral("properties")] = props;
    schema[QStringLiteral("required")]   = QJsonArray{QStringLiteral("index")};
    registry.registerCommand(QStringLiteral("extensions.removeRepository"),
                             QStringLiteral("Remove a repository by index (params: index)"),
                             schema,
                             &removeRepository);
  }
#endif
}

//--------------------------------------------------------------------------------------------------
// Command implementations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Returns the list of available extensions with install status.
 */
API::CommandResponse API::Handlers::ExtensionHandler::listAddons(const QString& id,
                                                                 const QJsonObject&)
{
  auto& mgr = Misc::ExtensionManager::instance();
  QJsonArray result;
  for (const auto& addon : mgr.extensions())
    result.append(QJsonObject::fromVariantMap(addon.toMap()));

  QJsonObject data;
  data.insert("count", result.count());
  data.insert("addons", result);
  return CommandResponse::makeSuccess(id, data);
}

/**
 * @brief Returns detailed info for a specific extension by ID.
 */
API::CommandResponse API::Handlers::ExtensionHandler::getAddonInfo(const QString& id,
                                                                   const QJsonObject& params)
{
  const auto extensionId = params.value("extensionId").toString();
  if (extensionId.isEmpty())
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing extensionId parameter");

  auto& mgr = Misc::ExtensionManager::instance();
  for (const auto& addon : mgr.extensions()) {
    const auto map = addon.toMap();
    if (map.value("id").toString() == extensionId) {
      auto data = QJsonObject::fromVariantMap(map);
      data.insert("installed", mgr.isInstalled(extensionId));
      data.insert("updateAvailable", mgr.hasUpdate(extensionId));
      data.insert("installedVersion", mgr.installedVersion(extensionId));
      return CommandResponse::makeSuccess(id, data);
    }
  }

  return CommandResponse::makeError(
    id, QStringLiteral("NOT_FOUND"), "Extension not found: " + extensionId);
}

/**
 * @brief Installs an extension by setting the selected index and triggering install.
 */
API::CommandResponse API::Handlers::ExtensionHandler::installExtension(const QString& id,
                                                                       const QJsonObject& params)
{
  const int index = params.value("addonIndex").toInt(-1);
  if (index < 0)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing or invalid addonIndex");

  auto& mgr = Misc::ExtensionManager::instance();
  mgr.setSelectedIndex(index);
  mgr.installExtension();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "installing"}
  });
}

/**
 * @brief Uninstalls an extension by setting the selected index and triggering removal.
 */
API::CommandResponse API::Handlers::ExtensionHandler::uninstallExtension(const QString& id,
                                                                         const QJsonObject& params)
{
  const int index = params.value("addonIndex").toInt(-1);
  if (index < 0)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing or invalid addonIndex");

  auto& mgr = Misc::ExtensionManager::instance();
  mgr.setSelectedIndex(index);
  mgr.uninstallExtension();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "uninstalled"}
  });
}

/**
 * @brief Returns the list of configured repository URLs.
 */
API::CommandResponse API::Handlers::ExtensionHandler::listRepositories(const QString& id,
                                                                       const QJsonObject&)
{
  auto& mgr = Misc::ExtensionManager::instance();
  QJsonArray repos;
  for (const auto& url : mgr.repositories())
    repos.append(url);

  QJsonObject data;
  data.insert("repositories", repos);
  return CommandResponse::makeSuccess(id, data);
}

/**
 * @brief Adds a new repository URL.
 */
API::CommandResponse API::Handlers::ExtensionHandler::addRepository(const QString& id,
                                                                    const QJsonObject& params)
{
  const auto url = params.value("url").toString();
  if (url.isEmpty())
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing url parameter");

  Misc::ExtensionManager::instance().addRepository(url);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "added"}
  });
}

/**
 * @brief Removes a repository by index.
 */
API::CommandResponse API::Handlers::ExtensionHandler::removeRepository(const QString& id,
                                                                       const QJsonObject& params)
{
  const int index = params.value("index").toInt(-1);
  if (index < 0)
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing or invalid index");

  Misc::ExtensionManager::instance().removeRepository(index);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "removed"}
  });
}

/**
 * @brief Triggers a refresh of all repository catalogs.
 */
API::CommandResponse API::Handlers::ExtensionHandler::refreshRepositories(const QString& id,
                                                                          const QJsonObject&)
{
  Misc::ExtensionManager::instance().refreshRepositories();
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "refreshing"}
  });
}

//--------------------------------------------------------------------------------------------------
// Plugin state persistence
//--------------------------------------------------------------------------------------------------

/**
 * @brief Saves a plugin's state to the current project file.
 *
 * The state is stored under "plugin:<pluginId>" in the project's
 * widgetSettings section, persisted to disk alongside layout data.
 */
API::CommandResponse API::Handlers::ExtensionHandler::savePluginState(const QString& id,
                                                                      const QJsonObject& params)
{
  const auto pluginId = params.value("pluginId").toString();
  if (pluginId.isEmpty())
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing pluginId parameter");

  const auto state = params.value("state").toObject();
  DataModel::ProjectModel::instance().savePluginState(pluginId, state);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"status", "saved"}
  });
}

/**
 * @brief Loads a plugin's state from the current project file.
 *
 * Returns the saved state object, or an empty object if no state was saved.
 */
API::CommandResponse API::Handlers::ExtensionHandler::loadPluginState(const QString& id,
                                                                      const QJsonObject& params)
{
  const auto pluginId = params.value("pluginId").toString();
  if (pluginId.isEmpty())
    return CommandResponse::makeError(
      id, QStringLiteral("INVALID_PARAMS"), "Missing pluginId parameter");

  const auto state = DataModel::ProjectModel::instance().pluginState(pluginId);
  return CommandResponse::makeSuccess(id,
                                      QJsonObject{
                                        {"state", state}
  });
}
