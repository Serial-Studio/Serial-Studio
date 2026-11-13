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
 * For commercial terms, see LICENSES/LicenseRef-SerialStudio-Commercial.txt.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-SerialStudio-Commercial
 */

#include "ApiHandlers/WindowHandler.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <QCoreApplication>
#include <QDebug>
#include <QGuiApplication>
#include <QJsonArray>
#include <QMessageBox>
#include <QPointer>
#include <QTextDocument>

#include "API/CommandRegistry.h"
#include "API/HandlerContext.h"
#include "API/SchemaBuilder.h"
#include "ApiHandlers/MessageBoxGate.h"
#include "Core/Api/UserWaitScope.h"
#include "Core/SSAssert.h"
#include "DataModel/PipelineModules.h"
#include "DataModel/ProjectModel.h"
#include "Misc/Utilities.h"
#include "UI/Taskbar.h"
#include "UI/UISessionRegistry.h"
#include "UI/WindowManager.h"

//--------------------------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------------------------

/**
 * @brief One message box icon the API accepts, by the name a caller passes.
 */
struct MessageBoxIcon {
  QLatin1StringView name;
  QMessageBox::Icon icon;
};

/**
 * @brief One message box button the API accepts, by the name a caller passes and reads back.
 */
struct MessageBoxButton {
  QLatin1StringView name;
  QMessageBox::StandardButton button;
};

// clang-format off
static constexpr MessageBoxIcon kMessageBoxIcons[] = {
  {QLatin1StringView("info"),     QMessageBox::Information},
  {QLatin1StringView("warning"),  QMessageBox::Warning    },
  {QLatin1StringView("critical"), QMessageBox::Critical   },
  {QLatin1StringView("question"), QMessageBox::Question   },
  {QLatin1StringView("none"),     QMessageBox::NoIcon     },
};

static constexpr MessageBoxButton kMessageBoxButtons[] = {
  {QLatin1StringView("ok"),      QMessageBox::Ok     },
  {QLatin1StringView("cancel"),  QMessageBox::Cancel },
  {QLatin1StringView("yes"),     QMessageBox::Yes    },
  {QLatin1StringView("no"),      QMessageBox::No     },
  {QLatin1StringView("retry"),   QMessageBox::Retry  },
  {QLatin1StringView("abort"),   QMessageBox::Abort  },
  {QLatin1StringView("ignore"),  QMessageBox::Ignore },
  {QLatin1StringView("close"),   QMessageBox::Close  },
  {QLatin1StringView("save"),    QMessageBox::Save   },
  {QLatin1StringView("discard"), QMessageBox::Discard},
  {QLatin1StringView("apply"),   QMessageBox::Apply  },
  {QLatin1StringView("reset"),   QMessageBox::Reset  },
};
// clang-format on

//--------------------------------------------------------------------------------------------------
// Helpers
//--------------------------------------------------------------------------------------------------

/**
 * @brief Build the standard "no dashboard session active" error response.
 */
static API::CommandResponse noSession(const QString& id)
{
  return API::CommandResponse::makeError(
    id, API::ErrorCode::ExecutionError, QStringLiteral("No dashboard session active"));
}

/**
 * @brief The button named @p name, or NoButton when the API does not offer it.
 */
[[nodiscard]] static QMessageBox::StandardButton messageBoxButton(const QString& name)
{
  for (const auto& entry : kMessageBoxButtons)
    if (name == entry.name)
      return entry.button;

  return QMessageBox::NoButton;
}

/**
 * @brief The API name of the button the user clicked; "none" when the box closed without one.
 */
[[nodiscard]] static QString messageBoxButtonName(int clicked)
{
  for (const auto& entry : kMessageBoxButtons)
    if (clicked == static_cast<int>(entry.button))
      return QString(entry.name);

  return QStringLiteral("none");
}

/**
 * @brief The caller's options for a box that waits: the buttons, the default and the captions.
 *        A label for a button not offered is an error, since the dialog would have no button to
 *        put it on.
 */
struct MessageBoxChoice {
  QMessageBox::StandardButtons buttons;
  QMessageBox::StandardButton defaultButton;
  ButtonTextMap labels;
  QString error;
};

