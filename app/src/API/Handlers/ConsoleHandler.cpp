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

#include "API/Handlers/ConsoleHandler.h"

#include "API/CommandRegistry.h"
#include "API/SchemaBuilder.h"
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
  registerDisplayCommands();
  registerFontAndChecksumCommands();
  registerIoAndExportCommands();
}

/**
 * @brief Register display + data-formatting commands.
 */
void API::Handlers::ConsoleHandler::registerDisplayCommands()
{
  static auto& registry = CommandRegistry::instance();

  const auto enabledSchema = API::makeSchema({
    {QStringLiteral("enabled"),
     QStringLiteral("boolean"),
     QStringLiteral("Whether to enable the feature")}
  });

  registry.registerCommand(QStringLiteral("console.setEcho"),
                           QStringLiteral("Enable/disable echo"),
                           enabledSchema,
                           &setEcho);
  registry.registerCommand(QStringLiteral("console.setShowTimestamp"),
                           QStringLiteral("Show/hide timestamps"),
                           enabledSchema,
                           &setShowTimestamp);
  registry.registerCommand(QStringLiteral("console.setCollapseDuplicates"),
                           QStringLiteral("Collapse consecutive duplicate lines"),
                           enabledSchema,
                           &setCollapseDuplicates);
  registry.registerCommand(QStringLiteral("console.setSearchCaseSensitive"),
                           QStringLiteral("Toggle case-sensitive console search"),
                           enabledSchema,
                           &setSearchCaseSensitive);
  registry.registerCommand(QStringLiteral("console.setDisplayMode"),
                           QStringLiteral("Set display mode"),
                           API::makeSchema({
                             {QStringLiteral("modeIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Display mode: 0=PlainText, 1=Hex")}
  }),
                           &setDisplayMode);
  registry.registerCommand(QStringLiteral("console.setDataMode"),
                           QStringLiteral("Set data mode"),
                           API::makeSchema({
                             {QStringLiteral("modeIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Data mode: 0=UTF8, 1=Hex")}
  }),
                           &setDataMode);
  registry.registerCommand(QStringLiteral("console.setLineEnding"),
                           QStringLiteral("Set line ending"),
                           API::makeSchema({
                             {QStringLiteral("endingIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Line ending: 0=None, 1=LF, 2=CR, 3=CRLF")}
  }),
                           &setLineEnding);
}

/**
 * @brief Register font, checksum, VT100/ANSI, and text-encoding commands.
 */
void API::Handlers::ConsoleHandler::registerFontAndChecksumCommands()
{
  static auto& registry = CommandRegistry::instance();

  registry.registerCommand(QStringLiteral("console.setFontFamily"),
                           QStringLiteral("Set font family"),
                           API::makeSchema({
                             {QStringLiteral("fontFamily"),
                              QStringLiteral("string"),
                              QStringLiteral("Font family name")}
  }),
                           &setFontFamily);
  registry.registerCommand(QStringLiteral("console.setFontSize"),
                           QStringLiteral("Set font size"),
                           API::makeSchema({
                             {QStringLiteral("fontSize"),
                              QStringLiteral("integer"),
                              QStringLiteral("Font size in points")}
  }),
                           &setFontSize);
  registry.registerCommand(QStringLiteral("console.setChecksumMethod"),
                           QStringLiteral("Set checksum method"),
                           API::makeSchema({
                             {QStringLiteral("methodIndex"),
                              QStringLiteral("integer"),
                              QStringLiteral("Checksum method index")}
  }),
                           &setChecksumMethod);
  registry.registerCommand(
    QStringLiteral("console.setVt100Emulation"),
    QStringLiteral("Toggle VT100 / xterm escape sequence interpretation"),
    API::makeSchema({
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("True to interpret cursor / clear / scroll codes as a terminal "
                      "emulator; false to display them as text")}
  }),
    &setVt100Emulation);
  registry.registerCommand(
    QStringLiteral("console.setAnsiColorsEnabled"),
    QStringLiteral("Toggle ANSI / xterm color sequences in console output"),
    API::makeSchema({
      {QStringLiteral("enabled"),
       QStringLiteral("boolean"),
       QStringLiteral("True to honor SGR color escape sequences, false to display them "
                      "as text")}
  }),
    &setAnsiColorsEnabled);
  registry.registerCommand(
    QStringLiteral("console.setEncoding"),
    QStringLiteral("Set the text encoding used to decode incoming bytes "
                   "(params: encoding -- index into console.getConfig.textEncodings)"),
    API::makeSchema({
      {QStringLiteral("encoding"),
       QStringLiteral("integer"),
       QStringLiteral("Text encoding index (UTF-8, UTF-16, ASCII, ...)")}
  }),
    &setEncoding);
}

/**
 * @brief Register lifecycle, send, export, and getConfig commands.
 */
