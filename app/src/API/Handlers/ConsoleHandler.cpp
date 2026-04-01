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

#include "API/Handlers/ConsoleHandler.h"

#include "API/CommandRegistry.h"
#include "Console/Export.h"
#include "Console/Handler.h"

//--------------------------------------------------------------------------------------------------
// Command registration
//--------------------------------------------------------------------------------------------------

/**
 * @brief Register all Console commands with the registry
 */
void API::Handlers::ConsoleHandler::registerCommands()
{
  // Obtain registry and build command schemas
  auto& registry = CommandRegistry::instance();

  // Schema for boolean "enabled" parameter
  QJsonObject enabledSchema;
  {
    QJsonObject props;
    QJsonObject enabledProp;
    enabledProp.insert("type", "boolean");
    enabledProp.insert("description", "Whether to enable the feature");
    props.insert("enabled", enabledProp);
    enabledSchema.insert("type", "object");
    enabledSchema.insert("properties", props);
    QJsonArray req;
    req.append("enabled");
    enabledSchema.insert("required", req);
  }

  // Empty schema for parameterless commands
  QJsonObject emptySchema;
  emptySchema.insert("type", "object");
  emptySchema.insert("properties", QJsonObject());

  registry.registerCommand(QStringLiteral("console.setEcho"),
                           QStringLiteral("Enable/disable echo"),
                           enabledSchema,
                           &setEcho);

  registry.registerCommand(QStringLiteral("console.setShowTimestamp"),
                           QStringLiteral("Show/hide timestamps"),
                           enabledSchema,
                           &setShowTimestamp);

  QJsonObject displayModeSchema;
  {
    QJsonObject props;
    QJsonObject modeIndexProp;
    modeIndexProp.insert("type", "integer");
    modeIndexProp.insert("description", "Display mode: 0=PlainText, 1=Hex");
    props.insert("modeIndex", modeIndexProp);
    displayModeSchema.insert("type", "object");
    displayModeSchema.insert("properties", props);
    QJsonArray req;
    req.append("modeIndex");
    displayModeSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setDisplayMode"),
                           QStringLiteral("Set display mode"),
                           displayModeSchema,
                           &setDisplayMode);

  QJsonObject dataModeSchema;
  {
    QJsonObject props;
    QJsonObject modeIndexProp;
    modeIndexProp.insert("type", "integer");
    modeIndexProp.insert("description", "Data mode: 0=UTF8, 1=Hex");
    props.insert("modeIndex", modeIndexProp);
    dataModeSchema.insert("type", "object");
    dataModeSchema.insert("properties", props);
    QJsonArray req;
    req.append("modeIndex");
    dataModeSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setDataMode"),
                           QStringLiteral("Set data mode"),
                           dataModeSchema,
                           &setDataMode);

  QJsonObject lineEndingSchema;
  {
    QJsonObject props;
    QJsonObject endingIndexProp;
    endingIndexProp.insert("type", "integer");
    endingIndexProp.insert("description", "Line ending: 0=None, 1=LF, 2=CR, 3=CRLF");
    props.insert("endingIndex", endingIndexProp);
    lineEndingSchema.insert("type", "object");
    lineEndingSchema.insert("properties", props);
    QJsonArray req;
    req.append("endingIndex");
    lineEndingSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setLineEnding"),
                           QStringLiteral("Set line ending"),
                           lineEndingSchema,
                           &setLineEnding);

  QJsonObject fontFamilySchema;
  {
    QJsonObject props;
    QJsonObject fontFamilyProp;
    fontFamilyProp.insert("type", "string");
    fontFamilyProp.insert("description", "Font family name");
    props.insert("fontFamily", fontFamilyProp);
    fontFamilySchema.insert("type", "object");
    fontFamilySchema.insert("properties", props);
    QJsonArray req;
    req.append("fontFamily");
    fontFamilySchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setFontFamily"),
                           QStringLiteral("Set font family"),
                           fontFamilySchema,
                           &setFontFamily);

  QJsonObject fontSizeSchema;
  {
    QJsonObject props;
    QJsonObject fontSizeProp;
    fontSizeProp.insert("type", "integer");
    fontSizeProp.insert("description", "Font size in points");
    props.insert("fontSize", fontSizeProp);
    fontSizeSchema.insert("type", "object");
    fontSizeSchema.insert("properties", props);
    QJsonArray req;
    req.append("fontSize");
    fontSizeSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setFontSize"),
                           QStringLiteral("Set font size"),
                           fontSizeSchema,
                           &setFontSize);

  QJsonObject checksumMethodSchema;
  {
    QJsonObject props;
    QJsonObject methodIndexProp;
    methodIndexProp.insert("type", "integer");
    methodIndexProp.insert("description", "Checksum method index");
    props.insert("methodIndex", methodIndexProp);
    checksumMethodSchema.insert("type", "object");
    checksumMethodSchema.insert("properties", props);
    QJsonArray req;
    req.append("methodIndex");
    checksumMethodSchema.insert("required", req);
  }
  registry.registerCommand(QStringLiteral("console.setChecksumMethod"),
                           QStringLiteral("Set checksum method"),
                           checksumMethodSchema,
                           &setChecksumMethod);

  registry.registerCommand(
    QStringLiteral("console.clear"), QStringLiteral("Clear console"), emptySchema, &clear);

  QJsonObject sendSchema;
  {
    QJsonObject props;
    QJsonObject dataProp;
    dataProp.insert("type", "string");
    dataProp.insert("description", "Data string to send to device");
    props.insert("data", dataProp);
    sendSchema.insert("type", "object");
    sendSchema.insert("properties", props);
    QJsonArray req;
    req.append("data");
    sendSchema.insert("required", req);
  }
  registry.registerCommand(
    QStringLiteral("console.send"), QStringLiteral("Send data to device"), sendSchema, &send);

  registry.registerCommand(QStringLiteral("console.export.setEnabled"),
                           QStringLiteral("Enable/disable console export"),
                           enabledSchema,
                           &exportSetEnabled);

  registry.registerCommand(QStringLiteral("console.export.close"),
                           QStringLiteral("Close console export file"),
                           emptySchema,
                           &exportClose);

  registry.registerCommand(QStringLiteral("console.export.getStatus"),
                           QStringLiteral("Get console export status"),
                           emptySchema,
                           &exportGetStatus);

  registry.registerCommand(QStringLiteral("console.getConfiguration"),
                           QStringLiteral("Get all console settings"),
                           emptySchema,
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable/disable echo
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setEcho(const QString& id,
                                                            const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  Console::Handler::instance().setEcho(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Show/hide timestamps
 * @param params Requires "enabled" (bool)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setShowTimestamp(const QString& id,
                                                                     const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  Console::Handler::instance().setShowTimestamp(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set display mode
 * @param params Requires "modeIndex" (int: 0=PlainText, 1=Hex)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setDisplayMode(const QString& id,
                                                                   const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("modeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: modeIndex"));
  }

  const int mode_index = params.value(QStringLiteral("modeIndex")).toInt();
  if (mode_index < 0 || mode_index > 1) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid modeIndex: must be 0 (PlainText) or 1 (Hex)"));
  }

  const auto mode = static_cast<Console::Handler::DisplayMode>(mode_index);
  Console::Handler::instance().setDisplayMode(mode);

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = mode_index;
  result[QStringLiteral("modeName")] =
    (mode_index == 0) ? QStringLiteral("PlainText") : QStringLiteral("Hexadecimal");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set data mode
 * @param params Requires "modeIndex" (int: 0=UTF8, 1=Hex)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setDataMode(const QString& id,
                                                                const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("modeIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: modeIndex"));
  }

  const int mode_index = params.value(QStringLiteral("modeIndex")).toInt();
  if (mode_index < 0 || mode_index > 1) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid modeIndex: must be 0 (UTF8) or 1 (Hex)"));
  }

  const auto mode = static_cast<Console::Handler::DataMode>(mode_index);
  Console::Handler::instance().setDataMode(mode);

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = mode_index;
  result[QStringLiteral("modeName")] =
    (mode_index == 0) ? QStringLiteral("UTF8") : QStringLiteral("Hexadecimal");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set line ending
 * @param params Requires "endingIndex" (int: 0=None, 1=LF, 2=CR, 3=CRLF)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setLineEnding(const QString& id,
                                                                  const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("endingIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: endingIndex"));
  }

  const int ending_index = params.value(QStringLiteral("endingIndex")).toInt();
  if (ending_index < 0 || ending_index > 3) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QStringLiteral("Invalid endingIndex: must be 0 (None), 1 (LF), 2 (CR), or 3 (CRLF)"));
  }

  const auto ending = static_cast<Console::Handler::LineEnding>(ending_index);
  Console::Handler::instance().setLineEnding(ending);

  QString ending_name;
  switch (ending_index) {
    case 0:
      ending_name = QStringLiteral("None");
      break;
    case 1:
      ending_name = QStringLiteral("LF");
      break;
    case 2:
      ending_name = QStringLiteral("CR");
      break;
    case 3:
      ending_name = QStringLiteral("CRLF");
      break;
  }

  QJsonObject result;
  result[QStringLiteral("endingIndex")] = ending_index;
  result[QStringLiteral("endingName")]  = ending_name;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set font family
 * @param params Requires "fontFamily" (string)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setFontFamily(const QString& id,
                                                                  const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("fontFamily"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: fontFamily"));
  }

  const QString font_family = params.value(QStringLiteral("fontFamily")).toString();
  if (font_family.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("fontFamily cannot be empty"));
  }

  Console::Handler::instance().setFontFamily(font_family);

  QJsonObject result;
  result[QStringLiteral("fontFamily")] = font_family;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set font size
 * @param params Requires "fontSize" (int)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setFontSize(const QString& id,
                                                                const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("fontSize"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: fontSize"));
  }

  const int font_size = params.value(QStringLiteral("fontSize")).toInt();
  if (font_size <= 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("fontSize must be greater than 0"));
  }

  Console::Handler::instance().setFontSize(font_size);

  QJsonObject result;
  result[QStringLiteral("fontSize")] = font_size;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set checksum method
 * @param params Requires "methodIndex" (int)
 */