/**
 * @brief Reads buttons, defaultButton and buttonLabels; on a bad value only error is set.
 */
[[nodiscard]] static MessageBoxChoice readMessageBoxChoice(const QJsonObject& params)
{
  MessageBoxChoice choice{QMessageBox::NoButton, QMessageBox::NoButton, {}, {}};
  const auto names = params.value(QStringLiteral("buttons")).toArray();
  for (const auto& name : names) {
    const auto button = messageBoxButton(name.toString());
    if (button == QMessageBox::NoButton) {
      choice.error = QStringLiteral("Unknown button: %1").arg(name.toString());
      return choice;
    }

    choice.buttons |= button;
  }

  if (choice.buttons == QMessageBox::NoButton)
    choice.buttons = QMessageBox::Ok;

  const auto defaultName = params.value(QStringLiteral("defaultButton")).toString();
  if (!defaultName.isEmpty()) {
    choice.defaultButton = messageBoxButton(defaultName);
    if (!choice.buttons.testFlag(choice.defaultButton)) {
      choice.error = QStringLiteral("defaultButton is not one of buttons: %1").arg(defaultName);
      return choice;
    }
  }

  const auto labels = params.value(QStringLiteral("buttonLabels")).toObject();
  for (auto it = labels.constBegin(); it != labels.constEnd(); ++it) {
    const auto button = messageBoxButton(it.key());
    if (!choice.buttons.testFlag(button) || !it.value().isString()) {
      choice.error = QStringLiteral("buttonLabels names a button not offered: %1").arg(it.key());
      return choice;
    }

    choice.labels.insert(button, it.value().toString());
  }

  return choice;
}

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all ui.window.* commands with the command registry.
 */
void API::Handlers::WindowHandler::registerCommands()
{
  registerStatusCommands();
  registerStateCommands();
  registerLayoutCommands();
  registerWidgetSettingCommands();
  registerMessageBoxCommands();
}

/**
 * @brief Register status / group-listing commands.
 */
void API::Handlers::WindowHandler::registerStatusCommands()
{
  auto& registry   = API::handlerContext().registry;
  const auto empty = API::emptySchema();

  registry.registerCommand(
    QStringLiteral("ui.window.getStatus"),
    QStringLiteral("Get dashboard window status: activeGroupId, groupCount, autoLayoutEnabled"),
    empty,
    &getStatus);
  registry.registerCommand(QStringLiteral("ui.window.listGroups"),
                           QStringLiteral("Get list of groups as [{id, text, icon}]"),
                           empty,
                           &getGroups);
  registry.registerCommand(QStringLiteral("ui.window.setActiveGroup"),
                           QStringLiteral("Set active group by ID (params: groupId int)"),
                           API::makeSchema({
                             {QStringLiteral("groupId"),
                              QStringLiteral("integer"),
                              QStringLiteral("The group ID to activate")}
  }),
                           &setActiveGroup);
  registry.registerCommand(QStringLiteral("ui.window.setActiveGroupIndex"),
                           QStringLiteral("Set active group by index (params: index int)"),
                           API::makeSchema({
                             {QStringLiteral("index"),
                              QStringLiteral("integer"),
                              QStringLiteral("The group index to activate")}
  }),
                           &setActiveGroupIndex);
}

/**
 * @brief Register window-state read/write commands.
 */
void API::Handlers::WindowHandler::registerStateCommands()
{
  auto& registry = API::handlerContext().registry;

  registry.registerCommand(
    QStringLiteral("ui.window.listWindowStates"),
    QStringLiteral("Get window states array [{id, state}] for current group"),
    API::emptySchema(),
    &getWindowStates);
  registry.registerCommand(
    QStringLiteral("ui.window.setWindowState"),
    QStringLiteral("Set window state (params: id int, state int: 0=normal,1=minimized,2=closed)"),
    API::makeSchema({
      {   QStringLiteral("id"),QStringLiteral("integer"),QStringLiteral("The window ID")                           },
      {QStringLiteral("state"),
       QStringLiteral("integer"),
       QStringLiteral("Window state: 0=normal, 1=minimized, 2=closed")}
  }),
    &setWindowState);
}

/**
 * @brief Register layout save/load/serialize commands.
 */