void API::Handlers::ConsoleHandler::registerIoAndExportCommands()
{
  static auto& registry = CommandRegistry::instance();

  const auto enabledSchema = API::makeSchema({
    {QStringLiteral("enabled"),
     QStringLiteral("boolean"),
     QStringLiteral("Whether to enable the feature")}
  });
  const auto empty         = API::emptySchema();

  registry.registerCommand(
    QStringLiteral("console.clear"), QStringLiteral("Clear console"), empty, &clear);
  registry.registerCommand(QStringLiteral("console.send"),
                           QStringLiteral("Send data to device"),
                           API::makeSchema({
                             {QStringLiteral("data"),
                              QStringLiteral("string"),
                              QStringLiteral("Data string to send to device")}
  }),
                           &send);

  registry.registerCommand(QStringLiteral("consoleExport.setEnabled"),
                           QStringLiteral("Enable/disable console export"),
                           enabledSchema,
                           &exportSetEnabled);
  registry.registerCommand(QStringLiteral("consoleExport.close"),
                           QStringLiteral("Close console export file"),
                           empty,
                           &exportClose);
  registry.registerCommand(QStringLiteral("consoleExport.getStatus"),
                           QStringLiteral("Get console export status"),
                           empty,
                           &exportGetStatus);
  registry.registerCommand(QStringLiteral("console.getConfig"),
                           QStringLiteral("Get all console settings"),
                           empty,
                           &getConfiguration);
}

//--------------------------------------------------------------------------------------------------
// Setters
//--------------------------------------------------------------------------------------------------

/**
 * @brief Enable/disable echo
 */