API::CommandResponse API::Handlers::ConsoleHandler::setChecksumMethod(const QString& id,
                                                                      const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("methodIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: methodIndex"));
  }

  const int method_index = params.value(QStringLiteral("methodIndex")).toInt();

  const auto& methods = Console::Handler::instance().checksumMethods();
  if (method_index < 0 || method_index >= methods.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid methodIndex: %1. Valid range: 0-%2"))
        .arg(method_index)
        .arg(methods.count() - 1));
  }

  Console::Handler::instance().setChecksumMethod(method_index);

  QJsonObject result;
  result[QStringLiteral("methodIndex")] = method_index;
  result[QStringLiteral("methodName")]  = methods.at(method_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Clear console
 */
API::CommandResponse API::Handlers::ConsoleHandler::clear(const QString& id,
                                                          const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  Console::Handler::instance().clear();

  QJsonObject result;
  result[QStringLiteral("cleared")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Send data to device
 * @param params Requires "data" (string)
 */
API::CommandResponse API::Handlers::ConsoleHandler::send(const QString& id,
                                                         const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("data"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"));
  }

  const QString data = params.value(QStringLiteral("data")).toString();
  Console::Handler::instance().send(data);

  QJsonObject result;
  result[QStringLiteral("sent")]       = true;
  result[QStringLiteral("dataLength")] = data.length();
  return CommandResponse::makeSuccess(id, result);
}

//--------------------------------------------------------------------------------------------------
// Getters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Get all console settings
 */
API::CommandResponse API::Handlers::ConsoleHandler::getConfiguration(const QString& id,
                                                                     const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto& console = Console::Handler::instance();

  QJsonObject result;
  result[QStringLiteral("echo")]           = console.echo();
  result[QStringLiteral("showTimestamp")]  = console.showTimestamp();
  result[QStringLiteral("displayMode")]    = static_cast<int>(console.displayMode());
  result[QStringLiteral("dataMode")]       = static_cast<int>(console.dataMode());
  result[QStringLiteral("lineEnding")]     = static_cast<int>(console.lineEnding());
  result[QStringLiteral("fontFamily")]     = console.fontFamily();
  result[QStringLiteral("fontSize")]       = console.fontSize();
  result[QStringLiteral("checksumMethod")] = console.checksumMethod();
  result[QStringLiteral("bufferLength")]   = console.bufferLength();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable console export
 */
API::CommandResponse API::Handlers::ConsoleHandler::exportSetEnabled(const QString& id,
                                                                     const QJsonObject& params)
{
  // Validate required parameter
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled = params.value(QStringLiteral("enabled")).toBool();
  Console::Export::instance().setExportEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Close the console export file
 */
API::CommandResponse API::Handlers::ConsoleHandler::exportClose(const QString& id,
                                                                const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  Console::Export::instance().closeFile();

  QJsonObject result;
  result[QStringLiteral("closed")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Get console export status
 */
API::CommandResponse API::Handlers::ConsoleHandler::exportGetStatus(const QString& id,
                                                                    const QJsonObject& params)
{
  // Retrieve current state
  Q_UNUSED(params)

  auto& exporter = Console::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")] = exporter.exportEnabled();
  result[QStringLiteral("isOpen")]  = exporter.isOpen();
  return CommandResponse::makeSuccess(id, result);
}