void API::Handlers::WindowHandler::registerLayoutCommands()
{
  auto& registry   = API::handlerContext().registry;
  const auto empty = API::emptySchema();

  registry.registerCommand(QStringLiteral("ui.window.setAutoLayout"),
                           QStringLiteral("Enable or disable auto layout (params: enabled bool)"),
                           API::makeSchema({
                             {QStringLiteral("enabled"),
                              QStringLiteral("boolean"),
                              QStringLiteral("Whether to enable auto layout")}
  }),
                           &setAutoLayout);
  registry.registerCommand(QStringLiteral("ui.window.saveLayout"),
                           QStringLiteral("Save current window layout to project"),
                           empty,
                           &saveLayout);
  registry.registerCommand(QStringLiteral("ui.window.loadLayout"),
                           QStringLiteral("Load window layout from project"),
                           empty,
                           &loadLayout);
  registry.registerCommand(QStringLiteral("ui.window.getLayout"),
                           QStringLiteral("Get serialized layout JSON from WindowManager"),
                           empty,
                           &getLayout);
  registry.registerCommand(
    QStringLiteral("ui.window.setLayoutPattern"),
    QStringLiteral("Set the auto-layout pattern of the workspace or group on screen. Pattern is "
                   "'' (Grid), master-stack, master-grid, row, column or spiral; ratio is the "
                   "primary split in sixteenths (1..15, default 8) and only applies to the "
                   "patterns with a primary region. Stored beside that workspace's window "
                   "geometry, so it does not touch the workspace list."),
    API::makeSchema(
      {
        {QStringLiteral("pattern"),
         QStringLiteral("string"),
         QStringLiteral("Pattern id; '' selects Grid")}
  },
      {{QStringLiteral("ratio"),
        QStringLiteral("integer"),
        QStringLiteral("Primary split in sixteenths, 1..15 (optional)")}}),
    &setLayoutPattern);
  registry.registerCommand(
    QStringLiteral("ui.window.setLayout"),
    QStringLiteral("Apply a layout to WindowManager (params: layout object)"),
    API::makeSchema({
      {QStringLiteral("layout"),
       QStringLiteral("object"),
       QStringLiteral("The layout object to apply")}
  }),
    &setLayout);
}

/**
 * @brief Register per-widget settings persistence commands.
 */
void API::Handlers::WindowHandler::registerWidgetSettingCommands()
{
  auto& registry = API::handlerContext().registry;

  registry.registerCommand(QStringLiteral("ui.window.getWidgetSettings"),
                           QStringLiteral("Get saved widget settings (params: widgetId string)"),
                           API::makeSchema({
                             {QStringLiteral("widgetId"),
                              QStringLiteral("string"),
                              QStringLiteral("The widget identifier")}
  }),
                           &getWidgetSettings);

  QJsonObject swsProps;
  swsProps[QStringLiteral("widgetId")] = QJsonObject{
    {       QStringLiteral("type"),                QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("The widget identifier")}
  };
  swsProps[QStringLiteral("key")] = QJsonObject{
    {       QStringLiteral("type"),          QStringLiteral("string")},
    {QStringLiteral("description"), QStringLiteral("The setting key")}
  };
  swsProps[QStringLiteral("settingValue")] = QJsonObject{
    {QStringLiteral("description"), QStringLiteral("The setting value")}
  };
  QJsonObject swsSchema;
  swsSchema[QStringLiteral("type")]       = QStringLiteral("object");
  swsSchema[QStringLiteral("properties")] = swsProps;
  swsSchema[QStringLiteral("required")] =
    QJsonArray{QStringLiteral("widgetId"), QStringLiteral("key"), QStringLiteral("settingValue")};
  registry.registerCommand(
    QStringLiteral("ui.window.setWidgetSetting"),
    QStringLiteral("Set a widget setting (params: widgetId string, key string, value any)"),
    swsSchema,
    &setWidgetSetting);
}

/**
 * @brief Register the message box command: icon, text, and an optional wait for the user's
 *        button, which only a control script may request.
 */
