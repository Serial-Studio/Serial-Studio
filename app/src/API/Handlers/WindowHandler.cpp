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

#include "API/Handlers/WindowHandler.h"

#include "API/CommandRegistry.h"
#include "DataModel/ProjectModel.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"
#include "UI/WindowManager.h"

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

static API::CommandResponse noSession(const QString& id)
{
  return API::CommandResponse::makeError(
    id, API::ErrorCode::ExecutionError, QStringLiteral("No dashboard session active"));
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all ui.window.* commands with the command registry.
 */
void API::Handlers::WindowHandler::registerCommands()
{
  auto& registry = CommandRegistry::instance();

  registry.registerCommand(
    QStringLiteral("ui.window.getStatus"),
    QStringLiteral("Get dashboard window status: activeGroupId, groupCount, autoLayoutEnabled"),
    &getStatus);

  registry.registerCommand(QStringLiteral("ui.window.getGroups"),
                           QStringLiteral("Get list of groups as [{id, text, icon}]"),
                           &getGroups);

  registry.registerCommand(QStringLiteral("ui.window.setActiveGroup"),
                           QStringLiteral("Set active group by ID (params: groupId int)"),
                           &setActiveGroup);

  registry.registerCommand(QStringLiteral("ui.window.setActiveGroupIndex"),
                           QStringLiteral("Set active group by index (params: index int)"),
                           &setActiveGroupIndex);

  registry.registerCommand(
    QStringLiteral("ui.window.getWindowStates"),
    QStringLiteral("Get window states array [{id, state}] for current group"),
    &getWindowStates);

  registry.registerCommand(
    QStringLiteral("ui.window.setWindowState"),
    QStringLiteral("Set window state (params: id int, state int: 0=normal,1=minimized,2=closed)"),
    &setWindowState);

  registry.registerCommand(QStringLiteral("ui.window.setAutoLayout"),
                           QStringLiteral("Enable or disable auto layout (params: enabled bool)"),
                           &setAutoLayout);

  registry.registerCommand(QStringLiteral("ui.window.saveLayout"),
                           QStringLiteral("Save current window layout to project"),
                           &saveLayout);

  registry.registerCommand(QStringLiteral("ui.window.loadLayout"),
                           QStringLiteral("Load window layout from project"),
                           &loadLayout);

  registry.registerCommand(QStringLiteral("ui.window.getLayout"),
                           QStringLiteral("Get serialized layout JSON from WindowManager"),
                           &getLayout);

  registry.registerCommand(
    QStringLiteral("ui.window.setLayout"),
    QStringLiteral("Apply a layout to WindowManager (params: layout object)"),
    &setLayout);

  registry.registerCommand(QStringLiteral("ui.window.getWidgetSettings"),
                           QStringLiteral("Get saved widget settings (params: widgetId string)"),
                           &getWidgetSettings);

  registry.registerCommand(
    QStringLiteral("ui.window.setWidgetSetting"),
    QStringLiteral("Set a widget setting (params: widgetId string, key string, value any)"),
    &setWidgetSetting);
}

//--------------------------------------------------------------------------------------------------
// Status & groups
//--------------------------------------------------------------------------------------------------

/**
 * @brief Return overall dashboard window status.
 */
API::CommandResponse API::Handlers::WindowHandler::getStatus(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  auto* wm      = UI::UISessionRegistry::instance().primaryWindowManager();

  QJsonObject result;

  if (!taskbar) {
    result[QStringLiteral("sessionActive")]      = false;
    result[QStringLiteral("activeGroupId")]      = -1;
    result[QStringLiteral("activeGroupIndex")]   = -1;
    result[QStringLiteral("groupCount")]         = 0;
    result[QStringLiteral("hasMaximizedWindow")] = false;
    result[QStringLiteral("autoLayoutEnabled")]  = false;
    return CommandResponse::makeSuccess(id, result);
  }

  const auto groups = taskbar->groupModel();

  result[QStringLiteral("sessionActive")]      = true;
  result[QStringLiteral("activeGroupId")]      = taskbar->activeGroupId();
  result[QStringLiteral("activeGroupIndex")]   = taskbar->activeGroupIndex();
  result[QStringLiteral("groupCount")]         = static_cast<int>(groups.size());
  result[QStringLiteral("hasMaximizedWindow")] = taskbar->hasMaximizedWindow();
  result[QStringLiteral("autoLayoutEnabled")]  = wm ? wm->autoLayoutEnabled() : false;

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Return all groups from the taskbar model.
 */
API::CommandResponse API::Handlers::WindowHandler::getGroups(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const auto model = taskbar->groupModel();
  QJsonArray groups;
  for (const auto& entry : model) {
    const auto map = entry.toMap();
    QJsonObject g;
    g[QStringLiteral("id")]   = map.value(QStringLiteral("id")).toInt();
    g[QStringLiteral("text")] = map.value(QStringLiteral("text")).toString();
    g[QStringLiteral("icon")] = map.value(QStringLiteral("icon")).toString();
    groups.append(g);
  }

  QJsonObject result;
  result[QStringLiteral("groups")] = groups;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Active group setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Switch active group by group ID.
 * @param params Requires "groupId" (int)
 */
API::CommandResponse API::Handlers::WindowHandler::setActiveGroup(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: groupId"));
  }

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const int group_id = params.value(QStringLiteral("groupId")).toInt();
  QMetaObject::invokeMethod(
    taskbar, [taskbar, group_id]() { taskbar->setActiveGroupId(group_id); }, Qt::QueuedConnection);

  QJsonObject result;
  result[QStringLiteral("groupId")] = group_id;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Switch active group by index.
 * @param params Requires "index" (int)
 */
API::CommandResponse API::Handlers::WindowHandler::setActiveGroupIndex(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("index"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: index"));
  }

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const int index = params.value(QStringLiteral("index")).toInt();
  QMetaObject::invokeMethod(
    taskbar, [taskbar, index]() { taskbar->setActiveGroupIndex(index); }, Qt::QueuedConnection);

  QJsonObject result;
  result[QStringLiteral("index")] = index;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Window states
//--------------------------------------------------------------------------------------------------

/**
 * @brief Return window states for all items in the taskbar buttons model.
 */
API::CommandResponse API::Handlers::WindowHandler::getWindowStates(const QString& id,
                                                                   const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  auto* model = taskbar->fullModel();
  QJsonArray states;

  for (int i = 0; i < model->rowCount(); ++i) {
    const auto* top = model->item(i);
    if (!top)
      continue;

    const bool is_group = top->data(UI::TaskbarModel::IsGroupRole).toBool();

    if (is_group) {
      for (int j = 0; j < top->rowCount(); ++j) {
        const auto* child = top->child(j);
        if (!child)
          continue;

        QJsonObject entry;
        entry[QStringLiteral("id")]    = child->data(UI::TaskbarModel::WindowIdRole).toInt();
        entry[QStringLiteral("state")] = child->data(UI::TaskbarModel::WindowStateRole).toInt();
        entry[QStringLiteral("name")]  = child->data(UI::TaskbarModel::WidgetNameRole).toString();
        states.append(entry);
      }
    } else {
      QJsonObject entry;
      entry[QStringLiteral("id")]    = top->data(UI::TaskbarModel::WindowIdRole).toInt();
      entry[QStringLiteral("state")] = top->data(UI::TaskbarModel::WindowStateRole).toInt();
      entry[QStringLiteral("name")]  = top->data(UI::TaskbarModel::WidgetNameRole).toString();
      states.append(entry);
    }
  }

  QJsonObject result;
  result[QStringLiteral("windows")] = states;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the window state for a window by ID.
 * @param params Requires "id" (int) and "state" (int: 0=normal,1=minimized,2=closed)
 */
API::CommandResponse API::Handlers::WindowHandler::setWindowState(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")) || !params.contains(QStringLiteral("state"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameters: id, state"));
  }

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const int win_id    = params.value(QStringLiteral("id")).toInt();
  const int state_int = params.value(QStringLiteral("state")).toInt();

  if (state_int < 0 || state_int > 2) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid state: %1. Valid range: 0-2").arg(state_int));
  }

  const auto state = static_cast<UI::TaskbarModel::WindowState>(state_int);
  QMetaObject::invokeMethod(
    taskbar,
    [taskbar, win_id, state]() { taskbar->setWindowState(win_id, state); },
    Qt::QueuedConnection);

  QJsonObject result;
  result[QStringLiteral("id")]    = win_id;
  result[QStringLiteral("state")] = state_int;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Layout operations
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable or disable auto layout.
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::WindowHandler::setAutoLayout(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  auto* wm = UI::UISessionRegistry::instance().primaryWindowManager();
  if (!wm)
    return noSession(id);

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  QMetaObject::invokeMethod(
    wm, [wm, enabled]() { wm->setAutoLayoutEnabled(enabled); }, Qt::QueuedConnection);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Save the current layout to the project (via Taskbar::saveLayout).
 */
API::CommandResponse API::Handlers::WindowHandler::saveLayout(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* taskbar = UI::UISessionRegistry::instance().primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  QMetaObject::invokeMethod(taskbar, [taskbar]() { taskbar->saveLayout(); }, Qt::QueuedConnection);

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Load the saved layout from the project (via WindowManager::loadLayout).
 */
API::CommandResponse API::Handlers::WindowHandler::loadLayout(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* wm = UI::UISessionRegistry::instance().primaryWindowManager();
  if (!wm)
    return noSession(id);

  QMetaObject::invokeMethod(wm, [wm]() { wm->loadLayout(); }, Qt::QueuedConnection);

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Return the serialized layout JSON from WindowManager.
 */
API::CommandResponse API::Handlers::WindowHandler::getLayout(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  auto* wm = UI::UISessionRegistry::instance().primaryWindowManager();
  if (!wm)
    return noSession(id);

  const QJsonObject layout = wm->serializeLayout();

  QJsonObject result;
  result[QStringLiteral("layout")] = layout;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Apply a layout to the WindowManager.
 * @param params Requires "layout" (object)
 */
API::CommandResponse API::Handlers::WindowHandler::setLayout(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("layout"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: layout"));
  }

  auto* wm = UI::UISessionRegistry::instance().primaryWindowManager();
  if (!wm)
    return noSession(id);

  const QJsonObject layout = params.value(QStringLiteral("layout")).toObject();
  QMetaObject::invokeMethod(
    wm, [wm, layout]() { wm->restoreLayout(layout); }, Qt::QueuedConnection);

  return CommandResponse::makeSuccess(id);
}

//--------------------------------------------------------------------------------------------------
// Widget settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Return saved widget settings for the given widgetId.
 * @param params Requires "widgetId" (string)
 */
API::CommandResponse API::Handlers::WindowHandler::getWidgetSettings(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("widgetId"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: widgetId"));
  }

  const QString widget_id    = params.value(QStringLiteral("widgetId")).toString();
  const QJsonObject settings = DataModel::ProjectModel::instance().widgetSettings(widget_id);

  QJsonObject result;
  result[QStringLiteral("widgetId")] = widget_id;
  result[QStringLiteral("settings")] = settings;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set a single widget setting key/value.
 * @param params Requires "widgetId" (string), "key" (string), "value" (any)
 */
API::CommandResponse API::Handlers::WindowHandler::setWidgetSetting(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("widgetId")) || !params.contains(QStringLiteral("key"))
      || !params.contains(QStringLiteral("value"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameters: widgetId, key, value"));
  }

  const QString widget_id = params.value(QStringLiteral("widgetId")).toString();
  const QString key       = params.value(QStringLiteral("key")).toString();
  const QVariant value    = params.value(QStringLiteral("value")).toVariant();

  auto* pm = &DataModel::ProjectModel::instance();
  QMetaObject::invokeMethod(
    pm,
    [pm, widget_id, key, value]() { pm->saveWidgetSetting(widget_id, key, value); },
    Qt::QueuedConnection);

  QJsonObject result;
  result[QStringLiteral("widgetId")] = widget_id;
  result[QStringLiteral("key")]      = key;
  return CommandResponse::makeSuccess(id, result);
}