API::CommandResponse API::Handlers::ConsoleHandler::setEcho(const QString& id,
                                                            const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setEcho(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Show/hide timestamps
 */
API::CommandResponse API::Handlers::ConsoleHandler::setShowTimestamp(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setShowTimestamp(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable/disable duplicate-line collapsing in the console view
 */
API::CommandResponse API::Handlers::ConsoleHandler::setCollapseDuplicates(const QString& id,
                                                                          const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setCollapseDuplicates(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable/disable case-sensitive console search
 */
API::CommandResponse API::Handlers::ConsoleHandler::setSearchCaseSensitive(
  const QString& id, const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setSearchCaseSensitive(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set display mode
 */
API::CommandResponse API::Handlers::ConsoleHandler::setDisplayMode(const QString& id,
                                                                   const QJsonObject& params)
{
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

  const auto mode      = static_cast<Console::Handler::DisplayMode>(mode_index);
  static auto& handler = Console::Handler::instance();
  handler.setDisplayMode(mode);

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = mode_index;
  result[QStringLiteral("modeName")] =
    (mode_index == 0) ? QStringLiteral("PlainText") : QStringLiteral("Hexadecimal");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set data mode
 */
API::CommandResponse API::Handlers::ConsoleHandler::setDataMode(const QString& id,
                                                                const QJsonObject& params)
{
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

  const auto mode      = static_cast<Console::Handler::DataMode>(mode_index);
  static auto& handler = Console::Handler::instance();
  handler.setDataMode(mode);

  QJsonObject result;
  result[QStringLiteral("modeIndex")] = mode_index;
  result[QStringLiteral("modeName")] =
    (mode_index == 0) ? QStringLiteral("UTF8") : QStringLiteral("Hexadecimal");
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set line ending
 */
API::CommandResponse API::Handlers::ConsoleHandler::setLineEnding(const QString& id,
                                                                  const QJsonObject& params)
{
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

  const auto ending    = static_cast<Console::Handler::LineEnding>(ending_index);
  static auto& handler = Console::Handler::instance();
  handler.setLineEnding(ending);

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
 */
API::CommandResponse API::Handlers::ConsoleHandler::setFontFamily(const QString& id,
                                                                  const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("fontFamily"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: fontFamily"));
  }

  const QString font_family = params.value(QStringLiteral("fontFamily")).toString();
  if (font_family.isEmpty()) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("fontFamily cannot be empty"));
  }

  static auto& handler = Console::Handler::instance();
  handler.setFontFamily(font_family);

  QJsonObject result;
  result[QStringLiteral("fontFamily")] = font_family;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set font size
 */
API::CommandResponse API::Handlers::ConsoleHandler::setFontSize(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("fontSize"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: fontSize"));
  }

  const int font_size = params.value(QStringLiteral("fontSize")).toInt();
  if (font_size <= 0) {
    return CommandResponse::makeError(
      id, ErrorCode::InvalidParam, QStringLiteral("fontSize must be greater than 0"));
  }

  static auto& handler = Console::Handler::instance();
  handler.setFontSize(font_size);

  QJsonObject result;
  result[QStringLiteral("fontSize")] = font_size;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set checksum method
 */
API::CommandResponse API::Handlers::ConsoleHandler::setChecksumMethod(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("methodIndex"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: methodIndex"));
  }

  const int method_index = params.value(QStringLiteral("methodIndex")).toInt();

  static auto& handler = Console::Handler::instance();
  const auto& methods  = handler.checksumMethods();
  if (method_index < 0 || method_index >= methods.count()) {
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid methodIndex: %1. Valid range: 0-%2"))
        .arg(method_index)
        .arg(methods.count() - 1));
  }

  handler.setChecksumMethod(method_index);

  QJsonObject result;
  result[QStringLiteral("methodIndex")] = method_index;
  result[QStringLiteral("methodName")]  = methods.at(method_index);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle VT100 / xterm escape sequence interpretation
 */
API::CommandResponse API::Handlers::ConsoleHandler::setVt100Emulation(const QString& id,
                                                                      const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setVt100Emulation(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Toggle ANSI color sequence handling
 */
API::CommandResponse API::Handlers::ConsoleHandler::setAnsiColorsEnabled(const QString& id,
                                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));

  const bool enabled   = params.value(QStringLiteral("enabled")).toBool();
  static auto& handler = Console::Handler::instance();
  handler.setAnsiColorsEnabled(enabled);

  QJsonObject result;
  result[QStringLiteral("enabled")] = enabled;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Set the text encoding used to decode incoming bytes
 */
API::CommandResponse API::Handlers::ConsoleHandler::setEncoding(const QString& id,
                                                                const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("encoding")))
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: encoding"));

  const int encoding   = params.value(QStringLiteral("encoding")).toInt();
  static auto& handler = Console::Handler::instance();
  const auto& choices  = handler.textEncodings();
  if (encoding < 0 || encoding >= choices.count())
    return CommandResponse::makeError(
      id,
      ErrorCode::InvalidParam,
      QString(QStringLiteral("Invalid encoding: %1. Valid range: 0-%2"))
        .arg(encoding)
        .arg(choices.count() - 1));

  handler.setEncoding(encoding);

  QJsonObject result;
  result[QStringLiteral("encoding")] = encoding;
  result[QStringLiteral("name")]     = choices.at(encoding);
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Clear console
 */
API::CommandResponse API::Handlers::ConsoleHandler::clear(const QString& id,
                                                          const QJsonObject& params)
{
  Q_UNUSED(params)

  static auto& handler = Console::Handler::instance();
  handler.clear();

  QJsonObject result;
  result[QStringLiteral("cleared")] = true;
  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Send data to device
 */
API::CommandResponse API::Handlers::ConsoleHandler::send(const QString& id,
                                                         const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("data"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: data"));
  }

  const QString data   = params.value(QStringLiteral("data")).toString();
  static auto& handler = Console::Handler::instance();
  handler.send(data);

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
  Q_UNUSED(params)

  static auto& console = Console::Handler::instance();

  QJsonObject result;
  result[QStringLiteral("echo")]                = console.echo();
  result[QStringLiteral("showTimestamp")]       = console.showTimestamp();
  result[QStringLiteral("displayMode")]         = static_cast<int>(console.displayMode());
  result[QStringLiteral("dataMode")]            = static_cast<int>(console.dataMode());
  result[QStringLiteral("lineEnding")]          = static_cast<int>(console.lineEnding());
  result[QStringLiteral("fontFamily")]          = console.fontFamily();
  result[QStringLiteral("fontSize")]            = console.fontSize();
  result[QStringLiteral("checksumMethod")]      = console.checksumMethod();
  result[QStringLiteral("bufferLength")]        = console.bufferLength();
  result[QStringLiteral("collapseDuplicates")]  = console.collapseDuplicates();
  result[QStringLiteral("searchCaseSensitive")] = console.searchCaseSensitive();

  return CommandResponse::makeSuccess(id, result);
}

/**
 * @brief Enable or disable console export
 */
API::CommandResponse API::Handlers::ConsoleHandler::exportSetEnabled(const QString& id,
                                                                     const QJsonObject& params)
{
  if (!params.contains(QStringLiteral("enabled"))) {
    return CommandResponse::makeError(
      id, ErrorCode::MissingParam, QStringLiteral("Missing required parameter: enabled"));
  }

  const bool enabled    = params.value(QStringLiteral("enabled")).toBool();
  static auto& exporter = Console::Export::instance();
  exporter.setExportEnabled(enabled);

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
  Q_UNUSED(params)

  static auto& exporter = Console::Export::instance();
  exporter.closeFile();

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
  Q_UNUSED(params)

  static auto& exporter = Console::Export::instance();

  QJsonObject result;
  result[QStringLiteral("enabled")] = exporter.exportEnabled();
  result[QStringLiteral("isOpen")]  = exporter.isOpen();
  return CommandResponse::makeSuccess(id, result);
}