void API::Handlers::WindowHandler::registerMessageBoxCommands()
{
  QJsonArray iconNames;
  for (const auto& icon : kMessageBoxIcons)
    iconNames.append(QString(icon.name));

  QJsonArray buttonNames;
  for (const auto& button : kMessageBoxButtons)
    buttonNames.append(QString(button.name));

  SchemaProp icon;
  icon.name        = QStringLiteral("icon");
  icon.type        = QStringLiteral("string");
  icon.description = QStringLiteral(
    "Icon: info (default), warning, critical, question, or none for the application logo");
  icon.enumValues   = iconNames;
  icon.defaultValue = QStringLiteral("info");

  const auto buttons = arrayProp(
    QStringLiteral("buttons"),
    QStringLiteral("Buttons to offer when wait is true (default [\"ok\"])"),
    QJsonObject{
      {QStringLiteral("type"), QStringLiteral("string")},
      {QStringLiteral("enum"),              buttonNames}
  });

  API::handlerContext().registry.registerCommand(
    QStringLiteral("ui.window.showMessageBox"),
    QStringLiteral("Show a message box. With wait false (default) it returns at once; with wait "
                   "true it blocks until the user clicks and returns {button}. Only a control "
                   "script may wait: a frame parser or transform would stall acquisition."),
    API::makeSchema(
      {
        {QStringLiteral("text"), QStringLiteral("string"), QStringLiteral("Headline, plain text")}
  },
      {{QStringLiteral("informativeText"),
        QStringLiteral("string"),
        QStringLiteral("Detail under the headline, plain text (optional)")},
       {QStringLiteral("title"), QStringLiteral("string"), QStringLiteral("Window title")},
       icon,
       {QStringLiteral("wait"),
        QStringLiteral("boolean"),
        QStringLiteral("Block until a button is clicked and return it (control scripts only)")},
       buttons,
       {QStringLiteral("defaultButton"),
        QStringLiteral("string"),
        QStringLiteral("The button Enter presses (one of buttons)")},
       {QStringLiteral("buttonLabels"),
        QStringLiteral("object"),
        QStringLiteral("Custom captions keyed by button name, e.g. {\"yes\": \"Start\"}")}}),
    &showMessageBox);
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

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  auto* wm                     = sessionRegistry.primaryWindowManager();

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

  const auto groups = taskbar->workspaceModel();

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

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const auto model = taskbar->workspaceModel();
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
 */
API::CommandResponse API::Handlers::WindowHandler::setActiveGroup(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("groupId")))
    return CommandResponse::makeMissingParam(id, "groupId");

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const int group_id = params.value(QStringLiteral("groupId")).toInt();
  QMetaObject::invokeMethod(taskbar,
                            [taskbar, group_id]() { taskbar->setActiveGroupId(group_id); });

  QJsonObject result;
  result[QStringLiteral("groupId")] = group_id;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Switch active group by index.
 */
API::CommandResponse API::Handlers::WindowHandler::setActiveGroupIndex(const QString& id,
                                                                       const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("index")))
    return CommandResponse::makeMissingParam(id, "index");

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  const int index = params.value(QStringLiteral("index")).toInt();
  QMetaObject::invokeMethod(taskbar, [taskbar, index]() { taskbar->setActiveGroupIndex(index); });

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

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  auto* model = taskbar->fullModel();
  QJsonArray states;

  auto entryFor = [](const QStandardItem* item) {
    QJsonObject entry;
    entry[QStringLiteral("id")]    = item->data(UI::TaskbarModel::WindowIdRole).toInt();
    entry[QStringLiteral("state")] = item->data(UI::TaskbarModel::WindowStateRole).toInt();
    entry[QStringLiteral("name")]  = item->data(UI::TaskbarModel::WidgetNameRole).toString();
    return entry;
  };

  for (int i = 0; i < model->rowCount(); ++i) {
    const auto* top = model->item(i);
    if (!top)
      continue;

    if (!top->data(UI::TaskbarModel::IsGroupRole).toBool()) {
      states.append(entryFor(top));
      continue;
    }

    for (int j = 0; j < top->rowCount(); ++j) {
      const auto* child = top->child(j);
      if (child)
        states.append(entryFor(child));
    }
  }

  QJsonObject result;
  result[QStringLiteral("windows")] = states;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the window state for a window by ID.
 */
API::CommandResponse API::Handlers::WindowHandler::setWindowState(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("id")) || !params.contains(QStringLiteral("state"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameters: id, state"));
  }

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
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
  QMetaObject::invokeMethod(taskbar,
                            [taskbar, win_id, state]() { taskbar->setWindowState(win_id, state); });

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
 */
API::CommandResponse API::Handlers::WindowHandler::setAutoLayout(const QString& id,
                                                                 const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeMissingParam(id, "enabled");

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* wm                     = sessionRegistry.primaryWindowManager();
  if (!wm)
    return noSession(id);

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  QMetaObject::invokeMethod(wm, [wm, enabled]() { wm->setAutoLayoutEnabled(enabled); });

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

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* taskbar                = sessionRegistry.primaryTaskbar();
  if (!taskbar)
    return noSession(id);

  QMetaObject::invokeMethod(taskbar, [taskbar]() { taskbar->saveLayout(); });

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Load the saved layout from the project (via WindowManager::loadLayout).
 */
API::CommandResponse API::Handlers::WindowHandler::loadLayout(const QString& id,
                                                              const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* wm                     = sessionRegistry.primaryWindowManager();
  if (!wm)
    return noSession(id);

  QMetaObject::invokeMethod(wm, [wm]() { wm->loadLayout(); });

  return CommandResponse::makeSuccess(id);
}

/**
 * @brief Return the serialized layout JSON from WindowManager.
 */
API::CommandResponse API::Handlers::WindowHandler::getLayout(const QString& id,
                                                             const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* wm                     = sessionRegistry.primaryWindowManager();
  if (!wm)
    return noSession(id);

  const QJsonObject layout = wm->serializeLayout();

  QJsonObject result;
  result[QStringLiteral("layout")]     = layout;
  result[QStringLiteral("pattern")]    = wm->layoutPattern();
  result[QStringLiteral("ratio")]      = wm->layoutRatio();
  result[QStringLiteral("autoLayout")] = wm->autoLayoutEnabled();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Applies an auto-layout pattern to the workspace or group currently on screen.
 */
API::CommandResponse API::Handlers::WindowHandler::setLayoutPattern(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("pattern")))
    return CommandResponse::makeMissingParam(id, "pattern");

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* wm                     = sessionRegistry.primaryWindowManager();
  if (!wm)
    return noSession(id);

  const auto pattern = params.value(QStringLiteral("pattern")).toString();
  const int ratio    = params.value(QStringLiteral("ratio")).toInt(wm->layoutRatio());
  wm->selectLayoutPattern(pattern, ratio);

  QJsonObject result;
  result[QStringLiteral("pattern")] = wm->layoutPattern();
  result[QStringLiteral("ratio")]   = wm->layoutRatio();
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Apply a layout to the WindowManager.
 */
API::CommandResponse API::Handlers::WindowHandler::setLayout(const QString& id,
                                                             const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("layout")))
    return CommandResponse::makeMissingParam(id, "layout");

  static auto& sessionRegistry = UI::UISessionRegistry::instance();
  auto* wm                     = sessionRegistry.primaryWindowManager();
  if (!wm)
    return noSession(id);

  const QJsonObject layout = params.value(QStringLiteral("layout")).toObject();
  bool restored            = false;
  QMetaObject::invokeMethod(wm,
                            [wm, layout, &restored]() { restored = wm->restoreLayout(layout); });

  if (!restored)
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Failed to restore layout"));

  return CommandResponse::makeSuccess(id);
}

//--------------------------------------------------------------------------------------------------
// Widget settings
//--------------------------------------------------------------------------------------------------

/**
 * @brief Return saved widget settings for the given widgetId.
 */
API::CommandResponse API::Handlers::WindowHandler::getWidgetSettings(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("widgetId")))
    return CommandResponse::makeMissingParam(id, "widgetId");

  const QString widget_id    = params.value(QStringLiteral("widgetId")).toString();
  auto& projectModel         = DataModel::pipelineModules().projectModel;
  const QJsonObject settings = projectModel.widgetSettings(widget_id);

  QJsonObject result;
  result[QStringLiteral("widgetId")] = widget_id;
  result[QStringLiteral("settings")] = settings;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set a single widget setting key/value.
 */
API::CommandResponse API::Handlers::WindowHandler::setWidgetSetting(const QString& id,
                                                                    const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("widgetId")) || !params.contains(QStringLiteral("key"))
      || !params.contains(QStringLiteral("settingValue"))) {
    return CommandResponse::makeError(
      id,
      ErrorCode::MissingParam,
      QStringLiteral("Missing required parameters: widgetId, key, settingValue"));
  }

  const QString widget_id = params.value(QStringLiteral("widgetId")).toString();
  const QString key       = params.value(QStringLiteral("key")).toString();
  const QVariant value    = params.value(QStringLiteral("settingValue")).toVariant();

  auto& projectModel = DataModel::pipelineModules().projectModel;
  auto* pm           = &projectModel;
  QMetaObject::invokeMethod(
    pm, [pm, widget_id, key, value]() { pm->saveWidgetSetting(widget_id, key, value); });

  QJsonObject result;
  result[QStringLiteral("widgetId")] = widget_id;
  result[QStringLiteral("key")]      = key;
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Message box
//--------------------------------------------------------------------------------------------------

/**
 * @brief The one API box in flight (spec 0120 R1): the dialog once it opens, and whether its
 *        closer asked to drop it while it was still queued.
 */
struct ApiBoxState {
  QPointer<QMessageBox> box;
  bool cancelled;
};

/**
 * @brief The error a gated request gets, worded for the caller that has to change.
 */
static QString messageBoxRefusalText(API::MessageBoxGate::Refusal refusal)
{
  using Refusal = API::MessageBoxGate::Refusal;
  SS_ASSERT_LOG(refusal != Refusal::None);

  switch (refusal) {
    case Refusal::WaitNotAllowed:
      return QStringLiteral("wait: true is only available to the control script's setup() and "
                            "loop() and to macros; other callers would block while the dialog is "
                            "open. Call again without wait.");
    case Refusal::HeadlessWait:
      return QStringLiteral("wait: true needs a display, and this session is headless, so nobody "
                            "could answer. Call again without wait.");
    case Refusal::Busy:
      return QStringLiteral("A message box from a script or API client is already showing. Try "
                            "again once it closes.");
    case Refusal::None:
      break;
  }

  return QString();
}

/**
 * @brief Appends the line a caller cannot change, so a box never passes as Serial Studio's own
 * (R2).
 */
static QString withProvenance(const QString& info)
{
  SS_ASSERT_LOG(qApp != nullptr);
  const auto line = QCoreApplication::translate(
    "WindowHandler", "Sent by the project's scripts or an API client, not by Serial Studio.");
  return info + QStringLiteral("<p><i>%1</i></p>").arg(line.toHtmlEscaped());
}

/**
 * @brief Takes the one API box slot and registers its closer (R1, R5, R8): the closer drops a box
 *        still queued and answers cancel for one already open. @p waiter is the marshaller whose
 *        script waits on the box, or null for a box nobody waits on.
 */
static std::shared_ptr<ApiBoxState> openApiBoxSlot(const void* waiter)
{
  SS_ASSERT_LOG(!API::UserWaitScope::boxOpen());

  auto state = std::make_shared<ApiBoxState>(ApiBoxState{nullptr, false});
  API::UserWaitScope::registerCloser(
    [state] {
      state->cancelled = true;
      if (state->box)
        state->box->done(QMessageBox::Cancel);
    },
    waiter);

  SS_ASSERT_LOG(API::UserWaitScope::boxOpen());
  return state;
}

/**
 * @brief The open hook of an API box: records the dialog for its closer, and keeps the
 *        application's own icon off a box that asked for none (R2).
 */
static MessageBoxHook apiBoxHook(const std::shared_ptr<ApiBoxState>& state, QMessageBox::Icon icon)
{
  SS_ASSERT_LOG(state != nullptr);
  return [state, icon](QMessageBox* box) {
    state->box = box;
    if (icon == QMessageBox::NoIcon)
      box->setIconPixmap(QPixmap());
  };
}

/**
 * @brief Queues an API box for the next event-loop pass. The slot stays taken until it closes, so
 *        a queued box counts as open (R1), and a closer that fires first drops it unshown.
 */
static void postApiMessageBox(const QString& headline,
                              const QString& info,
                              QMessageBox::Icon icon,
                              const QString& title)
{
  SS_ASSERT(qApp != nullptr, return);

  auto state = openApiBoxSlot(nullptr);
  QMetaObject::invokeMethod(
    qApp,
    [state, headline, info, icon, title] {
      if (!state->cancelled)
        (void)Misc::Utilities::showMessageBox(headline,
                                              info,
                                              icon,
                                              title,
                                              QMessageBox::Ok,
                                              QMessageBox::NoButton,
                                              ButtonTextMap(),
                                              apiBoxHook(state, icon));

      API::UserWaitScope::clearCloser();
    },
    Qt::QueuedConnection);
}

/**
 * @brief Shows an escaped message box via Misc::Utilities. Without wait it is posted; with wait
 *        (control script and macros only) it runs modally and answers the clicked button, and
 *        nothing dispatched inside that loop inherits the wait or the script's command policy.
 */
API::CommandResponse API::Handlers::WindowHandler::showMessageBox(const QString& id,
                                                                  const QJsonObject& params)
{
  const auto text = params.value(QStringLiteral("text")).toString();
  if (text.trimmed().isEmpty())
    return CommandResponse::makeError(id, ErrorCode::MissingParam, QStringLiteral("text is empty"));

  const auto iconName = params.value(QStringLiteral("icon")).toString(QStringLiteral("info"));
  const auto* iconIt  = std::find_if(std::begin(kMessageBoxIcons),
                                     std::end(kMessageBoxIcons),
                                     [&iconName](const auto& e) { return iconName == e.name; });
  if (iconIt == std::end(kMessageBoxIcons))
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("Unknown icon: %1").arg(iconName));

  const auto headline = text.toHtmlEscaped().replace(QLatin1Char('\n'), QStringLiteral("<br>"));
  const auto detail   = params.value(QStringLiteral("informativeText")).toString();
  const auto info     = detail.isEmpty() ? QString() : Qt::convertFromPlainText(detail);
  const auto title    = params.value(QStringLiteral("title")).toString();

  const bool wait = params.value(QStringLiteral("wait")).toBool();
  if (!wait
      && (params.contains(QStringLiteral("buttons"))
          || params.contains(QStringLiteral("defaultButton"))
          || params.contains(QStringLiteral("buttonLabels"))))
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("buttons, defaultButton and buttonLabels need wait: true"));

  const bool headless = QGuiApplication::platformName() == QLatin1String("offscreen");
  const auto refusal  = API::MessageBoxGate::refusal(
    wait, API::UserWaitScope::allowed(), headless, API::UserWaitScope::boxOpen());
  if (refusal != API::MessageBoxGate::Refusal::None)
    return CommandResponse::makeError(
      id, ErrorCode::ExecutionError, messageBoxRefusalText(refusal));

  QJsonObject result;
  if (!wait) {
    if (API::MessageBoxGate::logInsteadOfShow(headless))
      qInfo().noquote() << "[API] Message box with no display to show it:" << text << detail;
    else
      postApiMessageBox(headline, withProvenance(info), iconIt->icon, title);

    result[QStringLiteral("waited")] = false;
    return CommandResponse::makeSuccess(id, result);
  }

  const auto choice = readMessageBoxChoice(params);
  if (!choice.error.isEmpty())
    return CommandResponse::makeError(id, ErrorCode::InvalidParam, choice.error);

  const auto* waiter = API::UserWaitScope::waiter();
  const API::UserWaitScope noNestedWait(false);
  const API::DispatchPolicyScope noScriptPolicy((API::DispatchPolicy()));
  const auto state  = openApiBoxSlot(waiter);
  const int clicked = Misc::Utilities::showMessageBox(headline,
                                                      withProvenance(info),
                                                      iconIt->icon,
                                                      title,
                                                      choice.buttons,
                                                      choice.defaultButton,
                                                      choice.labels,
                                                      apiBoxHook(state, iconIt->icon));
  API::UserWaitScope::clearCloser();

  result[QStringLiteral("waited")] = true;
  result[QStringLiteral("button")] = messageBoxButtonName(clicked);
  return CommandResponse::makeSuccess(id, result);
}
